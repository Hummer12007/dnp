#include <stdlib.h>

#include "list.h"

list_t *list_new_node(void *data) {
	list_t *res = malloc(sizeof(list_t));
	res->next = NULL; res->data = data;
	return res;
}

void list_append(list_t *list, void *itemp) {
	if (!list)
		return;
	while (list->next)
		list = list->next;
	list->next = list_new_node(itemp);
}

void *list_search(void *key, list_t *base, int (*cmp)(void *, void *)) {
	while (base) {
		if (!cmp(key, base->data))
			return base->data;
		base = base->next;
	}
	return NULL;
}

void list_free(list_t *list) {
	list_t *p;
	while (list) {
		p = list;
		list = list->next;
		free(p);
	}
}

list_t *list_filter(void *key, list_t *base, int (*cmp)(void *, void *)) {
	list_t *res = list_new_node(NULL);
	list_t *last = res, *p;
	while (base) {
		if (!cmp(key, base->data)) {
			last->next = list_new_node(base->data);
			last = last->next;
		}
		base = base->next;
	}
	p = res->next;
	free(res);
	return p;
}
