#ifndef FILE_H
#define FILE_H

uint64_t fdsize(int fd);
uint64_t fsize(const char *filename);
char *read_file(char *path);

#endif
