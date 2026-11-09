"""Regression tests for the AI Audit correctness fixes."""
from __future__ import annotations

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from code_gen import emit_variant_decl, emit_variant_impl
from merge import load_with_imports


def _variant_options():
    return [
        {"name": "small", "id": 1, "cpp": "sub8::U8"},
        {"name": "large", "id": 7, "cpp": "sub8::U16"},
    ]


def test_variant_sparse_tags_reject_invalid_on_wire():
    op = {"name": "Packet", "namespace": "audit", "options": _variant_options()}
    decl = emit_variant_decl(op)
    assert "default: return sub8::BitFieldResult::ErrorInvalidBitFieldValue;" in decl
    assert "default: out.construct_null(); return sub8::BitFieldResult::ErrorInvalidBitFieldValue;" in decl
    assert "r = t.set_value(" in decl
    assert "if (r != sub8::BitFieldResult::Ok) return r;" in decl


@pytest.mark.parametrize("bad_ids", [[0], [-1], [1, 1], [0x100000000]])
def test_variant_tag_ids_must_be_unique_and_nonzero(bad_ids):
    opts = [{"name": f"choice{i}", "id": v, "cpp": "sub8::U8"} for i, v in enumerate(bad_ids)]
    with pytest.raises(ValueError, match="Invalid or duplicate"):
        emit_variant_decl({"name": "Packet", "options": opts})


def test_variant_generator_does_not_promise_nothrow_copy():
    op = {"name": "Packet", "namespace": "audit", "options": _variant_options()}
    decl = emit_variant_decl(op)
    impl = emit_variant_impl(op)
    assert "set_value(const Packet& v) { *this = v;" in decl
    assert "Packet(Packet&& o);" in decl
    assert "operator=(Packet&& o);" in decl
    assert "set_small(const sub8::U8& v);" in decl
    assert "audit::Packet::Packet(audit::Packet&& o) {" in impl
    assert "audit::Packet::set_small(const sub8::U8& v) {" in impl
    assert "sub8::U8 value_copy(v);" in impl


def test_import_cycle_reports_chain(tmp_path):
    a = tmp_path / "a.yaml"
    b = tmp_path / "b.yaml"
    a.write_text("import:\n  - b.yaml\n", encoding="utf-8")
    b.write_text("import:\n  - a.yaml\n", encoding="utf-8")
    with pytest.raises(ValueError, match="Cyclic YAML imports") as err:
        load_with_imports(str(a))
    assert "a.yaml" in str(err.value)
    assert "b.yaml" in str(err.value)


def test_repeated_import_without_cycle_is_allowed(tmp_path):
    common = tmp_path / "common.yaml"
    a = tmp_path / "a.yaml"
    b = tmp_path / "b.yaml"
    root = tmp_path / "root.yaml"
    common.write_text("types: {hello: {kind: object}}\n", encoding="utf-8")
    a.write_text("import:\n  - common.yaml\n", encoding="utf-8")
    b.write_text("import:\n  - common.yaml\n", encoding="utf-8")
    root.write_text("import:\n  - a.yaml\n  - b.yaml\n", encoding="utf-8")
    doc = load_with_imports(str(root))
    assert "hello" in doc["types"]
