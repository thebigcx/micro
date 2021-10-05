#include <signal.h>
#include <assert.h>
#include <string.h>

sighandler_t signal(int signo, sighandler_t handler)
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = handler;

	struct sigaction oact;
	int ret = sigaction(signo, &act, &oact);

	if (ret) return SIG_ERR;
	return oact.sa_handler;
}