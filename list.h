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
void list_foreach(list_t *base, void (*cb)(void *));
void list_free(list_t *list);
list_t *list_filter(void *key, list_t *base, int (*cmp)(void *, void *));
size_t list_length(list_t *list);
#endif //LIST_H
