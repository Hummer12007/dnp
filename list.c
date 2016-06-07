#include <stdlib.h>
#include <stdbool.h>

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

void list_cat(list_t *head, list_t *tail) {
	if (!head)
		return;
	while (head->next)
		head = head->next;
	head->next = tail;
}

void *list_search(void *key, list_t *base, int (*cmp)(void *, void *)) {
	while (base) {
		if (!cmp(key, base->data))
			return base->data;
		base = base->next;
	}
	return NULL;
}

list_t *list_remove(void *key, list_t *base, int(*pred)(void *, void *), bool free_data) {
	list_t *prev = NULL, *head = base, *p;
	while (base) {
		if (pred(key, base->data)) {
			p = base->next;
			if (prev)
				prev->next = p;
			else
				head = base;
			if (free_data)
				free(base->data);
			free(base);
			base = p;
		} else {
			prev = base;
			base = base->next;
		}
	}
	return head;
}

void list_foreach(list_t *base, void (*cb)(void *elem, void *data), void *data) {
	while (base) {
		cb(base->data, data);
		base = base->next;
	}
}

void list_free(list_t *list) {
	list_t *p;
	while (list) {
		p = list;
		list = list->next;
		free(p);
	}
}

list_t *list_filter(list_t *base, int (*pred)(void *)) {
	list_t *res = list_new_node(NULL);
	list_t *last = res, *p;
	while (base) {
		if (pred(base->data)) {
			last->next = list_new_node(base->data);
			last = last->next;
		}
		base = base->next;
	}
	p = res->next;
	free(res);
	return p;
}

list_t *list_filter_by_key(void *key, list_t *base, int (*pred)(void *, void *)) {
	list_t *res = list_new_node(NULL);
	list_t *last = res, *p;
	while (base) {
		if (pred(key, base->data)) {
			last->next = list_new_node(base->data);
			last = last->next;
		}
		base = base->next;
	}
	p = res->next;
	free(res);
	return p;
}

/* Vitter, Random sampling with a reservoir, 1985 */
list_t *list_random_sample(list_t *base, size_t count) {
	void **nodes = malloc(count * sizeof(void *));
	size_t i = 0, r;
	list_t *res, *final;
	for (; base; base = base->next) {
		if (i < count) {
			nodes[i] = base->data;
		} else {
			r = rand() % i;
			if (r < count)
				nodes[r] = base->data;
		}
		i++;
	}
	count = i > count ? count : i;
	if (!count)
		return NULL;
	final = res = list_new_node(nodes[0]);
	for (i = 1; i < count; ++i) {
		final->next = list_new_node(nodes[i]), final = final->next;
	}
	free(nodes);
	return res;
}

size_t list_length(list_t *list) {
	size_t i = 0;
	while (list)
		i++, list = list->next;
	return i;
}
