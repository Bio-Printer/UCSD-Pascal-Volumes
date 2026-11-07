/* touch.c -- TOUCH [-acm] [-d DATE | -t STAMP | -r FILE] FILE ...: each FILE's date and
   time of day set to now (or the one given), as touch(1) does; a FILE that is not
   there is made (empty):

     -c          a FILE that is not there is not made
     -a, -m      ignored (a file has one date and time here)
     -d DATE     YYYY-MM-DD, YYYY-MM-DDTHH:MM or YYYY-MM-DDTHH:MM:SS
     -t STAMP    [[CC]YY]MMDDhhmm[.ss] (as touch -t; YY 69 and more: 19YY)
     -r FILE     that file's date and time
   Now is the PC's date and time (emulator 2.13, its PC date and time on).  Without
   the clock: today's date (the one the system started with) and no time of day.
   Years 1970 to 2069.  A FILE may have the wild cards of DIR (* = ?) after VOL:,
   #n: or a path: every file that matches (none is made).  The time is kept by the
   systems of OS source revision 1.16 (SYSTEM.C, SYSTEM.PASCAL): DIR shows it.
   Run it from the shell: TOUCH A.C, TOUCH -d 2026-11-07T12:00 *.OBJ,
   TOUCH -r A.C B.C. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <psys.h>
#include <dirs.h>

int nocreate;
int sdate;                              /* the date and time to give (sdate 0: now) */
long sstamp;

/* the DATEREC of a full date; -1 when it is out of range */
int daterec(int y, int m, int d)
{
    if (y < 1970 || y > 2069 || m < 1 || m > 12 || d < 1 || d > 31)
        return -1;
    return (y % 100) << 9 | d << 4 | m;
}

/* -d's YYYY-MM-DD[THH:MM[:SS]] */
int parsed(char *s)
{
    int y, mo, d, h, mi, se, n;
    char t;
    h = mi = se = 0;
    n = sscanf(s, "%d-%d-%d%c%d:%d:%d", &y, &mo, &d, &t, &h, &mi, &se);
    if (n != 3 && n != 6 && n != 7)
        return 0;
    if (n > 3 && t != 'T' && t != 't')
        return 0;
    if (h > 23 || mi > 59 || se > 59 || h < 0 || mi < 0 || se < 0 || (sdate = daterec(y, mo, d)) < 0)
        return 0;
    sstamp = n > 3 ? (long)h * 3600 + mi * 60 + se + 1 : 0;
    return 1;
}

/* the value of n digits at s */
int digits(char *s, int n)
{
    int v;
    v = 0;
    while (n-- > 0) {
        if (!isdigit(*s))
            return -1;
        v = v * 10 + *s++ - '0';
    }
    return v;
}

/* -t's [[CC]YY]MMDDhhmm[.ss] */
int parset(char *s)
{
    char *dot;
    int n, y, mo, d, h, mi, se;
    dot = strchr(s, '.');
    n = dot ? dot - s : strlen(s);
    se = 0;
    if (dot && (strlen(dot) != 3 || (se = digits(dot + 1, 2)) < 0 || se > 59))
        return 0;
    if (n != 8 && n != 10 && n != 12)
        return 0;
    if (n == 12) {
        y = digits(s, 4);
        s = s + 4;
    } else if (n == 10) {
        y = digits(s, 2);
        y = y < 0 ? -1 : (y >= 69 ? 1900 + y : 2000 + y);
        s = s + 2;
    } else {
        y = (today() >> 9) & 127;       /* this year */
        y = y < 70 ? 2000 + y : 1900 + y;
    }
    mo = digits(s, 2);
    d = digits(s + 2, 2);
    h = digits(s + 4, 2);
    mi = digits(s + 6, 2);
    if (y < 0 || h < 0 || h > 23 || mi < 0 || mi > 59 || (sdate = daterec(y, mo, d)) < 0)
        return 0;
    sstamp = (long)h * 3600 + mi * 60 + se + 1;
    return 1;
}

/* -r's file: its entry's date and time */
int parser(char *name)
{
    int *dir;
    char leaf[16];
    int u, b, i;
    dir = (int *)malloc(2048);
    if (!dir || !pathdir(name, dir, &u, &b, leaf) || (i = findname(dir, leaf)) == 0) {
        fprintf(stderr, "touch: %s: no such file\n", name);
        exit(1);
    }
    sdate = dir[13 * i + 12];
    sstamp = entstamp(dir + 13 * i);
    free(dir);
    return 1;
}

int touch(char *name)
{
    FILE *f;
    int d;
    long s;
    if (filetime(name) == 0) {          /* not there */
        if (nocreate)
            return 0;
        f = fopen(name, "w");           /* made: the system gives it now */
        if (!f || fclose(f) != 0) {
            fprintf(stderr, "touch: %s: cannot make it\n", name);
            return 1;
        }
        if (!sdate)
            return 0;
    }
    if (sdate) {
        d = sdate;
        s = sstamp;
    } else if (!pcnow(&d, &s)) {        /* no clock: today, no time of day */
        d = today();
        s = 0;
    }
    if (!setfiletime(name, d, s)) {
        fprintf(stderr, "touch: %s: cannot set its time (a disk error?)\n", name);
        return 1;
    }
    return 0;
}

void usage(void)
{
    fprintf(stderr, "use: TOUCH [-acm] [-d YYYY-MM-DD[THH:MM[:SS]] | -t [[CC]YY]MMDDhhmm[.ss] | -r FILE] FILE ...\n");
    exit(1);
}

int main(int argc, char **argv)
{
    int i, j, nf, st, r;
    char *a;
    char *v;
    char *files[40];
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
                if (a[j] == 'd' || a[j] == 't' || a[j] == 'r') {
                    v = a[j + 1] ? a + j + 1 : (i + 1 < argc ? argv[++i] : NULL);
                    if (!v)
                        usage();
                    if (!(a[j] == 'd' ? parsed(v) : a[j] == 't' ? parset(v) : parser(v))) {
                        fprintf(stderr, "touch: invalid date format '%s'\n", v);
                        return 1;
                    }
                    break;
                }
                if (a[j] == 'c')
                    nocreate = 1;
                else if (a[j] != 'a' && a[j] != 'm') {
                    fprintf(stderr, "touch: invalid option -- '%c'\n", a[j]);
                    usage();
                }
            }
        } else if (nf < 40)
            files[nf++] = a;
    }
    if (nf == 0) {
        fprintf(stderr, "touch: missing file operand\n");
        usage();
    }
    st = 0;
    for (i = 0; i < nf; i++) {
        r = forfiles("touch", files[i], touch);
        if (r > st)
            st = r;
    }
    return st;
}
