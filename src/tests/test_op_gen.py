# tests/test_op_gen.py
#
# Stage2 (op plan) tests.
#
# This file injects a tiny in-test "languages.cpp.op_gens" module so stage2_process can run
# without requiring your real generator implementations to exist yet.
#
# The injected generators are intentionally minimal but exercise:
#   - header/source required validation
#   - version/codegen block output
#   - define/include ordering + dedupe via normalize_ops
#   - topo ordering via stage2 dependency extraction
#   - basic typeref string contract checks (we enforce in our fake generators)
#   - b5_string bounded/unbounded switching based on code_config.enable_stl_strings
#   - template engine: brace DSL + ${} + $tokens (numeric + identifier)

import sys
import types
import pytest

import op_gen as s2


# -----------------------------------------------------------------------------
# Inject a fake languages.cpp.op_gens module for stage2 reflection
# -----------------------------------------------------------------------------

class _GenDefine:
    def emit_config_ops(
        self,
        *,
        doc,
        cfg_name,
        cfg_def,
        cfg_value,
        codegen_root,
        lang_node,
        reg,
        ops,
        sub8_file_path,
        default_namespace,
    ):
        # schema says cpp.define
        name = lang_node["define"]
        ops.append({"op": "define", "name": name, "value": 1 if bool(cfg_value) else 0})


class _GenStruct:
    def emit_type_ops(
        self,
        *,
        doc,
        type_name,
        type_node,
        kind_def,
        codegen_root,
        lang_node,
        reg,
        ops,
        sub8_file_path,
        default_namespace,
    ):
        # include(s)
        incs = lang_node.get("includes") or lang_node.get("include") or []
        if isinstance(incs, str):
            incs = [incs]
        for inc in incs:
            ops.append({"op": "include", "path": f"{sub8_file_path}{inc}"})

        k = type_node["kind"]

        # minimal rendered type expansion used by other gens + assertions
        if k == "integer":
            bw = int(type_node["bitwidth"])
            sg = bool(type_node["signed"])
            rendered = f"sub8::Integer<{bw}, {'true' if sg else 'false'}>"
        elif k == "float":
            e = int(type_node["exponent"])
            f = int(type_node["fraction"])
            rendered = f"sub8::FloatingPoint<{e}, {f}>"
        elif k == "array":
            # encoding enum mapping
            enc = str(type_node.get("encoding", "three_plus_prefixed"))
            enc_map = {
                "three_plus_prefixed": "ThreePlusPrefixed",
                "delimited": "Delimited",
                "prefixed": "Prefixed",
            }
            if enc not in enc_map:
                raise ValueError("unknown array encoding")
            # typeref must be string
            tref = type_node["type"]
            if not isinstance(tref, str):
                raise ValueError("Stage1 must normalize typerefs to strings")
            # resolve referenced cpp from registry
            inner = reg.get(tref).rendered
            rendered = (
                f"sub8::Array<sub8::ObjectElement<{inner}>, "
                f"{int(type_node.get('min_elements', 0))}, {int(type_node['max_elements'])}, "
                f"sub8::ArrayEncoding::{enc_map[enc]}>"
            )
        elif k == "b5_string":
            # Template-driven path (if def exists): this is what new tests require.
            def_tmpl = (lang_node or {}).get("def")
            if isinstance(def_tmpl, str) and ("{" in def_tmpl or "${" in def_tmpl or "$" in def_tmpl):
                scopes = [
                    {
                        "schema": {
                            "max_length": type_node.get("max_length"),
                            "bounded_buffer_size": type_node.get("bounded_buffer_size"),
                        }
                    },
                    {"code_config": doc.get("code_config") or {}},
                    {"codegen": doc.get("codegen") or {}},
                ]
                rendered = s2.render_template(def_tmpl, scopes=scopes, adapters={})
                rendered = str(rendered).strip()
            else:
                enable_stl_strings = bool((doc.get("code_config") or {}).get("enable_stl_strings", False))
                if not enable_stl_strings:
                    if "bounded_buffer_size" not in type_node:
                        raise ValueError(f"{type_name} requires bounded_buffer_size when stl strings are disabled")
                    bbs = int(type_node["bounded_buffer_size"])
                    rendered = f"sub8::BoundedString<{bbs}, sub8::B5Enc<{int(type_node['max_length'])}>>"
                else:
                    rendered = f"sub8::UnboundedString<sub8::B5Enc<{int(type_node['max_length'])}>>"
        else:
            # generic fallback
            rendered = f"{type_name}__UNSUPPORTED_IN_TEST"

        reg.add(type_name, kind=k, rendered=rendered)
        ops.append({"op": "emit_alias", "name": type_name, "cpp": rendered})


