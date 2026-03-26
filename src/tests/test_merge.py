# tests/test_stage1_merge.py
#
# Unit tests for stage1_merge.py
# Covers:
#  - import/merge behavior
#  - substitute expansion (including nested ${...})
#  - inline type lifting (object fields, variant option types, nested array/optional/etc)
#  - fixed-point lifting (types created during lifting are revisited)
#  - emit_only_if_referenced pruning via dependency graph
#  - stage1 metadata (_stage1 roots/reachable)
#
# Run:
#   pip install pytest pyyaml
#   pytest -q
#
# Assumptions:
#  - stage1_merge.py is importable on PYTHONPATH (same dir as tests or installed)
#  - stage1_merge.py exposes stage1_process(root_yaml_path: str, prune_emit_only: bool=True) -> dict

from __future__ import annotations

import os
import sys
from pathlib import Path
from typing import Any, Dict, Set

import pytest
import yaml


# Add repo root (parent of tests/) to sys.path
REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

import merge  # now works reliably

# ----------------------------
# Fixtures: core + example yaml
# ----------------------------

CORE_YAML = r"""
codegen:
  schema:
    - field: { name: "sub8_file_path", type: "string"}
    - field: { name: "default_namespace", type: "string"}
  generators:
    cpp:
      schema:
        - field: { name: "header", type: "string", required: true }
        - field: { name: "source", type: "string", required: true }

codegen_config_kinds:
  never_throw_exceptions:
    schema: { type: bool, required: true, default_value: false }
    cpp: { type: "define", define: "SUB8_ENABLE_NEVER_THROW_EXCEPTIONS" }

  infallible_methods_only:
    schema: { type: bool, required: true, default_value: false }
    cpp: { type: "define", define: "SUB8_ENABLE_INFALLIBLE" }

  enable_stl_strings:
    schema: { type: bool, required: true, default_value: false }
    cpp: { type: "define", define: "SUB8_ENABLE_BASIC_STRING" }

kinds:
  object:
    cpp: { type: "cpp_object_code_gen" }
    schema:
      - field: { name: "namespace", type: "string", required: false }
      - map: { name: "fields",  key_type: string, value_type: "kind", required: true }

  variant:
    cpp: { type: "cpp_variant_code_gen" }
    schema:
      - field: { name: "namespace", type: "string", required: false }
      - map:
          name: "options"
          required: true
          key_type: string
          value_type:
            schema:
              - field: { name: "id", type: u32, required: true }
              - field: { name: "type", type: "type_ref", required: true }

  integer:
    schema:
      - field: { name: "bitwidth", type: u32, required: true }
      - field: { name: "signed", type: bool, required: true }
    code_gen:
      cpp:
        type: "struct"
        def: "Integer<{bitwidth}, {signed}>"
        namespace: "sub8"
        include: "sub8_primitives.h"

  vbr_integer:
    schema:
      - field: { name: "signed", type: bool, required: true }
      - field: { name: "segments", type: "u32[]", required: true }
    code_gen:
      cpp:
        type: "struct"
        def: "VbrInteger<{signed}, {segments}>"
        namespace: "sub8"
        include: "sub8_primitives.h"

  float:
    schema:
      - field: { name: "exponent", type: u32, required: true }
      - field: { name: "fraction", type: u32, required: true }
    code_gen:
      cpp:
        type: "struct"
        def: "FloatingPoint<{exponent}, {fraction}>"
        namespace: "sub8"
        include: "sub8_floats.h"

  optional:
    schema:
      - field: { name: "no_alloc", type: bool, required: false, default_value: true }
      - field: { name: "type", type: type_ref, required: true }
    code_gen:
      cpp:
        type: "struct"
        def: "sub8::TOptionalContainer<${no_alloc}, ${type}>"
        include: "sub8_optional.h"
        namespace: "sub8"

  array:
    schema:
      - field: { name: "min_elements", type: u32, required: false, default_value: 0 }
      - field: { name: "max_elements", type: u32, required: true }
      - field: { name: "encoding", type: "array_encoding", default_value: "three_plus_prefixed" }
      - field: { name: "type", type: "type_ref", required: true }
    code_gen:
      cpp:
        type: "struct"
        def: "sub8::Array<sub8::ObjectElement<${type}>, ${min_elements}, ${max_elements}, ${encoding}>"
        include: "sub8_arrays.h"
        namespace: "sub8"

  array_encoding:
    schema: { type: enum, required: true, values: [three_plus_prefixed, delimited, prefixed] }
    code_gen:
      cpp:
        type: "enum"
        def: "ArrayEncoding"
        namespace: "sub8"
        include: "sub8_arrays.h"
        values:
          three_plus_prefixed: "ThreePlusPrefixed"
          delimited: "Delimited"
          prefixed: "Prefixed"

  b5_string:
    schema:
      - field: { name: "max_length", type: u32, required: true }
      - field: { name: "terminated_sequence", type: bool, required: false, default_value: false }
      - field: { name: "starting_multi_code_state", type: bool, required: false, default_value: false }
      - field: { name: "bounded_buffer_size", type: u32, required: "{!enable_stl_strings}"}
    code_gen:
      cpp:
        include: "sub8_strings.h"
        def: "sub8::BoundedString<{bounded_buffer_size}, /*encoder*/void, /*decoder*/void>"

types:
  u16: { kind: integer, bitwidth: 16, signed: false, emit_only_if_referenced: true }
  u32: { kind: integer, bitwidth: 32, signed: false, emit_only_if_referenced: true }
  bfloat16: { kind: float, exponent: 8, fraction: 7, emit_only_if_referenced: true }
  UnusedType: { kind: integer, bitwidth: 7, signed: false, emit_only_if_referenced: true }
"""

