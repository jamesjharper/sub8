#!/usr/bin/env python3
import csv
import sys
import re
import unicodedata as ud
from pathlib import Path

# -------------------------------------------------------------------
# Config / constants
# -------------------------------------------------------------------

# We mirror the encoder’s notion of char-set bits:
#   - column idx 0..9  => bit 0..9
#   - column idx >=10  => EXTENDED bit
B5_EXTENDED_IDX = 10

NUM_CHARSETS = B5_EXTENDED_IDX + 1  # 0..10 inclusive
NUM_TABLES   = 3                     # T0, T1, T2 (T3 handled via 10-bit ext)
NUM_CODES    = 32                    # 5-bit codes 0..31

LATIN_BIT    = 0b00000000001
GREEK_BIT    = 0b00000000010
CYRILLIC_BIT = 0b00000000100
EXTENDED_BIT = 0b10000000000

# -------------------------------------------------------------------
# Helpers
# -------------------------------------------------------------------
def remove_control_characters(s):
    return "".join(ch for ch in s if ud.category(ch)[0]!="C")

def try_get_combining_mark(s: str) -> str | None:
    for ch in reversed(s):
        if ud.combining(ch) != 0:
            return ch
    return None

def get_code_point(s: str) -> list[int]:
    if not s:
        return []

    points = [ord(ch) for ch in s]
    if len(points) < 2:
        return points

    # extract multi-codepoint
    cm = try_get_combining_mark(s)
    if cm is not None:
        return [ord(cm)]   

    return points

def parse_header_col(h: str):
    """
    Parse TSV header cells like:  T0[0], T1[10], T2[27]
    Returns (table_index, idx) or None if not a data column.
    """
    m = re.match(r"T([0-2])\[(\d+)\]", h)
    if not m:
        return None
    table = int(m.group(1))  # 0,1,2 for T0,T1,T2
    idx = int(m.group(2))    # 0..27
    return table, idx

def norm_cell_value(v: str) -> str:
    """
    Normalize raw cell content from TSV:
      - strip whitespace
      - empty or '□' => treated as empty / no char
      - "' '" => actual space character
    """

    v = v.strip()

    CELL_CHAR_MAP = {
        "": "",
        "□": "",
        "'□'": "□",
        "' '": " ",
        "'": "\'",
        "\\": "\\\\",
    }
    if v in CELL_CHAR_MAP:
        return CELL_CHAR_MAP[v]
    return v

def cell_value_into_code_points(v: str) -> list[int]:
    """
    Convert a TSV cell representing one printable/symbolic character
    into one or more 32-bit codepoint integers.

    Returns:
        [] if empty or placeholder '□'
        [ord(ch), ...] for actual characters or multi-codepoint sequences
    """

    # Blank / missing cell
    if v == "":
        return []

    # Space explicitly encoded as "' '"
    if v == "' '" or v == " ":
        return [ord(" ")]

    if v == "'□'":
        return [ord("□")]

    # Map of symbolic names to control characters
    control_map = {
        "NUL": "\x00",
        "SOH": "\x01",
        "STX": "\x02",
        "ETX": "\x03",
        "EOT": "\x04",
        "ENQ": "\x05",
        "ACK": "\x06",
        "BEL": "\x07",
        "BS":  "\x08",
        "VT":  "\x0B",
        "FF":  "\x0C",
        "SO":  "\x0E",
        "SI":  "\x0F",
        "DLE": "\x10",
        "DC1": "\x11",
        "DC2": "\x12",
        "DC3": "\x13",
        "DC4": "\x14",
        "NAK": "\x15",
        "SYN": "\x16",
        "ETB": "\x17",
        "CAN": "\x18",
        "EM":  "\x19",
        "SUB": "\x1A",
        "ESC": "\x1B",
        "FS":  "\x1C",
        "GS":  "\x1D",
        "RS":  "\x1E",
        "US":  "\x1F",
        "NBSP": "\u00A0",
        "SHY":  "\u00AD",
    }

    # Symbolic name (control, NBSP, SHY)
    if v in control_map:
        ch = control_map[v]
        return get_code_point(ch)

    # Literal backslash in TSV: encoded as "\\"
    if v == r"\\":
        return get_code_point("\\")

    # Escaped control sequences
    if v == r"\t":
        return get_code_point("\t")
    if v == r"\n":
        return get_code_point("\n")
    if v == r"\r":
        return get_code_point("\r")

    # Otherwise treat the cell as literal characters.
    # It may contain multiple Unicode codepoints (e.g. combining marks).
    return get_code_point(v)

