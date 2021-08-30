#include <sys.h>

void syscall_handler(struct regs* r)
{
    printk("Syscall");
}

void sys_init()
{
    idt_set_handler(128, syscall_handler);
}