#include <micro/vfs.h>
#include <micro/types.h>
#include <micro/stdlib.h>
#include <micro/tree.h>
#include <micro/heap.h>
#include <micro/errno.h>
#include <micro/debug.h>

struct tree root;

struct file* vfs_create_file()
{
    struct file* file = kmalloc(sizeof(struct file));
    
    file->device = NULL;
    file->inode = 0;
    file->flags = 0;
    file->parent = NULL;
    memset(&file->ops, 0, sizeof(struct file_ops));

    return file;
}

void vfs_init()
{
    root = tree_create();
}

ssize_t vfs_read(struct file* file, void* buf, off_t off, size_t size)
{
    if (file->ops.read) return file->ops.read(file, buf, off, size);
    return 0;
}

ssize_t vfs_write(struct file* file, const void* buf, off_t off, size_t size)
{
    if (file->ops.write) return file->ops.write(file, buf, off, size);
    return 0;
}

struct file* vfs_find(struct file* dir, const char* name)
{
    if (dir && ((dir->flags & FL_DIR) || (dir->flags & FL_MNTPT)) && dir->ops.find)
    {
        return dir->ops.find(dir, name);
    }

    return NULL;
}

int vfs_readdir(struct file* dir, size_t idx, struct dirent* dirent)
{
    if (dir && ((dir->flags & FL_DIR) || (dir->flags & FL_MNTPT)) && dir->ops.readdir)
    {
        return dir->ops.readdir(dir, idx, dirent);
    }

    return -ENOTDIR;
}

// TODO: return int on sucess ?
void vfs_mkfile(const char* path)
{
    char* relat;
    struct file* file = vfs_getmnt(path, &relat);

    char* saveptr;
    char* token = strtok_r(relat, "/", &saveptr);
    char* next = strtok_r(NULL, "/", &saveptr);

    if (!file || !(file->flags & FL_MNTPT) || !token) return;

    while (next)
    {
        struct file* child = vfs_find(file, token);
        //kfree(file);
        file = child;
        if (!file) return;

        token = next;
        next = strtok_r(NULL, "/", &saveptr);
    }

    if (file && ((file->flags & FL_DIR) || (file->flags & FL_MNTPT)) && file->ops.mkfile)
        file->ops.mkfile(file, token);

    kfree(relat);
}

void vfs_mkdir(const char* path)
{
    char* relat;
    struct file* file = vfs_getmnt(path, &relat);

    char* saveptr;
    char* token = strtok_r(relat, "/", &saveptr);
    char* next = strtok_r(NULL, "/", &saveptr);

    if (!file || !(file->flags & FL_MNTPT) || !token) return;

    while (next)
    {
        struct file* child = vfs_find(file, token);
        //kfree(file);
        file = child;
        if (!file) return;

        token = next;
        next = strtok_r(NULL, "/", &saveptr);
    }

    if (file && ((file->flags & FL_DIR) || (file->flags & FL_MNTPT)) && file->ops.mkdir)
        file->ops.mkdir(file, token);

    kfree(relat);
}

void vfs_rm(struct file* dir, const char* name)
{
    if (dir && ((dir->flags & FL_DIR) || (dir->flags & FL_MNTPT)) && dir->ops.rm)
        dir->ops.rm(dir, name);
}

int vfs_ioctl(struct file* file, unsigned long req, void* argp)
{
    if (file && file->ops.ioctl)
        return file->ops.ioctl(file, req, argp);

    return -ENOTTY;
}

// Add a node (or 'struct file') to the VFS
int vfs_addnode(struct file* file, const char* path)
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
                nfile = vfs_create_file();
                strcpy(nfile->name, old);
                nfile->flags = FL_DIR;
            }
            else
                nfile = file;

            strcpy(nfile->name, old);

            nfile->parent = curr->data;

            tree_push_back(curr, nfile);
            curr = curr->children.tail->data;
        }
    }

    if (curr == &root)
    {
        // Modify the root node
        root.data = file;
    }

    kfree(path_cpy);

    return 0;
}

void* vfs_rmnode(const char* path)
{
    // TODO: impl
    return NULL;
}

