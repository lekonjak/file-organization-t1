#include "tad.h"
#include "utils.h"
#include <math.h>

struct registro {
    // Campos de tamanho fixo
    int codINEP;
    char dataAtiv[11];
    char uf[3];

    //Campos de tamanho variável
    char *nomeEscola;
    char *municipio;
    char *prestadora;
};

struct header {
    char status;
    int stackTop; //
};

enum {
    PRESTADORA,
    DATA_ATIV,
    COD_INEP,
    NOME_ESCOLA,
    MUNICIPIO,
    UF
};

#define COD_INEP_SIZE sizeof(int)
#define UF_SIZE 2
#define DATA_ATIV_SIZE 10
#define FIX_FIELDS_SIZE UF_SIZE + DATA_ATIV_SIZE + COD_INEP_SIZE
#define REG_SIZE 87

/* A função lê uma linha de cada vez do arquivo csv, separa cada campo, e escreve os dados
 * em outro arquivo binário. Uma mensagem de erro é printada caso o arquivo de entrada e/ou saída
 * não consiga ser aberto.
 *
 * A função freadline (utils.h) retorna a linha lida do arquivo de entrada
 * A função split (utils.h) separa cada um dos campos da linha com strsep (man 3 strsep)
 * Ps: strsep é mais "robusta" que strtok pois suporta delimitadores consecutivos.
 * No arquivo de entrada, ";;" indica um campo "null". Usando strtok os campos 'null' seriam ignorados
 */
void csv2bin(char *filename) {
    FILE *infile = NULL, *outfile = NULL;
    char *linha = NULL;
    char **fields = NULL;
    int regSize = 0;
    Registro r = {0};
    Header h = {0};

    infile = fopen(filename, "r");

    if(infile == NULL) {
        fprintf(stdout, "Falha no carregamento do arquivo\n");
        return;
    }

    outfile = fopen("output.dat", "wb");

    if(outfile == NULL) {
        fprintf(stdout, "Falha no carregamento do arquivo\n");
        return;
    }

    h.status = 0;
    h.stackTop = -1;

    //Write header
    fwrite(&h.status, sizeof(char), 1, outfile);
    fwrite(&h.stackTop, sizeof(int), 1, outfile);

    while(!feof(infile)) {
        linha = freadline(infile);

        //freadline returns null if it is EOF
        if(linha != NULL) {
            fields = split(linha);

            //Reg size
            regSize = strlen(fields[PRESTADORA]) + strlen(fields[NOME_ESCOLA]) + strlen(fields[MUNICIPIO]); // Campos de tamanho variável
            regSize += COD_INEP_SIZE + UF_SIZE + DATA_ATIV_SIZE + 3 * sizeof(int); // Campos de tamanho fixo + 3 indicadores de tamanho

            r.codINEP = atoi(fields[COD_INEP]);
            r.nomeEscola = fields[NOME_ESCOLA];
            r.municipio = fields[MUNICIPIO];
            r.prestadora = fields[PRESTADORA];

            int size;
            //Write each entry
            fwrite(&r.codINEP, sizeof(int), 1, outfile);
            fwrite(fields[DATA_ATIV], 10*sizeof(char), 1, outfile);
            fwrite(fields[UF], 2*sizeof(char), 1, outfile);

            //Tamanho e nome da escola
            size = strlen(r.nomeEscola);
            fwrite(&size, sizeof(int), 1, outfile);
            fwrite(r.nomeEscola, size*sizeof(char), 1, outfile);

            //Tamanho e nome do município
            size = strlen(r.municipio);
            fwrite(&size, sizeof(int), 1, outfile);
            fwrite(r.municipio, size*sizeof(char), 1, outfile);

            //O ultimo campo ocupará todo o restante do registro
            size = REG_SIZE - regSize + strlen(r.prestadora);
            fwrite(&size, sizeof(int), 1, outfile);
            fwrite(r.prestadora, size*sizeof(char), 1, outfile);

            for(int i = 0; i < 6; i++)
                free(fields[i]);
            free(fields);
            free(linha);
        }
    }

    fprintf(stdout, "Arquivo carregado.\n");

    //Fechar arquivos de entrada e saída
    fclose(infile);
    fclose(outfile);
}

