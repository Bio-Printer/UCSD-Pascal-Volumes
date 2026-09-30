/* gen.c -- P-code generation and the object file.
 *
 * Each C function becomes one II.0 procedure: code, then its jump table
 * (long jumps), DATASZ, PARMSZ, EXITIC, ENTRIC and procnum/lexlevel at
 * JTAB.  Everything inside a procedure is self-relative, so procedures are
 * written to the object file as finished byte images; the linker only
 * fills in procedure numbers and CXP operands.
 *
 * Global initialisers are compiled into "init" procedures (one emitter
 * of their own) that the program runs once at startup.
 */
#include "tc.h"
#pragma segment GEN

/* opcodes */
#define O_ABI  0x80
#define O_ADI  0x82
#define O_ADR  0x83
#define O_LAND 0x84
#define O_DVI  0x86
#define O_DVR  0x87
#define O_FLT  0x8A
#define O_LOR  0x8D
#define O_MODI 0x8E
#define O_MPI  0x8F
#define O_IXA  0xA4
#define O_MPR  0x90
#define O_NGI  0x91
#define O_NGR  0x92
#define O_NOT  0x93
#define O_SBI  0x95
#define O_SBR  0x96
#define O_STO  0x9A
#define O_CSP  0x9E
#define O_FJP  0xA1
#define O_INC  0xA2
#define O_IND  0xA3
#define O_LAO  0xA5
#define O_MOV  0xA8
#define O_LDO  0xA9
#define O_SRO  0xAB
#define O_XJP  0xAC
#define O_RNP  0xAD
#define O_EQU  0xAF
#define O_GEQ  0xB0
#define O_GRT  0xB1
#define O_LDA  0xB2
#define O_LDC  0xB3
#define O_LEQ  0xB4
#define O_LES  0xB5
#define O_LOD  0xB6
#define O_NEQ  0xB7
#define O_UJP  0xB9
#define O_LDM  0xBC
#define O_STM  0xBD
#define O_LDB  0xBE
#define O_STB  0xBF
#define O_EQUI 0xC3
#define O_GEQI 0xC4
#define O_GRTI 0xC5
#define O_LLA  0xC6
#define O_LDCI 0xC7
#define O_LEQI 0xC8
#define O_LESI 0xC9
#define O_LDL  0xCA
#define O_NEQI 0xCB
#define O_STL  0xCC
#define O_CXP  0xCD
#define O_CGP  0xCF
#define O_LPA  0xD0
#define O_EFJ  0xD3
#define O_NFJ  0xD4
#define O_SIND0 0xF8

#define CSP_EXIT 4
#define CSP_MVL  2
#define CSP_TNC  23

/* relocation kinds in the object file */
#define R_CALL    1         /* CXP s,p at pos */
#define R_FNPTR   2         /* LDCI word at pos+1: seg | proc << 8 */
#define R_GSTAT   3         /* 2-byte global operand at pos: module static */
#define R_GNAME   4         /* 2-byte global operand at pos: named variable */
#define R_NEAR    5         /* CGP p at pos: a call within the segment */

struct Emit {
    unsigned char *code;
    int max;
    int pc;
    int *labpos;
    int maxlab;
    int nlab;
    int *fixpos;
    int *fixlab;
    int maxfix;
    int nfix;
    int *relpos;
    int *reltype;
    char **relname;
    int maxrel;
    int nrel;
    int curlocal;
    int maxlocal;
    int nparam;
    int scratch;
};

static struct Emit fe;          /* the current function */
static struct Emit ie;          /* global initialisers */
static struct Emit *E;
static int ninit;
static int ininit;


static int lvtemp;              /* compound assignment: temp holding the address, 0 = simple lvalue */
static struct Node *lvnode;     /* compound assignment: the lvalue */

static void setupemit(struct Emit *e, int max, int maxlab, int maxfix, int maxrel)
{
    e->max = max;
    e->code = (unsigned char *)malloc(max);
    e->maxlab = maxlab;
    e->labpos = (int *)malloc(maxlab * sizeof(int));
    e->maxfix = maxfix;
    e->fixpos = (int *)malloc(maxfix * sizeof(int));
    e->fixlab = (int *)malloc(maxfix * sizeof(int));
    e->maxrel = maxrel;
    e->relpos = (int *)malloc(maxrel * sizeof(int));
    e->reltype = (int *)malloc(maxrel * sizeof(int));
    e->relname = (char **)malloc(maxrel * sizeof(char *));
    if (!e->code || !e->labpos || !e->fixlab || !e->relname)
        fatal(2 /* out of memory */, 0);
    e->pc = 0;
    e->nlab = 0;
    e->nfix = 0;
    e->nrel = 0;
}

/* the frame variables live in the emitter while it is not current */
static void saveframe(void)
{
    E->curlocal = curlocal;
    E->maxlocal = maxlocal;
    E->nparam = nparamwords;
    E->scratch = scratch;
}

static void loadframe(void)
{
    curlocal = E->curlocal;
    maxlocal = E->maxlocal;
    nparamwords = E->nparam;
    scratch = E->scratch;
}

/* ---- bytes ---- */

static void ob(int b)
{
    if (E->pc >= E->max)
        fatal(95 /* function too large */, 0);
    E->code[E->pc++] = b & 255;
}

static void big(int v)
{
    if (v >= 0 && v < 128)
        ob(v);
    else {
        ob(128 | ((v >> 8) & 127));
        ob(v & 255);
    }
}

static void opbig(int op, int v)
{
    ob(op);
    big(v);
}

static void ldc(int v)
{
    v = W16(v);
    if (v >= 0 && v < 128)
        ob(v);
    else if (v < 0 && v > -128) {
        ob(-v);
        ob(O_NGI);
    } else {
        ob(O_LDCI);
        ob(v & 255);
        ob((v >> 8) & 255);
    }
}

static void ldl(int off)
{
    if (off >= 1 && off <= 16)
        ob(0xD7 + off);
    else
        opbig(O_LDL, off);
}

