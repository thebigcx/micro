#include <micro/vfs.h>
#include <micro/types.h>
#include <micro/stdlib.h>
#include <micro/tree.h>
#include <micro/heap.h>
#include <micro/errno.h>
#include <micro/debug.h>
#include <micro/fcntl.h>
#include <micro/task.h>

//struct tree root;

static struct list mounts;

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
            if(!(file->mode & (mask << 6))) return -1;
            else return 0;
        }

        if (task_curr()->egid == file->gid)
        {
            if (!(file->mode & (mask << 3))) return -1;
            else return 0;
        }

        if (task_curr()->groupcnt)
        {
            // Supplementary groups
            for (size_t i = 0; i < task_curr()->groupcnt; i++)
            {
                if (file->gid == task_curr()->groups[i])
                {
                    if (!(file->mode & (mask << 3))) return -1;
                    else return 0;
                }
            }
        }

        if (!(file->mode & mask)) return -1;
    }

    return 0;
}

void vfs_init()
{
    mounts = list_create();
    //root = tree_create();
}

ssize_t vfs_read_new(struct fd* file, void* buf, size_t size)
{
    if (file->ops.read)
    {
        ssize_t nread = file->ops.read(file, buf, file->off, size);
        file->off += nread;
        return nread;
    }

    return 0;
}

ssize_t vfs_write_new(struct fd* file, const void* buf, size_t size)
{
    if (file->ops.write)
    {
        ssize_t nwrite = file->ops.write(file, buf, file->off, size);
        file->off += nwrite;
        return nwrite;
    }

    return 0;
}

ssize_t vfs_pread(struct fd* file, void* buf, size_t size, off_t off)
{
    if (file->ops.read)
        return file->ops.read(file, buf, off, size);

    return 0;
}

ssize_t vfs_pwrite(struct fd* file, const void* buf, size_t size, off_t off)
{
    if (file->ops.write)
        return file->ops.write(file, buf, off, size);

    return 0;
}

ssize_t vfs_getdents(struct file* dir, off_t off, size_t n, struct dirent* dirp)
{
    if (dir && S_ISDIR(dir->mode) && dir->ops.getdents)
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
        if (!S_ISDIR(file->mode)) return -ENOTDIR;

        CHECK_RPERM(file);

        struct dentry dentry;
        int e;
        if ((e = file->ops.lookup(file, token, &dentry))) return e;
        kfree(file);
        file = dentry.file;

        token = next;
        next = strtok_r(NULL, "/", &saveptr);
    }

    if (!S_ISDIR(file->mode)) return -ENOTDIR;

    memcpy(out, file, sizeof(struct file));
    *name = strdup(token);

    kfree(file);
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

int vfs_mknod(const char* path, mode_t mode, dev_t dev, uid_t uid, gid_t gid)
{
    struct file dir;
    char* name;
    int e;
    if ((e = get_parent_dir(path, &dir, &name))) return e;

    if (dir.ops.mknod)
        dir.ops.mknod(&dir, name, mode, dev, uid, gid);

    kfree(name);
    return 0;
}

static void vfs_do_unlink(struct file* dir, const char* name)
{
    if (dir->ops.unlink)
        dir->ops.unlink(dir, name);
}

int vfs_unlink(const char* pathname)
{
    struct file dir;
    char* name;
    int e;
    if ((e = get_parent_dir(pathname, &dir, &name))) return e;

    struct file file;
    if ((e = vfs_resolve(pathname, &file, 1))) return e;
    if (S_ISDIR(file.mode)) return -EISDIR;

    vfs_do_unlink(&dir, name);

    kfree(name);
    return 0;
}

int vfs_ioctl(struct fd* file, unsigned long req, void* argp)
{
    if (file && file->ops.ioctl)
        return file->ops.ioctl(file, req, argp);

    return -ENOTTY;
}

// vfs_getmnt(): returns a copy of the mount point of a path
// *relat: the relative path in the mounted filesystem
struct file* vfs_getmnt(const char* path, char** relat)
{
    struct mount* candidate = NULL;
    size_t match = 0;

    LIST_FOREACH(&mounts)
    {
        struct mount* mount = node->data;

        // Cannot be the mount point
        if (strlen(mount->path) > strlen(path))
            continue;

        // Match as much of 'path' as possible
        size_t i;
        for (i = 0; i < strlen(mount->path) && path[i] == mount->path[i]; i++);

        char* pathcpy = strdup(path);
        pathcpy[i] = 0;
        
        // Does this mount point better match the previous?
        if (i > match && !strcmp(pathcpy, mount->path))
        {
            candidate = mount;
            match = i;
        }

        kfree(pathcpy);
    }