class _GenObject:
    def emit_type_ops(
        self,
        *,
        doc,
        type_name,
        type_node,
        kind_def,
        codegen_root,
        lang_node,
        reg,
        ops,
        sub8_file_path,
        default_namespace,
    ):
        incs = lang_node.get("includes") or lang_node.get("include") or []
        if isinstance(incs, str):
            incs = [incs]
        for inc in incs:
            ops.append({"op": "include", "path": f"{sub8_file_path}{inc}"})
        # tests expect these always
        ops.append({"op": "include", "path": f"{sub8_file_path}sub8_io.h"})

        ns = type_node.get("namespace") or default_namespace
        cpp = f"{ns}::{type_name}" if ns else type_name

        # Register object type so other gens can reference it
        reg.add(type_name, kind="object", rendered=cpp)

        fields = type_node.get("fields") or {}
        if not isinstance(fields, dict):
            raise ValueError("fields must be a map")

        out_fields = []
        for fname, tref in fields.items():
            if not isinstance(tref, str):
                raise ValueError("Stage1 must normalize typerefs to strings")
            out_fields.append({"name": fname, "type": tref, "cpp": reg.get(tref).rendered})

        ops.append({"op": "emit_object_decl", "name": type_name, "namespace": ns, "cpp": cpp, "fields": out_fields})


class _GenVariant:
    def emit_type_ops(
        self,
        *,
        doc,
        type_name,
        type_node,
        kind_def,
        codegen_root,
        lang_node,
        reg,
        ops,
        sub8_file_path,
        default_namespace,
    ):
        incs = lang_node.get("includes") or lang_node.get("include") or []
        if isinstance(incs, str):
            incs = [incs]
        for inc in incs:
            ops.append({"op": "include", "path": f"{sub8_file_path}{inc}"})
        # tests expect these always
        ops.append({"op": "include", "path": f"{sub8_file_path}sub8_io.h"})
        ops.append({"op": "include", "path": f"{sub8_file_path}sub8_enums.h"})

        ns = type_node.get("namespace") or default_namespace
        cpp = f"{ns}::{type_name}" if ns else type_name
        reg.add(type_name, kind="variant", rendered=cpp)

        opts = type_node.get("options") or {}
        if not isinstance(opts, dict):
            raise ValueError("options must be a map")

        out_opts = []
        for oname, ov in opts.items():
            if not isinstance(ov, dict):
                raise ValueError("option must be a map")
            oid = ov.get("id")
            tref = ov.get("type")
            if not isinstance(tref, str):
                raise ValueError("Stage1 must normalize typerefs to strings")
            out_opts.append({"name": oname, "id": int(oid), "type": tref, "cpp": reg.get(tref).rendered})

        ops.append({"op": "emit_variant_decl", "name": type_name, "namespace": ns, "cpp": cpp, "options": out_opts})


@pytest.fixture(autouse=True)
def _inject_fake_languages_module():
    """
    stage2_process imports: languages.cpp.op_gens

    We inject:
      - languages (package)
      - languages.cpp (package)
      - languages.cpp.op_gens (module with get_generator_registry)
    """
    # Snapshot anything that might already exist
    saved = {k: sys.modules.get(k) for k in ("languages", "languages.cpp", "languages.cpp.op_gens")}

    languages = types.ModuleType("languages")
    languages.__path__ = []  # mark as package-ish

    languages_cpp = types.ModuleType("languages.cpp")
    languages_cpp.__path__ = []

    op_gens = types.ModuleType("languages.cpp.op_gens")

    def get_generator_registry():
        return {
            "cpp_define": _GenDefine,
            "cpp_struct": _GenStruct,
            "cpp_object": _GenObject,
            "cpp_variant": _GenVariant,
            # cpp_enum not needed for these tests
        }

    op_gens.get_generator_registry = get_generator_registry

    sys.modules["languages"] = languages
    sys.modules["languages.cpp"] = languages_cpp
    sys.modules["languages.cpp.op_gens"] = op_gens

    try:
        yield
    finally:
        # restore
        for k, v in saved.items():
            if v is None:
                sys.modules.pop(k, None)
            else:
                sys.modules[k] = v


# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------

def _base_stage1_doc(enable_stl_strings: bool = False):
    """
    Minimal-but-representative stage1_doc for CURRENT schema.
    """
    return {
        "codegen": {
            "sub8_file_path": "./sub8/",  # NOTE: stage2 does NOT rstrip in current code
            "default_namespace": "users::name::space",
            "cpp": {
                "header": "generated/sub8_types.h",
                "source": "generated/sub8_types.cpp",
            },
        },
        "code_config": {
            "enable_stl_strings": enable_stl_strings,
            "infallible_methods_only": False,
        },
        "codegen_config_kinds": {
            "infallible_methods_only": {
                "schema": {"type": "bool", "required": True, "default_value": False},
                "cpp": {"type": "cpp_define", "define": "SUB8_ENABLE_INFALLIBLE"},
            },
        },
        "kinds": {
            "object": {
                "cpp": {"type": "cpp_object", "includes": ["sub8_api.h"]},
            },
            "variant": {
                "cpp": {"type": "cpp_variant", "includes": ["sub8_api.h"]},
            },
            "integer": {
                "schema": [
                    {"field": {"name": "bitwidth", "type": "u32", "required": True}},
                    {"field": {"name": "signed", "type": "bool", "required": True}},
                ],
                "code_gen": {
                    "cpp": {
                        "type": "cpp_struct",
                        "def": "sub8::Integer<${schema.bitwidth}, ${schema.signed}>",
                        "namespace": "sub8",
                        "includes": ["sub8_primitives.h"],
                    }
                },
            },
            "float": {
                "schema": [
                    {"field": {"name": "exponent", "type": "u32", "required": True}},
                    {"field": {"name": "fraction", "type": "u32", "required": True}},
                ],
                "code_gen": {
                    "cpp": {
                        "type": "cpp_struct",
                        "def": "sub8::FloatingPoint<${schema.exponent}, ${schema.fraction}>",
                        "namespace": "sub8",
                        "includes": ["sub8_floats.h"],
                    }
                },
            },
            "array": {
                "schema": [
                    {"field": {"name": "min_elements", "type": "u32", "required": False, "default_value": 0}},
                    {"field": {"name": "max_elements", "type": "u32", "required": True}},
                    {"field": {"name": "encoding", "type": "array_encoding", "required": False, "default_value": "three_plus_prefixed"}},
                    {"field": {"name": "type", "type": "type_ref", "required": True}},
                ],
                "code_gen": {
                    "cpp": {
                        "type": "cpp_struct",
                        "def": "sub8::Array<sub8::ObjectElement<${schema.type}>, ${schema.min_elements}, ${schema.max_elements}, ${schema.encoding}>",
                        "namespace": "sub8",
                        "includes": ["sub8_arrays.h"],
                    }
                },
            },
            "b5_string": {
                "schema": [
                    {"field": {"name": "max_length", "type": "u32", "required": True}},
                    {"field": {"name": "bounded_buffer_size", "type": "u32", "required": False}},
                ],
                "code_gen": {
                    "cpp": {
                        "type": "cpp_struct",
                        "def": "/* complex template in real YAML */",
                        "namespace": "sub8",
                        "includes": ["sub8_strings.h"],
                    }
                },
            },
        },
        "types": {
            "u16": {"kind": "integer", "bitwidth": 16, "signed": False, "emit_only_if_referenced": True},
            "u32": {"kind": "integer", "bitwidth": 32, "signed": False, "emit_only_if_referenced": True},
            "bfloat16": {"kind": "float", "exponent": 8, "fraction": 7, "emit_only_if_referenced": True},
            "HelloRequestMessage_hello_phrase_T1": {
                "kind": "b5_string",
                "max_length": 64,
                "bounded_buffer_size": 64,
            },
            "HelloResponseMessage_response_phrase_T2": {
                "kind": "b5_string",
                "max_length": 64,
                "bounded_buffer_size": 64,
            },
            "HelloResponseMessage_list_T3": {
                "kind": "array",
                "min_elements": 0,
                "max_elements": 5,
                "encoding": "delimited",
                "type": "MessageItem",
            },
            "HelloRequestMessage": {
                "kind": "object",
                "namespace": "users::name::space",
                "fields": {"say_it_n_time": "u16", "hello_phrase": "HelloRequestMessage_hello_phrase_T1"},
            },
            "MessageItem": {
                "kind": "object",
                "namespace": "users::name::space",
                "fields": {"feild_1": "u16", "feild_2": "u32", "feild_3": "bfloat16"},
            },
            "HelloResponseMessage": {
                "kind": "object",
                "namespace": "users::name::space",
                "fields": {
                    "response_phrase": "HelloResponseMessage_response_phrase_T2",
                    "list": "HelloResponseMessage_list_T3",
                },
            },
            "HelloMessage": {
                "kind": "variant",
                "namespace": "users::name::space",
                "options": {
                    "hello_request": {"id": 1, "type": "HelloRequestMessage"},
                    "hello_response": {"id": 2, "type": "HelloResponseMessage"},
                },
            },
        },
        "_stage1": {
            "roots": ["HelloMessage", "HelloRequestMessage", "HelloResponseMessage", "MessageItem"],
            "reachable": [],  # empty => stage2 processes all types
        },
    }


