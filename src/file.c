#include <syscall.h>
#include <printf.h>
#include <unistd.h>
#include <malloc.h>
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

// emerg_read - made for files like in /proc
static int emerg_read(int fd){
	char buf[4096];
	uint32_t rsize = 0;
	while (true) {
		uint32_t rval = read(fd, buf, 4096);
		rsize += rval;
		if (rval != 0 || rval < 4096) 
			break;
	}

	lseek(fd, 0, SEEK_SET);
	return (rsize);
}

// read_file - reads a whole file and return an allocated char *
char *read_file(char *path) {
	int fd;
	int32_t file_size = 0;
	char *fcontent;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		log(DEBUG, "Cannot read file %s", path);
		return NULL;
	}

	file_size = fdsize(fd);
	if (file_size < 0)
		return NULL;
	if (file_size == 0) {
		file_size = emerg_read(fd);
	}
	fcontent = malloc(file_size + 1);
	read(fd, fcontent, file_size);
	fcontent[file_size] = '\0';
	
	return fcontent;
}
