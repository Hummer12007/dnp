#ifndef SERIAL_H
#define SERIAL_H
#include "stats.h"

char *ser_class(struct pk_class *);
char *ser_melee(struct action *);
char *ser_buff(struct action *);
char *ser_melee(struct action *);
char *ser_pokemon(struct pkmn *);

void *des_class(int, char **);
void *des_melee(int, char **);
void *des_buff(int, char **);
void *des_spell(int, char **);
void *des_pokemon(int, char **);
#endif //SERIAL_H
