/* fileargs.c -- Tiny-C library: a program's FILE arguments with the wild
   cards of DIR (forfiles, <dirs.h>) */
#include "libint.h"
#include <psys.h>
#include <dirs.h>

/* arg as it is when it has no wild card, else every file its name matches
   (* or = any characters, ? any one) after an optional VOL:, #n: or path
   (*.C, #5:*.TEXT, A/*.H, #5:/A/B/*.C): fn(name) for each, in the order of
   the directory, the name with what came before the pattern (#5:A.C).  The
   directory (2 KB) is on the heap only while forfiles runs.  Returns the
   largest of fn's results, 1 (said on stderr, as "prog: ...") when no file
   matches or the disk or sub-volume is not there, 2 when there is no memory.
   fn NULL: how many files arg names (0 none), nothing said. */
int forfiles(char *prog, char *arg, int (*fn)(char *name))
{
    int *dir;
    char pat[16];
    char name[16];
    char full[48];
    int u, base, i, n, r, st, found;
    if (strpbrk(arg, "*=?") == NULL)
        return fn ? fn(arg) : 1;
    dir = (int *)malloc(2048);
    if (!dir) {
        if (!fn)
            return 0;
        fprintf(stderr, "%s: %s: no memory for the directory\n", prog, arg);
        return 2;
    }
    st = 0;
    found = 0;
    if (!pathdir(arg, dir, &u, &base, pat)) {
        if (fn) {
            fprintf(stderr, "%s: %s: no such disk or sub-volume on line\n", prog, arg);
            st = 1;
        }
    } else {
        n = strlen(arg) - strlen(pat);  /* what goes before each name: VOL:, A/ ... */
        if (n > 31)
            n = 31;
        for (i = 1; i <= dir[8]; i++) {
            direntname(dir, i, name);
            if (!wildmatch(pat, name))
                continue;
            found++;
            if (!fn)
                continue;
            memcpy(full, arg, n);
            full[n] = 0;
            strcat(full, name);
            r = fn(full);
            if (r > st)
                st = r;
        }
        if (!fn)
            st = found;
        else if (!found) {
            fprintf(stderr, "%s: %s: no file matches\n", prog, arg);
            st = 1;
        }
    }
    free(dir);
    return st;
}
