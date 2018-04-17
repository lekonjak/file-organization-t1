//Mesma coisa dos ifguards, só que mais nice
#pragma once

// Não sei qualé o padrão que vocês costumam usar pra typedef de structs
typedef struct registro Registro;

typedef struct header Header;

void csv2bin(void);
void bin2out(void);
void bin2outGrep(char *category, void *element);
void bin2outRRN(int RRN);
void bin2trashRRN(int RRN);
void add2bin(char *argv[]);
void updateBin(char *argv[]);
void binDefrag(void);
void recBin(void);
