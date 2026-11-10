/* conio.c -- UCSD-C library: the code behind <conio.h> */
#include "libint.h"
#include <psys.h>
int getch(void)
{
    return __conrawgetc();
}

int getche(void)
{
    int c;
    char b;
    c = __conrawgetc();
    b = c;
    __cspv(6, 1, &b, 0, 1, 0, 0);
    return c;
}

int putch(int c)
{
    char b;
    __conflush();
    b = c;
    __cspv(6, 1, &b, 0, 1, 0, 0);
    return c;
}

int cputs(char *s)
{
    while (*s)
        putch(*s++);
    return 0;
}

/* a key waiting?  UNITBUSY (CSP 35) says so on a machine whose BIOS knows; the native P-Code
   engine (the emulator, the ucsd-pico firmware) answers it "no" always, and counts the keys
   waiting in SYSCOM->expansion[2] instead once expansion[1] is PX_KEYS_AHEAD (psys.h): the
   first kbhit() asks for that (it answers 0), later ones read the count.  (A program that set a
   key mode of its own, PX_KEYS ..., keeps it: kbhit() sees keys only in PX_KEYS_AHEAD's.)  (The keys Home, End,
   PgUp ... then come as one code each, KEY_HOME ...; exit() puts the keys back.) */
int __kbahead;

int kbhit(void)
{
    int e;
    __conflush();
    if (__cspi(35, 2) != 0)
        return 1;
    e = SYSCOM->expansion[1];
    if (e < PX_KEYS || e > PX_AHEAD_OK) {   /* (0, or what pexec left: not a key mode a program set) */
        SYSCOM->expansion[1] = PX_KEYS_AHEAD;
        __kbahead = 1;
        return 0;
    }
    return e == PX_AHEAD_OK && SYSCOM->expansion[2] != 0;
}

void gotoxy(int x, int y)
{
    __conflush();
    __cxp0v(29, x, y);
}

void clrscr(void)
{
    __conflush();
    __cxp0v(37);
}

