#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>

int split_args (char *cmd, char ***argv);
void free_args (int argc, char **argv);
const void *lsearch (const void *key, const void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *));
#endif //UTIL_H
