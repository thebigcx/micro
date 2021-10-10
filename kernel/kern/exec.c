#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/heap.h>
#include <micro/sched.h>
#include <micro/stdlib.h>
#include <micro/try.h>
#include <micro/debug.h>

SYSCALL_DEFINE(execve, const char* path, char* const argv[], char* const envp[])
{
    printk("execve(%s) of pid=%d\n", path, task_curr()->pid);
    PTRVALID(path);
    PTRVALID(argv);
    PTRVALID(envp);

    // Copy the arguments and environment variables from userspace

    char* arg_copy[16];
    size_t argc = 0;
    while (argv[argc] != NULL)
    {
        PTRVALID(argv[argc]);

        arg_copy[argc] = kmalloc(strlen(argv[argc]) + 1);
        strcpy(arg_copy[argc], argv[argc]);
        argc++;
    }
    arg_copy[argc] = NULL;

    char* env_copy[32];
    size_t envc = 0;
    while (envp[envc] != NULL)
    {
        PTRVALID(envp[envc]);

        env_copy[envc] = kmalloc(strlen(envp[envc]) + 1);
        strcpy(env_copy[envc], envp[envc]);
        envc++;
    }
    env_copy[envc] = NULL;

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    TRY2(task_execve(task_curr(), canon, arg_copy,
                     env_copy), kfree(canon));
        
    sched_yield();
    __builtin_unreachable();
}