void gen_stl(int off)
{
    opbig(O_STL, off);
}

static void lla(int off)
{
    opbig(O_LLA, off);
}

static void ldo(int off)
{
    if (off >= 1 && off <= 16)
        ob(0xE7 + off);
    else
        opbig(O_LDO, off);
}

static void sro(int off)
{
    opbig(O_SRO, off);
}

static void lao(int off)
{
    opbig(O_LAO, off);
}

static void ind(int k)
{
    if (k == 0)
        ob(O_SIND0);
    else if (k > 0 && k < 8)
        ob(O_SIND0 + k);
    else
        opbig(O_IND, k);
}

static void addconst(int bytes)
{
    if (bytes == 0)
        return;
    if (bytes > 0 && (bytes & 1) == 0)
        opbig(O_INC, bytes / 2);
    else {
        ldc(bytes);
        ob(O_ADI);
    }
}

static void csp(int n)
{
    ob(O_CSP);
    ob(n);
}

/* relocation targets outlive the statement they came from: keep one copy of each name */
#define NHASH 64
struct Name {
    char *s;
    struct Name *next;
};
static struct Name **names;     /* [NHASH], allocated per run */

static char *intern(char *s)
{
    struct Name *n;
    int h;
    h = hashstr(s) & (NHASH - 1);
    for (n = names[h]; n; n = n->next)
        if (strcmp(n->s, s) == 0)
            return n->s;
    n = (struct Name *)palloc(sizeof(struct Name));
    n->s = pstrdup(s);
    n->next = names[h];
    names[h] = n;
    return n->s;
}

static void reloc(int type, char *name)
{
    name = intern(name);
    if (E->nrel >= E->maxrel)
        fatal(96 /* too many calls in one function */, 0);
    E->relpos[E->nrel] = E->pc;
    E->reltype[E->nrel] = type;
    E->relname[E->nrel] = name;
    E->nrel++;
}

int newtemp(int words)
{
    curlocal = curlocal + words;
    if (curlocal > maxlocal)
        maxlocal = curlocal;
    return curlocal - words + 1;
}

/* discard n words from the evaluation stack (the P-machine has no pop) */
static void drop(int n)
{
    while (n-- > 0)
        gen_stl(scratch);
}

/* ---- labels and jumps ---- */

int newlabel(void)
{
    if (E->nlab >= E->maxlab)
        fatal(89 /* function too large (labels) */, 0);
    E->labpos[E->nlab] = -1;
    return E->nlab++;
}

void setlabel(int l)
{
    int i;
    /* a UJP to this very label just before it (a return at the end of a
       function, an if without else ...) is 2 bytes for nothing: drop it,
       with the labels already set after it */
    if (E->nfix > 0 && E->fixpos[E->nfix - 1] == E->pc - 1 && E->fixlab[E->nfix - 1] == l &&
        E->code[E->pc - 2] == O_UJP) {
        E->nfix--;
        E->pc = E->pc - 2;
        for (i = 0; i < E->nlab; i++)
            if (E->labpos[i] == E->pc + 2)
                E->labpos[i] = E->pc;
    }
    E->labpos[l] = E->pc;
}

static void jmpop(int op, int l)
{
    ob(op);
    if (E->nfix >= E->maxfix)
        fatal(97 /* function too large (jumps) */, 0);
    E->fixpos[E->nfix] = E->pc;
    E->fixlab[E->nfix] = l;
    E->nfix++;
    ob(0);
}

void jump(int l)
{
    jmpop(O_UJP, l);
}

/* a word in a case table: self-relative, target = address - word */
static void caseword(int l)
{
    if (E->nfix >= E->maxfix)
        fatal(97 /* function too large (jumps) */, 0);
    E->fixpos[E->nfix] = E->pc;
    E->fixlab[E->nfix] = -2 - l;
    E->nfix++;
    ob(0);
    ob(0);
}

/* ---- types ---- */

static int valwords(struct Type *t)
{
    if (t->kind == TY_STRUCT || t->kind == TY_UNION || t->kind == TY_ARRAY)
        return 1;           /* represented by its address */
    if (t->kind == TY_VOID)
        return 0;
    return twords(t);
}

static int ischar(struct Type *t)
{
    return t->kind == TY_CHAR || t->kind == TY_UCHAR;
}

static int ismulti(struct Type *t)
{
    return isfloatty(t) || islongty(t);
}

/* ---- lvalues ---- */

#define LV_LOCAL 1
#define LV_GLOBAL 2
#define LV_PTR 3

struct LV {
    int kind;
    int off;                    /* word offset of the variable */
    int boff;                   /* extra byte offset */
    struct Node *ptr;           /* LV_PTR: address expression */
    char *name;                 /* LV_GLOBAL: link name, 0 for a module static */
};

/* global variable access: operand is always two bytes so the linker can
   fill it in -- R_GSTAT: module static (operand = offset in the module),
   R_GNAME: named variable (operand = word offset within it) */
static void gglob(int op, struct LV *lv, int add)
{
    int v;
    ob(op);
    if (lv->name) {
        reloc(R_GNAME, lv->name);
        v = add;
    } else {
        reloc(R_GSTAT, "");
        v = lv->off + add;
    }
    ob(128 | ((v >> 8) & 127));
    ob(v & 255);
}

