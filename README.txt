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

BIGGY REVISION 1.20: THE SHELL'S COMMAND HISTORY AND LINE EDITING (BIGGY, TCEXTRA)
---------------------------------------------------------------------------
The shell keeps its last 10 commands in #4:SYSTEM.CMDS (written before
each command runs, read when the shell starts or comes back from a
program), so they are there the next time too.  On its command line:
    Up / Down        the commands before / after (Down past the newest:
                     what was being typed)
    Left / Right     a character back / on
    Home / End       the start / end of the line
    Insert           switches between inserting (at the start of every
                     line) and typing over
    Delete           deletes the character at the cursor
    Backspace        deletes the one before it
    ESC              clears the line
Empty lines, BYE and a repeat of the newest are not kept.  Home, End,
Insert and Delete need emulator 2.00 or later (the shell asks for them
through SYSCOM^.EXPANSION[1] while it reads a line).
VERSION.TEXT revision 1.20.  Big_Disk.BLK and TCEXTRA.BLK updated;
extracted files, report and manifest regenerated.

BATCH FILES BUILD ONTO THE VOLUME THE RESULT BELONGS ON (TCSRC, TOOLSRC)
---------------------------------------------------------------------------
    TCSRC:BUILD.TEXT    /L TINY-C:CC2=...     the rebuilt compiler goes
                        next to CC.CODE (rename it CC.CODE to use it)
    TCSRC:LIBS.TEXT     /J TINY-C:TCLIB2=...  the rebuilt library goes
                        next to TCLIB.OBJ (rename it TCLIB.OBJ to use it)
    TOOLSRC:TOOLS.TEXT  /L TOOLS:VI=..., /L TOOLS:GREP=...  each tool is
                        linked straight onto TOOLS: (TOOLS: must be on line)
The objects (NAME.OBJ) stay on the source volume.  The README.TEXT files
of TCSRC: and TOOLSRC: say so.  TCSRC.BLK and TOOLSRC.BLK updated.

ONE BATCH FILE FOR EVERYTHING: TOOLSRC:ALL.TEXT (TOOLSRC, TINY-C, TCSRC)
---------------------------------------------------------------------------
A batch file can now run other batch files: a line @OTHER runs
OTHER.TEXT, then the first one goes on.  TOOLSRC:ALL.TEXT is
    @TCSRC:BUILD     the compiler  -> TINY-C:CC2.CODE
    @TCSRC:LIBS      the library   -> TINY-C:TCLIB2.OBJ
    @TOOLS           every tool    -> TOOLS:
(prefix TOOLSRC:, X(ecute TINY-C:CC, answer @ALL; TCSRC:, TINY-C: and
TOOLS: on line).  An object (NAME.OBJ) now goes on its source's volume,
and /L and /J look for objects the way CC looks for sources, so BUILD's
and LIBS's objects stay on TCSRC:.  CC also no longer keeps memory from
one command to the next (link and join), which a run this long needs.
TINY-C.BLK (CC.CODE, its README), TCSRC.BLK (MAIN.C) and TOOLSRC.BLK
(ALL.TEXT, README) updated.

VI: THE MOUSE WHEEL SCROLLS (TOOLS, TOOLSRC, TINY-C)
---------------------------------------------------------------------------
With emulator 2.01 or later the mouse wheel scrolls VI's screen, 3 lines
a notch (as ^Y / ^E), in command mode; while typing, or on a : line, it
is ignored.  The emulator sends 96H (a notch away from you) and 97H
(towards you) only to a program that asks for one code per key, as VI
does (PSYS.H: KEY_WHEELUP, KEY_WHEELDN).  TOOLS.BLK (VI.CODE, README),
TOOLSRC.BLK (VIUCSD.C) and TINY-C.BLK (PSYS.H) updated.

VI: A BLOCK CURSOR (TOOLS, TOOLSRC, TINY-C)
---------------------------------------------------------------------------
With emulator 2.02 or later the cursor is a block while VI runs (its cell
inverted: green ground, the character black), easy to find; when VI ends
it is the usual line again.  VI asks for it with SYSCOM^.EXPANSION[1] =
25606 (PSYS.H: PX_KEYS_BLOCK; 25605, PX_KEYS, gives the keys alone, as the
shell uses).  TOOLS.BLK (VI.CODE, README), TOOLSRC.BLK (VIUCSD.C) and
TINY-C.BLK (PSYS.H) updated.

BIGGY REVISION 1.21: A SMALLER PRINTF (BIGGY, TINY-C, TCSRC, TCEXTRA, TCTESTS, TOOLS)
---------------------------------------------------------------------------
The library's printf (STDIO.C) uses much less stack: its working
procedure needed 51 words for its locals and now needs 16, because the
flags (- 0 + blank # l) are bits in one word, the digit buffer holds
just a long's 11 digits (the zeros a precision asks for are written,
not stored), and %f %e %g are formatted in a procedure of their own
that is on the stack only while a number is printed.  printf("%d")
takes 38 words (76 bytes) less stack in all; every program that uses
printf is about 500 bytes smaller.  %E and %G now print a capital E
for floats too.  Output is otherwise the same.
TINY-C.BLK (TCLIB.OBJ), TCSRC.BLK (STDIO.C, LIBINT.H), TCTESTS.BLK
(PRINTF.C, a new test, and the relinked tests), TCEXTRA.BLK and
TOOLS.BLK (relinked programs) updated; Big_Disk.BLK: SYSTEM.SHELL
relinked, VERSION.TEXT revision 1.21.

CC NEEDS LESS MEMORY: cc @all FROM THE SHELL HAS 2,954 WORDS FREE, WAS 589 (TINY-C, TCSRC, TOOLS, TOOLSRC)
---------------------------------------------------------------------------
The least free memory of a total rebuild -- the shell ($ at the Command:
prompt), then  cc @all  (TOOLSRC:ALL.TEXT: the compiler, the library, vi
and grep) -- was 589 words (compiling VICMD.C); it is 2,954 now (compiling
VICOLON.C; linking CC2.CODE 2,974, VITEXT.C 2,981, ...).  The compiler
changed, not what it makes: TCLIB.OBJ, BIGGY's SYSTEM.SHELL and the code
files of TCEXTRA and TCTESTS are byte for byte the ones already here.
  - printf/scanf format fix-up no longer loads a 2.3 KB code segment for
    every call of a variadic function; a switch's case labels are not two
    tables in memory that double at 64 cases; statements, parenthesised
    expressions and case-label runs take much less stack in the parser
  - expression nodes are 16 bytes (were 24); the linker allocates its code
    buffer after it has opened the object files, not before; code the
    passes do not need (the /L and /J commands, the message reader, the
    double conversions) is out of the segments that are always in memory
  - the preprocessor stores a macro in one block, and #includes in vi.h
    come before its 150 macros (opening a file needs 1,000 words of stack)
  - vi's < and > are a function of their own (VICMD.C), vi.h includes
    vipage.h first (VI.H), so VI.CODE is new too
TINY-C.BLK (CC.CODE), TCSRC.BLK (the compiler's sources), TOOLS.BLK
(VI.CODE) and TOOLSRC.BLK (VI.H, VICMD.C) updated.  UCSD-TinyC:
tools/shellalltest.py measures it (PSYS_MODE=z80 for the real thing).

TINY-C VERIFY ON TWO VOLUMES: TCVERIF.BLK, TCEXPCT.BLK (new)
---------------------------------------------------------------------------
The Tiny-C Verify content is on two volumes that go with TINY-C, TCSRC,
TCTESTS and TCEXTRA (units 5, 9, 10, 11; these two on 12 and 13):
TCVERIF: VERIFY.SCRIPT (the Verify keyboard script), RMFILES.C/.CODE;
TCEXPCT: NAME.EXPECT, what each of the 26 tests prints.  The script
compiles and runs every test where its source is, rebuilds the compiler
with @BUILD, checks CC2.CODE is IDENTICAL to CC.CODE, and runs HANOI with
CC2.  Passes in P-Code and Z80 mode (UCSD-TinyC tools/tcverify.py).

CC FLUSHES THE CONSOLE AT EACH PASS (TINY-C, TCSRC)
---------------------------------------------------------------------------
The name of the file CC is preprocessing, compiling or generating code
for is on the screen while the pass runs, not after it (a long pass
looked frozen).  cc @all from the shell: 2,898 words free at the least
(the flush costs 56).  TINY-C.BLK (CC.CODE), TCSRC.BLK (MAIN.C) updated.

CC'S PASS LINES IN COLUMNS (TINY-C, TCSRC)
---------------------------------------------------------------------------
Preprocessing NAME  (N free)  Compiling (N free)  Generating code (N free)
on one line of 97 characters: the source's name is padded to 18, the object
name is not repeated and "words" is left out, so a long volume and file
name no longer wraps.  The code is 22 bytes smaller.  cc @all from the
shell: 2,909 words free at the least.  TINY-C.BLK (CC.CODE) and TCSRC.BLK
(MAIN.C) updated.

BINDERC: THE BINDER IN TINY-C (TOOLS, TOOLSRC); VOLUMES REMOVED
---------------------------------------------------------------------------
BINDER.CODE (Big_Disk) has no Pascal source here, so BINDERC is a C
equivalent: TOOLS:BINDERC.CODE, source TOOLSRC:BINDERC.C (tool sources
live on TOOLSRC:; TOOLS: holds the programs).  BINDERC [FILE], or
X(ecute TOOLS:BINDERC, puts the GOTOXY procedure of a code file into
SYSTEM.PASCAL of the prefix volume; it makes the same SYSTEM.PASCAL as
BINDER (UCSD-TinyC tools/bindertest.py, native and Z80 mode).
TOOLS.BLK and TOOLSRC.BLK updated.

Removed from the volume set: the *_pre_8byte_double volumes (earlier) and
U134.4_OS_SOURCE_v1.06, _v1.07 and _v1.10 (_v1.11 and the unnumbered
U134.4_OS_SOURCE.BLK remain).

CC BANNER [0.5] (TINY-C, TCSRC)
---------------------------------------------------------------------------
CC starts with "Tiny-C compiler for UCSD Pascal II.0  [0.5]".  TINY-C.BLK
(CC.CODE) and TCSRC.BLK (MAIN.C) updated.  Not regression tested (a banner).

S, NOT $, STARTS THE SHELL: BIGGY 1.22, U134.4_OS_SOURCE_v1.12 (new)
---------------------------------------------------------------------------
At the Command: prompt the key that runs *SYSTEM.SHELL (the Tiny-C shell) is
now S; the ? prompt says S(hell.  U134.4_OS_SOURCE_v1.12.BLK is v1.11 with
SYSSEGS.B.TEXT GETCMD's '$' changed to 'S' (the command set, the CASE label
and the ? text) and VERSION.TEXT.  GETCMD's segment is 3118 bytes (was
3176: the CASE table is smaller); the rest of SYSTEM.PASCAL is as built
from v1.11.  Big_Disk.BLK (BIGGY) revision 1.22: SYSTEM.PASCAL is the OS
of v1.12 (compiled with the emulator's compiler from the BLK's files);
SYSOLD.PASCAL stays the 1.10 OS; VERSION.TEXT says 1.22.  TINY-C.BLK and
TCSRC.BLK: README texts say S.  Not regression tested (UCSD-TinyC's
scripts and tests type S now).

VI IS FASTER (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
vi compares the screen's rows only when something they depend on changed
(a cursor move within the screen redraws nothing but the cursor: about 4
times faster), puts the cursor codes (FGOTOXY of BIGGY 1.11 or later) into
the output buffer instead of a flush and a call per row, and polls the
keyboard every 8th row (a page of text about 30% faster).  TOOLS.BLK
(VI.CODE) and TOOLSRC.BLK updated.  Checked: UCSD-TinyC tools/viscreentest.py
(every screen the same as before, paged and not) and tools/vitest.py (native
and Z80 mode).

VI SCROLLS WITH THE EMULATOR'S SCROLL CODES (TINY-C, TOOLS, TOOLSRC)
---------------------------------------------------------------------------
Emulator 2.03 (UCSD-Pascal_Windows_Emulator) scrolls the console on request
(ESC [ n S, ESC [ n T, ESC [ top ; bottom r; a program stores 25607 in
SYSCOM^.EXPANSION[1] and the emulator answers 25608).  vi uses them: j or k
at the screen's edge, ^E, ^Y, the mouse wheel and a short ^D / ^U draw only
the rows that came in, not the whole screen.  A page (^F, ^B, Page Down,
Page Up) still draws almost every row.  With an older emulator vi works as
before.  vi's window in Z80 mode is 2.8 KB (was 3 KB: the code is 500 bytes
bigger).  TINY-C.BLK (psys.h), TOOLS.BLK (VI.CODE), TOOLSRC.BLK updated.

VI SHOWS ITS FIRST SCREEN AT ONCE; THE LIBRARY READS A BLOCK AT A TIME (BIGGY 1.23, TINY-C ...)
---------------------------------------------------------------------------
vi no longer reads the whole file before it draws: the first screen is up in
0.4 s in Z80 mode for a 94 KB file (it was 20 s); the rest of the file is read
while vi waits for a key, when the window needs it, and all at once by G, a
search, a : command and :w (the status line's line total counts what is loaded
until then).  A file with a line over 1023 characters or more than 128 chunks
that is found out only after the screen is up stays as far as it was loaded and
:w refuses it.  The Tiny-C library's fread, fgets and fwrite (to the console)
work a block at a time (the P-System's SCAN and block moves), memchr too: vi's
loading, grep, the shell.  vi's calls within the program are 2-byte (#pragma
segment MAIN): 690 bytes less code; its window in Z80 mode stays 2.8 KB.  The
Windows emulator is not changed.  TINY-C, TCSRC, TCEXTRA, TCTESTS, TOOLS,
TOOLSRC (everything relinked with the new library) and Big_Disk.BLK
(SYSTEM.SHELL, revision 1.23) updated.  cc @all from the shell: least free memory
2,900 words (was 2,909: vi.h and vipage.h have a few more declarations).

VI PAGE DOWN IN A SMALL WINDOW (TINY-C, TCSRC, TCEXTRA, TCTESTS, TOOLS, TOOLSRC)
---------------------------------------------------------------------------
Page Down / Up showed a few lines and then nothing (Z80 mode, window 2.5 KB):
vi kept a screen of text on both sides of the cursor, which the window cannot
hold.  Now the screen comes first, then half a screen above it, then the rest
below the cursor; a scroll moves as far as the window allows and brings more in.
Swap file writes go a block at a time.  KNOWN: cc @all least free memory is
2,613 words with this library (2,900 before); a fix follows.

CC @ALL MEMORY BACK TO 2,900 WORDS (TINY-C, TCSRC, TCEXTRA, TCTESTS, TOOLS, TOOLSRC)
---------------------------------------------------------------------------
The block-at-a-time library functions had cost the compiler 240 words: fread
and a fwrite for vi are now a library module of their own (stdioblk.c) that
only vi carries; the compiler and the other programs have the plain ones.
vipage.c is two modules.  cc @all from the shell: 2,900 words free at the least.

VI HAS ITS OWN SMALL FORMATTER (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
vi no longer links the library's printf (2.8 KB of code): its window in Z80 mode
is 4.3 KB, was 2.5 KB (a Page Down page is full more often).  TOOLS.BLK
(VI.CODE) and TOOLSRC.BLK (VI.H, VIUCSD.C, ...) updated.

VI SCROLLS UP WITH LINE FEEDS ON ANY CONSOLE (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
Without the emulator 2.03 scroll codes (older emulators), vi scrolls the screen
up with line feeds sent from the last row and draws only the rows that come in
(j past the bottom, ^E, a short ^D); scrolling down still redraws.  With 2.03 the
scroll codes are used as before.  TOOLS.BLK (VI.CODE) and TOOLSRC.BLK updated.

VI SCROLLS WHEN THE WINDOW MOVES TOO (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
j at the bottom of the screen repainted every row about every other time: a chunk
of the file loaded into the window at the same moment hid the scroll.  vi now finds
the scroll by the top line's line number.  TOOLS.BLK (VI.CODE), TOOLSRC.BLK updated.

VI STATUS LINE SHOWS ITS WINDOW; A CUT-SHORT REPAINT STILL SCROLLS (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
The status line ends with  w<a>+<b> b<lines>/<chunks> a<lines>/<chunks> [L]  : the bytes
of vi's memory window above and below the cursor, what is parked before and after the
window, L while loading.  Screen rows now remember their line numbers, so a repaint that
a typed-ahead 'j' interrupted no longer makes the next ones repaint every row; they scroll.
TOOLS.BLK (VI.CODE) and TOOLSRC.BLK updated.

VI: WINDOW INFO AT THE RIGHT; MOUSE WHEEL SCROLLS ONCE (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
The window information (w.. b.. a.. L) is at the far right of the status line.  Wheel
notches that are waiting are added up (3 lines each) into one scroll and one repaint.
TOOLS.BLK (VI.CODE) and TOOLSRC.BLK updated.

VI: THE WHEEL WAITS ABOUT 70 ms FOR THE NEXT NOTCH (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
Wheel notches that come within about 70 ms of each other make one scroll and one repaint
(it was one tick).  TOOLS.BLK (VI.CODE) and TOOLSRC.BLK updated.

VI: TYPING ON A LONG LINE REFRESHES ONE ROW (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
Putting a character in or taking one out of a line used to make vi format and sum all rows
of the screen again after every key (0.23 s a key in the Z80 emulator, more with a full
screen of long lines; typed-ahead keys piled up).  Now only the row of the edited line is
checked (0.035 s).  TOOLS.BLK (VI.CODE) and TOOLSRC.BLK updated.

VI KNOWS WHEN KEYS ARE WAITING (EMULATOR 2.04) (TOOLS, TOOLSRC, TINY-C)
---------------------------------------------------------------------------
kbhit() (UNITBUSY) is always false on this system, so vi never saw typed-ahead keys and
repainted after every key.  Emulator 2.04 keeps the number of keys waiting in
SYSCOM^.EXPANSION[2] (a program asks with 25609, PX_KEYS_AHEAD in PSYS.H, and gets 25610);
vi uses it: no repaint while keys wait (typing, j, the mouse wheel: one repaint after a
burst).  62 typed spaces: 3.0 s -> 1.0 s in the Z80 emulator.  With an older emulator vi works
as before.  TOOLS.BLK (VI.CODE), TOOLSRC.BLK, TINY-C.BLK (PSYS.H) updated.

VI REQUIRES EMULATOR 2.04; THE FALLBACKS ARE GONE (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
vi no longer works around older emulators: no line-feed scrolling, no PX_KEYS_SCROLL /
PX_KEYS_BLOCK steps, no kbhit.  On an emulator before 2.04 it prints "vi needs emulator
2.04 or later" and ends.  TOOLS.BLK (VI.CODE), TOOLSRC.BLK updated.

THE BREAK KEY: ` (BIGGY 1.24, EMULATOR 2.05)
---------------------------------------------------------------------------
SYSTEM.MISCINFO and NEW.MISCINFO on BIGGY: have the backquote as CRTINFO.BREAK (it was none;
^C is EOF/ETX and stays).  Emulator 2.05 acts on it: typed while a program runs and has not
read a key for 1.5 s (a loop that never ends, a long grep), it stops the program with the
run-time error "Program interrupted by user" (press <space>), back at the command level.  A
` typed at a prompt or soon after the program read a key (vi's mark command) is an ordinary
key.  Big_Disk.BLK updated (the same disk as BIGGY.zip in UCSD-TinyC).

VI: show_help() REMOVED (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
show_help() and what called it are gone from vi: the :features command and the -H / --
options (they now give the usage message like any unknown option).  TOOLS.BLK (VI.CODE)
and TOOLSRC.BLK updated.

GREP IS 20 TIMES FASTER (TOOLS, TOOLSRC)
---------------------------------------------------------------------------
GREP already read the file two blocks at a time (FBLOCKIO), but then looked at every character
in P-code and tried the pattern at every position with three calls.  Now a line is found with
the SCAN-based memchr (CR, DLE), a line without the pattern's first character is skipped without
formatting, a table says which characters a match can begin with, and the pattern is compared
with one call a step: 100 KB of source, a pattern that is not there: 24 s -> 1 s in the Z80
emulator (0.1 s native); a class first ([z]zqq) 34 s -> 5 s.  The same lines as before.
TOOLS.BLK (GREP.CODE) and TOOLSRC.BLK (GREP.C) updated.

SHELL: > >> < REDIRECTION AND | PIPES (BIGGY 1.25: SYSTEM.SHELL; TINY-C, TCSRC, TCEXTRA, TCTESTS, TOOLS, TOOLSRC)
---------------------------------------------------------------------------
COMMAND >FILE puts what a command writes into FILE (>>FILE appends), COMMAND <FILE is its standard
input, A | B | C runs the programs one after the other with each one's output as the next one's
input (a text file #4:PIPE1.TEXT / PIPE2.TEXT, gone when the last one ends).  It works for the
shell's own commands (DIR *.C >LIST.TEXT) and for a program whose main has argc and argv -- the
library's startup takes >NAME and <NAME off its command line (new library module REDIRECT, with
freopen; stdin and stdout are closed at exit when they are files): GREP PAT FILES >OUT.TEXT,
DIR #5:= | GREP .C.  GREP with no FILES reads its standard input.  A name ending in .TEXT is a text
file (what TYPE and the Editor read), any other is raw bytes.  \> \< \| are ordinary characters.
TCLIB.OBJ, TCLIB2 and the programs built with it changed; cc @all is at 2,929 words (it was 2,900).

(The shell's builtin names are a table now: the compiler in the P-System could not compile the long || chain, SHELL.C on TCEXTRA.  SYSTEM.SHELL, TCEXTRA, TOOLSRC etc. rebuilt.)

CAT (TOOLS:CAT.CODE, TOOLSRC:CAT.C)
---------------------------------------------------------------------------
CAT [-AbeEnstTuv] [FILE ...] from the shell: every option of cat(1) except the long ones (--show-all
and so on).  -A -vET, -b number the non-empty lines, -e -vE, -E $ at each line's end, -n number all
lines, -s one empty line for several, -t -vT, -T tabs as ^I, -u ignored, -v control characters as
^X (DEL ^?) and bytes of 128 and up as M-x; joined (-bn, -vET), -- ends the options.  No FILE or -:
the standard input, so the shell's < and | work (DIR #5:= | CAT -n).  .TEXT, .C and .H files are
text, any other file is read as the bytes it holds.  Checked against the host's cat option by option.

HEXDUMP (TOOLS:HEXDUMP.CODE, TOOLSRC:HEXDUMP.C)
---------------------------------------------------------------------------
HEXDUMP [-i] [FILE ...] from the shell (no FILE or -: the standard input; the files are one stream).
A line: the offset in hex, 16 bytes in hex with a blank between them and two more after the 8th, then
the bytes as characters (. for the unprintable):
  00000000  48 65 6C 6C 6F 20 77 6F   72 6C 64 0A 00 01 02 03  Hello world.....
-i: Intel Hex, 16 data bytes a record (:LLAAAATTDD..CC), an extended linear address record every
64 KB, the end record :00000001FF (HEXDUMP -i A.CODE >A.HEX).  .TEXT, .C and .H files are text, any
other file is read as the bytes it holds.  The Intel Hex of a 66 KB file is read back by the host's
objcopy -I ihex byte for byte.

OBJCOPY, MORE, LESS (TOOLS:, sources on TOOLSRC:)
---------------------------------------------------------------------------
OBJCOPY [-I ihex|hexdump] [-O binary] [INFILE [OUTFILE]]: HEXDUMP's output (either form) back into the
bytes; the offsets place them (gaps are filled with 0 bytes), Intel Hex checksums are checked; the
standard input / output without names (HEXDUMP -I A.CODE | OBJCOPY - B.CODE).  UCSD files do not keep a
length: a file that is not .TEXT/.C/.H is whole blocks, the last one filled up with 0 bytes.
MORE [-N] [-s] [-i] [+N] [FILE ...]: a screen at a time with a --More--(NN%) prompt (SPACE, RETURN, d,
/text, n, =, h, q).  LESS [-NSiF] [+N] [FILE ...]: forwards and backwards (SPACE/f, b, RETURN/j, k, d, u, g,
G, Ng, N%, /text, ?text, n, N, :n :p :x, =, h, q; PgUp/PgDn/Home/End and the mouse wheel), any size of
file: it reads 1 KB pages as the keys need them and keeps a sparse index of line starts.  LESS uses the
console's scroll codes (emulator 2.04).  Both take the standard input (DIR | LESS).

^S AND ^Q STOP AND START THE CONSOLE OUTPUT (EMULATOR 2.06)
---------------------------------------------------------------------------
BIGGY's SYSTEM.MISCINFO already had ^S as CRTINFO.STOP; nothing acted on it.  Emulator 2.06 does:
^S typed while a program is running and not waiting for a key stops its console output (the program
waits in its next write), ^Q (also what the arrow keys' Left sends) starts it again; the status bar says
"Output stopped (^S) -- ^Q goes on".  At a prompt, or while a program waits for a key, both are ordinary
keys.  No disk changed.  (Break is `, as before: it ends the program.)

MAN (TOOLS:MAN.CODE, TOOLS:MAN.TEXT, TOOLSRC:MAN.C)
---------------------------------------------------------------------------
MAN [-l] [-k WORD] [NAME ...] from the shell: the manual page of a program on TOOLS: -- what it is for
and every option and key (VI, GREP, CAT, HEXDUMP, OBJCOPY, MORE, LESS, MAN, BINDERC, and the SHELL's
commands, redirection and pipes).  MAN alone lists the pages, -k WORD finds the ones with WORD in them,
a long page stops at --More-- (SPACE, RETURN, q); MAN VI | LESS to go back and forth.  All the text is
the one file MAN.TEXT beside MAN.CODE (one directory slot): a page is a "%% NAME" line and what follows;
the head of MAN.TEXT says how to add one.  The sources keep it as ports/man/MAN.TEXT in UCSD-TinyC, and
tools/mkvolume.py refuses to build TOOLS: when a program on it has no page.

SED AND DIFF (TOOLS:SED.CODE, TOOLS:DIFF.CODE; sources on TOOLSRC:; pages in MAN.TEXT)
---------------------------------------------------------------------------
SED [-nEsur] [-i[SUFFIX]] [-e SCRIPT] [-f FILE] [SCRIPT] [FILE ...]: a stream editor.  Addresses (N $ /re/
first~step ranges addr,+N addr,~N 0,/re/ and !), the commands { } = a b c d D g G h H i l n N p P q Q r s t T
w x y z : #, s with the flags g p N i w and & \1..\9 \n \U \L \u \l \E, y, in basic and (-E) extended regular
expressions with groups and back references (SEDRE.C), -s and -i[SUFFIX] (the original kept as NAME with the
suffix before the extension).  The shell takes < > | itself: write \< \> \| or use -f.  Checked against GNU sed
on 150 scripts.  DIFF [-iwbBqs] [-u|-U N|-c|-C N] FILE1 FILE2: what differs between two text files, normal,
unified or context form, status 0/1/2, files of any size (a 40-line look-ahead for common lines: always a
correct diff, patch-able, not always the shortest); checked against the host's diff and patch.

WC, LC AND PATCH (TOOLS:WC.CODE, TOOLS:LC.CODE, TOOLS:PATCH.CODE; sources on TOOLSRC:; pages in MAN.TEXT)
---------------------------------------------------------------------------------------------------------
WC [-clmwL] [FILE ...]: the lines, words and bytes of files (-L the longest line, tabs to multiples of 8), a
total for several files, the standard input for no FILE or -; the numbers equal the host's wc.  LC
[-1xFsrSC] [VOL: | #n:][PATTERN ...]: the file names of a disk in columns as wide as the console, sorted, down the
columns (-x across), -F marks CODE files, -s sizes, -r reversed, -S by size; one name a line into a file or a
pipe (-C forces columns); "lc" was taken as Plan 9's lc, not a line count.  PATCH [-RsbClN] [-F N] [-pN] [-o OUT]
[-i PATCHFILE] [FILE]: applies normal, unified and context diffs (DIFF's or the host's): hunks that moved,
fuzz up to 2, rejects in NAME.REJ.TEXT, -R, -b (NAME.ORIG.TEXT), -o, -C, names from the headers, hunks of up to
190 lines.  Checked against the host's diff output in all five forms.

VI SAVE FIX, WC WILD CARDS (TOOLS:VI.CODE, TOOLS:WC.CODE)
---------------------------------------------------------
VI: after an edit (x, dd ...) :w and ZZ stopped the emulator with STK OFLOW and left the file as it was.  The
memory vi sets aside for saving a file (a 1 KB page and a file information block) had been handed to the
read of the file being loaded; when that read was closed nothing took the reserve back, a yank then used the
freed block, and the save's fopen had to take new memory, leaving the stack too little for FOPEN.  The reserve
is now taken again when the file being read is closed.  The status line shows the byte count after a :w
(%ld was not understood).  WC: the names may have the wild cards of DIR (WC *.C, WC #5:*.TEXT).

GREP -n (TOOLS:GREP.CODE)
-------------------------
GREP [-i] [-n] PATTERN [[VOL: | #n:]FILE ...]: -n prints the lines that do NOT contain the pattern (-in and -ni
both work), from files and from the standard input; the count line at the end counts the lines printed.

VI REMEMBERS WHERE IT LEFT A FILE (TOOLS:VI.CODE, TOOLS:MAN.TEXT)
-----------------------------------------------------------------
On leaving a text file (.TEXT, .C, .H) that was saved or not changed, vi writes the cursor's line and column
into 7 bytes of the file's 1 KB header page: 'V' 'I' 1, column, line (2 bytes), check, at byte 1000 -- the 892
bytes at the end of the Pascal Editor's page zero (HEAD.TEXT: FILLER) that the Editor never uses.  The next VI
of the file starts at that place (VI -c CMD wins; a file that has become shorter opens at its last line).  When
vi saves a file the library makes a new, empty header page; vi now puts the old one (the Editor's markers,
environment and dates) into a spare slot of VI.SWAP first and writes it back, so the Editor's information
survives vi's saves too (it was lost before).  If the Editor starts a file whose page it has not used it
clears the page and the place is gone: nothing else changes.  Emulator: any.

VI: SAVING TO THE PREFIX VOLUME (TOOLS:VI.CODE)
-----------------------------------------------
The UCSD system gives a new file the largest free piece of the disk, so VI.SWAP (opened when vi starts) took the
whole free area of the prefix volume and the file :w writes found no room ("I/O error") when it was on the same
disk.  VI.SWAP is now made the size the file needs -- the file's size and 25 KB -- by writing its last block and
closing it (the system keeps the size written), then opened again to read and write; the chunk tables are sized by
it too (77 entries instead of 128 for a 60 KB file), which keeps vi's small Z80 window.  Vi's own place record no
longer needs a number-to-text conversion: the jump goes straight to the line.  Emulator: any.

VI: SLOWNESS IN Z80 MODE FIXED; THE PLACE IS IN THE MIDDLE OF THE SCREEN (TOOLS:VI.CODE)
-----------------------------------------------------------------------------------------
The VI of the previous entry was slow and showed ~ rows in Z80 mode: its chunk table (one entry for each piece of
the file kept in VI.SWAP) was sized by the file but the chunks are only about half full, so the table filled and
vi could not move its window; and the window had shrunk (the new code) until it could not hold the screen and the
margin vi keeps, so every key moved chunks in and out.  The table is now two entries to a KB of the file (and
VI.SWAP a slot to an entry), the margin is made smaller when the window cannot hold it, the header code is leaner
(6 bytes: 'V' 'I' 1 column line-low line-high), and a file over 32 KB shows its size in K after :w.  The place
a file is opened at is in the middle of the screen.  Emulator: any.

VI BACK TO THE SAVE FIX (TOOLS:VI.CODE)
---------------------------------------
VI is back to the version of "VI SAVE FIX, WC WILD CARDS" above (UCSD-TinyC commit a525be8): the remembered place
in the header page, the sized VI.SWAP and chunk table and the smaller margin are taken out, to go forward again one
change at a time.  The other tools are as they were (GREP -n stays).

VI OPENS A FILE WHERE IT WAS LEFT (TOOLS:VI.CODE, TOOLS:MAN.TEXT)
-----------------------------------------------------------------
Step 1 after going back to the save fix.  On leaving a text file (.TEXT, .C, .H) that was saved or not changed,
vi writes the cursor's line and column, in binary, into 6 bytes of the file's 1 KB header page: 'V' 'I' 1,
column, line-low, line-high at byte 1000 -- in the 892 bytes at the end of the Pascal Editor's page zero that
the Editor never uses; the rest of the page is read and written back as it was.  The next VI of the file starts
at that place with the line in the middle of the screen (VI -c CMD wins; a file that has become shorter opens at
its last line).  VI.SWAP is unchanged (it still takes the largest free area of the prefix volume).  Cost: about
450 bytes of vi's window in Z80 mode (3236 -> 2788 bytes).  Emulator: any.

VI: THE PLACE IS KEPT AFTER ZZ; THE "L" GOES WHEN THE FILE HAS LOADED (TOOLS:VI.CODE)
-------------------------------------------------------------------------------------
ZZ after a change wrote the file but left vi's "modified" mark set, so the end of the session did not write the
place into the header page (only :w, or leaving an unchanged file, did): fixed.  A big file goes on loading while
vi waits for a key; when it is all in, the status line is now drawn again -- the total number of lines, and no
"L" (still loading) at its right end.  Before, both changed only at the next key.  Emulator: any.

VI: NO -R OR -c; THE RESTART LINE IS IN THE MIDDLE IN Z80 MODE TOO (TOOLS:VI.CODE, TOOLS:MAN.TEXT)
--------------------------------------------------------------------------------------------------
VI has no options now: VI [FILE ...] (-R read-only, -c CMD and EXINIT are gone).  That gives 634 bytes of code
back to vi's window in Z80 mode (3000 -> 3634 bytes), with the 1 KB reserve kept, and the window can now hold
the half screen above the remembered line: a restart (like typing NG at once) puts it in the middle of the
screen in Z80 mode as well, with no "~" rows.  The chunk table is sized by the file (two entries to a KB, plus
24).  VI.CODE is 72 blocks.  Emulator: v2.06.

VI: NO :e OR :f/^G; UNUSED CODE OUT; A SAVE ON THE PREFIX VOLUME WORKS (TOOLS:VI.CODE, TOOLS:MAN.TEXT)
-----------------------------------------------------------------------------------------------------
:e NAME, :e! and :f, ^G are gone (about 470 bytes of code back for vi's window), and so is the code of the
features that were switched off (signals, read-only, window-size queries, locale and 8-bit printing, shell
escapes).  VI.SWAP, vi's temporary file on the prefix volume, is now the size of the file plus 25 KB: vi
writes its last slot, closes it (the rest of the free area is free again) and opens it again to read and
write.  Before, it kept the volume's largest free area, so :w or ZZ of a file on the prefix volume said
"Write error: I/O error".  Vi's window in Z80 mode is about 4 KB (4040 bytes).  VI.CODE is 72 blocks.
Emulator: v2.06.
