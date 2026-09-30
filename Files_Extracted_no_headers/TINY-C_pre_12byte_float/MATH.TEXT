/* math.c -- Tiny-C library: the code behind <math.h> */
#include "libint.h"
double sqrt(double x) { return x <= 0.0 ? 0.0 : __cspf(31, x); }
double sin(double x) { return __cspf(25, x); }
double cos(double x) { return __cspf(26, x); }
double atan(double x) { return __cspf(28, x); }
double log(double x) { return __cspf(29, x); }
double exp(double x) { return __cspf(30, x); }
double log10(double x) { return __cspf(27, x); }
double fabs(double x) { return x < 0.0 ? -x : x; }
double tan(double x) { return sin(x) / cos(x); }
double atan2(double y, double x)
{
    double a;
    if (x == 0.0)
        return y > 0.0 ? M_PI / 2.0 : (y < 0.0 ? -M_PI / 2.0 : 0.0);
    a = atan(y / x);
    if (x < 0.0)
        a = y < 0.0 ? a - M_PI : a + M_PI;
    return a;
}

double asin(double x) { return atan2(x, sqrt(1.0 - x * x)); }
double acos(double x) { return atan2(sqrt(1.0 - x * x), x); }
double floor(double x)
{
    long l;
    if (x >= 8388608.0 || x <= -8388608.0)
        return x;               /* already an integer (24-bit mantissa) */
    l = (long)x;
    if (l > x)
        l--;
    return (double)l;
}

double ceil(double x)
{
    return -floor(-x);
}

double fmod(double x, double y)
{
    double q;
    if (y == 0.0)
        return 0.0;
    q = x / y;
    q = q < 0.0 ? ceil(q) : floor(q);
    return x - q * y;
}

double pow(double x, double y)
{
    long n;
    double r;
    int neg;
    if (y == floor(y) && y > -32768.0 && y < 32768.0) {
        n = (long)y;
        neg = n < 0;
        if (neg)
            n = -n;
        r = 1.0;
        while (n > 0) {
            if (n & 1)
                r = r * x;
            x = x * x;
            n = n >> 1;
        }
        return neg ? 1.0 / r : r;
    }
    if (x <= 0.0)
        return 0.0;
    return exp(y * log(x));
}

double sinh(double x) { return (exp(x) - exp(-x)) / 2.0; }
double cosh(double x) { return (exp(x) + exp(-x)) / 2.0; }
double tanh(double x) { double e; e = exp(x + x); return (e - 1.0) / (e + 1.0); }
double modf(double x, double *ip)
{
    *ip = x < 0.0 ? ceil(x) : floor(x);
    return x - *ip;
}

double ldexp(double x, int e)
{
    while (e > 0) { x = x * 2.0; e--; }
    while (e < 0) { x = x / 2.0; e++; }
    return x;
}

