#include "stats.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

void print_attrs(struct attrs);

struct pkmn *gen_pokemon(struct pk_class *cls, uint8_t lvl) {
	struct pkmn *pk = malloc(sizeof(struct pkmn));
	pk->cls = cls;
	pk->alive = 1;
	pk->attrs = add_attrs(base_attrs, cls->base_attributes);
	pk->lvl = 0;
	while (lvl--)
		level_up(pk);
	pk->hp = pk->attrs.CON;
	return pk;
}

struct pk_class test = {"default", SP_FIRE, attrs(.LCK = 3)};
struct action melee = {"melee", ACT_MELEE, TARGET_OPP, 3, {5, {3, D2, 3}, {0, 0, 0, 0}, attrs()}};
struct action magic = {"magic1", ACT_SPELL, TARGET_OPP, 5, {5, {0, 0, 0}, {true, SP_FIRE, D4, 2}, attrs()}};
struct action heal = {"heal", ACT_SPELL, TARGET_SELF, 2, {5, {0, 0, 0}, {false, SP_FIRE, D4, 2}, attrs()}};
struct action unstrong = {"unstrong", ACT_BUFF, TARGET_OPP, 3, {5, {0, 0, 0}, {0, 0, 0, 0}, attrs(.STR = -5)}};


void game(struct pkmn *p1, struct pkmn *p2) {
	struct attack_result res = attack(p1, p2, &melee, &melee);
}

void print_attrs(struct attrs a) {
	printf("%d %d %d %d %d %d\n", a.STR, a.DEF, a.CON, a.MAG, a.DEX, a.LCK);
}

void print_class(struct pk_class *cls) {
	puts(cls->name);
	printf("Base attributes: ");
	print_attrs(cls->base_attributes);
	printf("%d\n", cls->spec);
}

void print_pokemon(struct pkmn *p) {
	puts("Class:");
	puts("Attributes:");
	print_attrs(p->attrs);
	printf("Level %u, %u HP, alive: %d\n", p->lvl, p->hp, p->alive);
}

char *gs(uint8_t a) {
	switch (a) {
		case SUC_SAVED: return "SAVED";
		case SUC_INEF: return "INEF";
		case SUC_WEAK: return "WEAK";
		case SUC_NORM: return "NORM";
		case SUC_CRIT: return "CRIT";
	}
}

void print_pk_res(struct pk_attack_result res) {
	printf("Action: %s, Outcome: %s, dhp: %d, attrs: ", res.action->name, gs(res.outcome), res.dhp);
	print_attrs(res.dattrs);
}

void print_results(struct attack_result res) {
	printf("First attacker: %s\n", res.order ? "Player 1" : "Player 2");
	puts("Player 1 attack:");
	print_pk_res(res.p1);
	puts("Player 2 attack:");
	print_pk_res(res.p2);
}

struct action *get_action() {
	static struct action *actions[] = {&melee, &magic, &heal, &unstrong};
	return actions[rand() % (sizeof(actions) / sizeof(void *))];
}

int main() {
	struct timeval tv;
	gettimeofday(&tv , NULL);
	srand(tv.tv_sec + tv.tv_usec);
	struct pkmn *pk1 = gen_pokemon(&test, 50);
	struct pkmn *pk2 = gen_pokemon(&test, 50);
	struct attack_result res;
	do {
		res = attack(pk1, pk2, get_action(), get_action());
		print_results(res);
		print_pokemon(pk1);
		print_pokemon(pk2);
	} while (pk1->alive && pk2->alive);
}
