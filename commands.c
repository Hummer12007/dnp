#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "commands.h"
#include "util.h"

struct cmd_handler {
	char *cmd;
	void (*handle)(int argc, char **argv);
};

void quit_handler(int, char **);
void exec_handler(int, char **);

struct cmd_handler handlers[] = {
	{"exec", exec_handler},
	{"quit", quit_handler},
};


static int handler_cmp(const void *h1, const void *h2) {
	return strcasecmp(
		((struct cmd_handler *) h1)->cmd,
		((struct cmd_handler *) h2)->cmd);
}

static struct cmd_handler *find_handler(char *cmd) {
	struct cmd_handler local = {.cmd = cmd};
	return bsearch(&local, handlers,
		sizeof(handlers) / sizeof(struct cmd_handler),
		sizeof(struct cmd_handler),
		handler_cmp);
}

void handle_cmd(char *cmd) {
	fprintf(stderr, "Handling command: %s\n", cmd);
	if (!*cmd) {
		fprintf(stderr, "Empty command!\n");
		return;
	}
	char **argv;
	int argc = split_args(cmd, &argv);
	struct cmd_handler *handler = find_handler(argv[0]);
	if (!handler) {
		fprintf(stderr, "Unknown command: %s\n", argv[0]);
		goto cleanup;
	}
		handler->handle(argc - 1, argv + 1);
	cleanup:
		free_args(argc, argv);
		free(argv);
}

void quit_handler(int argc, char **argv) {
	fprintf(stderr, "Quitting!\n");
	exit(0);
}

void exec_handler(int argc, char **argv) {
	pid_t pid;
	int status;
	int fd[2];
	if (pipe(fd) == -1) {
		perror("pipe(2) failed:");
		return;
	}
	switch ((pid = fork())) {
	case 0:
		close(fd[0]);
		dup2(fd[1], 1);
		dup2(fd[1], 2);
		close(fd[1]);
		char **args = calloc(argc + 1, sizeof(char *));
		memcpy(args, argv, argc * sizeof(char *));
		args[argc] = 0;
		if (execvp(*args, args) < 0)
			perror("execvp(3) failed");
		_exit(EXIT_FAILURE);
		break;
	case -1:
		perror("Could not fork");
		break;
	default:
		close(fd[1]);
		char buf[1024];
		ssize_t len;
		while ((len = read(fd[0], buf, sizeof(buf)))) {
			buf[len] = 0;
			fputs(buf, stdout);
			fflush(stdout);
		}
		waitpid(pid, NULL, 0);
		break;
	}
}