static void lvinfo(struct Node *n, struct LV *lv)
{
    struct Node *p;
    lv->name = 0;
    if (n->op == N_VAR) {
        lv->kind = n->sym->kind == S_LOCAL ? LV_LOCAL : LV_GLOBAL;
        lv->off = n->sym->offset;
        lv->boff = 0;
        lv->ptr = 0;
        lv->name = lv->kind == LV_GLOBAL && lv->off < 0 ? n->sym->name : 0;
        return;
    }
    if (n->op == N_MEMBER) {
        lvinfo(n->a, lv);
        lv->boff = W16(lv->boff + n->val);
        return;
    }
    if (n->op == N_DEREF) {
        p = n->a;
        lv->kind = LV_PTR;
        lv->off = 0;
        lv->boff = 0;
        lv->ptr = p;
        if (p->op == N_ADD && p->b->op == N_NUM && p->type->kind == TY_PTR) {
            lv->ptr = p->a;
            lv->boff = p->b->val;
        }
        return;
    }
    if (n->op == N_CALL || n->op == N_ASSIGN || n->op == N_COND || n->op == N_COMMA) {
        /* structure-valued: the value is its address */
        lv->kind = LV_PTR;
        lv->off = 0;
        lv->boff = 0;
        lv->ptr = n;
        return;
    }
    error(57 /* lvalue required */, 0);
    lv->kind = LV_LOCAL;
    lv->off = scratch;
    lv->boff = 0;
    lv->ptr = 0;
}

static void gen_ptrvalue(struct Node *p)
{
    if (p->type->kind == TY_STRUCT || p->type->kind == TY_UNION)
        gen_value(p);           /* a structure-valued call etc.: its address */
    else
        gen_value(p);
}

/* push the address of lvalue n */
static void gen_addr(struct Node *n)
{
    struct LV lv;
    if (n->op == N_STR || n->op == N_FUNC) {
        gen_value(n);
        return;
    }
    lvinfo(n, &lv);
    if (lv.kind == LV_LOCAL) {
        lla(lv.off + (lv.boff >> 1));
        addconst(lv.boff & 1);
    } else if (lv.kind == LV_GLOBAL) {
        gglob(O_LAO, &lv, lv.boff >> 1);
        addconst(lv.boff & 1);
    } else {
        gen_ptrvalue(lv.ptr);
        addconst(lv.boff);
    }
}

/* push base and byte index for LDB/STB */
static void gen_byteaddr(struct Node *n)
{
    struct LV lv;
    struct Node *p;
    lvinfo(n, &lv);
    if (lv.kind == LV_LOCAL) {
        lla(lv.off + (lv.boff >> 1));
        ldc(lv.boff & 1);
    } else if (lv.kind == LV_GLOBAL) {
        gglob(O_LAO, &lv, lv.boff >> 1);
        ldc(lv.boff & 1);
    } else {
        p = lv.ptr;
        if (lv.boff == 0 && p->op == N_ADD && p->type->kind == TY_PTR && ischar(p->type->base)) {
            gen_value(p->a);
            gen_value(p->b);
        } else {
            gen_ptrvalue(p);
            ldc(lv.boff);
        }
    }
}

static void signext(void)
{
    reloc(R_CALL, "__sx");
    ob(O_CXP);
    ob(0);
    ob(0);
}

static void loadchar(struct Type *t)
{
    ob(O_LDB);
    if (t->kind == TY_CHAR)
        signext();
}

static int islv(struct Node *n)
{
    return n->op == N_VAR || n->op == N_MEMBER || n->op == N_DEREF;
}

/* a value of which only the low byte matters: char loads skip the sign
   extension, and conversions to char are unnecessary */
static void gen_lowbyte(struct Node *n)
{
    while (n->op == N_CAST && isword(n->type) && isword(n->a->type))
        n = n->a;
    if (islv(n) && ischar(n->type)) {
        gen_byteaddr(n);
        ob(O_LDB);
    } else
        gen_value(n);
}

/* load the value of lvalue n */
static void gen_load(struct Node *n)
{
    struct LV lv;
    struct Type *t;
    int w;
    t = n->type;
    if (t->kind == TY_ARRAY || t->kind == TY_STRUCT || t->kind == TY_UNION) {
        gen_addr(n);
        return;
    }
    if (ischar(t)) {
        gen_byteaddr(n);
        loadchar(t);
        return;
    }
    lvinfo(n, &lv);
    w = twords(t);
    if (w == 1) {
        if (lv.kind == LV_LOCAL && !(lv.boff & 1))
            ldl(lv.off + lv.boff / 2);
        else if (lv.kind == LV_GLOBAL && !(lv.boff & 1))
            gglob(O_LDO, &lv, lv.boff / 2);
        else if (lv.kind == LV_PTR && !(lv.boff & 1) && lv.boff >= 0) {
            gen_ptrvalue(lv.ptr);
            ind(lv.boff / 2);
        } else {
            gen_addr(n);
            ind(0);
        }
        return;
    }
    gen_addr(n);
    ob(O_LDM);
    ob(w);
}

/* store: the address part (pushed before the value) */
static void storepre(struct Node *n)
{
    struct LV lv;
    struct Type *t;
    t = n->type;
    if (ischar(t)) {
        gen_byteaddr(n);
        return;
    }
    lvinfo(n, &lv);
    if (twords(t) == 1 && (lv.kind == LV_LOCAL || lv.kind == LV_GLOBAL) && !(lv.boff & 1))
        return;
    gen_addr(n);
}

/* store: after the value was pushed */
static void storepost(struct Node *n)
{
    struct LV lv;
    struct Type *t;
    t = n->type;
    if (ischar(t)) {
        ob(O_STB);
        return;
    }
    lvinfo(n, &lv);
    if (twords(t) == 1) {
        if (lv.kind == LV_LOCAL && !(lv.boff & 1))
            gen_stl(lv.off + lv.boff / 2);
        else if (lv.kind == LV_GLOBAL && !(lv.boff & 1))
            gglob(O_SRO, &lv, lv.boff / 2);
        else
            ob(O_STO);
        return;
    }
    ob(O_STM);
    ob(twords(t));
}

static int simplelv(struct Node *n)
{
    struct LV lv;
    lvinfo(n, &lv);
    return lv.kind != LV_PTR;
}

/* via a temp holding the address */
static void loadvia(int ta, struct Type *t)
{
    ldl(ta);
    if (ischar(t)) {
        ldc(0);
        loadchar(t);
    } else if (twords(t) == 1)
        ind(0);
    else {
        ob(O_LDM);
        ob(twords(t));
    }
}

