/* tr.c -- TR [-cdst] SET1 [SET2]: the standard input to the standard output with
   characters changed or deleted, as tr(1) does it (the long options are not here):

     TR SET1 SET2     each character of SET1 becomes the one at the same place in
                      SET2 (SET2's last one repeats when SET2 is shorter)
     -t               SET1 cut to SET2's length instead
     -d SET1          the characters of SET1 deleted
     -s SET1          a run of the same character of SET1 becomes one; with SET2
                      (-s SET1 SET2, -ds SET1 SET2) a run of one of SET2 after the
                      change or the deletion
     -c               SET1 is every character that is not in it (in order)
   A SET is characters, and:
     a-z              the range          \n \t \r \f \v \a \b \\  the escapes
     \NNN             octal NNN (1 to 3 digits): \040 is a blank
     [:alpha:] [:digit:] [:alnum:] [:upper:] [:lower:] [:space:] [:blank:]
     [:punct:] [:print:] [:graph:] [:cntrl:] [:xdigit:]       the classes
     [c*N]            c N times (N octal when it starts with 0)
     [c*]             (SET2) c as often as makes it as long as SET1
     [=c=]            c
   The shell splits the line at blanks and takes no quotes: \040 is a blank.  Run
   it from the shell: TR a-z A-Z <A.TEXT, TR -d \r, TR -s \040,
   CAT A.C | TR -cs [:alnum:] \n  (each word on a line). */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXSET 1024

unsigned char s1[MAXSET], s2[MAXSET];
int n1, n2;
int fill2 = -1;                         /* SET2's [c*]: where the fill goes */
int fillc;

char *classes[] = { "alnum", "alpha", "blank", "cntrl", "digit", "graph", "lower",
                    "print", "punct", "space", "upper", "xdigit", 0 };

int inclass(int k, int c)
{
    switch (k) {
    case 0: return isalnum(c);
    case 1: return isalpha(c);
    case 2: return c == ' ' || c == '\t';
    case 3: return iscntrl(c);
    case 4: return isdigit(c);
    case 5: return isgraph(c);
    case 6: return islower(c);
    case 7: return isprint(c);
    case 8: return ispunct(c);
    case 9: return isspace(c);
    case 10: return isupper(c);
    }
    return isxdigit(c);
}

void fail(char *msg, char *arg)
{
    fprintf(stderr, "tr: %s%s\n", msg, arg);
    exit(1);
}

/* one character of a set at *pp: an escape or itself */
int onechar(char **pp)
{
    char *p;
    int c, n;
    p = *pp;
    c = *p++ & 255;
    if (c == '\\' && *p) {
        c = *p++ & 255;
        switch (c) {
        case 'a': c = 7; break;
        case 'b': c = 8; break;
        case 'f': c = 12; break;
        case 'n': c = '\n'; break;
        case 'r': c = '\r'; break;
        case 't': c = '\t'; break;
        case 'v': c = 11; break;
        default:
            if (c >= '0' && c <= '7') {
                c = c - '0';
                for (n = 1; n < 3 && *p >= '0' && *p <= '7'; n++)
                    c = c * 8 + *p++ - '0';
                c = c & 255;
            }
        }
    }
    *pp = p;
    return c;
}

void add(unsigned char *s, int *n, int c)
{
    if (*n >= MAXSET)
        fail("a set is too long (", "1024 characters at most)");
    s[(*n)++] = c;
}

/* a set's text into s (n characters); two: SET2 ([c*] allowed) */
void parse(char *p, unsigned char *s, int *n, int two)
{
    int c, d, k, len, rep;
    char *q;
    char *e;
    while (*p) {
        if (p[0] == '[' && p[1] == ':' && (q = strstr(p + 2, ":]")) != NULL) {
            len = q - (p + 2);
            for (k = 0; classes[k]; k++)
                if ((int)strlen(classes[k]) == len && strncmp(classes[k], p + 2, len) == 0)
                    break;
            if (!classes[k])
                fail("invalid character class: ", p);
            for (c = 0; c < 256; c++)
                if (inclass(k, c))
                    add(s, n, c);
            p = q + 2;
            continue;
        }
        if (p[0] == '[' && p[1] == '=' && p[2] && p[3] == '=' && p[4] == ']') {
            add(s, n, p[2] & 255);
            p = p + 5;
            continue;
        }
        if (p[0] == '[' && p[1] && p[1] != '\\' && p[2] == '*' && strchr(p + 3, ']')) {
            c = p[1] & 255;
            q = p + 3;
            if (*q == ']') {            /* [c*]: fills SET2 */
                if (!two)
                    fail("the [c*] repeat is only in SET2: ", p);
                fill2 = *n;
                fillc = c;
                p = q + 1;
                continue;
            }
            rep = (int)strtol(q, &e, *q == '0' ? 8 : 10);
            if (*e != ']')
                fail("invalid repeat count in ", p);
            if (rep == 0) {             /* [c*0]: the same as [c*] */
                if (!two)
                    fail("the [c*] repeat is only in SET2: ", p);
                fill2 = *n;
                fillc = c;
            }
            while (rep-- > 0)
                add(s, n, c);
            p = e + 1;
            continue;
        }
        c = onechar(&p);
        if (*p == '-' && p[1]) {        /* a range */
            p++;
            d = onechar(&p);
            if (d < c)
                fail("range-endpoints are in reverse order: ", "");
            for (; c <= d; c++)
                add(s, n, c);
            continue;
        }
        add(s, n, c);
    }
}

