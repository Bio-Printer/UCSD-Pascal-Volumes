/* sort.c -- SORT [-bdfinrsuc] [-t C] [-k KEY ...] [-o FILE] [-S N] [-T VOL] [FILE ...]:
   the lines of the files, sorted, on the standard output, as sort(1) sorts them in
   the C locale (the bytes' order; the long options are not here):

     -b  leading blanks of a key do not count     -d  only blanks, letters, digits count
     -f  lower case as upper case                 -i  only printable characters count
     -n  a key's number: blanks, an optional -, digits, an optional . and digits
         (anything after it does not count; no number: 0)
     -r  backwards                                -s  stable: equal keys keep their order
     -u  of lines with equal keys only the first  -c  only checks the order: the first
                                                      line out of order said, status 1
     -t C   fields are separated by C (a tab \t, a blank \040: the shell takes no
            quotes); without -t a field is its leading blanks and what follows
     -k KEY a key: F[.C][OPTS][,F[.C][OPTS]], from field F (character C of it) to the
            end of the second field (character C of it); no second part: to the end
            of the line.  OPTS: b d f i n r, for this key only (a key with none takes
            the -b -d -f -i -n -r given).  Several -k: compared in turn.
     -o FILE the sorted lines to FILE (it may be one of the inputs)
     -S N   at most N bytes of memory for the lines (N may end in b, k or K, M)
     -T VOL the temporary files on VOL: (#n, a path), not on the prefix volume
   Lines with equal keys are put in the order of their bytes (unless -s or -u).
   No FILE or a FILE "-" is the standard input; a FILE may have the wild cards of
   DIR (* = ?) after VOL:, #n: or a path.  .TEXT, .C, .H and .BAS files are text files.

   What does not fit in memory is sorted a part at a time into temporary files on
   the prefix volume (SORTTMPn.TEXT; -T for another), which are then merged, four at
   a time, and removed.  The P-System gives a new file the largest free area of its
   disk until it is closed: with the output going into a new file (>FILE, a pipe)
   on the disk of the temporary files or of -o FILE, they cannot be made there --
   put them on another disk (-T, -o).
   Run it from the shell: SORT A.TEXT, DIR | SORT -k2n, SORT -t: -k3,3 -o B.TEXT A.TEXT. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirs.h>

#define MAXLINE 1100
#define MAXKEY 10
#define FANIN 4                         /* runs merged at once (FOPEN_MAX 6: an output, one spare) */
#define NOEND 32767                     /* a key to the end of the line */
#define RESERVE 7000                    /* bytes not taken for the lines */
#define MINBUF ((FANIN + 1) * MAXLINE)  /* merging's lines */

struct key {
    int sword, schar;                   /* from field sword (0 the first), character schar */
    int eword, echar;                   /* to field eword, character echar (0: its end); NOEND */
    int skipsb, skipeb;                 /* b on the start, on the end */
    int dict, fold, ignp, num, rev;
};

struct key keys[MAXKEY + 1];
int nkeys;
struct key gkey;                        /* the options given outside -k; the whole line */
int tab = -1;                           /* -t (-1: blanks) */
int unique, stable, check, reverse;
char *outname;
long maxbuf;                            /* -S (0: as much as there is) */
int status;

char *buf;                              /* the lines in memory, and their pointers from its end */
unsigned bufsize;
unsigned used;                          /* bytes of lines */
char **lines;                           /* lines[0..nlines-1]: at the end of buf, growing down */
int nlines;
char **tmp;

#define MAXRUNS 100
int nruns;                              /* SORTTMP1 .. SORTTMPn */
int order[MAXRUNS];                     /* the runs not merged yet, in the input's order */
int norder;
char line[MAXLINE];

void die(char *msg, char *arg)
{
    fprintf(stderr, "sort: %s%s\n", msg, arg);
    exit(2);
}

int blank(int c)
{
    return c == ' ' || c == '\t';
}

/* ---- the comparison (coreutils' begfield, limfield, keycompare) ---- */

char *begfield(char *p, char *lim, struct key *k)
{
    int w;
    w = k->sword;
    if (tab >= 0)
        while (p < lim && w--) {
            while (p < lim && (*p & 255) != tab)
                p++;
            if (p < lim)
                p++;
        }
    else
        while (p < lim && w--) {
            while (p < lim && blank(*p))
                p++;
            while (p < lim && !blank(*p))
                p++;
        }
    if (k->skipsb)
        while (p < lim && blank(*p))
            p++;
    return lim - p < k->schar ? lim : p + k->schar;
}