// vfs_getmnt(): returns a copy of the mount point of a path
// *relat: the relative path in the mounted filesystem
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

    struct file* mnt = kmalloc(sizeof(struct file));
    memcpy(mnt, curr->data, sizeof(struct file));
    return mnt;
}

char* vfs_mkcanon(const char* path, const char* work)
{
    char* tmp;

    if (!work || path[0] == '/')
    {
        tmp = strdup(path);
    }
    else
    {
        tmp = kmalloc(strlen(path) + 1 + strlen(work) + 1);
        strcpy(tmp, work);
        strcpy(tmp + strlen(tmp), "/");
        strcpy(tmp + strlen(tmp), path);
    }

    struct list tokens = list_create();

    char* saveptr;
    char* token = strtok_r(tmp, "/", &saveptr);

    size_t fsize = token ? 0 : 1;

    while (token)
    {
        if (token[0] == 0 || !strcmp(token, ".")) {}
        else if (!strcmp(token, ".."))
        {
            if (tokens.size)
                fsize -= strlen(list_pop_back(&tokens));
        }
        else
        {
            list_push_back(&tokens, token);
            fsize += strlen(token) + 1;
        }

        token = strtok_r(NULL, "/", &saveptr);
    }

    char* ret = kmalloc(fsize + 1);
    memset(ret, 0, fsize + 1);

    if (!tokens.size)
        strcpy(ret, "/");

    LIST_FOREACH(&tokens)
    {
        char* token = node->data;
        strcpy(ret + strlen(ret), "/");
        strcpy(ret + strlen(ret), token);
    }

    kfree(tmp);
    
    return ret;
}

// TODO: vfs_resolve should return an error code

// Will always return a file that can be kfree'd
struct file* vfs_resolve(const char* path)
{
    char* relat;
    struct file* file = vfs_getmnt(path, &relat);

    char* saveptr;
    char* token = strtok_r(relat, "/", &saveptr);

    if (!file || !(file->flags & FL_MNTPT)) // Not a mounted filesystem
    {
        if (relat[0] == 0)
        {
            // Make a copy so the caller can free once done without affecting the VFS
            struct file* ret = kmalloc(sizeof(struct file));
            memcpy(ret, file, sizeof(struct file));
            return ret; // Virtual filesystem node
        }
        else return NULL; // 'path' does not exist in the VFS
    }

    if (!token) // Return a copy of the mounted filesystem root
    {
        struct file* ret = kmalloc(sizeof(struct file));
        memcpy(ret, file, sizeof(struct file));
        return ret;
    }

    while (token)
    {
        struct file* child = vfs_find(file, token);
        kfree(file);
        file = child;
        if (!file) return NULL;

        token = strtok_r(NULL, "/", &saveptr);
    }

    return file;
}

struct fd* vfs_open(struct file* file, uint32_t flags, mode_t mode)
{
    if (file->ops.open) return file->ops.open(file, flags, mode);

    struct fd* fd = kmalloc(sizeof(struct fd));
    fd->filp = file;
    fd->off = 0;
    return fd;
}

void vfs_close(struct fd* fd)
{
    if (fd->filp->ops.close)
    {
        fd->filp->ops.close(fd);
        return;
    }

    //kfree(fd->filp);
    //kfree(fd);
    // TODO: free memory
}

int vfs_access(const char* path, int mode)
{
    struct file* file = vfs_resolve(path);
    if (!file) return -ENOENT;

    // TODO: permission checks
    return 0;
}

static struct fs_type fs_types[64];
static unsigned int fs_count;

int vfs_mount_fs(const char* dev, const char* mnt, const char* fs, void* data)
{
    for (unsigned int i = 0; i < fs_count; i++)
    {
        if (!strcmp(fs_types[i].name, fs))
        {
            struct file* file = fs_types[i].mount(dev, data);
            vfs_addnode(file, mnt);
            return;
        }
    }

    return -ENODEV;
}

int vfs_umount_fs(const char* mnt)
{
    // TODO: implement
    return 0;
}

// mount: mount callback
void vfs_register_fs(char* fs, mount_t mount)
{
    fs_types[fs_count++] = (struct fs_type)
    {
        .name = fs,
        .mount = mount
    };
}