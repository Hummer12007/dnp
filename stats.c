#include "stats.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>

#include <stdio.h>

struct attrs base_attrs = { 5, 5, 10, 5, 5, 1 };
#define so(X) offsetof(struct attrs, X)
static size_t attr_offsets[6] = {so(STR), so(DEF), so(CON), so(MAG), so(DEX), so(LCK)};
#undef so
static size_t increments[6] = {1, 1, 1, 1, 1, 1};
int attack_efficiency[SP_NONE][SP_NONE] = {{0, 5, -5, 10}, {5, 0, 10, -5}, {-5, 5, 0, 10}, {-10, 5, 10, 0}};
struct attrs attr_caps = {-1, -1, -1, -1, -1, 20};

#define gattr(x, attr) ((int8_t *)((int8_t *)(&(x)) + attr_offsets[attr]))

void enforce_caps(struct attrs *attrs) {
	int8_t *dst, *cap;
	for (int attr = 0; attr < 6; ++attr) {
		dst = gattr(*attrs, attr), cap = gattr(attr_caps, attr);
		if (*dst < 0)
			dst = 0;
		if (*cap < 0)
			continue;
		if (*dst > *cap)
			*dst = *cap;
	}
}

uint8_t throw_dice(enum dice d, int count, struct pkmn *pokemon) {
	int outcome = 0;
	while (count--)
		outcome += rand() % d + 1;
	if ((rand() % 100) < pokemon->attrs.LCK) 
		outcome++;
	return outcome;
}

void level_up(struct pkmn *pokemon) {
	if (pokemon->lvl == UINT8_MAX)
		return;
	pokemon->lvl++;
	int points = throw_dice(D4, 1, pokemon);
	struct attrs dst;
	while (points--) {
		int attr = throw_dice(D6, 1, pokemon) % 6;
		dst = attrs();
		*gattr(dst, attr) = increments[attr];
		pokemon->attrs = add_attrs(pokemon->attrs, dst);
	}
}

static void process_attack(struct pkmn *, struct pkmn *, struct action *, struct pk_attack_result *);

static enum attack_outcome calc_attack_success(struct pkmn *first, struct pkmn *target, struct action *a) {
	enum attack_outcome o = SUC_INEF;
	switch (a->type) {
	case ACT_MELEE:
		{
		uint8_t d   = throw_dice(D20, 1, first);
		uint8_t d1 = d + first->attrs.STR + a->data.melee.str_mod;
		uint8_t d2 = AC_BASE + target->attrs.DEF;
		int8_t dc  = DC_BASE + a->data.dc_mod + first->attrs.STR - target->attrs.DEF;
		uint8_t saving_throw = dc < 10 ? 10 : (uint8_t) dc;
		if ((d1 > d2 && d1 - d2 > 5) || d == 20)
			o = SUC_CRIT;
		else if (d1 > d2)
			o = SUC_NORM;
		else if (d2 - d1 < 1)
			o = SUC_WEAK;
		else
			o = SUC_INEF;
		uint8_t dice = throw_dice(D20, 1, target);
		if (o != SUC_INEF)
			if (dice >= saving_throw || dice == 20)
				o =  SUC_SAVED;
		return o;
		}
	case ACT_SPELL:
		{
		if (a->data.spell.harming) {
			uint8_t d1 = throw_dice(D20, 1, first) + first->attrs.MAG;
			int8_t d2 = (a->data.spell.specialization == SP_NONE) ? 0 : attack_efficiency[a->data.spell.specialization][target->cls->spec];
			int8_t dc = DC_BASE + a->data.dc_mod + first->attrs.MAG - target->attrs.MAG;
			uint8_t saving_throw = dc < 10 ? 10 : (uint8_t) dc;
			if (d2 < 0 && (d2 & ~0x7F) > d1)
				return SUC_INEF;
			d1 += d2;
			if (d1 - target->attrs.MAG > 20)
				o = SUC_CRIT;
			else if (d1 - target->attrs.MAG > 5)
				o = SUC_NORM;
			else if (d1 > target->attrs.MAG)
				o = SUC_WEAK;
			else o = SUC_INEF;
			uint8_t dice = throw_dice(D20, 1, target);
			if (o != SUC_INEF)
				if (dice >= saving_throw || dice == 20)
					o = SUC_SAVED;
			return o;
		} else {
			enum attack_outcome o;
			uint8_t dice = throw_dice(D20, 1, first);
			if (dice > 18)
				o = SUC_CRIT;
			else if (dice > 10)
				o =  SUC_NORM;
			else if (dice > 5)
				o =  SUC_WEAK;
			else o = SUC_INEF;
			return o;
		}
		}
	case ACT_BUFF:
		{
		if (a->target == TARGET_OPP) {
			uint8_t d1 = throw_dice(D20, 1, first) + first->attrs.MAG;
			uint8_t d2 = throw_dice(D20, 1, target) + target->attrs.MAG + a->data.dc_mod;
			if (d1 > d2)
				o = SUC_NORM;
			else if (d2 - d1 < 2)
				o = SUC_WEAK;
			else
				o = SUC_INEF;
			int8_t dc = DC_BASE + a->data.dc_mod + first->attrs.MAG - target->attrs.MAG;
			uint8_t saving_throw = dc < 10 ? 10 : (uint8_t) dc;
			uint8_t dice = throw_dice(D20, 1, target);
			if (o != SUC_INEF)
				if (dice >= saving_throw || dice == 20)
					o = SUC_SAVED;
			return o;
		} else {
			uint8_t d1 = throw_dice(D20, 1, first) + first->attrs.MAG / 10 - a->data.dc_mod;
			if (d1 > 10)
				o = SUC_NORM;
			else if (d1 > 5)
				o = SUC_WEAK;
			else o = SUC_INEF;
			return o;
		}
		}
	}
	return o;
}

