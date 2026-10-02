/* files: stdio text files, then io.h / fcntl.h binary files */
#include <stdio.h>
#include <string.h>
#include <io.h>

int main(void)
{
    FILE *f;
    char line[80];
    char buf[64];
    int fd;
    int i;
    int n;
    f = fopen("FIO.TEXT", "w");
    if (!f) {
        printf("cannot create FIO.TEXT\n");
        return 1;
    }
    for (i = 1; i <= 5; i++)
        fprintf(f, "line %d: %d squared is %d\n", i, i, i * i);
    fclose(f);
    f = fopen("FIO.TEXT", "r");
    n = 0;
    while (fgets(line, 80, f)) {
        n++;
        printf("%s", line);
    }
    fclose(f);
    printf("%d lines read back\n", n);

    fd = open("FIO.DATA", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
    for (i = 0; i < 64; i++)
        buf[i] = i * 3;
    write(fd, buf, 64);
    write(fd, buf, 64);
    close(fd);
    fd = open("FIO.DATA", O_RDONLY | O_BINARY);
    lseek(fd, 70L, 0);
    n = read(fd, buf, 4);
    printf("read %d bytes at 70: %d %d %d %d\n", n, buf[0], buf[1], buf[2], buf[3]);
    close(fd);
    unlink("FIO.TEXT");
    unlink("FIO.DATA");
    printf("files removed\n");
    return 0;
}
