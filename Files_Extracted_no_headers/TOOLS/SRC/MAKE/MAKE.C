/* make.c -- MAKE [-f FILE] [-nqBsik] [NAME=VALUE ...] [TARGET ...]: brings targets up to
   date from their prerequisites, as make(1) does, by the files' dates and times of day
   (the systems of OS source revision 1.16 keep the time: SYSTEM.C, SYSTEM.PASCAL with
   emulator 2.13).

   The makefile is MAKEFILE.TEXT on the prefix volume (or the shell's path), or -f FILE
   (FILE, or FILE.TEXT).  What it may hold:
     NAME = VALUE     NAME := VALUE    NAME += VALUE    NAME ?= VALUE    (NAME=VALUE on the
                      command line wins)
     TARGET ... : PREREQUISITE ... [; RECIPE]
     <blank or tab>RECIPE                  a recipe line: a shell command
     %.OBJ : %.C      a pattern rule ($* the stem); .C.OBJ : a suffix rule, the same
     .PHONY : NAME ...                     names that are not files
     include FILE     ifeq (A,B)  ifneq (A,B)  ifdef NAME  ifndef NAME  else  endif
     # a comment (but #5: is a unit)       \ at a line's end: it goes on
   $(NAME) ${NAME} $N; the automatic variables $@ (the target) $< (the first prerequisite)
   $^ (all) $? (the newer ones) $* (the stem); the functions subst patsubst strip
   wildcard basename notdir dir suffix addprefix addsuffix filter filter-out sort words
   firstword if, and $(NAME:A=B).  comma and space hold , and a blank (for
   $(subst $(space),$(comma),...)); MAKE is MAKE.  Built-in rules: %.OBJ from %.C
   (CC -C $*) and %.CODE from %.C (CC $*).

   A target is made when its file is not there, or a prerequisite is newer (to the
   second; a file without a time of day is the start of its day), or it is .PHONY.
   What is to be made is all decided first; a target to be made counts as newer than
   everything, so what depends on it is made too.  A rule's colon must be followed by a
   blank, a ; or the line's end (TOOLS:VI.CODE : VI.OBJ is a target on TOOLS:); names
   compare without regard to case.

   One program runs at a time here, so MAKE does not run the recipes itself: it writes
   them into #4:MAKEn.TEXT and has the shell run them after it ends, as a batch file
   (@): each shown as it starts; one that ends with an exit status other than 0 stops
   them -- unless it starts with - (or -i).  A line starting @ is not shown in make(1):
   here the @ is taken off (the shell shows every line).  A CD in a recipe stays for the
   lines after it.  MAKE must be run from the shell (S).
   Options: -n the recipes shown, not run; -q nothing run, exit status 0 when all is up
   to date, 1 when not; -B every target made; -i errors ignored; -s, -k ignored.
   Exit status 2 on an error (no rule for a file that is not there ...). */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <psys.h>
#include <dirs.h>

#define LINEMAX 1100
#define BATCHFILE "#4:SYSTEM.BATCH"
#define NEWEST 0xFFFFFFFFUL             /* a target to be made: newer than anything */

struct var {
    char *name;
    char *value;
    int simple;                         /* := (expanded already) */
    int cmdline;                        /* NAME=VALUE on the command line: wins */
    struct var *next;
};

struct dep {
    char *name;
    struct dep *next;
};

struct cmd {
    char *text;
    struct cmd *next;
};

struct target {
    char *name;
    struct dep *deps;
    struct cmd *cmds;
    int hasrule;                        /* named in a rule */
    int phony;
    int state;                          /* 0 not seen, 1 being made, 2 done */
    unsigned long time;                 /* after: its time, NEWEST when it is made */
    struct target *next;
};

struct prule {
    char *tpat;                         /* %.OBJ */
    struct dep *deps;                   /* %.C ... */
    struct cmd *cmds;
    struct prule *next;
};

struct var *vars;
struct target *targets;
struct target *lasttarget;
struct prule *prules;
struct prule *lastprule;
char *defgoal;
int optn, optq, optB, opti;
int status;
FILE *out;                              /* the batch file being written (or NULL: -n, -q) */
int nmade;                              /* recipe lines written */
char *mfname;

void die(char *msg, char *arg)
{
    fprintf(stderr, "make: %s%s\n", msg, arg);
    exit(2);
}

void *xmalloc(unsigned n)
{
    void *p;
    p = malloc(n);
    if (!p)
        die("no memory", "");
    return p;
}

char *xstrdup(char *s)
{
    char *p;
    p = (char *)xmalloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}

/* ---- growing strings ---- */

struct buf {
    char *s;
    int n, cap;
};

void binit(struct buf *b)
{
    b->cap = 64;
    b->s = (char *)xmalloc(b->cap);
    b->n = 0;
    b->s[0] = 0;
}

