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
			csv2bin(argv[2]);
			break;
		case 2:
			bin2out();
            break;
		case 3:
            bin2outGrep(argv[2], argv[3], selectCmp(argv[2][0]) );
			break;
		case 4:
            bin2outRRN(atoi(argv[2]));
			break;
		case 5:
            bin2trashRRN(atoi(argv[2]));
			break;
		case 6:
            add2bin(argv);
			break;
		case 7:
            updateBin(argv);
			break;
		case 8:
            binDefrag();
			break;
		case 9:
            recBin();
			break;
	}

    return 0;
}