    if (!candidate) return NULL;

    // getmnt() called with the mount point itself (no children)
    if (path[match] == 0)
        *relat = strdup("");
    else
    {    
        *relat = kmalloc(strlen(path) - match + 1);
        strcpy(*relat, path + match);
    }

    return memdup(candidate->file, sizeof(struct file));
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
            list_enqueue(&tokens, token);
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
        if (!S_ISDIR(file->mode)) return -ENOTDIR;

        CHECK_RPERM(file);

        struct dentry dentry;
        int e;
        if ((e = file->ops.lookup(file, token, &dentry))) return e;
        kfree(file);
        file = dentry.file;

        if (!file) return -ENOENT;

        if (S_ISLNK(file->mode) && symlinks)
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

int vfs_open_new(const char* path, struct fd* file, uint32_t flags)
{
    struct file* inode = kcalloc(sizeof(struct file));
    int e = vfs_resolve(path, inode, 1);
    if (e) return e;

    // Defaults
    file->filp  = inode;
    file->flags = flags;
    file->off   = 0;
    file->ops   = inode->fops;

    if (inode->fops.open)
        return inode->fops.open(inode, file);

    return 0;
}

void vfs_close(struct fd* fd)
{
    if (fd->ops.close)
    {
        fd->ops.close(fd);
        return;
    }

    //kfree(fd->filp);
    //kfree(fd);
    // TODO: free memory
}

int vfs_access(const char* path, int mode)
{
    //struct file* file = kmalloc(sizeof(struct file));
    //int e = vfs_resolve(path, file, 1);

    //if (e) return e;

    struct fd file;
    int e = vfs_open_new(path, &file, O_RDONLY);
    if (e) return e;

    if (mode & R_OK) CHECK_RPERM(file.filp);
    if (mode & W_OK) CHECK_WPERM(file.filp);
    if (mode & X_OK) CHECK_XPERM(file.filp);

    return 0;
}

void vfs_mmap(struct fd* file, struct vm_area* area)
{
    if (file->ops.mmap)
        file->ops.mmap(file, area);
}

int vfs_chmod(struct fd* file, mode_t mode)
{
    if (file->ops.chmod)
        return file->ops.chmod(file, mode);

    return -ENOENT;
}

int vfs_chown(struct fd* file, uid_t uid, gid_t gid)
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

int vfs_symlink(const char* target, const char* link)
{
    struct file file;

    if (vfs_access(link, F_OK) == 0) return -EEXIST;

    vfs_mknod(link, S_IFLNK | 0777, 0, task_curr()->euid, task_curr()->egid);

    int e;
    if ((e = vfs_resolve(link, &file, 0))) return e;

    if (file.ops.symlink)
        return file.ops.symlink(&file, target);

    return -ENOENT;
}

int vfs_link(const char* old, const char* new)
{
    struct file file;
    int e;
    if ((e = vfs_resolve(old, &file, 1))) return e;

    struct file dir;
    char* name;
    if ((e = get_parent_dir(new, &dir, &name))) return e;

    if (file.ops.link)
        return file.ops.link(&file, name, &dir);

    return -ENOENT;
}

int vfs_rename(const char* old, const char* new)
{
    int e;
    if ((e = vfs_link(old, new))) return e;

    struct file dir;
    char* name;
    if ((e = get_parent_dir(old, &dir, &name))) return e;

    vfs_do_unlink(&dir, name);

    return 0;
}

int vfs_rmdir(const char* path)
{
    // TODO: check empty
    struct file dir;
    char* name;
    int e;
    if ((e = get_parent_dir(path, &dir, &name))) return e;

    vfs_do_unlink(&dir, name);

    return 0;
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
            struct file* fsroot = kcalloc(sizeof(struct file));

            fsroot->mode = S_IFDIR;

            int e;
            if ((e = fs_types[i].mount(dev, data, fsroot))) return e;
            
            struct mount* mount = kmalloc(sizeof(struct mount));

            mount->file = fsroot;
            mount->path = strdup(mnt);

            list_enqueue(&mounts, mount);
            return 0;
        }
    }

    return -ENODEV;
}

int vfs_umount_fs(const char* mnt)
{
    size_t i = 0;
    LIST_FOREACH(&mounts)
    {
        struct mount* mount = node->data;

        if (!strcmp(mount->path, mnt))
        {
            // TODO: prepare for umount and clean up stuff

            // Can remove directly - about to break from loop
            list_remove(&mounts, i);
            return 0;
        }

        i++;
    }

    return -ENOENT;
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