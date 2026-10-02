/* Towers of Hanoi, recursion */
#include <stdio.h>

int moves;

void hanoi(int n, char from, char to, char via)
{
    if (n == 0)
        return;
    hanoi(n - 1, from, via, to);
    moves++;
    if (n >= 5)
        printf("move disk %d from %c to %c\n", n, from, to);
    hanoi(n - 1, via, to, from);
}

int main(void)
{
    hanoi(7, 'A', 'C', 'B');
    printf("%d moves\n", moves);
    return 0;
}
