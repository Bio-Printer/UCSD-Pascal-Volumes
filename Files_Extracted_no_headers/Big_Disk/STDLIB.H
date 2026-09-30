/* stdlib.h -- Tiny-C
 * The heap is the P-machine heap (NEW); freed blocks are kept on a
 * first-fit free list.  exit() flushes and closes stdio files, then
 * leaves the program (EXIT from the startup procedure). */
#ifndef __STDLIB_H
#define __STDLIB_H
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 32767

typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;

/* heap block: one word holding the size in words (header included) */

void *malloc(size_t n);

void free(void *v);

void *calloc(size_t n, size_t size);

void *realloc(void *v, size_t n);

int abs(int x);

long labs(long x);

div_t div(int a, int b);


int rand(void);

void srand(unsigned seed);

long strtol(char *s, char **end, int base);

unsigned long strtoul(char *s, char **end, int base);

int atoi(char *s);

long atol(char *s);

float strtod(char *s, char **end);

float atof(char *s);

double strtold(char *s, char **end);   /* doubles: P-Code mode only */

double atold(char *s);

char *getenv(char *name);


void qsort(void *base, size_t n, size_t size, int (*cmp)(void *, void *));

void *bsearch(void *key, void *base, size_t n, size_t size, int (*cmp)(void *, void *));

/* release the whole heap back to a mark (all blocks allocated since are gone) */

void __heapsave(void);

void __heaprestore(void);


void exit(int status);

void abort(void);

#endif
