#include <syscall.h>
#include <unistd.h>
#include <genitor.h>

uint64_t fsize(const char *filename) {
	struct stat st; 

	if (stat(filename, &st) == 0)
		return st.st_size;

	return -1; 
}

uint64_t fdsize(int fd) {
	struct stat st; 

	if (fstat(fd, &st) == 0)
		return st.st_size;

	return -1; 
}
