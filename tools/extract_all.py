#!/usr/bin/env python3
"""
Regenerate Files_Extracted_Raw/ and Files_Extracted_no_headers/ from the
UCSD Pascal II.0 volume images in BLK_format/ (v1.03).

  python3 extract_all.py <BLK_format dir> <out dir>

UCSD directory (blocks 2..5): 26-byte volume header, then DNUMFILES live
26-byte entries (first block, last block+1, kind in the low 4 bits of the
third word, name length + 15 name bytes, bytes used in the last block).
ONLY the first DNUMFILES entries are files. Slots after them are left over
from deleted/moved files and point at blocks since reused by other files;
v1.00 of this archive wrongly extracted those too.

.TEXT (kind 3) layout, as written by the UCSD editor: a 1024-byte editor
header, then 1024-byte pages. Each page holds only whole lines; the unused
tail of a page is NUL-filled. Each line is an optional DLE (0x10) followed
by (32 + number of leading spaces), the text, and a CR (0x0D).
The no_headers version: header removed, DLE indents expanded to spaces,
NUL padding dropped, each CR -> CR LF. Every other byte (including TABs,
which the original sources use, and the few damaged bytes noted in the
README) is kept exactly. Non-text files are identical to the Raw version.
"""
import os, sys, struct, hashlib

def w(b, o): return b[o] | (b[o + 1] << 8)

def live_entries(img):
    n = w(img, 1024 + 16)
    for i in range(1, n + 1):
        o = 1024 + 26 * i
        yield dict(first=w(img, o), last=w(img, o + 2), kind=w(img, o + 4) & 15,
                   name=bytes(img[o + 7:o + 7 + img[o + 6]]).decode('latin1'), lastbyte=w(img, o + 22))

def decode_text(raw):
    """Strict decode; returns (bytes_with_crlf, anomalies)."""
    out, anomalies, nlines = bytearray(), [], 0
    if len(raw) < 1024:
        return bytes(raw), [f"shorter than the 1024-byte header ({len(raw)} bytes); copied unchanged"]
    body = raw[1024:]
    if len(body) % 1024:
        anomalies.append(f"body is not a whole number of 1K pages ({len(body)} bytes)")
    for p in range(0, len(body), 1024):
        page, i, pg = body[p:p + 1024], 0, p // 1024
        while i < len(page):
            if page[i] == 0:
                if any(page[i:]):
                    anomalies.append(f"page {pg}: non-NUL bytes after the NUL padding (kept as a line)")
                    i += len(page[i:]) - len(page[i:].lstrip(b'\0'))
                    continue
                break
            indent = 0
            if page[i] == 0x10 and i + 1 < len(page):
                indent = max(page[i + 1] - 32, 0); i += 2
            j = page.find(b'\r', i)
            if j < 0:
                tail = page[i:].rstrip(b'\0')
                anomalies.append(f"page {pg}: last line has no CR -- text is cut off here: {tail[-50:]!r}")
                out += b' ' * indent + tail + b'\r\n'; nlines += 1
                break
            text = page[i:j]
            odd = sorted(set(c for c in text if (c < 0x20 and c != 0x09) or c > 0x7E))
            if odd:
                anomalies.append(f"line {nlines + 1}: unusual byte(s) {[hex(c) for c in odd]} kept as-is")
            out += b' ' * indent + text + b'\r\n'; nlines += 1
            i = j + 1
    return bytes(out), anomalies

def safe(name):
    bad = '<>:"/\\|?*'
    return ''.join('_' if (c in bad or ord(c) < 32) else c for c in name).rstrip('. ') or '_UNNAMED_'

def main(src, dst):
    raw_root, nh_root = os.path.join(dst, 'Files_Extracted_Raw'), os.path.join(dst, 'Files_Extracted_no_headers')
    report, manifest = [], []
    for f in sorted(os.listdir(src)):
        if not f.upper().endswith('.BLK'): continue
        img = open(os.path.join(src, f), 'rb').read()
        disk = f[:-4]
        vol = bytes(img[1024 + 7:1024 + 7 + img[1024 + 6]]).decode('latin1')
        os.makedirs(os.path.join(raw_root, disk), exist_ok=True)
        os.makedirs(os.path.join(nh_root, disk), exist_ok=True)
        seen, ntext = {}, 0
        ents = list(live_entries(img))
        report.append(f"{disk}  (volume {vol}, {len(ents)} files)")
        for e in ents:
            nb = e['last'] - e['first']
            if e['last'] < e['first'] or e['last'] * 512 > len(img) or not (1 <= e['lastbyte'] <= 512):
                report.append(f"    ! {e['name']}: implausible entry {e}; skipped"); continue
            data = img[e['first'] * 512:e['first'] * 512 + (nb - 1) * 512 + e['lastbyte']] if nb else b''
            name = safe(e['name'])
            if name in seen:
                seen[name] += 1; stem, dot, ext = name.rpartition('.')
                name = f"{stem} (dup{seen[name]}).{ext}" if dot else f"{name} (dup{seen[name]})"
                report.append(f"    ! duplicate live directory entry '{e['name']}' (blocks {e['first']}-{e['last']}) saved as '{name}'")
            else:
                seen[name] = 1
            open(os.path.join(raw_root, disk, name), 'wb').write(data)
            nh = data
            if e['kind'] == 3:
                ntext += 1
                nh, anomalies = decode_text(data)
                for a in anomalies: report.append(f"    ! {e['name']}: {a}")
            open(os.path.join(nh_root, disk, name), 'wb').write(nh)
            for tree, content in (('Files_Extracted_Raw', data), ('Files_Extracted_no_headers', nh)):
                manifest.append(f"{hashlib.sha256(content).hexdigest()}  {tree}/{disk}/{name}")
        report.append(f"    {len(ents)} files extracted, {ntext} text files decoded")
    open(os.path.join(dst, 'EXTRACTION_REPORT.txt'), 'w').write('\n'.join(report) + '\n')
    for bd in ('BLK_format', 'raw_8inch_format'):
        p = os.path.join(dst, bd)
        if os.path.isdir(p):
            for f in sorted(os.listdir(p)):
                manifest.append(f"{hashlib.sha256(open(os.path.join(p, f), 'rb').read()).hexdigest()}  {bd}/{f}")
    open(os.path.join(dst, 'MANIFEST_SHA256.txt'), 'w').write('\n'.join(sorted(manifest, key=lambda s: s[66:])) + '\n')
    print('\n'.join(report))

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
