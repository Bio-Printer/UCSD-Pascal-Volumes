/* assert.h -- Tiny-C */
#include <stdio.h>
#include <stdlib.h>
#undef assert
#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#define assert(e) ((e) ? (void)0 : __assertfail(#e, __FILE__, __LINE__))
#ifndef __ASSERT_H
#define __ASSERT_H
void __assertfail(char *e, char *file, int line);
#endif
#endif