void baddn(struct buf *b, char *s, int n)
{
    char *t;
    if (b->n + n + 1 > b->cap) {
        while (b->n + n + 1 > b->cap)
            b->cap = b->cap * 2;
        t = (char *)xmalloc(b->cap);
        memcpy(t, b->s, b->n + 1);
        free(b->s);
        b->s = t;
    }
    memcpy(b->s + b->n, s, n);
    b->n = b->n + n;
    b->s[b->n] = 0;
}

void badd(struct buf *b, char *s)
{
    baddn(b, s, strlen(s));
}

void baddc(struct buf *b, int c)
{
    char t[2];
    t[0] = c;
    t[1] = 0;
    baddn(b, t, 1);
}

/* ---- names ---- */

/* the first n characters the same, without regard to case */
int nsame(char *a, char *b, int n)
{
    while (n-- > 0) {
        if (toupper(*a) != toupper(*b))
            return 0;
        if (!*a)
            return 1;
        a++;
        b++;
    }
    return 1;
}

int same(char *a, char *b)
{
    while (*a && toupper(*a) == toupper(*b)) {
        a++;
        b++;
    }
    return toupper(*a) == toupper(*b);
}

/* name against a pattern with one %: the stem in stem; 1 if it matches */
int pmatch(char *pat, char *name, char *stem)
{
    char *pc;
    int pre, post, n;
    pc = strchr(pat, '%');
    if (!pc)
        return same(pat, name);
    pre = pc - pat;
    post = strlen(pc + 1);
    n = strlen(name);
    if (n < pre + post)
        return 0;
    if (!nsame(pat, name, pre) || !same(pc + 1, name + n - post))
        return 0;
    if (stem) {
        memcpy(stem, name + pre, n - pre - post);
        stem[n - pre - post] = 0;
    }
    return 1;
}

/* pat with its % replaced by stem */
void psubst(char *pat, char *stem, struct buf *b)
{
    char *pc;
    pc = strchr(pat, '%');
    if (!pc) {
        badd(b, pat);
        return;
    }
    baddn(b, pat, pc - pat);
    badd(b, stem);
    badd(b, pc + 1);
}

/* ---- variables ---- */

struct var *findvar(char *name)
{
    struct var *v;
    for (v = vars; v; v = v->next)
        if (strcmp(v->name, name) == 0)
            return v;
    return NULL;
}

void setvar(char *name, char *value, int simple, int cmdline)
{
    struct var *v;
    v = findvar(name);
    if (v && v->cmdline && !cmdline)
        return;
    if (!v) {
        v = (struct var *)xmalloc(sizeof(struct var));
        v->name = xstrdup(name);
        v->next = vars;
        vars = v;
    } else
        free(v->value);
    v->value = xstrdup(value);
    v->simple = simple;
    v->cmdline = cmdline;
}

/* the automatic variables of the recipe being expanded */
struct target *at;
char *astem;
char *afirst;
char *anewer;
char *aall;

char *expand(char *s);

/* the next word of *sp into w (255 characters at most); 0 at the end */
int nextword(char **sp, char *w)
{
    char *s;
    int n;
    s = *sp;
    while (*s == ' ' || *s == '\t')
        s++;
    if (!*s)
        return 0;
    n = 0;
    while (*s && *s != ' ' && *s != '\t' && n < 255)
        w[n++] = *s++;
    w[n] = 0;
    *sp = s;
    return 1;
}

void addword(struct buf *b, char *w)
{
    if (b->n > 0)
        baddc(b, ' ');
    badd(b, w);
}

/* a function's arguments: split at the top level's commas (n at most) */
int fargs(char *s, char **arg, int max)
{
    int n, depth;
    n = 0;
    arg[n++] = s;
    depth = 0;
    for (; *s; s++) {
        if (*s == '(' || *s == '{')
            depth++;
        else if (*s == ')' || *s == '}')
            depth--;
        else if (*s == ',' && depth == 0 && n < max) {
            *s = 0;
            arg[n++] = s + 1;
        }
    }
    return n;
}

/* $(wildcard PAT): its files */
struct buf *wbuf;
int addfile(char *name)
{
    addword(wbuf, name);
    return 0;
}

int rmatch(char *pat, char *w)
{
    return strchr(pat, '%') ? pmatch(pat, w, NULL) : same(pat, w);
}

