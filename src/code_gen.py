#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import sys
from typing import Any, Dict, List, Tuple

import yaml


# ------------------------------------------------------------
# Utilities
# ------------------------------------------------------------

def read_text(path: str) -> str:
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def ensure_dir(path: str) -> None:
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)


def banner(title: str) -> str:
    return (
        "\n// ============================================================\n"
        f"// {title}\n"
        "// ============================================================\n\n"
    )


def split_ns(ns: str) -> List[str]:
    return [p for p in ns.split("::") if p]


def ns_open(ns: str | None) -> str:
    if not ns:
        return ""
    parts = split_ns(ns)
    out = []
    for p in parts:
        out.append(f"namespace {p} {{\n")
    return "".join(out)


def ns_close(ns: str | None) -> str:
    if not ns:
        return ""
    parts = split_ns(ns)
    out = []
    for p in reversed(parts):
        out.append(f"}} // namespace {p}\n")
    return "".join(out)


def qualify(ns: str | None, name: str) -> str:
    return f"{ns}::{name}" if ns else name


# ------------------------------------------------------------
# Op normalization
# ------------------------------------------------------------

def normalize_op(raw: Any) -> Dict[str, Any]:
    """
    Supports:
      - {op: "...", ...}       (your v2 format)
      - {type: "...", ...}     (older)
      - {"include": {...}}     (single-key)
    """
    if not isinstance(raw, dict):
        raise TypeError(f"Op must be dict, got {type(raw).__name__}: {raw!r}")

    if "op" in raw:
        out = dict(raw)
        out["type"] = out.pop("op")
        return out

    if "type" in raw:
        return dict(raw)

    if len(raw) == 1:
        (k, v), = raw.items()
        if v is None:
            v = {}
        if not isinstance(v, dict):
            raise TypeError(f"Op '{k}' value must be dict, got {type(v).__name__}")
        out = dict(v)
        out["type"] = k
        return out

    raise KeyError(f"Op missing 'op'/'type' and not single-key: {list(raw.keys())}")


# ------------------------------------------------------------
# Include / inline
# ------------------------------------------------------------

def _installed_cpp_src_dir() -> str:
    """Default location of shipped sub8 C++ headers for inline-includes.

    In a source checkout this resolves to: <repo>/src/languages/cpp/src
    In a wheel install, you should package these files alongside the module and
    adjust this function to use importlib.resources (future).
    """
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(here, "languages", "cpp", "src")


def _resolve_inline_include_path(inc_path: str, repo_root: str) -> str:
    """Resolve an include path for inlining.

    Search order:
      1) repo_root / inc_path (honors codegen.sub8_file_path that stage2 baked into inc_path)
      2) installed C++ stdlib dir / <basename-or-tail>

    Raises FileNotFoundError with a helpful message if not found.
    """
    candidates: list[str] = []

    # 1) As-is under repo_root
    candidates.append(os.path.join(repo_root, inc_path))

    # 2) Fall back to installed C++ headers location.
    inc_norm = inc_path.replace("\\", "/")
    inc_norm = inc_norm.lstrip("./")
    tail = inc_norm.split("/")[-1] if inc_norm else inc_norm
    candidates.append(os.path.join(_installed_cpp_src_dir(), tail))

    # Optional user override(s) via env (colon-separated)
    env = os.environ.get("SUB8_CPP_INCLUDE_PATH", "") or os.environ.get("SUB8_CPP_INCLUDE_DIR", "")
    if env:
        for p in env.split(os.pathsep):
            p = p.strip()
            if not p:
                continue
            candidates.append(os.path.join(p, tail))

    for c in candidates:
        if c and os.path.exists(c):
            return c

    raise FileNotFoundError("Inline include not found. Tried:\n  " + "\n  ".join(candidates))


def emit_include(op: Dict[str, Any], repo_root: str, global_inline_headers: bool) -> str:
    inc_path = op["path"]

    # master switch: if global is false, do not inline anything
    inline = bool(global_inline_headers) and bool(op.get("inline", True))

    if not inline:
        return f'#include "{inc_path}"\n'

    full = _resolve_inline_include_path(inc_path, repo_root=repo_root)
    return banner(f"BEGIN INLINE {inc_path}") + read_text(full) + banner(f"END INLINE {inc_path}")



# ------------------------------------------------------------
# Header emitters
# ------------------------------------------------------------

