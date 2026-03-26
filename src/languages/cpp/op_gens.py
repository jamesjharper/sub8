# languages/cpp/op_gens.py
from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple

import op_gen


# -----------------------------------------------------------------------------
# Registry (reflection entrypoint)
# -----------------------------------------------------------------------------

def get_generator_registry() -> Dict[str, type]:
    return {
        "cpp_define": CppDefineGen,
        "cpp_struct": CppStructGen,
        "cpp_enum": CppEnumGen,
        "cpp_object": CppObjectGen,
        "cpp_variant": CppVariantGen,
    }


# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------

def _ensure_dict(x: Any, msg: str) -> Dict[str, Any]:
    if not isinstance(x, dict):
        raise op_gen.SchemaError(msg)
    return x


def _ensure_list(x: Any) -> List[Any]:
    if x is None:
        return []
    if isinstance(x, list):
        return x
    return [x]


def _qualify(ns: str, name: str) -> str:
    ns = (ns or "").strip()
    if not ns:
        return name
    return f"{ns}::{name}"


def _render(value: Any, *, scopes: List[Dict[str, Any]], adapters: Optional[Dict[str, Any]] = None) -> Any:
    return op_gen.render_template(value, scopes=scopes, adapters=adapters or {})


def _compute_required(field_def: Dict[str, Any], *, scopes: List[Dict[str, Any]]) -> bool:
    req = field_def.get("required", False)
    if isinstance(req, bool):
        return req
    if isinstance(req, str):
        # allow ${...} or raw expression-like strings
        rendered = _render(req, scopes=scopes)
        if isinstance(rendered, str):
            t = rendered.strip().lower()
            if t in ("true", "1", "yes"):
                return True
            if t in ("false", "0", "no", ""):
                return False
        return bool(rendered)
    return bool(req)


def _enum_kind_to_cpp_type(doc: Dict[str, Any], enum_kind_name: str) -> str:
    kd = _ensure_dict((doc.get("kinds") or {}).get(enum_kind_name) or {}, f"Unknown enum kind '{enum_kind_name}'")
    cg = _ensure_dict((kd.get("code_gen") or {}).get("cpp") or {}, f"enum kind '{enum_kind_name}' missing code_gen.cpp")
    enum_name = cg.get("def")
    ns = cg.get("namespace", "")
    if not enum_name:
        raise op_gen.SchemaError(f"enum kind '{enum_kind_name}' missing code_gen.cpp.def")
    return _qualify(ns, str(enum_name))


def _enum_value_to_cpp(doc: Dict[str, Any], enum_kind_name: str, value: str) -> str:
    kd = _ensure_dict((doc.get("kinds") or {}).get(enum_kind_name) or {}, f"Unknown enum kind '{enum_kind_name}'")
    cg = _ensure_dict((kd.get("code_gen") or {}).get("cpp") or {}, f"enum kind '{enum_kind_name}' missing code_gen.cpp")
    enum_name = cg.get("def")
    ns = cg.get("namespace", "")
    mapping = cg.get("values") or {}
    if not enum_name or not isinstance(mapping, dict):
        raise op_gen.SchemaError(f"enum kind '{enum_kind_name}' missing code_gen.cpp.def/values")
    if value not in mapping:
        raise op_gen.SchemaError(f"enum kind '{enum_kind_name}' unknown value '{value}'")
    return _qualify(ns, f"{enum_name}::{mapping[value]}")


def _coerce_schema_value(doc: Dict[str, Any], field_type: str, raw: Any) -> Any:
    # primitives
    if field_type in ("u32", "u16", "u8", "i32", "i16", "i8"):
        if not isinstance(raw, int):
            raise op_gen.SchemaError(f"Expected int for schema type '{field_type}', got {type(raw)}")
        return raw
    if field_type == "bool":
        if not isinstance(raw, bool):
            raise op_gen.SchemaError(f"Expected bool for schema type 'bool', got {type(raw)}")
        return raw
    if field_type == "string":
        if not isinstance(raw, str):
            raise op_gen.SchemaError(f"Expected str for schema type 'string', got {type(raw)}")
        return raw
    if field_type.endswith("[]"):
        if not isinstance(raw, list):
            raise op_gen.SchemaError(f"Expected list for schema type '{field_type}', got {type(raw)}")
        return raw

    # kind::SomeEnumKind  (your YAML uses this)
    if isinstance(field_type, str) and field_type.startswith("kind::"):
        enum_kind = field_type.split("::", 1)[1]
        if not isinstance(raw, str):
            raise op_gen.SchemaError(f"Expected enum string for '{field_type}'")
        return _enum_value_to_cpp(doc, enum_kind, raw)

    # direct enum kind name as a schema type (also supported)
    kd = (doc.get("kinds") or {}).get(field_type)
    if isinstance(kd, dict):
        sch = kd.get("schema")
        if isinstance(sch, dict) and sch.get("type") == "enum":
            if not isinstance(raw, str):
                raise op_gen.SchemaError(f"Expected enum string for enum kind '{field_type}'")
            return _enum_value_to_cpp(doc, field_type, raw)

    # type_ref is left as string here (resolution happens via adapters when rendering def)
    if field_type == "type_ref":
        if not isinstance(raw, str):
            raise op_gen.SchemaError("type_ref must be a string")
        return raw

    # unknown schema type: pass-through
    return raw


