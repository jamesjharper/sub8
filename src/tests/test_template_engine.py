from __future__ import annotations

import pytest


def _render(value, scopes, adapters=None):
    # Import via public API to ensure the op_gen facade re-exports keep working.
    import op_gen

    return op_gen.render_template(value, scopes=scopes, adapters=adapters or {})


def test_dollar_tokens_numbers_and_bools_and_null():
    scopes = [{}]
    assert _render("$16", scopes) == "16"
    assert _render("$true $false", scopes) == "true false"
    assert _render("$null $None", scopes) == "null null"


def test_basic_interpolation_with_schema_fields():
    scopes = [{"schema": {"bitwidth": 16, "signed": False}}]
    assert (
        _render("sub8::Integer<${schema.bitwidth}, ${schema.signed}>", scopes)
        == "sub8::Integer<16, false>"
    )


def test_bare_identifier_can_resolve_from_schema_convenience():
    scopes = [{"schema": {"bitwidth": 16}}]
    assert _render("${bitwidth}", scopes) == "16"


def test_nested_interpolation_fn_is_false_matches_core_yaml_pattern():
    # Mirrors: "${fn.is_false(${code_config.enable_stl_strings})}"
    scopes = [
        {
            "code_config": {"enable_stl_strings": False},
        }
    ]
    assert _render("${fn.is_false(${code_config.enable_stl_strings})}", scopes) == "true"


def test_brace_ternary_has_picks_true_branch_when_present():
    scopes = [{"schema": {"bounded_buffer_size": 12}}]
    tpl = "{has(bounded_buffer_size)? A${schema.bounded_buffer_size} : B}"
    assert _render(tpl, scopes) == "A12"


def test_brace_ternary_has_picks_false_branch_when_missing():
    scopes = [{"schema": {}}]
    tpl = "{has(bounded_buffer_size)? A : B}"
    assert _render(tpl, scopes) == "B"


def test_cpp_scope_operator_does_not_break_colon_in_ternary_parsing():
    scopes = [{"schema": {"signed": False}}]
    tpl = "${fn.is_true(${schema.signed})? sub8::X::Y : sub8::Z}"
    assert _render(tpl, scopes) == "sub8::Z"


def test_adapters_can_inject_values_or_callables():
    scopes = [{}]
    assert _render("${cpp_type}", scopes, adapters={"cpp_type": "int"}) == "int"
    assert (
        _render("${cpp_type}", scopes, adapters={"cpp_type": lambda: "long"})
        == "long"
    )


def test_unbalanced_interpolation_is_left_as_is():
    scopes = [{"schema": {"bitwidth": 16}}]
    assert _render("x${schema.bitwidth", scopes) == "x${schema.bitwidth"


def test_render_is_recursive_over_lists_and_dicts():
    scopes = [{"schema": {"bitwidth": 16, "signed": False}}]
    assert _render(["${schema.bitwidth}", "$false"], scopes) == ["16", "false"]
    assert _render({"a": "${schema.signed}"}, scopes) == {"a": "false"}


def test_interpolation_allows_simple_expressions_with_dotted_lookups():
    scopes = [{"schema": {"bitwidth": 16}}]
    assert _render("${schema.bitwidth + 1}", scopes) == "17"