static void storeviapre(int ta, struct Type *t)
{
    ldl(ta);
    if (ischar(t))
        ldc(0);
}

static void storeviapost(struct Type *t)
{
    if (ischar(t))
        ob(O_STB);
    else if (twords(t) == 1)
        ob(O_STO);
    else {
        ob(O_STM);
        ob(twords(t));
    }
}

/* ---- conversions ---- */

static void gen_dcast(struct Node *n);

static void gen_cast(struct Node *n)
{
    struct Type *t;
    int fk;
    t = n->type;
    fk = n->a->type->kind;
    if (t->kind == TY_VOID) {
        gen_discard(n->a);
        return;
    }
    if (isdblty(t) || isdblty(n->a->type)) {
        gen_dcast(n);
        return;
    }
    gen_value(n->a);
    if (isfloatty(t) && !isfloatty(n->a->type)) {
        if (!islongty(n->a->type))
            ob(O_FLT);
        return;
    }
    if (!isfloatty(t) && isfloatty(n->a->type)) {
        csp(CSP_TNC);
        fk = TY_INT;
    }
    if (t->kind == TY_UCHAR && fk != TY_UCHAR) {
        ldc(255);
        ob(O_LAND);
    } else if (t->kind == TY_CHAR && fk != TY_CHAR)
        signext();
}

/* ---- calls ---- */

static void gen_call(struct Node *n, int want)
{
    struct Type *ft;
    struct Node *a;
    struct Param *p;
    struct Sym *fs;
    int pw;
    int rw;
    int sret;
    int blk;
    int bw;
    int pos;
    int t;
    int L;
    int A;
    ft = n->a->type;
    if (ft->kind == TY_PTR)
        ft = ft->base;
    fs = n->a->op == N_FUNC ? n->a->sym : 0;
    rw = retwords(ft);
    sret = 0;
    pw = 0;
    if (ft->base->kind == TY_STRUCT || ft->base->kind == TY_UNION) {
        sret = newtemp((ft->base->size + 1) / 2);
        lla(sret);
        pw++;
    }
    /* variadic extras go to a block in the caller's frame */
    blk = 0;
    if (ft->variadic) {
        p = ft->params;
        a = n->b;
        while (p && a) {
            p = p->next;
            a = a->next;
        }
        bw = 0;
        for (pos = 0; a; a = a->next)
            bw = bw + (a->type->kind == TY_STRUCT || a->type->kind == TY_UNION ? (a->type->size + 1) / 2 : twords(a->type));
        blk = newtemp(bw > 0 ? bw : 1);
        p = ft->params;
        a = n->b;
        while (p && a) {
            p = p->next;
            a = a->next;
        }
        pos = blk;
        for (; a; a = a->next) {
            if (a->type->kind == TY_STRUCT || a->type->kind == TY_UNION) {
                lla(pos);
                gen_value(a);
                opbig(O_MOV, (a->type->size + 1) / 2);
                pos = pos + (a->type->size + 1) / 2;
            } else if (twords(a->type) == 1) {
                gen_value(a);
                gen_stl(pos);
                pos++;
            } else {
                lla(pos);
                gen_value(a);
                ob(O_STM);
                ob(twords(a->type));
                pos = pos + twords(a->type);
            }
        }
    }
    p = ft->params;
    for (a = n->b; a; a = a->next) {
        if (ft->variadic && !p)
            break;
        if (a->type->kind == TY_STRUCT || a->type->kind == TY_UNION) {
            gen_value(a);
            ob(O_LDM);
            ob((a->type->size + 1) / 2);
            pw = pw + (a->type->size + 1) / 2;
        } else {
            gen_value(a);
            pw = pw + twords(a->type);
        }
        if (p)
            p = p->next;
    }
    if (ft->variadic) {
        lla(blk);
        pw++;
    }
    while (pw < rw) {
        ldc(0);
        pw++;
    }
    if (fs && n->a->val) {
        reloc(R_NEAR, fs->name);        /* the parser knows it is in this segment */
        ob(O_CGP);
        ob(0);
    } else if (fs) {
        reloc(R_CALL, fs->name);
        ob(O_CXP);
        ob(0);
        ob(0);
    } else {
        /* indirect call: patch the operands of the CXP that follows */
        gen_value(n->a);
        t = newtemp(1);
        gen_stl(t);
        ob(O_LPA);
        ob(0);
        A = E->pc;
        L = t <= 16 ? 1 : (t < 128 ? 2 : 3);
        ldc(4 + L);
        ob(O_ADI);
        ldl(t);
        ob(O_STO);
        if (E->pc != A + 3 + L)
            fatal(98 /* internal: indirect call layout */, 0);
        ob(O_CXP);
        ob(0);
        ob(0);
    }
    if (!want)
        drop(rw);
}

/* ---- intrinsics ---- */

static void gen_intrinsic(struct Node *n, int want)
{
    struct Node *a;
    int code;
    int num;
    code = n->val;
    a = n->a;
    if (code == 3) {            /* __va_start */
        ldl(n->val2);
        if (!want)
            drop(1);
        return;
    }
    if (code == 10) {           /* __exitprog */
        ldc(1);
        ldc(1);
        csp(CSP_EXIT);
        return;
    }
    if (code == 9 || code == 11) {  /* __osvar(n): OS global word n */
        ob(code == 9 ? O_LOD : O_LDA);
        ob(2);
        big(a->val);
        if (!want)
            drop(1);
        return;
    }
    num = 0;
    if (code >= 4) {
        num = a->val;
        a = a->next;
    }
    for (; a; a = a->next)
        gen_value(a);
    switch (code) {
    case 1: ob(O_DVI); break;
    case 2: ob(O_MODI); break;
    case 4: case 5: case 6: case 12: csp(num); break;
    case 7: case 8:
        ob(O_CXP);
        ob(0);
        ob(num);
        break;
    }
    if (!want)
        drop(valwords(n->type));
}

