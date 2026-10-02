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
