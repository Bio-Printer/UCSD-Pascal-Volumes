/* uniq.c -- UNIQ [-cdDiu] [-f N] [-s N] [-w N] [INPUT [OUTPUT]]: the lines of INPUT
   (no INPUT or "-": the standard input) with each run of the same line in a row
   written once, to OUTPUT (or the standard output), as uniq(1) does (the long
   options are not here):

     -c    each line with how many times it came, 7 wide, before it
     -d    only lines that came more than once (once each)
     -D    every line of each run of more than one
     -u    only lines that came once
     -i    upper and lower case the same
     -f N  the first N fields are not compared (a field: blanks, then non-blanks)
     -s N  then the first N characters are not compared either
     -w N  at most N characters are compared
   A run is of lines next to each other: SORT first to count every copy of a line
   (SORT A.TEXT | UNIQ -c).  .TEXT, .C, .H and .BAS files are text files. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLINE 1100

int optc, optd, optD, optu, opti;
int skipf, skipc;
int width = -1;                         /* -w: -1 the whole line */
char la[MAXLINE], lb[MAXLINE];
FILE *out;

/* a line of f into s without its new line; 0 at the end */
int getline(FILE *f, char *s)
{
    int n;
    if (!fgets(s, MAXLINE, f))
        return 0;
    n = strlen(s);
    if (n > 0 && s[n - 1] == '\n')
        s[n - 1] = 0;
    return 1;
}

/* where the compared part of s starts */
char *key(char *s)
{
    int i;
    for (i = 0; i < skipf; i++) {
        while (*s == ' ' || *s == '\t')
            s++;
        while (*s && *s != ' ' && *s != '\t')
            s++;
    }
    for (i = 0; i < skipc && *s; i++)
        s++;
    return s;
}

int same(char *a, char *b)
{
    int n, ca, cb;
    a = key(a);
    b = key(b);
    for (n = 0; width < 0 || n < width; n++) {
        ca = *a++ & 255;
        cb = *b++ & 255;
        if (opti) {
            ca = toupper(ca);
            cb = toupper(cb);
        }
        if (ca != cb)
            return 0;
        if (ca == 0)
            return 1;
    }
    return 1;
}

void put(char *s, long n)
{
    if (optc)
        fprintf(out, "%7ld %s\n", n, s);
    else
        fprintf(out, "%s\n", s);
}

/* the end of a run of n lines, its first s */
void run(char *s, long n)
{
    if (optD)
        return;                         /* written as they came */
    if ((optd && n < 2) || (optu && n > 1))
        return;
    put(s, n);
}

/* a number argument */
int num(char *s, char *what)
{
    char *e;
    long n;
    n = strtol(s, &e, 10);
    if (*s == 0 || *e || n < 0 || n > 32767) {
        fprintf(stderr, "uniq: %s: invalid number of %s\n", s, what);
        exit(1);
    }
    return (int)n;
}

int main(int argc, char **argv)
{
    int i, j, nf;
    char *a;
    char *v;
    char *files[2];
    char *prev;
    char *cur;
    char *t;
    long n;
    FILE *in;
    nf = 0;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (a[0] == '-' && a[1]) {
            for (j = 1; a[j]; j++) {
                if (a[j] == 'f' || a[j] == 's' || a[j] == 'w') {
                    v = a[j + 1] ? a + j + 1 : (i + 1 < argc ? argv[++i] : "");
                    if (a[j] == 'f')
                        skipf = num(v, "fields to skip");
                    else if (a[j] == 's')
                        skipc = num(v, "bytes to skip");
                    else
                        width = num(v, "bytes to compare");
                    break;
                }
                switch (a[j]) {
                case 'c': optc = 1; break;
                case 'd': optd = 1; break;
                case 'D': optD = 1; break;
                case 'u': optu = 1; break;
                case 'i': opti = 1; break;
                default:
                    fprintf(stderr, "uniq: invalid option -- '%c'\n", a[j]);
                    fprintf(stderr, "use: UNIQ [-cdDiu] [-f N] [-s N] [-w N] [INPUT [OUTPUT]]\n");
                    return 1;
                }
            }
        } else if (nf < 2)
            files[nf++] = a;
        else {
            fprintf(stderr, "uniq: extra operand '%s'\n", a);
            return 1;
        }
    }
    if (optc && optD) {
        fprintf(stderr, "uniq: printing all duplicated lines and repeat counts is meaningless\n");
        return 1;
    }
    in = stdin;
    if (nf > 0 && strcmp(files[0], "-") != 0 && (in = fopen(files[0], "r")) == NULL) {
        fprintf(stderr, "uniq: %s: cannot open it\n", files[0]);
        return 1;
    }
    out = stdout;
    if (nf > 1 && (out = fopen(files[1], "w")) == NULL) {
        fprintf(stderr, "uniq: %s: cannot write it (no room on its disk, or a new file being\n"
                "  written there already, as >FILE: the P-System gives that one the free area)\n", files[1]);
        return 1;
    }
    prev = la;
    cur = lb;
    if (!getline(in, prev))
        return 0;
    n = 1;
    while (getline(in, cur)) {
        if (same(prev, cur)) {
            if (optD) {
                if (n == 1)
                    put(prev, 1);
                put(cur, 1);
            }
            n++;
            continue;
        }
        run(prev, n);
        t = prev;
        prev = cur;
        cur = t;
        n = 1;
    }
    run(prev, n);
    if (out != stdout)
        fclose(out);
    return 0;
}
