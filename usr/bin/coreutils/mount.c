#include <sys/mount.h>
#include <stdio.h>
#include <mntent.h>
#include <unistd.h>

void usage()
{
    printf("usage: mount <source> <mountpoint> <fstype>\n");
}

int mount_all()
{
    FILE* file = setmntent("/etc/fstab", "r");
    struct mntent* ent;
    
    while ((ent = getmntent(file)))
        mount(ent->mnt_fsname, ent->mnt_dir, ent->mnt_type, 0, NULL);
    
    endmntent(file);

    return 0;
}

int main(int argc, char** argv)
{
    char opt;
    while ((opt = getopt(argc, argv, "a")) != -1)
    {
        switch (opt)
        {
            case 'a':
                return mount_all();
            default:
                usage();
                return -1;
        }
    }
    
    if (argc < 4)
    {
        usage();
        return -1;
    }

    if (mount(argv[1], argv[2], argv[3], 0, NULL) == -1)
    {
        perror("mount");
        return -1;
    }

    return 0;
}