def _doc_with_b5_string_brace_def(enable_stl_strings: bool):
    doc = _base_stage1_doc(enable_stl_strings=enable_stl_strings)
    doc["kinds"]["b5_string"]["code_gen"]["cpp"]["def"] = """{has(bounded_buffer_size)?
  sub8::BoundedString<${bounded_buffer_size}, sub8::B5Enc<${max_length}>, {fn.is_false(code_config.enable_stl_strings)}>
: sub8::UnboundedString<sub8::B5Enc<${max_length}>>
}"""
    return doc


def _plan(ir, name: str):
    return next(p for p in ir["plans"] if p.get("name") == name)


def _ops_by(ir, op_name, plan: str = "header"):
    return [o for o in _plan(ir, plan)["ops"] if o.get("op") == op_name]


def _first_index(ir, op_name, plan: str = "header"):
    for i, o in enumerate(_plan(ir, plan)["ops"]):
        if o.get("op") == op_name:
            return i
    return None


# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

def test_stage2_requires_codegen_header_and_source():
    doc = _base_stage1_doc()
    doc["codegen"]["cpp"].pop("header")
    with pytest.raises(s2.SchemaError, match=r"codegen\.cpp\.header and codegen\.cpp\.source are required"):
        s2.stage2_process(doc)


def test_stage2_outputs_version_and_codegen_block():
    ir = s2.stage2_process(_base_stage1_doc())
    assert ir["version"] == 2
    assert ir["codegen"]["header"] == "generated/sub8_types.h"
    assert ir["codegen"]["source"] == "generated/sub8_types.cpp"
    # NOTE: current stage2 DOES NOT rstrip; keep exact
    assert ir["codegen"]["sub8_file_path"] == "./sub8/"
    assert ir["codegen"]["default_namespace"] == "users::name::space"


def test_stage2_define_ops_come_before_include_ops_bugfix_guard():
    ir = s2.stage2_process(_base_stage1_doc())
    i_define = _first_index(ir, "define")
    i_include = _first_index(ir, "include")
    assert i_define is not None, "expected at least one define op"
    assert i_include is not None, "expected include ops"
    assert i_define < i_include, "define ops must come before include ops"


def test_stage2_define_values_reflect_code_config():
    doc = _base_stage1_doc()
    ir = s2.stage2_process(doc)
    defs = _ops_by(ir, "define")
    assert {"op": "define", "name": "SUB8_ENABLE_INFALLIBLE", "value": 0} in defs

    doc2 = _base_stage1_doc()
    doc2["code_config"]["infallible_methods_only"] = True
    ir2 = s2.stage2_process(doc2)
    defs2 = _ops_by(ir2, "define")
    assert {"op": "define", "name": "SUB8_ENABLE_INFALLIBLE", "value": 1} in defs2


