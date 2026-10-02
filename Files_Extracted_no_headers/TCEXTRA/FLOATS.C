/* 0.1 + 0.2 == 0.3 holds for the 24-bit II.0 real (not for IEEE double); sizes are 16-bit */
/* floating point and math.h */
#include <stdio.h>
#include <math.h>

int main(void)
{
    float x;
    float y;
    int i;
    x = 1.0;
    for (i = 0; i < 10; i++)
        x = x * 1.5;
    printf("%.3f\n", x);
    y = sqrt(2.0);
    printf("%.5f %.5f\n", y, y * y);
    printf("%.4f %.4f %.4f\n", sin(0.5), cos(0.5), atan(1.0) * 4.0);
    printf("%.4f %.4f %.4f\n", exp(1.0), log(10.0), log10(1000.0));
    printf("%.3f %.3f %.3f %.3f\n", floor(2.7), ceil(2.1), fabs(-3.25), pow(2.0, 10.0));
    printf("%d %d %d\n", (int)3.99, (int)-3.99, (int)(0.1 + 0.2 == 0.3));
    printf("%e %g %g\n", 12345.678, 0.0001234, 123456789.0);
    for (i = 0; i <= 8; i++)
        printf("%6.2f", i * 0.25);
    printf("\n");
    return 0;
}
