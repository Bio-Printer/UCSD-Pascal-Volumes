/* tcrt.c -- Tiny-C library: the code behind <tcrt.h> */
#include "libint.h"
#pragma nofltused
unsigned __udiv(unsigned a, unsigned b)
{
    unsigned q;
    unsigned r;
    if (b == 0)
        return __dvi(1, 0);             /* the P-machine's divide-by-zero error */
    if ((int)b < 0)
        return a >= b;
    if ((int)a >= 0)
        return __dvi(a, b);
    q = __dvi(__dvi(a & 32767, 2) + 16384, b);
    q = q + q;
    r = a - q * b;
    if (r >= b)
        q++;
    return q;
}

unsigned __umod(unsigned a, unsigned b)
{
    return a - __udiv(a, b) * b;
}

int __divi(int a, int b)
{
    int neg;
    unsigned q;
    neg = 0;
    if (a < 0) {
        a = -a;
        neg = 1;
    }
    if (b < 0) {
        b = -b;
        neg = !neg;
    }
    q = __udiv(a, b);
    return neg ? -(int)q : (int)q;
}

int __modi(int a, int b)
{
    unsigned r;
    int neg;
    neg = a < 0;
    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;
    r = __umod(a, b);
    return neg ? -(int)r : (int)r;
}

int __shl(int a, int n)
{
    while (n > 0) {
        a = a + a;
        n--;
    }
    return a;
}

unsigned __ushr(unsigned a, int n)
{
    int p;
    if (n <= 0)
        return a;
    if (n >= 16)
        return 0;
    if ((int)a < 0)
        a = __dvi(a & 32767, 2) + 16384;
    else
        a = __dvi(a, 2);
    n--;
    p = 1;
    while (n > 0) {
        p = p + p;
        n--;
    }
    return __dvi(a, p);
}

int __shr(int a, int n)
{
    if (a >= 0)
        return __ushr(a, n);
    return ~__ushr(~a, n);
}

int __xor(int a, int b)
{
    return (a | b) & ~(a & b);
}

int __sx(int c)
{
    c = c & 255;
    if (c > 127)
        return c - 256;
    return c;
}

float __utof(unsigned u)
{
    if ((int)u >= 0)
        return (int)u;
    return (int)(u & 32767) + 32768.0;
}

unsigned __ftou(float f)
{
    if (f < 32768.0)
        return (int)f;
    return (unsigned)(int)(f - 32768.0) + 32768;
}

long __ladd(long a, long b)
{
    unsigned *x;
    unsigned *y;
    unsigned lo;
    x = (unsigned *)&a;
    y = (unsigned *)&b;
    lo = x[0] + y[0];
    x[1] = x[1] + y[1] + (lo < x[0]);
    x[0] = lo;
    return a;
}

long __lsub(long a, long b)
{
    unsigned *x;
    unsigned *y;
    unsigned lo;
    x = (unsigned *)&a;
    y = (unsigned *)&b;
    lo = x[0] - y[0];
    x[1] = x[1] - y[1] - (x[0] < y[0]);
    x[0] = lo;
    return a;
}

long __lneg(long a)
{
    unsigned *x;
    x = (unsigned *)&a;
    x[0] = ~x[0];
    x[1] = ~x[1];
    x[0] = x[0] + 1;
    if (x[0] == 0)
        x[1] = x[1] + 1;
    return a;
}

long __lnot(long a)
{
    unsigned *x;
    x = (unsigned *)&a;
    x[0] = ~x[0];
    x[1] = ~x[1];
    return a;
}

long __land(long a, long b)
{
    unsigned *x;
    unsigned *y;
    x = (unsigned *)&a;
    y = (unsigned *)&b;
    x[0] = x[0] & y[0];
    x[1] = x[1] & y[1];
    return a;
}

long __lor(long a, long b)
{
    unsigned *x;
    unsigned *y;
    x = (unsigned *)&a;
    y = (unsigned *)&b;
    x[0] = x[0] | y[0];
    x[1] = x[1] | y[1];
    return a;
}

long __lxor(long a, long b)
{
    unsigned *x;
    unsigned *y;
    x = (unsigned *)&a;
    y = (unsigned *)&b;
    x[0] = __xor(x[0], y[0]);
    x[1] = __xor(x[1], y[1]);
    return a;
}

void __mul32(unsigned u, unsigned v, unsigned *r)
{
    unsigned u0;
    unsigned u1;
    unsigned v0;
    unsigned v1;
    unsigned mid;
    unsigned c;
    unsigned lo;
    unsigned add;
    u0 = u & 255;
    u1 = __ushr(u, 8);
    v0 = v & 255;
    v1 = __ushr(v, 8);
    lo = u0 * v0;
    r[1] = u1 * v1;
    mid = u0 * v1;
    c = u1 * v0;
    mid = mid + c;
    c = mid < c;                        /* carry out of the middle sum */
    add = (mid & 255) * 256;
    r[0] = lo + add;
    r[1] = r[1] + __ushr(mid, 8) + c * 256 + (r[0] < lo);
}

long __lmul(long a, long b)
{
    unsigned *x;
    unsigned *y;
    unsigned r[2];
    x = (unsigned *)&a;
    y = (unsigned *)&b;
    __mul32(x[0], y[0], r);
    r[1] = r[1] + x[0] * y[1] + x[1] * y[0];
    x[0] = r[0];
    x[1] = r[1];
    return a;
}

