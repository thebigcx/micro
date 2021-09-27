#pragma once

#include <micro/types.h>

typedef struct
{

} siginfo_t;

typedef void (*sighandler_t)(int);

struct sigaction
{
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t*, void*);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
};

#define SIG_DFL     ((sighandler_t)0)
#define SIG_IGN     ((sighandler_t)1)

#define SIGINVAL    0
#define SIGALRM     1
#define SIGBUS      2
#define SIGCHLD     3
#define SIGCONT     4
#define SIGFPE      5
#define SIGHUP      6
#define SIGILL      7
#define SIGINT      8
#define SIGKILL     9
#define SIGPIPE     10
#define SIGPOLL     11
#define SIGPROF     12
#define SIGQUIT     13
#define SIGSEGV     14
#define SIGSTOP     15
#define SIGTSTP     16
#define SIGSYS      17
#define SIGTERM     18
#define SIGTRAP     19
#define SIGTTIN     20
#define SIGTTOU     21
#define SIGURG      22
#define SIGUSR1     23
#define SIGUSR2     24
#define SIGVTALRM   25
#define SIGXCPU     26
#define SIGXFSZ     27
#define SIGWINCH    28
#define SIGABRT     29