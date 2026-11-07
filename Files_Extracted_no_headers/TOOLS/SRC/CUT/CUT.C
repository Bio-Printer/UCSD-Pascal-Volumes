/* cut.c -- CUT -b LIST | -c LIST | -f LIST [-d C] [-sn] [--complement]
   [--output-delimiter=S] [FILE ...]: parts of each line on the standard output, as
   cut(1) cuts them:

     -b LIST  these bytes             -c LIST  these characters (the same: one byte each)
     -f LIST  these fields: parts between the delimiters; a line without one is
              written whole (unless -s)
     -d C     the fields' delimiter (a tab when not given)
     -s       (-f) lines without the delimiter are not written
     -n       ignored
     --complement           everything but what LIST says
     --output-delimiter=S   S between the fields (or between the byte ranges)
   LIST: numbers from 1 and ranges, with commas between them: 3, 2-5, 4- (to the
   end), -3 (from the start); 1,4-6,9-.  What is written keeps the order of the line,
   each part once.  The shell takes no quotes: C may be an escape, \t a tab, \040 a
   blank, \\ a backslash (CUT -d\040 -f2).  No FILE or a FILE "-" is the standard
   input; a FILE may have the wild cards of DIR (* = ?) after VOL:, #n: or a path.
   .TEXT, .C, .H and .BAS files are text files.  Run it from the shell:
   CUT -c1-8 A.TEXT, DIR | CUT -c1-15, CUT -d: -f1,3 A.TEXT. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirs.h>

#define MAXLINE 1100
#define MAXPOS 1100                     /* positions in sel[]; beyond: openfrom */

char sel[MAXPOS + 2];                   /* sel[i]: position i is wanted */
int openfrom;                           /* every position from here on (0 none) */
int mode;                               /* 'b', 'c' or 'f' */
int delim = '\t';
char *odelim;                           /* --output-delimiter (NULL: none given) */
int onlydelim, complement;
char line[MAXLINE];

void usage(void)
{
    fprintf(stderr, "use: CUT -b LIST | -c LIST | -f LIST [-d C] [-s] [--complement] [FILE ...]\n");
    exit(1);
}

int wanted(int i)
{
    int w;
    w = i <= MAXPOS ? sel[i] : 0;
    if (openfrom && i >= openfrom)
        w = 1;
    return complement ? !w : w;
}

/* LIST into sel[] */
void list(char *s)
{
    long lo, hi;
    int i;
    char *e;
    while (*s) {
        lo = 1;
        hi = -1;
        if (*s == '-') {
            hi = strtol(s + 1, &e, 10);
            if (e == s + 1)
                goto bad;
        } else {
            lo = strtol(s, &e, 10);
            if (e == s || lo < 1)
                goto bad;
            hi = lo;
            if (*e == '-') {
                s = e + 1;
                if (*s == ',' || *s == 0) {
                    hi = -1;            /* N-: to the end */
                    e = s;
                } else {
                    hi = strtol(s, &e, 10);
                    if (e == s)
                        goto bad;
                }
            }
        }
        if (hi >= 0 && hi < lo) {
            fprintf(stderr, "cut: invalid decreasing range\n");
            exit(1);
        }
        if (hi < 0) {
            if (!openfrom || lo < openfrom)
                openfrom = (int)(lo > MAXPOS + 1 ? MAXPOS + 1 : lo);
        } else
            for (i = (int)lo; i <= hi && i <= MAXPOS; i++)
                sel[i] = 1;
        s = e;
        if (*s == ',')
            s++;
        else if (*s)
            goto bad;
    }
    return;
bad:
    fprintf(stderr, "cut: invalid field value or range: '%s'\n", s);
    exit(1);
}

/* one character: an escape or itself */
int onechar(char *s)
{
    int c, n;
    if (s[0] != '\\' || s[1] == 0)
        return s[0] & 255;
    s++;
    if (*s == 't')
        return '\t';
    if (*s >= '0' && *s <= '7') {
        c = 0;
        for (n = 0; n < 3 && *s >= '0' && *s <= '7'; n++)
            c = c * 8 + *s++ - '0';
        return c & 255;
    }
    return *s & 255;
}

