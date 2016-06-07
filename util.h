#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>

int split_args (char *cmd, char ***argv, char sep);
void free_args (int argc, char **argv);
const void *lsearch (const void *key, const void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *));
char *dir_child(char *, char *);
char *join_args(int argc, char **argv, char sep);
#endif //UTIL_H
