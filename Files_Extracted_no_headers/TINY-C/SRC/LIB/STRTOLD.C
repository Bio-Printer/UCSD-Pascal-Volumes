/* strtold.c -- strtold, atold: text to an 8-byte double (CSP 135;
   P-Code mode only) */
#include <stdlib.h>

double strtold(char *s, char **end)
{
    double d;
    int n;
    n = __cspi(135, s, &d);
    if (end)
        *end = s + n;
    return d;
}

double atold(char *s)
{
    return strtold(s, NULL);
}
