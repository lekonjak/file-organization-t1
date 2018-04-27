/* Botem teus nomes aqui, bando de bagual
 *
 * Ricardo A Araujo - 9364890
 * Tiago Esperança Triques - 9037713
 * Fábio Augusto Romanini Pinto - 9791312
 */

#include "tad.h"

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stdout, "usage: %s OPTION\n", argv[0]);
        exit(0);
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
            bin2outGrep(argv[2], maybeConvert(argv[3], argv[2][0]), selectCmp(argv[2][0]));
            break;
        case 4:
            bin2outRRN(atoi(argv[2]));
            break;
        case 5:
            bin2trashRRN(atoi(argv[2]));
            break;
        case 6:
            // documentação interna!
            if (argc < 7) {
                printf("usage: %s 6 codINEP dataAtiv UF nomeEscola municipio prestadora\n", argv[0]);
                break;
            }
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
