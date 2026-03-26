#!/usr/bin/env python3
from __future__ import annotations

import argparse
import copy
import json
import os
import re
import sys
from dataclasses import dataclass
from typing import Any, Dict, List, Set, Tuple

try:
    import yaml  # pip install pyyaml
except ImportError:
    print("Missing dependency: pyyaml. Install with: pip install pyyaml", file=sys.stderr)
    raise


# -----------------------------
# Deep merge
# -----------------------------

def deep_merge(a: Any, b: Any) -> Any:
    if isinstance(a, dict) and isinstance(b, dict):
        out = dict(a)
        for k, bv in b.items():
            if k in out:
                out[k] = deep_merge(out[k], bv)
            else:
                out[k] = copy.deepcopy(bv)
        return out
    if isinstance(a, list) and isinstance(b, list):
        # policy: concatenate
        return list(a) + list(b)
    return copy.deepcopy(b)


# -----------------------------
# Imports
# -----------------------------

def load_yaml_file(path: str) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f) or {}
    if not isinstance(data, dict):
        raise ValueError(f"Top-level YAML must be a map: {path}")
    return data


def _normalize_import_entry(entry: Any) -> str:
    if isinstance(entry, str):
        return entry
    if isinstance(entry, dict) and "file" in entry:
        return str(entry["file"])
    raise ValueError(f"Unsupported import entry: {entry}")


def _default_import_search_paths() -> List[str]:
    """Return directories to search for imported YAML files.

    Order matters: earlier entries win.
    """
    paths: List[str] = []

    # 1) Explicit env override (colon-separated, like PATH)
    env = os.environ.get("SUB8_YAML_PATH") or os.environ.get("SUB8_STDLIB_YAML_PATH") or ""
    if env.strip():
        for p in env.split(os.pathsep):
            p = p.strip()
            if p:
                paths.append(os.path.abspath(p))

    # 2) Source-tree stdlib folder (works in-repo)
    here = os.path.dirname(os.path.abspath(__file__))
    paths.append(os.path.join(here, "std_yaml"))

    # 3) Packaged resources (works when installed as a wheel)
    try:
        import importlib.resources as _ilr  # py3.9+
        # If this repo becomes a package, store standard YAML under: <package>/stdlib_yaml/
        # We try a couple of likely package names without failing hard.
        for pkg in ("sub8", "sub8_codegen", "op_gen_core"):
            try:
                root = _ilr.files(pkg)  # type: ignore[attr-defined]
                p = root / "stdlib_yaml"
                paths.append(str(p))
            except Exception:
                continue
    except Exception:
        pass

    # De-dupe while preserving order
    seen: Set[str] = set()
    out: List[str] = []
    for p in paths:
        ap = os.path.abspath(p)
        if ap not in seen:
            seen.add(ap)
            out.append(ap)
    return out


def _resolve_import_path(rel_or_abs: str, base_dir: str, search_paths: List[str]) -> str:
    """Resolve an import entry to an absolute path.

    - Absolute paths are used directly (must exist).
    - Relative paths are tried relative to the importing file's directory first,
      then across the provided search_paths.
    """
    cand = rel_or_abs
    if os.path.isabs(cand):
        if os.path.exists(cand):
            return cand
        raise FileNotFoundError(cand)

    tried: List[str] = []

    # 1) Relative to the importing YAML file
    p0 = os.path.abspath(os.path.join(base_dir, cand))
    tried.append(p0)
    if os.path.exists(p0):
        return p0

    # 2) Relative to any configured stdlib/import search dirs
    for sp in search_paths:
        p = os.path.abspath(os.path.join(sp, cand))
        tried.append(p)
        if os.path.exists(p):
            return p

    raise FileNotFoundError(
        f"Could not resolve import '{rel_or_abs}'. Tried: " + ", ".join(tried)
    )


def load_with_imports(root_path: str, *, import_search_paths: List[str] | None = None) -> Dict[str, Any]:
    root_abs = os.path.abspath(root_path)
    root_dir = os.path.dirname(root_abs)
    root = load_yaml_file(root_abs)

    merged: Dict[str, Any] = {}
    imports = root.get("import", []) or []
    if not isinstance(imports, list):
        raise ValueError("'import' must be a list")

    search_paths = import_search_paths if import_search_paths is not None else _default_import_search_paths()

    for imp in imports:
        rel = _normalize_import_entry(imp)
        imp_abs = _resolve_import_path(rel, base_dir=root_dir, search_paths=search_paths)
        imp_data = load_with_imports(imp_abs, import_search_paths=search_paths)
        merged = deep_merge(merged, imp_data)

    merged = deep_merge(merged, root)
    return merged

# -----------------------------
# Substitutions
# -----------------------------

_SUBST_RE = re.compile(r"\$\{([A-Za-z0-9_.-]+)\}")