def _build_schema_values(
    doc: Dict[str, Any],
    kind_def: Dict[str, Any],
    type_node: Dict[str, Any],
    *,
    scopes: List[Dict[str, Any]],
) -> Dict[str, Any]:
    """
    Fill kind schema fields from the type_node, applying defaults and required checks.
    Supports schema:
      - list form: [{field:{...}}, ...]
      - dict enum form: {type: enum, values:[...]}  (no fields to fill)
    """
    schema = kind_def.get("schema")

    # enum schema dict => no parameter fields
    if isinstance(schema, dict) and schema.get("type") == "enum":
        return {}

    out: Dict[str, Any] = {}
    if schema is None:
        return out
    if not isinstance(schema, list):
        raise op_gen.SchemaError("kind.schema must be list (or enum dict)")

    for entry in schema:
        if not isinstance(entry, dict) or "field" not in entry:
            continue
        f = entry["field"]
        if not isinstance(f, dict):
            continue
        name = f.get("name")
        ftype = f.get("type")
        if not name or not ftype:
            continue

        required = _compute_required(f, scopes=scopes)

        if name in type_node:
            raw = type_node[name]
        else:
            raw = f.get("default_value", None)
            if raw is None and required:
                raise op_gen.SchemaError(f"Missing required field '{name}' for kind '{type_node.get('kind')}'")

        # render templates in the raw (defaults can be templated)
        raw = _render(raw, scopes=scopes)
        out[name] = _coerce_schema_value(doc, str(ftype), raw)

    return out


def _make_adapters_for_schema(reg: op_gen.TypeRegistry, schema_values: Dict[str, Any]) -> Dict[str, Any]:
    """
    Lets templates like ${schema.type} inline a previously-registered type_ref's rendered form.
    """
    adapters: Dict[str, Any] = {}

    def _resolver(field_name: str):
        def _fn():
            v = schema_values.get(field_name)
            if isinstance(v, str):
                # try resolve as a type_ref (registered earlier in topo order)
                try:
                    return reg.get(v).rendered
                except Exception:
                    return v
            return v
        return _fn

    for k, v in schema_values.items():
        if isinstance(v, str):
            adapters[f"schema.{k}"] = _resolver(k)

    return adapters


def _includes_from_lang_node(lang_node: Dict[str, Any]) -> List[str]:
    incs = lang_node.get("includes")
    if incs is None:
        incs = lang_node.get("include")
    return [str(x) for x in _ensure_list(incs)]


def _sources_from_lang_node(lang_node: Dict[str, Any]) -> List[str]:
    """Additional translation units / source files to be included in the *source* output plan."""
    srcs = lang_node.get("source")
    if srcs is None:
        srcs = lang_node.get("sources")
    return [str(x) for x in _ensure_list(srcs)]


def _emit_includes(
    ops: List[Dict[str, Any]],
    *,
    sub8_file_path: str,
    includes: List[str],
    into: str,
) -> None:
    for inc in includes:
        ops.append({"op": "include", "path": _join_inc(sub8_file_path, inc), "into": into})


# -----------------------------------------------------------------------------
# Generators
# -----------------------------------------------------------------------------

class CppDefineGen(op_gen.BaseOpGenerator):
    def emit_config_ops(
        self,
        *,
        doc: Dict[str, Any],
        cfg_name: str,
        cfg_def: Dict[str, Any],
        cfg_value: Any,
        codegen_root: Dict[str, Any],
        lang_node: Dict[str, Any],
        reg: op_gen.TypeRegistry,
        ops: List[Dict[str, Any]],
        sub8_file_path: str,
        default_namespace: str,
    ) -> None:
        define_name = lang_node.get("define")
        if not define_name:
            raise op_gen.SchemaError(f"codegen_config_kinds.{cfg_name}.cpp.define missing")
        ops.append({"op": "define", "name": str(define_name), "value": 1 if bool(cfg_value) else 0})

    def emit_type_ops(self, **kwargs) -> None:  # not used for types
        raise op_gen.SchemaError("cpp_define is a config generator, not a type generator")