char *limfield(char *p, char *lim, struct key *k)
{
    int w, ec;
    if (k->eword == NOEND)
        return lim;
    w = k->eword;
    ec = k->echar;
    if (ec == 0)
        w++;                            /* to the end of field eword */
    if (tab >= 0)
        while (p < lim && w--) {
            while (p < lim && (*p & 255) != tab)
                p++;
            if (p < lim && (w || ec))
                p++;
        }
    else
        while (p < lim && w--) {
            while (p < lim && blank(*p))
                p++;
            while (p < lim && !blank(*p))
                p++;
        }
    if (ec) {
        if (k->skipeb)
            while (p < lim && blank(*p))
                p++;
        p = lim - p < ec ? lim : p + ec;
    }
    return p;
}

/* a number's parts: *sign -1, 0 (zero) or 1; the digits before the point (no
   leading zeros) at *ip, *in of them; after it at *fp, *fn (no trailing zeros) */
void numparts(char *p, char *lim, int *sign, char **ip, int *in, char **fp, int *fn)
{
    int neg;
    while (p < lim && blank(*p))
        p++;
    neg = p < lim && *p == '-';
    if (neg)
        p++;
    while (p < lim && *p == '0')
        p++;
    *ip = p;
    while (p < lim && isdigit(*p))
        p++;
    *in = p - *ip;
    *fp = p;
    *fn = 0;
    if (p < lim && *p == '.') {
        p++;
        *fp = p;
        while (p < lim && isdigit(*p))
            p++;
        while (p > *fp && p[-1] == '0')
            p--;
        *fn = p - *fp;
    }
    *sign = *in || *fn ? (neg ? -1 : 1) : 0;
}

int numcmp(char *a, char *la, char *b, char *lb)
{
    int sa, sb, na, nb, fa, fb, d, i;
    char *ia;
    char *ib;
    char *pa;
    char *pb;
    numparts(a, la, &sa, &ia, &na, &pa, &fa);
    numparts(b, lb, &sb, &ib, &nb, &pb, &fb);
    if (sa != sb)
        return sa < sb ? -1 : 1;
    if (sa == 0)
        return 0;
    if (na != nb)
        d = na < nb ? -1 : 1;
    else {
        d = memcmp(ia, ib, na);
        for (i = 0; d == 0 && (i < fa || i < fb); i++)
            d = (i < fa ? pa[i] : '0') - (i < fb ? pb[i] : '0');
    }
    if (d)
        d = d < 0 ? -1 : 1;
    return sa < 0 ? -d : d;
}

int skipped(struct key *k, int c)
{
    if (k->dict && !(isalnum(c) || blank(c)))
        return 1;
    return k->ignp && !isprint(c);
}

int bytecmp(char *a, int la, char *b, int lb)
{
    int d;
    d = memcmp(a, b, la < lb ? la : lb);
    if (d)
        return d < 0 ? -1 : 1;
    return la < lb ? -1 : la > lb;
}

int keycmp(char *a, char *b)
{
    struct key *k;
    char *ea;
    char *eb;
    char *ta;
    char *tb;
    char *la;
    char *lb;
    int i, d, ca, cb;
    ea = a + strlen(a);
    eb = b + strlen(b);
    for (i = 0; i < nkeys; i++) {
        k = &keys[i];
        ta = begfield(a, ea, k);
        tb = begfield(b, eb, k);
        la = limfield(a, ea, k);
        lb = limfield(b, eb, k);
        if (la < ta)
            la = ta;
        if (lb < tb)
            lb = tb;
        if (k->num)
            d = numcmp(ta, la, tb, lb);
        else if (k->dict || k->fold || k->ignp) {
            for (;;) {
                while (ta < la && skipped(k, *ta & 255))
                    ta++;
                while (tb < lb && skipped(k, *tb & 255))
                    tb++;
                if (!(ta < la && tb < lb)) {
                    d = (ta < la) - (tb < lb);
                    break;
                }
                ca = *ta & 255;
                cb = *tb & 255;
                if (k->fold) {
                    ca = toupper(ca);
                    cb = toupper(cb);
                }
                d = ca - cb;
                if (d)
                    break;
                ta++;
                tb++;
            }
        } else
            d = bytecmp(ta, la - ta, tb, lb - tb);
        if (d) {
            d = d < 0 ? -1 : 1;
            return k->rev ? -d : d;
        }
    }
    return 0;
}

