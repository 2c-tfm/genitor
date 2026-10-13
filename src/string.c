#include <genitor.h>
#include <malloc.h>

uint32_t strlen(const char *s) {
	uint32_t i = 0;
	if (s == NULL)
		return 0;

	while (s[i] != '\0')
		i++;

	return (i);
}

uint32_t strnlen(const char *s, uint32_t maxlen) {
	uint32_t i = 0;

	if (s == NULL)
		return 0;

	while (i < maxlen && s[i] != '\0')
		i++;

	return i;
}

char *strchr(const char *s, int c) {
	int i;

	i = 0;
	while (s[i] != (char) c)
	{
		if (s[i] == '\0')
			return (NULL);
		i++;
	}
	return ((char *)(s + i));
}

size_t strlcpy(char *dst, const char *src, size_t size) {
	size_t	i;

	i = 0;
	while (i + 1 < size && src[i] != '\0')
	{
		dst[i] = src[i];
		i++;
	}
	if (i < size)
		dst[i] = '\0';
	return (strlen(src));
}

size_t strlcat(char *dst, const char *src, size_t size) {
	size_t	dstlength;
	size_t	i;

	if (dst == NULL && !size)
		return (strlen(src));
	i = 0;
	dstlength = strlen(dst);
	if (dstlength >= size)
		return (size + strlen(src));
	while (i + 1 < (size - dstlength) && src[i] != '\0')
	{
		dst[dstlength + i] = src[i];
		i++;
	}
	dst[dstlength + i] = '\0';
	return (dstlength + strlen(src));
}

int strcmp(const char *s1, const char *s2) {
	size_t i = 0;

	while ((unsigned char)s1[i] == (unsigned char)s2[i]) {
		if (s1[i] == '\0' || s2[i] == '\0')
			return 0;
		i++;
	}

	return (unsigned char)s1[i] - (unsigned char)s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
	size_t	i;

	i = 0;
	while ((i < n && n != 0) && (s1[i] != '\0' || s2[i] != '\0'))
	{
		if ((unsigned char) s1[i] != (unsigned char) s2[i])
			return ((unsigned char) s1[i] - (unsigned char) s2[i]);
		i++;
	}
	return (0);
}

char *strdup(const char *s) {
	char *str ;

	str = (char *) malloc(strlen(s) + 1);
	if (str == NULL)
		return NULL;
	(void) strlcpy(str, s, strlen(s) +1);
	return str;
}

char *strjoin(char const *s1, char const *s2) {
	char *str_buffer;

	str_buffer = (char *)malloc(strlen(s1) + strlen(s2) + 1);
	if (str_buffer == NULL)
		return (NULL);
	strlcpy(str_buffer, s1, strlen(s1) + 1);
	strlcat(str_buffer, s2, (strlen(s2) + strlen(s1) + 1));
	return (str_buffer);
}
char *strrchr(const char *s, int c) {
	int length;

	length = (int)strlen(s) + 1;
	while (length-- > 0)
	{
		if (s[length] == (char) c)
			return ((char *)(s + length));
	}
	return (NULL);
}

char *substr(char const *s, unsigned int start, size_t len) {
	char	*alc_s;
	char	*s_allptr;

	if (s == NULL)
		return (NULL);
	if (start >= strlen(s))
		return (strdup(""));
	s_allptr = (char *) s ;
	s_allptr = (s_allptr + start);
	alc_s = strdup(s_allptr);
	if (alc_s == NULL)
		return (NULL);
	*(alc_s + len) = '\0';
	return (alc_s);
}

void *memset(void *b, char c, size_t len) {
	int i;

	i = 0;
	while (i < (int) len)
	{
		*((unsigned char *) b + i) = (unsigned char) c;
		i++;
	}
	return (b);
}

int memcmp(const void *s1, const void *s2, size_t n) {
	int i;
	unsigned char *cs1;
	unsigned char *cs2;

	i = 0;
	cs1 = (unsigned char *) s1;
	cs2 = (unsigned char *) s2;
	while (i < (int) n)
	{
		if (*cs1 != *cs2)
			return (*cs1 - *cs2);
		cs1++;
		cs2++;
		i++;
	}
	return (0);
}

void *memcpy(void *dst, const void *src, size_t n) {
	int	i;

	if (dst == NULL && src == NULL)
		return (NULL);
	i = 0;
	while (i < (int) n)
	{
		*((unsigned char *) dst + i) = *((unsigned char *) src + i);
		i++;
	}
	return (dst);
}

void *memchr(const void *s, int c, size_t n) {
	int	i;

	i = 0;
	while (i < (int) n)
	{
		if (*((unsigned char *)s + i) == (unsigned char) c)
			return ((void *)((unsigned char *) s + i));
		i++;
	}
	return (NULL);
}

void bzero(void *s, uint32_t n) {
	memset(s, 0, n);
}

int atoi(const char *str) {
	int	number;
	int	i;
	int	sign;

	number = 0;
	sign = 1;
	i = 0;
	while (str[i] == 32 || str[i] == '\t' || str[i] == '\v'
		|| str[i] == '\f' || str[i] == '\r' || str[i] == '\n')
		i++;
	if (str[i] == '-')
	{
		sign = -1;
		i++;
	}
	else if (str[i] == '+')
		i++;
	while (str[i] != '\0' && (str[i] >= '0' && str[i] <= '9'))
	{
		number = (number * 10) + (str[i] - '0');
		i++;
	}
	return (number * sign);
}
