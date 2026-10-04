/* types.c -- type predicates shared by the parser and the code generator
   (resident: both passes use them). */
#include "tc.h"
#pragma segment MAIN

int isintegral(struct Type *t)
{
    return t->kind >= TY_CHAR && t->kind <= TY_ULONG;
}

int isfloatty(struct Type *t)
{
    return t->kind >= TY_FLOAT && t->kind <= TY_LDOUBLE;
}

/* the 8-byte type: double, long double */
int isdblty(struct Type *t)
{
    return t->kind == TY_DOUBLE || t->kind == TY_LDOUBLE;
}

int islongty(struct Type *t)
{
    return t->kind == TY_LONG || t->kind == TY_ULONG;
}

int isunsignedty(struct Type *t)
{
    return t->kind == TY_UCHAR || t->kind == TY_UINT || t->kind == TY_ULONG;
}

int isword(struct Type *t)
{
    return (t->kind >= TY_CHAR && t->kind <= TY_UINT) || t->kind == TY_PTR;
}

int isptrlike(struct Type *t)
{
    return t->kind == TY_PTR;
}

int isscalar(struct Type *t)
{
    return isintegral(t) || isfloatty(t) || t->kind == TY_PTR;
}

int isaggregate(struct Type *t)
{
    return t->kind == TY_STRUCT || t->kind == TY_UNION || t->kind == TY_ARRAY;
}

int twords(struct Type *t)
{
    if (t->kind == TY_CHAR || t->kind == TY_UCHAR)
        return 1;
    return (t->size + 1) / 2;
}

int retwords(struct Type *ft)
{
    struct Type *r;
    r = ft->base;
    if (r->kind == TY_VOID)
        return 0;
    if (r->kind == TY_STRUCT || r->kind == TY_UNION)
        return 1;
    return twords(r);
}

