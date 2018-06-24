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

typedef struct no{
    int n;
    int filho[10];
    int cod[9];
    int rnn[9];
}No;

typedef struct BufferPool{
    No pool[5];
}bufferPool;

#define COD_INEP_SIZE sizeof(int)
#define UF_SIZE 2*sizeof(char)
#define DATA_ATIV_SIZE 10*sizeof(char)
#define FIX_FIELDS_SIZE UF_SIZE + DATA_ATIV_SIZE + COD_INEP_SIZE
#define REG_SIZE 87*sizeof(char)
#define NODE_SIZE 116*sizeof(char)
#define NODE_HEADER_SIZE sizeof(char) + (2*sizeof(int)

//Page hit e Page Faults
int fault = 0;
int hit = 0;

//Most Recently Used Flag
int mru;

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

/* Função para recuperar todos registros não removidos do arquivo de dados
 * e imprimir as informações na tela.
 */
void bin2out(void) {
    FILE *fp;
    Registro r = {0};
    int sizeEscola, sizeMunicipio, sizePrestadora;

    fp = fopen("output.dat", "rb");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo\n");
    }

    fseek(fp, sizeof(char) + sizeof(int), SEEK_SET);

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

/* Função que imprime os dados de todos os registros que contenham um certo campo buscado
 */
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
            if(r.codINEP != -1) {
                catReg(&r, sizeEscola, sizeMunicipio, sizePrestadora);
                flag++;
            }
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

/* Função que converte uma string para um número int caso for necessário
 * Se o conteúdo de *c for um número, ele será convertido para inteiro.
 * Se for uma string nada será feito.
 */
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
    return cat == 'c' ? &intCmp : &sstrCmp;
}

/* Função de comparação entre 2 inteiros
 * Retorna 0 se forem iguais, 1 caso contrário
 */
int intCmp(void *a, void *b) {
    return *((int *)(a)) == *((int *)(b)) ? 0 : 1;
}

/* Função de comparação entre 2 strings
 * Retorna 0 se forem iguais, 1 caso contrário
 */
int sstrCmp(void *a, void *b) {
    return strcmp((char *)a, (char *)b);
}

/* Função que retorna os dados do registro de RRN especificado.
 *
 * Checamos se existe um registro com o tal RRN. Se não houver, exibe-se uma mensagem de erro
 * Caso o registro for encontrado, imprimir os dados na tela.
 */
void bin2outRRN(int RRN) {
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
            fprintf(stdout, "Registro inexistente.\n");
        }
    } else {
        fprintf(stdout, "Registro inexistente.\n");
    }

    fclose(fp);
}

/* Função que remove o registro especificado pelo RRN passado por parâmetro
 * Caso o registro já tenha sido removido ou caso seja inexistente, uma mensagem de erro é exibida.
 * A remoção é apenas lógica - marcamos os registros removidos com -1 no campo codINEP.
 */
void bin2trashRRN(int RRN) {
    FILE *fp = fopen("output.dat", "r+b");

    if(fp == NULL) {
        fprintf(stdout, "Falha no processamento do arquivo.\n");
        return;
    }

    int max = eof(fp);

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

    if(check == -1) {
        fprintf(stdout, "Registro inexistente.\n");
        fclose(fp);
        return;
    }

    //Go back 4 bytes
    fseek(fp, -4, SEEK_CUR);

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

/* Função que adiciona um novo registro no arquivo de dados
 *
 * A inserção começa com uma "busca" pelos registros removidos:
 *  - Se algum registro foi anteriormente removido, o novo entrará no lugar dele.
 *  - Caso contrário, registro será inserido no final
 */
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
        //Outro print desnecessário aqui
        //printf("%ld\n", ftell(fp));
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
    //Print desnecessário aqui
    //printf("Falha no processamento do arquivo.\n");
}

/* Funcao de update, recebe o argumento do console,
 * busca o registro por RNN e o troca seus campos pelos
 * campos do argumento
 */
