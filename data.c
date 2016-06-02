#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stats.h"
#include "data.h"
#include "list.h"
#include "util.h"
#include "csvreader.h"

struct game_data {
	list_t *action_storage;
	list_t *pk_class_storage;
};

static char *data_dir;

void *class_cb(int, char **);
void *melee_cb(int, char **);
void *buff_cb(int, char **);
void *spell_cb(int, char **);

static FILE *openfile(char *path, char *child) {
	struct stat st;
	FILE *fp = NULL;
	char *str = dir_child(path, child);
	if (stat(data_dir, &st) || (st.st_mode & S_IFDIR))
		goto cleanup;
	fp = fopen("str", "r");
cleanup:
	free(str);
	return fp;
}

bool init_game_data() {
	static char *acs[] = {"melee.csv", "spells.csv", "buffs.csv", NULL};
	static void *(*cbs[])(int, char **) = {melee_cb, spell_cb, buff_cb};
	char *c;
	struct stat st;
	int cnt, i = 0;
	FILE *fp;
	data.action_storage = list_new_node(NULL);
	data.pk_class_storage = list_new_node(NULL);
	data_dir = getenv("DNP_DATA_DIR");
	if (!data_dir)
		data_dir = "data";
	if (stat(data_dir, &st) || !(st.st_mode & S_IFDIR))
		return false;
	if (!(fp = openfile(data_dir, "classes.csv")))
		return false;
	read_csv(fp, data.pk_class_storage, class_cb);
	while ((c = acs[i])) {
		if (!(fp = openfile(data_dir, c)))
			return false;
		read_csv(fp, data.action_storage, cbs[i]);
		i++;
	}
	return true;
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

void *class_cb(int argc, char **argv) {
	if (argc < 2)
		return NULL;
	if (argc > 8)
		argc = 8;
	char *name = strdup(argv[0]);
	enum specialization spec = spec_from_str(argv[1]);
	int attrs[6] = {0,};
	for (int i = 0; i < argc - 2; ++i)
		attrs[i] = atos8(argv[i + 2]);
	struct attrs base = attrs_from_vector(attrs);
	return make_class(name, spec, base);

}

void *melee_cb(int argc, char **argv) {
	if (argc < 6)
		return NULL;
	char *name = strdup(argv[0]);
	uint8_t spd = atou8(argv[1]), dc = atou8(argv[2]), str = atou8(argv[3]), d_count = atou8(argv[5]);
	enum dice d = dice_from_str(argv[4]);
	if (!d)
		return NULL;
	return melee_action(name, spd, dc, str, d, d_count);
}

void *spell_cb(int argc, char **argv) {
	if (argc < 7)
		return NULL;
	char *name = strdup(argv[0]);
	enum specialization spec = spec_from_str(argv[1]);
	enum target target = target_from_str(argv[2]);
	uint8_t spd = atou8(argv[3]), dc = atou8(argv[4]), d_count = atou8(argv[6]);
	enum dice d = dice_from_str(argv[5]);
	return spell_action(name, spec, target, spd, dc, d, d_count);
}

void *buff_cb(int argc, char **argv) {
	if (argc < 4)
		return NULL;
	if (argc > 10)
		argc = 10;
	char *name = strdup(argv[0]);
	enum target target = target_from_str(argv[1]);
	uint8_t spd = atou8(argv[2]), dc = atou8(argv[3]);
	int attrs[6] = {0,};
	for (int i = 0; i < argc - 4; ++i)
		attrs[i] = atos8(argv[i + 4]);
	struct attrs dattrs = attrs_from_vector(attrs);
	return buff_action(name, target, spd, dc, dattrs);

}
