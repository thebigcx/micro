#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/heap.h>
#include <micro/sched.h>
#include <micro/stdlib.h>

SYSCALL_DEFINE(execve, const char* path, const char* argv[], const char* envp[])
{
    PTRVALID(path);
    PTRVALID(argv);
    PTRVALID(envp);

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    struct file file;
    int e = vfs_resolve(canon, &file, 1);
    
    kfree(canon);

    if (e) return e;
    if (file.type != S_IFREG) return -EACCES;

    CHECK_XPERM(&file);

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

    // TODO: return error code from task_execve
    if ((e = task_execve(task_curr(), canon,
                         (const char**)arg_copy,
                         (const char**)env_copy)))
        return e;
        
    sched_yield();
    return -1;
}