/* ---- expressions ---- */

static int isfconst(struct Node *n)
{
    return n->op == N_FNUM;
}

static void gen_fconst(unsigned char *f)
{
    ob(O_LDC);
    ob(2);
    if ((E->pc & 1) == 1)
        ob(0);
    ob(f[2]);
    ob(f[3]);
    ob(f[0]);
    ob(f[1]);
}

/* a double: LDC 4, the words last first (as for a REAL) */
static void gen_dconst(unsigned char *f)
{
    int w;
    ob(O_LDC);
    ob(4);
    if ((E->pc & 1) == 1)
        ob(0);
    for (w = 3; w >= 0; w--) {
        ob(f[2 * w]);
        ob(f[2 * w + 1]);
    }
}

/* doubles: CSP 100.. (the engine's NativeDouble.inc) */
#define CSP_DADD 100
#define CSP_DNEG 104
#define CSP_DCMP 105

/* DCMP's relation codes: 0 == 1 != 2 < 3 <= 4 > 5 >=, + 8 negates */
static int dblrel(int op)
{
    switch (op) {
    case N_EQ: return 0;
    case N_NE: return 1;
    case N_LT: return 2;
    case N_LE: return 3;
    case N_GT: return 4;
    }
    return 5;
}

/* conversions to and from double: CSP 106..111, 136, 137 */
static void gen_dcast(struct Node *n)
{
    struct Type *t;
    struct Type *f;
    int a;
    int b;
    t = n->type;
    f = n->a->type;
    if (isdblty(t) && isdblty(f)) {
        gen_value(n->a);
        return;
    }
    if (isdblty(t)) {
        if (isunsignedty(f) && !islongty(f))
            ldc(0);                     /* unsigned: as a long, high word 0 */
        gen_value(n->a);
        if (isfloatty(f))
            csp(106);                   /* FTOD */
        else if (islongty(f) && isunsignedty(f))
            csp(136);                   /* ULTOD */
        else if (islongty(f) || isunsignedty(f))
            csp(110);                   /* LTOD */
        else
            csp(108);                   /* ITOD */
        return;
    }
    gen_value(n->a);
    if (isfloatty(t)) {
        csp(107);                       /* DTOF */
        return;
    }
    if (islongty(t)) {
        csp(isunsignedty(t) ? 137 : 111);   /* DTOUL, DTOL */
        return;
    }
    if (isunsignedty(t) && t->kind != TY_UCHAR) {
        csp(111);                       /* DTOL, keep the low word */
        a = newtemp(1);
        b = newtemp(1);
        gen_stl(a);
        gen_stl(b);
        ldl(a);
        return;
    }
    csp(109);                           /* DTOI */
    if (t->kind == TY_UCHAR) {
        ldc(255);
        ob(O_LAND);
    } else if (t->kind == TY_CHAR)
        signext();
}

static void gen_str(struct Node *n)
{
    int i;
    if (n->slen > 255)
        error(99 /* string literal longer than 254 characters */, 0);
    ob(O_LPA);
    ob(n->slen);
    for (i = 0; i < n->slen; i++)
        ob(n->str[i]);
}

static int relop(int op, int isfloat, int invert)
{
    if (invert) {
        switch (op) {
        case N_EQ: op = N_NE; break;
        case N_NE: op = N_EQ; break;
        case N_LT: op = N_GE; break;
        case N_LE: op = N_GT; break;
        case N_GT: op = N_LE; break;
        case N_GE: op = N_LT; break;
        }
    }
    switch (op) {
    case N_EQ: return isfloat ? O_EQU : O_EQUI;
    case N_NE: return isfloat ? O_NEQ : O_NEQI;
    case N_LT: return isfloat ? O_LES : O_LESI;
    case N_LE: return isfloat ? O_LEQ : O_LEQI;
    case N_GT: return isfloat ? O_GRT : O_GRTI;
    }
    return isfloat ? O_GEQ : O_GEQI;
}

/* is n a char load whose sign does not matter against constant c? */
static int charsafe(struct Node *n, struct Node *c)
{
    while (n->op == N_CAST && isword(n->a->type))
        n = n->a;
    return n->type->kind == TY_CHAR && c->op == N_NUM && c->val >= 0 && c->val < 128;
}

/* push the two operands of a comparison, biased for unsigned order */
static void gen_cmpops(struct Node *n)
{
    int uns;
    int eq;
    struct Node *a;
    struct Node *b;
    a = n->a;
    b = n->b;
    eq = n->op == N_EQ || n->op == N_NE;
    uns = !eq && (isunsignedty(a->type) || a->type->kind == TY_PTR);
    if (eq && charsafe(a, b))
        gen_lowbyte(a);
    else
        gen_value(a);
    if (uns) {
        ldc(-32768);
        ob(O_ADI);
    }
    if (uns && b->op == N_NUM)
        ldc(b->val ^ -32768);
    else {
        if (eq && charsafe(b, a))
            gen_lowbyte(b);
        else
            gen_value(b);
        if (uns) {
            ldc(-32768);
            ob(O_ADI);
        }
    }
}

static void emitrelop(int op, struct Type *t, int invert)
{
    int o;
    if (isdblty(t)) {
        ldc(dblrel(op) + (invert ? 8 : 0));
        csp(CSP_DCMP);
        return;
    }
    o = relop(op, isfloatty(t), invert);
    ob(o);
    if (isfloatty(t))
        ob(2);
}