/* a function call: name and the text after it */
int function(char *name, char *rest, struct buf *b)
{
    char *arg[3];
    char w[256];
    char stem[256];
    char *s;
    char *t;
    char *p;
    char *list[200];
    int n, i, j, k;
    struct buf r;
    static char *names[] = { "subst", "patsubst", "strip", "wildcard", "basename", "notdir", "dir",
                             "suffix", "addprefix", "addsuffix", "filter", "filter-out", "sort",
                             "words", "firstword", "if", "shell", 0 };
    for (k = 0; names[k]; k++)
        if (strcmp(names[k], name) == 0)
            break;
    if (!names[k])
        return 0;
    if (k == 16)
        die("$(shell ...) is not possible here: one program runs at a time", "");
    n = fargs(rest, arg, k == 0 || k == 1 || k == 15 ? 3 : (k == 8 || k == 9 || k == 10 || k == 11) ? 2 : 1);
    for (i = 0; i < n; i++) {
        if (k == 15 && i > 0)
            break;                      /* $(if): only the branch taken */
        arg[i] = expand(arg[i]);
    }
    binit(&r);
    switch (k) {
    case 0:                             /* subst FROM,TO,TEXT */
        if (n < 3)
            break;
        s = arg[2];
        j = strlen(arg[0]);
        while (*s) {
            if (j > 0 && strncmp(s, arg[0], j) == 0) {
                badd(&r, arg[1]);
                s = s + j;
            } else
                baddc(&r, *s++);
        }
        break;
    case 1:                             /* patsubst PAT,REPL,TEXT */
        if (n < 3)
            break;
        s = arg[2];
        while (nextword(&s, w))
            if (pmatch(arg[0], w, stem)) {
                if (r.n)
                    baddc(&r, ' ');
                psubst(arg[1], stem, &r);
            } else
                addword(&r, w);
        break;
    case 2:                             /* strip */
        s = arg[0];
        while (nextword(&s, w))
            addword(&r, w);
        break;
    case 3:                             /* wildcard */
        s = arg[0];
        wbuf = &r;
        while (nextword(&s, w))
            if (strpbrk(w, "*?="))
                forfiles("make", w, addfile);
            else if (filetime(w))
                addword(&r, w);
        break;
    case 4: case 5: case 6: case 7:     /* basename notdir dir suffix */
        s = arg[0];
        while (nextword(&s, w)) {
            p = NULL;                   /* the last / or : */
            for (t = w; *t; t++)
                if (*t == '/' || *t == ':')
                    p = t;
            t = strrchr(p ? p : w, '.');
            if (k == 4) {
                if (t)
                    *t = 0;
                addword(&r, w);
            } else if (k == 5)
                addword(&r, p ? p + 1 : w);
            else if (k == 6) {
                if (p)
                    p[1] = 0;
                addword(&r, p ? w : "./");
            } else if (t)
                addword(&r, t);
        }
        break;
    case 8: case 9:                     /* addprefix P,LIST  addsuffix S,LIST */
        if (n < 2)
            break;
        s = arg[1];
        while (nextword(&s, w)) {
            if (r.n)
                baddc(&r, ' ');
            if (k == 8)
                badd(&r, arg[0]);
            badd(&r, w);
            if (k == 9)
                badd(&r, arg[0]);
        }
        break;
    case 10: case 11:                   /* filter PATS,TEXT  filter-out */
        if (n < 2)
            break;
        s = arg[1];
        while (nextword(&s, w)) {
            t = arg[0];
            j = 0;
            while (!j && nextword(&t, stem))
                j = rmatch(stem, w);
            if (j == (k == 10))
                addword(&r, w);
        }
        break;
    case 12:                            /* sort: and duplicates out */
        s = arg[0];
        j = 0;
        while (j < 200 && nextword(&s, w))
            list[j++] = xstrdup(w);
        for (i = 1; i < j; i++)
            for (n = i; n > 0 && strcmp(list[n - 1], list[n]) > 0; n--) {
                t = list[n];
                list[n] = list[n - 1];
                list[n - 1] = t;
            }
        for (i = 0; i < j; i++) {
            if (i == 0 || strcmp(list[i], list[i - 1]) != 0)
                addword(&r, list[i]);
            free(list[i]);
        }
        break;
    case 13:                            /* words */
        s = arg[0];
        j = 0;
        while (nextword(&s, w))
            j++;
        sprintf(w, "%d", j);
        badd(&r, w);
        break;
    case 14:                            /* firstword */
        s = arg[0];
        if (nextword(&s, w))
            badd(&r, w);
        break;
    case 15:                            /* if COND,THEN,ELSE */
        s = arg[0];
        i = nextword(&s, w) ? 1 : 2;
        if (i < n) {
            t = expand(arg[i]);
            badd(&r, t);
            free(t);
        }
        n = 1;
        break;
    }
    badd(b, r.s);
    free(r.s);
    for (i = 0; i < n; i++)
        free(arg[i]);
    return 1;
}

