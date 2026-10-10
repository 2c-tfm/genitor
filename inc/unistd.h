#ifndef UNISTD_H
#define UNISTD_H

int read (uint32_t fd, char *buf, uint64_t count);
int write (uint32_t fd, const char *buf, uint64_t count);
int open (const char *filename, int32_t flags);
int mount (const char *src, const char *target, 
		const char *fstype, uint64_t flags, 
		const void *data);
int close(int fd);
int dup2(int oldfd, int newfd);
pid_t fork(void);
int execve(const char *path, const char *argv[], const char *envp);

// taken from glibc2.44 bits/fcntl.h:24
/* File access modes for `open' and `fcntl'.  */
#define	O_RDONLY	0	/* Open read-only.  */
#define	O_WRONLY	1	/* Open write-only.  */
#define	O_RDWR		2	/* Open read/write.  */
#define	O_CREAT		0x0200	/* Create file if it doesn't exist.  */
#define	O_EXCL		0x0800	/* Fail if file already exists.  */
#define	O_TRUNC		0x0400	/* Truncate file to zero length.  */
#define	O_NOCTTY	0x8000	/* Don't assign a controlling terminal.  */
#define	O_ASYNC		0x0040	/* Send SIGIO to owner when data is ready.  */
#define	O_FSYNC		0x0080	/* Synchronous writes.  */
#define	O_SYNC		O_FSYNC
#define	O_APPEND	0x0008	/* Writes append to the file.  */
#define	O_NONBLOCK	0x0004	/* Non-blocking I/O.  */

typedef int pid_t;

#endif