def cpp_char32_literal(ch: str) -> str:
    """
    Produce a C++11 char32_t literal for a single Unicode codepoint.

    Examples:
      'a'      -> U'a'
      '\\n'     -> U'\\n'
      'ø'      -> U'\\u00F8'
      '𝄞' (U+1D11E) -> U'\\U0001D11E'
    """


    o = cell_value_into_code_points(ch)[0]

    # Special handling for '
    if  o == 0x27:
        return f"U'\\u{o:04X}'"
    
    # Printable ASCII
    if 0x20 <= o <= 0x7E:
        return f"U'{ch}'"

    # BMP
    if o <= 0xFFFF:
        return f"U'\\u{o:04X}'"
    # Beyond BMP
    return f"U'\\U{o:08X}'"

def load_tsv(path: Path):
    with path.open(encoding="utf-8") as f:
        lines = f.read().splitlines()
    rows = list(csv.reader(lines, delimiter="\t"))
    if not rows:
        raise RuntimeError("TSV file is empty")
    header = rows[0]
    data_rows = rows[1:]
    return header, data_rows

def bit_index(mask: int) -> int:
    """
    Given a power-of-two mask, return its bit index.
    Example: 1<<0 -> 0, 1<<5 -> 5.
    """
    if mask == 0 or (mask & (mask - 1)) != 0:
        raise ValueError(f"mask {mask} is not a single bit")
    idx = 0
    while mask > 1:
        mask >>= 1
        idx += 1
    return idx


# -------------------------------------------------------------------
# Build char_info (same semantics as encoder)
# -------------------------------------------------------------------

def build_char_info(header, data_rows):
    """
    Build a dict:

      char_info[ch] = {
          "addr": row_index,          # 0-based row index (a=0, b=1, ...)
          "ext":  ext_index,          # 0 for base, idx-10 for extended columns
          "tables": set({0,1,2}),     # which T0/T1/T2 columns it appears in
          "char_sets": set({bitmask}) # bitmasks for column-based char-set (0..9, 10=EXTENDED)
      }

    NOTE: This mirrors the encoder's logic: column idx 0..9 are direct char-set
    bits, idx>=10 collapse to EXTENDED bit.
    """
    col_meta = {}
    for col_idx, h in enumerate(header):
        parsed = parse_header_col(h)
        if parsed is None:
            continue
        table, idx = parsed    # table=0..2, idx=0..27
        col_meta[col_idx] = {"table": table, "idx": idx}

    char_info = {}

    for row_idx, row in enumerate(data_rows):
        for col_idx, raw in enumerate(row):
            meta = col_meta.get(col_idx)
            if not meta:
                continue

            val = norm_cell_value(raw)
            if not val:
                continue

            ch = val
            cps = cell_value_into_code_points(val)
            table = meta["table"]  # 0,1,2
            idx  = meta["idx"]     # column index

            addr = row_idx
            ext  = idx
            ext_char_idx  = idx - 10 if idx >= 10 else 0

            # Column-derived char-set bit:
            #   - 0..9 => direct bit
            #   - >=10 => EXTENDED bit
            cs_bit_idx = idx if idx < B5_EXTENDED_IDX else B5_EXTENDED_IDX
            cs_mask = 1 << cs_bit_idx

            info = char_info.get(ch)
            if info is None:
                info = {
                    "addr": addr,
                    "ext": ext,
                    "ext_char_idx": ext_char_idx,
                    "tables": {table},
                    "char_sets": {cs_mask},
                    "code_points": cps
                }
                char_info[ch] = info
            else:
                # Address/ext must match the encoder’s canonical position
                if info["addr"] != addr or info["ext_char_idx"] != ext_char_idx:
                    # This just logs; canonical remains the first occurrence
                    info.setdefault("conflicts", set()).add((addr, ext))

                info["tables"].add(table)
                info["char_sets"].add(cs_mask)

    return char_info


