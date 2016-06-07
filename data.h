#ifndef DATA_H
#define DATA_H
#include "stats.h"

extern struct game_data data;

bool init_game_data();
struct action *get_action(char *name);
struct pk_class *get_class(char *name);
void add_action(struct action *);
void add_class(struct pk_class *);
list_t *get_actions();
list_t *get_classes();
int cls_cmp(void *, void *);
int act_cmp(void *, void *);
#endif //DATA_H
