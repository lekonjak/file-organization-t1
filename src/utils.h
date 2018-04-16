//Mesma coisa do outro
#pragma once

#include <stdlib.h>
#include <stdio.h>

enum {
	false, true,
};

/* Reads a line from stdin until (\n|EOF)
 */
char *readline(void);

/* Reads a line from stdin until (\n|EOF)
 */
char *freadline(FILE *fp);

char **split(char *string);