/* $(NAME), $(NAME:A=B), $(function ...): the text between the brackets */
void reference(char *ref, struct buf *b)
{
    char *colon;
    char *eq;
    char *sp;
    char *v;
    char *t;
    char w[256];
    char stem[256];
    struct var *var;
    struct buf r;
    sp = ref;
    while (*sp && *sp != ' ' && *sp != '\t' && *sp != ':')
        sp++;
    if (*sp == ' ' || *sp == '\t') {    /* a function */
        *sp = 0;
        if (function(ref, sp + 1, b))
            return;
        *sp = ' ';
    }
    colon = NULL;
    eq = NULL;
    if (!strchr(ref, '$')) {
        colon = strchr(ref, ':');
        if (colon)
            eq = strchr(colon, '=');
    }
    if (colon && eq)
        *colon = 0;
    if (strcmp(ref, "@") == 0)
        v = xstrdup(at ? at->name : "");
    else if (strcmp(ref, "<") == 0)
        v = xstrdup(afirst ? afirst : "");
    else if (strcmp(ref, "^") == 0 || strcmp(ref, "+") == 0)
        v = xstrdup(aall ? aall : "");
    else if (strcmp(ref, "?") == 0)
        v = xstrdup(anewer ? anewer : "");
    else if (strcmp(ref, "*") == 0)
        v = xstrdup(astem ? astem : "");
    else {
        t = expand(ref);
        var = findvar(t);
        free(t);
        if (!var)
            v = xstrdup("");
        else if (var->simple)
            v = xstrdup(var->value);
        else
            v = expand(var->value);
    }
    if (colon && eq) {                  /* $(NAME:A=B) */
        *eq = 0;
        binit(&r);
        t = v;
        while (nextword(&t, w)) {
            if (strchr(colon + 1, '%')) {
                if (pmatch(colon + 1, w, stem)) {
                    if (r.n)
                        baddc(&r, ' ');
                    psubst(eq + 1, stem, &r);
                    continue;
                }
            } else if (strlen(w) >= strlen(colon + 1) && same(w + strlen(w) - strlen(colon + 1), colon + 1)) {
                w[strlen(w) - strlen(colon + 1)] = 0;
                if (r.n)
                    baddc(&r, ' ');
                badd(&r, w);
                badd(&r, eq + 1);
                continue;
            }
            addword(&r, w);
        }
        free(v);
        v = r.s;
    }
    badd(b, v);
    free(v);
}

int depth;

/* s with its references expanded: a new string */
char *expand(char *s)
{
    struct buf b;
    struct buf ref;
    char one[2];
    int d, open, close;
    if (++depth > 40)
        die("a variable refers to itself (recursion)", "");
    binit(&b);
    while (*s) {
        if (*s != '$') {
            baddc(&b, *s++);
            continue;
        }
        s++;
        if (*s == '$') {
            baddc(&b, '$');
            s++;
        } else if (*s == '(' || *s == '{') {
            open = *s;
            close = open == '(' ? ')' : '}';
            s++;
            binit(&ref);
            d = 1;
            while (*s) {
                if (*s == open)
                    d++;
                else if (*s == close && --d == 0)
                    break;
                baddc(&ref, *s++);
            }
            if (*s)
                s++;
            reference(ref.s, &b);
            free(ref.s);
        } else if (*s) {
            one[0] = *s++;
            one[1] = 0;
            reference(one, &b);
        }
    }
    depth--;
    return b.s;
}

/* ---- targets and rules ---- */

struct target *findtarget(char *name, int make)
{
    struct target *t;
    for (t = targets; t; t = t->next)
        if (same(t->name, name))
            return t;
    if (!make)
        return NULL;
    t = (struct target *)xmalloc(sizeof(struct target));
    memset(t, 0, sizeof(struct target));
    t->name = xstrdup(name);
    if (lasttarget)
        lasttarget->next = t;
    else
        targets = t;
    lasttarget = t;
    return t;
}

void adddeps(struct dep **list, char *words)
{
    char w[256];
    struct dep *d;
    struct dep **end;
    for (end = list; *end; end = &(*end)->next)
        ;
    while (nextword(&words, w)) {
        d = (struct dep *)xmalloc(sizeof(struct dep));
        d->name = xstrdup(w);
        d->next = NULL;
        *end = d;
        end = &d->next;
    }
}

void addcmd(struct cmd **list, char *text)
{
    struct cmd *c;
    struct cmd **end;
    for (end = list; *end; end = &(*end)->next)
        ;
    c = (struct cmd *)xmalloc(sizeof(struct cmd));
    c->text = xstrdup(text);
    c->next = NULL;
    *end = c;
}

void recipeline(char *text);

/* the rules a recipe line goes to: the targets of the last rule line */
struct target *rtargets[40];
int nrtargets;
struct prule *rprule;

/* a suffix rule's name (.C.OBJ): its two suffixes; 1 if it is one */
int suffixrule(char *name, char *from, char *to)
{
    char *p;
    if (name[0] != '.' || strchr(name, '%') || strchr(name, '/') || strchr(name, ':'))
        return 0;
    p = strchr(name + 1, '.');
    if (!p || strchr(p + 1, '.'))
        return 0;
    memcpy(from, name, p - name);
    from[p - name] = 0;
    strcpy(to, p);
    return 1;
}

