/* mount.c -- MOUNT [#n IMAGE | -u #n]: a disk unit gets another disk image, by its name, as the
   emulator's File > Open Unit does it (the emulator's CSP 140 and 141: version 2.14).

     MOUNT                 every disk unit: its volume and the image file it is; then the
                           volumes there are to mount (CSP 143: emulator 2.18) -- the .BLK
                           files beside the boot disk's image, on the RP2040 the volumes of
                           its flash (and their flash addresses: CSP 142)
     MOUNT #10 U132.A_PASCAL_COMPILER_SOURCE
                           unit 10 gets that image: NAME or NAME.BLK in the folder on the PC
                           of the image unit 10 has now (else the boot disk's); the old
                           image's file keeps what was written to it
     MOUNT -u #10          unit 10's image taken off

   Units 5 and 9 to 14; not 4, the boot disk (its programs would be gone).  The unit may be
   given as #10, #10: or 10.  An IMAGE has letters, digits, . _ and - only (no folder).  The
   new volume is read at once, so its name finds it (DIR U132A:).  A program of yours must not
   have a file open on the unit.  Status 0, 1 no such image, 2 a bad unit or none there, 3 a
   bad name.  Run it from the shell, or in a makefile's recipes: a makefile cannot look at
   files on a volume not yet on line, so MOUNT, then $(MAKE) -f of a makefile that names them,
   as MAKE -f UCSD-C:/ORIG/MAKEFILE does for the source volumes of the P-System. */
#pragma segment MAIN    /* calls within the program: 2-byte CGPs, no NOP after them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <psys.h>
#include <dirs.h>

int dir[1024];

/* "#10", "#10:", "10": the unit; 0 if it is not one */
int unitarg(char *s)
{
    int u;
    if (*s == '#')
        s++;
    if (!isdigit(*s))
        return 0;
    u = atoi(s);
    while (isdigit(*s))
        s++;
    if (*s == ':')
        s++;
    return *s ? 0 : u;
}

void show(int u)
{
    char img[80];
    char vol[16];
    int n;
    n = __cspi(141, u, img, (int)sizeof img);
    if (n < 0)
        return;
    vol[0] = 0;
    if (dirread(u, dir))
        direntname(dir, 0, vol);
    printf("#%-3d %-8s %s\n", u, vol[0] ? vol : "-", img);
}

/* CSP 143 VOLINFO's record: the index-th volume there is to mount (emulator 2.18) */
struct volinfo {
    char name[24];                      /* as MOUNT takes it */
    unsigned blocks;
    unsigned unit;                      /* on this unit now; 0 none */
    unsigned flags;                     /* 1 read-only, 2 persistent */
    unsigned pad;
};
struct volinfo vi;

/* CSP 142 VOLADDR: the volume's place, a long (on the RP2040 its flash address); as a float's
   two words, then read as the long they are */
long voladdr(int i)
{
    union { float f; long l; } u;
    u.f = __cspf(142, i);
    return u.l;
}

void available(void)
{
    int i;
    long a;
    if (__cspi(143, 0, &vi) != 1)
        return;
    printf("\nto mount (MOUNT #n NAME):\n");
    for (i = 0; __cspi(143, i, &vi) == 1; i++) {
        printf("     %-23s %5u blocks", vi.name, vi.blocks);
        a = voladdr(i);
        if ((unsigned long)a >= 0x10000000L)
            printf("  at %08lx", a);
        if (vi.unit)
            printf("  on #%u", vi.unit);
        if (vi.flags & 1)
            printf("  read-only");
        if (vi.flags & 2)
            printf("  kept");
        printf("\n");
    }
}

void usage(void)
{
    fprintf(stderr, "use: MOUNT [#n IMAGE | -u #n]\n");
    exit(2);
}

int main(int argc, char **argv)
{
    int u, r;
    char path[8];
    FILE *f;
    if (argc == 1) {
        show(4);
        show(5);
        for (u = 9; u <= 14; u++)
            show(u);
        available();
        return 0;
    }
    if (argc != 3)
        usage();
    if (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "-U") == 0) {
        u = unitarg(argv[2]);
        argv[2] = "";
    } else
        u = unitarg(argv[1]);
    if (u == 0 || u == 4) {
        fprintf(stderr, "mount: %s: not a unit that can be changed (5, 9 .. 14)\n", argv[1]);
        return 2;
    }
    r = __cspi(140, u, argv[2]);
    if (r == -1) {
        fprintf(stderr, "mount: %s: no such image (beside the one on unit %d, or the boot disk)\n", argv[2], u);
        return 1;
    }
    if (r == -2) {
        fprintf(stderr, "mount: unit %d cannot be changed (emulator 2.14 or later?)\n", u);
        return 2;
    }
    if (r == -3) {
        fprintf(stderr, "mount: %s: a bad name (letters, digits, . _ - only)\n", argv[2]);
        return 3;
    }
    /* the system reads the new directory (its UNITABLE gets the volume's name) when a
       file on #n: is looked for */
    sprintf(path, "#%d:", u);
    strcat(path, "\001");
    f = fopen(path, "r");
    if (f)
        fclose(f);
    if (r > 0)
        show(u);
    return 0;
}
