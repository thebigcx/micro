.extern __libc_start_main

.global _start
_start:
    and $0xfffffffffffffff0, %rsp
    call __libc_start_main