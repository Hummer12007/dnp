#include <stdlib.h>
#include "csvreader.h"
#include "util.h"

int csv_readfile(FILE *fp, list_t *dst, void *(*cb)(int argc, char **argv)) {
	char *lineptr = NULL;
	size_t n = 0;
	int count = 0;
	void *data;
	if (!dst)
		return -1;
	while (dst->next)
		dst = dst->next;
	while (getline(&lineptr, &n, fp) != -1) {
		data = csv_readline(lineptr, cb);
		if (data) {
			dst->next = list_new_node(data);
			dst = dst->next;
			count++;
		}
		free(lineptr);
		lineptr = NULL, n = 0;
	}
	fclose(fp);
	return count;
}

void *csv_readline(char *line, void *(*cb)(int argc, char **argv)) {
	char **argv = NULL;
	int argc;
	void *data;
	if (!line || *line == '#')
		return NULL;
	argc = split_args(line, &argv, ',');
	data = cb(argc, argv);
	free_args(argc, argv);
	return data;
	
}
