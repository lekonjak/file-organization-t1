#include "tad.h"
#include "utils.h"

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
    int stackTop; //🔝
};

enum {
    PRESTADORA,
    DATA_ATIV,
    COD_INEP,
    NOME_ESCOLA,
    MUNICIPIO,
    UF
};

#define COD_INEP_SIZE 4
#define UF_SIZE 2
#define DATA_ATIV_SIZE 10
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

            size = strlen(r.municipio);
            fwrite(&size, sizeof(int), 1, outfile);
            fwrite(r.municipio, size*sizeof(char), 1, outfile);

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
    fseek(fp, offset, SEEK_SET);

    int max = eof(fp);

    while(!workingfeof(fp, max)) {
        fread(&r.codINEP, sizeof(int), 1, fp);
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
 * Deve ser usado apenas para printar os registros dentro
 * dos ifs de debug
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
    int max = eof(fp);
    fseek(fp, offset + (RRN * REG_SIZE), SEEK_SET);
    if(!workingfeof(fp, max)) {
        fread(&r.codINEP, sizeof(int), 1, fp);
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

    fclose(fp);
}

void bin2trashRRN(int RRN) {
    (void) RRN;
}

void add2bin(char *argv[]) {
    (void) argv;
}

void updateBin(char *argv[]) {
    (void) argv;
}

void binDefrag(void) {

}

void recBin(void) {
    FILE *fp = fopen("output.dat", "rb");

    //Pular campo de status
    fseek(fp, 1, SEEK_SET);

    int top;
    //Ler o topo da pilha
    fread(&top, sizeof(int), 1, fp);

    if(top == -1) {
        fprintf(stdout, "Pilha vazia.\n");
    } else {
        fprintf(stderr, "Ainda não implementamos essa funcionalidade ¯\\_(ツ)_/¯\n");
    }

    fclose(fp);
}
