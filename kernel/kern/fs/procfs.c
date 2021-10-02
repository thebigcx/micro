#include <micro/procfs.h>
#include <micro/heap.h>
#include <micro/vfs.h>

void procfs_init()
{
    /*struct inode* proc = kcalloc(sizeof(struct inode));

    proc->type  = S_IFDIR;
    proc->perms = 0555;
    proc->uid   = 0;
    proc->gid   = 0;

    vfs_addnode(proc, "/proc");*/
}