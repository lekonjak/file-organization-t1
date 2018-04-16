#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Reads a line from stdin until (\n | EOF)
 * Returns NULL if no byteswere read
 */
char *readline(void) {
	char *string = NULL;
	char aux;
	size_t nBytes = 0;

	do {
		aux = fgetc(stdin);
		string = realloc(string, ++nBytes * sizeof(char));
		string[nBytes - 1] = aux;
	} while(aux != '\n' && aux != EOF);

	if(!nBytes)
		return NULL;

	// Always put '\0' at the end of a string
	string[nBytes] = 0;

	return string;
}

/* Reads a line from a file until (\n | EOF)
 * Returns NULL if no bytes were read
 */
char *freadline(FILE *fp) {
	char *string = NULL;
	char aux;
	size_t nBytes = 0;

	do {
		aux = fgetc(fp);
		string = realloc(string, ++nBytes * sizeof(char));
		string[nBytes - 1] = aux;
	} while(aux != '\n' && aux != EOF);

	if(!nBytes || feof(fp)) {
		free(string);
		return NULL;
	}

	string[nBytes-1] = 0;

	return string;
}

/* We already have a function to read an entire line from a file
 * I don't think we need to make another to read each field from the csv file
 * Just take the line previously read and split it at ';'
 * 
 * strtok doesn't work if we have ';;'. It just skips this "token" and messes up
 * everything else
 */
char **split(char *string) {
	char **tokens = NULL;
	char *token = NULL;
	
	/*
	 * prestadora	dataAtiv	codINEP		nomeEscola				municipio	uf
	 * CTBC;		18/09/2009;	31031917;	EM PERCILIA LEONARDO;	ARAUJOS;	MG
	 */
	int i = 0;
	while((token = strsep(&string, ";")) != NULL) {
		switch(i) {
			case 0:
				printf("prestadora ");
				break;
			case 1:
				printf("dataAtiv ");
				break;
			case 2:
				printf("codINEP ");
				break;
			case 3:
				printf("nomeEscola ");
				break;
			case 4:
				printf("municipio ");
				break;
			case 5:
				printf("uf ");
				break;
		}
		
		printf(">%s<\n", token);

		i++;
	}
	
	return tokens;
}
