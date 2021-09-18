#pragma once

#include <sys/types.h>
#include <stdint.h>
#include <micro/signal.h>

typedef void (*sighandler_t)(int);

struct sigaction
{
	sighandler_t   sa_handler;
	void         (*sa_sigaction)(int);
	uint64_t       mask;
	int            sa_flags;
	void         (*sa_restorer)();
};

int kill(pid_t pid, int sig);
int raise(int sig);
int sigaction(int signum, const struct sigaction* act, struct sigaction* old);
sighandler_t signal(int signo, sighandler_t handler);