def emit_define_header(op: Dict[str, Any]) -> str:
    # Match your example: guarded defaults
    name = op["name"]
    value = op.get("value", 0)
    return (
        f"#ifndef {name}\n"
        f"#define {name} {value}\n"
        f"#endif\n"
    )


def emit_alias(op: Dict[str, Any]) -> str:
    return f"using {op['name']} = {op['cpp']};\n"


def emit_object_decl(op: Dict[str, Any]) -> str:
    """
    Emit a DTO-like struct (expanded macro-style), but with:
      - method prototypes only for non-constexpr/non-template methods
      - template read/write free functions emitted in header (must be header)
    """
    name = op["name"]
    ns = op.get("namespace")
    fields: List[Dict[str, Any]] = op.get("fields", [])

    out: List[str] = []
    out.append(ns_open(ns))

    out.append(f"struct {name} {{\n")
    out.append(f"  using Type = {name};\n")
    out.append(f"  using InitType = {name};\n")
    out.append(f"  using ValueType = {name};\n\n")

    # constexpr sizes stay in header
    out.append("  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {\n")
    out.append("    sub8::BitSize len;\n")
    for f in fields:
        out.append(f"    len += {f['cpp']}::MaxPossibleSize;\n")
    out.append("    return len;\n")
    out.append("  }();\n\n")

    out.append("  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {\n")
    out.append("    sub8::BitSize len;\n")
    for f in fields:
        out.append(f"    len += {f['cpp']}::MinPossibleSize;\n")
    out.append("    return len;\n")
    out.append("  }();\n\n")

    out.append("  sub8::BitSize max_possible_size() { return MaxPossibleSize; }\n")
    out.append("  sub8::BitSize min_possible_size() { return MinPossibleSize; }\n")
    out.append("  sub8::BitSize actual_size() const noexcept;\n\n")

    out.append(f"  const {name}& value() const noexcept {{ return *this; }}\n")
    out.append(f"  sub8::BitFieldResult set_value(const {name}& v) noexcept {{ *this = v; return sub8::BitFieldResult::Ok; }}\n\n")

    # fields
    for f in fields:
        out.append(f"  {f['cpp']} {f['name']}{{}};\n")
    out.append("\n")

    out.append(f"  bool operator==(const {name}& o) const noexcept;\n")
    out.append(f"  bool operator!=(const {name}& o) const noexcept {{ return !(*this == o); }}\n")
    out.append("};\n")

    out.append(ns_close(ns))

    # sub8::write_field/read_field templates (header-only)
    qname = qualify(ns, name)
    out.append("\nnamespace sub8 {\n")
    out.append("template <typename Storage>\n")
    out.append(f"inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const {qname}& v) {{\n")
    out.append("  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;\n")
    for f in fields:
        out.append(f"  r = write_field(bw, v.{f['name']});\n")
        out.append("  if (r != sub8::BitFieldResult::Ok) return r;\n")
    out.append("  return sub8::BitFieldResult::Ok;\n")
    out.append("}\n\n")

    out.append("template <typename Storage>\n")
    out.append(f"inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, {qname}& out) {{\n")
    out.append("  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;\n")
    for f in fields:
        out.append(f"  r = read_field(br, out.{f['name']});\n")
        out.append("  if (r != sub8::BitFieldResult::Ok) return r;\n")
    out.append("  return sub8::BitFieldResult::Ok;\n")
    out.append("}\n")
    out.append("} // namespace sub8\n")

    return "".join(out)


