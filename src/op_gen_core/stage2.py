from __future__ import annotations

import importlib
from typing import Any, Dict, Iterable, List, Set, Tuple

from .errors import SchemaError
from .registry import TypeRegistry


def _iter_type_refs(node: Any) -> Iterable[str]:
    """Yield typeref strings found in a type node."""
    if isinstance(node, str):
        return []
    if not isinstance(node, dict):
        return []

    k = node.get("kind")
    if k == "object":
        fields = node.get("fields")
        if isinstance(fields, dict):
            return [v for v in fields.values() if isinstance(v, str)]
    if k == "variant":
        opts = node.get("options")
        if isinstance(opts, dict):
            out: List[str] = []
            for ov in opts.values():
                if isinstance(ov, dict) and isinstance(ov.get("type"), str):
                    out.append(str(ov["type"]))
            return out
    # generic typeref field commonly named "type"
    if isinstance(node.get("type"), str):
        return [str(node["type"])]
    return []


def _topo_sort_types(types_map: Dict[str, Any]) -> List[str]:
    deps: Dict[str, Set[str]] = {}
    for tname, tnode in types_map.items():
        ds = set(_iter_type_refs(tnode))
        # only depend on known types (ignore external/primitive)
        ds = {d for d in ds if d in types_map}
        deps[tname] = ds

    perm: Set[str] = set()
    temp: Set[str] = set()
    out: List[str] = []

    def visit(n: str) -> None:
        if n in perm:
            return
        if n in temp:
            raise SchemaError(f"Cyclic type reference involving '{n}'")
        temp.add(n)
        for d in sorted(deps.get(n, set())):
            visit(d)
        temp.remove(n)
        perm.add(n)
        out.append(n)

    for n in sorted(types_map.keys()):
        visit(n)
    return out


