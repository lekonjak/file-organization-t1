#include "tad.h"
#include "utils.h"

/* PDF diz: 'registro de 87 bytes'
 * Precisamos "alinhar" com o tamanho de página de disco
 * (veja https://0x0.st/6C - slide 36)
 *
 * Uma página tem 2^12 = 4096 bytes, então precisamos deixar cada registro com
 * 2^7 = 128 bytes. Teremos 2^(12 - 7) = 2^5 = 32 registros por página de disco
 *
 * Ou não, se alguém souber me avisa
 */
struct registro {
    // Campos de tamanho fixo
    int codINEP;		//4 bytes
    char dataAtiv[11];  //10 bytes
    char uf[3]; 		//2 bytes

    //Campos de tamanho variável
    char *nomeEscola;
    char *municipio;
    char *prestadora;
};

struct header {
    char status;
    int stackTop; //🔝
};

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
            regSize = strlen(fields[0]) + strlen(fields[3]) + strlen(fields[4]);
            regSize += 10+2+4+4+4;

            //TODO Melhorar essa djabo
            //TODO 2: Registro tem tamanho fixo, precisamos garantir que estamos
            //escrevendo 87 bytes toda vez.
            //TODO 3: Campos NULL:
            //  - tamanho fixo: preencher tudo com 0
            //  - tamanho variável: size = 0
            //Another gambs
            r.prestadora = calloc(87, sizeof(char));
            strncpy(r.prestadora, fields[0], strlen(fields[0]));

            //Fixo 10 bytes
            strncpy(r.dataAtiv, fields[1], 10);

            //Fixo 4 bytes
            r.codINEP = atoi(fields[2]);

            r.nomeEscola = calloc(strlen(fields[3])+1, sizeof(char));
            strncpy(r.nomeEscola, fields[3], strlen(fields[3]));

            r.municipio = calloc(strlen(fields[4])+1, sizeof(char));
            strncpy(r.municipio, fields[4], strlen(fields[4]));

            //Fixo 2 bytes
            strncpy(r.uf, fields[5], 2);

            int aux;
            //Write each entry
            fwrite(&r.codINEP, sizeof(int), 1, outfile);
            fwrite(r.dataAtiv, 10*sizeof(char), 1, outfile);
            fwrite(r.uf, 2*sizeof(char), 1, outfile);

            //Tamanho e nome da escola
            aux = strlen(r.nomeEscola);
            fwrite(&aux, sizeof(int), 1, outfile);
            fwrite(r.nomeEscola, aux*sizeof(char), 1, outfile);

            aux = strlen(r.municipio);
            fwrite(&aux, sizeof(int), 1, outfile);
            fwrite(r.municipio, aux*sizeof(char), 1, outfile);

            aux = 87 - regSize;
            fwrite(&aux, sizeof(int), 1, outfile);
            fwrite(r.prestadora, aux*sizeof(char), 1, outfile);

            free(r.prestadora);
            free(r.nomeEscola);
            free(r.municipio);

            for(int i = 0; i < 6; i++)
                free(fields[i]);
            free(fields);
            free(linha);
        }
    }

    //Fechar arquivos de entrada e saída
    fclose(infile);
    fclose(outfile);
}

int eof(FILE *fp) {
    int cur = ftell(fp);

    fseek(fp, 0, SEEK_END);
    int max = ftell(fp);

    fseek(fp, cur, SEEK_SET);

    return max;
}

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

void catReg(Registro *reg, int sizeEscola, int sizeMunicipio, int sizePrestadora) {
    fprintf(stdout, "%d %s %s %d %s %d %s %d %s\n", \
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
        r.nomeEscola = (char*) malloc (sizeof(char)*sizeEscola+1);
        fread(r.nomeEscola, sizeof(char)*sizeEscola, 1, fp);
        fread(&sizeMunicipio, sizeof(int), 1, fp);
        r.municipio = (char*) malloc (sizeof(char)*sizeMunicipio+1);
        fread(r.municipio, sizeof(char)*sizeMunicipio, 1, fp);
        fread(&sizePrestadora, sizeof(int), 1, fp);
        r.prestadora = (char*) malloc (sizeof(char)*sizePrestadora+1);
        fread(r.prestadora, sizeof(char)*sizePrestadora, 1, fp);

        sizePrestadora = strlen(r.prestadora);

        this = category[0] == 'c' ? &r.codINEP :\
                category[0] == 'd' ? &r.dataAtiv:\
                 category[0] == 'u' ? &r.uf:\
                  category[0] == 'n' ? &r.nomeEscola:\
                   category[0] == 'p' ? &r.prestadora:\
                    category[0] == 'm' ? &r.municipio: (void*)NULL; // precompilator gave me no choice, i had to cast do avoid warning

        if(this == NULL) {
            fclose(fp);
            free(r.nomeEscola);
            free(r.municipio);
            free(r.prestadora);
            printf("Falha no processamento do arquivo.\n");
            return;
        }

        if(!cmp(element, this)) {
            catReg(&r, sizeEscola, sizeMunicipio, sizePrestadora);
            flag++;
        }

        free(r.nomeEscola);
        free(r.municipio);
        free(r.prestadora);
    }

    if(!flag)
        printf("Registro inexistente.\n");

    fclose(fp);
}

void *selectCmp(char cat) {
    return cat == 'c' ? &intCmp : &sstrCmp;
}

int intCmp(void *a, void *b) {
    return *((int*)(a)) == *((int *)(b)) ? 0 : 1;
}

int sstrCmp(void *a, void *b) {
    return strcmp((char*)a, (char*)b);
}

char *strClear(char *s) {
    char *out = malloc ((strlen(s)-2)*sizeof(char));
    int i;

    for(i = 1; s[i] != '\''; i++) {
        out[i-1] = s[i];
    }
    out[i] = '\0';

    return out;
}

void bin2outRRN(int RRN) {
    /*
     *  we need to search by RRN not directly, but dinamically - next step is to write a search function
     * */
    FILE *fp;
    Registro r = {0};
    int sizeEscola, sizeMunicipio, sizePrestadora;
	long offset = 5;

    fp = fopen("output.dat", "rb");
    int max = eof(fp);
    fseek(fp, offset + (RRN*87), SEEK_SET);
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
        printf("Registro inexistente.\n");
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

}