def apply_substitutions(node: Any, subs: Dict[str, Any]) -> Any:
    if isinstance(node, dict):
        return {k: apply_substitutions(v, subs) for k, v in node.items()}
    if isinstance(node, list):
        return [apply_substitutions(v, subs) for v in node]
    if isinstance(node, str):
        whole = _SUBST_RE.fullmatch(node.strip())
        if whole:
            key = whole.group(1)
            if key in subs:
                return copy.deepcopy(subs[key])
            return node

        def repl(m: re.Match) -> str:
            key = m.group(1)
            if key not in subs:
                return m.group(0)
            return str(subs[key])

        return _SUBST_RE.sub(repl, node)
    return node

def flatten_with_prefix(prefix: str, obj: Any) -> Dict[str, Any]:
    """
    Flattens nested dicts into dotted keys:
      {"a": {"b": 1}} with prefix "code_config" => {"code_config.a.b": 1}
    Non-dicts produce no keys.
    """
    out: Dict[str, Any] = {}
    if not isinstance(obj, dict):
        return out

    def walk(cur: Dict[str, Any], path: str) -> None:
        for k, v in cur.items():
            key = f"{path}.{k}" if path else str(k)
            if isinstance(v, dict):
                walk(v, key)
            else:
                out[key] = v

    walk(obj, prefix)
    return out

# -----------------------------
# Normalization helpers
# -----------------------------

@dataclass
class InlineLiftResult:
    doc: Dict[str, Any]
    lifted_names: List[str]

def lift_inline_types(doc: Dict[str, Any]) -> InlineLiftResult:
    types = doc.setdefault("types", {})
    if not isinstance(types, dict):
        raise ValueError("'types' must be a map")

    counter = 0
    lifted: List[str] = []

    def next_name(prefix: str) -> str:
        nonlocal counter
        counter += 1
        return f"{prefix}{counter}"

    # Derive kind tokens from YAML "kinds:" section
    kinds_map = doc.get("kinds") or {}
    kind_names_from_yaml: Set[str] = set()
    if isinstance(kinds_map, dict):
        # Keys are the canonical kind tokens (e.g., "object", "array", "b5_string", ...)
        kind_names_from_yaml = {str(k) for k in kinds_map.keys()}


    KIND_NAMES = kind_names_from_yaml

    def normalize_typeref(x: Any, suggested_prefix: str) -> str:
        if isinstance(x, str):
            return x

        if not isinstance(x, dict):
            raise ValueError(f"Unsupported type_ref: {x}")

        # Inline named type def: {name: Foo, type|kind: object|array|..., ...}
        if "name" in x and ("kind" in x or "type" in x):
            name = str(x["name"])
            payload = dict(x)
            payload.pop("name", None)

            # Decide kind source without clobbering array.element 'type'
            if "kind" in payload:
                kind = payload["kind"]
            else:
                # payload["type"] could be either a kind token OR an element typeref
                t = payload.get("type")
                if isinstance(t, str) and t in KIND_NAMES:
                    kind = t
                    payload.pop("type", None)  # only pop if it was actually the kind token
                else:
                    raise ValueError(f"Inline named type '{name}' missing kind (got type={t!r})")

            payload["kind"] = kind

            if name not in types:
                types[name] = payload
                lifted.append(name)
            return name

        # Inline anonymous type def: {kind: array, type: MessageItem, ...} or {type: object, fields: ...}
        payload = dict(x)

        # Determine kind safely
        if "kind" in payload:
            kind = payload["kind"]
        else:
            t = payload.get("type")
            if isinstance(t, str) and t in KIND_NAMES:
                kind = t
                payload.pop("type", None)  # pop only when it was being used as kind token
            else:
                raise ValueError(f"Inline type_ref missing kind/type token: {x}")

        payload["kind"] = kind

        name = next_name(suggested_prefix)
        types[name] = payload
        lifted.append(name)
        return name

    # keep normalizing until no new types are added
    while True:
        before = len(types)

        # iterate over snapshot; loop repeats if we add new types
        for tname, tdef in list(types.items()):
            if not isinstance(tdef, dict):
                continue

            kind = tdef.get("kind") or tdef.get("type")

            if kind == "object":
                fields = tdef.get("fields") or {}
                if isinstance(fields, dict):
                    new_fields = {}
                    for fname, fref in fields.items():
                        new_fields[fname] = normalize_typeref(fref, f"{tname}_{fname}_T")
                    tdef["fields"] = new_fields

            elif kind == "variant":
                options = tdef.get("options") or {}
                if isinstance(options, dict):
                    new_opts = {}
                    for oname, odef in options.items():
                        if not isinstance(odef, dict):
                            raise ValueError(f"variant option '{tname}.{oname}' must be a map")
                        odef2 = dict(odef)
                        odef2["type"] = normalize_typeref(odef2.get("type"), f"{tname}_{oname}_T")
                        new_opts[oname] = odef2
                    tdef["options"] = new_opts

            elif kind in ("array", "optional"):
                if "type" in tdef:
                    tdef["type"] = normalize_typeref(tdef["type"], f"{tname}_type_T")

            # Generic: many kinds carry a nested "type" typeref (array/optional/etc).
            # After stage1, ANY such "type" must be a string type name.
            if "type" in tdef and kind not in ("variant",):  # variant handled separately per-option
                tdef["type"] = normalize_typeref(tdef["type"], f"{tname}_type_T")

        if len(types) == before:
            break

    return InlineLiftResult(doc=doc, lifted_names=lifted)

