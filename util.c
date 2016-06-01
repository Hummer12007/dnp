#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

int split_args(char *cmd, char ***argv) {
	size_t argv_size = 2;
	size_t count = 0;
	char **args = malloc(sizeof(char *) * argv_size);
	char **tmp;
	char *c1 = cmd;
	char *c2 = NULL;
	int len;
	while (c1) {
		while(isspace(*c1))
			c1++;
		if (*c1 == '"') {
			c1++;
			c2 = strchr(c1, '"');
		}
		else c2 = strchr(c1, ' ');
		len = c2 ? (c2 - c1) : strlen(c1);
		if (!len)
			break;
		if (count == argv_size) {
			if ((tmp = realloc(args, sizeof(char *) * (argv_size *= 2)))) {
				args = tmp;
				tmp = NULL;
			} else {
				free(args);
				return -1;
			}
		}
		args[count] = calloc(len + 1, sizeof(char));
		strncpy(args[count], c1, len);
		if (c2 && *c2 == '"')
			c2++;
		c1 = c2;
		count++;
	}
	*argv = args;
	return count;
}

void free_args(int argc, char **argv) {
	int i;
	for (i = 0; i < argc; ++i)
		free(argv[i]);
}

const void *lsearch (const void *key, const void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *)) {
	const char *curr, *end = base + size * nmemb;
	for (curr = base; curr <= end; curr += size) {
		if (!cmp((void *) curr, key))
			return curr;
	}
	return NULL;
}
