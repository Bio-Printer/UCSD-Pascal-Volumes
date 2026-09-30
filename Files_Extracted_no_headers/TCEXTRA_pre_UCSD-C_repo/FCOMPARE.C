/* real comparisons, all six operators, signs, zero and magnitudes */
#include <stdio.h>

float v[9] = { -1000.0, -2.5, -1.0, -0.001, 0.0, 0.001, 1.0, 2.5, 1000.0 };

int main(void)
{
    int i;
    int j;
    int n[6];
    float z;
    for (i = 0; i < 6; i++)
        n[i] = 0;
    for (i = 0; i < 9; i++)
        for (j = 0; j < 9; j++) {
            if (v[i] < v[j]) n[0]++;
            if (v[i] <= v[j]) n[1]++;
            if (v[i] > v[j]) n[2]++;
            if (v[i] >= v[j]) n[3]++;
            if (v[i] == v[j]) n[4]++;
            if (v[i] != v[j]) n[5]++;
        }
    printf("< %d  <= %d  > %d  >= %d  == %d  != %d\n", n[0], n[1], n[2], n[3], n[4], n[5]);
    for (i = 0; i < 9; i++)
        printf("%d", (v[i] < 0.0) + 2 * (v[i] == 0.0) + 4 * (v[i] > 0.0));
    printf("\n");
    z = 0.0;
    printf("%d %d %d %d\n", !z, z ? 1 : 0, v[5] ? 1 : 0, 1.0e30 > 1.0e29);
    printf("%d %d\n", 1.0e-30 < 1.0e-29, -1.0e30 < 1.0e-30);
    return 0;
}
