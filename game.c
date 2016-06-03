#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "stats.h"
#include "list.h"
#include "data.h"

static int eq_spec(struct pkmn *pk, struct action *a) {
	return a->type == ACT_SPELL && a->data.spell.specialization == pk->cls->spec;
}

static int is_melee(struct action *a) { return a->type == ACT_MELEE; }

static int misc_actions(struct pkmn *pk, struct action *a) {
	return !(is_melee(a) || eq_spec(pk, a));
}

struct pkmn *gen_pokemon(struct pk_class *cls, uint8_t lvl) {
	struct pkmn *pk = malloc(sizeof(struct pkmn));
	size_t skills;
	pk->cls = cls;
	list_t *m = list_filter(get_actions(), is_melee);
	list_t *sp = list_filter_by_key(pk, get_actions(), eq_spec);
	list_t *ot = list_filter_by_key(pk, get_actions(), misc_actions);
	list_t *mf, *spf, *otf;
	pk->alive = 1;
	pk->attrs = add_attrs(base_attrs, cls->base_attributes);
	pk->lvl = 0;
	while (lvl--)
		level_up(pk);
	pk->hp = pk->attrs.CON;
	skills = throw_dice(D2, 1, pk) + throw_dice(D2, 1, pk);
	mf = list_random_sample(m, 1);
	spf = list_random_sample(sp, 2);
	otf = list_random_sample(ot, skills);
	list_cat(spf, otf), list_cat(mf, spf);
	list_free(m), list_free(sp), list_free(ot);
	pk->skills = mf;
	return pk;
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


void print_skill(void *skill) {
	printf("%s\n", ((struct action *) skill)->name);
}

void print_pokemon(struct pkmn *p) {
	puts("Class:");
//	print_class(p->cls);
	puts("Attributes:");
	print_attrs(p->attrs);
	printf("Level %u, %u HP, alive: %d\n", p->lvl, p->hp, p->alive);
	puts("Skills: ");
//	list_foreach(p->skills, print_skill);
}

char *gs(uint8_t a) {
	switch (a) {
		case SUC_SAVED: return "SAVED";
		case SUC_INEF: return "INEF";
		case SUC_WEAK: return "WEAK";
		case SUC_NORM: return "NORM";
		case SUC_CRIT: return "CRIT";
		default: puts("This should not happen!");
		assert(false);
		return "WTF?";
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

struct action *gaction(struct pkmn *pk) {
	list_t *l = list_random_sample(pk->skills, 1);
	struct action *a = l->data;
	free(l);
	return a;
}

int main() {
	struct timeval tv;
	gettimeofday(&tv , NULL);
	srand(tv.tv_sec + tv.tv_usec);
	init_game_data();
	list_t *l = list_random_sample(get_classes(), 2);
	struct pk_class *c1, *c2;
	c1 = l->data, l = l->next, c2 = l->data;
	list_free(l);
	struct pkmn *pk1 = gen_pokemon(c1, 50);
	struct pkmn *pk2 = gen_pokemon(c2, 50);
	print_pokemon(pk1);
	print_pokemon(pk2);

 	struct attack_result res;
	do {
		res = attack(pk1, pk2, gaction(pk1), gaction(pk2));
		print_results(res);
		print_pokemon(pk1);
		print_pokemon(pk2);
	} while (pk1->alive && pk2->alive);
}
