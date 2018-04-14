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
void readFromFileWriteToFile(char *filename) {
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
		
	int nFields = 0;
	char *string = NULL;
	char **fields = NULL;

	while(!feof(infile)) {
		fprintf(stderr, "Entrei na porra do while(!feof(fp))\n");
		string = freadline(infile);
		fields = split(string, &nFields);

		fprintf(stderr, "A porra de field count is %d\n", nFields);
		for(int i = 0; i < nFields; i++) {
			fprintf(stderr, "field %d -> >%s<\n", i, fields[i]);
		}

		//Liberar memória alocada
		for(int i = 0; i < nFields; i++) {
			free(fields[i]);
		}
		free(fields);
		free(string);
	}

	//Fechar arquivos de entrada e saída	
	fclose(infile);
	fclose(outfile);

	return;
}