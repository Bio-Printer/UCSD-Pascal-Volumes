/* guess the number: rand, scanf */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int secret;
    int g;
    int tries;
    srand(1984);
    secret = rand() % 100 + 1;
    printf("I am thinking of a number from 1 to 100.\n");
    tries = 0;
    for (;;) {
        printf("Your guess? ");
        if (scanf("%d", &g) != 1)
            break;
        tries++;
        if (g < secret)
            printf("%d is too low\n", g);
        else if (g > secret)
            printf("%d is too high\n", g);
        else {
            printf("%d is right, in %d tries\n", g, tries);
            break;
        }
    }
    return 0;
}