struct prule *addprule(char *tpat, char *deps)
{
    struct prule *r;
    r = (struct prule *)xmalloc(sizeof(struct prule));
    r->tpat = xstrdup(tpat);
    r->deps = NULL;
    r->cmds = NULL;
    r->next = NULL;
    adddeps(&r->deps, deps);
    if (lastprule)
        lastprule->next = r;
    else
        prules = r;
    lastprule = r;
    return r;
}

void rule(char *tlist, char *deps, char *recipe)
{
    char w[256];
    char from[32];
    char to[32];
    char pat[40];
    char dpat[40];
    char *s;
    struct target *t;
    struct dep *d;
    nrtargets = 0;
    rprule = NULL;
    s = tlist;
    while (nextword(&s, w)) {
        if (strcmp(w, ".PHONY") == 0) {
            s = deps;
            while (nextword(&s, w))
                findtarget(w, 1)->phony = 1;
            return;
        }
        if (strcmp(w, ".SUFFIXES") == 0 || strcmp(w, ".SILENT") == 0 || strcmp(w, ".PRECIOUS") == 0 ||
            strcmp(w, ".DEFAULT") == 0 || strcmp(w, ".IGNORE") == 0)
            return;
        if (suffixrule(w, from, to) && !*deps) {    /* .C.OBJ: %.OBJ: %.C */
            sprintf(pat, "%%%s", to);
            sprintf(dpat, "%%%s", from);
            rprule = addprule(pat, dpat);
            continue;
        }
        if (strchr(w, '%')) {
            rprule = addprule(w, deps);
            continue;
        }
        t = findtarget(w, 1);
        t->hasrule = 1;
        adddeps(&t->deps, deps);
        if (nrtargets < 40)
            rtargets[nrtargets++] = t;
        if (!defgoal && w[0] != '.')
            defgoal = t->name;
    }
    if (recipe)
        recipeline(recipe);
}

void recipeline(char *text)
{
    int i;
    if (rprule)
        addcmd(&rprule->cmds, text);
    for (i = 0; i < nrtargets; i++)
        if (!rtargets[i]->cmds || i == 0 || rtargets[i]->cmds != rtargets[0]->cmds)
            addcmd(&rtargets[i]->cmds, text);
}

/* ---- the makefile ---- */

/* the place of the rule's colon in s (outside $(...)): one followed by a blank, ;,
   = or the end (a volume's VOL: is followed by its name); NULL none */
char *rulecolon(char *s, int *assign)
{
    int d;
    char *p;
    d = 0;
    *assign = 0;
    for (p = s; *p; p++) {
        if (*p == '(' || *p == '{')
            d++;
        else if (*p == ')' || *p == '}')
            d--;
        else if (d == 0 && *p == '=') {
            *assign = 1;
            return p;
        } else if (d == 0 && *p == ':') {
            if (p[1] == '=' || (p[1] == ':' && p[2] == '=')) {
                *assign = 1;
                return p;
            }
            if (p[1] == ':')
                p[1] = ' ';             /* :: as : */
            if (p[1] == 0 || p[1] == ' ' || p[1] == '\t' || p[1] == ';')
                return p;
        }
    }
    return NULL;
}

/* # starts a comment, but not #n: (a unit) nor \# */
void uncomment(char *s)
{
    char *p;
    char *q;
    for (p = s; *p; p++) {
        if (*p == '\\' && p[1] == '#') {
            memmove(p, p + 1, strlen(p));
            continue;
        }
        if (*p != '#')
            continue;
        q = p + 1;
        while (isdigit(*q))
            q++;
        if (q > p + 1 && *q == ':')
            continue;
        *p = 0;
        return;
    }
}

char *trim(char *s)
{
    int n;
    while (*s == ' ' || *s == '\t')
        s++;
    n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t'))
        s[--n] = 0;
    return s;
}

/* ifeq (A,B) / ifeq "A" "B": 1 when the two are the same */
int cond(char *kw, char *rest)
{
    char *a;
    char *b;
    char *x;
    char *y;
    char *p;
    int r, d;
    rest = trim(rest);
    if (strcmp(kw, "ifdef") == 0 || strcmp(kw, "ifndef") == 0) {
        x = expand(rest);
        r = findvar(trim(x)) && *findvar(trim(x))->value;
        free(x);
        return strcmp(kw, "ifdef") == 0 ? r : !r;
    }
    a = rest;
    b = "";
    if (*rest == '(') {
        a = rest + 1;
        d = 0;
        for (p = a; *p; p++) {
            if (*p == '(')
                d++;
            else if (*p == ')' && d-- == 0)
                break;
            else if (*p == ',' && d == 0) {
                *p = 0;
                b = p + 1;
            }
        }
        *p = 0;
    } else
        die("ifeq wants (A,B): ", rest);
    x = expand(trim(a));
    y = expand(trim(b));
    r = strcmp(x, y) == 0;
    free(x);
    free(y);
    return strcmp(kw, "ifeq") == 0 ? r : !r;
}

