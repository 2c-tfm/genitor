#ifndef UNISTD_H
#define UNISTD_H

#include "types.h"
#include "time.h"

struct stat {
    uint64_t st_dev;
    uint64_t st_ino;
    uint64_t st_nlink;

    uint32_t st_mode;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t __pad0;

    uint64_t st_rdev;
    int64_t  st_size;
    int64_t  st_blksize;
    int64_t  st_blocks;

    int64_t  st_atime;
    uint64_t st_atime_nsec;

    int64_t  st_mtime;
    uint64_t st_mtime_nsec;

    int64_t  st_ctime;
    uint64_t st_ctime_nsec;

    int64_t  __unused[3];
};


int read (uint32_t fd, void *buf, uint64_t count);
int write (uint32_t fd, const char *buf, uint64_t count);
int open (const char *filename, int32_t flags);
int stat(const char *fpath, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
off_t lseek(int fildes, off_t offset, int whence);
int mount (const char *src, const char *target, 
		const char *fstype, uint64_t flags, 
		const void *data);
int umount2(const char * target, int flags);
int swapon (const char *path, int swapflags);
int close(int fd);
int dup2(int oldfd, int newfd);
pid_t fork(void);
int execve(const char *path, const char *argv[], char **envp);
void exit(int status);
int chdir(const char *path);
int kill(pid_t pid, int sig);
pid_t wait4(pid_t pid, int *wstatus, int options, void *rusage);

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, uint64_t off);
int mprotect(void *addr, size_t len, int prot);
int munmap(void *addr, size_t len);
int nanosleep(struct timespec *req, struct timespec *rmtp);
void *mremap(void *p, size_t old_size, size_t new_size, int flags);

ssize_t readlinkat(int dirfd, const char *path, char *buf, uint64_t size);
int finit_module(int fd, const char *param_values, int flags);
int64_t getdents64(int fd, void *dirp, uint32_t count);
int64_t getdents(int fd, char *buf, uint32_t count);
int openat(int fd, const char *path, int oflag, int mode);

// taken from glibc2.44 bits/fcntl.h:24
/* File access modes for `open' and `fcntl'.  */
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002

#define O_CREAT     0x0040
#define O_EXCL      0x0080
#define O_NOCTTY    0x0100
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
#define O_NONBLOCK  0x0800

#define O_DSYNC     0x001000
#define O_ASYNC     0x002000
#define O_SYNC      0x101000
#define O_FSYNC     O_SYNC

#define O_DIRECT    0x004000
#define O_DIRECTORY 0x010000
#define O_NOFOLLOW  0x020000
#define O_NOATIME   0x040000
#define O_CLOEXEC   0x080000

#define CHAR_BIT 8
#define INT_MAX  2147483647


// taken from glibc bits/mman.h:32
#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_TYPE      0x0f
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS
#define MAP_FAILED    ((void *) -1)

#define MS_RDONLY 1UL

#define SEEK_SET 0
#define AT_FDCWD -100

#ifndef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)
#endif

#endif
