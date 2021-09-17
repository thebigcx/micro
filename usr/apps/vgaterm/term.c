#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>

struct vga_char
{
    uint8_t c;
    uint8_t col;
};

enum VGA_COLS
{
    VGA_BLACK,
    VGA_BLUE,
	VGA_GREEN,
	VGA_CYAN,
	VGA_RED,
	VGA_MAGENTA,
	VGA_BROWN,
	VGA_LGRAY,
	VGA_DGRAY,
	VGA_LBLUE,
	VGA_LGREEN,
	VGA_LCYAN,
	VGA_LRED,
	VGA_PINK,
	VGA_YELLOW,
	VGA_WHITE
};

int main()
{
    int vga = open("/dev/vga0", O_RDWR, 0);

    struct vga_char c =
    {
        .c = 'X',
        .col = (VGA_BLACK << 4) | VGA_WHITE
    };
    write(vga, &c, 1);

    int pid = fork();
    if (pid == 0)
    {
        const char* argv[] = { "/usr/bin/sh", NULL };
        const char* envp[] = { NULL };
        execve(argv[0], argv, envp);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}