void readmake(char *name, int must)
{
    FILE *f;
    char line[LINEMAX];
    struct buf l;
    char *s;
    char *p;
    char *c;
    char *v;
    char kw[8];
    int n, inrule, assign, skip, level, k;
    int ifs[16];                        /* per level: taking its lines */
    f = fopen(name, "r");
    if (!f) {
        if (must)
            die("cannot open the makefile ", name);
        return;
    }
    inrule = 0;
    level = 0;
    skip = 0;
    while (fgets(line, LINEMAX, f)) {
        binit(&l);
        for (;;) {                      /* \ at the end: the next line goes on */
            n = strlen(line);
            while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r'))
                line[--n] = 0;
            if (n > 0 && line[n - 1] == '\\') {
                line[n - 1] = ' ';
                badd(&l, line);
                if (!fgets(line, LINEMAX, f))
                    break;
                continue;
            }
            badd(&l, line);
            break;
        }
        s = l.s;
        if ((*s == '\t' || *s == ' ') && inrule && *trim(s) && !skip) {   /* a recipe line */
            if (*trim(s) != '#')        /* (a comment there: not a command) */
                recipeline(trim(s));
            free(l.s);
            continue;
        }
        uncomment(s);
        s = trim(s);
        for (k = 0; k < 7 && isalpha(s[k]); k++)
            kw[k] = s[k];
        kw[k] = 0;
        if (strcmp(kw, "ifeq") == 0 || strcmp(kw, "ifneq") == 0 || strcmp(kw, "ifdef") == 0 || strcmp(kw, "ifndef") == 0) {
            if (level >= 15)
                die("conditionals too deep", "");
            ifs[++level] = skip ? 2 : cond(kw, s + k);  /* 2: inside a skipped one */
            skip = ifs[level] != 1;
            for (n = 1; n < level; n++)
                if (ifs[n] != 1)
                    skip = 1;
            free(l.s);
            continue;
        }
        if (strcmp(s, "else") == 0 || strcmp(s, "endif") == 0) {
            if (level == 0)
                die("else or endif without an if", "");
            if (strcmp(s, "else") == 0)
                ifs[level] = ifs[level] == 2 ? 2 : !ifs[level];
            else
                level--;
            skip = 0;
            for (n = 1; n <= level; n++)
                if (ifs[n] != 1)
                    skip = 1;
            free(l.s);
            continue;
        }
        if (skip || !*s) {
            free(l.s);
            continue;
        }
        if (strcmp(kw, "include") == 0 && (s[7] == ' ' || s[7] == '\t')) {
            v = expand(trim(s + 8));
            readmake(v, 1);
            free(v);
            inrule = 0;
            free(l.s);
            continue;
        }
        c = rulecolon(s, &assign);
        if (c && assign) {              /* NAME = VALUE, :=, +=, ?= */
            k = *c == ':';
            p = c;
            if (k)
                c = c[1] == ':' ? c + 2 : c + 1;    /* (::=) */
            *p = 0;
            n = strlen(s);
            if (!k && n > 0 && (s[n - 1] == '+' || s[n - 1] == '?')) {
                kw[0] = s[n - 1];
                s[n - 1] = 0;
            } else
                kw[0] = 0;
            s = trim(s);
            v = trim(c + 1);
            if (kw[0] == '?' && findvar(s)) {
                free(l.s);
                continue;
            }
            if (kw[0] == '+' && findvar(s)) {
                binit(&l);
                badd(&l, findvar(s)->value);
                p = findvar(s)->simple ? expand(v) : xstrdup(v);
                if (l.n && *p)
                    baddc(&l, ' ');
                badd(&l, p);
                free(p);
                setvar(s, l.s, findvar(s)->simple, 0);
            } else if (k) {
                p = expand(v);
                setvar(s, p, 1, 0);
                free(p);
            } else
                setvar(s, v, 0, 0);
            inrule = 0;
            free(l.s);
            continue;
        }
        if (c) {                        /* TARGETS : PREREQUISITES [; RECIPE] */
            *c = 0;
            p = strchr(c + 1, ';');
            if (p)
                *p++ = 0;
            v = expand(s);
            s = expand(c + 1);
            rule(v, s, p ? trim(p) : NULL);
            free(v);
            free(s);
            inrule = 1;
            free(l.s);
            continue;
        }
        die("a line that is not a rule or an assignment: ", s);
    }
    fclose(f);
    if (level)
        die("an if without its endif in ", name);
}

