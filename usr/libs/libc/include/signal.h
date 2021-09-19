#pragma once

#include <sys/types.h>
#include <stdint.h>
#include <micro/signal.h>

typedef int sig_atomic_t;

typedef void (*sighandler_t)(int);

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)

struct sigaction
{
	sighandler_t   sa_handler;
	void         (*sa_sigaction)(int);
	uint64_t       sa_mask;
	int            sa_flags;
	void         (*sa_restorer)();
};

int kill(pid_t pid, int sig);
int raise(int sig);
int sigaction(int signum, const struct sigaction* act, struct sigaction* old);
sighandler_t signal(int signo, sighandler_t handler);