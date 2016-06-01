#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "termcolors.h"

static char *_color_seq(int *, int);

void tput(FILE *f, int format, int mods) {
	char *c = format_str(format, mods);
	fputs(c, f);
	fflush(f);
	free(c);
}

int escape_num(int format, int mods) {
	if (mods & (C_FMT_ON | C_FMT_OFF)) {
		int increment = mods & C_FMT_OFF ? 20 : 0;
		if (format & C_NONE)
			return 0;
		if (format & C_BOLD)
			return 1 + increment;
		if (format & C_UL)
			return 4 + increment;
		if (format & C_BLINK)
			return 5 + increment;
		return -1;
	}
	return format + 30 + 10 * !!(mods & C_BG) + 60 * !!(mods & C_LIGHT);
}


char *format_str(int format, int mods) {
	int increment;
	int formats[16] = {0};
	int count = 0;
	if (mods & (C_FMT_ON | C_FMT_OFF)) {
		increment = mods & C_FMT_OFF ? 20 : 0;
		if (format & C_NONE)
			formats[count++] = 0;
		if (format & C_BOLD)
			formats[count++] = 1 + increment;
		if (format & C_UL)
			formats[count++] = 4 + increment;
		if (format & C_BLINK)
			formats[count++] = 5 + increment;
	} else {
		formats[count++] = escape_num(format, mods);
	}
	return _color_seq(formats, count);
}

char *_color_seq(int *formats, int count) {
	char *res = calloc(6 * count + 1, sizeof(char));
	char buf[16];
	int i;
	for (i = 0; i < count; ++i) {
		memset(buf, 0, 16);
		sprintf(buf, "\x1B[%dm", formats[i]);
		strcat(res, buf);
	}
	return res;
}
