#ifndef FILE_H
#define FILE_H

int64_t fdsize(int fd);
int64_t fsize(const char *filename);
char *read_file(char *path);

#endif