class CppStructGen(op_gen.BaseOpGenerator):
    """
    For scalar-ish kinds that provide:
      kind.code_gen.cpp:
        type: cpp_struct
        include(s): ...
        def: "... ${schema.*} ..."
    Emits ops:
      - include
      - alias (emit_alias)
    Registers:
      reg.add(type_name, rendered=<cpp type expr>)
    """
    def emit_config_ops(self, **kwargs) -> None:
        pass

    def emit_type_ops(
        self,
        *,
        doc: Dict[str, Any],
        type_name: str,
        type_node: Dict[str, Any],
        kind_def: Dict[str, Any],
        codegen_root: Dict[str, Any],
        lang_node: Dict[str, Any],
        reg: op_gen.TypeRegistry,
        ops: List[Dict[str, Any]],
        sub8_file_path: str,
        default_namespace: str,
    ) -> None:
        def_tmpl = lang_node.get("def")
        if not def_tmpl:
            raise op_gen.SchemaError(f"kind '{type_node.get('kind')}' missing code_gen.cpp.def")

        # base scopes for resolving defaults and def
        scopes = [
            {"this": type_node},
            {"codegen": codegen_root},
            {"code_config": doc.get("code_config") or {}},
        ]

        schema_vals = _build_schema_values(doc, kind_def, type_node, scopes=scopes)
        adapters = _make_adapters_for_schema(reg, schema_vals)
        scopes2 = [{"schema": schema_vals}, *scopes]

        cpp_expr = _render(def_tmpl, scopes=scopes2, adapters=adapters)
        if isinstance(cpp_expr, str):
            cpp_expr = cpp_expr.strip()

        # header includes
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_includes_from_lang_node(lang_node),
            into="header",
        )

        # source includes (new)
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_sources_from_lang_node(lang_node),
            into="source",
        )

        reg.add(type_name, kind=str(type_node.get("kind", "")), rendered=str(cpp_expr))
        ops.append({"op": "emit_alias", "name": type_name, "cpp": str(cpp_expr)})


class CppEnumGen(op_gen.BaseOpGenerator):
    """
    Two roles:
      A) enum kinds used only as parameters (kind::X) => handled elsewhere, no ops needed.
      B) a reachable "type" whose kind schema is enum dict => we emit an enum definition op.

    Assumes the enum kind has:
      kind.schema: {type: enum, values:[...]}
      kind.code_gen.cpp: { def: "EnumName", namespace: "...", values: {yaml_value: "CppEnumerator"} }

    For a concrete type named T where kind is that enum kind:
      - register rendered as <namespace>::<def>
      - emit op: emit_enum_def (dedup not handled here; keep reachable list sane)
    """
    def emit_config_ops(self, **kwargs) -> None:
        pass

    def emit_type_ops(
        self,
        *,
        doc: Dict[str, Any],
        type_name: str,
        type_node: Dict[str, Any],
        kind_def: Dict[str, Any],
        codegen_root: Dict[str, Any],
        lang_node: Dict[str, Any],
        reg: op_gen.TypeRegistry,
        ops: List[Dict[str, Any]],
        sub8_file_path: str,
        default_namespace: str,
    ) -> None:
        schema = kind_def.get("schema")
        if not (isinstance(schema, dict) and schema.get("type") == "enum"):
            # If someone wired cpp_enum onto a non-enum kind, fail loudly.
            raise op_gen.SchemaError(f"cpp_enum used for non-enum kind '{type_node.get('kind')}'")

        # header includes
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_includes_from_lang_node(lang_node),
            into="header",
        )

        # source includes (optional)
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_sources_from_lang_node(lang_node),
            into="source",
        )

        # C++ enum type name is from kind's code_gen.cpp.def, NOT from the concrete type_name
        enum_cpp_type = _enum_kind_to_cpp_type(doc, str(type_node["kind"]))

        # Register: concrete type name resolves to enum type
        reg.add(type_name, kind=str(type_node["kind"]), rendered=enum_cpp_type)

        # Emit enum definition op (for stage3)
        kd = (doc.get("kinds") or {}).get(type_node["kind"]) or {}
        cg = (kd.get("code_gen") or {}).get("cpp") or {}
        ops.append({
            "op": "emit_enum_def",
            "enum_cpp": enum_cpp_type,
            "underlying": str(cg.get("underlying", "uint32_t")),
            "mapping": dict(cg.get("values") or {}),
        })


