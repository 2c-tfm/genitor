#ifndef PRINTF_H
#define PRINTF_H

#define DBG
#define LOG_COLOR

typedef enum LOG_LVL {
	SUCCESS,
	EMERG,
	ERROR,
	WARN,
	INFO,
	DEBUG,
} LOG_LVL;

typedef struct {
	char   *buf;
	size_t  size;
	size_t  len;
} snprint_ctx;

uint32_t printf(const char *fmt, ...);
void puts(const char *s);
uint32_t fprintf(int fd, const char *fmt, ...);
uint32_t vprintf(int fd, const char *fmt, va_list args);
uint32_t log(LOG_LVL log, char *fmt, ...);
int snprintf(char *buf, size_t size, const char *fmt, ...);

#endif