EXAMPLE_YAML = r"""
import:
  - core.yaml

substitute:
  some_example_value: false

codegen:
  sub8_file_path: "./sub8/"
  default_namespace: "users::name::space"
  cpp:
    header: "generated/sub8_types.h"
    source: "generated/sub8_types.cpp"

code_config:
  never_throw_exceptions: false
  infallible_methods_only: "${some_example_value}"
  enable_stl_strings: false

types:
  HelloRequestMessage:
    kind: "object"
    namespace: "users::name::space"
    fields:
      say_it_n_time: u16
      hello_phrase:
        kind: b5_string
        max_length: 64
        terminated_sequence: false
        starting_multi_code_state: false
        bounded_buffer_size: 64

  HelloMessage:

    kind: "variant"
    namespace: "users::name::space"
    options:
      hello_request:
        id: 1
        type: HelloRequestMessage
      hello_response:
        id: 2
        type:
          name: HelloResponseMessage
          type: object
          fields:
            response_phrase:
              kind: b5_string
              max_length: 64
              terminated_sequence: false
              starting_multi_code_state: false
              bounded_buffer_size: 64
            list:
              kind: array
              min_elements: 0
              max_elements: 5
              encoding: "delimited"
              type: MessageItem

  MessageItem:
    kind: "object"
    namespace: "users::name::space"
    fields:
      feild_1: u16
      feild_2: u32
      feild_3: bfloat16
"""


# ----------------------------
# Helpers
# ----------------------------

def _write(tmp_path: Path, name: str, content: str) -> Path:
    p = tmp_path / name
    p.write_text(content, encoding="utf-8")
    return p

def _load_yaml(p: Path) -> Dict[str, Any]:
    return yaml.safe_load(p.read_text(encoding="utf-8")) or {}

def _all_type_names(doc: Dict[str, Any]) -> Set[str]:
    return set((doc.get("types") or {}).keys())

def _assert_all_object_fields_are_strings(doc: Dict[str, Any]) -> None:
    types = doc.get("types") or {}
    for tname, tdef in types.items():
        if not isinstance(tdef, dict):
            continue
        if tdef.get("kind") == "object":
            fields = tdef.get("fields") or {}
            if isinstance(fields, dict):
                for fname, fref in fields.items():
                    assert isinstance(fref, str), f"{tname}.fields.{fname} is not a string: {type(fref)}"

def _assert_all_variant_option_types_are_strings(doc: Dict[str, Any]) -> None:
    types = doc.get("types") or {}
    for tname, tdef in types.items():
        if not isinstance(tdef, dict):
            continue
        if tdef.get("kind") == "variant":
            opts = tdef.get("options") or {}
            if isinstance(opts, dict):
                for oname, odef in opts.items():
                    assert isinstance(odef, dict), f"{tname}.options.{oname} must be a map"
                    assert isinstance(odef.get("type"), str), f"{tname}.options.{oname}.type is not a string"

def _assert_all_array_type_slots_are_strings(doc: Dict[str, Any]) -> None:
    types = doc.get("types") or {}
    for tname, tdef in types.items():
        if not isinstance(tdef, dict):
            continue
        if tdef.get("kind") == "array":
            assert "type" in tdef, f"array type '{tname}' missing 'type' slot"
            assert isinstance(tdef["type"], str), f"array '{tname}'.type is not a string"

def _import_stage1():
    # Import here so pytest collection doesn't fail if the file isn't importable yet.
    return merge


# ----------------------------
# Tests
# ----------------------------

def test_stage1_import_and_merge(tmp_path: Path):
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)

    # basic merge: core sections exist
    assert "kinds" in doc
    assert "codegen_config_kinds" in doc

    # example overrides codegen values
    assert doc["codegen"]["sub8_file_path"] == "./sub8/"
    assert doc["codegen"]["default_namespace"] == "users::name::space"
    assert doc["codegen"]["cpp"]["header"] == "generated/sub8_types.h"
    assert doc["codegen"]["cpp"]["source"] == "generated/sub8_types.cpp"