int compare(char *a, char *b)
{
    int d;
    d = keycmp(a, b);
    if (d || unique || stable)
        return d;
    d = bytecmp(a, strlen(a), b, strlen(b));
    return reverse ? -d : d;
}

/* ---- the keys ---- */

/* OPTS letters at s into k; where they end */
char *keyopts(char *s, struct key *k, int end)
{
    for (;; s++) {
        switch (*s) {
        case 'b': if (end) k->skipeb = 1; else k->skipsb = 1; break;
        case 'd': k->dict = 1; break;
        case 'f': k->fold = 1; break;
        case 'i': k->ignp = 1; break;
        case 'n': k->num = 1; break;
        case 'r': k->rev = 1; break;
        default: return s;
        }
    }
}

int fieldnum(char **sp, char *spec)
{
    int n;
    char *s;
    s = *sp;
    if (!isdigit(*s))
        die("invalid number at field start: invalid count at start of ", spec);
    n = 0;
    while (isdigit(*s))
        n = n * 10 + *s++ - '0';
    *sp = s;
    return n;
}

void addkey(char *spec)
{
    struct key *k;
    char *s;
    int n;
    if (nkeys >= MAXKEY)
        die("too many keys: ", spec);
    k = &keys[nkeys++];
    memset(k, 0, sizeof *k);
    s = spec;
    n = fieldnum(&s, spec);
    if (n == 0)
        die("field number is zero: invalid field specification ", spec);
    k->sword = n - 1;
    if (*s == '.') {
        s++;
        n = fieldnum(&s, spec);
        if (n == 0)
            die("character offset is zero: invalid field specification ", spec);
        k->schar = n - 1;
    }
    s = keyopts(s, k, 0);
    k->eword = NOEND;
    if (*s == ',') {
        s++;
        n = fieldnum(&s, spec);
        if (n == 0)
            die("field number is zero: invalid field specification ", spec);
        k->eword = n - 1;
        if (*s == '.') {
            s++;
            k->echar = fieldnum(&s, spec);
        }
        s = keyopts(s, k, 1);
    }
    if (*s)
        die("invalid key specification ", spec);
}

/* a key with no options of its own takes the global ones */
void inherit(void)
{
    int i;
    struct key *k;
    for (i = 0; i < nkeys; i++) {
        k = &keys[i];
        if (k->skipsb || k->skipeb || k->dict || k->fold || k->ignp || k->num || k->rev)
            continue;
        k->skipsb = gkey.skipsb;
        k->skipeb = gkey.skipeb;
        k->dict = gkey.dict;
        k->fold = gkey.fold;
        k->ignp = gkey.ignp;
        k->num = gkey.num;
        k->rev = gkey.rev;
    }
    if (nkeys == 0) {                   /* the whole line, with the global options */
        keys[0] = gkey;
        keys[0].eword = NOEND;
        nkeys = 1;
    }
}

/* ---- memory, runs, merging ---- */

void msort(char **a, int n)
{
    int m, i, j, k;
    if (n < 2)
        return;
    m = n / 2;
    msort(a, m);
    msort(a + m, n - m);
    i = 0;
    j = m;
    k = 0;
    while (i < m && j < n)
        tmp[k++] = compare(a[j], a[i]) < 0 ? a[j++] : a[i++];     /* equal: the first stays first */
    while (i < m)
        tmp[k++] = a[i++];
    while (j < n)
        tmp[k++] = a[j++];
    memcpy(a, tmp, n * sizeof(char *));
}

char *tdir = "";                        /* -T: where the temporary files go (VOL:, #n:, A/) */

void runname(int r, char *s)
{
    sprintf(s, "%sSORTTMP%d.TEXT", tdir, r);
}

FILE *openout(char *name)
{
    FILE *f;
    if (!name)
        return stdout;
    f = fopen(name, "w");
    if (!f) {
        fprintf(stderr, "sort: cannot write %s (no room on its disk, or a new file being written there\n"
                "  already, as >FILE: the P-System gives that one the free area; -T VOL: puts the\n"
                "  temporary files on another disk)\n", name);
        exit(2);
    }
    return f;
}

void closeout(FILE *f, char *name)
{
    if (f != stdout && fclose(f) != 0)
        die("cannot write (no room?) ", name);
}

