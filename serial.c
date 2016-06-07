#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "serial.h"
#include "stats.h"
#include "data.h"

#define STR_LEN 1024

static struct pk_class *make_class(char *name, enum specialization, struct attrs base_attrs);
static struct action *melee_action(char *name, uint8_t speed_penalty, uint8_t dc, uint8_t str, enum dice d_type, uint8_t d_count);
static struct action *spell_action(char *name, enum specialization spec, enum target target, uint8_t speed_penalty, uint8_t dc, enum dice d_type, uint8_t d_count);
static struct action *buff_action(char *name, enum target target, uint8_t speed_penalty, uint8_t dc, struct attrs dattrs);

char *ser_attrs(struct attrs a) {
	char *t = calloc(STR_LEN, 1);
	sprintf(t, "%d,%d,%d,%d,%d,%d", a.STR, a.DEF, a.CON, a.MAG, a.DEX, a.LCK);
	return t;
}

char *ser_spec(enum specialization sp) {
	switch (sp) {
		case SP_FIRE: return "fire";
		case SP_WATER: return "water";
		case SP_LIGHT: return "light";
		case SP_DARK: return "dark";
		default: return "healing";
	}
}

char *ser_target(enum target target) {
	switch (target) {
		case TARGET_SELF: return "self";
		default: return "opponent";
	}
}

char *ser_dice(uint8_t d) {
	switch (d) {
		case D2: return "d2";
		case D4: return "d4";
		case D6: return "d6";
		case D8: return "d8";
		case D10: return "d10";
		case D12: return "d12";
default:	case D20: return "d20";
	}
}

char *ser_actt(enum action_type a) {
	switch (a) {
		case ACT_MELEE: return "melee";
		case ACT_SPELL: return "spell";
		case ACT_BUFF: return "buff";
		default: return "melee";
	}
}

char *ser_out(enum attack_outcome a) {
	switch (a) {
		case SUC_SAVED: return "saved";
		case SUC_INEF: return "ineffective";
		case SUC_WEAK: return "weak";
		case SUC_NORM: return "hit";
		case SUC_CRIT: return "critical hit";
		default: return "";
	}
}

static int atos8(char *c) {
	int a = atoi(c);
	if (a < INT8_MIN || a > INT8_MAX)
		return 0;
	return a;
}

static int atou8(char *c) {
	int a = atoi(c);
	if (a < 0 || a > UINT8_MAX)
		return 0;
	return a;
}

void *des_class(int argc, char **argv) {
	if (argc < 2)
		return NULL;
	if (argc > 8)
		argc = 8;
	char *name = strdup(argv[0]);
	enum specialization spec = des_spec(argv[1]);
	int attrs[6] = {0,};
	if (spec == SP_NONE)
		return NULL;
	for (int i = 0; i < argc - 2; ++i)
		attrs[i] = atos8(argv[i + 2]);
	struct attrs base = attrs_from_vector(attrs);
	return make_class(name, spec, base);

}

char *ser_class(struct pk_class *cls) {
	char *buf = calloc(STR_LEN, 1);
	char *a;
	sprintf(buf, "%s,%s,%s", cls->name, ser_spec(cls->spec), a = ser_attrs(cls->base_attributes));
	free(a);
	return buf;
}

void *des_melee(int argc, char **argv) {
	if (argc < 6)
		return NULL;
	char *name = strdup(argv[0]);
	uint8_t spd = atou8(argv[1]), dc = atou8(argv[2]), str = atou8(argv[3]), d_count = atou8(argv[5]);
	enum dice d = des_dice(argv[4]);
	if (!d)
		return NULL;
	return melee_action(name, spd, dc, str, d, d_count);
}

char *ser_melee(struct action *a) {
	char *buf = calloc(STR_LEN, 1);
	sprintf(buf, "%s,%u,%u,%u,%s,%u", a->name, a->speed_penalty, a->data.dc_mod,
		a->data.melee.str_mod, ser_dice(a->data.melee.d_type), a->data.melee.d_count);
	return buf;
}

void *des_spell(int argc, char **argv) {
	if (argc < 7)
		return NULL;
	char *name = strdup(argv[0]);
	enum specialization spec = des_spec(argv[1]);
	enum target target = des_target(argv[2]);
	uint8_t spd = atou8(argv[3]), dc = atou8(argv[4]), d_count = atou8(argv[6]);
	enum dice d = des_dice(argv[5]);
	return spell_action(name, spec, target, spd, dc, d, d_count);
}

