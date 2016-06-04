#ifndef STATS_H
#define STATS_H
#include <stdint.h>
#include <stdbool.h>
#include "list.h"

#define DC_BASE 10
#define AC_BASE 10
#define HP_BASE 10

struct attrs {
	int8_t STR; //strength
	int8_t DEF; //defence
	int8_t CON; //constitution
	int8_t MAG; //magic
	int8_t DEX; //speed
	int8_t LCK; //luck (0--10), the higher the level, the higher the probability of random+1 increase (up to 25%)
};

#define attrs(...) ((struct attrs){.STR = 0, .DEF = 0, .CON = 0, .MAG = 0, .DEX = 0, .LCK = 0, __VA_ARGS__ })

enum specialization { SP_FIRE, SP_WATER, SP_LIGHT, SP_DARK, SP_NONE };

enum action_type { ACT_MELEE, ACT_SPELL, ACT_BUFF };

enum attack_outcome { SUC_SAVED, SUC_INEF, SUC_WEAK, SUC_NORM, SUC_CRIT };

enum target { TARGET_SELF, TARGET_OPP };

struct action_data {
	uint8_t dc_mod; // resist roll
	struct melee_data {
		uint8_t str_mod;
		uint8_t d_type;
		uint8_t d_count;
	} melee;
	struct spell_data {
		bool harming;
		uint8_t specialization;
		uint8_t d_type;
		uint8_t d_count;
	} spell;
	struct buff_data {
		struct attrs d_attrs;
	} buff;
};

struct action {
	char *name;
	enum action_type type;
	enum target target;
	uint8_t speed_penalty;
	struct action_data data;
};

struct pk_class {
	char *name;
	enum specialization spec;
	struct attrs base_attributes;
};

struct pkmn {
	struct pk_class *cls;
	struct attrs attrs;
	bool alive;
	uint8_t lvl;
	uint8_t hp;
	list_t *skills;
};

struct pk_attack_result {
	uint8_t outcome;
	uint8_t dhp;
	struct attrs dattrs;
	struct action *action;
	struct pkmn *target;
};

struct attack_result {
	struct pk_attack_result p1;
	struct pk_attack_result p2;
	uint8_t order;
};

enum dice {D2 = 2, D4 = 4, D6 = 6, D8 = 8, D10 = 10, D12 = 12, D20 = 20};

extern struct attrs base_attrs;
extern int attack_efficiency[SP_NONE][SP_NONE];

void level_up(struct pkmn *);
uint8_t throw_dice(enum dice, int count, struct pkmn *);
struct attack_result attack(struct pkmn *, struct pkmn *, struct action *, struct action *);
struct attrs add_attrs(struct attrs, struct attrs);

enum specialization spec_from_str(char *c);
enum dice dice_from_str(char *c);
enum target target_from_str(char *c);
struct attrs attrs_from_vector(int attrs[6]);

struct pk_class *make_class(char *name, enum specialization, struct attrs base_attrs);
struct action *melee_action(char *name, uint8_t speed_penalty, uint8_t dc, uint8_t str, enum dice d_type, uint8_t d_count);
struct action *spell_action(char *name, enum specialization spec, enum target target, uint8_t speed_penalty, uint8_t dc, enum dice d_type, uint8_t d_count);
struct action *buff_action(char *name, enum target target, uint8_t speed_penalty, uint8_t dc, struct attrs dattrs);
#endif //STATS_H