/* the lines in memory, sorted, to f (equal ones once with -u) */
void writelines(FILE *f)
{
    int i;
    char *t;
    for (i = 0; i < nlines / 2; i++) {  /* the pointers grew down: the input's order */
        t = lines[i];
        lines[i] = lines[nlines - 1 - i];
        lines[nlines - 1 - i] = t;
    }
    msort(lines, nlines);
    for (i = 0; i < nlines; i++)
        if (!unique || i == 0 || compare(lines[i - 1], lines[i]) != 0) {
            fputs(lines[i], f);
            putc('\n', f);
        }
}

/* memory full: the lines in memory sorted into the next run */
void spill(void)
{
    char name[48];
    FILE *f;
    if (norder >= MAXRUNS)
        die("too many temporary files: give -S more", "");
    runname(++nruns, name);
    order[norder++] = nruns;
    f = openout(name);
    writelines(f);
    closeout(f, name);
    nlines = 0;
    used = 0;
    lines = (char **)(buf + bufsize);
}

/* a line into memory */
void keep(char *s)
{
    unsigned n;
    n = strlen(s) + 1;
    if (used + n + 2 * sizeof(char *) * (nlines + 1) > bufsize)
        spill();
    if (used + n + 2 * sizeof(char *) * (nlines + 1) > bufsize)
        die("a line is too long for the memory", "");
    memcpy(buf + used, s, n);
    lines--;
    *lines = buf + used;
    used += n;
    nlines++;
    tmp = lines - nlines;               /* the merge sort's room, below the pointers */
}

/* a line of f into line[] without its new line; 0 at the end */
int getline(FILE *f)
{
    int n;
    if (!fgets(line, MAXLINE, f))
        return 0;
    n = strlen(line);
    if (n > 0 && line[n - 1] == '\n')
        line[n - 1] = 0;
    return 1;
}

char *prevline;                         /* -c: the line before */
long checkno;

int input(char *name)
{
    FILE *f;
    if (strcmp(name, "-") == 0)
        f = stdin;
    else if ((f = fopen(name, "r")) == NULL) {
        fprintf(stderr, "sort: %s: cannot open it\n", name);
        status = 2;
        return 2;
    }
    while (getline(f)) {
        if (check) {
            checkno++;
            if (prevline && compare(prevline, line) >= (unique ? 0 : 1)) {
                fprintf(stderr, "sort: %s:%ld: disorder: %s\n", name, checkno, line);
                exit(1);
            }
            if (!prevline)
                prevline = buf;
            strcpy(prevline, line);
            continue;
        }
        keep(line);
    }
    if (f != stdin)
        fclose(f);
    return 0;
}

/* the n runs ids[] (in the input's order: of equal lines the earlier run's first)
   merged into f */
void merge(int *ids, int n, FILE *f)
{
    FILE *in[FANIN];
    char *cur[FANIN];
    char *last;
    char name[48];
    int i, best, haslast;
    for (i = 0; i < n; i++) {
        runname(ids[i], name);
        in[i] = fopen(name, "r");
        if (!in[i])
            die("cannot read the temporary file ", name);
        cur[i] = buf + i * MAXLINE;     /* the lines' memory is free now */
        if (!fgets(cur[i], MAXLINE, in[i])) {
            cur[i][0] = 0;
            fclose(in[i]);
            in[i] = NULL;
        } else
            cur[i][strlen(cur[i]) - 1] = 0;
    }
    last = buf + FANIN * MAXLINE;
    haslast = 0;
    for (;;) {
        best = -1;
        for (i = 0; i < n; i++)
            if (in[i] && (best < 0 || compare(cur[i], cur[best]) < 0))
                best = i;               /* equal: the earlier run first */
        if (best < 0)
            break;
        if (!unique || !haslast || compare(last, cur[best]) != 0) {
            fputs(cur[best], f);
            putc('\n', f);
        }
        strcpy(last, cur[best]);
        haslast = 1;
        if (fgets(cur[best], MAXLINE, in[best]))
            cur[best][strlen(cur[best]) - 1] = 0;
        else {
            fclose(in[best]);
            in[best] = NULL;
        }
    }
    for (i = 0; i < n; i++) {
        runname(ids[i], name);
        remove(name);
    }
}

void usage(void)
{
    fprintf(stderr, "use: SORT [-bdfinrsuc] [-t C] [-k KEY ...] [-o FILE] [-S N] [-T VOL] [FILE ...]\n");
    exit(2);
}

