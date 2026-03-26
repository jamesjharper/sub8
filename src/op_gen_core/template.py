from __future__ import annotations

"""Tiny template engine used by the YAML -> C++ op generators.

Supported constructs inside YAML strings:
  - $tokens (e.g. $16, $false)
  - ${expr} interpolations (nesting supported)
  - {cond? a : b} ternaries (best-effort nesting)
"""

import re
from typing import Any, Dict, List, Optional, Tuple


_RE_DOLLAR_TOKEN = re.compile(r"\$(?P<t>(?:\d+)|(?:[A-Za-z_][A-Za-z0-9_]*))")
_RE_INTERP_START = re.compile(r"\$\{")


def _lookup(scopes: List[Dict[str, Any]], key: str) -> Any:
    """Resolve either dotted paths (a.b.c) or bare identifiers."""

    def _get_from(obj: Any, path: List[str]) -> Tuple[bool, Any]:
        cur = obj
        for p in path:
            if isinstance(cur, dict) and p in cur:
                cur = cur[p]
            else:
                return False, None
        return True, cur

    # dotted path
    if "." in key:
        path = key.split(".")
        for s in scopes:
            ok, v = _get_from(s, path)
            if ok:
                return v
        return None

    # bare identifier
    for s in scopes:
        if key in s:
            return s[key]
        # convenience: allow schema.* via bare name
        sch = s.get("schema")
        if isinstance(sch, dict) and key in sch:
            return sch[key]
    return None


class _Fn:
    @staticmethod
    def is_true(v: Any) -> bool:
        return bool(v) is True

    @staticmethod
    def is_false(v: Any) -> bool:
        return bool(v) is False

    @staticmethod
    def has_value(v: Any) -> bool:
        return v is not None



def _expr_literal(v: Any) -> str:
    # Convert evaluated value into a literal that the expression evaluator can consume safely.
    if isinstance(v, bool):
        return "true" if v else "false"
    if v is None:
        return "null"
    if isinstance(v, (int, float)):
        return str(v)
    if isinstance(v, str):
        return repr(v)
    return repr(v)


def _expand_nested_interpolations_in_expr(expr: str, *, scopes: List[Dict[str, Any]], adapters: Dict[str, Any]) -> str:
    """Expand any nested ${...} blocks that appear *inside* an expression string.

    Example: "fn.is_false(${code_config.enable_stl_strings})" -> "fn.is_false(false)"
    """
    out = expr
    # Keep expanding until no more ${...} remain.
    while True:
        start = out.find("${")
        if start == -1:
            return out

        # Find matching '}' for the *first* '${' using a simple nesting counter.
        i = start + 2
        depth = 1
        while i < len(out) and depth > 0:
            if out.startswith("${", i):
                depth += 1
                i += 2
                continue
            if out[i] == "}":
                depth -= 1
                if depth == 0:
                    break
            i += 1

        if depth != 0:
            # Unbalanced; leave as-is.
            return out

        inner = out[start + 2 : i]
        inner_v = _eval_expr(inner, scopes=scopes, adapters=adapters)
        lit = _expr_literal(inner_v)

        out = out[:start] + lit + out[i + 1 :]

def _eval_expr(expr: str, *, scopes: List[Dict[str, Any]], adapters: Dict[str, Any]) -> Any:
    expr = expr.strip()

    # Allow nested ${...} inside expressions (stage1-style composition).
    if "${" in expr:
        expr = _expand_nested_interpolations_in_expr(expr, scopes=scopes, adapters=adapters)

    # C-style ternary inside ${...}. We avoid naive splits due to C++ '::'.
    if "?" in expr and ":" in expr:
        qm = -1
        sep = -1
        depth_paren = 0
        for i, ch in enumerate(expr):
            if ch == "(":
                depth_paren += 1
            elif ch == ")":
                if depth_paren > 0:
                    depth_paren -= 1
            elif ch == "?" and depth_paren == 0 and qm == -1:
                qm = i
            elif ch == ":" and depth_paren == 0 and qm != -1:
                prev_c = expr[i - 1] if i > 0 else ""
                next_c = expr[i + 1] if i + 1 < len(expr) else ""
                if prev_c == ":" or next_c == ":":
                    continue
                sep = i
                break
        if qm != -1 and sep != -1 and sep > qm:
            cond_s = expr[:qm].strip()
            t_s = expr[qm + 1 : sep].strip()
            f_s = expr[sep + 1 :].strip()
            cond_v = _eval_expr(cond_s, scopes=scopes, adapters=adapters)
            return t_s if bool(cond_v) else f_s

    # adapters override / inject computed values
    if expr in adapters:
        a = adapters[expr]
        return a() if callable(a) else a

    # dotted/bare lookups
    if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)*", expr):
        v = _lookup(scopes, expr)
        if v is None and expr in adapters:
            a = adapters[expr]
            return a() if callable(a) else a
        return v

    # has(x)
    m = re.fullmatch(r"has\(([^)]+)\)", expr)
    if m:
        inner = m.group(1).strip()
        v = _lookup(scopes, inner)
        return v is not None

    # fn.is_true(x) / fn.is_false(x) / fn.has_value(x)
    m = re.fullmatch(r"fn\.(is_true|is_false|has_value)\((.+)\)", expr)
    if m:
        fn_name = m.group(1)
        inner = m.group(2)
        inner_v = _eval_expr(inner, scopes=scopes, adapters=adapters)
        return getattr(_Fn, fn_name)(inner_v)

    # ultra-restricted eval for simple literals + dotted lookups
    env: Dict[str, Any] = {
        "true": True,
        "false": False,
        "null": None,
        "None": None,
    }

    def repl(mo: re.Match) -> str:
        ident = mo.group(0)
        if ident in env:
            return ident
        return f"__lookup__('{ident}')"

    expr2 = re.sub(r"[A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)+", repl, expr)
    return eval(
        expr2,
        {"__builtins__": {}},
        {"__lookup__": lambda k: _eval_expr(k, scopes=scopes, adapters=adapters), **env},
    )


