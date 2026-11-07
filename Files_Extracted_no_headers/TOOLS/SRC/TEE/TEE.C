/* tee.c -- TEE [-ai] [FILE ...]: the standard input to the standard output and to
   each FILE, as tee(1) copies it:

     -a  added to the end of each FILE (a FILE not there yet is made)
     -i  ignored (there are no interrupts to ignore)
   At most 6 FILEs (FOPEN_MAX: the most files a program has open besides the
   standard ones).  A FILE called .TEXT, .C, .H or .BAS is a text file.

   The P-System gives a new file the largest free area of its disk until it is
   closed, so a FILE cannot be on the disk where the output is going into a new file
   at the same time: >FILE, or the pipe's file (#4:PIPE1.TEXT) when TEE's output goes
   on through a pipe.  Run it from the shell: DIR | TEE #5:DIR.TEXT | GREP .C,
   CC A.C | TEE -a #5:LOG.TEXT (with the prefix not on #4). */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <string.h>

#define MAXOUT FOPEN_MAX

int main(int argc, char **argv)
{
    FILE *out[MAXOUT];
    char *name[MAXOUT];
    int i, j, n, c, append, status;
    char *a;
    append = 0;
    n = 0;
    status = 0;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (a[0] == '-' && a[1] == '-' && a[2] == 0) {
            i++;
            break;
        }
        if (a[0] != '-' || a[1] == 0)
            break;
        for (j = 1; a[j]; j++)
            if (a[j] == 'a')
                append = 1;
            else if (a[j] != 'i') {
                fprintf(stderr, "tee: invalid option -- '%c'\n", a[j]);
                fprintf(stderr, "use: TEE [-ai] [FILE ...]   (the standard input to the output and each FILE)\n");
                return 1;
            }
    }
    for (; i < argc; i++) {
        if (n == MAXOUT) {
            fprintf(stderr, "tee: %s: more than %d files\n", argv[i], MAXOUT);
            status = 1;
            continue;
        }
        out[n] = fopen(argv[i], append ? "a" : "w");
        if (!out[n] && append)
            out[n] = fopen(argv[i], "w");
        if (!out[n]) {
            fprintf(stderr, "tee: %s: cannot write it (no room on its disk, or a new file being\n"
                    "  written there already, as >FILE or a pipe's: the P-System gives that one\n"
                    "  the free area)\n", argv[i]);
            status = 1;
            continue;
        }
        name[n++] = argv[i];
    }
    while ((c = getchar()) != EOF) {
        putchar(c);
        for (j = 0; j < n; j++)
            putc(c, out[j]);
    }
    for (j = 0; j < n; j++)
        if (fclose(out[j]) != 0) {
            fprintf(stderr, "tee: %s: cannot write it (no room?)\n", name[j]);
            status = 1;
        }
    return status;
}
