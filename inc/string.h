#ifndef STRING_H
#define STRING_H

uint32_t strlen(const char *s);
char *strchr(const char *s, int c);
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strdup(const char *s);
char *strjoin(char const *s1, char const *s2);
char *strrchr(const char *s, int c);
char *substr(char const *s, unsigned int start, size_t len);

void *memset(void *b, char c, size_t len);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memchr(const void *s, int c, size_t n);
uint32_t strnlen(const char *s, uint32_t maxlen);

void bzero(void *s, uint32_t n);

int atoi(const char *str);

#define SKIP_WHITESPACE(p) \
	do { \
		while (*(p) == ' ' || *(p) == '\t') \
			(p)++; \
	} while (0)

#define SKIP_LINE(ptr) do { \
	while (*(ptr) != '\n' && *(ptr) != '\0') \
		(ptr)++; \
	if (*(ptr) == '\n') \
		(ptr)++; \
	} while (0)

#endif
