#include <micro/vfs.h>
#include <micro/types.h>
#include <micro/stdlib.h>
#include <micro/tree.h>
#include <micro/heap.h>
#include <micro/errno.h>

struct tree root;

static struct file* file_create(const char* name)
{
    struct file* file = kmalloc(sizeof(struct file));
    
    strcpy(file->name, name);
    file->device = NULL;
    file->inode = 0;
    file->flags = 0;
    memset(&file->ops, 0, sizeof(struct file_ops));

    return file;
}

void vfs_init()
{
    root = tree_create();
    root.data = file_create("root");
}

ssize_t vfs_read(struct file* file, void* buf, off_t off, size_t size)
{
    if (file->ops.read) return file->ops.read(file, buf, off, size);
    return 0;
}

ssize_t vfs_write(struct file* file, void* buf, off_t off, size_t size)
{
    if (file->ops.write) return file->ops.write(file, buf, off, size);
    return 0;
}

struct file* vfs_find(struct file* dir, const char* name)
{
    if (dir && (dir->flags & FL_DIR) && dir->ops.find)
    {
        return dir->ops.find(dir, name);
    }

    return NULL;
}

int vfs_mount(struct file* file, const char* path)
{
    struct tree* curr = &root;

    char* path_cpy = kmalloc(strlen(path));
    strcpy(path_cpy, path + 1);

    char* saveptr;
    char* token = strtok_r(path_cpy, "/", &saveptr);

    while (token)
    {
        int found = 0;
        LIST_FOREACH(&curr->children)
        {
            // (struct tree*) node: child of 'curr' tree node
            struct file* child_file = ((struct tree*)node->data)->data;
            if (!strcmp(child_file->name, token))
            {
                found = 1;
                curr = node->data;
                break;
            }
        }

        char* old = token;
        token = strtok_r(NULL, "/", &saveptr);

        if (!found)
        {
            struct file* nfile;
            if (token) // Create a new directory
            {
                nfile = file_create(old);
                nfile->flags = FL_DIR;
            }
            else
                nfile = file;

            strcpy(nfile->name, old);

            tree_push_back(curr, nfile);
            curr = curr->children.tail->data;
        }
    }

    kfree(path_cpy);

    return 0;
}

struct file* vfs_getmnt(const char* path, char** relat)
{
    struct tree* curr = &root;

    size_t pos = 0;

    char* path_cpy = kmalloc(strlen(path));
    strcpy(path_cpy, path + 1);

    char* saveptr;
    char* token = strtok_r(path_cpy, "/", &saveptr);

    while (token)
    {
        int found = 0;
        LIST_FOREACH(&curr->children)
        {
            // (struct tree*) node: child of 'curr' tree node
            struct file* file = ((struct tree*)node->data)->data;
            if (!strcmp(file->name, token))
            {
                found = 1;
                curr = node->data;
                break;
            }
        }

        if (!found)
        {
            pos++;
            break;
        }

        pos += strlen(token) + 1;
        token = strtok_r(NULL, "/", &saveptr);
    }

    kfree(path_cpy);

    *relat = kmalloc(strlen(path) - pos + 1);
    strcpy(*relat, path + pos);    

    return (struct file*)curr->data;
}

struct file* vfs_resolve(const char* path)
{
    char* relat;
    struct file* file = vfs_getmnt(path, &relat);

    if (!(file->flags & FL_DIR))
    {
        return file; // File type
    }

    char* saveptr;
    char* token = strtok_r(relat, "/", &saveptr);

    while (token)
    {
        file = vfs_find(file, token);
        if (!file) return NULL;

        token = strtok_r(NULL, "/", &saveptr);
    }

    return file;
}

struct fd* vfs_open(struct file* file)
{
    struct fd* fd = kmalloc(sizeof(struct fd));
    fd->filp = file;
    fd->off = 0;
    return fd;
}

void vfs_close(struct fd* fd)
{
    // TODO: free memory
}

int vfs_access(const char* path, int mode)
{
    struct file* file = vfs_resolve(path);
    if (!file) return -ENOENT;

    // TODO: permission checks
    return 0;
}