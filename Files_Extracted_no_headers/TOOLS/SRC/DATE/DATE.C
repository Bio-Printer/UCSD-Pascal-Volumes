/* date.c -- DATE [-r FILE] [+FORMAT]: the date and time, as date(1) writes them in the
   C locale (there is no time zone here: none is written):

     DATE                Sat Nov  7 12:34:56 2026
     -r FILE             that file's date and time instead (DIR shows them too)
     +FORMAT             FORMAT with %Y (2026) %y (26) %m (11) %d (07) %e ( 7) %H (12)
                         %I (12) %M (34) %S (56) %p (PM) %a (Sat) %A (Saturday) %b %h
                         (Nov) %B (November) %j (311) %u (6) %w (6) %s (seconds since
                         1970) %D (11/07/26) %F (2026-11-07) %T (12:34:56) %R (12:34)
                         %n (a new line) %t (a tab) %% (%)
   The date and time are the PC's (emulator 2.13, its PC date and time on).  Without
   that clock the date is the one the system started with and the time is not
   known: the time's parts are left out (a FORMAT's are --:--).  The date cannot be
   set here: it is the PC's.  A two-digit year is 2000-2069 for 00-69, 1970-1999 for
   70-99.  Run it from the shell: DATE, DATE +%F, DATE -r A.C, DATE +%Y%m%d >D.TEXT. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psys.h>
#include <dirs.h>

char *wday[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
char *mon[] = { "January", "February", "March", "April", "May", "June", "July", "August",
                "September", "October", "November", "December" };
int mstart[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

int year, month, day, hour, minute, second, yday, dow;
int timeknown;
unsigned long secs;                     /* since 1970 (datesecs - 1) */

void set(int date, long stamp)
{
    month = date & 15;
    day = (date >> 4) & 31;
    year = (date >> 9) & 127;
    year = year < 70 ? 2000 + year : 1900 + year;
    timeknown = stamp > 0;
    if (stamp < 1)
        stamp = 1;
    hour = (int)((stamp - 1) / 3600);
    minute = (int)((stamp - 1) / 60 % 60);
    second = (int)((stamp - 1) % 60);
    yday = mstart[month - 1] + day + (month > 2 && year % 4 == 0);
    secs = datesecs(date, stamp) - 1;
    dow = (int)((secs / 86400 + 4) % 7);  /* 1970-01-01 was a Thursday */
}

void two(char *f, int v)
{
    printf(f, v);
}

void format(char *f)
{
    int c;
    for (; *f; f++) {
        if (*f != '%' || !f[1]) {
            putchar(*f);
            continue;
        }
        c = *++f;
        if (!timeknown && strchr("HIMSpTRs", c)) {
            fputs(c == 'T' ? "--:--:--" : c == 'p' ? "--" : c == 's' ? "-" : c == 'R' ? "--:--" : "--", stdout);
            continue;
        }
        switch (c) {
        case 'Y': printf("%d", year); break;
        case 'y': two("%02d", year % 100); break;
        case 'm': two("%02d", month); break;
        case 'd': two("%02d", day); break;
        case 'e': two("%2d", day); break;
        case 'H': two("%02d", hour); break;
        case 'I': two("%02d", hour % 12 == 0 ? 12 : hour % 12); break;
        case 'M': two("%02d", minute); break;
        case 'S': two("%02d", second); break;
        case 'p': fputs(hour < 12 ? "AM" : "PM", stdout); break;
        case 'a': printf("%.3s", wday[dow]); break;
        case 'A': fputs(wday[dow], stdout); break;
        case 'b': case 'h': printf("%.3s", mon[month - 1]); break;
        case 'B': fputs(mon[month - 1], stdout); break;
        case 'j': printf("%03d", yday); break;
        case 'u': printf("%d", dow == 0 ? 7 : dow); break;
        case 'w': printf("%d", dow); break;
        case 's': printf("%lu", secs); break;
        case 'D': printf("%02d/%02d/%02d", month, day, year % 100); break;
        case 'F': printf("%d-%02d-%02d", year, month, day); break;
        case 'T': printf("%02d:%02d:%02d", hour, minute, second); break;
        case 'R': printf("%02d:%02d", hour, minute); break;
        case 'n': putchar('\n'); break;
        case 't': putchar('\t'); break;
        case '%': putchar('%'); break;
        default: putchar('%'); putchar(c); break;
        }
    }
    putchar('\n');
}

int main(int argc, char **argv)
{
    int i, d;
    long s;
    char *fmt;
    int *dir;
    char leaf[16];
    int u, b, k;
    fmt = NULL;
    d = 0;
    s = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '+')
            fmt = argv[i] + 1;
        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            dir = (int *)malloc(2048);
            if (!dir || !pathdir(argv[++i], dir, &u, &b, leaf) || (k = findname(dir, leaf)) == 0) {
                fprintf(stderr, "date: %s: no such file\n", argv[i]);
                return 1;
            }
            d = dir[13 * k + 12];
            s = entstamp(dir + 13 * k);
            if ((d & 15) == 0) {
                fprintf(stderr, "date: %s: it has no date\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--set") == 0) {
            fprintf(stderr, "date: the date is the PC's: set it there\n");
            return 1;
        } else {
            fprintf(stderr, "date: invalid argument '%s'\n", argv[i]);
            fprintf(stderr, "use: DATE [-r FILE] [+FORMAT]\n");
            return 1;
        }
    }
    if (d == 0 && !pcnow(&d, &s)) {
        d = today();
        s = 0;
        if ((d & 15) == 0) {
            fprintf(stderr, "date: the date is not known\n");
            return 1;
        }
    }
    set(d, s);
    if (fmt)
        format(fmt);
    else if (timeknown)
        format("%a %b %e %H:%M:%S %Y");
    else
        format("%a %b %e %Y");
    return 0;
}