# -------------------------------------------------------------------
# Build decoder LUTs
# -------------------------------------------------------------------

def build_decoders(char_info):
    """
    Build:
      - base_lut[NUM_CHARSETS][NUM_TABLES][NUM_CODES] -> char (or '\0')
      - ext_lists[NUM_TABLES]: per-table list of (code10, ch) for 10-bit extended chars
    """
    # Initialize base LUT with NULs
    base_lut = [
        [
            ["\u0000" for _ in range(NUM_CODES)]  # codes 0..31
            for _ in range(NUM_TABLES)            # tables T0..T2
        ]
        for _ in range(NUM_CHARSETS)              # charsets 0..10
    ]

    # Per-table extended entries: ext_entries_per_table[table] = [(code10, ch), ...]
    ext_entries_per_table = [[] for _ in range(NUM_TABLES)]

    for ch, info in char_info.items():
        addr   = info["addr"]      # 0..(rows-1)
        ext    = info["ext"]       # 0 for base, >0 for extended columns
        ext_char_idx = info["ext_char_idx"]       # 0 for base, >0 for extended columns
        tables = info["tables"]    # set of table indices (0,1,2)
        char_sets = info["char_sets"]

        for cs_mask in char_sets:
            cs_idx = bit_index(cs_mask)           # 0..10

            for tbl in tables:
                # Base 5-bit entries: ext == 0 AND charset not EXTENDED
                if ext_char_idx == 0 and cs_idx < B5_EXTENDED_IDX:
                    # This is a normal 5-bit code in (cs_idx, tbl, addr)
                    existing = base_lut[cs_idx][tbl][addr]
                    if existing not in ("\u0000", ch):
                        # Optional: debug conflicting cells
                        print(f"Conflict at charset={cs_idx} table={tbl} code={addr}: {existing!r} vs {ch!r}", file=sys.stderr)
                        pass
                    base_lut[cs_idx][tbl][addr] = ch
                else:
                    # Extended (10-bit) entry: ext>0 or charset EXTENDED
                    code10 = (addr << 5) | (ext & 0x1F)
                    ext_entries_per_table[tbl].append((code10, ch))

    # Deduplicate / canonicalize ext entries per table by code10 (keep first)
    ext_lists = []
    for tbl in range(NUM_TABLES):
        ext_map = {}
        for code10, ch in ext_entries_per_table[tbl]:
            if code10 not in ext_map:
                ext_map[code10] = ch
        ext_list = sorted(ext_map.items(), key=lambda x: x[0])
        ext_lists.append(ext_list)

    return base_lut, ext_lists


# -------------------------------------------------------------------
# Header generation
# -------------------------------------------------------------------