/* ---- making ---- */

unsigned long ftime(char *name)
{
    return filetime(name);
}

unsigned long make(char *name, char *needer);

/* a pattern rule for name whose prerequisites exist or can be made; its stem in stem */
struct prule *implicit(char *name, char *stem, int lvl)
{
    struct prule *r;
    struct dep *d;
    struct buf b;
    char sub[256];
    int ok;
    if (lvl > 3)
        return NULL;
    for (r = prules; r; r = r->next) {
        if (!pmatch(r->tpat, name, stem))
            continue;
        ok = 1;
        for (d = r->deps; d && ok; d = d->next) {
            binit(&b);
            psubst(d->name, stem, &b);
            ok = ftime(b.s) != 0 || (findtarget(b.s, 0) && findtarget(b.s, 0)->hasrule) ||
                 implicit(b.s, sub, lvl + 1) != NULL;
            free(b.s);
        }
        if (ok) {
            pmatch(r->tpat, name, stem);
            return r;
        }
    }
    return NULL;
}

/* a recipe line expanded for target t (its prerequisites' names in deps), written */
void schedule(struct target *t, struct cmd *cmds, char *stem, struct dep *deps, unsigned long tt)
{
    struct buf all;
    struct buf newer;
    struct dep *d;
    struct dep *e;
    char *line;
    char *p;
    int dup;
    binit(&all);
    binit(&newer);
    for (d = deps; d; d = d->next) {
        dup = 0;
        for (e = deps; e != d; e = e->next)
            if (same(e->name, d->name))
                dup = 1;
        if (dup)
            continue;
        addword(&all, d->name);
        if (tt == 0 || findtarget(d->name, 0) == NULL || findtarget(d->name, 0)->time > tt)
            addword(&newer, d->name);
    }
    at = t;
    astem = stem;
    afirst = deps ? deps->name : "";
    aall = all.s;
    anewer = newer.s;
    for (; cmds; cmds = cmds->next) {
        line = expand(cmds->text);
        p = trim(line);
        while (*p == '@' || *p == '+')
            p = trim(p + 1);
        if (*p) {
            if (strlen(p) > 79)
                fprintf(stderr, "make: a command longer than the shell's 80 characters: %s\n", p);
            if (out) {
                if (opti && *p != '-')
                    fputc('-', out);
                fprintf(out, "%s\n", p);
            } else if (optn)
                printf("%s\n", p);
            nmade++;
        }
        free(line);
    }
    at = NULL;
    free(all.s);
    free(newer.s);
}

/* make name (needer: what needs it); its time after: NEWEST when it is to be made */
unsigned long make(char *name, char *needer)
{
    struct target *t;
    struct prule *r;
    struct dep *d;
    struct dep *deps;
    struct dep **end;
    struct cmd *cmds;
    struct buf b;
    char stem[256];
    unsigned long tt, dt;
    int remake;
    t = findtarget(name, 1);
    if (t->state == 2)
        return t->time;
    if (t->state == 1) {
        fprintf(stderr, "make: Circular %s <- %s dependency dropped.\n", needer ? needer : "", name);
        return t->time;
    }
    t->state = 1;
    deps = t->deps;
    cmds = t->cmds;
    stem[0] = 0;
    if (!cmds && !t->phony && (r = implicit(name, stem, 0)) != NULL) {
        cmds = r->cmds;                 /* a pattern rule: its prerequisites first, then the rule's own */
        deps = NULL;
        end = &deps;
        for (d = r->deps; d; d = d->next) {
            binit(&b);
            psubst(d->name, stem, &b);
            *end = (struct dep *)xmalloc(sizeof(struct dep));
            (*end)->name = b.s;
            (*end)->next = NULL;
            end = &(*end)->next;
        }
        *end = t->deps;
    } else if (!t->hasrule) {
        tt = ftime(name);
        if (tt == 0 && !t->phony) {
            if (needer)
                fprintf(stderr, "make: *** No rule to make target '%s', needed by '%s'.  Stop.\n", name, needer);
            else
                fprintf(stderr, "make: *** No rule to make target '%s'.  Stop.\n", name);
            exit(2);
        }
        t->state = 2;
        t->time = tt;
        return tt;
    }
    tt = t->phony ? 0 : ftime(name);
    t->time = tt;
    remake = optB || t->phony || tt == 0;
    for (d = deps; d; d = d->next) {
        dt = make(d->name, name);
        if (dt > tt || dt == NEWEST)
            remake = 1;
    }
    if (remake && cmds)
        schedule(t, cmds, stem[0] ? stem : NULL, deps, tt);
    t->time = remake ? NEWEST : tt;
    t->state = 2;
    return t->time;
}

/* ---- the batch the shell runs ---- */

/* #4:SYSTEM.BATCH: the shell's batch files (as it saved them before MAKE ran), and
   MAKE's file as the innermost: its lines next */
