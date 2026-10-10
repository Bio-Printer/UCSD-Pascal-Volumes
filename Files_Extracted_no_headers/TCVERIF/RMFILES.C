/* rmfiles.c -- remove the files named, one per line, until an empty line
 * (Tiny-C Verify: makes room in the volume's 77-entry directory). */
#include <stdio.h>
#include <string.h>

int main(void)
{
    char name[44];
    int n;
    int removed;
    removed = 0;
    for (;;) {
        printf("Remove? ");
        if (!fgets(name, 40, stdin))
            break;
        n = strlen(name);
        while (n > 0 && (name[n - 1] == '\n' || name[n - 1] == '\r' || name[n - 1] == ' '))
            name[--n] = 0;
        if (n == 0)
            break;
        if (remove(name) == 0)
            removed++;
        else
            printf("cannot remove %s\n", name);
    }
    printf("REMOVED %d\n", removed);
    return 0;
}
