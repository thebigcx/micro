set(SOURCES ${SOURCES} ${KERNSRC}/init.c
                       ${KERNSRC}/stdlib.c
                       ${KERNSRC}/acpi.c
                       ${KERNSRC}/debug/ubsan.c
                       ${KERNSRC}/debug/serial.c
                       ${KERNSRC}/heap.c
                       ${KERNSRC}/list.c
                       ${KERNSRC}/sched.c
                       ${KERNSRC}/task.c
                       ${KERNSRC}/thread.c
                       ${KERNSRC}/sys.c
                       ${KERNSRC}/tree.c
                       ${KERNSRC}/vfs/vfs.c
                       ${KERNSRC}/binfmt.c
                       PARENT_SCOPE)
