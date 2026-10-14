#ifndef SYSCALL_H
#define SYSCALL_H

#include "signal.h"
#include "unistd.h"


#define SYS_READ		0
#define SYS_WRITE		1
#define SYS_OPEN		2
#define SYS_CLOSE		3
#define SYS_STAT		4
#define SYS_FSTAT		5
#define SYS_WAITPID		7
#define SYS_LSEEK		8
#define SYS_MMAP		9
#define SYS_MPROTECT		10
#define SYS_MUNMAP		11
#define SYS_MREMAP		25
#define SYS_DUP2		33
#define SYS_NANOSLEEP		35
#define SYS_FORK		57
#define SYS_EXECVE		59
#define SYS_EXIT		60
#define SYS_WAIT4		61
#define SYS_KILL		62
#define SYS_SIGACTION		67
#define SYS_GETDENTS		78
#define SYS_CHDIR		80
#define SYS_MOUNT		165
#define SYS_UMOUNT2		166
#define SYS_SWAPON		167
#define SYS_GETDENTS64		217
#define SYS_CLOCK_GETTIME	228
#define SYS_OPENAT		257
#define SYS_READLINKAT		267
#define SYS_FINIT_MODULE	379


extern long syscall(long nr, ...);

#endif
