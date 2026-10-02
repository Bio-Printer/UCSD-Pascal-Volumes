/* stddef.h -- Tiny-C */
#ifndef __STDDEF_H
#define __STDDEF_H
#ifndef NULL
#define NULL ((void *)0)
#endif
typedef unsigned size_t;
typedef int ptrdiff_t;
typedef char wchar_t;
#define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif
