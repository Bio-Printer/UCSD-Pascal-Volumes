UCSD Pascal II.0 Distribution Disk Set -- Verified Reference Copy  (v1.03)
===========================================================================

  BLK_format/                  "Big Disk" images (5,120,000 bytes each), the
                               format the emulator mounts directly (unit #4,
                               or #5/#9/#10).
  raw_8inch_format/            Original 8" SSSD floppy images (256,256 bytes).
  Files_Extracted_Raw/         Every file on every BLK volume, exact on-disk
                               bytes, one folder per disk.
  Files_Extracted_no_headers/  The same files, with .TEXT files converted to
                               plain Windows text (see below). Non-text files
                               are identical to the Raw copies.
  tools/extract_all.py         Regenerates both Files_Extracted folders,
                               EXTRACTION_REPORT.txt and MANIFEST_SHA256.txt
                               from BLK_format/.
  EXTRACTION_REPORT.txt        Per-disk list of what was extracted and every
                               anomaly found.
  MANIFEST_SHA256.txt          SHA-256 of every disk image and extracted file.
  VERIFICATION_REPORT.txt      How each disk image copy was chosen (v1.00),
                               plus the v1.03 addendum.

---------------------------------------------------------------------------
WHAT CHANGED IN v1.03 (regenerated from v1.00)
---------------------------------------------------------------------------
1. Extraction now uses ONLY the live directory entries.
   The volume header's file count (DNUMFILES) says how many of the 77
   directory slots are files. The v1.00 extractor read every slot, so it
   also "extracted" 13 phantom files from slots left behind by deleted
   files, whose blocks now belong to other files:
       Big_Disk: CPMINC.TEXT, CPMIO.TEXT, FPT.TEXT, INTERP.TEXT, MICRO.LST,
                 TEST.TEXT, and duplicates of SYSTEM.LIST, SYSTEM.MICRO and
                 SYSTEM.WRK.CODE (x2)
       U002A.5_Z80_SYS1: SYSTEM.WRK.CODE (x2)
       U124.1_LSI-11_ASSEM_SOURCE: a 4.7 MB second "11.OPCODES"
   These are gone. (v1.00's README described several of them as historical
   artifacts of the disks, and Big_Disk's INTERP.TEXT as "editor help text";
   both explanations were wrong -- they were this extraction bug.)
   All 212 other live files are byte-identical to v1.00's copies.

2. U132.A BODYPART.A.TEXT repaired.
   The v1.00 image (and the 8" raw image it came from) has this file cut
   off mid-statement: the last line on its last page is
   "BEGIN ERROR(6); SKIP(FSYS " followed by NUL bytes, with no end of line.
   The complete file (510 lines; the first 506 identical to the damaged one)
   was taken from the working U132-A volume and written block-for-block
   into BLK_format/U132.A_PASCAL_COMPILER_SOURCE.BLK. Only 2 blocks of the
   image changed (the file's editor header and the second half of its last
   page); the directory is untouched.
   Proof it is right: the Pascal compiler compiled from this volume's
   sources is identical to the shipped SYSTEM.COMPILER (segments 10-15
   byte-for-byte, apart from 3 alignment-padding bytes whose value is
   whatever was in the compiler's buffer). The 8" raw image is left as it
   was and still contains the truncated file.

3. .TEXT decoding rewritten (Files_Extracted_no_headers).
   Verified UCSD Pascal II.0 .TEXT format (checked against the editor,
   which reads and writes files in exactly this form):
     - bytes 0-1023: editor header (not text);
     - then 1024-byte PAGES. A page holds only whole lines; the rest of the
       page is NUL-filled. (v1.01 described this as 512-byte blocks, which
       was wrong.)
     - each line: optional DLE (0x10) + (32 + number of leading spaces),
       the text, then CR (0x0D).
   The Windows version removes the header, expands the DLE codes to spaces,
   drops the NUL padding and ends lines with CR LF. Every other byte is kept
   exactly -- including TAB characters, which the original sources use --
   and blank lines are kept (the v1.01 decoder silently dropped them, e.g.
   U134.4 SYSTEM.A.TEXT is 425 lines, not 408). The decoder is strict: any
   line not ending before its page end, or any bytes after the padding,
   would be reported in EXTRACTION_REPORT.txt. Across all 121 text files
   the only anomalies are three single damaged bytes inside comments of the
   original U120 sources, kept as found:
       BOOT.TEXT line 137  "next dir ent?y"   ('r' damaged to 0xFF)
       FPT.TEXT  line 256  "ret add?"         ('r' damaged to 0xFF)
       STP.TEXT  line 623  stray 0x11 at the end of a comment -- also
                           present in the SYSTEM.MICRO assembly listing,
                           so the damage predates that listing.

---------------------------------------------------------------------------
STILL-VALID NOTES FROM v1.00
---------------------------------------------------------------------------
Every BLK/raw image was chosen by cross-checking all copies found in the
original working archive; see VERIFICATION_REPORT.txt. Big_Disk.BLK was also
checked register-by-register against a real UCSD Pascal system's execution
trace. U121-2_LSI-11_P-CODE_SOURCE exists only as a .raw image (it uses a
different physical format: 256-byte sectors, 13 per track, no skew). The
Z80-format raw floppy images are in physical sector order; de-skewing them
into logical order has not been solved.

IMPORTANT DATA ISSUE FOUND DURING EXTRACTION:
UCSD_SYS1_Z80.BLK and UCSD_SYS2_Z80.BLK are NOT distinct disk images.
  - UCSD_SYS1_Z80.BLK is byte-for-byte identical to
    U138.3_UTILITY_SOURCE.BLK (internal volume name "U138.3").
  - UCSD_SYS2_Z80.BLK is byte-for-byte identical to U012.1_SYS_2.BLK
    (internal volume name "U012.1").
This was checked against every copy of these two files found across
the entire working archive (5+ locations each) -- all consistently
show the same mislabeling, including in the oldest available copies.
This is a pre-existing issue in the source material itself, not
something introduced during this project's verification or packaging
work. A genuinely different UCSD_SYS2_Z80.BLK variant was found in one
location, but it is also internally labeled "U012.1" -- just a
different modified copy of the same wrong content, not a correct SYS2
disk. No correct, distinct UCSD_SYS1 or UCSD_SYS2 content has been
located anywhere in the archive to date. Their extracted folders
are included for completeness (they contain real files, just the same
ones as U138.3_UTILITY_SOURCE and U012.1_SYS_2 respectively) but
should not be relied on as genuine SYS1/SYS2 system disk content.

Big_Disk.BLK is a working disk from this project, not a distribution
disk. Its live file 'M' (blocks 930-10000, about 4.5 MB) is a deliberate
marker covering the free space; it is extracted as it is.

===================================================================
ADDENDUM (v1.05) -- U132.A replaced with a version that includes FIXUP
===================================================================

U132.A_PASCAL_COMPILER_SOURCE.BLK was replaced with a new copy (20
directory entries, up from 15) that adds the missing piece for
actually using a self-compiled Pascal compiler:

  FIXUP.TEXT / FIXUP.CODE  -- a utility program (source + compiled)
  that patches *SYSTEM.WRK.CODE (the raw output of compiling
  COMPILER.TEXT against itself) into a properly runnable
  *SYSTEM.COMPILER. Per its own header comment: COMPILER.TEXT declares
  placeholder segments for OS segments 0..9 so the real compiler lands
  in segments 10..15, meaning the raw compiled output's segment 1 (where
  the system enters a program) just returns immediately rather than
  calling the compiler. FIXUP rewrites *SYSTEM.COMPILER with the real
  segments 10..15 plus a small stub in segment 1 that calls into the
  compiler proper, backs up the previous *SYSTEM.COMPILER as
  *SYSOLD.COMPILER first, and marks both as CODE files in the volume
  directory (a Pascal REWRITE alone would leave them as DATA files,
  which the system refuses to run as a compiler). It locates the
  system volume by scanning units 4,5,9-12 for the one holding both
  SYSTEM.WRK.CODE and SYSTEM.COMPILER, and only patches the directory
  if the two new entries are exactly the files it just wrote.

  COMP-V160.CODE -- a pre-built compiler binary (version 160),
  presumably useful as a known-good reference/bootstrap.

BODYPART.A.TEXT was re-verified in this new copy: still decodes to the
same 494 lines ending in "END (*SELECTOR*) ;" -- the fix from v1.02
carried over correctly.

Two large duplicate directory entries in this new copy --
FIXUP.TEXT (a second entry, blocks 676-10000) and COMPILER.BACK
(kind 243, blocks 676-10000) -- were checked directly and are just
zero-filled free space, the same pattern as every other such entry
found elsewhere in this project; extracted as-is per the established
approach.

===================================================================
ADDENDUM (v1.06) -- Big_Disk.BLK replaced
===================================================================

BLK_format/Big_Disk.BLK (volume BIGGY) was replaced with the user's
updated image. It carries an updated SYSTEM.MISCINFO that correctly
drives the screen handler for the editor, and its SYSTEM.EDITOR has
been switched to the L2 editor. It is a working system disk (33 live
directory entries), so it also contains build leftovers such as
FIXUP.TEXT/.CODE/.BACK, SYSTEM.COMPILER, SYSOLD.COMPILER and
TCTEMP.TEXT. The old 'M' free-space marker file is gone.

Files_Extracted_Raw/, Files_Extracted_no_headers/, EXTRACTION_REPORT.txt
and MANIFEST_SHA256.txt were regenerated for the whole set with the
extractor in tools/ (only live directory entries are extracted).
No other changes were made in this version.

===================================================================
ADDENDUM (v1.07) -- 12-byte floating point support
===================================================================

Three volumes were replaced with versions carrying 12-byte floating
point support (P-Machine CSP extensions plus Tiny-C DOUBLE type
support):
  Big_Disk.BLK  (volume BIGGY, 50 live entries, up from 36)
  TCEXTRA.BLK   (volume TCEXTRA, 37 live entries, up from 35)
  TINY-C.BLK    (volume TINY-C, 62 live entries, up from 61)

Per the standing preference to keep older revisions rather than
silently discard them, the prior copies of all three were kept
alongside, suffixed _pre_12byte_float:
  Big_Disk_pre_12byte_float.BLK
  TCEXTRA_pre_12byte_float.BLK
  TINY-C_pre_12byte_float.BLK

Files_Extracted_Raw/, Files_Extracted_no_headers/,
EXTRACTION_REPORT.txt and MANIFEST_SHA256.txt were regenerated for the
full set (27 volumes) with tools/extract_all.py. No other changes were
made in this version; the u132/u134/u128 _fixed volumes from v1.06 are
unchanged.

===================================================================
ADDENDUM (v1.08) -- sourced from the UCSD-TinyC GitHub repository (then UCSD-C)
===================================================================

Big_Disk.BLK, TINY-C.BLK and TCEXTRA.BLK were replaced with the
canonical builds from https://github.com/Bio-Printer/UCSD-TinyC (branch
main, pulled 2026-09-29), built directly from that repo's own
tools/mkbiggy.py and tools/mkvolume.py rather than taken as loose
files. This repo is Tiny-C's home: the compiler, its C library and
headers, the P-System-side test/demo programs, the emulator project
(UCSD-Pascal---P-Machine_work, now at v1.88, with the 12-byte float
CSPs integrated), and a Linux validation harness (setup.sh,
runtests.py, crosscheck.py, selfcompile.py, voltest.py, tcverify.py,
f12test.py, etc.).

  Big_Disk.BLK   volume BIGGY,   46 files (TINYC.CODE, TCLIB.OBJ,
                 TCMSGS.TEXT and headers added to the boot volume)
  TINY-C.BLK     volume TINY-C,  47 files
  TCEXTRA.BLK    volume TCEXTRA, 37 files

Prior revisions were kept rather than discarded, suffixed
_pre_UCSD-C_repo (these were the loose "12-byte float" volumes added
in v1.07):
  Big_Disk_pre_UCSD-C_repo.BLK
  TINY-C_pre_UCSD-C_repo.BLK
  TCEXTRA_pre_UCSD-C_repo.BLK

Validation run against this repo's own suite before packaging (see
VALIDATION_LOG.txt):
  tools/setup.sh          -- clean build of the emulator's Linux runner
  tools/runtests.py       -- 16/16 tests passed (host-built compiler)
  tools/crosscheck.py     -- 16/16 identical (P-System-built vs host)
  tools/selfcompile.py    -- TINYC2.CODE byte-identical to host build
  tools/mkvolume.py + voltest.py -- volume test PASSED (native mode)
  tools/mkverify.py + tcverify.py native -- PASSED, 657 steps
  tools/tcverify.py z80   -- see VALIDATION_LOG.txt for the result
  tools/f12test.py        -- 51/51 float-CSP checks passed

