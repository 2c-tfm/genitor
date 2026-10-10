#ifndef SYSCALL_H
#define SYSCALL_H

static inline int mount (const char *src, const char *target, 
		const char * fstype, uint64_t flags, 
		const void *_Nullable data);

#define SYS_MOUNT	165

#endif