def test_stage1_substitute_expansion(tmp_path: Path):
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)

    # ${some_example_value} should be expanded to boolean False
    assert doc["code_config"]["infallible_methods_only"] is False


def test_stage1_inline_type_lift_object_field(tmp_path: Path):
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)
    types = doc["types"]

    # HelloRequestMessage.hello_phrase should be a string type name
    hp = types["HelloRequestMessage"]["fields"]["hello_phrase"]
    assert isinstance(hp, str)

    # Lifted type must exist and have kind b5_string
    assert hp in types
    assert types[hp]["kind"] == "b5_string"
    assert types[hp]["max_length"] == 64
    assert types[hp]["bounded_buffer_size"] == 64


def test_stage1_inline_named_type_in_variant_option(tmp_path: Path):
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)
    types = doc["types"]

    # The inline named HelloResponseMessage must exist as a proper type
    assert "HelloResponseMessage" in types
    assert types["HelloResponseMessage"]["kind"] == "object"

    # And the variant option must reference it by string
    opt_type = types["HelloMessage"]["options"]["hello_response"]["type"]
    assert opt_type == "HelloResponseMessage"


def test_stage1_fixed_point_lifting_nested_fields_and_array(tmp_path: Path):
    """
    Ensures stage1 continues lifting until stable:
      - HelloResponseMessage.fields.* are strings
      - The array type has a string element type
    """
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)
    types = doc["types"]

    hr = types["HelloResponseMessage"]
    assert hr["kind"] == "object"
    assert isinstance(hr["fields"]["response_phrase"], str)
    assert isinstance(hr["fields"]["list"], str)

    list_t = hr["fields"]["list"]
    assert list_t in types
    assert types[list_t]["kind"] == "array"
    assert types[list_t]["encoding"] == "delimited"
    assert types[list_t]["min_elements"] == 0
    assert types[list_t]["max_elements"] == 5

    # critical: array element type must be a string
    assert "type" in types[list_t]
    assert isinstance(types[list_t]["type"], str)
    assert types[list_t]["type"] == "MessageItem"


def test_stage1_normalizes_all_typerefs_to_strings(tmp_path: Path):
    """
    Global contract stage2 depends on:
      - object.fields values are strings
      - variant.options[*].type are strings
      - array.type is a string
    """
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)

    _assert_all_object_fields_are_strings(doc)
    _assert_all_variant_option_types_are_strings(doc)
    _assert_all_array_type_slots_are_strings(doc)


def test_stage1_prunes_emit_only_if_referenced_by_default(tmp_path: Path):
    """
    core.yaml defines UnusedType with emit_only_if_referenced: true
    and example.yaml never references it. It should be removed when prune is enabled.
    """
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)
    names = _all_type_names(doc)

    assert "UnusedType" not in names  # pruned
    # referenced core aliases should remain
    assert "u16" in names
    assert "u32" in names
    assert "bfloat16" in names


def test_stage1_no_prune_keeps_emit_only_if_referenced(tmp_path: Path):
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=False)
    names = _all_type_names(doc)

    assert "UnusedType" in names  # kept when pruning disabled


def test_stage1_emits_dependency_metadata(tmp_path: Path):
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)

    assert "_stage1" in doc
    meta = doc["_stage1"]
    assert "roots" in meta and isinstance(meta["roots"], list)
    assert "reachable" in meta and isinstance(meta["reachable"], list)

    reachable = set(meta["reachable"])
    # must include user declared roots
    assert "HelloMessage" in reachable
    assert "HelloRequestMessage" in reachable
    assert "MessageItem" in reachable
    # must include referenced core aliases
    assert "u16" in reachable
    assert "u32" in reachable
    assert "bfloat16" in reachable


def test_stage1_preserves_core_schema_values(tmp_path: Path):
    """
    Ensures core kinds/schema are still present after merge + normalization.
    Also ensures the quoted u32[] survives as a string.
    """
    s1 = _import_stage1()
    _write(tmp_path, "core.yaml", CORE_YAML)
    root = _write(tmp_path, "example.yaml", EXAMPLE_YAML)

    doc = s1.stage1_process(str(root), prune_emit_only=True)

    assert "kinds" in doc
    assert "vbr_integer" in doc["kinds"]
    vbr_schema = doc["kinds"]["vbr_integer"]["schema"]
    # find the 'segments' field schema item and verify type is "u32[]"
    seg_items = [
        x for x in vbr_schema
        if isinstance(x, dict) and "field" in x and isinstance(x["field"], dict) and x["field"].get("name") == "segments"
    ]
    assert len(seg_items) == 1
    assert seg_items[0]["field"]["type"] == "u32[]"
