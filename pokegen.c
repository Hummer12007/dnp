#include "stats.h"
#include "list.h"
#include "data.h"

#include <stdlib.h>

static int eq_spec(void *p, void *b) {
	struct pkmn *pk = p;
	struct action *a = b;
	return a->type == ACT_SPELL && a->data.spell.specialization == pk->cls->spec;
}

static int is_melee(void *a) { return ((struct action *)a)->type == ACT_MELEE; }

static int misc_actions(void *pk, void *a) {
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
	skills = throw_dice(D2, 2, pk);
	mf = list_random_sample(m, 1);
	spf = list_random_sample(sp, 2);
	otf = list_random_sample(ot, skills);
	list_cat(spf, otf), list_cat(mf, spf);
	list_free(m), list_free(sp), list_free(ot);
	pk->skills = mf;
	return pk;
}

