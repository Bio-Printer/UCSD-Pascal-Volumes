/* Eight queens: count all solutions, print the first */
#include <stdio.h>

int col[8];
int solutions;
int first[8];

int safe(int row, int c)
{
    int r;
    for (r = 0; r < row; r++) {
        int d;
        d = col[r] - c;
        if (d == 0 || d == row - r || d == r - row)
            return 0;
    }
    return 1;
}

void place(int row)
{
    int c;
    int i;
    if (row == 8) {
        if (solutions == 0)
            for (i = 0; i < 8; i++)
                first[i] = col[i];
        solutions++;
        return;
    }
    for (c = 0; c < 8; c++)
        if (safe(row, c)) {
            col[row] = c;
            place(row + 1);
        }
}

int main(void)
{
    int r;
    int c;
    place(0);
    printf("%d solutions\n", solutions);
    for (r = 0; r < 8; r++) {
        for (c = 0; c < 8; c++)
            putchar(first[r] == c ? 'Q' : '.');
        putchar('\n');
    }
    return 0;
}