def emit_variant_decl(op: Dict[str, Any]) -> str:
    """
    Emit a tagged-union variant like your example, but with method prototypes
    whose definitions will be emitted in the .cpp.
    """
    name = op["name"]
    ns = op.get("namespace")
    opts: List[Dict[str, Any]] = op.get("options", [])

    tag_enum = f"{name}Type"
    tag_meta = f"{name}__VariantTagMeta"
    tag_field_base = f"{name}TypeFieldBase"
    tag_field = f"{name}TypeField"

    # Prepare codes including NullValue=0
    codes: List[Tuple[str, int, str]] = [("NullValue", 0, "sub8::NullValue")]
    for o in opts:
        codes.append((o["name"], int(o["id"]), o["cpp"]))

    out: List[str] = []
    out.append(ns_open(ns))

    # Tag enum
    out.append(f"enum class {tag_enum} : uint32_t {{\n")
    out.append("  NullValue = 0,\n")
    for i, o in enumerate(opts):
        comma = "," if i + 1 < len(opts) else ","
        out.append(f"  {o['name']} = {int(o['id'])}{comma}\n")
    out.append("};\n\n")

    # Tag meta (min/max)
    out.append(f"struct {tag_meta} {{\n")
    out.append("  inline static constexpr uint32_t kCodes[] = {\n")
    for _, cid, _ in codes:
        out.append(f"    {cid}u,\n")
    out.append("  };\n")
    out.append("  inline static constexpr size_t kCount = sizeof(kCodes) / sizeof(kCodes[0]);\n")
    out.append("  inline static constexpr uint32_t MinCodeU32 = []() constexpr noexcept {\n")
    out.append("    uint32_t m = kCodes[0];\n")
    out.append("    for (size_t i = 1; i < kCount; ++i) if (kCodes[i] < m) m = kCodes[i];\n")
    out.append("    return m;\n")
    out.append("  }();\n")
    out.append("  inline static constexpr uint32_t MaxCodeU32 = []() constexpr noexcept {\n")
    out.append("    uint32_t m = kCodes[0];\n")
    out.append("    for (size_t i = 1; i < kCount; ++i) if (kCodes[i] > m) m = kCodes[i];\n")
    out.append("    return m;\n")
    out.append("  }();\n")
    out.append(f"  inline static constexpr {tag_enum} MinEnum = static_cast<{tag_enum}>(MinCodeU32);\n")
    out.append(f"  inline static constexpr {tag_enum} MaxEnum = static_cast<{tag_enum}>(MaxCodeU32);\n")
    out.append("};\n\n")

    out.append(f"using {tag_field_base} = sub8::Enumeration<{tag_enum}, {tag_meta}::MinEnum, {tag_meta}::MaxEnum>;\n")
    out.append(f"struct {tag_field} : {tag_field_base} {{\n")
    out.append(f"  using {tag_field_base}::{tag_field_base};\n")
    out.append(f"  {tag_field}() noexcept = default;\n")
    out.append("};\n\n")

    # Variant struct + union
    out.append(f"struct {name} {{\n")
    out.append(f"  {tag_enum} type{{{tag_enum}::NullValue}};\n")
    out.append("  union VariantValue {\n")
    out.append("    sub8::NullValue null_v;\n")
    for o in opts:
        out.append(f"    {o['cpp']} {o['name']};\n")
    out.append("    VariantValue() {}\n")
    out.append("    ~VariantValue() {}\n")
    out.append("  } variant;\n\n")

    out.append(f"  using Type = {name};\n")
    out.append(f"  using InitType = {name};\n")
    out.append(f"  using ValueType = {name};\n\n")

    # constexpr size bounds (header)
    out.append("  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {\n")
    out.append("    sub8::BitSize payload = sub8::NullValue::MaxPossibleSize;\n")
    for o in opts:
        out.append(f"    if ({o['cpp']}::MaxPossibleSize > payload) payload = {o['cpp']}::MaxPossibleSize;\n")
    out.append(f"    return payload + {tag_field}::MaxPossibleSize;\n")
    out.append("  }();\n\n")

    out.append("  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {\n")
    out.append("    sub8::BitSize payload = sub8::NullValue::MinPossibleSize;\n")
    for o in opts:
        out.append(f"    if ({o['cpp']}::MinPossibleSize < payload) payload = {o['cpp']}::MinPossibleSize;\n")
    out.append(f"    return payload + {tag_field}::MinPossibleSize;\n")
    out.append("  }();\n\n")

    out.append("  sub8::BitSize max_possible_size() { return MaxPossibleSize; }\n")
    out.append("  sub8::BitSize min_possible_size() { return MinPossibleSize; }\n")
    out.append("  sub8::BitSize actual_size() const noexcept;\n\n")

    # lifecycle + helpers (prototypes, cpp definitions)
    out.append("  void construct_null() noexcept;\n")
    out.append("  void destroy_active() noexcept;\n\n")

    out.append(f"  {name}() noexcept;\n")
    out.append(f"  {name}(const {name}& o);\n")
    out.append(f"  {name}({name}&& o) noexcept;\n")
    out.append(f"  {name}& operator=(const {name}& o);\n")
    out.append(f"  {name}& operator=({name}&& o) noexcept;\n")
    out.append(f"  ~{name}();\n\n")

    out.append(f"  const {name}& value() const noexcept {{ return *this; }}\n")
    out.append(f"  sub8::BitFieldResult set_value(const {name}& v) noexcept {{ *this = v; return sub8::BitFieldResult::Ok; }}\n")
    out.append("  bool is_null() const noexcept;\n\n")

    # getters/setters prototypes
    for o in opts:
        out.append(f"  const {o['cpp']}* get_{o['name']}() const noexcept;\n")
        out.append(f"  bool is_{o['name']}() const noexcept;\n")
        out.append(f"  sub8::BitFieldResult set_{o['name']}(const {o['cpp']}& v) noexcept;\n\n")

    out.append(f"  bool operator==(const {name}& o) const noexcept;\n")
    out.append(f"  bool operator!=(const {name}& o) const noexcept {{ return !(*this == o); }}\n")
    out.append("};\n")

    out.append(ns_close(ns))

    # sub8::write_field/read_field templates (header-only)
    qname = qualify(ns, name)
    out.append("\nnamespace sub8 {\n")
    out.append("template <typename Storage>\n")
    out.append(f"inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const {qname}& v) {{\n")
    out.append("  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;\n")
    out.append(f"  {qualify(ns, tag_field)} t; t.set_value(static_cast<{qualify(ns, tag_enum)}>(static_cast<uint32_t>(v.type)));\n")
    out.append("  r = write_field(bw, t);\n")
    out.append("  if (r != sub8::BitFieldResult::Ok) return r;\n")
    out.append("  switch (static_cast<uint32_t>(v.type)) {\n")
    out.append("    case 0u: return sub8::BitFieldResult::Ok;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: return write_field(bw, v.variant.{o['name']});\n")
    out.append("    default: return sub8::BitFieldResult::Ok;\n")
    out.append("  }\n")
    out.append("}\n\n")

    out.append("template <typename Storage>\n")
    out.append(f"inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, {qname}& out) {{\n")
    out.append("  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;\n")
    out.append(f"  {qualify(ns, tag_field)} t;\n")
    out.append("  r = read_field(br, t);\n")
    out.append("  if (r != sub8::BitFieldResult::Ok) return r;\n")
    out.append("  out.destroy_active();\n")
    out.append(f"  out.type = static_cast<{qualify(ns, tag_enum)}>(static_cast<uint32_t>(t.value()));\n")
    out.append("  switch (static_cast<uint32_t>(out.type)) {\n")
    out.append("    case 0u: out.construct_null(); return sub8::BitFieldResult::Ok;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: new (&out.variant.{o['name']}) {o['cpp']}{{}}; return read_field(br, out.variant.{o['name']});\n")
    out.append("    default: out.construct_null(); return sub8::BitFieldResult::Ok;\n")
    out.append("  }\n")
    out.append("}\n")
    out.append("} // namespace sub8\n")

    return "".join(out)