char *ser_spell(struct action *a) {
	char *buf = calloc(STR_LEN, 1);
	sprintf(buf, "%s,%s,%s,%u,%u,%s,%u", a->name, ser_spec(a->data.spell.specialization),
		ser_target(a->target), a->speed_penalty, a->data.dc_mod,
		ser_dice(a->data.spell.d_type), a->data.spell.d_count);
	return buf;
}

void *des_buff(int argc, char **argv) {
	if (argc < 4)
		return NULL;
	if (argc > 10)
		argc = 10;
	char *name = strdup(argv[0]);
	enum target target = des_target(argv[1]);
	uint8_t spd = atou8(argv[2]), dc = atou8(argv[3]);
	int attrs[6] = {0,};
	for (int i = 0; i < argc - 4; ++i)
		attrs[i] = atos8(argv[i + 4]);
	struct attrs dattrs = attrs_from_vector(attrs);
	return buff_action(name, target, spd, dc, dattrs);

}

char *ser_buff(struct action *a) {
	char *buf = calloc(STR_LEN, 1);
	char *c;
	sprintf(buf, "%s,%s,%u,%u,%s", a->name, ser_target(a->target),
		a->speed_penalty, a->data.dc_mod, c = ser_attrs(a->data.buff.d_attrs));
	free(a);
	return buf;
}

char *ser_bool(bool a) {
	return a ? "true" : "false";
}

bool des_bool(char *c) {
	if (!strcasecmp(c, "true"))
		return true;
	else	return false;
}

void append_skill(void *skill, void *e) {
	char *c = (char *)e;
	struct action *a = (struct action *)skill;
	strcat(c, a->name);
	strcat(c, ",");
}

char *ser_skills(list_t *skills) {
	char *buf = calloc(STR_LEN, 1), *c;
	list_foreach(skills, append_skill, buf);
	c = strrchr(buf, ',');
	if (c)
		buf[c - buf] = '\0';
	return buf;
}

int name_in_list(void *key, void *data) {
	char **names = (char **)key;
	struct action *a = (struct action *)data;
	for (char *c = *names; c != NULL; c++)
		if (!strcasecmp(a->name, c))
			return true;
	return false;
}

int same_name(void *key, void *data) {
	char *name = (char *)key;
	struct pk_class *cls = (struct pk_class *)data;
	if (!strcasecmp(name, cls->name))
		return true;
	return false;
}

void *des_pokemon(int argc, char **argv) {
	if (argc < 11)
		return NULL;
	struct pk_class *cls = list_search(argv[9], get_classes(), same_name);
	uint8_t lvl = atou8(argv[1]), hp = atou8(argv[2]);
	bool alive = des_bool(argv[3]);
	int attrs[6];
	for (int i = 0; i < 6; ++i)
		attrs[i] = atos8(argv[i + 4]);
	struct attrs attrs_s = attrs_from_vector(attrs);
	char **skls = calloc(argc - 9, sizeof(char *));
	memcpy(skls, argv + 10, (argc - 10) * sizeof(char *));
	skls[argc - 10] = NULL;
	free(skls);
	list_t *skills = list_filter_by_key(skls, get_actions(), name_in_list);
	struct pkmn *pk = malloc(sizeof(struct pkmn));
	pk->cls = cls, pk->lvl = lvl, pk->hp = hp, pk->alive = alive, pk->attrs = attrs_s,
	pk->skills = skills;
	return pk;
}

char *ser_pokemon(struct pkmn *p) {
	char *buf = calloc(STR_LEN, 1);
	char *c;
	sprintf(buf, "%s,%u,%u,%s,%s,%s", p->cls->name, p->lvl, p->hp,
		ser_bool(p->alive), c = ser_attrs(p->attrs),
		ser_skills(p->skills));
	free(c);
	return buf;
}

enum specialization des_spec(char *c) {
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

enum target des_target(char *c) {
	if (!strcasecmp(c, "self"))
		return TARGET_SELF;
	else if (!strcasecmp(c, "opponent"))
		return TARGET_OPP;
	return TARGET_OPP;
}

enum dice des_dice(char *c) {
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
