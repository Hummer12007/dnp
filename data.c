#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stats.h"
#include "data.h"
#include "list.h"
#include "util.h"
#include "csvreader.h"
#include "serial.h"

struct game_data {
	list_t *action_storage;
	list_t *pk_class_storage;
} data;

static char *data_dir;

static FILE *openfile(char *path, char *child) {
	struct stat st;
	FILE *fp = NULL;
	char *str = dir_child(path, child);
	if (stat(str, &st) || (st.st_mode & S_IFDIR))
		goto cleanup;
	fp = fopen(str, "r");
cleanup:
	free(str);
	return fp;
}

bool init_game_data() {
	static bool inited = false;
	if (inited)
		return true;
	static char *acs[] = {"melee.csv", "spells.csv", "buffs.csv", NULL};
	static void *(*cbs[])(int, char **) = {des_melee, des_spell, des_buff};
	char *c;
	struct stat st;
	int i = 0;
	FILE *fp;
	list_t *l = NULL;
	data.action_storage = list_new_node(NULL);
	data.pk_class_storage = list_new_node(NULL);
	data_dir = getenv("DNP_DATA_DIR");
	if (!data_dir)
		data_dir = "data";
	if (stat(data_dir, &st) || !(st.st_mode & S_IFDIR))
		return false;
	if (!(fp = openfile(data_dir, "classes.csv")))
		return false;
	csv_readfile(fp, data.pk_class_storage, des_class);
	while ((c = acs[i])) {
		if (!(fp = openfile(data_dir, c)))
			return false;
		csv_readfile(fp, data.action_storage, cbs[i]);
		i++;
	}
	if (data.action_storage->next) {
		l = data.action_storage;
		data.action_storage = l->next;
		free(l);
	}
	if (data.pk_class_storage->next) {
		l = data.pk_class_storage;
		data.pk_class_storage = l->next;
		free(l);
	}
	inited = true;
	return true;
}

list_t *get_actions() { return data.action_storage; }
list_t *get_classes() { return data.pk_class_storage; }

int act_cmp(void *key, void *data) {
	char *keyname = (char *)key, *dataname = ((struct action *)data)->name;
	return strcasecmp(keyname, dataname);
}
struct action *get_action(char *name) {
	return list_search(name, data.action_storage, act_cmp);
}

int cls_cmp(void *key, void *data) {
	char *keyname = (char *)key, *dataname = ((struct pk_class *)data)->name;
	return strcasecmp(keyname, dataname);
}
struct pk_class *get_class(char *name) {
	return list_search(name, data.pk_class_storage, cls_cmp);
}