def generate_decode_header(base_lut, ext_lists, tsv_path: Path, out_path: Path):
    lines = []
    lines.append("// Auto-generated from: {}".format(tsv_path.name))
    lines.append("// Do not edit this file directly; regenerate from the TSV instead.")
    lines.append("")
    lines.append("// B5DecoderLut[charset_index][table_index][code]")
    lines.append("// table_index:   0=T0, 1=T1, 2=T2")
    lines.append("// code:          0..31 (5-bit)")
    lines.append("")

    # ---------------------------------------------------------------
    # Determine which charsets are non-empty (any non-'\0' entry)
    # ---------------------------------------------------------------
    non_empty_charsets = []
    for cs_idx in range(NUM_CHARSETS):
        is_empty = True
        for tbl_idx in range(NUM_TABLES):
            for ch in base_lut[cs_idx][tbl_idx]:
                if ch != "\u0000":
                    is_empty = False
                    break
            if not is_empty:
                break

        if not is_empty:
            non_empty_charsets.append(cs_idx)

    logical_charset_count = len(non_empty_charsets)

    # ---------------------------------------------------------------
    # Size constants
    # ---------------------------------------------------------------
    lines.append(f"const std::size_t B5_NUM_CHARSETS = {logical_charset_count};")
    lines.append("")

    # ---------------------------------------------------------------
    # Base LUT (only non-empty charsets)
    # ---------------------------------------------------------------
    lines.append("const char32_t B5DecoderLut[B5_NUM_CHARSETS][B5_NUM_TABLES][B5_NUM_CODES] = {")
    for logical_idx, cs_idx in enumerate(non_empty_charsets):
        lines.append(f"    {{ // charset logical index {logical_idx}, original index {cs_idx}")
        for tbl_idx in range(NUM_TABLES):
            row = base_lut[cs_idx][tbl_idx]
            elems = []
            for ch in row:
                if ch == "\u0000":
                    elems.append("U'\\0'")
                else:
                    elems.append(f"{cpp_char32_literal(ch)} /* {ch} */")
            lines.append(f"        {{ " + ", ".join(elems) + f" }}, // table {tbl_idx}")
        lines.append("    },")
    lines.append("};")
    lines.append("")

    # ---------------------------------------------------------------
    # Extended LUT: per-table arrays
    # ---------------------------------------------------------------

    # One array per table: B5DecoderLutExt_T0, B5DecoderLutExt_T1, B5DecoderLutExt_T2
    for tbl_idx, ext_list in enumerate(ext_lists):
        lines.append(f"static constexpr B5DecoderExtEntry B5DecoderLutExt_T{tbl_idx}[] = {{")
        for code10, ch in ext_list:
            lines.append(f"    {{ 0x{code10:04X}, {cpp_char32_literal(ch)} }}, /* {ch} */")
        lines.append("};")
        lines.append("")

    # Array-of-pointers to per-table arrays: effectively B5DecoderExtEntry[B5_NUM_TABLES][]
    lines.append("const B5DecoderExtEntry* B5DecoderLutExt[B5_NUM_TABLES] = {")
    for tbl_idx in range(NUM_TABLES):
        lines.append(f"    B5DecoderLutExt_T{tbl_idx},")
    lines.append("};")
    lines.append("")

    # Per-table counts
    lines.append("const std::size_t B5DecoderLutExtCount[B5_NUM_TABLES] = {")
    for tbl_idx in range(NUM_TABLES):
        lines.append(
            f"    sizeof(B5DecoderLutExt_T{tbl_idx}) / sizeof(B5DecoderLutExt_T{tbl_idx}[0]),"
        )
    lines.append("};")
    lines.append("")

    out_path.write_text("\n".join(lines), encoding="utf-8")

from pathlib import Path
import sys

