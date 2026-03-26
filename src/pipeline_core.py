#!/usr/bin/env python3
from __future__ import annotations

"""
Pipeline orchestrator (in-memory).

Stages:
  1) merge.stage1_process (root YAML -> merged normalized YAML dict)
  2) op_gen.stage2_process (merged YAML dict -> codegen_plan dict)
  3) code_gen.emit_plan (codegen_plan dict -> {path: file_text})

This module contains *no* CLI parsing and performs *no* filesystem writes
(other than reading inline includes during stage3 when enabled). It is safe
to import and call from tests.
"""

from dataclasses import dataclass
from typing import Any, Dict, Mapping, Optional

import os

# Stage 1
import merge as stage1_merge

# Stage 2
import op_gen as stage2_opgen  # re-exports stage2_process from op_gen_core

# Stage 3
import code_gen as stage3_emit


JsonObj = Dict[str, Any]


@dataclass(frozen=True)
class PipelineResult:
    """Full in-memory result of running the pipeline end-to-end."""
    merged_doc: JsonObj
    plan_doc: JsonObj
    outputs: Dict[str, str]  # relative path -> file contents


def run_stage1(root_yaml_path: str, *, prune_emit_only: bool = True) -> JsonObj:
    return stage1_merge.stage1_process(root_yaml_path, prune_emit_only=prune_emit_only)


def run_stage2(merged_doc: Mapping[str, Any]) -> JsonObj:
    # stage2_process expects a dict-like YAML document
    return stage2_opgen.stage2_process(dict(merged_doc))


def _compute_header_include_for_cpp(codegen_cfg: Mapping[str, Any]) -> Optional[str]:
    header_path = codegen_cfg.get("header")
    source_path = codegen_cfg.get("source")
    if not header_path or not source_path:
        return None
    if os.path.dirname(str(header_path)) == os.path.dirname(str(source_path)):
        return os.path.basename(str(header_path))
    return str(header_path)


def run_stage3(
    plan_doc: Mapping[str, Any],
    *,
    repo_root: str = ".",
) -> Dict[str, str]:
    """
    Returns:
      Dict[path, file_text]
    """
    doc = dict(plan_doc)
    if doc.get("version") != 2:
        raise ValueError("Only codegen_plan v2 supported")

    codegen_cfg = doc.get("codegen", {}) or {}
    global_inline_headers = bool(codegen_cfg.get("inline_headers", False))
    header_include_for_cpp = _compute_header_include_for_cpp(codegen_cfg)

    outputs: Dict[str, str] = {}
    plans = doc.get("plans", [])
    if not isinstance(plans, list):
        raise TypeError("plan_doc.plans must be a list")

    for plan in plans:
        if not isinstance(plan, dict):
            raise TypeError(f"plan entry must be dict, got {type(plan).__name__}")
        path = str(plan.get("path", ""))
        is_header = path.endswith(".h") or path.endswith(".hpp")
        text = stage3_emit.emit_plan(
            plan,
            repo_root=repo_root,
            global_inline_headers=global_inline_headers,
            is_header=is_header,
            header_include_for_cpp=header_include_for_cpp if not is_header else None,
        )
        outputs[path] = text

    return outputs


def run_all(
    root_yaml_path: str,
    *,
    prune_emit_only: bool = True,
    repo_root: str = ".",
) -> PipelineResult:
    merged = run_stage1(root_yaml_path, prune_emit_only=prune_emit_only)
    plan = run_stage2(merged)
    outputs = run_stage3(plan, repo_root=repo_root)
    return PipelineResult(merged_doc=merged, plan_doc=plan, outputs=outputs)