static void process_attack(struct pkmn *attacker, struct pkmn *target, struct action *a, struct pk_attack_result *o) {
	bool d_positive = false;
	o->action = a;
	o->dhp = 0;
	o->dattrs = attrs();
	o->outcome = calc_attack_success(attacker, target, a);
	o->target = target;
	switch (a->type) {
	case ACT_MELEE:
	case ACT_SPELL:
		{
		uint8_t damage_roll = a->type == ACT_MELEE ?
			throw_dice(a->data.melee.d_type, a->data.melee.d_count, attacker) :
			throw_dice(a->data.spell.d_type, a->data.spell.d_count, attacker);
		d_positive = a->type == ACT_SPELL && !a->data.spell.harming;
		switch (o->outcome) {
		case SUC_INEF:
		case SUC_SAVED:
			o->dhp = 0;
			break;
		case SUC_WEAK:
			o->dhp = damage_roll / 2;
			break;
		case SUC_NORM:
			o->dhp = damage_roll;
			break;
		case SUC_CRIT:
			o->dhp = damage_roll + damage_roll * (throw_dice(D2, 1, target) % 2) / 2;
			break;
		}
		break;
		}

	case ACT_BUFF:
		{
		switch (o->outcome) {
		case SUC_INEF:
		case SUC_SAVED:
			for (int attr = 0; attr < 6; ++attr) {
				*gattr(o->dattrs, attr) = 0;
			}
		case SUC_WEAK:
		case SUC_NORM:
		case SUC_CRIT:
			for (int attr = 0; attr < 6; ++attr) {
				int8_t *delta = gattr(a->data.buff.d_attrs, attr);
				int8_t *final = gattr(o->dattrs, attr);
				*final = *delta;
				if (o->outcome == SUC_WEAK)
					*final = (*final > 0) - (*final < 0);
			}
		break;
		default: { break; }
		}
		}
	default: { break; }
	}
	if (d_positive) {
		target->hp += o->dhp;
	} else {
		if (target->hp == 0) {
			o->dhp = 0;
			if (a->type != ACT_BUFF && o->outcome != SUC_SAVED)
				target->alive = false;
		} else if (o->dhp > target->hp) {
			o->dhp -= target->hp;
			target->hp = 0;
			target->alive = false;
		} else {
			target->hp -= o->dhp;
		}

	}
	target->attrs = add_attrs(target->attrs, o->dattrs);
	enforce_caps(&target->attrs);
	if (target->hp > target->attrs.CON)
		target->hp = target->attrs.CON;
}

