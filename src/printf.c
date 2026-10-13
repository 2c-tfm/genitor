#include <genitor.h>
#include <syscall.h>
#include <string.h>

static const char *log_lvl_repr[] = {"SUCCESS", "EMERG", "ERROR", "WARN", 
	"INFO", "DEBUG"};
static const char *log_lvl_colors[] = {"\033[0;32m", "\033[41m", "\033[0;31m", 
	"\033[0;33m", "\033[0;34m", "\033[0;36m"};

void puts(const char *s) {
	write(1, s, strlen(s));
	write(1, "\n", 1);
}

void fputs(int fd, const char *s) {
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

uint32_t vprintf(int fd, const char *fmt, va_list args) {
	char *fmtit;
	uint32_t total = 0;


	fmtit = (char *) fmt;
	if (fmtit == NULL)
		return (0);

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

	return total;
}

uint32_t printf(const char *fmt, ...) {
	va_list args;
	uint32_t total = 0;


	if (fmt == NULL)
		return (0);

	va_start(args, fmt);
	total = vprintf(1, fmt, args);
	va_end(args);
	return (total);
}

uint32_t fprintf(int fd, const char *fmt, ...) {
	va_list args;
	uint32_t total = 0;


	if (fmt == NULL)
		return (0);

	va_start(args, fmt);
	total = vprintf(fd, fmt, args);
	va_end(args);
	return (total);
}

static void snputc(snprint_ctx *ctx, char c) {
	if (ctx->size > 0 && ctx->len < ctx->size - 1)
		ctx->buf[ctx->len] = c;

	ctx->len++;
}

static void snputs(snprint_ctx *ctx, const char *s) {
	if (s == NULL)
		s = "(null)";

	while (*s != '\0')
		snputc(ctx, *s++);
}

static void snputd(snprint_ctx *ctx, int n) {
	char tmp[sizeof(unsigned int) * CHAR_BIT];
	size_t i = 0;
	unsigned int value;

	if (n < 0) {
		snputc(ctx, '-');
		value = (unsigned int)(-(n + 1)) + 1U;
	} else {
		value = (unsigned int)n;
	}

	do {
		tmp[i++] = (char)('0' + value % 10U);
		value /= 10U;
	} while (value != 0);

	while (i > 0)
		snputc(ctx, tmp[--i]);
}

static void snputp(snprint_ctx *ctx, void *p) {
	static const char hex[] = "0123456789abcdef";
	char tmp[sizeof(uintptr_t) * 2];
	uintptr_t value = (uintptr_t)p;
	size_t i = 0;

	snputs(ctx, "0x");

	do {
		tmp[i++] = hex[value % 16U];
		value /= 16U;
	} while (value != 0);

	while (i > 0)
		snputc(ctx, tmp[--i]);
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
	snprint_ctx ctx = {
		.buf = buf,
		.size = size,
		.len = 0
	};

	if (size > 0 && buf == NULL)
		return -1;

	if (fmt == NULL) {
		if (size > 0)
			buf[0] = '\0';
		return 0;
	}

	while (*fmt != '\0') {
		if (*fmt != '%') {
			snputc(&ctx, *fmt++);
			continue;
		}

		fmt++;

		if (*fmt == '\0') {
			snputc(&ctx, '%');
			break;
		}

		switch (*fmt) {
		case 's':
			snputs(&ctx, va_arg(args, char *));
			break;

		case 'd':
			snputd(&ctx, va_arg(args, int));
			break;

		case 'p':
			snputp(&ctx, va_arg(args, void *));
			break;

		case '%':
			snputc(&ctx, '%');
			break;

		default:
			snputc(&ctx, '%');
			snputc(&ctx, *fmt);
			break;
		}

		fmt++;
	}

	if (size > 0)
		buf[ctx.len < size ? ctx.len : size - 1] = '\0';

	if (ctx.len > INT_MAX)
		return -1;

	return (int)ctx.len;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vsnprintf(buf, size, fmt, args);
	va_end(args);

	return ret;
}

uint32_t log(LOG_LVL log, char *fmt, ...) {
	va_list args;
	uint32_t total = 0;

	if (fmt == NULL)
		return (0);

#ifndef DBG
	if (log == DEBUG)
		return (0);
#endif

	va_start(args, fmt);
#ifdef LOG_COLOR
	printf("[%s%s\033[0m] - ", log_lvl_colors[log], log_lvl_repr[log]);
#else
	printf("[%s] - ", log_lvl_repr[log]);
#endif
	total = vprintf(1, fmt, args);
	puts("");
	va_end(args);
	if (log == EMERG)
		exit(-1);
	return (total);
}

