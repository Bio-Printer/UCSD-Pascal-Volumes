/* demo.c -- the smallest program: no stdio.h (much less to link);
   console output through UNITWRITE, recursion, arrays, structs, longs */

void puts1(char *s)
{
    int n;
    n = 0;
    while (s[n])
        n++;
    __cspv(6, 1, s, 0, n, 0, 0);           /* UNITWRITE(1, s, n) */
}

void putnum(long v)
{
    char buf[12];
    int i;
    int neg;
    neg = v < 0;
    if (neg)
        v = -v;
    i = 11;
    buf[i] = 0;
    do {
        buf[--i] = '0' + (int)(v % 10);
        v = v / 10;
    } while (v != 0);
    if (neg)
        buf[--i] = '-';
    puts1(buf + i);
}

struct point { int x; int y; };

int fib(int n)
{
    return n < 2 ? n : fib(n - 1) + fib(n - 2);
}

int main()
{
    struct point p;
    int i;
    long f;
    puts1("Tiny-C demo\r");
    for (i = 0; i <= 10; i++) {
        putnum(fib(i));
        puts1(" ");
    }
    puts1("\r");
    f = 1;
    for (i = 1; i <= 12; i++)
        f = f * i;
    puts1("12! = ");
    putnum(f);
    puts1("\r");
    p.x = -7;
    p.y = p.x * p.x;
    putnum(p.x);
    puts1(" squared is ");
    putnum(p.y);
    puts1("\r");
    return 0;
}
