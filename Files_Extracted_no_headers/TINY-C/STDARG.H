/* stdarg.h -- Tiny-C
 * A variadic function receives its extra arguments in a block built by
 * the caller (each argument word-aligned, char/short promoted to int; a
 * float stays 4 bytes, a double is 8); __va_start() returns the block's
 * address. */
#ifndef __STDARG_H
#define __STDARG_H
typedef char *va_list;
#define __va_size(t) ((sizeof(t) + 1) & ~1)
#define va_start(ap, last) ((ap) = __va_start())
#define va_arg(ap, t) (*(t *)(((ap) = (ap) + __va_size(t)) - __va_size(t)))
#define va_end(ap) ((void)0)
#define va_copy(d, s) ((d) = (s))
#endif
