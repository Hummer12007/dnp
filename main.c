#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "commands.h"
#include "termcolors.h"

int main() {
	char *buf;
	char prompt[255];
	int i = 0;
	sprintf(prompt, "%s%s>%s%s",
		format_str(C_BOLD | C_UL, C_FMT_ON),
		format_str(C_MAGENTA, C_LIGHT),
		format_str(C_UL, C_FMT_OFF),
		format_str(C_CYAN, C_LIGHT | C_BG));
	while ((buf = readline(prompt))) {
		tput(stdout, C_DEFAULT, C_BG);
		handle_cmd(buf);
		if (*buf)
			add_history(buf);
		free(buf);
	}
	tput(stdout, C_DEFAULT, C_BG);
}