struct attrs add_attrs(struct attrs lhs, struct attrs rhs) {
	struct attrs res;
	for (int attr = 0; attr < 6; ++attr) {
		int8_t *dest = gattr(res, attr), *orig = gattr(lhs, attr), *delt = gattr(rhs, attr);
		if (INT8_MAX - *orig < *delt) {
			*dest = INT8_MAX;
			continue;
		}
		if (*orig + *delt > 0)
			*dest = *orig + *delt;
		else
			*dest = 0;
	}
	enforce_caps(&res);
	return res;
}

struct attack_result attack(struct pkmn *first, struct pkmn *second, struct action *a1, struct action *a2) {
	struct attack_result res;
	res.p1.action = a1, res.p2.action = a2;
	uint8_t d1 = throw_dice(D20, 1, first), d2 = throw_dice(D20, 1, second);
	uint8_t _a = d1 + first->attrs.DEX, _b = d2 + second->attrs.DEX;
	res.order = !(_b > _a || (_b == _a && d2 > d1));
	struct pkmn *f, *s;
	struct action *a, *b;
	struct pk_attack_result *o1, *o2;
	if (res.order)
		f = first, s = second, a = a1, b = a2, o1 = &(res.p1), o2 = &(res.p2);
	else
		f = second, s = first, a = a2, b = a1, o1 = &(res.p2), o2 = &(res.p1);
	process_attack(f, s, a, o1);
	if (f->alive && s->alive) {
		process_attack(s, f, b, o2);
		res.order |= 1 << 1;
	}
	return res;
}

struct attrs attrs_from_vector(int attrs[6]) {
	struct attrs res;
	for (int attr = 0; attr < 6; ++attr) {
		*gattr(res, attr) = attrs[attr];
	}
	return res;
}

struct pk_class *make_class(char *name, enum specialization spec, struct attrs base_attrs) {
	struct pk_class *res = malloc(sizeof(struct pk_class));
	*res = (struct pk_class){.name = name, .spec = spec, .base_attributes = base_attrs};
	return res;
}

struct action *melee_action(char *name, uint8_t speed_penalty, uint8_t dc, uint8_t str, enum dice d_type, uint8_t d_count) {
	struct action *res = malloc(sizeof(struct action));
	struct action_data data = {dc, {str, d_type, d_count}, {0, 0, 0, 0}, {attrs()}};
	*res = (struct action){.name = name, .type = ACT_MELEE, .target = TARGET_OPP, .speed_penalty = speed_penalty, .data = data};
	return res;
}

struct action *spell_action(char *name, enum specialization spec, enum target target, uint8_t speed_penalty, uint8_t dc, enum dice d_type, uint8_t d_count) {
	struct action *res = malloc(sizeof(struct action));
	struct action_data data = {dc, {0, 0, 0}, {spec != SP_NONE, spec, d_type, d_count}, {attrs()}};
	*res = (struct action){.name = name, .type = ACT_SPELL, .target = target, .speed_penalty = speed_penalty, .data = data};
	return res;
}

struct action *buff_action(char *name, enum target target, uint8_t speed_penalty, uint8_t dc, struct attrs dattrs) {
	struct action *res = malloc(sizeof(struct action));
	struct action_data data = {dc, {0, 0, 0}, {0, 0, 0, 0}, {dattrs}};
	*res = (struct action){.name = name, .type = ACT_BUFF, .target = target, .speed_penalty = speed_penalty, .data = data};
	return res;
}

enum specialization spec_from_str(char *c) {
	if (!strcasecmp(c, "fire"))
		return SP_FIRE;
	else if (!strcasecmp(c, "water"))
		return SP_WATER;
	else if (!strcasecmp(c, "light"))
		return SP_LIGHT;
	else if (!strcasecmp(c, "dark"))
		return SP_DARK;
	else return SP_NONE;
}

enum dice dice_from_str(char *c) {
	static const enum dice ds[] = {D2, D4, D6, D8, D10, D12, D20, 0};
	int dice, i = 0;
	if (!c || (*c != 'd' && *c != 'D'))
		return 0;
	dice = atoi(++c);
	while (ds[i] != 0)
		if (ds[i++] == dice)
			return dice;
	return 0;
}

enum target target_from_str(char *c) {
	if (!strcasecmp(c, "self"))
		return TARGET_SELF;
	else if (!strcasecmp(c, "opponent"))
		return TARGET_OPP;
	return TARGET_OPP;
}
