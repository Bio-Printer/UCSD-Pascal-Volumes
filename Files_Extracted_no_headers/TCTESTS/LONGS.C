/* 32-bit long arithmetic */
#include <stdio.h>

long fact(int n)
{
    return n <= 1 ? 1L : n * fact(n - 1);
}

int main(void)
{
    long a;
    long b;
    unsigned long u;
    int i;
    for (i = 1; i <= 12; i++)
        printf("%d! = %ld\n", i, fact(i));
    a = 123456789L;
    b = -98765L;
    printf("%ld %ld %ld %ld\n", a + b, a - b, a / b, a % b);
    printf("%ld %ld\n", a * 3, b * b);
    u = 4000000000UL;
    printf("%lu %lu %lx\n", u, u / 3, u >> 4);
    printf("%ld %ld\n", a << 2, -a >> 3);
    printf("%d %d %d\n", a > b, a < b, a == 123456789L);
    return 0;
}
