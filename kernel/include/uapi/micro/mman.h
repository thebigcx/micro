#pragma once

#define PROT_NONE 	        0x0000
#define PROT_READ 	        0x0001
#define PROT_WRITE 	        0x0002
#define PROT_EXEC  	        0x0004

#define MAP_SHARED          0x0001
#define MAP_PRIVATE	        0x0002

#define MAP_ANONYMOUS       0x0004
#define MAP_FIXED           0x0008

#define PROT_READ	0x1
#define PROT_WRITE	0x2
#define PROT_EXEC	0x4
#define PROT_NONE	0x0
#define PROT_GROWSDOWN	0x01000000
#define PROT_GROWSUP	0x02000000

#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#define MAP_FIXED	0x10		/* Interpret addr exactly.  */
#define MAP_ANONYMOUS 0x20		/* Don't use a file.  */