# -----------------------------
# Dependency graph + pruning
# -----------------------------

def build_type_dependency_graph(types: Dict[str, Any]) -> Dict[str, Set[str]]:
    """
    Edge A -> B if A references B.
    Only tracks references by string names.
    """
    graph: Dict[str, Set[str]] = {name: set() for name in types.keys()}

    def add_edge(a: str, b: Any):
        if isinstance(b, str) and b in types:
            graph[a].add(b)

    for name, tdef in types.items():
        if not isinstance(tdef, dict):
            continue
        kind = tdef.get("kind") or tdef.get("type")

        if kind == "object":
            fields = tdef.get("fields") or {}
            if isinstance(fields, dict):
                for _, tref in fields.items():
                    add_edge(name, tref)

        elif kind == "variant":
            opts = tdef.get("options") or {}
            if isinstance(opts, dict):
                for _, odef in opts.items():
                    if isinstance(odef, dict):
                        add_edge(name, odef.get("type"))

        elif kind == "array":
            add_edge(name, tdef.get("type"))

        elif kind == "optional":
            add_edge(name, tdef.get("type"))

        # buffer/integer/float/etc: no named type refs

    return graph


def compute_reachable(graph: Dict[str, Set[str]], roots: Set[str]) -> Set[str]:
    seen: Set[str] = set()
    stack = list(roots)
    while stack:
        n = stack.pop()
        if n in seen:
            continue
        seen.add(n)
        for m in graph.get(n, set()):
            if m not in seen:
                stack.append(m)
    return seen


def choose_roots(types: Dict[str, Any]) -> Set[str]:
    """
    Root policy for pruning:
      - If emit_only_if_referenced is False OR missing => root
      - Otherwise not a root (likely core)
    This matches your intent: "emit_only_if_referenced types can be omitted"
    """
    roots: Set[str] = set()
    for name, tdef in types.items():
        if not isinstance(tdef, dict):
            continue
        e = tdef.get("emit_only_if_referenced")
        if e is False or e is None:
            roots.add(name)
    # fallback: if none, keep everything
    if not roots:
        roots = set(types.keys())
    return roots


def prune_emit_only_if_unreferenced(doc: Dict[str, Any]) -> Dict[str, Any]:
    types = doc.get("types") or {}
    if not isinstance(types, dict):
        return doc

    graph = build_type_dependency_graph(types)
    roots = choose_roots(types)
    reachable = compute_reachable(graph, roots)

    # Always keep roots, and anything reachable. Drop unreachable.
    new_types = {k: v for k, v in types.items() if k in reachable}
    doc["types"] = new_types
    # store graph metadata for debugging downstream if desired
    doc.setdefault("_stage1", {})
    doc["_stage1"]["roots"] = sorted(list(roots))
    doc["_stage1"]["reachable"] = sorted(list(reachable))
    return doc


# -----------------------------
# Public API (in-memory)
# -----------------------------

def stage1_process(root_yaml_path: str, *, prune_emit_only: bool = True) -> Dict[str, Any]:
    merged = load_with_imports(root_yaml_path)

    # substitutions: substitute + code_config values are injectable
    subs: Dict[str, Any] = {}
    subs.update(merged.get("substitute", {}) or {})
    merged = apply_substitutions(merged, subs)

    # 2) then expand code_config.* paths (leave unchanged if missing)
    code_cfg = merged.get("code_config", {}) or {}
    cfg_subs = flatten_with_prefix("code_config", code_cfg)
    merged = apply_substitutions(merged, cfg_subs)

    # normalize: lift inline types into named doc['types']
    merged = lift_inline_types(merged).doc

    # prune
    if prune_emit_only:
        merged = prune_emit_only_if_unreferenced(merged)

    # remove import/substitute from final normalized doc (optional)
    merged.pop("import", None)
    merged.pop("substitute", None)

    return merged


# -----------------------------
# CLI
# -----------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description="Stage1: merge+import+substitute+normalize+prune into single YAML")
    ap.add_argument("root", help="Root YAML file (e.g. example.yaml)")
    ap.add_argument("-o", "--out", default="", help="Output YAML path (default: stdout)")
    ap.add_argument("--no-prune", action="store_true", help="Disable emit_only_if_referenced pruning")
    ap.add_argument("--json", action="store_true", help="Output JSON instead of YAML")
    args = ap.parse_args()

    doc = stage1_process(args.root, prune_emit_only=not args.no_prune)

    if args.json:
        text = json.dumps(doc, indent=2, sort_keys=True) + "\n"
    else:
        text = yaml.safe_dump(doc, sort_keys=False)

    if args.out:
        os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
        with open(args.out, "w", encoding="utf-8") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
