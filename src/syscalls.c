#include <genitor.h>
#include <syscall.h>

int read (uint32_t fd, void *buf, uint64_t count) {
	return syscall(SYS_READ, fd, buf, count);
}

int write (uint32_t fd, const char *buf, uint64_t count) {
	return syscall(SYS_WRITE, fd, buf, count);
}

int open (const char *filename, int32_t flags) {
	return syscall(SYS_OPEN, filename, flags);
}

int close(int fd) {
	return syscall(SYS_CLOSE, fd);
}

int dup2(int oldfd, int newfd) {
	return syscall(SYS_DUP2, oldfd, newfd);
}

pid_t fork(void) {
	return syscall(SYS_FORK);
}

int execve(const char *path, const char *argv[], const char *envp) {
	return syscall(SYS_EXECVE, path, argv, envp);
}


int sigaction(int signum, const struct sigaction *act,
		struct sigaction *oldact) {
	return syscall(SYS_SIGACTION, signum, act, oldact);
}

sighandler_t signal (int signum, sighandler_t handler) {
	struct sigaction sa;

	sa.sa_handler = handler;
	sa.sa_mask = 0;
	sa.sa_flags = SA_RESTART;
	sigaction(signum, &sa, NULL);
	return 0;		// for now
}


int mount (const char *src, const char *target, 
		const char *fstype, uint64_t flags, 
		const void *data) {
	return syscall(SYS_MOUNT, src, target, fstype, flags, data);
}

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, uint64_t off) {
	return (void *)syscall(SYS_MMAP, addr, len, prot, flags, fildes, off);
}

int mprotect(void *addr, size_t len, int prot) {
	return syscall(SYS_MPROTECT, addr, len, prot);
}

int munmap(void *addr, size_t len) {
	return syscall(SYS_MUNMAP, addr, len);
}

void *mremap(void *p, size_t oldsize, size_t newsize, int flags) {
	return (void *) syscall(SYS_MREMAP,p, oldsize, newsize, flags);
}

int stat(const char *fpath, struct stat *statbuf){
	return syscall(SYS_STAT, fpath, statbuf);
}

int fstat(int fd, struct stat *statbuf){
	return syscall(SYS_FSTAT, fd, statbuf);
}

ssize_t readlinkat(int dirfd, const char *path, char *buf, uint64_t size) {
	return syscall(SYS_READLINKAT, dirfd, path, buf, size);
}

off_t lseek(int fildes, off_t offset, int whence) {
	return syscall(SYS_LSEEK, fildes, offset, whence);
}
