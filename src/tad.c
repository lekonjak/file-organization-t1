#include "tad.h"

struct registro {
	// Campos de tamanho fixo
	int codINEP;
	char dataAtiv[10];
	char uf[2];

	//Campos de tamanho variável
	char *nomeEscola;
	char *municipio;
	char *prestadora;
};