/* Retorna o tamanho do arquivo fp
 * Usado para checar se chegamos no fim do arquivo durante a leitura dos dados
 */
int eof(FILE *fp) {
    int cur = ftell(fp);

    fseek(fp, 0, SEEK_END);
    int max = ftell(fp);

    fseek(fp, cur, SEEK_SET);

    return max;
}

/* Checa se a posição atual no arquivo é menor que o tamanho, ou seja
 * verifica se não é fim de arquivo.
 */
int workingfeof(FILE *fp, int size) {
    if(ftell(fp) < size)
        return 0;
    return 1;
}


void bin2out(void) {
    FILE *fp;
    Registro r = {0};
    int sizeEscola, sizeMunicipio, sizePrestadora;
    long offset = 5;

    fp = fopen("output.dat", "rb");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo\n");
    }

    fseek(fp, offset, SEEK_SET);

    int max = eof(fp);

    while(!workingfeof(fp, max)) {
        fread(&r.codINEP, sizeof(int), 1, fp);
        if(r.codINEP == -1) {
            fprintf(stderr, "Registro removido\n");
            fseek(fp, REG_SIZE - COD_INEP_SIZE, SEEK_CUR);
        } else {
            fread(r.dataAtiv, 10*sizeof(char), 1, fp);
            fread(r.uf, 2*sizeof(char), 1, fp);
            fread(&sizeEscola, sizeof(int), 1, fp);
            r.nomeEscola = calloc (sizeEscola+1, sizeof(char));
            fread(r.nomeEscola, sizeEscola*sizeof(char), 1, fp);
            fread(&sizeMunicipio, sizeof(int), 1, fp);
            r.municipio = calloc (sizeMunicipio+1, sizeof(char));
            fread(r.municipio, sizeMunicipio*sizeof(char), 1, fp);
            fread(&sizePrestadora, sizeof(int), 1, fp);
            r.prestadora = calloc (sizePrestadora+1, sizeof(char));
            fread(r.prestadora, sizePrestadora*sizeof(char), 1, fp);

            sizePrestadora = strlen(r.prestadora);
            catReg(&r, sizeEscola, sizeMunicipio, sizePrestadora);

            free(r.nomeEscola);
            free(r.municipio);
            free(r.prestadora);
        }
        continue;
    }

    fclose(fp);
}

/* Função para printar os cados de um registro
 */
void catReg(Registro *reg, int sizeEscola, int sizeMunicipio, int sizePrestadora) {
    fprintf(stdout, "%d %s %s %d %s %d %s %d %s\n", \
            reg->codINEP, reg->dataAtiv, reg->uf, sizeEscola,\
            reg->nomeEscola, sizeMunicipio, reg->municipio, \
            sizePrestadora, reg->prestadora);
}

/* Imprime registro em stderr.
 * Deve ser usado apenas para printar os registros
 * dentro dos ifs de debug
 */
void stderrCatReg(Registro *reg, int sizeEscola, int sizeMunicipio, int sizePrestadora) {
    fprintf(stderr, "%d %s %s %d %s %d %s %d %s\n", \
            reg->codINEP, reg->dataAtiv, reg->uf, sizeEscola,\
            reg->nomeEscola, sizeMunicipio, reg->municipio, \
            sizePrestadora, reg->prestadora);
}

