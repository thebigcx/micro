#pragma once

#include <sys/types.h>

#define WEXITSTATUS(stat) (((stat) & 0xff00) >> 8)
#define WTERMSIG(stat) ((stat) & 0x7f)
#define WSTOPSIG(stat) WEXITSTATUS(stat)
#define WIFEXITED(stat) (WTERMSIG(stat) == 0)

pid_t waitpid(pid_t pid, int* wstatus, int options);