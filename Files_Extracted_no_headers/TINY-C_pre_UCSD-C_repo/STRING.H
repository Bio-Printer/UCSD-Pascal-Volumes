/* string.h -- Tiny-C */
#ifndef __STRING_H
#define __STRING_H
#include <stddef.h>

size_t strlen(char *s);

char *strcpy(char *d, char *s);

char *strncpy(char *d, char *s, size_t n);

char *strcat(char *d, char *s);

char *strncat(char *d, char *s, size_t n);

int strcmp(char *a, char *b);

int strncmp(char *a, char *b, size_t n);

char *strchr(char *s, int c);

char *strrchr(char *s, int c);

char *strstr(char *s, char *t);

void *memcpy(void *d, void *s, size_t n);

void *memmove(void *d, void *s, size_t n);

void *memset(void *d, int c, size_t n);

int memcmp(void *a, void *b, size_t n);

void *memchr(void *s, int c, size_t n);

size_t strspn(char *s, char *set);

size_t strcspn(char *s, char *set);

char *strpbrk(char *s, char *set);

extern char *__strtok;

char *strtok(char *s, char *delim);


char *strdup(char *s);

#endif
