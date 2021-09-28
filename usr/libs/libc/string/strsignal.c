#include <string.h>
#include <signal.h>

static char* __libc_signal_strings[] =
{
    [SIGINVAL ] = "SIGINVAL",
    [SIGALRM  ] = "SIGALRM",
    [SIGBUS   ] = "SIGBUS",
    [SIGCHLD  ] = "SIGCHLD",
    [SIGCONT  ] = "SIGCONT",
    [SIGFPE   ] = "SIGFPE",
    [SIGHUP   ] = "SIGHUP",
    [SIGILL   ] = "SIGILL",
    [SIGINT   ] = "SIGINT",
    [SIGKILL  ] = "SIGKILL",
    [SIGPIPE  ] = "SIGPIPE",
    [SIGPOLL  ] = "SIGPOLL",
    [SIGPROF  ] = "SIGPROF",
    [SIGQUIT  ] = "SIGQUIT",
    [SIGSEGV  ] = "SIGSEGV",
    [SIGSTOP  ] = "SIGSTOP",
    [SIGTSTP  ] = "SIGTSTP",
    [SIGSYS   ] = "SIGSYS",
    [SIGTERM  ] = "SIGTERM",
    [SIGTRAP  ] = "SIGTRAP",
    [SIGTTIN  ] = "SIGTTIN",
    [SIGTTOU  ] = "SIGTTOU",
    [SIGURG   ] = "SIGURG",
    [SIGUSR1  ] = "SIGUSR1",
    [SIGUSR2  ] = "SIGUSR2",
    [SIGVTALRM] = "SIGVTALRM",
    [SIGXCPU  ] = "SIGXCPU",
    [SIGXFSZ  ] = "SIGXFSZ",
    [SIGWINCH ] = "SIGWINCH",
    [SIGABRT  ] = "SIGABRT",
};

char* strsignal(int sig)
{
    if (sig < 0 || sig >= sizeof(__libc_signal_strings) / sizeof(char*))
        return NULL;

    return __libc_signal_strings[sig];
}