#ifndef DATA_H
#define DATA_H
#include "stats.h"

struct game_data data;

void init_game_data();
struct action *get_action(char *name);
void add_action(struct action *);
struct pk_class *add_class(struct pk_class *);
#endif //DATA_H
