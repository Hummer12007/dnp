#include <stdlib.h>

#include "stats.h"
#include "data.h"
#include "list.h"

struct game_data {
	list_t *action_storage;
	list_t *pk_class_storage;
};

void init_game_data() {
	data.action_storage = list_new_node(NULL);
	data.pk_class_storage = list_new_node(NULL);
}