void bytesof(char *s)
{
    int i, n, prev;
    n = strlen(s);
    prev = 0;
    for (i = 1; i <= n; i++)
        if (wanted(i)) {
            if (odelim && prev && prev != i - 1)
                fputs(odelim, stdout);
            putchar(s[i - 1]);
            prev = i;
        }
    putchar('\n');
}

void fieldsof(char *s)
{
    int f, first;
    char *e;
    if (strchr(s, delim) == NULL) {
        if (!onlydelim)
            printf("%s\n", s);
        return;
    }
    first = 1;
    for (f = 1;; f++) {
        e = strchr(s, delim);
        if (wanted(f)) {
            if (!first) {
                if (odelim)
                    fputs(odelim, stdout);
                else
                    putchar(delim);
            }
            first = 0;
            if (e)
                fwrite(s, 1, e - s, stdout);
            else
                fputs(s, stdout);
        }
        if (!e)
            break;
        s = e + 1;
    }
    putchar('\n');
}

int cut(char *name)
{
    FILE *f;
    int n;
    if (strcmp(name, "-") == 0)
        f = stdin;
    else if ((f = fopen(name, "r")) == NULL) {
        fprintf(stderr, "cut: %s: cannot open it\n", name);
        return 1;
    }
    while (fgets(line, MAXLINE, f)) {
        n = strlen(line);
        if (n > 0 && line[n - 1] == '\n')
            line[n - 1] = 0;
        if (mode == 'f')
            fieldsof(line);
        else
            bytesof(line);
    }
    if (f != stdin)
        fclose(f);
    return 0;
}

int main(int argc, char **argv)
{
    int i, j, nf, status, r, gotd;
    char *a;
    char *v;
    char *files[40];
    nf = 0;
    gotd = 0;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (strcmp(a, "--complement") == 0) {
            complement = 1;
            continue;
        }
        if (strncmp(a, "--output-delimiter=", 19) == 0) {
            odelim = a + 19;
            continue;
        }
        if (a[0] == '-' && a[1] == '-' && a[2] == 0) {
            for (i++; i < argc && nf < 40; i++)
                files[nf++] = argv[i];
            break;
        }
        if (a[0] == '-' && a[1]) {
            for (j = 1; a[j]; j++) {
                if (a[j] == 'b' || a[j] == 'c' || a[j] == 'f' || a[j] == 'd') {
                    v = a[j + 1] ? a + j + 1 : (i + 1 < argc ? argv[++i] : NULL);
                    if (!v)
                        usage();
                    if (a[j] == 'd') {
                        if (strlen(v) != 1 && v[0] != '\\') {
                            fprintf(stderr, "cut: the delimiter must be a single character\n");
                            return 1;
                        }
                        delim = onechar(v);
                        gotd = 1;
                    } else {
                        if (mode && mode != a[j]) {
                            fprintf(stderr, "cut: only one type of list may be specified\n");
                            return 1;
                        }
                        mode = a[j];
                        list(v);
                    }
                    break;
                }
                if (a[j] == 's')
                    onlydelim = 1;
                else if (a[j] != 'n') {
                    fprintf(stderr, "cut: invalid option -- '%c'\n", a[j]);
                    usage();
                }
            }
        } else if (nf < 40)
            files[nf++] = a;
    }
    if (!mode) {
        fprintf(stderr, "cut: you must specify a list of bytes, characters, or fields\n");
        usage();
    }
    if (mode != 'f' && (gotd || onlydelim)) {
        fprintf(stderr, "cut: %s only when operating on fields\n", gotd ? "an input delimiter may be specified" : "suppressing non-delimited lines makes sense");
        return 1;
    }
    if (nf == 0)
        files[nf++] = "-";
    status = 0;
    for (i = 0; i < nf; i++) {
        r = forfiles("cut", files[i], cut);
        if (r > status)
            status = r;
    }
    return status;
}