def normalize_ops(ops: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """Deduplicate includes and keep define ops before include ops."""

    define_ops: List[Dict[str, Any]] = []
    include_ops: List[Dict[str, Any]] = []
    other_ops: List[Dict[str, Any]] = []

    seen_includes: Set[str] = set()

    for o in ops:
        op = o.get("op")
        if op == "define":
            define_ops.append(o)
        elif op == "include":
            p = o.get("path")
            if isinstance(p, str) and p not in seen_includes:
                seen_includes.add(p)
                include_ops.append(o)
        else:
            other_ops.append(o)

    # stable ordering: define, include, then others.
    return define_ops + include_ops + other_ops


def _split_ops_by_plan(ops: List[Dict[str, Any]]) -> Dict[str, List[Dict[str, Any]]]:
    """Split a flat op list into per-output plans.

    Ops may specify an `into` field with a string plan name (e.g. "header", "source").
    If missing, ops default into "header".

    The returned ops do NOT include the `into` field.
    """

    out: Dict[str, List[Dict[str, Any]]] = {}
    for o in ops:
        into = o.get("into")
        plan = str(into) if isinstance(into, str) and into else "header"
        o2 = dict(o)
        o2.pop("into", None)
        out.setdefault(plan, []).append(o2)
    return out



def _apply_inline_flags(ops: List[Dict[str, Any]], inline_headers: bool) -> None:
    """If inline_headers is enabled, mark include ops as inline for Stage3."""
    if not inline_headers:
        return
    for o in ops:
        if o.get("op") == "include":
            # Avoid clobbering if already specified.
            if "inline" not in o:
                o["inline"] = True
def _plans_from_ops(
    *,
    header_path: str,
    source_path: str,
    ops: List[Dict[str, Any]],
    inline_headers: bool,
) -> List[Dict[str, Any]]:
    by_plan = _split_ops_by_plan(ops)

    # Ensure the canonical plans exist even if empty.
    hdr_ops = normalize_ops(by_plan.get("header", []))
    src_ops = normalize_ops(by_plan.get("source", []))

    _apply_inline_flags(hdr_ops, inline_headers)
    _apply_inline_flags(src_ops, inline_headers)

    # Add any other user-defined plans in stable order.
    extras: List[Tuple[str, List[Dict[str, Any]]]] = []
    for k, v in by_plan.items():
        if k in ("header", "source"):
            continue
        extra_ops = normalize_ops(v)
        _apply_inline_flags(extra_ops, inline_headers)
        extras.append((k, extra_ops))
    extras.sort(key=lambda kv: kv[0])

    plans: List[Dict[str, Any]] = [
        {"name": "header", "path": header_path, "ops": hdr_ops},
        {"name": "source", "path": source_path, "ops": src_ops},
    ]
    for name, ops_n in extras:
        plans.append({"name": name, "path": "", "ops": ops_n})
    return plans


def stage2_process(doc: Dict[str, Any]) -> Dict[str, Any]:
    if not isinstance(doc, dict):
        raise SchemaError("root must be a mapping")

    codegen_root = doc.get("codegen") or {}
    if not isinstance(codegen_root, dict):
        raise SchemaError("codegen must be a mapping")

    # current schema: codegen.cpp.header/source required
    cpp = codegen_root.get("cpp") or {}
    if not isinstance(cpp, dict):
        raise SchemaError("codegen.cpp must be a mapping")
    header = cpp.get("header")
    source = cpp.get("source")
    if not header or not source:
        raise SchemaError("codegen.cpp.header and codegen.cpp.source are required")

    sub8_file_path = str(codegen_root.get("sub8_file_path") or "")
    default_namespace = str(codegen_root.get("default_namespace") or "")

    # codegen.cpp.inline_headers defaults to true
    inline_headers = cpp.get("inline_headers", True)
    if not isinstance(inline_headers, bool):
        raise SchemaError("codegen.cpp.inline_headers must be a bool")

    # Load language generator registry.
    lang_mod = importlib.import_module("languages.cpp.op_gens")
    reg_fact = getattr(lang_mod, "get_generator_registry", None)
    if not callable(reg_fact):
        raise SchemaError("languages.cpp.op_gens.get_generator_registry missing")
    gen_registry: Dict[str, Any] = reg_fact()
    if not isinstance(gen_registry, dict):
        raise SchemaError("generator registry must be a dict")

    ops: List[Dict[str, Any]] = []
    type_reg = TypeRegistry()

    # 1) Emit config ops
    cfg_kinds = doc.get("codegen_config_kinds") or {}
    cfg_values = doc.get("code_config") or {}
    if not isinstance(cfg_kinds, dict):
        raise SchemaError("codegen_config_kinds must be a mapping")
    if not isinstance(cfg_values, dict):
        raise SchemaError("code_config must be a mapping")

    for cfg_name, cfg_def in cfg_kinds.items():
        if not isinstance(cfg_def, dict):
            continue
        lang_node = (cfg_def.get("cpp") or {})
        if not isinstance(lang_node, dict):
            continue
        gen_type = lang_node.get("type")
        if not gen_type:
            continue
        gen_cls = gen_registry.get(str(gen_type))
        if gen_cls is None:
            raise SchemaError(f"Unknown cpp generator '{gen_type}' for config '{cfg_name}'")
        gen = gen_cls()
        cfg_value = cfg_values.get(cfg_name, cfg_def.get("schema", {}).get("default_value"))
        gen.emit_config_ops(
            doc=doc,
            cfg_name=cfg_name,
            cfg_def=cfg_def,
            cfg_value=cfg_value,
            codegen_root=codegen_root,
            lang_node=lang_node,
            reg=type_reg,
            ops=ops,
            sub8_file_path=sub8_file_path,
            default_namespace=default_namespace,
        )

    # 2) Emit type ops in topo order
    types_map = doc.get("types") or {}
    if not isinstance(types_map, dict):
        raise SchemaError("types must be a mapping")

    order = _topo_sort_types(types_map)
    kinds = doc.get("kinds") or {}
    if not isinstance(kinds, dict):
        raise SchemaError("kinds must be a mapping")

    for type_name in order:
        type_node = types_map[type_name]
        if not isinstance(type_node, dict):
            continue
        kind_name = type_node.get("kind")
        if not kind_name:
            continue
        kind_def = kinds.get(str(kind_name))
        if not isinstance(kind_def, dict):
            raise SchemaError(f"Unknown kind '{kind_name}' for type '{type_name}'")

        # current schema: kind.code_gen.cpp contains generator node.
        # (tiny legacy fallback to kind.cpp to avoid hard breaks)
        lang_node: Any = None
        if isinstance(kind_def.get("code_gen"), dict):
            lang_node = (kind_def.get("code_gen") or {}).get("cpp")
        if not isinstance(lang_node, dict):
            legacy = kind_def.get("cpp")
            if isinstance(legacy, dict):
                lang_node = legacy
        if not isinstance(lang_node, dict):
            raise SchemaError(f"kind '{kind_name}' missing code_gen.cpp")

        gen_type = lang_node.get("type")
        if not gen_type:
            raise SchemaError(f"kind '{kind_name}' missing code_gen.cpp.type")
        gen_cls = gen_registry.get(str(gen_type))
        if gen_cls is None:
            raise SchemaError(f"Unknown cpp generator '{gen_type}' for kind '{kind_name}'")
        gen = gen_cls()
        gen.emit_type_ops(
            doc=doc,
            type_name=type_name,
            type_node=type_node,
            kind_def=kind_def,
            codegen_root=codegen_root,
            lang_node=lang_node,
            reg=type_reg,
            ops=ops,
            sub8_file_path=sub8_file_path,
            default_namespace=default_namespace,
        )

    plans = _plans_from_ops(header_path=str(header), source_path=str(source), ops=ops, inline_headers=inline_headers)
    return {
        "version": 2,
        "codegen": {
            "header": header,
            "source": source,
            "sub8_file_path": sub8_file_path,
            "default_namespace": default_namespace,
            "inline_headers": inline_headers,
        },
        "plans": plans,
    }