void bin2outGrep(char *category, void *element, int (*cmp)(void *, void *)) {
    FILE *fp;
    Registro r = {0};

    int sizeEscola, sizeMunicipio, sizePrestadora, flag = 0;
    long offset = 5;
    void *this = NULL;
    fp = fopen("output.dat", "rb");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo\n");
        return;
    }

    fseek(fp, offset, SEEK_SET);

    int max = eof(fp);

    while(!workingfeof(fp, max)) {
        fread(&r.codINEP, sizeof(int), 1, fp);
        fread(r.dataAtiv, 10*sizeof(char), 1, fp);
        fread(r.uf, 2*sizeof(char), 1, fp);
        fread(&sizeEscola, sizeof(int), 1, fp);
        r.nomeEscola = calloc (sizeEscola+1, sizeof(char));
        fread(r.nomeEscola, sizeof(char)*sizeEscola, 1, fp);
        fread(&sizeMunicipio, sizeof(int), 1, fp);
        r.municipio = calloc (sizeMunicipio+1, sizeof(char));
        fread(r.municipio, sizeof(char)*sizeMunicipio, 1, fp);
        fread(&sizePrestadora, sizeof(int), 1, fp);
        r.prestadora = calloc (sizePrestadora+1, sizeof(char));
        fread(r.prestadora, sizeof(char)*sizePrestadora, 1, fp);

        sizePrestadora = strlen(r.prestadora);

#ifdef DEBUG
        fprintf(stderr, "Category %c\n", category[0]);
        stderrCatReg(&r, sizeEscola, sizeMunicipio, sizePrestadora);
#endif

        //Retorna qual dos campos estamos querendo fazer a busca
        this = category[0] == 'c' ? &r.codINEP :\
                category[0] == 'd' ? r.dataAtiv :\
                 category[0] == 'u' ? r.uf :\
                  category[0] == 'n' ? r.nomeEscola :\
                   category[0] == 'p' ? r.prestadora :\
                    category[0] == 'm' ? r.municipio: this; // precompilator gave me no choice, i had to cast do avoid warning

        //Se não for nenhum dos campos existentes no registro
        if(this == NULL) {
            fclose(fp);
            free(r.nomeEscola);
            free(r.municipio);
            free(r.prestadora);

            category[0] == 'c' ? free(element) : "c:";

            fprintf(stdout, "Falha no processamento do arquivo.\n");
            return;
        }

        //Compara campo buscado com o do registro
        //Se for igual, imprime tal registro
        if(!cmp(element, this)) {
            catReg(&r, sizeEscola, sizeMunicipio, sizePrestadora);
            flag++;
        }

        free(r.nomeEscola);
        free(r.municipio);
        free(r.prestadora);
    }

    if(!flag)
        fprintf(stdout, "Registro inexistente.\n");

    category[0] == 'c' ? free(element) : 0;
    fclose(fp);
}

void *maybeConvert(char *c, char d) {
    if(d == 'c') {
        int *a = (int *) malloc (sizeof(int));
        *a = atoi(c);
        return a;
    }
    return c;
}

/* Retorna uma função de comparação dependendo do tipo do campo buscado:
 * Se for codINEP, a função deverá comparar 2 inteiros.
 * Caso contrário, a função compara strings
 */
void *selectCmp(char cat) {
#ifdef DEBUG
    fprintf(stderr, "cmp int? %s\n", cat == 'c'? "yes" : "no");
#endif
    return cat == 'c' ? &intCmp : &sstrCmp;
}

/* Função de comparação entre 2 inteiros
 * Retorna 0 se forem iguais, 1 caso contrário
 */
int intCmp(void *a, void *b) {
#ifdef DEBUG
    fprintf(stderr, "comparing %d %d.... equal? %s\n", *((int *)(a)), *((int *)(b)), *((int *)(a)) == *((int *)(b)) ? "yes" : "no");
#endif
    return *((int *)(a)) == *((int *)(b)) ? 0 : 1;
}

/* Função de comparação entre 2 strings
 * Retorna 0 se forem iguais, 1 caso contrário
 */
int sstrCmp(void *a, void *b) {
#ifdef DEBUG
    fprintf(stderr, "comparing %s %s.... equal? %s\n", (char *)a, (char *)b,strcmp((char *)a, (char *)b) == 0 ? "yes" : "no");
#endif
    return strcmp((char *)a, (char *)b);
}

