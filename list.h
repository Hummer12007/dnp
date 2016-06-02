#ifndef LIST_H
#define LIST_H
#include <stddef.h>

typedef struct {
	void *next;
	void *data;
} list_t;

list_t *list_new_node(void *data);
void list_append(list_t *list, void *item);
void *list_search(void *key, list_t *base, int (*cmp)(void *, void *));
void list_free(list_t *list);
list_t *list_filter(void *key, list_t *base, int (*cmp)(void *, void *));
#endif //LIST_H