def test_stage2_includes_are_deduped_and_prefixed_with_sub8_path():
    ir = s2.stage2_process(_base_stage1_doc())
    incs = [o["path"] for o in _ops_by(ir, "include")]

    # always-present from our fake generators
    assert "./sub8/sub8_api.h" in incs
    assert "./sub8/sub8_io.h" in incs
    assert "./sub8/sub8_enums.h" in incs

    # from struct-like kinds used by types
    assert "./sub8/sub8_primitives.h" in incs
    assert "./sub8/sub8_floats.h" in incs
    assert "./sub8/sub8_strings.h" in incs
    assert "./sub8/sub8_arrays.h" in incs

    assert len(incs) == len(set(incs)), "include ops should be deduped"


def test_stage2_emits_object_ops_with_resolved_field_cpp_types():
    ir = s2.stage2_process(_base_stage1_doc())
    objs = [o for o in _plan(ir, "header")["ops"] if o.get("op") == "emit_object_decl"]
    names = {o["name"] for o in objs}
    assert {"HelloRequestMessage", "HelloResponseMessage", "MessageItem"} <= names

    hr = next(o for o in objs if o["name"] == "HelloRequestMessage")
    assert hr["namespace"] == "users::name::space"
    assert hr["cpp"] == "users::name::space::HelloRequestMessage"
    f = {x["name"]: x for x in hr["fields"]}
    assert f["say_it_n_time"]["type"] == "u16"
    assert "sub8::Integer<16" in f["say_it_n_time"]["cpp"]
    assert f["hello_phrase"]["type"] == "HelloRequestMessage_hello_phrase_T1"
    assert "sub8::BoundedString<64" in f["hello_phrase"]["cpp"]


def test_stage2_emits_variant_ops_with_resolved_option_cpp_types():
    ir = s2.stage2_process(_base_stage1_doc())
    vars_ = [o for o in _plan(ir, "header")["ops"] if o.get("op") == "emit_variant_decl"]
    assert len(vars_) >= 1
    hm = next(o for o in vars_ if o["name"] == "HelloMessage")
    assert hm["namespace"] == "users::name::space"
    assert hm["cpp"] == "users::name::space::HelloMessage"
    opts = {x["name"]: x for x in hm["options"]}
    assert opts["hello_request"]["id"] == 1
    assert opts["hello_request"]["type"] == "HelloRequestMessage"
    assert opts["hello_request"]["cpp"] == "users::name::space::HelloRequestMessage"
    assert opts["hello_response"]["id"] == 2
    assert opts["hello_response"]["cpp"] == "users::name::space::HelloResponseMessage"


def test_stage2_emits_alias_ops_for_non_object_non_variant_types():
    ir = s2.stage2_process(_base_stage1_doc())
    aliases = [o for o in _plan(ir, "header")["ops"] if o.get("op") == "emit_alias"]
    names = {o["name"] for o in aliases}

    assert "u16" in names
    assert "u32" in names
    assert "bfloat16" in names
    assert "HelloRequestMessage_hello_phrase_T1" in names
    assert "HelloResponseMessage_list_T3" in names

    arr = next(o for o in aliases if o["name"] == "HelloResponseMessage_list_T3")
    assert "sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>" in arr["cpp"]
    assert "sub8::ArrayEncoding::Delimited" in arr["cpp"]


def test_stage2_topological_order_emits_dependencies_before_dependers():
    ir = s2.stage2_process(_base_stage1_doc())
    type_ops = [o for o in _plan(ir, "header")["ops"] if o["op"].startswith("emit_")]
    idx = {o["name"]: i for i, o in enumerate(type_ops)}

    assert idx["HelloResponseMessage_list_T3"] < idx["HelloResponseMessage"]
    assert idx["HelloResponseMessage_response_phrase_T2"] < idx["HelloResponseMessage"]
    assert idx["HelloRequestMessage"] < idx["HelloMessage"]
    assert idx["HelloResponseMessage"] < idx["HelloMessage"]