void branch(struct Node *n, int l, int jumpif)
{
    int skip;
    struct Type *t;
    switch (n->op) {
    case N_NUM:
        if (!islongty(n->type)) {
            if ((n->val != 0) == (jumpif != 0))
                jump(l);
            return;
        }
        break;
    case N_NOT:
        branch(n->a, l, !jumpif);
        return;
    case N_ANDAND:
        if (!jumpif) {
            branch(n->a, l, 0);
            branch(n->b, l, 0);
        } else {
            skip = newlabel();
            branch(n->a, skip, 0);
            branch(n->b, l, 1);
            setlabel(skip);
        }
        return;
    case N_OROR:
        if (jumpif) {
            branch(n->a, l, 1);
            branch(n->b, l, 1);
        } else {
            skip = newlabel();
            branch(n->a, skip, 1);
            branch(n->b, l, 0);
            setlabel(skip);
        }
        return;
    case N_EQ:
    case N_NE:
        t = n->a->type;
        if (!isfloatty(t)) {
            gen_cmpops(n);
            /* EFJ jumps if not equal, NFJ jumps if equal */
            if ((n->op == N_EQ) != (jumpif != 0))
                jmpop(O_EFJ, l);
            else
                jmpop(O_NFJ, l);
            return;
        }
        gen_cmpops(n);
        emitrelop(n->op, t, jumpif);
        jmpop(O_FJP, l);
        return;
    case N_LT:
    case N_LE:
    case N_GT:
    case N_GE:
        gen_cmpops(n);
        emitrelop(n->op, n->a->type, jumpif);
        jmpop(O_FJP, l);
        return;
    }
    t = n->type;
    if (isdblty(t)) {
        gen_value(n);
        gen_dconst((unsigned char *)"\0\0\0\0\0\0\0\0");
        ldc(jumpif ? 0 : 1);            /* == 0: FJP jumps when not; != 0 */
        csp(CSP_DCMP);
        jmpop(O_FJP, l);
        return;
    }
    if (isfloatty(t)) {
        gen_value(n);
        gen_fconst((unsigned char *)"\0\0\0\0");
        ob(jumpif ? O_EQU : O_NEQ);
        ob(2);
        jmpop(O_FJP, l);
        return;
    }
    if (islongty(t)) {
        gen_value(n);
        ob(O_LOR);
    } else
        gen_lowbyte(n);
    ldc(0);
    jmpop(jumpif ? O_EFJ : O_NFJ, l);
}

/* assignment family */
/* does the tree refer to the lvalue being updated (compound assignment)? */
static int haslvref(struct Node *n)
{
    for (; n; n = n->next) {
        if (n->op == N_LVREF)
            return 1;
        if ((n->a && haslvref(n->a)) || (n->b && haslvref(n->b)) || (n->c && haslvref(n->c)))
            return 1;
    }
    return 0;
}

static void gen_assign(struct Node *n, int want)
{
    struct Node *lhs;
    struct Node *rhs;
    struct Type *t;
    int ta;
    int w;
    int savet;
    struct Node *saven;
    lhs = n->a;
    rhs = n->b;
    t = lhs->type;
    if (n->op == N_ASSIGN && (t->kind == TY_STRUCT || t->kind == TY_UNION || t->kind == TY_ARRAY)) {
        if (t->kind == TY_ARRAY) {         /* char array = string literal (initialisers) */
            gen_value(rhs);
            ldc(0);
            gen_addr(lhs);
            ldc(0);
            ldc(rhs->slen < t->size ? rhs->slen : t->size);
            csp(CSP_MVL);
            return;
        }
        gen_addr(lhs);
        gen_value(rhs);
        opbig(O_MOV, (t->size + 1) / 2);
        if (want)
            gen_addr(lhs);
        return;
    }
    w = twords(t);
    savet = lvtemp;
    saven = lvnode;
    if (simplelv(lhs)) {
        lvtemp = 0;
        lvnode = lhs;
        if (n->op == N_POSTINC && want)
            gen_load(lhs);
        storepre(lhs);
        if (ischar(t))
            gen_lowbyte(rhs);
        else
            gen_value(rhs);
        storepost(lhs);
        if (want && n->op != N_POSTINC)
            gen_load(lhs);
    } else if (n->op == N_ASSIGN && !want && !haslvref(rhs)) {
        /* a plain store whose value is not used: the address stays on
           the stack (no temporary: STL t; SLDL t saved) */
        gen_addr(lhs);
        if (ischar(t)) {
            ldc(0);
            gen_lowbyte(rhs);
        } else
            gen_value(rhs);
        storeviapost(t);
    } else {
        ta = newtemp(1);
        gen_addr(lhs);
        gen_stl(ta);
        lvtemp = ta;
        lvnode = lhs;
        if (n->op == N_POSTINC && want)
            loadvia(ta, t);
        storeviapre(ta, t);
        if (ischar(t))
            gen_lowbyte(rhs);
        else
            gen_value(rhs);
        storeviapost(t);
        if (want && n->op != N_POSTINC)
            loadvia(ta, t);
    }
    lvtemp = savet;
    lvnode = saven;
    w = w;
}

