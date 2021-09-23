#include <errno.h>
#include <string.h>

static const char* errno_strs[] =
{
    "Success (not an error)",

    [EBADF       ] = "Bad file descriptor",
    [EFAULT      ] = "Bad address",
    [ELOOP       ] = "Loop",
    [ENAMETOOLONG] = "Name too long",
    [ENOENT      ] = "No such file or directory",
    [ENOMEM      ] = "Out of memory",
    [ENOTDIR     ] = "Not a directory",
    [EOVERFLOW   ] = "Overflow",
    [ENOTTY      ] = "No tty",
    [ERANGE      ] = "Out of range",
    [ENOSYS      ] = "No such syscall",
    [ESRCH       ] = "Search error",
    [ECHILD      ] = "Invalid child",
    [EISDIR      ] = "Is directory",
    [EINVAL      ] = "Invalid argument",
    [EEXIST      ] = "Already exists",
    [EMFILE      ] = "Too many open files",
    [ENODEV      ] = "No such device",
    [EACCES      ] = "Operation not permitted",
    [EIO         ] = "I/O error",
    [EINTR       ] = "Interrupted syscall",
    [EPERM       ] = "Permission denied",
    [ENOEXEC     ] = "Invalid executable"
};

char* strerror(int errnum)
{
    errnum = -errnum; // Reverse sign

    if (errnum < 0 || errnum >= EMAXERRNO)
        return "unknown error code";

    return errno_strs[errnum];
}

char* strerror_r(int errnum, char* buf, size_t len)
{
    errnum = -errnum; // Reverse sign

    if (errnum < 0 || errnum >= EMAXERRNO)
    {
        errno = -EINVAL;
        return NULL;
    }

    const char* e = errno_strs[errnum];

    while (len-- && *e != 0) *buf++ = *e++;
    *buf = 0;

    return buf;
}