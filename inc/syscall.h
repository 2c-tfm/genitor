#ifndef SYSCALL_H
#define SYSCALL_H

#include "signal.h"
#include "unistd.h"


#define SYS_READ	0
#define SYS_WRITE	1
#define SYS_OPEN	2
#define SYS_CLOSE	3
#define SYS_STAT	4
#define SYS_FSTAT	5
#define SYS_LSEEK	8
#define SYS_MMAP	9
#define SYS_MPROTECT	10
#define SYS_MUNMAP	11
#define SYS_MREMAP	25
#define SYS_DUP2	33
#define SYS_FORK	57
#define SYS_EXECVE	59
#define SYS_SIGACTION	67
#define SYS_MOUNT	165
#define SYS_UMOUNT2	166
#define SYS_READLINKAT	267

extern long syscall(long nr, ...);

#endif