def _render_dollar_tokens(s: str) -> str:
    def _tok(m: re.Match) -> str:
        t = m.group("t")
        if t.isdigit():
            return t
        low = t.lower()
        if low == "true":
            return "true"
        if low == "false":
            return "false"
        if low in ("null", "none"):
            return "null"
        return t

    return _RE_DOLLAR_TOKEN.sub(_tok, s)


def _render_interpolations(s: str, *, scopes: List[Dict[str, Any]], adapters: Dict[str, Any]) -> str:
    """Render ${...} with support for nesting."""

    out = s
    while True:
        if not _RE_INTERP_START.search(out):
            return out

        stack: List[int] = []
        replaced = False
        i = 0
        while i < len(out):
            if out.startswith("${", i):
                stack.append(i)
                i += 2
                continue
            if out[i] == "}" and stack:
                start = stack.pop()
                if not stack:
                    expr = out[start + 2 : i]
                    v = _eval_expr(expr, scopes=scopes, adapters=adapters)
                    if isinstance(v, bool):
                        rep = "true" if v else "false"
                    elif v is None:
                        rep = "null"
                    else:
                        rep = str(v)
                    out = out[:start] + rep + out[i + 1 :]
                    replaced = True
                    break
            i += 1

        if not replaced:
            # Unbalanced; return as-is.
            return out


def _render_brace_ternaries(s: str, *, scopes: List[Dict[str, Any]], adapters: Dict[str, Any]) -> str:
    """Render {cond? a : b} blocks."""

    def find_innermost(text: str) -> Optional[Tuple[int, int]]:
        last_open = -1
        depth = 0
        for i, ch in enumerate(text):
            if ch == "{":
                depth += 1
                last_open = i
            elif ch == "}":
                if depth > 0:
                    start = last_open
                    end = i
                    return start, end
        return None

    out = s
    while True:
        rng = find_innermost(out)
        if rng is None:
            break
        a, b = rng
        inner = out[a + 1 : b]

        qm = -1
        sep = -1
        depth = 0
        for i, ch in enumerate(inner):
            if ch == "{":
                depth += 1
            elif ch == "}":
                if depth > 0:
                    depth -= 1
            elif ch == "?" and depth == 0 and qm == -1:
                qm = i
            elif ch == ":" and depth == 0 and qm != -1:
                prev_c = inner[i - 1] if i > 0 else ""
                next_c = inner[i + 1] if i + 1 < len(inner) else ""
                if prev_c == ":" or next_c == ":":
                    continue
                sep = i
                break

        if qm == -1 or sep == -1 or sep < qm:
            expr_s = _render_dollar_tokens(inner.strip())
            expr_s = _render_interpolations(expr_s, scopes=scopes, adapters=adapters)
            v = _eval_expr(expr_s, scopes=scopes, adapters=adapters)
            if isinstance(v, bool):
                rep = "true" if v else "false"
            elif v is None:
                rep = "null"
            else:
                rep = str(v)
        else:
            cond_s = inner[:qm].strip()
            t_s = inner[qm + 1 : sep].strip()
            f_s = inner[sep + 1 :].strip()
            cond_s_r = _render_dollar_tokens(cond_s)
            cond_s_r = _render_interpolations(cond_s_r, scopes=scopes, adapters=adapters)
            cond_v = _eval_expr(cond_s_r, scopes=scopes, adapters=adapters)
            rep = t_s if bool(cond_v) else f_s

        rep = _render_dollar_tokens(rep)
        rep = _render_interpolations(rep, scopes=scopes, adapters=adapters)
        out = out[:a] + rep + out[b + 1 :]
    return out


def render_template(value: Any, *, scopes: List[Dict[str, Any]], adapters: Optional[Dict[str, Any]] = None) -> Any:
    """Render templates embedded in YAML strings."""

    adapters = adapters or {}
    if value is None:
        return None
    if isinstance(value, (int, float, bool)):
        return value
    if isinstance(value, list):
        return [render_template(v, scopes=scopes, adapters=adapters) for v in value]
    if isinstance(value, dict):
        return {k: render_template(v, scopes=scopes, adapters=adapters) for k, v in value.items()}
    if not isinstance(value, str):
        return value

    s = value
    s = _render_dollar_tokens(s)
    if "{" in s and "}" in s and "?" in s:
        s = _render_brace_ternaries(s, scopes=scopes, adapters=adapters)
    s = _render_interpolations(s, scopes=scopes, adapters=adapters)
    s = _render_dollar_tokens(s)
    return s
