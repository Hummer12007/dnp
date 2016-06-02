#include <stdlib.h>
#include "csvreader.h"
#include "util.h"

int read_csv(FILE *fp, list_t *dst, void *(*cb)(int argc, char **argv)) {
	char *lineptr = NULL, **argv = NULL;
	size_t n = 0;
	int count = 0, argc;
	void *data;
	if (!dst)
		return -1;
	while (dst->next)
		dst = dst->next;
	while (getline(&lineptr, &n, fp) != -1) {
		split_args(lineptr, &argv, ',');
		data = cb(argc, argv);
		if (data) {
			dst->next = list_new_node(data);
			dst = dst->next;
			count++;
		}
		free_args(argc, argv);
		free(lineptr);
		lineptr = NULL, n = 0;
	}
	fclose(fp);
	return count;
}
