#pragma once

#include <sys/types.h>
#include <stdint.h>

#define SIGABRT     0
#define SIGALRM     1
#define SIGBUS      2
#define SIGCHLD     3
#define SIGCONT     4
#define SIGFPE      5
#define SIGHUP      6
#define SIGILL      7
#define SIGINFO     8
#define SIGINT      9
#define SIGKILL     10
#define SIGPIPE     11
#define SIGPOLL     12
#define SIGPROF     13
#define SIGQUIT     14
#define SIGSEGV     15
#define SIGSTKFLT   16
#define SIGSTOP     17
#define SIGTSTP     18
#define SIGSYS      19
#define SIGTERM     20
#define SIGTRAP     21
#define SIGTTIN     22
#define SIGTTOU     23
#define SIGUNUSED   24
#define SIGURG      25
#define SIGUSR1     26
#define SIGUSR2     27
#define SIGVTALRM   28
#define SIGXCPU     29
#define SIGXFSZ     30
#define SIGWINCH    31

typedef void (*sighandler_t)(int);

struct sigaction
{
	sighandler_t sa_handler;
	void (*sa_sigaction)(int);
	uint64_t mask;
	int sa_flags;
	void (*sa_restorer)();
};

int kill(pid_t pid, int sig);
int raise(int sig);
int sigaction(int signum, const struct sigaction* act, struct sigaction* old);
sighandler_t signal(int signo, sighandler_t handler);