#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

int split_args(char *cmd, char ***argv, char sep) {
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
		else c2 = strchr(c1, sep);
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
		if (c1 && *c1 == sep)
			c1++;
		count++;
	}
	*argv = args;
	char *c = &(args[count - 1][strlen(args[count-1]) - 1]);
	while (isspace(*c)) {
		*c = '\0';
		c--;
	}
	return count;
}

void free_args(int argc, char **argv) {
	int i;
	for (i = 0; i < argc; ++i)
		free(argv[i]);
}

char *dir_child(char *parent, char *child) {
	size_t len = strlen(parent) + strlen(child);
	char *res = malloc(len + 2);
	sprintf(res, "%s/%s", parent, child);
	return res;
}

char *join_args(int argc, char **argv, char sep) {
	int len = 0, i;
	for (i = 0; i < argc; ++i)
		len += strlen(argv[i]) + 1;
	char *res = malloc(len);
	len = 0;
	for (i = 0; i < argc; ++i) {
		strcpy(res + len, argv[i]);
		len += strlen(argv[i]);
		res[len++] = sep;
	}
	res[len - 1] = '\0';
	return res;
}
