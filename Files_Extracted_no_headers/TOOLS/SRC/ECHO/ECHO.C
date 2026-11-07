/* echo.c -- ECHO [-neE] [ARG ...]: the ARGs, one blank between them, and a new line, on the
   standard output, as echo(1) (coreutils) writes them:

     -n  no new line at the end
     -e  the backslash escapes below are what they stand for
     -E  they are not (the default)
   An option is a word of n, e and E only (-ne); any other word, the first that is not
   an option and every word after it, is written.  With -e:
     \\ a backslash   \a bell   \b backspace   \c nothing more (not even the new line)
     \e escape   \f form feed   \n new line   \r return   \t tab   \v vertical tab
     \0NNN the character of octal NNN (up to 3 digits)   \xHH of hex HH (1 or 2 digits)
   The shell splits the line at blanks and takes no quotes: several blanks between two
   words become one; \040 (with -e) is a blank that stays.  Run it from the shell:
   ECHO hello >A.TEXT, ECHO -e a\tb, ECHO -n more >>A.TEXT. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <string.h>

int hexval(int c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

/* s with its escapes; 0 when \c ended the output */
int escaped(char *s)
{
    int c, n, v;
    while ((c = *s++) != 0) {
        if (c != '\\' || *s == 0) {
            putchar(c);
            continue;
        }
        c = *s++;
        switch (c) {
        case 'a': c = 7; break;
        case 'b': c = 8; break;
        case 'c': return 0;
        case 'e': c = 27; break;
        case 'f': c = 12; break;
        case 'n': c = '\n'; break;
        case 'r': c = '\r'; break;
        case 't': c = '\t'; break;
        case 'v': c = 11; break;
        case '\\': break;
        case '0':
            v = 0;
            for (n = 0; n < 3 && *s >= '0' && *s <= '7'; n++)
                v = v * 8 + *s++ - '0';
            c = v & 255;
            break;
        case 'x':
            if (hexval(*s) < 0) {       /* \x without a digit: as it is */
                putchar('\\');
                break;
            }
            v = hexval(*s++);
            if (hexval(*s) >= 0)
                v = v * 16 + hexval(*s++);
            c = v;
            break;
        default:                        /* not an escape: the backslash stays */
            putchar('\\');
            break;
        }
        putchar(c);
    }
    return 1;
}

int main(int argc, char **argv)
{
    int i, j, nl, esc, ok;
    char *a;
    nl = 1;
    esc = 0;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (a[0] != '-' || a[1] == 0 || strspn(a + 1, "neE") != strlen(a + 1))
            break;
        for (j = 1; a[j]; j++)
            if (a[j] == 'n')
                nl = 0;
            else if (a[j] == 'e')
                esc = 1;
            else
                esc = 0;
    }
    for (; i < argc; i++) {
        if (esc) {
            if (!escaped(argv[i]))
                return 0;
        } else
            fputs(argv[i], stdout);
        if (i + 1 < argc)
            putchar(' ');
    }
    if (nl)
        putchar('\n');
    return 0;
}
