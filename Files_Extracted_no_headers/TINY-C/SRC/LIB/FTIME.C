/* ftime.c -- Tiny-C library: files' dates and times of day (<dirs.h>)

   A directory entry's date is DACCESS (word 12, a DATEREC: month bits 0..3, day
   4..8, year 9..15: 00-69 the 2000s, 70-99 the 1900s).  Since the systems of OS
   source revision 1.16 (SYSTEM.C, SYSTEM.PASCAL; emulator 2.13) an entry also has
   the time of day the file was written: seconds since midnight + 1 (1..86400, 0
   none), its low 11 bits in word 2's bits 4..14 and its high 6 in word 11's bits
   10..15 (DLASTBYTE is word 11's low 10).  The PC's date and time come from TIME:
   both variables 25611, the time's two parts; both 25612, today's DATEREC (0, 0
   when the emulator's clock is off). */
#include "libint.h"
#include <psys.h>
#include <dirs.h>
#pragma segment FTIME

long entstamp(int *e)
{
    return ((long)((e[11] >> 10) & 63) << 11) | ((e[2] >> 4) & 2047);
}

void setentstamp(int *e, long s)
{
    e[2] = (e[2] & 0x800F) | (int)(s & 2047) << 4;      /* (bit 15: the Filer's STATUS) */
    e[11] = (e[11] & 1023) | (int)((s >> 11) & 63) << 10;
}

/* the PC's date (a DATEREC) and time (seconds since midnight + 1): 0 when there is
   no clock (both 0) */
int pcnow(int *date, long *stamp)
{
    int hi, lo;
    long s;
    hi = lo = PX_TIME_STAMP;
    __cspv(9, &hi, &lo);
    s = ((long)lo << 11) | (hi & 2047);
    hi = lo = PX_TIME_DATE;
    __cspv(9, &hi, &lo);
    *date = hi;
    hi = lo = PX_TIME_STAMP;
    __cspv(9, &hi, &lo);
    *stamp = ((long)lo << 11) | (hi & 2047);
    if (*stamp < s) {                   /* midnight came between: the new day's date */
        hi = lo = PX_TIME_DATE;
        __cspv(9, &hi, &lo);
        *date = hi;
    }
    return *date != 0;
}

/* today: the PC's date, else the boot disk's (the date the system started with) */
int today(void)
{
    int blk[256];
    int d;
    long s;
    if (pcnow(&d, &s))
        return d;
    __cspv(5, SYSCOM->sysunit, blk, 0, 512, 2, 0);      /* UNITREAD: DLASTBOOT */
    return __cspi(34) == 0 ? blk[10] : 0;
}

/* a date and time as one number to compare: seconds since 1970-01-01 00:00, + 1;
   0 for no date (a time 0: the day's start) */
unsigned long datesecs(int date, long stamp)
{
    static int mdays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int m, d, y;
    long days;
    m = date & 15;
    d = (date >> 4) & 31;
    y = (date >> 9) & 127;
    if (m < 1 || m > 12 || d < 1 || y >= 100)
        return 0;
    y = y < 70 ? 2000 + y : 1900 + y;
    days = (long)(y - 1970) * 365 + (y - 1969) / 4 + mdays[m - 1] + d - 1;
    if (m > 2 && y % 4 == 0)            /* (2000 is a leap year; 1900 and 2100 are out of range) */
        days++;
    return (unsigned long)days * 86400L + (stamp > 0 ? stamp - 1 : 0) + 1;
}

/* path's entry: its directory read into dir (2 KB), the unit and base in *up, *bp;
   its index, 0 when there is no such file */
static int fileentry(char *path, int *dir, int *up, int *bp)
{
    char leaf[16];
    if (!pathdir(path, dir, up, bp, leaf) || !leaf[0])
        return 0;
    return findname(dir, leaf);
}

/* a file's date and time (datesecs); 0 when there is no such file */
unsigned long filetime(char *path)
{
    int *dir;
    int i, u, b;
    unsigned long t;
    dir = (int *)malloc(2048);
    if (!dir)
        return 0;
    t = 0;
    if ((i = fileentry(path, dir, &u, &b)) != 0) {
        t = datesecs(dir[13 * i + 12], entstamp(dir + 13 * i));
        if (t == 0)
            t = 1;                      /* there, but undated: older than anything dated */
    }
    free(dir);
    return t;
}

/* a file's date and time := date, stamp (TOUCH): 0 when there is no such file or
   the directory cannot be written */
int setfiletime(char *path, int date, long stamp)
{
    int *dir;
    int i, u, b, ok;
    dir = (int *)malloc(2048);
    if (!dir)
        return 0;
    ok = 0;
    if ((i = fileentry(path, dir, &u, &b)) != 0) {
        dir[13 * i + 12] = date;
        setentstamp(dir + 13 * i, stamp);
        ok = dirwriteat(u, b, dir);
    }
    free(dir);
    return ok;
}
