#include <genitor.h>

uint32_t strlen(const char *s) {
	uint32_t i = 0;
	if (s == NULL)
		return 0;

	while (s[i] != '\0')
		i++;

	return (i);
}

void *memset(void *s, char c, uint32_t n) {
	uint32_t i = 0;

	if (s == NULL)
		return 0;

	while (i < n)
		((char *)s)[i++] = c;

	return (s);
}
