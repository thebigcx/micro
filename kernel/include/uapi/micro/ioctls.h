#pragma once

#define TCSETS     2
#define TCSETSW    3
#define TCSETSF    4
#define TCGETS     5

#define TIOCGWINSZ	0x5413
#define TIOCSWINSZ	0x5414

#define _IOC_NONE	0U
#define _IOC_WRITE	1U
#define _IOC_READ	2U

#define _IOC_NRBITS	    8
#define _IOC_TYPEBITS	8
#define _IOC_SIZEBITS	14
#define _IOC_DIRBITS	2

#define _IOC_NRSHIFT	0
#define _IOC_TYPESHIFT	(_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT	(_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT	(_IOC_SIZESHIFT+_IOC_SIZEBITS)

#define _IOC(dir,type,nr,size)    \
	(((dir)  << _IOC_DIRSHIFT) |  \
	 ((type) << _IOC_TYPESHIFT) | \
	 ((nr)   << _IOC_NRSHIFT) |   \
	 ((size) << _IOC_SIZESHIFT))

#define _IOC_TYPECHECK(t) (sizeof(t))

#define _IOR(type,nr,size) _IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOW(type,nr,size) _IOC(_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))

#define TIOCGPTN   _IOR('T', 0x30, unsigned int)
#define TIOCSPTLCK _IOW('T', 0x31, int)