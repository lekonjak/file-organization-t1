//Mesma coisa do outro
#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "tad.h"

enum {
	false, true,
};

/* Reads a line from stdin until (\n|EOF)
 */
char *readline(void);

/* Reads a line from stdin until (\n|EOF)
 */
char *freadline(FILE *fp);

/* Split a string at ';' and returns the tokens
 */
char **split(char *string);
