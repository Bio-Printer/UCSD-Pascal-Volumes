/* io.h -- Tiny-C low-level file I/O: handles 0..2 are the console, the
   others are stdio streams underneath. */
#ifndef __IO_H
#define __IO_H
#include <stdio.h>
#include <fcntl.h>

int open(char *name, int flags, ...);

int creat(char *name, int mode);

int __fdok(int fd);

int read(int fd, void *buf, unsigned n);

int write(int fd, void *buf, unsigned n);

int close(int fd);

long lseek(int fd, long off, int whence);

long tell(int fd);

int eof(int fd);

long filelength(int fd);

int unlink(char *name);

#endif
