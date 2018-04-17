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
		if(string != NULL)
			fields = split(string);

		//freadline returns null if it is EOF
		if(string != NULL)
			free(string);
	}

	//Fechar arquivos de entrada e saída	
	fclose(infile);
	fclose(outfile);

	return;
}

void bin2out(void);

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

void bin2outRRN(int RRN);
void bin2trashRRN(int RRN);
void add2bin(char *argv[]);
void updateBin(char *argv[]);
void binDefrag(void);
void recBin(void);
