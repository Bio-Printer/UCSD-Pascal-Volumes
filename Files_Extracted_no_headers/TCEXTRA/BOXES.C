/* conio.h: clear the screen, draw boxes with gotoxy (0-based, as the
   P-System's GOTOXY), wait for a key */
#include <conio.h>

void box(int x, int y, int w, int h)
{
    int i;
    gotoxy(x, y);
    putch('+');
    for (i = 0; i < w - 2; i++)
        putch('-');
    putch('+');
    for (i = 1; i < h - 1; i++) {
        gotoxy(x, y + i);
        putch('|');
        gotoxy(x + w - 1, y + i);
        putch('|');
    }
    gotoxy(x, y + h - 1);
    putch('+');
    for (i = 0; i < w - 2; i++)
        putch('-');
    putch('+');
}

int main(void)
{
    int c;
    clrscr();
    box(2, 1, 40, 9);
    box(6, 3, 14, 5);
    box(24, 3, 14, 5);
    gotoxy(9, 5);
    cputs("Tiny-C");
    gotoxy(27, 5);
    cputs("conio.h");
    gotoxy(2, 11);
    cputs("Press a key: ");
    c = getch();
    gotoxy(2, 12);
    cputs("You pressed ");
    putch(c);
    cputs("\r\n");
    return 0;
}