/* -T VOL: (or #n, a path): what goes before the temporary files' names */
void tempdir(char *v)
{
    static char t[48];
    int n;
    n = strlen(v);
    if (n == 0 || n > 40)
        die("invalid temporary disk ", v);
    strcpy(t, v);
    if (t[n - 1] != ':' && t[n - 1] != '/')
        strcat(t, strchr(t, ':') ? "/" : ":");
    tdir = t;
}

/* -S's N with its suffix */
long size(char *s)
{
    long n;
    char *e;
    n = strtol(s, &e, 10);
    if (e == s || n <= 0)
        die("invalid buffer size ", s);
    if (*e == 'b')
        n *= 512;
    else if (*e == 'k' || *e == 'K')
        n *= 1024;
    else if (*e == 'M')
        n = 32000;
    else if (*e)
        die("invalid buffer size ", s);
    return n;
}

/* one character: an escape or itself */
int onechar(char *s)
{
    int c, n;
    if (s[0] != '\\' || s[1] == 0) {
        if (s[1])
            die("multi-character tab ", s);
        return s[0] & 255;
    }
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

int main(int argc, char **argv)
{
    int i, j, nf, r;
    char *a;
    char *v;
    unsigned avail;
    char *files[40];
    char name[48];
    FILE *f;
    nf = 0;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (a[0] == '-' && a[1] == '-' && a[2] == 0) {
            for (i++; i < argc && nf < 40; i++)
                files[nf++] = argv[i];
            break;
        }
        if (a[0] == '-' && a[1]) {
            for (j = 1; a[j]; j++) {
                if (a[j] == 'k' || a[j] == 't' || a[j] == 'o' || a[j] == 'S' || a[j] == 'T') {
                    v = a[j + 1] ? a + j + 1 : (i + 1 < argc ? argv[++i] : NULL);
                    if (!v)
                        usage();
                    if (a[j] == 'k')
                        addkey(v);
                    else if (a[j] == 't')
                        tab = onechar(v);
                    else if (a[j] == 'S')
                        maxbuf = size(v);
                    else if (a[j] == 'T')
                        tempdir(v);
                    else
                        outname = v;
                    break;
                }
                switch (a[j]) {
                case 'b': gkey.skipsb = gkey.skipeb = 1; break;
                case 'd': gkey.dict = 1; break;
                case 'f': gkey.fold = 1; break;
                case 'i': gkey.ignp = 1; break;
                case 'n': gkey.num = 1; break;
                case 'r': gkey.rev = 1; reverse = 1; break;
                case 's': stable = 1; break;
                case 'u': unique = 1; break;
                case 'c': check = 1; break;
                default:
                    fprintf(stderr, "sort: invalid option -- '%c'\n", a[j]);
                    usage();
                }
            }
        } else if (nf < 40)
            files[nf++] = a;
    }
    inherit();
    if (nf == 0)
        files[nf++] = "-";
    /* the memory for the lines: all but RESERVE bytes (left free for the files'
       buffers, the directory and the stack); merging uses it for its lines.
       (malloc gives a freed block back whole: nothing is freed to make room.) */
    avail = (unsigned)__cspi(40);       /* MEMAVAIL, words */
    bufsize = avail > (RESERVE + MINBUF) / 2 ? avail * 2 - RESERVE : 0;
    if (bufsize > 32000)
        bufsize = 32000;
    if (maxbuf && bufsize > maxbuf)
        bufsize = maxbuf < MINBUF ? MINBUF : (unsigned)maxbuf;
    bufsize = bufsize & ~1;
    if (bufsize < MINBUF || (buf = (char *)malloc(bufsize)) == NULL)
        die("no memory", "");
    lines = (char **)(buf + bufsize);
    for (i = 0; i < nf; i++) {
        r = forfiles("sort", files[i], input);
        if (r > status)
            status = r;
    }
    if (check)
        return status;
    if (nruns == 0) {
        f = openout(outname);
        writelines(f);
        closeout(f, outname);
        return status;
    }
    if (nlines)
        spill();
    while (norder > FANIN) {            /* the first FANIN runs into a new one, first in the order */
        runname(++nruns, name);
        f = openout(name);
        merge(order, FANIN, f);
        closeout(f, name);
        order[0] = nruns;
        norder = norder - FANIN + 1;
        memmove(order + 1, order + FANIN, (norder - 1) * sizeof(int));
    }
    f = openout(outname);
    merge(order, norder, f);
    closeout(f, outname);
    return status;
}