def generate_encode_header(char_info, tsv_path: Path, out_path: Path):
    """
    Emit a header with:

      const B5EncoderLutNode B5EncoderLut[] = {
          /* a */ { U'a', FiveBitCharAddress{ addr, ext, charsetFlags, tableFlags } },
          ...
      };

    Sorted by ch (codepoint) ascending for binary search.
    """
    conflict_chars = {
        ch: info for ch, info in char_info.items() if "conflicts" in info
    }
    ignored_chars = {
        ch: info for ch, info in char_info.items() if len(info["code_points"]) >= 2
    }
    valid_chars = [
        c for c in char_info.keys()
        if len(cell_value_into_code_points(c)) == 1
    ]


    lines = []
    lines.append("// Auto-generated from: {}".format(tsv_path.name))
    lines.append("// Do not edit this file directly; regenerate from the TSV instead.")
    lines.append("")

    if conflict_chars:
        lines.append("// NOTE: Some characters appear at multiple (addr, ext) positions.")
        lines.append("// The first occurrence encountered is used as canonical; others are noted here:")
        for ch in sorted(conflict_chars, key=lambda c: cell_value_into_code_points(c)[0]):
            display_char = remove_control_characters(ch)
            info = conflict_chars[ch]
            canon = (info["addr"], info["ext"])
            others = ", ".join(f"({a},{e})" for a, e in sorted(info["conflicts"]))
            lines.append( f"// '{display_char}' ({cpp_char32_literal(ch)}): canonical=(addr={canon[0]}, ext={canon[1]}) ; others={others}"
            )
        lines.append("")


    if ignored_chars:
        lines.append("// NOTE: Some characters are multi characters.")
        lines.append("// The following chars have been excluded from the LUT")
        for ch in ignored_chars:
            display_char = remove_control_characters(ch)
            info = ignored_chars[ch]
            canon = (info["addr"], info["ext"])
            lines.append( f"// '{display_char}' ({cpp_char32_literal(ch)}): canonical=(addr={canon[0]}, ext={canon[1]})"
            )
        lines.append("")

    lines.append("const B5EncoderLutNode B5EncoderLut[] = {")

    # Sort by codepoint so binary search on .ch is stable
    for ch in sorted(valid_chars, key=lambda c: cell_value_into_code_points(c)[0]):
        info = char_info[ch]
        addr = info["addr"]
        ext  = info["ext"]

        # Table flags: map {0,1,2} -> T0, T1, T2
        table_bits = []
        if 0 in info["tables"]:
            table_bits.append("B5_T0")
        if 1 in info["tables"]:
            table_bits.append("B5_T1")
        if 2 in info["tables"]:
            table_bits.append("B5_T2")
        table_expr = " | ".join(table_bits) if table_bits else "0"

        char_set_bits = []
        if LATIN_BIT in info["char_sets"]:
            char_set_bits.append("B5_LATIN")
        if GREEK_BIT in info["char_sets"]:
            char_set_bits.append("B5_GREEK")
        if CYRILLIC_BIT in info["char_sets"]:
            char_set_bits.append("B5_CYRILLIC")
        if EXTENDED_BIT in info["char_sets"]:
            char_set_bits.append("B5_EXTENDED")
        
        charset_expr = " | ".join(char_set_bits) if char_set_bits else "0"

        # For readability: inline comment showing glyph.
        # (Even for weird symbols / combining marks it’s nice to see something.)
        comment_display = remove_control_characters(ch)

        # Make use of the following #define B5_CHAR_ADDRESS(CH, ROW, EXT, CS, TBL) { CH, FiveBitCharAddress{ ROW, EXT, CS, TBL } }
        lines.append(
            f"    /* {comment_display} */ "
            f"{{ {cpp_char32_literal(ch)}, "
            f"B5_CHAR_ADDRESS( {addr}, {ext}, {charset_expr}, {table_expr} ) }},"
        )

    lines.append("};")
    lines.append("")

    out_path.write_text("\n".join(lines), encoding="utf-8")

# -------------------------------------------------------------------
# CLI
# -------------------------------------------------------------------
def main():
    if len(sys.argv) < 4:
        print("Usage:", file=sys.stderr)
        print("  sub8_b5_lut.py encode <in.tsv> <out.h>", file=sys.stderr)
        print("  sub8_b5_lut.py decode <in.tsv> <out.h>", file=sys.stderr)
        sys.exit(1)

    mode = sys.argv[1]
    tsv_path = Path(sys.argv[2])
    out_path = Path(sys.argv[3])

    if not tsv_path.exists():
        print(f"Error: TSV file not found: {tsv_path}", file=sys.stderr)
        sys.exit(1)

    header, data = load_tsv(tsv_path)
    char_info = build_char_info(header, data)

    if mode == "decode":
        
        base_lut, ext_lists = build_decoders(char_info)
        generate_decode_header(base_lut, ext_lists, tsv_path, out_path)

        print(f"Wrote decoder LUT header to {out_path}", file=sys.stderr)

    elif mode == "encode":

        generate_encode_header(char_info, tsv_path, out_path)

        print(f"Wrote encoder LUT header to {out_path}", file=sys.stderr)
    else:
        print(f"Unknown mode: {mode}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
