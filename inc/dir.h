#ifndef DIR_H
#define DIR_H

#define DENTS_BUF_SIZE 4096

struct linux_dirent64 {
	uint64_t        d_ino;
	int64_t         d_off;
	unsigned short  d_reclen;
	unsigned char   d_type;
	char            d_name[];
};

typedef struct linux_dirent64 dirent;

typedef struct dir_iterator{
	int fd;
	long nread;
	long pos;
	int initialized;
	char buf[DENTS_BUF_SIZE];
	struct {
		unsigned char type;
		char name[256];
	} entry;
} dir_iterator;


void dir_close(dir_iterator *it); 
dir_iterator *dir_open(const char *path);
dirent *dir_read (dir_iterator *it);

#endif
