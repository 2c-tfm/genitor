#ifndef TYPES_H
#define TYPES_H

// #include <stdint.h>
// #include <stddef.h>
// #include <stdbool.h>
// #include <stdarg.h>

typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef short              int16_t;
typedef unsigned int       uint32_t;
typedef int                int32_t;
typedef unsigned long long uint64_t;
typedef unsigned long long size_t;
typedef long long          int64_t;
typedef long long          ssize_t;

typedef int			pid_t;
typedef uint64_t		off_t;
typedef unsigned long		uintptr_t;

#if !(defined(__STDC_VERSION__) && __STDC_VERSION__ > 202000L)
typedef _Bool              bool;
#endif
#define SIZE_MAX ((size_t)-1)
#ifndef true
#define true               1
#define false              0
#endif

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef INT32_MIN
#define INT32_MIN (-2147483647 - 1)
#endif

#endif