void updateBin(char *argv[]) {
	//Converte o rnn
	int rnn = atoi(argv[2]);

    //Copia os dados
    Registro r = {0};
    r.codINEP = atoi(argv[3]);
    strcpy(r.dataAtiv, argv[4]);
    strcpy(r.uf, argv[5]);
    r.nomeEscola = argv[6];
    r.municipio = argv[7];
    r.prestadora = argv[8];

    //Abre o arquivo corrente
    FILE *current = fopen("output.dat", "r+b");

    //Se o arquivo for nulo, relata o erro
    if(current == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //Verifica se e um rnn plausivel, se nao for, relata o erro
    if(rnn* REG_SIZE > eof(current)){
        printf("Registro inexistente.\n");
        return;
    }

    //Vai para o rnn
    fseek(current, 5+(rnn* REG_SIZE) , SEEK_SET);
    int status;
    fread(&status, sizeof(int), 1, current);

    //Se tiver sido removido relata o erro
    if(status == -1) {
        printf("Registro inexistente.\n");
        return;
    }
    //Caso nao ocorra nenhum erro
    else {
        //Volta o fp
        fseek(current, -4, SEEK_CUR);
        //Escreve os campos fixos
        int regSize = FIX_FIELDS_SIZE + 3 * sizeof(int);
        fwrite(&r.codINEP, sizeof(int), 1, current);
        fwrite(r.dataAtiv, sizeof(char), 10, current);
        fwrite(r.uf, sizeof(char), 2, current);
        //Verifica o tamanho, escreve o tamanho e o campo variavel "Nome Escola"
        int size = strlen(r.nomeEscola);
        regSize += size;
        fwrite(&size, sizeof(int), 1, current);
        fwrite(r.nomeEscola, sizeof(char), size, current);
        //Verifica o tamanho, escreve o tamanho e o campo variavel "Municipio"
        size = strlen(r.municipio);
        regSize += size;
        fwrite(&size, sizeof(int), 1, current);
        fwrite(r.municipio, sizeof(char), size, current);
        ////Verifica o tamanho, escreve o tamanho e o campo variavel "Prestadora"
        size = REG_SIZE - regSize;
        char *formatted_prestadora = (char *) calloc(size, sizeof(char));
        formatted_prestadora = strcpy(formatted_prestadora, r.prestadora);
        fwrite(&size, sizeof(int), 1, current);
        fwrite(formatted_prestadora, sizeof(char), size, current);
        free(formatted_prestadora);
        printf("Registro alterado com sucesso.\n");
    }
    fclose(current);
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

    fprintf(stdout, "Arquivo de dados compactado com sucesso.\n");

    fclose(fp_old);
    fclose(fp_new);

    //Deletar arquivo de dados antigo
    remove("output.dat.old");
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

/*Buffer Info Append, informacoes de faults e hits guardadas apos cada execuçao*/
void bufferInfo()
{
    //Texto a ser escrito
    char pf[12] = {'P','a','g','e',' ','f','a','u','l','t',':',' '};
    char ph[12] = {';',' ','P','a','g','e',' ','h','i','t',':',' '};
    //Abre o arquivo e verifica se pode ser utilizado
    FILE *append;
    fopen("buffer-info.text", "a");
    if(append == NULL){
        fprintf(stdout, "Falha no processamento do arquivo 'buffer-info.text'.\n");
        return;
    }
    //Escreve no final 
    fwrite(pf, sizeof(char), 12, append);
    fwrite(&fault, sizeof(int), 1, append);
    fwrite(ph, sizeof(char), 12, append);
    fwrite(&hit, sizeof(int), 1, append);

    fclose(append);
}

/*Funçao que cria o buffer pool, e armazena o no raiz nela*/
bufferPool criaBuffer()
{
    //Cria Buffer Pool
    bufferPool buffer;

    //Abre o arquivo de indice
    FILE *indice;
    indice = fopen("indice.dat", "rb");

    //Busca o rnn local da raiz
    int root_rnn;
    fseek(indice, sizeof(char), SEEK_SET);
    fread(&root_rnn, sizeof(int), 1, indice);

    //Busca a raiz
    fseek(indice, (NODE_SIZE*root_rnn) + sizeof(int), SEEK_CUR);

    //Copia os elementos
    fread(&buffer.pool[0].n, sizeof(int), 1, indice);
    for (int i = 0; i < buffer.pool[0].n; ++i)
    {
        //Le o ponteiro
        fread(&buffer.pool[0].filho[i], sizeof(int), 1, indice);
        //Le o cod
        fread(&buffer.pool[0].cod[i], sizeof(int), 1, indice);
        //Le o rnn
        fread(&buffer.pool[0].rnn[i], sizeof(int), 1, indice); 
    }
    //Le o ultimo ponteiro
    fread(&buffer.pool[0].filho[9], sizeof(int), 1, indice);

    fclose(indice);
    //Retorna o bufferPool
    return buffer;
}   


/*Funcao que busca o No na pagina do arquivo de indice, a partir do rnn requerido*/
No *indexGetNo(int rnn)
{
    //Abre o arquivo de indice
    FILE *indice;
    indice = fopen("indice.dat", "rb");

    //Coloca o fp no local do rnn
    fseek(indice, (sizeof(char) + (2*sizeof(int)) + NODE_SIZE * rnn), SEEK_SET);
    //Cria o no e copia os dados
    No *node = (No*) malloc(sizeof(No));
    //Copia os elementos
    fread(&node->n, sizeof(int), 1, indice);
    for (int i = 0; i < node->n; ++i)
    {
        //Le o ponteiro
        fread(&node->filho[i], sizeof(int), 1, indice);
        //Le o cod
        fread(&node->cod[i], sizeof(int), 1, indice);
        //Le o rnn
        fread(&node->rnn[i], sizeof(int), 1, indice); 
    }
    //Le o ultimo ponteiro
    fread(&node->filho[9], sizeof(int), 1, indice);
    //Fecha o arquivo e retorna o no
    fclose(indice);
    return node;
}


/*Funcao que retorna o no do buffer pool necessario*/
No *bufferGetNo(bufferPool *buffer, int rnn)
{   
    //Busca em cada elemento 
    for (int i = 0; i < 5; ++i)
    {
        //Se for a pagina que queremos
        if(*(buffer->pool[i].cod) == rnn)
        {
            hit++;
            return &buffer->pool[i];
        }
    }   
    //Se nao encontrar busca no arquivo indice
    fault++;
    //Copia o no da memoria
    No *aux =  (No*) malloc(sizeof(No));
    aux = indexGetNo(rnn);
    buffer->pool[mru].n = aux->n;
    for (int i = 0; i < 9; ++i)
    {
        buffer->pool[mru].filho[i] = aux->filho[i];
        buffer->pool[mru].cod[i] = aux->cod[i];
        buffer->pool[mru].rnn[i] = aux->rnn[i];
    }
    buffer->pool[mru].filho[9] = aux->filho[9];
    return &buffer->pool[mru];
}   

/*Funcao que atualiza o no no buffer Pool*/
void bufferAtualizaNo(bufferPool *buffer, No *atualizar, int rnn)
{
    //Abre o arquivo de indice
    FILE *indice;
    indice = fopen("indice.dat", "rb");

    //Busca o No no BufferPool
    for (int i = 0; i < 5; ++i)
    {
        //Se for o mesmo
        if(*(buffer->pool[i].cod) == rnn)
        {
            //Atualiza no BufferPool
            memcpy(&(buffer->pool[i]), atualizar, sizeof(No));
            //Atualiza no Arquivo de Indice
            fseek(indice, (sizeof(char) + (2*sizeof(int))), SEEK_SET);

            //Copia os elementos
            fwrite(&atualizar->n, sizeof(int), 1, indice);
            for (int i = 0; i < atualizar->n; ++i)
            {
                //Escreve o ponteiro
                fwrite(&atualizar->filho[i], sizeof(int), 1, indice);
                //Escreve o cod
                fwrite(&atualizar->cod[i], sizeof(int), 1, indice);
                //Escreve o rnn
                fwrite(&atualizar->rnn[i], sizeof(int), 1, indice); 
            }
            //Escreve o ultimo ponteiro
            fwrite(&atualizar->filho[9], sizeof(int), 1, indice);

            break;
        }
    }

    fclose(indice);
}



