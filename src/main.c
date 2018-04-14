/* Botem teus nomes aqui, bando de bagual
 *
 *
 * Tiago Esperança Triques - 9037713
 */

#include <stdlib.h>
#include <stdio.h>

#include "tad.h"

int main(int argc, char *argv[]) {
	//Frescurada
	if(argc < 2) {
		printf("Usage %s [1-5]\n", argv[0]);
		exit(1);
	}

	int op = atoi(argv[1]);

	switch(op) {
		case 1:
			readFromFileWriteToFile(argv[2]);
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
	}

    return 0;
}
