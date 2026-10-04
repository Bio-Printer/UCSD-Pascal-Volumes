/* function pointers, qsort, arrays of function pointers */
#include <stdio.h>
#include <stdlib.h>

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }

int (*ops[3])(int, int) = { add, sub, mul };

int apply(int (*f)(int, int), int x, int y)
{
    return f(x, y);
}

int cmp(void *a, void *b)
{
    return *(int *)a - *(int *)b;
}

int main(void)
{
    int i;
    int v[10];
    int (*fp)(int, int);
    for (i = 0; i < 3; i++)
        printf("op%d: %d\n", i, ops[i](7, 3));
    fp = sub;
    printf("apply %d %d\n", apply(add, 20, 22), apply(fp, 20, 22));
    printf("same %d\n", fp == sub);
    for (i = 0; i < 10; i++)
        v[i] = (i * 7 + 3) % 10;
    qsort(v, 10, sizeof(int), cmp);
    for (i = 0; i < 10; i++)
        printf("%d ", v[i]);
    printf("\n");
    return 0;
}