void bin2outRRN(int RRN) {
    /*
     *  we need to search by RRN not directly, but dinamically - next step is to write a search function
     * */
    FILE *fp;
    Registro r = {0};
    int sizeEscola, sizeMunicipio, sizePrestadora;
    long offset = sizeof(int) + sizeof(char);

    fp = fopen("output.dat", "rb");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo.\n");
        return;
    }

    int max = eof(fp);
    fseek(fp, offset + (RRN * REG_SIZE), SEEK_SET);
    if(!workingfeof(fp, max)) {
        fread(&r.codINEP, sizeof(int), 1, fp);
        if(r.codINEP != -1) {
            fread(r.dataAtiv, 10*sizeof(char), 1, fp);
            fread(r.uf, 2*sizeof(char), 1, fp);
            fread(&sizeEscola, sizeof(int), 1, fp);
            r.nomeEscola = calloc(sizeEscola+1, sizeof(char));
            fread(r.nomeEscola, sizeEscola*sizeof(char), 1, fp);
            fread(&sizeMunicipio, sizeof(int), 1, fp);
            r.municipio = calloc(sizeMunicipio+1, sizeof(char));
            fread(r.municipio, sizeMunicipio*sizeof(char), 1, fp);
            fread(&sizePrestadora, sizeof(int), 1, fp);
            r.prestadora = calloc(sizePrestadora+1, sizeof(char));
            fread(r.prestadora, sizePrestadora*sizeof(char), 1, fp);

            sizePrestadora = strlen(r.prestadora);

            catReg(&r, sizeEscola, sizeMunicipio, sizePrestadora);

            free(r.nomeEscola);
            free(r.municipio);
            free(r.prestadora);
        } else {
            printf("Registro inexistente.\n");
        }
    } else {
        fprintf(stdout, "Registro inexistente.\n");
    }

    fclose(fp);
}

void bin2trashRRN(int RRN) {
    FILE *fp = fopen("output.dat", "r+b");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo.\n");
        return;
    }

    int max = eof(fp);

#ifdef DEBUG
    fprintf(stderr, "File size is %d\n", max);
#endif

    int stackTop;
    int removed = -1;
    int check;

    //Skipar char de status
    fseek(fp, 1, SEEK_SET);
    //Ler topo da pilha
    fread(&stackTop, sizeof(int), 1, fp);

    //Caso não exista o registro
    if((RRN * REG_SIZE) > max) {
        fprintf(stderr, "Max is %d RRN * regSize is %d\n", max, (RRN * REG_SIZE));
        fprintf(stdout, "Registro inexistente.\n");
        fclose(fp);
        return;
    }

    fseek(fp, (RRN * REG_SIZE), SEEK_CUR);

    fread(&check, sizeof(int), 1, fp);

#ifdef DEBUG
    fprintf(stdout, "check is %d\n", check);
#endif

    if(check == -1) {
        fprintf(stdout, "Registro inexistente.\n");
        fclose(fp);
        return;
    }

#ifdef DEBUG
    fprintf(stderr, "ftell is %ld\n", ftell(fp));
#endif

    //Go back 4 bytes
    fseek(fp, -4, SEEK_CUR);

#ifdef DEBUG
    fprintf(stderr, "ftell after fseek is %ld\n", ftell(fp));
#endif

    //TODO descobrir porque o fwrite não funciona
    //Modo incorreto na abertuda do arquivo?

    //Escrever -1 para sinalizar registro como "apagado"
    fwrite(&removed, sizeof(int), 1, fp);

    //Escrever topo antigo da pilha
    fwrite(&stackTop, sizeof(int), 1, fp);

    //Voltar no arquivo e escrever o novo topo da pilha de remoções
    fseek(fp, 1, SEEK_SET);
    fwrite(&RRN, sizeof(int), 1, fp);

    fprintf(stdout, "Registro removido com sucesso.\n");

    fclose(fp);
}

