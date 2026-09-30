/* math.c -- Tiny-C library: the code behind <math.h> */
#include "libint.h"
float sqrt(float x) { return x <= 0.0 ? 0.0 : __cspf(31, x); }
float sin(float x) { return __cspf(25, x); }
float cos(float x) { return __cspf(26, x); }
float atan(float x) { return __cspf(28, x); }
float log(float x) { return __cspf(29, x); }
float exp(float x) { return __cspf(30, x); }
float log10(float x) { return __cspf(27, x); }
float fabs(float x) { return x < 0.0 ? -x : x; }
float tan(float x) { return sin(x) / cos(x); }
float atan2(float y, float x)
{
    float a;
    if (x == 0.0)
        return y > 0.0 ? M_PI / 2.0 : (y < 0.0 ? -M_PI / 2.0 : 0.0);
    a = atan(y / x);
    if (x < 0.0)
        a = y < 0.0 ? a - M_PI : a + M_PI;
    return a;
}

float asin(float x) { return atan2(x, sqrt(1.0 - x * x)); }
float acos(float x) { return atan2(sqrt(1.0 - x * x), x); }
float floor(float x)
{
    long l;
    if (x >= 8388608.0 || x <= -8388608.0)
        return x;               /* already an integer (24-bit mantissa) */
    l = (long)x;
    if (l > x)
        l--;
    return (float)l;
}

float ceil(float x)
{
    return -floor(-x);
}

float fmod(float x, float y)
{
    float q;
    if (y == 0.0)
        return 0.0;
    q = x / y;
    q = q < 0.0 ? ceil(q) : floor(q);
    return x - q * y;
}

float pow(float x, float y)
{
    long n;
    float r;
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

float sinh(float x) { return (exp(x) - exp(-x)) / 2.0; }
float cosh(float x) { return (exp(x) + exp(-x)) / 2.0; }
float tanh(float x) { float e; e = exp(x + x); return (e - 1.0) / (e + 1.0); }
float modf(float x, float *ip)
{
    *ip = x < 0.0 ? ceil(x) : floor(x);
    return x - *ip;
}

float ldexp(float x, int e)
{
    while (e > 0) { x = x * 2.0; e--; }
    while (e < 0) { x = x / 2.0; e++; }
    return x;
}

