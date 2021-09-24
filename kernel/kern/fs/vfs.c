#include <micro/vfs.h>
#include <micro/types.h>
#include <micro/stdlib.h>
#include <micro/tree.h>
#include <micro/heap.h>
#include <micro/errno.h>
#include <micro/debug.h>
#include <micro/fcntl.h>
#include <micro/task.h>

struct tree root;

struct file* vfs_create_file()
{
    struct file* file = kmalloc(sizeof(struct file));
    memset(file, 0, sizeof(struct file));
    return file;
}

int vfs_checkperm(struct file* file, unsigned int mask)
{
    if (task_curr() && task_curr()->euid != 0)
    {
        if (task_curr()->euid == file->uid)
        {
            if(!(file->perms & (mask << 6))) return -1;
            else return 0;
        }

        if (task_curr()->egid == file->gid)
        {
            if (!(file->perms & (mask << 3))) return -1;
            else return 0;
        }

        if (!(file->perms & mask)) return -1;
    }

    return 0;
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
    if (dir && (dir->type == FL_DIR) && dir->ops.find)
    {
        return dir->ops.find(dir, name);
    }

    return NULL;
}

ssize_t vfs_getdents(struct file* dir, off_t off, size_t n, struct dirent* dirp)
{
    if (dir && (dir->type == FL_DIR) && dir->ops.getdents)
    {
        return dir->ops.getdents(dir, off, n, dirp);
    }

    return -ENOTDIR;
}

static int get_parent_dir(const char* path, struct file* out, char** name)
{
    char* relat;
    struct file* file = vfs_getmnt(path, &relat);

    char* saveptr;
    char* token = strtok_r(relat, "/", &saveptr);
    char* next = strtok_r(NULL, "/", &saveptr);

    if (!file || !token) return -ENOENT;

    while (next)
    {
        if (!(file->type == FL_DIR)) return -ENOTDIR;

        CHECK_RPERM(file);

        struct file* child = vfs_find(file, token);
        kfree(file);
        file = child;
        if (!file) return -ENOENT;

        token = next;
        next = strtok_r(NULL, "/", &saveptr);
    }

    if (file->type != FL_DIR) return -ENOTDIR;

    memcpy(out, file, sizeof(struct file));
    *name = strdup(token);

    kfree(file);
    return 0;
}

int vfs_mkfile(const char* path, mode_t mode, uid_t uid, gid_t gid)
{
    struct file dir;
    char* name;
    int e;
    if ((e = get_parent_dir(path, &dir, &name))) return e;

    CHECK_WPERM(&dir);

    if (dir.ops.mkfile)
        dir.ops.mkfile(&dir, name, mode, uid, gid);

    kfree(name);
    return 0;
}

int vfs_mkdir(const char* path, mode_t mode, uid_t uid, gid_t gid)
{
    struct file dir;
    char* name;
    int e;
    if ((e = get_parent_dir(path, &dir, &name))) return e;

    if (dir.ops.mkdir)
        dir.ops.mkdir(&dir, name, mode, uid, gid);

    kfree(name);
    return 0;
}

int vfs_unlink(const char* pathname)
{
    struct file dir;
    char* name;
    int e;
    if ((e = get_parent_dir(pathname, &dir, &name))) return e;

    struct file file;
    if ((e = vfs_resolve(pathname, &file, 1))) return e;
    if (file.type == FL_DIR) return -EISDIR;

    if (dir.ops.unlink)
        dir.ops.unlink(&dir, name);

    kfree(name);
    return 0;
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
                nfile->type = FL_DIR;
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

// Hold the depth of symlinks, so we can return ELOOP is necessary
static int do_vfs_resolve(const char* path, struct file* out, int symlinks, int depth)
{
    if (depth >= 8) return -ELOOP;

    char* relat;
    struct file* file = vfs_getmnt(path, &relat);

    char* saveptr;
    char* token = strtok_r(relat, "/", &saveptr);

    if (!token) // Return the mounted filesystem root
    {
        memcpy(out, file, sizeof(struct file));
        return 0;
    }

    while (token)
    {
        if (file->type != FL_DIR) return -ENOTDIR;

        CHECK_RPERM(file);

        struct file* child = vfs_find(file, token);
        kfree(file);
        file = child;

        if (!file) return -ENOENT;

        if (file->type == FL_SYMLINK && symlinks)
        {
            char link[60];
            ssize_t n = vfs_readlink(file, link, 60);
            link[n] = 0; // Must null-terminate as readlink() does not
            
            return do_vfs_resolve(link, out, 1, ++depth); // Resolve the symlink path
        }

        token = strtok_r(NULL, "/", &saveptr);
    }

    memcpy(out, file, sizeof(struct file));
    return 0;
}

int vfs_resolve(const char* path, struct file* out, int symlinks)
{
    return do_vfs_resolve(path, out, symlinks, 0);
}

struct fd* vfs_open(struct file* file, uint32_t flags, mode_t mode)
{
    if (file->ops.open) return file->ops.open(file, flags, mode);

    struct fd* fd = kmalloc(sizeof(struct fd));

    fd->filp  = file;
    fd->off   = 0;
    fd->flags = flags;

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
    struct file* file = kmalloc(sizeof(struct file));
    int e = vfs_resolve(path, file, 1);

    if (e) return e;
    return 0;
}

void vfs_mmap(struct file* file, struct vm_area* area)
{
    if (file->ops.mmap)
        file->ops.mmap(file, area);
}

int vfs_chmod(struct file* file, mode_t mode)
{
    if (file->ops.chmod)
        return file->ops.chmod(file, mode);

    return -ENOENT;
}

int vfs_chown(struct file* file, uid_t uid, gid_t gid)
{
    if (file->ops.chown)
        return file->ops.chown(file, uid, gid);

    return -ENOENT;
}

int vfs_readlink(struct file* file, char* buf, size_t n)
{
    if (file->ops.readlink)
        return file->ops.readlink(file, buf, n);

    return -EINVAL;
}

static struct fs_type fs_types[64];
static unsigned int fs_count;

int vfs_mount_fs(const char* dev, const char* mnt,
                 const char* fs, const void* data)
{
    for (unsigned int i = 0; i < fs_count; i++)
    {
        if (!strcmp(fs_types[i].name, fs))
        {
            struct file* file = fs_types[i].mount(dev, data);
            vfs_addnode(file, mnt);
            return 0;
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