# ------------------------------------------------------------
# CPP emitters (definitions)
# ------------------------------------------------------------

def emit_object_impl(op: Dict[str, Any]) -> str:
    name = op["name"]
    ns = op.get("namespace")
    fields: List[Dict[str, Any]] = op.get("fields", [])
    qname = qualify(ns, name)

    out: List[str] = []
    out.append(f"sub8::BitSize {qname}::actual_size() const noexcept {{\n")
    out.append("  sub8::BitSize len;\n")
    for f in fields:
        out.append(f"  len += {f['name']}.actual_size();\n")
    out.append("  return len;\n")
    out.append("}\n\n")

    out.append(f"bool {qname}::operator==(const {qname}& o) const noexcept {{\n")
    for f in fields:
        out.append(f"  if ({f['name']} != o.{f['name']}) return false;\n")
    out.append("  return true;\n")
    out.append("}\n")
    return "".join(out)


def emit_variant_impl(op: Dict[str, Any]) -> str:
    name = op["name"]
    ns = op.get("namespace")
    opts: List[Dict[str, Any]] = op.get("options", [])
    qname = qualify(ns, name)

    tag_enum = f"{name}Type"
    tag_field = f"{name}TypeField"

    out: List[str] = []

    out.append(f"sub8::BitSize {qname}::actual_size() const noexcept {{\n")
    out.append(f"  auto size = {tag_field}::ActualSize;\n")
    out.append("  switch (static_cast<uint32_t>(type)) {\n")
    out.append("    case 0u: return sub8::BitSize::from_bits(0) + size;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: size += variant.{o['name']}.actual_size(); break;\n")
    out.append("    default: return sub8::BitSize::from_bits(0) + size;\n")
    out.append("  }\n")
    out.append("  return size;\n")
    out.append("}\n\n")

    out.append(f"void {qname}::construct_null() noexcept {{ new (&variant.null_v) sub8::NullValue{{}}; type = {tag_enum}::NullValue; }}\n\n")

    out.append(f"void {qname}::destroy_active() noexcept {{\n")
    out.append("  switch (static_cast<uint32_t>(type)) {\n")
    out.append("    case 0u: variant.null_v.~NullValue(); break;\n")
    for o in opts:
        # destructor name should be unqualified type; use field type name tail if present, otherwise ok
        # call explicitly on the member's declared type by naming the member's type in the union
        # easiest: call the member's destructor via its type token as in your example
        # We'll emit based on the C++ type tail.
        t_cpp = o["cpp"]
        t_tail = t_cpp.split("::")[-1]
        out.append(f"    case {int(o['id'])}u: variant.{o['name']}.~{t_tail}(); break;\n")
    out.append("    default: break;\n")
    out.append("  }\n")
    out.append("}\n\n")

    out.append(f"{qname}::{name}() noexcept {{ construct_null(); }}\n\n")

    out.append(f"{qname}::{name}(const {qname}& o) {{\n")
    out.append("  type = o.type;\n")
    out.append("  switch (static_cast<uint32_t>(type)) {\n")
    out.append("    case 0u: new (&variant.null_v) sub8::NullValue(o.variant.null_v); break;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: new (&variant.{o['name']}) {o['cpp']}(o.variant.{o['name']}); break;\n")
    out.append("    default: construct_null(); break;\n")
    out.append("  }\n")
    out.append("}\n\n")

    out.append(f"{qname}::{name}({qname}&& o) noexcept {{\n")
    out.append("  type = o.type;\n")
    out.append("  switch (static_cast<uint32_t>(type)) {\n")
    out.append("    case 0u: new (&variant.null_v) sub8::NullValue(std::move(o.variant.null_v)); break;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: new (&variant.{o['name']}) {o['cpp']}(std::move(o.variant.{o['name']})); break;\n")
    out.append("    default: construct_null(); break;\n")
    out.append("  }\n")
    out.append("}\n\n")

    # copy/move assignment
    out.append(f"{qname}& {qname}::operator=(const {qname}& o) {{\n")
    out.append("  if (this == &o) return *this;\n")
    out.append("  destroy_active();\n")
    out.append("  type = o.type;\n")
    out.append("  switch (static_cast<uint32_t>(type)) {\n")
    out.append("    case 0u: new (&variant.null_v) sub8::NullValue(o.variant.null_v); break;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: new (&variant.{o['name']}) {o['cpp']}(o.variant.{o['name']}); break;\n")
    out.append("    default: construct_null(); break;\n")
    out.append("  }\n")
    out.append("  return *this;\n")
    out.append("}\n\n")

    out.append(f"{qname}& {qname}::operator=({qname}&& o) noexcept {{\n")
    out.append("  if (this == &o) return *this;\n")
    out.append("  destroy_active();\n")
    out.append("  type = o.type;\n")
    out.append("  switch (static_cast<uint32_t>(type)) {\n")
    out.append("    case 0u: new (&variant.null_v) sub8::NullValue(std::move(o.variant.null_v)); break;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: new (&variant.{o['name']}) {o['cpp']}(std::move(o.variant.{o['name']})); break;\n")
    out.append("    default: construct_null(); break;\n")
    out.append("  }\n")
    out.append("  return *this;\n")
    out.append("}\n\n")

    out.append(f"{qname}::~{name}() {{ destroy_active(); }}\n\n")

    out.append(f"bool {qname}::is_null() const noexcept {{ return static_cast<uint32_t>(type) == 0u; }}\n\n")

    for o in opts:
        out.append(f"const {o['cpp']}* {qname}::get_{o['name']}() const noexcept {{\n")
        out.append(f"  if (static_cast<uint32_t>(type) == {int(o['id'])}u) return &variant.{o['name']};\n")
        out.append("  return nullptr;\n")
        out.append("}\n\n")

        out.append(f"bool {qname}::is_{o['name']}() const noexcept {{ return static_cast<uint32_t>(type) == {int(o['id'])}u; }}\n\n")

        out.append(f"sub8::BitFieldResult {qname}::set_{o['name']}(const {o['cpp']}& v) noexcept {{\n")
        out.append("  destroy_active();\n")
        out.append(f"  new (&variant.{o['name']}) {o['cpp']}(v);\n")
        out.append(f"  type = static_cast<{tag_enum}>({int(o['id'])}u);\n")
        out.append("  return sub8::BitFieldResult::Ok;\n")
        out.append("}\n\n")

    out.append(f"bool {qname}::operator==(const {qname}& o) const noexcept {{\n")
    out.append("  if (static_cast<uint32_t>(type) != static_cast<uint32_t>(o.type)) return false;\n")
    out.append("  switch (static_cast<uint32_t>(type)) {\n")
    out.append("    case 0u: return true;\n")
    for o in opts:
        out.append(f"    case {int(o['id'])}u: return variant.{o['name']} == o.variant.{o['name']};\n")
    out.append("    default: return true;\n")
    out.append("  }\n")
    out.append("}\n")

    return "".join(out)


