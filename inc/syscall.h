#ifndef SYSCALL_H
#define SYSCALL_H

#include "signal.h"
#include "unistd.h"


#define SYS_READ	0
#define SYS_WRITE	1
#define SYS_OPEN	2
#define SYS_CLOSE	3
#define SYS_DUP2	33
#define SYS_FORK	57
#define SYS_EXECVE	59
#define SYS_SIGACTION	67
#define SYS_MOUNT	165

extern long syscall(long nr, ...);

#endif
