#include <signal.h>
#include <unistd.h>
#include <stddef.h>
#include <libc/sysdeps-internal.h>

int kill(pid_t pid, int sig)
{
	return sys_kill(pid, sig);
}

int raise(int sig)
{
    return kill((int)getpid(), sig);
}

sighandler_t signal(int signo, sighandler_t handler)
{
	// TODO
	return NULL;
}

/*int sigaction(int signum, const struct sigaction* act, struct sigaction* old)
{
	return sys_sigaction(signum, act, old);
}*/