void pushbatch(char *file)
{
    FILE *f;
    char lines[4][100];
    int n, i;
    n = 0;
    f = fopen(BATCHFILE, "rb");
    if (f) {
        while (n < 4 && fgets(lines[n], 100, f) && lines[n][0] != 0)
            n++;
        fclose(f);
    }
    if (n >= 4)
        die("batch files 4 deep at most: MAKE cannot run its recipes from here", "");
    sprintf(lines[n++], "0 %s\n", file);
    f = fopen(BATCHFILE, "wb");
    if (!f)
        die("cannot write ", BATCHFILE);
    for (i = 0; i < n; i++)
        fputs(lines[i], f);
    fputc(0, f);
    if (fclose(f) != 0)
        die("cannot write ", BATCHFILE);
}

/* the depth this make's batch file has (its name: #4:MAKEn.TEXT) */
int batchdepth(void)
{
    FILE *f;
    char line[100];
    int n;
    n = 0;
    f = fopen(BATCHFILE, "rb");
    if (f) {
        while (n < 4 && fgets(line, 100, f) && line[0] != 0)
            n++;
        fclose(f);
    }
    return n + 1;
}

void usage(void)
{
    fprintf(stderr, "use: MAKE [-f FILE] [-nqBsik] [NAME=VALUE ...] [TARGET ...]\n");
    exit(2);
}

int main(int argc, char **argv)
{
    int i, j, ngoals;
    char *a;
    char *eq;
    char *goals[20];
    char bfile[24];
    char mf[80];
    struct target *t;
    unsigned long r;
    ngoals = 0;
    mfname = NULL;
    setvar("MAKE", "MAKE", 1, 0);
    setvar("comma", ",", 1, 0);
    setvar("space", " ", 1, 0);
    setvar("CC", "CC", 1, 0);
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (a[0] == '-' && a[1]) {
            for (j = 1; a[j]; j++) {
                if (a[j] == 'f') {
                    mfname = a[j + 1] ? a + j + 1 : (i + 1 < argc ? argv[++i] : NULL);
                    if (!mfname)
                        usage();
                    break;
                }
                switch (a[j]) {
                case 'n': optn = 1; break;
                case 'q': optq = 1; break;
                case 'B': optB = 1; break;
                case 'i': opti = 1; break;
                case 's': case 'k': case 'r': case 'R': break;
                default:
                    fprintf(stderr, "make: invalid option -- '%c'\n", a[j]);
                    usage();
                }
            }
        } else if ((eq = strchr(a, '=')) != NULL && eq > a) {
            *eq = 0;
            setvar(a, eq + 1, 1, 1);
        } else if (ngoals < 20)
            goals[ngoals++] = a;
    }
    /* the built-in rules (a makefile's own come first) */
    if (mfname) {
        strcpy(mf, mfname);
        if (!strchr(mf, '.') && filetime(mf) == 0 && strlen(mf) < 70)
            strcat(mf, ".TEXT");
        readmake(mf, 1);
    } else if (filetime("MAKEFILE.TEXT"))
        readmake("MAKEFILE.TEXT", 1);
    else if (filetime("MAKEFILE"))
        readmake("MAKEFILE", 1);
    else if (ngoals == 0)
        die("*** No targets specified and no makefile found.  Stop.", "");
    rule("", "", NULL);
    addcmd(&addprule("%.OBJ", "%.C")->cmds, "$(CC) -C $*");
    addcmd(&addprule("%.CODE", "%.C")->cmds, "$(CC) $*");
    if (ngoals == 0) {
        if (!defgoal)
            die("*** No targets.  Stop.", "");
        goals[ngoals++] = defgoal;
    }
    out = NULL;
    if (!optn && !optq) {
        if (SYSCOM->expansion[0] != PX_CHILD) {
            fprintf(stderr, "make: run MAKE from the shell (S): the shell runs the recipes after it\n");
            return 2;
        }
        sprintf(bfile, "#4:MAKE%d.TEXT", batchdepth());
        out = fopen(bfile, "w");
        if (!out)
            die("cannot write ", bfile);
    }
    for (i = 0; i < ngoals; i++) {
        j = nmade;
        r = make(goals[i], NULL);
        t = findtarget(goals[i], 0);
        if (nmade == j && !optq) {
            if (t && (t->cmds || !t->hasrule) && !t->phony)
                printf("make: '%s' is up to date.\n", goals[i]);
            else
                printf("make: Nothing to be done for '%s'.\n", goals[i]);
        }
    }
    if (optq)
        return nmade ? 1 : 0;
    if (out) {
        if (fclose(out) != 0)
            die("cannot write ", bfile);
        if (nmade)
            pushbatch(bfile);
        else
            remove(bfile);
    }
    return 0;
}