void gen_value(struct Node *n)
{
    int l1;
    int l2;
    struct Type *t;
    t = n->type;
    switch (n->op) {
    case N_NUM:
        if (islongty(t)) {
            ldc(n->val2);
            ldc(n->val);
        } else
            ldc(n->val);
        return;
    case N_FNUM:
        if (isdblty(n->type))
            gen_dconst(n->fimg);
        else
            gen_fconst(n->fimg);
        return;
    case N_STR:
        gen_str(n);
        return;
    case N_HEAPSTR:
        {
            int t;
            t = newtemp(1);
            lla(t);
            ldc((n->slen + 1) / 2);
            csp(1);                     /* NEW */
            n->op = N_STR;
            gen_str(n);
            ldc(0);
            ldl(t);
            ldc(0);
            ldc(n->slen);
            csp(CSP_MVL);
            ldl(t);
        }
        return;
    case N_VAR:
    case N_MEMBER:
    case N_DEREF:
        gen_load(n);
        return;
    case N_LVREF:
        if (lvtemp)
            loadvia(lvtemp, lvnode->type);
        else
            gen_load(lvnode);
        return;
    case N_FUNC:
        ob(O_LDCI);
        reloc(R_FNPTR, n->sym->name);
        E->relpos[E->nrel - 1] = E->pc - 1;
        ob(0);
        ob(0);
        return;
    case N_ADDR:
        if (n->a->op == N_FUNC) {
            gen_value(n->a);
            return;
        }
        gen_addr(n->a);
        return;
    case N_CAST:
        gen_cast(n);
        return;
    case N_NEG:
        gen_value(n->a);
        if (isdblty(t))
            csp(CSP_DNEG);
        else
            ob(isfloatty(t) ? O_NGR : O_NGI);
        return;
    case N_BNOT:
        gen_value(n->a);
        ob(O_NOT);
        return;
    case N_NOT:
    case N_ANDAND:
    case N_OROR:
        l1 = newlabel();
        l2 = newlabel();
        branch(n, l1, 0);
        ldc(1);
        jump(l2);
        setlabel(l1);
        ldc(0);
        setlabel(l2);
        return;
    case N_EQ:
    case N_NE:
    case N_LT:
    case N_LE:
    case N_GT:
    case N_GE:
        gen_cmpops(n);
        emitrelop(n->op, n->a->type, 0);
        return;
    case N_COND:
        l1 = newlabel();
        l2 = newlabel();
        branch(n->a, l1, 0);
        gen_value(n->b);
        jump(l2);
        setlabel(l1);
        gen_value(n->c);
        setlabel(l2);
        return;
    case N_COMMA:
        gen_discard(n->a);
        gen_value(n->b);
        return;
    case N_ASSIGN:
    case N_OPASSIGN:
    case N_POSTINC:
        gen_assign(n, 1);
        return;
    case N_CALL:
        gen_call(n, 1);
        return;
    case N_INTRIN:
        gen_intrinsic(n, 1);
        return;
    case N_ADD:
    case N_SUB:
    case N_MUL:
        gen_value(n->a);
        if (n->op == N_ADD && t->kind == TY_PTR && n->b->op == N_MUL && n->b->b->op == N_NUM &&
            n->b->b->val > 0 && !(n->b->b->val & 1)) {
            /* pointer + index * (a whole number of words): IXA */
            gen_value(n->b->a);
            opbig(O_IXA, n->b->b->val / 2);
            return;
        }
        if (n->op == N_ADD && n->b->op == N_NUM && !isfloatty(t) && n->b->val > 0 && !(n->b->val & 1) && n->b->val < 256) {
            opbig(O_INC, n->b->val / 2);
            return;
        }
        gen_value(n->b);
        if (isdblty(t))
            csp(CSP_DADD + (n->op == N_ADD ? 0 : (n->op == N_SUB ? 1 : 2)));
        else if (isfloatty(t))
            ob(n->op == N_ADD ? O_ADR : (n->op == N_SUB ? O_SBR : O_MPR));
        else
            ob(n->op == N_ADD ? O_ADI : (n->op == N_SUB ? O_SBI : O_MPI));
        return;
    case N_DIV:
        gen_value(n->a);
        gen_value(n->b);
        if (isdblty(t))
            csp(CSP_DADD + 3);
        else
            ob(O_DVR);
        return;
    case N_AND:
        gen_value(n->a);
        gen_value(n->b);
        ob(O_LAND);
        return;
    case N_OR:
        gen_value(n->a);
        gen_value(n->b);
        ob(O_LOR);
        return;
    }
    error(100 /* internal: cannot generate node */, 0);
}

void gen_discard(struct Node *n)
{
    switch (n->op) {
    case N_ASSIGN:
    case N_OPASSIGN:
    case N_POSTINC:
        gen_assign(n, 0);
        return;
    case N_CALL:
        gen_call(n, 0);
        return;
    case N_INTRIN:
        gen_intrinsic(n, 0);
        return;
    case N_COMMA:
        gen_discard(n->a);
        gen_discard(n->b);
        return;
    case N_CAST:
        if (n->type->kind == TY_VOID) {
            gen_discard(n->a);
            return;
        }
        break;
    case N_COND:
        {
            int l1;
            int l2;
            l1 = newlabel();
            l2 = newlabel();
            branch(n->a, l1, 0);
            gen_discard(n->b);
            jump(l2);
            setlabel(l1);
            gen_discard(n->c);
            setlabel(l2);
        }
        return;
    case N_NUM:
    case N_VAR:
        return;
    }
    gen_value(n);
    drop(valwords(n->type));
}

void gen_return(struct Node *n, struct Type *ft, int sretoff)
{
    int rw;
    struct Type *t;
    if (!n)
        return;
    t = ft->base;
    if (t->kind == TY_STRUCT || t->kind == TY_UNION) {
        ldl(sretoff);
        gen_value(n);
        opbig(O_MOV, (t->size + 1) / 2);
        ldl(sretoff);
        gen_stl(1);
        return;
    }
    rw = retwords(ft);
    if (rw == 1) {
        gen_value(n);
        gen_stl(1);
    } else {
        lla(1);
        gen_value(n);
        ob(O_STM);
        ob(rw);
    }
}

void gen_switch(int t, int *vals, int *labs, int n, int deflab)
{
    int lo;
    int hi;
    int i;
    int v;
    int range;
    if (n == 0) {
        jump(deflab);
        return;
    }
    lo = vals[0];
    hi = vals[0];
    for (i = 1; i < n; i++) {
        if (vals[i] < lo)
            lo = vals[i];
        if (vals[i] > hi)
            hi = vals[i];
    }
    range = hi - lo + 1;
    if (n >= 4 && range > 0 && range <= 3 * n + 6 && range < 1000) {
        ldl(t);
        ob(O_XJP);
        if (E->pc & 1)
            ob(0);
        ob(lo & 255);
        ob((lo >> 8) & 255);
        ob(hi & 255);
        ob((hi >> 8) & 255);
        jump(deflab);
        for (v = lo; v <= hi; v++) {
            for (i = 0; i < n; i++)
                if (vals[i] == v)
                    break;
            caseword(i < n ? labs[i] : deflab);
        }
        return;
    }
    for (i = 0; i < n; i++) {
        ldl(t);
        ldc(vals[i]);
        jmpop(O_NFJ, labs[i]);
    }
    jump(deflab);
}