# ------------------------------------------------------------
# Plan execution
# ------------------------------------------------------------

def emit_plan(plan: Dict[str, Any], repo_root: str, global_inline_headers: bool, *,
              is_header: bool, header_include_for_cpp: str | None) -> str:
    out: List[str] = []

    if is_header:
        out.append("#pragma once\n\n")
        out.append("// Auto-generated (stage3_emit)\n\n")
        out.append("#include <cstddef>\n")
        out.append("#include <cstdint>\n")
        out.append("#include <utility>\n\n")
    else:
        out.append("// Auto-generated (stage3_emit)\n\n")
        if header_include_for_cpp:
            out.append(f'#include "{header_include_for_cpp}"\n')
        out.append("#include <utility>\n\n")

    ops = plan.get("ops", [])
    if not isinstance(ops, list):
        raise TypeError(f"plan.ops must be a list, got {type(ops).__name__}")

    for idx, raw in enumerate(ops):
        try:
            op = normalize_op(raw)
            t = op["type"]
        except Exception as e:
            raise RuntimeError(
                f"Failed to parse op #{idx} in plan path={plan.get('path')!r}. "
                f"Raw op was: {raw!r}. Error: {e}"
            ) from e

        if t == "include":
            out.append(emit_include(op, repo_root, global_inline_headers))
        elif t == "define":
            if is_header:
                out.append(emit_define_header(op))
        elif t == "emit_alias":
            if is_header:
                out.append(emit_alias(op))
        elif t == "emit_object_decl":
            if is_header:
                out.append(emit_object_decl(op))
        elif t == "emit_object_impl":
            if not is_header:
                out.append(emit_object_impl(op))
        elif t == "emit_variant_decl":
            if is_header:
                out.append(emit_variant_decl(op))
        elif t == "emit_variant_impl":
            if not is_header:
                out.append(emit_variant_impl(op))
        else:
            raise ValueError(f"Unknown op type: {t} (op #{idx} in {plan.get('path')})")

        out.append("\n")

    return "".join(out)


