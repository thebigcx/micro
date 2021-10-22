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

#define SIGHUP    1
#define SIGINT    2
#define SIGQUIT   3
#define SIGILL    4
#define SIGTRAP   5
#define SIGABRT   6
#define SIGIOT    SIGABRT
#define SIGBUS    7
#define SIGFPE    8
#define SIGKILL   9
#define SIGUSR1   10
#define SIGSEGV   11
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGSTKFLT 16
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22
#define SIGURG    23
#define SIGXCPU   24
#define SIGXFSZ   25
#define SIGVTALRM 26
#define SIGPROF   27
#define SIGWINCH  28
#define SIGIO     29
#define SIGPOLL   29
#define SIGPWR    30
#define SIGSYS    31

// sigprocmask() values
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
