#pragma once

// TODO: thread local storage
extern int errno;

#define EBADF           1
#define EFAULT          2
#define ELOOP           3
#define ENAMETOOLONG    4
#define ENOENT          5
#define ENOMEM          6
#define ENOTDIR         7
#define EOVERFLOW       8
#define ENOTTY          9
#define ERANGE          10
#define ENOSYS          11
#define ESRCH           12
#define ECHILD          13
#define EISDIR          14
#define EINVAL          15
#define EEXIST          16
#define EMFILE          17
#define ENODEV          18
#define EACCES          19
#define EIO             20
#define EINTR           21
#define EPERM           22
#define ENOEXEC         23
#define ELIBBAD         24
#define E2BIG           25
#define ESPIPE          26
#define EAGAIN          27
#define EDOM            28
#define EFBIG           29
#define ENOSPC          30
#define ENOTSUP         31
#define EAFNOSUPPORT    32
#define EXDEV           33

#define EMAXERRNO       34

#define EOPNOTSUPP      ENOTSUP