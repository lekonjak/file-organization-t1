#include "tad.h"
#include "utils.h"

/* PDF diz: 'registro de 87 bytes'
 * Precisamos "alinhar" com o tamanho de página de disco 
 * (veja https://0x0.st/6C - slide 36)
 *
 * Uma página tem 2^12 = 4096 bytes, então precisamos deixar cada registro com
 * 2^7 = 128 bytes. Teremos 2^(12 - 7) = 2^5 = 32 registros por página de disco
 *
 */
struct registro {
	// Campos de tamanho fixo
	int codINEP;		//4 bytes
	char dataAtiv[10];  //10 bytes
	char uf[2]; 		//2 bytes

	//Campos de tamanho variável
	char *nomeEscola;   //8 bytes
	char *municipio;	//8 bytes
	char *prestadora;	//8 bytes
};

struct header {
	char status;
	int stackTop; //🔝
};

/*
 * Especificação do csv:
 * prestadora	dataAtiv	codINEP		nomeEscola				municipio	uf
 * CTBC;		18/09/2009;	31031917;	EM PERCILIA LEONARDO;	ARAUJOS;	MG
 */
void csv2bin(char *filename) {
	fprintf(stderr, "Nome do arquivo para abrir: >%s<\n", filename);

	FILE *infile = NULL, *outfile = NULL;

	infile = fopen(filename, "r");

	if(infile == NULL) {
		fprintf(stdout, "Falha no carregamento do arquivo\n");
		return;
	}
	
	outfile = fopen("output.dat", "w");

	if(outfile == NULL) {
		fprintf(stdout, "Falha no carregamento do arquivo\n");
		return;
	}
		
	char *string = NULL;
	char **fields = NULL;

	while(!feof(infile)) {
		string = freadline(infile);

		//freadline returns null if it is EOF
		if(string != NULL) {
			fields = split(string);

            for(int i = 0; i < 6; i++)
                printf("fields[%d] = [%s]\n", i, fields[i]);
        
            for(int i = 0; i < 6; i++)
                free(fields[i]);
            free(fields);
			free(string);
        }
	}

	//Fechar arquivos de entrada e saída	
	fclose(infile);
	fclose(outfile);

	return;
}

void bin2out(void) {
    FILE *fp;
    Registro r;
    int sizeEscola ,sizeMunicipio ,sizePrestadora;
	long offset = 5;

    fp = fopen("output.dat", "r");
    fseek(fp, offset, SEEK_SET);

    while(fpeek(fp)){       // nao esquecer de resolver saporrinha
        fread(&r.codINEP, sizeof(int), 1, fp);
        fread(&r.dataAtiv, sizeof(char)*10, 1, fp);
        fread(&r.uf, sizeof(char)*2, 1, fp);
        fread(&sizeEscola, sizeof(int), 1, fp);
            r.nomeEscola = (char*) malloc ( sizeof(char)*sizeEscola+1);    
        fread(&r.nomeEscola, sizeof(char)*sizeEscola, 1, fp);
        fread(&sizeMunicipio, sizeof(int), 1, fp);
            r.municipio = (char*) malloc ( sizeof(char)*sizeMunicipio+1);    
        fread(&r.municipio, sizeof(char)*sizeMunicipio, 1, fp);
        fread(&sizePrestadora, sizeof(int), 1, fp);
            r.prestadora = (char*) malloc ( sizeof(char)*sizePrestadora+1);    
        fread(&r.prestadora, sizeof(char)*sizePrestadora, 1, fp);
        
        catReg(&r, sizeEscola, sizeMunicipio, sizePrestadora);

        free(r.nomeEscola);
        free(r.municipio);
        free(r.prestadora);
    }
    fclose(fp);
    return;
}
/*
	// Campos de tamanho fixo
	int codINEP;		//4 bytes
	char dataAtiv[10];  //10 bytes
	char uf[2]; 		//2 bytes

	//Campos de tamanho variável
	char *nomeEscola;   //8 bytes
	char *municipio;	//8 bytes
	char *prestadora;	//8 bytes
};

struct header {
	char status;
	int stackTop; //🔝
};

 * Especificação do csv:
 * prestadora	dataAtiv	codINEP		nomeEscola				municipio	uf
 * CTBC;		18/09/2009;	31031917;	EM PERCILIA LEONARDO;	ARAUJOS;	MG
*/
int fpeek(FILE *fp){
    char test = (char)fgetc(fp);
    ungetc( test, fp);
    if( test == EOF) return 0;
    return 1;
}

void catReg(Registro *reg, int sizeEscola, int sizeMunicipio, int sizePrestadora){
    fprintf(stdout,"%d %s %s %d %s %d %s %d %s\n",reg->codINEP, reg->dataAtiv, reg->uf, sizeEscola, reg->nomeEscola \
            , sizeMunicipio, reg->municipio, sizePrestadora, reg->prestadora);
}
void bin2outGrep(char *category, void *element, int (*cmp)(void *, void *)){
    
}

void *selectCmp(char cat){
    return cat == 'c' ? &intCmp : &sstrCmp;
}

int intCmp(void *a, void *b){
    return *((int*)(a)) == *((int *)(b)) ? 0 : 1;
}

int sstrCmp(void *a, void *b){
    return strcmp((char*)a, (char*)b);
}

void bin2outRRN(int RRN) {

}

void bin2trashRRN(int RRN) {

}

void add2bin(char *argv[]) {

}

void updateBin(char *argv[]) {

}

void binDefrag(void) {

}

void recBin(void) {

}
