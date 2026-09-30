/* structs, unions, pointers to structs, linked list, typedef, enum */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    int value;
    char name[10];
    struct node *next;
} Node;

enum color { RED, GREEN = 5, BLUE };

union word {
    unsigned w;
    unsigned char b[2];
};

struct pair { int a; long b; float c; };

struct pair makepair(int a)
{
    struct pair p;
    p.a = a;
    p.b = a * 1000L;
    p.c = a / 2.0;
    return p;
}

Node *push(Node *list, int v, char *name)
{
    Node *n;
    n = malloc(sizeof(Node));
    n->value = v;
    strcpy(n->name, name);
    n->next = list;
    return n;
}

int main(void)
{
    Node *list;
    Node *p;
    union word u;
    struct pair q;
    struct pair r;
    int total;
    list = NULL;
    list = push(list, 1, "one");
    list = push(list, 2, "two");
    list = push(list, 3, "three");
    total = 0;
    for (p = list; p; p = p->next) {
        printf("%d %s\n", p->value, p->name);
        total += p->value;
    }
    printf("total %d sizeof(Node) %d\n", total, (int)sizeof(Node));
    printf("colors %d %d %d\n", RED, GREEN, BLUE);
    u.w = 0x1234;
    printf("union %x %x\n", u.b[0], u.b[1]);
    q = makepair(7);
    r = q;
    r.a++;
    printf("pair %d %ld %.1f / %d\n", q.a, q.b, q.c, r.a);
    return 0;
}
