/* fltfmt.c -- printf's %f, %e and %g.  A program that uses floating
   point refers to __fltused, which links this module; its initialiser
   installs the formatter for stdio.c. */
#include "libint.h"

int __fdigits(float v, int prec, int style, int alt, char *out)
{
    int e;
    int n;
    int i;
    int d;
    int ndig;
    float r;
    long ip;
    char digs[40];
    n = 0;
    if (prec > 20)
        prec = 20;
    e = 0;
    if (v != 0.0) {
        while (v >= 10.0) {
            v = v / 10.0;
            e++;
        }
        while (v < 1.0) {
            v = v * 10.0;
            e--;
        }
    }
    /* v in [1, 10): value = v * 10^e */
    if (style == 'g') {
        if (prec == 0)
            prec = 1;
        if (e < -4 || e >= prec)
            style = 'e';
        else
            style = 'f';
        if (style == 'e')
            prec = prec - 1;
        else
            prec = prec - 1 - e;
        if (prec < 0)
            prec = 0;
        alt = alt | 2;          /* strip trailing zeros */
    }
    ndig = style == 'e' ? prec + 1 : e + 1 + prec;
    if (ndig > 38)
        ndig = 38;
    /* round at digit ndig */
    if (ndig >= 0) {
        r = 0.5;
        for (i = 0; i < ndig - 1; i++)
            r = r / 10.0;
        v = v + r;
        if (v >= 10.0) {
            v = v / 10.0;
            e++;
            if (style == 'f')
                ndig++;
        }
    }
    for (i = 0; i < ndig && i < 38; i++) {
        if (i < 8) {
            d = (int)v;
            if (d > 9)
                d = 9;
            v = (v - d) * 10.0;
        } else
            d = 0;
        digs[i] = '0' + d;
    }
    if (ndig < 0)
        ndig = 0;
    if (style == 'f') {
        if (e < 0)
            out[n++] = '0';
        for (i = 0; i <= e; i++)
            out[n++] = i < ndig ? digs[i] : '0';
        if (prec > 0 || (alt & 1)) {
            out[n++] = '.';
            for (i = 0; i < prec; i++) {
                d = e + 1 + i;
                out[n++] = d < 0 ? '0' : (d < ndig ? digs[d] : '0');
            }
        }
    } else {
        out[n++] = ndig > 0 ? digs[0] : '0';
        if (prec > 0 || (alt & 1)) {
            out[n++] = '.';
            for (i = 1; i <= prec; i++)
                out[n++] = i < ndig ? digs[i] : '0';
        }
    }
    if ((alt & 2) && !(alt & 1)) {
        for (i = 0; i < n; i++)
            if (out[i] == '.')
                break;
        if (i < n) {
            while (out[n - 1] == '0')
                n--;
            if (out[n - 1] == '.')
                n--;
        }
    }
    if (style == 'e') {
        out[n++] = 'e';
        if (e < 0) {
            out[n++] = '-';
            e = -e;
        } else
            out[n++] = '+';
        out[n++] = '0' + e / 10;
        out[n++] = '0' + e % 10;
    }
    return n;
}

int __fltinstall(void)
{
    __fltfmt = __fdigits;
    return 1;
}

int __fltused = __fltinstall();