int main(int argc, char **argv)
{
    int i, j, c, last, sets;
    int optc, optd, opts, optt;
    unsigned char in1[256], sq[256], map[256];
    unsigned char t[MAXSET];
    int nt;
    char *set[2];
    char *a;
    optc = optd = opts = optt = 0;
    sets = 0;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (a[0] == '-' && a[1] && sets == 0) {
            if (a[1] == '-' && a[2] == 0) {
                i++;
                break;
            }
            for (j = 1; a[j]; j++)
                switch (a[j]) {
                case 'c': case 'C': optc = 1; break;
                case 'd': optd = 1; break;
                case 's': opts = 1; break;
                case 't': optt = 1; break;
                default:
                    fprintf(stderr, "tr: invalid option -- '%c'\n", a[j]);
                    fprintf(stderr, "use: TR [-cdst] SET1 [SET2]   (the standard input to the standard output)\n");
                    return 1;
                }
            continue;
        }
        break;
    }
    for (; i < argc; i++) {
        if (sets == 2)
            fail("extra operand ", argv[i]);
        set[sets++] = argv[i];
    }
    if (sets == 0)
        fail("missing operand", "");
    if (optd && !opts && sets == 2)
        fail("extra operand ", set[1]);
    if (!optd && !opts && sets < 2)
        fail("missing operand after ", set[0]);
    if (optd && opts && sets < 2)
        fail("missing operand after ", set[0]);
    parse(set[0], s1, &n1, 0);
    if (fill2 >= 0)
        fail("the [c*] repeat is only in SET2", "");
    if (sets == 2)
        parse(set[1], s2, &n2, 1);
    memset(in1, 0, sizeof in1);
    for (i = 0; i < n1; i++)
        in1[s1[i]] = 1;
    if (optc) {                         /* every other character, in order */
        n1 = 0;
        for (c = 0; c < 256; c++)
            if (!in1[c])
                s1[n1++] = c;
        memset(in1, 0, sizeof in1);
        for (i = 0; i < n1; i++)
            in1[s1[i]] = 1;
    }
    if (sets == 2 && fill2 >= 0) {      /* [c*]: SET2 as long as SET1 */
        nt = 0;
        for (i = 0; i < fill2; i++)
            t[nt++] = s2[i];
        for (j = n1 - n2; j > 0 && nt < MAXSET; j--)
            t[nt++] = fillc;
        for (i = fill2; i < n2 && nt < MAXSET; i++)
            t[nt++] = s2[i];
        memcpy(s2, t, nt);
        n2 = nt;
    }
    for (c = 0; c < 256; c++)
        map[c] = c;
    if (!optd && sets == 2) {           /* the translation */
        if (n2 == 0 && n1 > 0)
            fail("when not truncating set1, string2 must be non-empty", "");
        if (optt && n1 > n2)
            n1 = n2;
        for (i = 0; i < n1; i++)
            map[s1[i]] = s2[i < n2 ? i : n2 - 1];
    }
    memset(sq, 0, sizeof sq);
    if (opts) {                         /* the squeezed set */
        if (sets == 2)
            for (i = 0; i < n2; i++)
                sq[s2[i]] = 1;
        else
            for (i = 0; i < n1; i++)
                sq[s1[i]] = 1;
    }
    last = -1;
    while ((c = getchar()) != EOF) {
        c = c & 255;
        if (optd && in1[c])
            continue;
        c = map[c];
        if (opts && sq[c] && c == last)
            continue;
        putchar(c);
        last = c;
    }
    return 0;
}
