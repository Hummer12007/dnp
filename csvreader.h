#ifndef CSV_READER_H
#define CSV_READER_H
#include <stdio.h>
#include "list.h"

int csv_readfile(FILE *fp, list_t *dst, void *(*cb)(int argc, char **argv));
void *csv_readline(char *line, void *(*cb)(int argc, char **argv));
#endif //CSV_READER_H