void add2bin(char *argv[]) {
    Registro r = {0};
    r.codINEP = atoi(argv[2]);
    strcpy(r.dataAtiv, argv[3]);
    strcpy(r.uf, argv[4]);
    r.nomeEscola = argv[5];
    r.municipio = argv[6];
    r.prestadora = argv[7];

    FILE *fp = fopen("output.dat", "r+b");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo.\n");
        return;
    }

    // começamos a trabalhar no arquivo, então ele está indisponĩvel
    char status = 1;
    fwrite(&status, sizeof(char), 1, fp);

    int stackTop;
    fread(&stackTop, sizeof(int), 1, fp);

    int newTop = stackTop;
    // se pilha está vazia, o arquivo não possui espaços para serem reutilizados
    if(stackTop == -1) {
        fseek(fp, 0, SEEK_END);
    } else {
        int offset = stackTop * REG_SIZE + sizeof(int); // vamos ao rrn do último removido e pulamos o codINEP
        fseek(fp, offset, SEEK_CUR);                    // para ler diretamente o pŕoximo topo
        fread(&newTop, sizeof(int), 1, fp);
        fseek(fp, -2 * sizeof(int), SEEK_CUR); // retrocedemos ao começo do registro
        printf("%ld\n", ftell(fp));
    }

    int regSize = FIX_FIELDS_SIZE + 3 * sizeof(int);
    fwrite(&r.codINEP, sizeof(int), 1, fp);
    fwrite(r.dataAtiv, sizeof(char), 10, fp);
    fwrite(r.uf, sizeof(char), 2, fp);

    int size = strlen(r.nomeEscola);
    regSize += size;
    fwrite(&size, sizeof(int), 1, fp);
    fwrite(r.nomeEscola, sizeof(char), size, fp);

    size = strlen(r.municipio);
    regSize += size;
    fwrite(&size, sizeof(int), 1, fp);
    fwrite(r.municipio, sizeof(char), size, fp);

    size = REG_SIZE - regSize;
    char *formatted_prestadora = (char *) calloc(size, sizeof(char));
    formatted_prestadora = strcpy(formatted_prestadora, r.prestadora);
    fwrite(&size, sizeof(int), 1, fp);
    fwrite(formatted_prestadora, sizeof(char), size, fp);
    free(formatted_prestadora);

    status = 0;
    fseek(fp, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, fp);
    fwrite(&newTop, sizeof(int), 1, fp);
    fclose(fp);
}

/*Funcao de update, recebe o argumento do console,
 *busca o registro por RNN e o troca seus campos pelos
 *campos do argumento
 */
void updateBin(char *argv[]) {
    return;
}

/* Função para desfragmentar arquivo de dados
 *
 * Lê um arquivo de dados, que supostamente terão arquivos removidos,
 * recuperando os registros e armazenando-os novamente - aqueles que não foram removidos -
 * em outro arquivo de dados
 */
