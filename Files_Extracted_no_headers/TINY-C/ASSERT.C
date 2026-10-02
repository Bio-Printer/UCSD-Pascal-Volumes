/* assert.c -- Tiny-C library: the code behind <assert.h> */
#include "libint.h"
void __assertfail(char *e, char *file, int line)
{
    printf("Assertion failed: %s, file %s, line %d\n", e, file, line);
    exit(1);
}

