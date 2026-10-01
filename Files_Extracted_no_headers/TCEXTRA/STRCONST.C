/* string literals kept in pointers and arrays, across segments and calls

   String literals are inline constants in the code (LPA): kept in pointer
   variables and arrays, passed to functions, copied and compared, used in
   segments that are loaded on call and released on return, at every depth of
   a recursion, and through a function pointer. Run in every memory layout of
   the emulator (in its Harvard layout they are copied out of the code into a
   constant pool when their segment is loaded). A pointer to a literal of a
   segment is only used while that segment is in memory, as C on the P-System
   requires. */
#include <stdio.h>
#include <string.h>

char *gname;                        /* a global that holds a literal of main's segment */
char gbuf[64];

void sortp(char **v, int n)        /* order an array of string pointers */
{
    int i, j;
    char *t;
    for (i = 0; i < n; i++)
        for (j = n - 1; j > i; j--)
            if (strcmp(v[j - 1], v[j]) > 0) {
                t = v[j];
                v[j] = v[j - 1];
                v[j - 1] = t;
            }
}

#pragma segment SEGB
int depth(int n, char *acc)         /* a literal at every level of a recursion */
{
    char *mark = "<>";
    if (n == 0) return strlen(acc);
    strcat(acc, n % 2 ? "ab" : mark);
    return depth(n - 1, acc);
}

void fill(char *dst)                /* copy literals of this segment to the caller */
{
    char *a = "segment ";
    char *b = "B";
    strcpy(dst, a);
    strcat(dst, b);
}

char *hold;                         /* a literal of SEGB, used only while SEGB runs */

int inside(void)
{
    hold = "kept while SEGB is in memory";
    return strlen(hold) + (strcmp(hold, "kept while SEGB is in memory") == 0);
}

#pragma segment SEGC
int count(char *s, char c)          /* called through a pointer, compares with literals */
{
    int n = 0;
    while (*s) if (*s++ == c) n++;
    if (strcmp("x", "x") != 0) n = -1;
    return n;
}

#pragma segment MAIN
int main(void)
{
    char *p = "hello";
    char *q;
    char *names[5];
    char local[] = "local array";
    char *empty = "";
    char *longs = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcd";
    char acc[40];
    int (*f)(char *, char);
    int i;

    q = p;
    printf("%s %s %d\n", p, q, strlen(p));
    names[0] = "pear"; names[1] = "apple"; names[2] = "fig";
    names[3] = "banana"; names[4] = "cherry";
    sortp(names, 5);
    for (i = 0; i < 5; i++) printf("%s%s", names[i], i < 4 ? " " : "\n");
    gname = "global";
    printf("%s %d\n", gname, strcmp(gname, "global"));
    local[0] = 'L';
    printf("%s / %s\n", local, "local array");
    printf("[%s] %d\n", empty, strlen(empty));
    printf("%d %c%c\n", strlen(longs), longs[0], longs[199]);
    fill(gbuf);
    printf("%s\n", gbuf);
    acc[0] = 0;
    printf("%d %s\n", depth(6, acc), acc);
    printf("%d\n", inside());
    f = count;
    printf("%d %d\n", f("mississippi", 's'), f("banana", 'a'));
    printf("%s %s\n", p, gname);
    return 0;
}