void binDefrag(void) {
    //Renomear o arquivo de dados "antigo"
    //Para que o atualizado sempre esteja com o mesmo nome
    if(rename("output.dat", "output.dat.old") != 0) {
        fprintf(stdout, "Falha no processamento do arquivo.\n");
        fprintf(stderr, "output.dat does not exist\n");
        return;
    }

    //Abrir os 2 arquivos
    FILE *fp_old = fopen("output.dat.old","r+b");
    FILE *fp_new = fopen("output.dat", "w+b");

    int status = 1;
    int stackTop = -1;
    fwrite(&status, sizeof(char), 1, fp_new);
    fwrite(&stackTop, sizeof(int), 1, fp_new);

    Registro r = {0};
    int max = eof(fp_old);
    int regSize, sizeEscola, sizeMunicipio, sizePrestadora;

    fseek(fp_old, sizeof(char) + sizeof(int), SEEK_SET);

    while(!workingfeof(fp_old, max)) {
        regSize += COD_INEP_SIZE + UF_SIZE + DATA_ATIV_SIZE + 3 * sizeof(int);
        fread(&r.codINEP, COD_INEP_SIZE, 1, fp_old);

#ifdef DEBUG
        fprintf(stderr, "codInep is %d\n", r.codINEP);
#endif

        //Ler todos os campos do registro
        //Se não foi removido, ler o restante dos campos
        //e escrever no novo arquivo de dados
        if(r.codINEP != -1) {
            fread(r.dataAtiv, 10*sizeof(char), 1, fp_old);
            fread(r.uf, 2*sizeof(char), 1, fp_old);
            fread(&sizeEscola, sizeof(int), 1, fp_old);
            r.nomeEscola = calloc (sizeEscola+1, sizeof(char));
            fread(r.nomeEscola, sizeEscola*sizeof(char), 1, fp_old);
            fread(&sizeMunicipio, sizeof(int), 1, fp_old);
            r.municipio = calloc (sizeMunicipio+1, sizeof(char));
            fread(r.municipio, sizeMunicipio*sizeof(char), 1, fp_old);
            fread(&sizePrestadora, sizeof(int), 1, fp_old);
            r.prestadora = calloc (sizePrestadora+1, sizeof(char));
            fread(r.prestadora, sizePrestadora*sizeof(char), 1, fp_old);

            //Reescrever no novo arquivo
            //TODO arrumar sizePrestadora
#ifdef DEBUG
            fprintf(stderr, "new file at %ld\n", ftell(fp_new));
#endif

            fwrite(&r.codINEP, sizeof(int), 1, fp_new);
            fwrite(r.dataAtiv, 10*sizeof(char), 1, fp_new);
            fwrite(r.uf, 2*sizeof(char), 1, fp_new);
            fwrite(&sizeEscola, sizeof(int), 1, fp_new);
            fwrite(r.nomeEscola, sizeEscola*sizeof(char), 1, fp_new);
            fwrite(&sizeMunicipio, sizeof(int), 1, fp_new);
            fwrite(r.municipio, sizeMunicipio*sizeof(char), 1, fp_new);
            fwrite(&sizePrestadora, sizeof(int), 1, fp_new);
            fwrite(r.prestadora, sizePrestadora*sizeof(char), 1, fp_new);

            free(r.nomeEscola);
            free(r.municipio);
            free(r.prestadora);
        } else {
            //Removed, skip this one
            fseek(fp_old, REG_SIZE - COD_INEP_SIZE, SEEK_CUR);
            continue;
        }
    }

    //Quando finalizar, escrever 0 no status
    status = 0;
    fseek(fp_new, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, fp_new);

    fclose(fp_old);
    fclose(fp_new);

    //Deletar arquivo de dados antigo
    //remove("output.dat.old");
}

/* Função que imprime a pilha de registros removidos
 */
void printPilha(FILE *fp, int top, int offset) {
    printf("%d", top);
    fseek(fp, top * REG_SIZE + offset + sizeof(int), SEEK_SET);
    fread(&top, sizeof(int), 1, fp);

    //Se o topo for -1, acabou a pilha
    if (top == -1) {
        printf("\n");
    } else {
        //Caso contrário, chama recursivamente para o próximo elemento da pilha
        printf(" ");
        printPilha(fp, top, offset);
    }
}

/* rec2bin recupera e imprime na tela a pilha de RRNs removidos do arquivo de dados
 */
void recBin(void) {
    FILE *fp = fopen("output.dat", "rb");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo.\n");
        return;
    }

    char status = 1;
    fwrite(&status, sizeof(char), 1, fp);
    fseek(fp, 1, SEEK_CUR);

    //Lê o topo da pilha
    int top; //🔝
    fread(&top, sizeof(int), 1, fp);

    if(top == -1) {
        fprintf(stdout, "Pilha vazia.\n");
    } else {
        printPilha(fp, top, sizeof(int) + sizeof(char));
    }

    status = 0;
    fseek(fp, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, fp);
    fclose(fp);
}
