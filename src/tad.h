//Mesma coisa dos ifguards, só que mais nice
//#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct registro Registro;

typedef struct header Header;

typedef struct iheader Iheader;

typedef struct no No;

typedef struct BufferPool bufferPool;

int fpeek(FILE *fp);
int intCmp(void *a, void *b);
int sstrCmp(void *a, void *b);

void *maybeConvert(char *c, char d);
void *selectCmp(char cat);

void add2bin(char *argv[]);
void bin2out(void); // working here
void bin2outGrep(char *category, void *element, int (*cmp)(void *, void *));
void bin2outRRN(int RRN);
void bin2trashRRN(int RRN);
void binDefrag(void);
void catReg(Registro *reg, int sizeEscola, int sizeMunicipio, int sizePrestadora);
void csv2bin(char *filename);
void recBin(void);
void updateBin(char *argv[]);
