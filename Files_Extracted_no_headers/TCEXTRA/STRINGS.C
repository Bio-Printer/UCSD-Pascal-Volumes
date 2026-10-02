/* string.h, sprintf, sscanf, ctype, macros with arguments */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))
#define STR(x) #x
#define CAT(a, b) a##b

void reverse(char *s)
{
    int i;
    int j;
    char t;
    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        t = s[i];
        s[i] = s[j];
        s[j] = t;
    }
}

int main(void)
{
    char buf[80];
    char word[20];
    int a;
    int b;
    int CAT(my, var);
    char *p;
    strcpy(buf, "Hello");
    strcat(buf, ", Tiny-C");
    printf("%s (%d)\n", buf, (int)strlen(buf));
    reverse(buf);
    printf("%s\n", buf);
    printf("%d %d %d\n", strcmp("abc", "abd") < 0, strcmp("b", "a") > 0, strncmp("abcx", "abcy", 3));
    p = strchr("find the x here", 'x');
    printf("%s|%s\n", p, strstr("haystack needle hay", "needle"));
    sprintf(buf, "%d-%s-%c", 42, "str", 'z');
    printf("[%s]\n", buf);
    sscanf("17 29 word", "%d %d %s", &a, &b, word);
    printf("%d %s %d\n", a + b, word, MAX(a, b));
    printf("%d %s\n", SQUARE(a + 1), STR(hello world));
    myvar = 5;
    printf("%d\n", myvar);
    for (p = "MiXeD 123"; *p; p++)
        putchar(isupper(*p) ? tolower(*p) : toupper(*p));
    putchar('\n');
    return 0;
}