def test_stage2_rejects_non_string_typerefs_in_object_fields():
    doc = _base_stage1_doc()
    doc["types"]["HelloRequestMessage"]["fields"]["say_it_n_time"] = {"kind": "integer", "bitwidth": 8, "signed": False}
    with pytest.raises(ValueError, match=r"Stage1 must normalize typerefs to strings"):
        s2.stage2_process(doc)


def test_stage2_rejects_non_string_typerefs_in_variant_options():
    doc = _base_stage1_doc()
    doc["types"]["HelloMessage"]["options"]["hello_request"]["type"] = {"name": "X", "kind": "object", "fields": {}}
    with pytest.raises(ValueError, match=r"Stage1 must normalize typerefs to strings"):
        s2.stage2_process(doc)


def test_stage2_b5_string_mode_switches_with_enable_stl_strings():
    ir = s2.stage2_process(_base_stage1_doc(enable_stl_strings=False))
    alias = next(o for o in _plan(ir, "header")["ops"] if o.get("op") == "emit_alias" and o["name"] == "HelloRequestMessage_hello_phrase_T1")
    assert "sub8::BoundedString<64" in alias["cpp"]

    ir2 = s2.stage2_process(_base_stage1_doc(enable_stl_strings=True))
    alias2 = next(o for o in _plan(ir2, "header")["ops"] if o.get("op") == "emit_alias" and o["name"] == "HelloRequestMessage_hello_phrase_T1")
    assert "sub8::UnboundedString<" in alias2["cpp"]


def test_stage2_b5_string_requires_bounded_buffer_size_when_stl_strings_disabled():
    doc = _base_stage1_doc(enable_stl_strings=False)
    doc["types"]["HelloRequestMessage_hello_phrase_T1"].pop("bounded_buffer_size", None)
    with pytest.raises(ValueError, match=r"requires bounded_buffer_size"):
        s2.stage2_process(doc)


def test_stage2_template_engine_brace_has_multiline_and_dollar_tokens_selects_bounded():
    doc = _doc_with_b5_string_brace_def(enable_stl_strings=False)

    ir = s2.stage2_process(doc)
    alias = next(o for o in _plan(ir, "header")["ops"] if o.get("op") == "emit_alias" and o["name"] == "HelloRequestMessage_hello_phrase_T1")

    # Chose bounded branch (because bounded_buffer_size exists)
    assert "sub8::BoundedString<64" in alias["cpp"]
    # $max_length substituted
    assert "sub8::B5Enc<64>" in alias["cpp"]
    # brace-expression token evaluated; fn.is_false(False) -> true
    assert "true" in alias["cpp"]


def test_stage2_template_engine_brace_has_multiline_and_dollar_tokens_selects_unbounded_when_missing():
    doc = _doc_with_b5_string_brace_def(enable_stl_strings=True)

    # Remove bounded_buffer_size so has(bounded_buffer_size) is false
    doc["types"]["HelloRequestMessage_hello_phrase_T1"].pop("bounded_buffer_size", None)

    ir = s2.stage2_process(doc)
    alias = next(o for o in _plan(ir, "header")["ops"] if o.get("op") == "emit_alias" and o["name"] == "HelloRequestMessage_hello_phrase_T1")

    # Chose unbounded branch
    assert "sub8::UnboundedString<" in alias["cpp"]
    assert "sub8::B5Enc<64>" in alias["cpp"]


def test_stage2_template_engine_supports_numeric_dollar_tokens():
    doc = _base_stage1_doc(enable_stl_strings=False)

    # Make def contain a raw $64 token (matches the failure you saw: "$64)? ...")
    doc["kinds"]["b5_string"]["code_gen"]["cpp"]["def"] = """{has(bounded_buffer_size)?
  sub8::BoundedString<$64, sub8::B5Enc<$64>>
: sub8::UnboundedString<sub8::B5Enc<$64>>
}"""

    # Ensure bounded_buffer_size exists so we take the first branch
    ir = s2.stage2_process(doc)
    alias = next(o for o in _plan(ir, "header")["ops"] if o.get("op") == "emit_alias" and o["name"] == "HelloRequestMessage_hello_phrase_T1")

    # $64 must be substituted to 64
    assert "sub8::BoundedString<64" in alias["cpp"]
    assert "sub8::B5Enc<64>" in alias["cpp"]
