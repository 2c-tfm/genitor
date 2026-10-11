#ifndef PRINTF_H
#define PRINTF_H

uint32_t printf(const char *fmt, ...);
void puts(const char *s);
uint32_t fprintf(int fd, const char *fmt, ...);
uint32_t vprintf(int fd, const char *fmt, va_list args);

#endif