/* ---- the object file ---- */

static void outw(int w)
{
    putc(w & 255, objout);
    putc((w >> 8) & 255, objout);
}

static void outs(char *s)
{
    int n;
    n = strlen(s);
    putc(n, objout);
    while (*s)
        putc(*s++, objout);
}

static char *genmod;

void gen_objheader(char *modname)
{
    genmod = modname;
    names = (struct Name **)calloc(NHASH, sizeof(struct Name *));
    if (!names)
        fatal(2 /* out of memory */, 0);
    ninit = 0;
    ininit = 0;
    lvtemp = 0;
    lvnode = 0;
    setupemit(&fe, MAXCODE, MAXLABEL, MAXFIX, MAXREL);
    setupemit(&ie, 1600, 40, 80, 300);
    E = &fe;
    fputs("TCOB", objout);
    putc('M', objout);
    outs(modname);
}

void gen_objdata(char *name, int words, int strong)
{
    putc('D', objout);
    putc(strong, objout);
    outs(name);
    outw(words);
}

void gen_objuse(char *name)
{
    putc('U', objout);
    outs(name);
}

void gen_objend(int staticwords)
{
    putc('G', objout);
    outw(staticwords);
    putc('E', objout);
}

/* finish the procedure in E and write it: flags 1 = init, 2 = static */
static void endproc(char *name, char *seg, int exitlab, int rw, int flags)
{
    int longlab[MAXLONGJ];
    int nlong;
    int i;
    int k;
    int t;
    int pos;
    int off;
    int jtab;
    int exitpos;
    int base;
    exitpos = E->labpos[exitlab];
    /* resolve jumps */
    nlong = 0;
    for (i = 0; i < E->nfix; i++) {
        pos = E->fixpos[i];
        if (E->fixlab[i] <= -2)
            continue;
        t = E->labpos[E->fixlab[i]];
        if (t < 0) {
            error(101 /* internal: undefined label */, name);
            continue;
        }
        off = t - (pos + 1);
        if (off >= 0 && off <= 127) {
            E->code[pos] = off;
            continue;
        }
        for (k = 0; k < nlong; k++)
            if (longlab[k] == t)
                break;
        if (k == nlong) {
            if (nlong >= MAXLONGJ)
                fatal(102 /* function too large (more than 60 long jumps); split it */, name);
            longlab[nlong++] = t;
        }
        E->code[pos] = (256 - 10 - 2 * k) & 255;
    }
    if (E->pc & 1)
        ob(0);
    base = E->pc;
    jtab = base + 2 * nlong + 8;
    for (k = nlong - 1; k >= 0; k--) {
        pos = jtab - 10 - 2 * k;
        ob(pos - longlab[k]);
        ob((pos - longlab[k]) >> 8);
    }
    ob((maxlocal - nparamwords) * 2);
    ob(((maxlocal - nparamwords) * 2) >> 8);
    ob(nparamwords * 2);
    ob((nparamwords * 2) >> 8);
    ob(jtab - 4 - exitpos);
    ob((jtab - 4 - exitpos) >> 8);
    ob(jtab - 2);
    ob((jtab - 2) >> 8);
    ob(0);                          /* procedure number: the linker */
    ob(1);                          /* lex level */
    /* case tables (after everything else is placed) */
    for (i = 0; i < E->nfix; i++) {
        if (E->fixlab[i] > -2)
            continue;
        pos = E->fixpos[i];
        t = E->labpos[-2 - E->fixlab[i]];
        E->code[pos] = (pos - t) & 255;
        E->code[pos + 1] = ((pos - t) >> 8) & 255;
    }
    putc('P', objout);
    putc(flags, objout);
    outs(name);
    outs(seg);
    outw(nparamwords * 2);
    putc(rw, objout);
    outw(E->pc);
    outw(jtab);
    for (i = 0; i < E->pc; i++)
        putc(E->code[i], objout);
    outw(E->nrel);
    for (i = 0; i < E->nrel; i++) {
        outw(E->relpos[i]);
        putc(E->reltype[i], objout);
        outs(E->relname[i]);
    }
}

void gen_funcbegin(void)
{
    E = &fe;
    E->pc = 0;
    E->nlab = 0;
    E->nfix = 0;
    E->nrel = 0;
    lvtemp = 0;
}

void gen_funcend(char *name, struct Type *ft, int exitlab, int isstatic, char *seg)
{
    int rw;
    rw = retwords(ft);
    setlabel(exitlab);
    ob(O_RNP);
    ob(rw);
    endproc(name, seg, exitlab, rw, isstatic ? 2 : 0);
}

/* ---- initialisers ---- */

static int initexit;

static void initstart(void)
{
    E->pc = 0;
    E->nlab = 0;
    E->nfix = 0;
    E->nrel = 0;
    curlocal = 0;
    scratch = 1;
    curlocal = 1;
    maxlocal = 1;
    nparamwords = 0;
    initexit = newlabel();
}

void gen_initflush(void)
{
    char name[40];
    struct Emit *save;
    save = E;
    saveframe();
    E = &ie;
    if (ininit == 0 && E->pc > 0) {
        loadframe();
        setlabel(initexit);
        ob(O_RNP);
        ob(0);
        strcpy(name, genmod);
        strcat(name, "'init");
        itoa10(ninit, name + strlen(name));
        ninit++;
        endproc(name, "INIT", initexit, 0, 1);
        E->pc = 0;
    }
    E = save;
    loadframe();
}

void gen_initbegin(void)
{
    if (ininit++)
        return;
    saveframe();
    E = &ie;
    if (E->pc == 0)
        initstart();
    else
        loadframe();
}

void gen_initend(void)
{
    if (--ininit)
        return;
    saveframe();
    E = &fe;
    loadframe();
    if (ie.pc > ie.max - 500 || ie.nrel > ie.maxrel - 60)
        gen_initflush();
}
