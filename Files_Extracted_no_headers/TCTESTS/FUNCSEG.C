/* function pointers across segments: CALLI to a function in another segment
   loads that segment (and releases it on return); a call through a pointer
   from one such segment into another and back; recursion through a pointer */
#include <stdio.h>

int plus1(int x) { return x + 1; }
int (*sp)(int);

#pragma segment SEGB
int times2(int x) { return x * 2; }
int minus5(int x) { return x - 5; }
int viaB(int (*f)(int), int x) { return f(x) + 1; }

#pragma segment SEGC
int times100(int x) { return x * 100; }
int callp1(int (*f)(int), int x) { return viaB(f, x) + f(x); }

#pragma segment SEGD
int sumto(int n)
{
    return n <= 0 ? 0 : n + sp(n - 1);
}

int main(void)
{
    int (*tab[4])(int);
    int (*g)(int (*)(int), int);
    int i;
    int s;
    tab[0] = plus1;
    tab[1] = times2;
    tab[2] = minus5;
    tab[3] = times100;
    for (i = 0; i < 4; i++)
        printf("tab[%d](7) = %d\n", i, tab[i](7));
    printf("viaB(plus1, 10) = %d\n", viaB(plus1, 10));
    printf("viaB(times100, 3) = %d\n", viaB(times100, 3));
    printf("callp1(times2, 21) = %d\n", callp1(times2, 21));
    g = viaB;
    printf("g(minus5, 9) = %d\n", g(minus5, 9));
    s = 0;
    for (i = 0; i < 20; i++)
        s += tab[i % 4](i);
    printf("sum = %d\n", s);
    sp = sumto;
    printf("sumto(30) = %d\n", sp(30));
    return 0;
}
