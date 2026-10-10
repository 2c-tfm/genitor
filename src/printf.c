#include <genitor.h>
#include <syscall.h>
#include <string.h>

void puts(int fd, const char *s) {
	write(fd, s, strlen(s));
	write(fd, "\n", 1);
}

static uint32_t print_ptr(int fd, void *p) {
	const char *hex = "0123456789abcdef";
	uint64_t pp;
	char buf[32];
	uint16_t i = 0;
	uint32_t total = 0;

	pp = (uint64_t) p;

	total += write(fd, "0x", 2);
	if (pp == 0) {
		write(fd, "0", 1);
	} else {
		while (pp > 0) {
			buf[i++] = hex[pp % 16];
			pp /= 16;
		}
	}

	total += i;
	while (i > 0) {
		write(fd, &buf[--i], 1);
	}
	return total;
}

static uint32_t print_str(int fd, const char *s) {
	uint32_t sit = 0;
	if (s == NULL)
		s = "(null)";
	while (s[sit] != '\0')
		sit++;
	write(fd, s, sit);
	return sit;
}

static uint32_t print_sint(int fd, int32_t n) {
	char buf[32];
	int32_t i = 0;
	int32_t total;

	if (n == 0) {
		write(fd, "0", 1);
		return 1;
	}
	if (n < 0) {
		write(fd, "-", 1);
		total++;
		if (n == INT32_MIN) {
			write(fd, "2147483648", 10);
			return total + 10;
		}
		n = -n;
	}
	while (n > 0) {
		buf[i++] = (n % 10) + '0';
		n /= 10;
	}
	total += i;
	while (i > 0) {
		write(fd, &buf[--i], 1);
	}
	return (total);
}

uint32_t printf(int fd, const char *fmt, ...) {
	va_list args;
	char *fmtit;
	uint32_t total = 0;


	fmtit = (char *) fmt;
	if (fmtit == NULL)
		return (0);

	va_start(args, fmt);

	while (*fmtit != '\0') {
		if (*fmtit != '%') {
			write(fd, fmtit, 1);
			total++;
			fmtit++;
			continue;
		}

		fmtit++;
		total++;

		if (*fmtit == '\0')
			break;
		else if (*fmtit == 's') 
			total += print_str(fd, va_arg(args, char *));
		else if (*fmtit == 'd')
			total += print_sint(fd, va_arg(args, int));
		else if (*fmtit == 'p')
			total += print_ptr(fd, va_arg(args, void *));
		else if (*fmtit == '%')
			total += write(fd, "%", 1);

		fmtit++;

	}
	va_end(args);
	return (total);
}
