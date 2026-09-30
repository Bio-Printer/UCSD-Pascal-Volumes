/* io.c -- Tiny-C library: the code behind <io.h> */
#include "libint.h"
int open(char *name, int flags, ...)
{
    FILE *f;
    char *mode;
    int acc;
    acc = flags & 3;
    if (acc == O_RDONLY)
        mode = "rb";
    else if (flags & O_APPEND)
        mode = acc == O_RDWR ? "a+" : "a";
    else if (flags & O_TRUNC || (flags & O_CREAT && acc == O_WRONLY))
        mode = acc == O_RDWR ? "w+" : "w";
    else
        mode = "r+";
    f = fopen(name, mode);
    if (!f)
        return -1;
    return f - __files;
}

int creat(char *name, int mode)
{
    return open(name, O_WRONLY | O_CREAT | O_TRUNC);
}

int __fdok(int fd)
{
    return fd >= 0 && fd < FOPEN_MAX + 3 && __files[fd].flags != 0;
}

int read(int fd, void *buf, unsigned n)
{
    if (!__fdok(fd))
        return -1;
    if (fd == 0) {
        /* console: up to the end of a line */
        char *b;
        unsigned i;
        int c;
        b = buf;
        for (i = 0; i < n; i++) {
            c = fgetc(stdin);
            if (c == EOF)
                break;
            b[i] = c;
            if (c == '\n') {
                i++;
                break;
            }
        }
        return i;
    }
    return fread(buf, 1, n, &__files[fd]);
}

int write(int fd, void *buf, unsigned n)
{
    if (!__fdok(fd))
        return -1;
    return fwrite(buf, 1, n, &__files[fd]);
}

int close(int fd)
{
    if (fd < 3)
        return 0;
    if (!__fdok(fd))
        return -1;
    return fclose(&__files[fd]);
}

long lseek(int fd, long off, int whence)
{
    if (!__fdok(fd) || fseek(&__files[fd], off, whence))
        return -1L;
    return ftell(&__files[fd]);
}

long tell(int fd)
{
    return __fdok(fd) ? ftell(&__files[fd]) : -1L;
}

int eof(int fd)
{
    int c;
    if (!__fdok(fd))
        return -1;
    c = fgetc(&__files[fd]);
    if (c == EOF)
        return 1;
    ungetc(c, &__files[fd]);
    return 0;
}

long filelength(int fd)
{
    long here;
    long end;
    here = tell(fd);
    end = lseek(fd, 0L, SEEK_END);
    lseek(fd, here, SEEK_SET);
    return end;
}

int unlink(char *name)
{
    return remove(name);
}