int __ulcmp(unsigned long a, unsigned long b)
{
    unsigned *x;
    unsigned *y;
    x = (unsigned *)&a;
    y = (unsigned *)&b;
    if (x[1] != y[1])
        return x[1] < y[1] ? -1 : 1;
    if (x[0] != y[0])
        return x[0] < y[0] ? -1 : 1;
    return 0;
}

int __lcmp(long a, long b)
{
    int *x;
    int *y;
    unsigned *xu;
    unsigned *yu;
    x = (int *)&a;
    y = (int *)&b;
    xu = (unsigned *)&a;
    yu = (unsigned *)&b;
    if (x[1] != y[1])
        return x[1] < y[1] ? -1 : 1;
    if (xu[0] != yu[0])
        return xu[0] < yu[0] ? -1 : 1;
    return 0;
}

void __udiv32(unsigned *n, unsigned *d, unsigned *q, unsigned *r)
{
    int i;
    unsigned bit;
    unsigned w;
    q[0] = 0;
    q[1] = 0;
    r[0] = 0;
    r[1] = 0;
    if (d[1] == 0 && d[0] == 0) {
        q[0] = __dvi(1, 0);
        return;
    }
    if (n[1] == 0 && d[1] == 0) {
        q[0] = __udiv(n[0], d[0]);
        r[0] = n[0] - q[0] * d[0];
        return;
    }
    for (i = 31; i >= 0; i--) {
        /* r = r << 1 | bit i of n */
        r[1] = r[1] + r[1] + ((int)r[0] < 0);
        r[0] = r[0] + r[0];
        w = i >= 16 ? n[1] : n[0];
        bit = __ushr(w, i & 15) & 1;
        r[0] = r[0] | bit;
        if (r[1] > d[1] || (r[1] == d[1] && r[0] >= d[0])) {
            w = r[0];
            r[0] = r[0] - d[0];
            r[1] = r[1] - d[1] - (w < d[0]);
            if (i >= 16)
                q[1] = q[1] | __shl(1, i - 16);
            else
                q[0] = q[0] | __shl(1, i);
        }
    }
}

unsigned long __uldiv(unsigned long a, unsigned long b)
{
    unsigned q[2];
    unsigned r[2];
    unsigned *x;
    __udiv32((unsigned *)&a, (unsigned *)&b, q, r);
    x = (unsigned *)&a;
    x[0] = q[0];
    x[1] = q[1];
    return a;
}

unsigned long __ulmod(unsigned long a, unsigned long b)
{
    unsigned q[2];
    unsigned r[2];
    unsigned *x;
    __udiv32((unsigned *)&a, (unsigned *)&b, q, r);
    x = (unsigned *)&a;
    x[0] = r[0];
    x[1] = r[1];
    return a;
}

long __ldiv(long a, long b)
{
    int neg;
    neg = 0;
    if (__lcmp(a, 0L) < 0) {
        a = __lneg(a);
        neg = 1;
    }
    if (__lcmp(b, 0L) < 0) {
        b = __lneg(b);
        neg = !neg;
    }
    a = __uldiv(a, b);
    return neg ? __lneg(a) : a;
}

long __lmod(long a, long b)
{
    int neg;
    neg = 0;
    if (__lcmp(a, 0L) < 0) {
        a = __lneg(a);
        neg = 1;
    }
    if (__lcmp(b, 0L) < 0)
        b = __lneg(b);
    a = __ulmod(a, b);
    return neg ? __lneg(a) : a;
}

long __lshl(long a, int n)
{
    unsigned *x;
    x = (unsigned *)&a;
    if (n >= 16) {
        x[1] = x[0];
        x[0] = 0;
        n = n - 16;
    }
    while (n > 0) {
        x[1] = x[1] + x[1] + ((int)x[0] < 0);
        x[0] = x[0] + x[0];
        n--;
    }
    return a;
}

unsigned long __ulshr(unsigned long a, int n)
{
    unsigned *x;
    x = (unsigned *)&a;
    if (n >= 16) {
        x[0] = x[1];
        x[1] = 0;
        n = n - 16;
    }
    if (n > 0) {
        x[0] = __ushr(x[0], n) | __shl(x[1], 16 - n);
        x[1] = __ushr(x[1], n);
    }
    return a;
}

long __lshr(long a, int n)
{
    int *x;
    if (__lcmp(a, 0L) >= 0)
        return __ulshr(a, n);
    return __lnot(__ulshr(__lnot(a), n));
}

long __itol(int i)
{
    long r;
    int *x;
    x = (int *)&r;
    x[0] = i;
    x[1] = i < 0 ? -1 : 0;
    return r;
}

long __utol(unsigned u)
{
    long r;
    unsigned *x;
    x = (unsigned *)&r;
    x[0] = u;
    x[1] = 0;
    return r;
}

int __ltoi(long a)
{
    int *x;
    x = (int *)&a;
    return x[0];
}

float __ltof(long a)
{
    int *x;
    unsigned *xu;
    x = (int *)&a;
    xu = (unsigned *)&a;
    return x[1] * 65536.0 + __utof(xu[0]);
}

float __ultof(unsigned long a)
{
    unsigned *x;
    x = (unsigned *)&a;
    return __utof(x[1]) * 65536.0 + __utof(x[0]);
}

unsigned long __ftoul(float f)
{
    unsigned long r;
    unsigned *x;
    unsigned h;
    x = (unsigned *)&r;
    if (f < 0.0)
        f = 0.0 - f;
    h = __ftou(f / 65536.0);
    x[1] = h;
    x[0] = __ftou(f - __utof(h) * 65536.0);
    return r;
}

long __ftol(float f)
{
    if (f < 0.0)
        return __lneg(__ftoul(0.0 - f));
    return __ftoul(f);
}

