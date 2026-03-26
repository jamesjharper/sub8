#!/usr/bin/env python3
"""Stage2: YAML -> op-plan IR (public façade).

This file intentionally stays small.

The project historically imported these symbols from `op_gen.py`:

  - SchemaError
  - TypeInfo / TypeRegistry / BaseOpGenerator
  - render_template
  - normalize_ops / stage2_process
  - main (CLI)

To keep backwards compatibility while making the code easier to
maintain, the implementation is split into small modules in
`op_gen_core/`.
"""

from __future__ import annotations

# Public re-exports (keep import paths stable for existing callers)
from op_gen_core.errors import SchemaError
from op_gen_core.registry import BaseOpGenerator, TypeInfo, TypeRegistry
from op_gen_core.stage2 import normalize_ops, stage2_process
from op_gen_core.template import render_template
from op_gen_core.cli import main

__all__ = [
    "SchemaError",
    "TypeInfo",
    "TypeRegistry",
    "BaseOpGenerator",
    "render_template",
    "normalize_ops",
    "stage2_process",
    "main",
]


if __name__ == "__main__":
    raise SystemExit(main())
