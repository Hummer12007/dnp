#ifndef CSV_READER_H
#define CSV_READER_H
#include <stdio.h>
#include "list.h"

int read_csv(FILE *fp, list_t *dst, void *(*cb)(int argc, char **argv));
#endif //CSV_READER_H
