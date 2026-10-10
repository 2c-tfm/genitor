#include <genitor.h>
#include <syscall.h>

static inline int mount (const char *src, const char *target, 
		const char * fstype, uint64_t flags, 
		const void *_Nullable data) {
	
	int32_t ret;
	(void) 

	__asm__ volatile (
		".intel_syntax noprefix;"
		"syscall;"
		".att_syntax;"
		: "=a"(ret)
		: "0"((long)SYS_mount),
		  "D"(source),
		  "S"(target),
		  "d"(filesystemtype),
		  "r10"(mountflags),
		  "r8"(data)
		: "rcx", "r11", "memory"
	);
}
