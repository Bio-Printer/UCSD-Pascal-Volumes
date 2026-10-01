/* pi.c -- Archimedes' method with doubles (P-Code mode only).
   Polygons with 6, 12, 24, ... sides inside and around a circle of
   diameter 1: their perimeters are a lower and an upper bound for pi.
   Doubling the sides:
       upper' = 2 * upper * lower / (upper + lower)   (harmonic mean)
       lower' = sqrt(upper' * lower)                  (geometric mean)
   Each step makes the gap about 4 times smaller (0.6 digits); both bounds
   close in on pi until they agree to 15 significant digits. */
#include <stdio.h>
#include <math.h>

char ref[] = "3.14159265358979";            /* pi, 15 significant digits */

/* how many leading digits of x (as %.14lf) agree with pi */
int digits(double x)
{
    char buf[40];
    int i;
    int n;
    sprintf(buf, "%.14lf", x);
    n = 0;
    for (i = 0; ref[i] && buf[i] == ref[i]; i++)
        if (ref[i] != '.')
            n++;
    return n;
}

int main(void)
{
    double upper;
    double lower;
    double sides;
    int i;
    upper = 2.0 * sqrt(3.0L);               /* hexagon around the circle */
    lower = 3.0;                            /* hexagon inside it */
    sides = 6.0;
    printf("Pi by Archimedes' method (polygons around and inside a circle)\n\n");
    printf("step       sides  lower bound         upper bound         digits\n");
    for (i = 0; i <= 30; i++) {
        printf("%4d %11.0lf  %.16lf  %.16lf  %2d %2d\n", i, sides, lower, upper,
               digits(lower), digits(upper));
        if (digits(lower) >= 15 && digits(upper) >= 15)
            break;
        upper = 2.0 * upper * lower / (upper + lower);
        lower = sqrt(upper * lower);
        sides = sides * 2.0;
    }
    printf("\npi = %.14lf (atan: %.14lf)\n", (lower + upper) / 2.0, 4.0 * atan(1.0L));
    return 0;
}