class CppObjectGen(op_gen.BaseOpGenerator):
    """
    object kinds:
      kind: object
      type_node: { namespace?, fields:{name: type_ref, ...} }

    Emits:
      - include ops (from kind.cpp includes)
      - emit_object op containing resolved field cpp types
    Registers:
      reg.add(type_name, rendered=<qualified name>)
    """
    def emit_config_ops(self, **kwargs) -> None:
        pass

    def emit_type_ops(
        self,
        *,
        doc: Dict[str, Any],
        type_name: str,
        type_node: Dict[str, Any],
        kind_def: Dict[str, Any],
        codegen_root: Dict[str, Any],
        lang_node: Dict[str, Any],
        reg: op_gen.TypeRegistry,
        ops: List[Dict[str, Any]],
        sub8_file_path: str,
        default_namespace: str,
    ) -> None:
        ns = str(type_node.get("namespace") or default_namespace or "")
        cpp_name = _qualify(ns, type_name)

        # header includes
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_includes_from_lang_node(lang_node),
            into="header",
        )

        # source includes (optional)
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_sources_from_lang_node(lang_node),
            into="source",
        )

        # register early so other templates can refer to it (if needed)
        reg.add(type_name, kind="object", rendered=cpp_name)

        fields = _ensure_dict(type_node.get("fields") or {}, f"type '{type_name}'.fields must be a map")
        out_fields: List[Dict[str, Any]] = []
        for fname, tref in fields.items():
            if not isinstance(tref, str):
                raise op_gen.SchemaError(f"type '{type_name}'.fields.{fname} must be a type_ref string")
            cpp_t = reg.get(tref).rendered  # topo order ensures already registered
            out_fields.append({"name": str(fname), "type": tref, "cpp": cpp_t})

        ops.append({
            "op": "emit_object_decl",
            "name": type_name,
            "namespace": ns,
            "cpp": cpp_name,
            "fields": out_fields,
            "into": "header",
        })

        ops.append({
            "op": "emit_object_impl",
            "name": type_name,
            "namespace": ns,
            "cpp": cpp_name,
            "fields": out_fields,
            "into": "source",
        })


class CppVariantGen(op_gen.BaseOpGenerator):
    """
    variant kinds:
      kind: variant
      type_node: { namespace?, options:{Name:{id:int, type:type_ref}, ...} }

    Emits:
      - include ops (from kind.cpp includes)
      - emit_variant op
    Registers:
      reg.add(type_name, rendered=<qualified name>)
    """
    def emit_config_ops(self, **kwargs) -> None:
        pass

    def emit_type_ops(
        self,
        *,
        doc: Dict[str, Any],
        type_name: str,
        type_node: Dict[str, Any],
        kind_def: Dict[str, Any],
        codegen_root: Dict[str, Any],
        lang_node: Dict[str, Any],
        reg: op_gen.TypeRegistry,
        ops: List[Dict[str, Any]],
        sub8_file_path: str,
        default_namespace: str,
    ) -> None:
        ns = str(type_node.get("namespace") or default_namespace or "")
        cpp_name = _qualify(ns, type_name)

        # header includes
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_includes_from_lang_node(lang_node),
            into="header",
        )

        # source includes (optional)
        _emit_includes(
            ops,
            sub8_file_path=sub8_file_path,
            includes=_sources_from_lang_node(lang_node),
            into="source",
        )

        reg.add(type_name, kind="variant", rendered=cpp_name)

        options = _ensure_dict(type_node.get("options") or {}, f"type '{type_name}'.options must be a map")
        out_opts: List[Dict[str, Any]] = []
        for oname, ov in options.items():
            if not isinstance(ov, dict):
                raise op_gen.SchemaError(f"type '{type_name}'.options.{oname} must be a map")
            oid = ov.get("id")
            otype = ov.get("type")
            if not isinstance(oid, int) or not isinstance(otype, str):
                raise op_gen.SchemaError(f"type '{type_name}'.options.{oname} must have int id and string type")
            cpp_t = reg.get(otype).rendered
            out_opts.append({"name": str(oname), "id": int(oid), "type": otype, "cpp": cpp_t})

        ops.append({
            "op": "emit_variant_decl",
            "name": type_name,
            "namespace": ns,
            "cpp": cpp_name,
            "options": out_opts,
            "into": "header",
        })

        ops.append({
            "op": "emit_variant_impl",
            "name": type_name,
            "namespace": ns,
            "cpp": cpp_name,
            "options": out_opts,
            "into": "source",
        })


def _join_inc(sub8_file_path: str, inc: str) -> str:
    # keep it simple: if sub8_file_path is empty, just return inc
    base = (sub8_file_path or "").rstrip("/")
    if not base:
        return inc
    return f"{base}/{inc}"
