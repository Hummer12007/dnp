#ifndef TERMCOLORS_H
#define TERMCOLORS_H
#include <stdio.h>

enum term_colors { C_BLACK, C_RED, C_GREEN, C_YELLOW, C_BLUE, C_MAGENTA, C_CYAN, C_WHITE, C_DEFAULT = 9 };
enum term_formats { C_NONE = 1 << 0, C_BOLD = 1 << 1, C_UL = 1 << 4, C_BLINK = 1 << 5 };
enum term_color_mods { C_FG = 0, C_BG = 1, C_LIGHT = 1 << 1, C_FMT_ON = 1 << 2, C_FMT_OFF = 1 << 3};

void tput(FILE *f, int format, int mods);
int escape_num(int format, int mods);
char *format_str(int format, int mods);
#endif //TERMCOLORS_H
