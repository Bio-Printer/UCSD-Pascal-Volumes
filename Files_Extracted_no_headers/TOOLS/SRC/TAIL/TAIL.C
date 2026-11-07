/* tail.c -- TAIL [-n [+]N] [-c [+]N] [-N] [+N] [-qv] [FILE ...]: the last part of each
   file on the standard output, as tail(1) writes it (the long options are not here):

     -n N   the last N lines (10 when no option says)      -N  the same as -n N
     -n +N  from line N on (+1: the whole file)            +N  the same as -n +N
     -c N   the last N bytes          -c +N  from byte N on
     -q     no ==> NAME <== headers   -v     a header even for one file
   N may end in b (512), k or K (1024), kB (1000), M (1024 * 1024) or MB (1000000).
   With more than one file each has a header, ==> NAME <==, and an empty line
   comes before every header but the first.  No FILE or a FILE "-" is the standard
   input.  The last N lines are found by reading the input twice (no memory is
   needed for them): from a file, a pipe or <FILE, not from the console (+N reads
   once and works from it too).  -f (follow a growing file) is not possible here:
   one program runs at a time.

   .TEXT, .C, .H and .BAS files are text files (a line is what ends in a CR on the
   disk, the DLE blanks are put back); any other file is the bytes it holds, whole
   512-byte blocks.  A FILE may have the wild cards of DIR (* = ?) after VOL:, #n:
   or a path: TAIL *.C, TAIL -n 5 #5:*.TEXT, DIR | TAIL -3. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psys.h>
#include <dirs.h>

int bytes;                              /* -c */
int from;                               /* +N: from unit N on */
long count = 10;
int quiet, verbose;
int nfiles;                             /* the files the arguments name */
int shown;                              /* files shown so far */

/* N with its suffix; -1 when it is not a number */
long number(char *s)
{
    long n;
    if (*s < '0' || *s > '9')
        return -1;
    n = 0;
    while (*s >= '0' && *s <= '9')
        n = n * 10 + *s++ - '0';
    if (strcmp(s, "b") == 0)
        n *= 512;
    else if (strcmp(s, "k") == 0 || strcmp(s, "K") == 0)
        n *= 1024;
    else if (strcmp(s, "kB") == 0 || strcmp(s, "KB") == 0)
        n *= 1000;
    else if (strcmp(s, "M") == 0)
        n *= 1048576L;
    else if (strcmp(s, "MB") == 0)
        n *= 1000000L;
    else if (*s)
        return -1;
    return n;
}

/* the units in f (lines or bytes); a last line without its new line counts */
long total(FILE *f)
{
    long n;
    int c, last;
    n = 0;
    last = '\n';
    while ((c = getc(f)) != EOF) {
        if (bytes || c == '\n')
            n++;
        last = c;
    }
    if (!bytes && last != '\n')
        n++;
    return n;
}

int tail(char *name)
{
    FILE *f;
    long n, skip;
    int c;
    if (strcmp(name, "-") == 0)
        f = stdin;
    else if ((f = fopen(name, "r")) == NULL) {
        fprintf(stderr, "tail: %s: cannot open it\n", name);
        return 1;
    }
    if ((nfiles > 1 && !quiet) || verbose)
        printf("%s==> %s <==\n", shown ? "\n" : "", f == stdin ? "standard input" : name);
    shown++;
    if (from)
        skip = count > 0 ? count - 1 : 0;
    else {
        if (f->flags & __F_CON) {
            fprintf(stderr, "tail: the last %s are found by reading the input twice: not from the console\n",
                    bytes ? "bytes" : "lines");
            return 1;
        }
        skip = total(f) - count;
        if (skip < 0)
            skip = 0;
        rewind(f);
    }
    n = 0;
    while (n < skip && (c = getc(f)) != EOF)
        if (bytes || c == '\n')
            n++;
    while ((c = getc(f)) != EOF)
        putchar(c);
    if (f != stdin)
        fclose(f);
    return 0;
}

void usage(void)
{
    fprintf(stderr, "use: TAIL [-n [+]N] [-c [+]N] [-N] [+N] [-qv] [FILE ...]   (no FILE or -: the standard input)\n");
    exit(1);
}

/* the number after -n or -c (in the word or the next one) */
void numopt(char *s, int *ip, int argc, char **argv, int isbytes)
{
    if (!*s) {
        if (*ip + 1 >= argc)
            usage();
        s = argv[++*ip];
    }
    bytes = isbytes;
    from = *s == '+';
    count = number(*s == '+' || *s == '-' ? s + 1 : s);
    if (count < 0) {
        fprintf(stderr, "tail: invalid number of %s: '%s'\n", isbytes ? "bytes" : "lines", s);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    int i, j, first, status, r;
    char *a;
    char *files[40];
    int nf;
    nf = 0;
    first = 1;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (a[0] == '-' && a[1] == '-' && a[2] == 0) {
            for (i++; i < argc && nf < 40; i++)
                files[nf++] = argv[i];
            break;
        }
        if ((a[0] == '-' || a[0] == '+') && a[1] >= '0' && a[1] <= '9' && first) {
            bytes = 0;                  /* -N, +N: -n N, -n +N */
            from = a[0] == '+';
            count = number(a + 1);
            if (count < 0)
                usage();
        } else if (a[0] == '-' && a[1]) {
            for (j = 1; a[j]; j++) {
                if (a[j] == 'n' || a[j] == 'c') {
                    numopt(a + j + 1, &i, argc, argv, a[j] == 'c');
                    break;
                }
                if (a[j] == 'q') {
                    quiet = 1;
                    verbose = 0;
                } else if (a[j] == 'v') {
                    verbose = 1;
                    quiet = 0;
                } else if (a[j] == 'f' || a[j] == 'F') {
                    fprintf(stderr, "tail: -%c: not possible here (one program runs at a time)\n", a[j]);
                    return 1;
                } else {
                    fprintf(stderr, "tail: invalid option -- '%c'\n", a[j]);
                    usage();
                }
            }
        } else if (nf < 40)
            files[nf++] = a;
        first = 0;
    }
    if (nf == 0)
        files[nf++] = "-";
    for (i = 0; i < nf; i++)
        nfiles += forfiles("tail", files[i], NULL);
    if (nf > 1 && nfiles < 2)
        nfiles = 2;                     /* several arguments: headers, as tail(1) */
    status = 0;
    for (i = 0; i < nf; i++) {
        r = forfiles("tail", files[i], tail);
        if (r > status)
            status = r;
    }
    return status;
}
