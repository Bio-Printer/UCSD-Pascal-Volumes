/* a calculator: recursive descent over + - * / % ( ), 32-bit longs */
#include <stdio.h>
#include <ctype.h>

char *p;
int bad;

long expr(void);

void skip(void)
{
    while (*p == ' ')
        p++;
}

long factor(void)
{
    long v;
    skip();
    if (*p == '-') {
        p++;
        return -factor();
    }
    if (*p == '(') {
        p++;
        v = expr();
        skip();
        if (*p == ')')
            p++;
        else
            bad = 1;
        return v;
    }
    if (!isdigit(*p)) {
        bad = 1;
        return 0;
    }
    v = 0;
    while (isdigit(*p))
        v = v * 10 + (*p++ - '0');
    return v;
}

long term(void)
{
    long v;
    long d;
    int op;
    v = factor();
    for (;;) {
        skip();
        op = *p;
        if (op != '*' && op != '/' && op != '%')
            return v;
        p++;
        d = factor();
        if (op == '*')
            v = v * d;
        else if (d == 0)
            bad = 2;
        else if (op == '/')
            v = v / d;
        else
            v = v % d;
    }
}

long expr(void)
{
    long v;
    int op;
    v = term();
    for (;;) {
        skip();
        op = *p;
        if (op != '+' && op != '-')
            return v;
        p++;
        if (op == '+')
            v = v + term();
        else
            v = v - term();
    }
}

int main(void)
{
    char line[80];
    long v;
    printf("Tiny-C calculator: + - * / %% ( ), empty line ends\n");
    for (;;) {
        printf("> ");
        if (!fgets(line, 80, stdin) || line[0] == '\n' || line[0] == 0)
            break;
        p = line;
        bad = 0;
        v = expr();
        skip();
        if (*p != '\n' && *p != 0)
            bad = 1;
        if (bad == 2)
            printf("division by zero\n");
        else if (bad)
            printf("syntax error\n");
        else
            printf("= %ld\n", v);
    }
    printf("bye\n");
    return 0;
}
