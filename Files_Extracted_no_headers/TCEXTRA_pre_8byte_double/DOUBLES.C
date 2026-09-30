/* doubles (12 bytes: IEEE binary64, CSP 100..; P-Code mode only) */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

char buf[48];
double ad[3];
unsigned long ul;
unsigned uu;
long ll;

void show(double d)
{
    __cspv(133, d, 'g', 17, buf);       /* DTOA: %.17g */
    printf("%s\n", buf);
}

double half(double x)
{
    return x / 2.0;
}

struct P {
    int k;
    double v;
};

void more(void)
{
    double d;
    double e;
    double f;
    struct P q;
    char *end;
    int n;
    float g;
    d = 1.5L;                           /* ++ and -- */
    e = d++;
    f = ++d;
    printf("%lg %lg %lg\n", d, e, f);
    e = d--;
    f = --d;
    printf("%lg %lg %lg\n", d, e, f);
    ad[1] = 2.25L;
    ad[1]++;
    ++ad[1];
    q.v = 0.5L;
    q.v--;
    --q.v;
    printf("%lg %lg\n", ad[1], q.v);
    d += 2;
    d -= 0.5;
    d *= 3;
    d /= 4;
    printf("%lg\n", d);
    ul = 4000000000UL;                  /* unsigned long and long */
    d = ul;
    printf("%.1lf\n", d);
    d = 3000000000.0L;
    ul = d;
    printf("%lu\n", ul);
    d = -2147483647L - 1;
    printf("%.1lf %lg %.1lf\n", d, (double)-100000L, (double)4000000000UL);
    uu = 50000;
    d = uu;
    d = d + 10000.7L;
    uu = d;
    ll = -123456L;
    d = ll;
    ll = d * 2;
    printf("%u %ld\n", uu, ll);
    n = sscanf("0.1 -2.5e-3 7.25", "%lf %Lf %f", &d, &e, &g);      /* text */
    printf("%d %.17lg %.17lg %g\n", n, d, e, g);
    d = strtold("  3.14159265358979323846xyz", &end);
    printf("%.17lg [%s] %lg\n", d, end, atold("1e300") * 10.0L);
    d = 37.34;                          /* %f %e %g of a double: l inserted */
    e = 987.32345;
    f = 13432.8888888888888;
    printf("result = %g\n", d + (e / f));
    printf("%f %e %10.3f|%-8.2G|%*.*f %d\n", d, e, f, d, 9, 2, e, 42);
    g = 2.5;
    printf("%f %lf %%\n", g, g);
    sscanf("1.25 7", "%f %*d", &d);
    printf("%.3f\n", d);
}

int main(void)
{
    double a;
    double b;
    triple t;
    float f;
    int i;
    long l;
    unsigned u;
    double arr[3];
    struct P p;
    a = 0.1;
    b = 0.2;
    show(a + b);
    show(a * 3.0);
    show(1.0L / 3.0);
    show(-a);
    t = 1e300L;
    show(t * 10.0);
    f = 0.1;
    a = f;
    show(a);
    f = a * 2.0;
    printf("%d\n", (int)(f * 10));
    i = 7;
    a = i;
    show(a / 2);
    i = a * 3.0;
    printf("%d\n", i);
    l = 100000L;
    a = l;
    show(a);
    l = a * 3.0;
    printf("%ld\n", l);
    u = 50000;
    a = u;
    show(a);
    u = a + 1.0;
    printf("%u\n", u);
    a = 2.0;
    b = 3.0;
    printf("%d %d %d %d %d %d\n", a < b, a <= b, a > b, a >= b, a == b, a != b);
    if (a)
        printf("nonzero\n");
    a = 0.0;
    if (!a)
        printf("zero\n");
    show(half(5.0));
    a = 1.5;
    a += 2.25;
    show(a);
    a *= 2.0;
    show(a);
    arr[1] = 1.25;
    arr[2] = arr[1] * 4.0;
    show(arr[2]);
    p.v = 9.5;
    show(p.v - 0.5);
    printf("%d\n", (int)sizeof(double));
    a = 2.0;
    printf("%lf %le %Lg %LG\n", a / 3.0, a * 1e10L, a, a * 1e-10L);
    printf("[%10.3lf] [%-8.2lf] %+lf\n", 3.14159L, -2.5L, 1.0L);
    show(sqrt(a));
    show(sin(a));
    show(cos(a));
    show(atan2(a, -1.0));
    show(pow(a, 0.5));
    show(exp(a));
    show(log(a));
    show(fabs(-a));
    show(floor(-2.5L));
    show(ldexp(a, 3));
    show(fmod(a * 3.5, 2.0));
    printf("%.3f\n", sqrt(2.0));
    more();
    return 0;
}
