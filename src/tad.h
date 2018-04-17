//Mesma coisa dos ifguards, só que mais nice
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Não sei qualé o padrão que vocês costumam usar pra typedef de structs
typedef struct registro Registro;

typedef struct header Header;

void csv2bin(char *filename);
void bin2out(void);
    void catReg(Registro *reg, int sizeEscola, int sizeMunicipio, int sizePrestadora);
    int fpeek(FILE *fp);
void bin2outGrep(char *category, void *element, int (*cmp)(void*, void*));
    void *selectCmp(char cat);
    int intCmp(void *a, void *b);
    int sstrCmp(void *a, void *b);
    char *strClear(char *s);
void bin2outRRN(int RRN);
void bin2trashRRN(int RRN);
void add2bin(char *argv[]);
void updateBin(char *argv[]);
void binDefrag(void);
void recBin(void);
