/* switch, goto, do/while, break/continue, ?:, comma, logical ops */
#include <stdio.h>

char *name(int n)
{
    switch (n) {
    case 0: return "zero";
    case 1: return "one";
    case 2: return "two";
    case 3: return "three";
    case 4: return "four";
    case 10: return "ten";
    case -1: return "minus one";
    default: return "many";
    }
}

int sparse(int n)
{
    switch (n) {
    case 100: return 1;
    case 2000: return 2;
    case -30000: return 3;
    }
    return 0;
}

int main(void)
{
    int i;
    int j;
    int n;
    for (i = -1; i < 12; i++)
        printf("%d:%s ", i, name(i));
    printf("\n%d %d %d %d\n", sparse(100), sparse(2000), sparse(-30000), sparse(5));
    i = 0;
    do {
        i++;
        if (i == 3)
            continue;
        if (i > 6)
            break;
        printf("%d", i);
    } while (i < 100);
    printf("\n");
    n = 0;
again:
    n++;
    if (n < 5)
        goto again;
    printf("goto %d\n", n);
    for (i = 0, j = 10; i < j; i++, j--)
        ;
    printf("comma %d %d\n", i, j);
    printf("logic %d %d %d %d\n", 3 && 0, 3 || 0, !5, (i > 2) ? 7 : 8);
    return 0;
}