Files_Extracted_Raw/, Files_Extracted_no_headers/,
EXTRACTION_REPORT.txt and MANIFEST_SHA256.txt were regenerated for the
full set (30 volumes). The u132/u134/u128 _fixed volumes from v1.06
are unchanged.

===================================================================
ADDENDUM (v1.09) -- 8-byte double format (breaking change from 12-byte)
===================================================================

Big_Disk.BLK, TINY-C.BLK and TCEXTRA.BLK were replaced again, this
time with a real format change upstream, not just a rebuild: doubles
moved from 12 bytes (6 words, 4 reserved/unused) to a true 8-byte (4
word) IEEE-754 binary64 representation. Per the repo's own docs
(docs/DOUBLES.md, replacing docs/FLOAT12.md): "Until Tiny-C [0.4] a
double was 12 bytes... They were never used and were removed;
programs compiled for the 12-byte engine must be recompiled, and the
12-byte and 8-byte engines and compilers do not mix. `triple` went
with them." NativeFloat12.inc was replaced by NativeDouble.inc in the
emulator project.

  Big_Disk.BLK   volume BIGGY,   46 files
  TINY-C.BLK     volume TINY-C,  47 files
  TCEXTRA.BLK    volume TCEXTRA, 37 files

Prior (12-byte-double) revisions were kept, suffixed _pre_8byte_double
(Big_Disk, TINY-C, TCEXTRA); they have since been removed (see the last
section).

