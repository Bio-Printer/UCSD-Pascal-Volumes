/* Sieve of Eratosthenes: primes below 2000 */
#include <stdio.h>

#define N 2000
char flags[N];

int main(void)
{
    int i;
    int k;
    int count;
    count = 0;
    for (i = 2; i < N; i++)
        flags[i] = 1;
    for (i = 2; i < N; i++) {
        if (flags[i]) {
            count++;
            for (k = i + i; k < N; k += i)
                flags[k] = 0;
        }
    }
    printf("%d primes below %d\n", count, N);
    for (i = N - 1; i > 1900; i--)
        if (flags[i])
            printf("%d ", i);
    printf("\n");
    return 0;
}
