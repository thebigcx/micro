#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/syscall.h>

int main(int argc, char** argv)
{
    mount("/dev/sda1", "/", "ext2", 0, NULL);

    int ps2 = open("/lib/modules/ps2.ko", O_RDONLY);
    syscall(SYS_finit_module, ps2, NULL, 0);

    pid_t pid = fork();
    if (pid == 0)
    {
        // Run the shell
        char* const argv[] = { "/usr/bin/vgaterm", NULL };
        char* const envp[] = { NULL };
        execve(argv[0], argv, envp);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }

    syscall(SYS_reboot);

    return 0;
}