Full validation re-run against this pull (see VALIDATION_LOG.txt):
runtests.py 16/16, crosscheck.py 16/16 identical, selfcompile.py
byte-identical, voltest.py PASSED, tcverify.py native PASSED (658
steps), f12test.py 51/51. tcverify.py z80 was run in the background;
see VALIDATION_LOG.txt for its result.

Files_Extracted_Raw/, Files_Extracted_no_headers/,
EXTRACTION_REPORT.txt and MANIFEST_SHA256.txt regenerated for the full
set (33 volumes).

---------------------------------------------------------------------------
BIG_DISK: FILER AND EDITOR FOR C SOURCE FILES (.C / .H)
---------------------------------------------------------------------------
BLK_format/Big_Disk.BLK now runs a Filer and an Editor that accept a C
source or header (NAME.C, NAME.H) as the workfile under its exact name:
    SYSTEM.FILER   built from U134.4_OS_SOURCE_v1.06 (FILER.D.TEXT; see the
                   section below -- not from U134_4_OS_fixed): G(et,
                   S(ave and the size check take NAME.C / NAME.H as they are
                   instead of looking for NAME.C.TEXT / NAME.C.CODE
    SYSTEM.EDITOR  the editor that opens and updates such a workfile
The previous programs are kept on the disk: SYSORIG.FILER (the original
II.0 Filer, whose G(et of any .C file answers "No file loaded") and
SYSL2.EDITOR (which answers "ERROR: Workfile lost" for a .C workfile).
Both new programs are the ones on the Tiny-C repository's boot disk
(UCSD-TinyC: volumes/Big_Disk---8_byte_floats.BLK). SYSTEM.COMPILER is
unchanged. Checked: G(et of #10:PI.C and #10:TEST.C, E(dit, U(pdate,
S(ave back to PI.C, G(et of a NAME.TEXT workfile, Tiny-C compile and run.

The older copies named above (*_pre_12byte_float, *_pre_UCSD-C_repo)
are no longer in BLK_format/, and their leftover folders in
Files_Extracted_Raw/ and Files_Extracted_no_headers/, with their
MANIFEST_SHA256.txt lines and EXTRACTION_REPORT.txt sections, were
removed too: the extracted files again match BLK_format/ one to one
(Empty_Big_Disk has no files). The git history still holds them all.

---------------------------------------------------------------------------
SOURCES OF BIGGY'S FILER AND EDITOR: *_SOURCE_v1.06
---------------------------------------------------------------------------
BLK_format/U134.4_OS_SOURCE_v1.06.BLK and U128_L2_YALOE_SOURCE_v1.06.BLK
(each with a VERSION.TEXT) are the source volumes of the programs on
Big_Disk.BLK (BIGGY) revision 1.06:
    U134.4_OS_SOURCE_v1.06     the distribution OS and Filer source; only
                               FILER.D.TEXT changed (C workfiles, exact
                               names). Compile II.0.FILER: SYSTEM.FILER.
    U128_L2_YALOE_SOURCE_v1.06 the L2 editor source; only L2.TEXT changed
                               (.C/.H and exact names, MAXSW=131).
                               Compile L2: SYSTEM.EDITOR.
Checked by compiling them on BIGGY (II.0 compiler, emulator 1.94): the
code files equal SYSTEM.FILER and SYSTEM.EDITOR on BIGGY in every byte
of code; the only differences are alignment padding and the unused tails
of blocks (whatever the compiler's buffer held). The older U134_4_OS_fixed
and U128_L2_YALOE_fixed volumes hold earlier versions of the same changes
and do not reproduce BIGGY's programs exactly.

BIGGY's SYSTEM.PASCAL is the distribution binary (U002A.5_Z80_SYS1) with
the VT-52 FGOTOXY of MYGOTOXY.TEXT bound in (procedure 29 is the only
difference). The OS source in these volumes is a slightly different
revision from that binary: compiled, it differs in PRINTLOCS (checks
MISCINFO.IS_FLIPT) and in the length of the system file name strings in
INITIALIZE and GETCMD, besides FGOTOXY and padding.

---------------------------------------------------------------------------
BIGGY REVISION 1.07: UNITS 13 AND 14; THE OS BUILT FROM ITS SOURCE
---------------------------------------------------------------------------
BLK_format/Big_Disk.BLK (BIGGY) revision 1.07 (see its VERSION.TEXT):
    SYSTEM.PASCAL  compiled from U134.4_OS_SOURCE_v1.07, then MYGOTOXY's
                   FGOTOXY bound in with X(ecute BINDER
    SYSTEM.FILER   compiled from U134.4_OS_SOURCE_v1.07 (II.0.FILER)
    MYGOTOXY.CODE  compiled from MYGOTOXY.TEXT (132 x 45; the code file
                   on the disk was an older 80 x 24 version, while the OS
                   had the 132 x 45 one bound in)
    SYSOLD.PASCAL, SYSOLD.FILER  revision 1.06's OS and Filer
SYSTEM.EDITOR is unchanged (it does not use the OS unit table).

U134.4_OS_SOURCE_v1.07.BLK is v1.06 with:
  - GLOBALS.TEXT MAX_SEG = 15 (was 31) and SYSTEM.A.TEXT PRINTLOCS
    without the MISCINFO.IS_FLIPT test. These make the source match the
    OS in use: compiled with MAXUNIT = 12 it equals the distribution
    SYSTEM.PASCAL (U002A.5_Z80_SYS1) in every procedure except FGOTOXY --
    that binary was itself made by binding a simple FGOTOXY over this
    compile, whose own FGOTOXY is still in it, unreferenced, byte for
    byte -- and BIGGY 1.06's SYSTEM.PASCAL is that plus MYGOTOXY bound in.
    With MAX_SEG = 31 SYSCOM's segment table has 32 entries, but
    SYSTEM.MICRO's has 16: an OS built that way writes over the Z80
    interpreter's code and halts in Z80 mode (the source as distributed
    belongs to a later, 32-segment system).
  - GLOBALS.TEXT MAXUNIT = 14 and SYSSEGS.A.TEXT disk units [4,5,9..14],
    for units 13 and 14 (emulator 1.95). Against 1.06, the new OS and
    Filer differ only by that: 24 more bytes of globals (two more unit
    table entries; FILENAME moves from 204 to 216), loop limits 12 -> 14
    and the disk-unit set, besides alignment padding.
Built and checked with the emulator (Z80 mode for the compiles and
BINDER): units 11-14 on line in the Filer, G(et of a .C file, programs
compiled to and run from units 12 and 13, and the emulator's and Tiny-C's
full test suites (see their repositories).

BIGGY REVISION 1.08: THE COMPILER BUILT FROM U132_A_PASCAL_COMPILER_fixed
---------------------------------------------------------------------------
Up to 1.07 BIGGY's SYSTEM.COMPILER was the distribution compiler. With the
PC's clock (emulator option "PC date and time") a compile can finish in
under a second, and its ROUND((3600/LOWTIME)*SCREENDOTS) lines/min then
exceeds 32767: "Floating point error, S# 10, P# 1, I# 268" after the line
count, before the code file is finished (seen compiling II.0.FILER in
P-Code mode with Harvard on). BLOCK.TEXT in U132_A_PASCAL_COMPILER_fixed
prints tenths of a second and a rate that cannot overflow.

    SYSTEM.COMPILER  COMPILER.TEXT of U132_A_PASCAL_COMPILER_fixed compiled
                     to *SYSTEM.WRK.CODE (Z80 mode), then X(ecute FIXUP
                     from the same volume
    SYSOLD.COMPILER  revision 1.07's (distribution) compiler
Nothing else on BIGGY changed.

The source on that volume is what was compiled, unchanged. Its COMPINIT.TEXT
prints the banner 'PASCAL Compiler [II.0.A.1]   EDN Built! ' and '<edn0>';
SYSFIX.COMPILER on the volume is the same compiler built before those two
strings were changed: against the new SYSTEM.COMPILER it differs only in
them, and in alignment padding (bytes after a procedure's return).
Checked with the emulator: the old compiler fails as above and the new one
compiles II.0.FILER ("2479 lines, 0.1 secs, 1784880 lines/min") to code
that equals SYSTEM.FILER but for one padding byte; builds in P-Code and
Z80 mode differ only in padding; the emulator's and Tiny-C's full test
suites pass.


WHICH COMPILER SOURCE VOLUME IS THE LATEST: U132.A_PASCAL_COMPILER_SOURCE_v1.08
---------------------------------------------------------------------------
Like the OS (U134.4_OS_SOURCE_v1.07), the compiler source that BIGGY's
SYSTEM.COMPILER is built from now has a volume named for the BIGGY revision
that uses it:

    U132.A_PASCAL_COMPILER_SOURCE_v1.08  the latest: the source of BIGGY
                    1.08's SYSTEM.COMPILER. It is U132_A_PASCAL_COMPILER_fixed
                    (since removed) plus a VERSION.TEXT; every other file is
                    identical.
    U132.A_PASCAL_COMPILER_SOURCE  the distribution source (lines/min can
                    overflow, see above), kept as the original.

Against the distribution source the latest volume changes only BLOCK.TEXT
(compile summary); FIXUP.TEXT differs only in two trailing blank lines.
It also holds SYSFIX.COMPILER (an earlier build, see above), BLOCK.BACK (the
same text as BLOCK.TEXT), BLOCK-BAD.TEXT (the distribution BLOCK.TEXT, with
one extra blank line) and COMP-V160.CODE (as on the distribution volume).

REMOVED VOLUMES
---------------------------------------------------------------------------
Removed from BLK_format/, with their extracted files and their lines in
MANIFEST_SHA256.txt and EXTRACTION_REPORT.txt:

    U132_A_PASCAL_COMPILER_fixed.BLK  every file is on
                    U132.A_PASCAL_COMPILER_SOURCE_v1.08, unchanged. Where this
                    README (and BIGGY's VERSION.TEXT) says BIGGY 1.08's
                    compiler was built from it, that is the same source.
    Big_Disk_pre_8byte_double.BLK, TINY-C_pre_8byte_double.BLK,
    TCEXTRA_pre_8byte_double.BLK  the 12-byte floating point revisions,
                    superseded by the 8-byte ones.

They remain in the git history (the commit before their removal).

LINE ENDINGS OF Files_Extracted_no_headers; MANIFEST_SHA256.txt CHECKS CLEAN
---------------------------------------------------------------------------
The no_headers text files end their lines with CR LF, as tools/extract_all.py
writes them and as MANIFEST_SHA256.txt hashes them. Git had been storing 274
of them with LF line ends, so on a Linux checkout or a GitHub zip download
"sha256sum -c MANIFEST_SHA256.txt" failed for those files although the
content was right. They are now stored exactly as extracted (line ends were
the only difference), and .gitattributes marks the folder -text so git never
converts them. sha256sum -c MANIFEST_SHA256.txt now passes for all 845 files.

BIGGY REVISION 1.09: PSYS.H (SYSCOM FOR TINY-C PROGRAMS)
---------------------------------------------------------------------------
Big_Disk.BLK (BIGGY), TINY-C.BLK and TCEXTRA.BLK, from the UCSD-C repository:

    PSYS.H        (BIGGY and TINY-C) Tiny-C header for the OS's SYSCOM
                  record: SYSCOM->memtop, SYSCOM->crtinfo.width,
                  SYSCOM->segtable[n] ... SYSCOM is the OS's first global
                  variable, read with one LOD 2,1, so it is found in every
                  engine layout (0x02E4, or 0x0164 in P-Code mode with
                  reclaimed memory).
    SYSCOM.C, SYSCOM.CODE  (TCEXTRA) its test; DEMOS.TEXT and FILES.TEXT
                  list it (TINY-C's FILES.TEXT lists PSYS.H).
    VERSION.TEXT  (BIGGY) revision 1.09; its SYSTEM.COMPILER line now
                  names U132.A_PASCAL_COMPILER_SOURCE_v1.08.
Nothing else changed.

BIGGY REVISION 1.10: PEXEC (A PROGRAM RUNS ANOTHER, THEN ITSELF AGAIN)
---------------------------------------------------------------------------
U134.4_OS_SOURCE_v1.10.BLK (first published as U134.4_OS_SOURCE_v1.08;
renamed for the BIGGY revision it is the source of, like the others) is
v1.07 with three insertions in SYSSEGS.B.TEXT
(every other line, and every other file but VERSION.TEXT, is unchanged):
GETCMD's constants PXRUN, PXCHILD, PXBACK; the function LOADSEGS (what
ASSOCIATE does after FOPEN: point SYSCOM^.SEGTABLE 1 and 7..15 at the
linked code file whose block 0 is a given block of a given unit); and, at
the start of GETCMD, a hook driven by SYSCOM^.EXPANSION[0..5] (words no
part of the system used):

    [0]  PXRUN     a program asked to run the code file at [1],[2] (unit,
                   first block) and then the one at [3],[4] (itself):
                   LOADSEGS([1],[2]); GETCMD returns SYSPROG, as X(ecute
         PXCHILD   that program has ended (or stopped with an execution
                   error: [5] := -2): LOADSEGS([3],[4]), state PXBACK,
                   SYSPROG -- the first program starts again
         PXBACK    cleared by the next GETCMD
    [5]  exit status (-1: could not be started)

Tiny-C's pexec() (PSYS.H) sets these words and exits. Nothing of the
calling program stays in memory while the other one runs, and the
resident operating system is unchanged: SYSTEM.PASCAL is still 16896
bytes, segment 0 still 7840 bytes; only GETCMD's segment (loaded only
while GETCMD runs) grew from 2688 to 3068 bytes. A program started this
way has exactly the free memory it has when started with X(ecute (checked
in P-Code, Harvard and Z80 mode with UCSD-C's tools/pexectest.py).
Built as 1.07: compile SYSTEM, BINDER with MYGOTOXY.CODE.

Big_Disk.BLK (BIGGY) revision 1.10:
    SYSTEM.PASCAL  the OS of U134.4_OS_SOURCE_v1.10
    SYSOLD.PASCAL  the OS of revision 1.09 (1.07 source); SYSOLD.FILER is
                   still revision 1.06's Filer
    TINYC.CODE, TCLIB.OBJ, PSYS.H  Tiny-C with pexec() (exit() records the
                   status for pexec_status())
TINY-C.BLK: TINYC.CODE, TCLIB.OBJ, PSYS.H, PEXEC.C (library module),
STDLIB.C, LIBS.TEXT, README.TEXT, FILES.TEXT. TCEXTRA.BLK: SHELL.C/.CODE
(a mini-shell built on pexec), MEMFREE.C/.CODE (a program's free memory),
every program relinked with the new library, DEMOS.TEXT, README.TEXT,
FILES.TEXT.

BIGGY REVISION 1.11: $ STARTS THE SHELL; FGOTOXY BUILT IN (NO BINDER)
---------------------------------------------------------------------------
U134.4_OS_SOURCE_v1.11.BLK is v1.10 with these changes (every other line,
and every other file but VERSION.TEXT, is unchanged):

  SYSSEGS.B.TEXT  $ at the Command: prompt runs *SYSTEM.SHELL (ASSOCIATE,
                  as X(ecute); "No file *SYSTEM.SHELL" when it is not
                  there); the ? prompt lists it: "U(ser restart,
                  I(nitialize, H(alt, $(hell". The main prompt line is
                  unchanged (PL is a STRING[80] and that line is 76).
                  $ is not in the FILENAME table of system programs:
                  that would move the OS globals the Filer and the
                  compilers are compiled against.
  SYSTEM.A.TEXT   FGOTOXY is MYGOTOXY.TEXT's (132 x 45: CHR(1), X+32,
                  Y+32, X clamped to 0..131, Y to 0..44) instead of the
                  Datamedia one, so SYSTEM.PASCAL is simply the compiled
                  SYSTEM: BINDER is no longer needed.

The resident operating system is smaller: segment 0 is 7756 bytes (7840
with MYGOTOXY bound in), so programs have 42 words more; GETCMD's segment
is 3176 bytes. Build: compile SYSTEM; the code file is SYSTEM.PASCAL.

Big_Disk.BLK (BIGGY) revision 1.11:
    SYSTEM.PASCAL  the OS of U134.4_OS_SOURCE_v1.11
    SYSOLD.PASCAL  the OS of revision 1.10 (U134.4_OS_SOURCE_v1.10)
    SYSTEM.SHELL   the Tiny-C shell (UCSD-C examples/shell.c), run by $
    PSYS.H         says "BIGGY 1.10 or later" for pexec
    VERSION.TEXT   revision 1.11
MYGOTOXY.CODE stays (the OS has the same FGOTOXY built in). TINY-C.BLK:
PSYS.H and PEXEC.C (comments only). Checked with UCSD-C's suites,
pexectest.py included ($ starts the shell; a program started from the
shell still has exactly the free memory it has from X(ecute).

BIGGY REVISION 1.12: COMMAND LINES FOR PEXEC (main(argc, argv))
---------------------------------------------------------------------------
No operating system change (SYSTEM.PASCAL is still U134.4_OS_SOURCE_v1.11's).
Tiny-C's pexec("NAME ARG1 ARG2 ...") passes the words to NAME's
main(int argc, char **argv) (argv[0] = NAME): the command line is kept in
the OS's prompt-line string PL (OS global word 70, a STRING[80]: 80
characters at most), which nothing writes between two programs, with
SYSCOM^.EXPANSION[6] = 25604 and [7] a checksum of it. The child takes it
only while it is the program pexec started (EXPANSION[0] = PXCHILD) and the
checksum matches; started any other way, argc is 1 and argv[0] is "".

Big_Disk.BLK (BIGGY) revision 1.12: TINYC.CODE and TCLIB.OBJ (the linker
calls the library's __callmain when main has parameters), PSYS.H,
SYSTEM.SHELL (passes arguments; says when a line is longer than 80),
VERSION.TEXT. TINY-C.BLK: the same, plus PEXEC.C (__callmain is in the
pexec module) and LINK.C.

THREE TINY-C VOLUMES: a UCSD directory holds 77 files, and rebuilding the
programs on a volume (@DEMOS) leaves a NAME.OBJ for each, so the demos and
the tests are now on volumes of their own:

    TINY-C.BLK   the compiler, the library, the headers and all their
                 sources (unchanged)
    TCEXTRA.BLK  the demos: BOXES CALC DEMO GUESS HANOI PI QUEENS SIEVE,
                 SHELL, MEMFREE, ARGS (new: does what its arguments say --
                 from SHELL, ARGS ADD 2 3, ARGS MUL 6 7, ARGS REPEAT 3
                 HELLO, ARGS ECHO A B C; no arguments: usage) and CMPCODE;
                 DEMOS.TEXT (@DEMOS) rebuilds them
    TCTESTS.BLK  (new) the test programs: CONTROL DOUBLES FCOMPARE FILEIO
                 FLOATS FUNCPTR FUNCSEG LONGS STRCONST STRINGS STRUCTS
                 SYSCOM; TESTS.TEXT (@TESTS) rebuilds them

After @DEMOS, TCEXTRA has 41 of its 77 directory entries in use, TCTESTS
likewise after @TESTS. (TINY-C uses all 77 after @LIBS followed by @BUILD.)

FOUR TINY-C VOLUMES: TINY-C SPLIT INTO TINY-C (USE) AND TCSRC (SOURCES)
---------------------------------------------------------------------------
TINY-C.BLK keeps everything needed to use Tiny-C -- TINYC.CODE, TCLIB.OBJ,
TCMSGS.TEXT, the headers (*.H), README.TEXT, FILES.TEXT -- and keeps its
name: the compiler looks for TCLIB.OBJ, TCMSGS.TEXT and headers on
TINY-C: when they are not on the prefix or boot volume. The new TCSRC.BLK
holds the compiler's and the library's sources and the batch files that
rebuild them: with the prefix on TCSRC: (and TINY-C: mounted), X(ecute
TINY-C:TINYC and answer @BUILD (TCSRC:TINYC2.CODE) or @LIBS
(TCSRC:TCLIB2.OBJ); TCEXTRA:CMPCODE checks them against TINY-C:'s.

    TINY-C.BLK   20 files (after @DEMOS / @TESTS elsewhere: still 20)
    TCSRC.BLK    31 files; 59 after @LIBS and @BUILD
    TCEXTRA.BLK  27 files; 41 after @DEMOS
    TCTESTS.BLK  27 files; 41 after @TESTS
(a UCSD directory holds 77). Nothing in the files themselves changed.

BIGGY REVISION 1.13: THE SHELL LOOKS FOR A PROGRAM ON EVERY DISK
---------------------------------------------------------------------------
SYSTEM.SHELL (and SHELL.C/.CODE on TCEXTRA): a program name without a
volume (ARGS rather than TCEXTRA:ARGS or #10:ARGS) is looked for on every
disk unit on line (4, 5, 9..14: the directory of each is read). Found
once, it runs from there; found on several disks, the shell lists them
(VOLUME:NAME and its unit) and asks which one to run (RETURN: none); not
found: "no such program". A name with a volume runs as before.
VERSION.TEXT revision 1.13. Nothing else changed.

BIGGY REVISION 1.14: NO TINYC.CODE ON BIGGY; ONE-KEY CHOICE IN THE SHELL
---------------------------------------------------------------------------
TINYC.CODE is no longer on BIGGY: the compiler is TINY-C:TINYC (X(ecute
TINY-C:TINYC). TCLIB.OBJ, TCMSGS.TEXT and the headers stay: the compiler
looks for them on the prefix volume, then the boot volume, then TINY-C:.
SYSTEM.SHELL (and TCEXTRA's SHELL): when a program is on several disks
(up to 8, one per disk unit), the shell lists them and one key chooses --
1..n runs that one at once, no RETURN; any other key runs nothing.
VERSION.TEXT revision 1.14.

BIGGY REVISION 1.15: TINY-C ONLY ON TINY-C:; THE COMPILER IS CC.CODE
---------------------------------------------------------------------------
One copy of each file: TCLIB.OBJ, TCMSGS.TEXT and the headers (*.H) left
BIGGY (TINYC.CODE already left in 1.14). Tiny-C is on TINY-C: only; the
compiler finds them there (it looks on the prefix volume, the boot volume,
then TINY-C:). BIGGY keeps SYSTEM.SHELL, which $ runs.

The compiler's code file is now CC.CODE (was TINYC.CODE): X(ecute
TINY-C:CC. @BUILD on TCSRC: links CC2.CODE (was TINYC2.CODE); compare it
with TINY-C:CC.CODE using TCEXTRA:CMPCODE. TINY-C.BLK, TCSRC.BLK (BUILD.TEXT,
LIBS.TEXT, README.TEXT, MAIN.C comments), TCEXTRA.BLK and TCTESTS.BLK
(README.TEXT, DEMOS.TEXT, TESTS.TEXT, CMPCODE.C comment) say CC.
VERSION.TEXT revision 1.15.

CC TAKES ITS COMMANDS AS ARGUMENTS (TINY-C, TCSRC)
---------------------------------------------------------------------------
The compiler (TINY-C:CC.CODE) runs the commands given as arguments, from
the shell ($ at the Command: prompt), without a prompt: with the prefix on
TCSRC:, CC @BUILD @LIBS rebuilds the compiler (CC2.CODE) and the library
(TCLIB2.OBJ); CC /Z HANOI SIEVE compiles two programs. Options (/Z, /C,
/L, /J) and the word after them make one command; the first that fails
stops CC with exit status 1. X(ecute TINY-C:CC still prompts. TINY-C.BLK
(CC.CODE, README.TEXT) and TCSRC.BLK (MAIN.C, README.TEXT) updated; BIGGY
unchanged.

CC: A NEW LINE AFTER LINKING (TINY-C, TCSRC)
---------------------------------------------------------------------------
After linking, CC now ends the line ("(15677 words free)"), so the next
command of CC @BUILD @LIBS ("> @LIBS") and "Done." start a line of their
own. TINY-C.BLK (CC.CODE) and TCSRC.BLK (MAIN.C) updated; BIGGY unchanged.

GEN.C SPLIT IN TWO: @BUILD WORKS IN Z80 MODE AGAIN (TINY-C, TCSRC)
---------------------------------------------------------------------------
In Z80 mode (and P-Code mode without reclaimed memory) CC ran out of stack
compiling GEN.C, the largest module (*STK OFLOW*, exit status -2 in the
shell).  GEN.C is now two modules: GEN.C (the emitter, procedures, the
object file) and GENX.C (code for expressions, calls, switch), with their
shared declarations in GEN.H; both are in segment GEN.  CC @BUILD @LIBS
now runs in Z80 mode.  TCSRC.BLK (GEN.C, GENX.C, GEN.H, BUILD.TEXT,
README.TEXT) and TINY-C.BLK (CC.CODE) updated; BIGGY unchanged.

MEMMARK; MEMFILL/MEMGAP IN PSYS.H (TINY-C, TCSRC, TCEXTRA)
---------------------------------------------------------------------------
PSYS.H: memfill() fills the free memory with a pattern; memgap() in a
program run later returns the longest run of it still intact, an estimate
of the least free memory of the programs in between.  TCEXTRA:MEMMARK
(MEMMARK FILL, the program, MEMMARK SCAN) does that.  The exact figure:
the emulator 1.97, Options > Track Least Free Memory.  TINY-C.BLK
(TCLIB.OBJ, PSYS.H), TCSRC.BLK (MEMSCAN.C, LIBS.TEXT) and TCEXTRA.BLK
(MEMMARK.C, MEMMARK.CODE, README.TEXT) updated; BIGGY unchanged.

COMPILE.C SPLIT FROM STMT.C; @FILE USES LESS STACK (TINY-C, TCSRC)
---------------------------------------------------------------------------
The parser's pass (compile, helpers, pragma) moved from STMT.C to the new
COMPILE.C: STMT.C needed one more 1 KB block of declarations than the
others.  CC's @FILE batches read into the command buffer instead of a
buffer of their own.  Least free memory in Z80 mode: compiling STMT.C 324
-> 397 words; CC @BUILD @LIBS from the shell 186 -> 352.  TCSRC.BLK
(STMT.C, COMPILE.C, MAIN.C, BUILD.TEXT, README.TEXT) and TINY-C.BLK
(CC.CODE) updated; BIGGY unchanged.

CC USES LESS MEMORY: PCHUNK 256, FILES BETWEEN PASSES (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC's permanent pool takes 256-byte blocks (was 1 KB); files CC opens
between passes (does NAME.C exist, the @FILE batch) no longer leave their
buffer under the next pass; the prompt takes 149 characters; include
names are held in 30 bytes.  Least free memory in Z80 mode compiling
every module from X(ecute: 397 -> 1069 words; CC @BUILD @LIBS from the
shell 352 -> 1024.  TINY-C.BLK (CC.CODE) and TCSRC.BLK (UTIL.C, MAIN.C,
PP.C, COMPILE.C) updated; BIGGY unchanged.

SMALLER CODE GENERATOR BUFFERS (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC's code generator allocated about 9,000 words of buffers for the largest
possible procedure and a 1024-case switch; they are now sized for 1.5
times the largest procedure of the compiler, library, demos and tests, and
the case table for each switch.  Its permanent pool takes 512-byte blocks.
Least free memory in Z80 mode compiling every module from X(ecute: 1069 ->
1497 words; CC @BUILD @LIBS from the shell 1024 -> 1452.  TINY-C.BLK
(CC.CODE) and TCSRC.BLK (TC.H, GEN.C, IR.C, UTIL.C) updated.

CC'S "WORDS FREE" IS EACH PASS'S LEAST (TINY-C, TCSRC)
---------------------------------------------------------------------------
After each pass CC printed the free memory at its end.  With the emulator
1.99 it prints the least free memory during the pass, which the emulator
keeps in SYSCOM^.EXPANSION[8] (PSYS.H: memleast_start, memleast,
memleast_stop); the least of the figures is what Options > Track Least
Free Memory shows.  On another machine CC prints the free memory at the
pass's end as before.  TINY-C.BLK (CC.CODE, PSYS.H) and TCSRC.BLK (MAIN.C)
updated; BIGGY unchanged.

RUNTIME HELPERS DECLARED WHEN NEEDED (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC declared all 35 runtime helpers (__divi, __lmul ...) in every module;
it now declares each when first needed.  Least free memory in Z80 mode:
every @BUILD/@LIBS command from X(ecute 1472 -> 1748 words; CC @BUILD
@LIBS from the shell 1427 -> 1641.  TINY-C.BLK (CC.CODE) and TCSRC.BLK
(EXPR.C, COMPILE.C, PARSE.H) updated; BIGGY unchanged.

LINKER GIVES BACK ITS REFERENCE LISTS (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC's linker frees the lists of what each procedure uses once it knows what
to link, before it writes the code.  Least free memory in Z80 mode linking
CC2.CODE: 1703 -> 2229 words.  TINY-C.BLK (CC.CODE) and TCSRC.BLK (LINK.C,
UTIL.C, TC.H) updated; BIGGY unchanged.

LESS RESIDENT WHILE COMPILING (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC finishes each module's Compiling pass outside the PARSE segment, the
preprocessor converts real constants (the Compiling pass no longer loads
REALLIT deep in an expression), and the linker's tables grow in blocks.
Least free memory in Z80 mode, every @BUILD/@LIBS command from X(ecute:
1777 -> 1953 words; CC @BUILD @LIBS from the shell 1637 -> 1813.
TINY-C.BLK (CC.CODE) and TCSRC.BLK (COMPILE.C, IR.C, LEX.C, PP.C, MAIN.C,
LINK.C, TC.H, PARSE.H) updated; BIGGY unchanged.

SMALLER SYMBOL AND TYPE RECORDS (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC's symbol records are 14 bytes (were 18), its type records 12 (were 22).
Every Compiling pass has about 667 words more; the least free memory of
every @BUILD/@LIBS command in Z80 mode is now the link of CC2.CODE (1954
words from X(ecute, 1909 from the shell).  TINY-C.BLK (CC.CODE) and
TCSRC.BLK (TC.H, PARSE.H, PSYM.C, DECL.C, EXPR.C, STMT.C, COMPILE.C, IR.C,
GENX.C) updated; BIGGY unchanged.

SMALLER LINKER RECORDS (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC's linker keeps 12-byte procedure records (were 16) and static names
without their MODULE' prefix; the permanent pool takes 448-byte blocks.
Least free memory in Z80 mode: linking CC2.CODE 1954 -> 2813 words; every
@BUILD/@LIBS command from X(ecute 1954 -> 2713; CC @BUILD @LIBS from the
shell 1909 -> 2573.  TINY-C.BLK (CC.CODE) and TCSRC.BLK (LINK.C, UTIL.C)
updated; BIGGY unchanged.

SETJMP AND LONGJMP (TINY-C, TCSRC, TCTESTS)
---------------------------------------------------------------------------
The library has setjmp() and longjmp() with a SETJMP.H header: longjmp
returns from the setjmp call again, out of any calls and segments (their
segments are given back as their returns would have).  TINY-C.BLK
(TCLIB.OBJ, SETJMP.H), TCSRC.BLK (SETJMP.C) and TCTESTS.BLK (SETJMP.C,
SETJMP.CODE) updated; BIGGY unchanged.

COMPILER AND LIBRARY FIXES FOUND PORTING VI (TINY-C, TCSRC, TCTESTS)
---------------------------------------------------------------------------
The preprocessor no longer expands a macro again inside its own expansion
(#define rows (G.rows) with f(rows) gave G.(G.rows)), and carries out
#if/#else/#endif lines inside a call spread over several lines.  A static
array inside a function, sized by its initializer, is no longer
initialized over the first global variables.  Writing a text file, a full
page moves the unfinished line to the next page without needing memory
(lines under 512 characters); fputc reports an error instead of dropping
part of a line.  An if / else if chain is parsed in a loop (no stack per
link).  New tests: MACROS, STATICS, TEXTPAGE, ELSEIF on TCTESTS.
TINY-C.BLK (CC.CODE, TCLIB.OBJ), TCSRC.BLK (PP.C, STMT.C, STDIO.C) and
TCTESTS.BLK updated; BIGGY unchanged.

TOOLS AND TOOLSRC (NEW VOLUMES)
---------------------------------------------------------------------------
    TOOLS.BLK    tools written in Tiny-C, ready to run: VI.CODE, the
                 screen editor vi (the BusyBox "tiny vi"): from the shell,
                 VI NAME.C; from X(ecute TOOLS:VI it asks for the file
    TOOLSRC.BLK  their sources: VI.H and the modules VIMAIN.C, VISCREEN.C,
                 VITEXT.C, VICOLON.C, VICMD.C, VIPAGE.C (with VIPAGE.H, the
                 window), VIUCSD.C (the P-System side), and TOOLS.TEXT: with the prefix on
                 TOOLSRC:, X(ecute TINY-C:CC and answer @TOOLS
For now VI.C compiles on the P-System only in P-Code mode with Options >
Reclaim Z80 Interpreter and BIOS Memory and Options > Harvard Mode (it
then gives exactly TOOLS:VI.CODE); in one piece it needs more memory than
the compiler has in the normal layout and in Z80 mode.

VI: A WINDOW INTO BIG FILES; LINKER FIX (TOOLS, TOOLSRC, TINY-C, TCSRC)
---------------------------------------------------------------------------
As the L2 editor does it, VI now keeps only a window of the file in
memory; the rest goes into VI.SWAP (on the prefix volume, deleted when VI
ends) in 1 KB slots, on a stack of the lines before the window and a
stack of the lines after it.  j, k, ^F, ^B ... move the window at its
edges; / ? n N { } search on through the whole file; G, :N, marks and ''
jump anywhere; line numbers count from the file's start.  A range
(5dd, :100,200d, :%s/a/b/) is brought into memory whole, or refused with
a message if it does not fit (working through bigger ranges a window at
a time is still to come).  :w and ZZ write the whole file.  The window is
about 4 KB in Z80 mode, 12 KB in P-Code mode, 30 KB with the Harvard
layout; files up to about 125 KB.  New source VIPAGE.H on TOOLSRC:.
Also: p/P with an empty register no longer makes the next '.' loop for
ever.  The linker took segment lengths over 32767 bytes as negative (VI's
code segment is now 34 KB): CC.CODE and LINK.C, UTIL.C, TC.H updated.
TOOLS.BLK, TOOLSRC.BLK, TINY-C.BLK and TCSRC.BLK updated; BIGGY unchanged.

VI IN MODULES (TOOLSRC, TOOLS)
---------------------------------------------------------------------------
VI.C (123 KB) was too big to edit with VI itself.  It is now VI.H (what
the modules share) and VIMAIN.C, VISCREEN.C, VITEXT.C, VICOLON.C,
VICMD.C, VIPAGE.C, VIUCSD.C (8 to 25 KB each; VIUCSD.H became VIUCSD.C,
VIPAGE.H keeps only the window's declarations).  @TOOLS compiles each
module (/Z /C) and links them (/L VI=...); in modules VI compiles in Z80
mode and the normal layout too (it needed P-Code mode with the Harvard
layout).  VI.CODE does the same as before.  TOOLS.BLK and TOOLSRC.BLK
updated.

BIGGY REVISION 1.16: CD AND DIR IN THE SHELL (BIGGY, TCEXTRA)
---------------------------------------------------------------------------
The Tiny-C shell (SYSTEM.SHELL on BIGGY, SHELL.C and SHELL.CODE on
TCEXTRA) has two commands of its own:
    CD #5  (or CD 5, CD VOL, CD *)   the prefix becomes unit 5's volume,
           as the Filer's Prefix does it (the OS's DKVID): the files of
           later commands, and of the programs they run, are there when
           they name no volume.  CD alone shows the prefix.
    DIR [VOL: or #5:][PATTERN]   the files of the prefix volume, or of
           VOL: or unit 5, whose names match the pattern: * or = any
           characters, ? any one (DIR *.C, DIR #9:, DIR TOOLSRC:VI*.C);
           size, date and kind, then the files and blocks used and free;
           a screen at a time (ESC stops).
VERSION.TEXT revision 1.16.  Big_Disk.BLK and TCEXTRA.BLK updated;
extracted files, report and manifest regenerated.

BIGGY REVISION 1.17: TYPE AND DELETE IN THE SHELL (BIGGY, TCEXTRA)
---------------------------------------------------------------------------
Two more commands in the Tiny-C shell:
    TYPE [VOL: or #5:]NAME   a text file on the console (no pause at a
           screenful); with wildcards (* = ?) each file that matches,
           under its name; a file that is not text: says so.
    DELETE (or DEL) [VOL: or #5:]PATTERN   deletes the files that match;
           with wildcards it lists them first and asks (one key: Y
           deletes, any other keeps them); a plain name: at once.
VERSION.TEXT revision 1.17.  Big_Disk.BLK and TCEXTRA.BLK updated;
extracted files, report and manifest regenerated.

WHICH DISK CC TAKES A FILE FROM (TINY-C, TCSRC)
---------------------------------------------------------------------------
A source, an #include file or an @batch file named with a volume
(TOOLSRC:VI.H, #5:X.C, *X.H) is taken from that volume only; copies of
the same name elsewhere do not matter.  Named without one, CC reads the
directory of every disk unit (4, 5, 9..14):
    on one disk only          that one;
    on several                the one on the disk of the file that names
                              it (the including file; for a source, the
                              @batch file), else the one on the prefix
                              volume (the Filer's Prefix, the shell's CD),
                              else an error (message 117) that lists the
                              volumes, and the compile stops.
This replaces the old order for <x.h> (prefix, boot volume, TINY-C:).
The code that looks (PP.C, segment FIND) is in memory only while it
looks.  TINY-C.BLK (CC.CODE, TCMSGS.TEXT) and TCSRC.BLK (PP.C, MAIN.C,
TC.H) updated; BIGGY unchanged.

BIGGY REVISION 1.18: WHEREIS AND VOLUMES IN THE SHELL (BIGGY, TCEXTRA)
---------------------------------------------------------------------------
    WHEREIS [VOL: or #5:]PATTERN   the files that match (* = ?) on every
           disk on line, each with its unit, volume, size, date and
           kind, then how many on how many volumes; with a volume named,
           on that one only.
    VOLUMES (or VOLS)   every disk on line: unit, volume name, number of
           files, blocks used of the volume's size; the boot volume and
           the prefix volume are marked.
VERSION.TEXT revision 1.18.  Big_Disk.BLK and TCEXTRA.BLK updated;
extracted files, report and manifest regenerated.

BIGGY REVISION 1.19: COPY, MOVE AND RENAME IN THE SHELL (BIGGY, TCEXTRA)
---------------------------------------------------------------------------
    COPY SOURCE DEST   SOURCE is [VOL: or #5:]PATTERN (wildcards * = ?);
           DEST is a volume (#9:, TOOLSRC:), keeping the names, or for one
           file a new name ([VOL:]NAME; without a volume, the prefix).
           The copy keeps the original's kind (a .C file stays a text
           file), date and length; a file of that name there is replaced.
    MOVE SOURCE DEST   the same, then the original is deleted; on its own
           disk only its name changes.
    RENAME (or REN) [VOL: or #5:]NAME NEWNAME   on the same disk; refused
           when NEWNAME is there already.
VERSION.TEXT revision 1.19.  Big_Disk.BLK and TCEXTRA.BLK updated;
extracted files, report and manifest regenerated.

VI: PAGE UP, PAGE DOWN, HOME, END, INSERT, DELETE (TOOLS, TOOLSRC, TINY-C)
---------------------------------------------------------------------------
The emulator types the L2 editor's commands for these keys (Page Down
>P, Page Up <P>, Home JB, End JE, Insert I, Delete D ^U ^C), which VI
took as its own commands.  VI now sets SYSCOM^.EXPANSION[1] to 25605
while it runs (PX_KEYS in PSYS.H, which also names the codes) and clears
it when it ends.  An emulator with the keys change (UCSD-C:
emulator/PSystemEngine-keys.patch, emulator/KEYS.md) then sends one code
each: Home 84H, End 85H, Insert 86H, Delete 87H, Page Up 88H, Page Down
89H, which VI already understands (start/end of the line, insert, delete
a character, a screen back/forward).  The L2 editor and every other
program get the keys as before.  TOOLS.BLK (VI.CODE), TOOLSRC.BLK
(VIUCSD.C) and TINY-C.BLK (PSYS.H) updated; BIGGY unchanged.
The emulator does this from version 2.00 (UCSD-Pascal_Windows_Emulator);
TINY-C.BLK (PSYS.H comment) and TOOLS.BLK (README.TEXT) say so.

GREP (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
TOOLS:GREP.CODE, source TOOLSRC:GREP.C (@TOOLS builds it after VI).  From
the shell:  GREP [-i] PATTERN [VOL: or #5:]FILES ...
    prints each line of a text file that matches as VOL:NAME:LINE: text,
    then how many lines in how many files.  Case is ignored unless -i is
    given (-i makes case count: the other way round from Unix).  FILES
    takes the wildcards * = ? and is looked for on every disk on line
    unless it names a volume.  PATTERN is a regular expression: . any
    character, [abc] [^abc] [a-z] a class, * + ? repeat what is before
    it, ^ $ the start and end of the line, \c the character c, \s a blank
    (the shell splits its words at blanks).
    GREP printf *.C     GREP -i ^int #9:*.H     GREP fopen\s*\( TOOLSRC:VI*.C
TINY-C.BLK (PSYS.H: a comment) updated too; BIGGY unchanged.
GREP update: * in a PATTERN now means any characters, as in file names
(GREP vi*pageup *.C finds VI_K_PAGEUP); . is any one character, x+ and x?
still repeat, \* is a star.  TOOLS.BLK and TOOLSRC.BLK updated.
