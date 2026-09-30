/* conio.h -- Tiny-C console I/O (unbuffered, P-System console) */
#ifndef __CONIO_H
#define __CONIO_H
#include <stdio.h>

int getch(void);

int getche(void);

int putch(int c);

int cputs(char *s);

/* a key is waiting (UNITBUSY on the console) */
int kbhit(void);

/* the operating system's FGOTOXY and CLEARSCREEN */
void gotoxy(int x, int y);

void clrscr(void);

#endif