# ------------------------------------------------------------
# CLI
# ------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description="Stage3: emit C++ from codegen_plan.yaml (v2) - no macros")
    ap.add_argument("plan", help="codegen_plan.yaml")
    ap.add_argument("--repo-root", default=".", help="Repo root for inline includes")
    ap.add_argument("-o", "--out", default=".", help="Output base directory")
    args = ap.parse_args()

    with open(args.plan, "r", encoding="utf-8") as f:
        doc = yaml.safe_load(f)

    if doc.get("version") != 2:
        raise SystemExit("Only codegen_plan v2 supported")

    codegen_cfg = doc.get("codegen", {}) or {}
    global_inline_headers = bool(codegen_cfg.get("inline_headers", False))
    header_path = codegen_cfg.get("header")
    source_path = codegen_cfg.get("source")

    # Compute header include for cpp (same dir heuristic if both provided)
    header_include_for_cpp = None
    if header_path and source_path:
        # If both in same folder (e.g. generated/...), include just basename.
        if os.path.dirname(header_path) == os.path.dirname(source_path):
            header_include_for_cpp = os.path.basename(header_path)
        else:
            header_include_for_cpp = header_path

    for plan in doc["plans"]:
        path = plan.get("path", "")
        is_header = path.endswith(".h") or path.endswith(".hpp")
        text = emit_plan(
            plan,
            args.repo_root,
            global_inline_headers,
            is_header=is_header,
            header_include_for_cpp=header_include_for_cpp if not is_header else None,
        )
        out_path = os.path.join(args.out, path)
        ensure_dir(out_path)
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(text)
        print(f"Wrote {out_path}")

    return 0


if __name__ == "__main__":
    sys.exit(main())