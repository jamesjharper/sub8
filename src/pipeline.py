#!/usr/bin/env python3
from __future__ import annotations

"""
Central CLI for the full codegen pipeline.

Goals:
  - One entrypoint that can run:
      * stage1 only (merge/normalize)
      * stage2 only (YAML -> op-plan)
      * stage3 only (op-plan -> generated files)
      * the full pipeline (in-memory, writing only final outputs)
  - Keep CLI / filesystem concerns *here*.
  - Keep stage logic inside pipeline_core.py and the stage modules.

Examples:
  # stage1 (merge)
  python3 pipeline.py stage1 ./example/example.yaml -o build/merged.yaml

  # stage2 (op-plan)
  python3 pipeline.py stage2 build/merged.yaml -o build/codegen_plan.yaml

  # stage3 (emit C++)
  python3 pipeline.py stage3 build/codegen_plan.yaml --repo-root . -o .

  # full pipeline (no intermediate files unless requested)
  python3 pipeline.py all ./example/example.yaml --repo-root . -o .
"""

import argparse
import json
import os
import sys
from typing import Any, Dict

import yaml

import pipeline_core


def _read_yaml(path: str) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        doc = yaml.safe_load(f) or {}
    if not isinstance(doc, dict):
        raise ValueError(f"Top-level YAML must be a map: {path}")
    return doc


def _write_text(path: str, text: str) -> None:
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def _write_yaml(path: str, doc: Dict[str, Any], *, as_json: bool = False) -> None:
    if as_json:
        text = json.dumps(doc, indent=2, sort_keys=True) + "\n"
    else:
        text = yaml.safe_dump(doc, sort_keys=False)
    _write_text(path, text)


def _cmd_stage1(args: argparse.Namespace) -> int:
    merged = pipeline_core.run_stage1(args.root, prune_emit_only=not args.no_prune)
    if args.out:
        _write_yaml(args.out, merged, as_json=args.json)
    else:
        if args.json:
            sys.stdout.write(json.dumps(merged, indent=2, sort_keys=True) + "\n")
        else:
            sys.stdout.write(yaml.safe_dump(merged, sort_keys=False))
    return 0


def _cmd_stage2(args: argparse.Namespace) -> int:
    merged = _read_yaml(args.merged)
    plan = pipeline_core.run_stage2(merged)
    if args.out:
        _write_yaml(args.out, plan, as_json=args.json)
    else:
        if args.json:
            sys.stdout.write(json.dumps(plan, indent=2, sort_keys=True) + "\n")
        else:
            sys.stdout.write(yaml.safe_dump(plan, sort_keys=False))
    return 0


def _cmd_stage3(args: argparse.Namespace) -> int:
    plan = _read_yaml(args.plan)
    outputs = pipeline_core.run_stage3(plan, repo_root=args.repo_root)

    out_base = args.out or "."
    for rel, text in outputs.items():
        out_path = os.path.join(out_base, rel)
        _write_text(out_path, text)
        print(f"Wrote {out_path}")
    return 0


def _cmd_all(args: argparse.Namespace) -> int:
    res = pipeline_core.run_all(
        args.root,
        prune_emit_only=not args.no_prune,
        repo_root=args.repo_root,
    )

    # Optionally write intermediates
    if args.emit_merged:
        _write_yaml(args.emit_merged, res.merged_doc, as_json=args.json)
        print(f"Wrote {args.emit_merged}")
    if args.emit_plan:
        _write_yaml(args.emit_plan, res.plan_doc, as_json=args.json)
        print(f"Wrote {args.emit_plan}")

    # Always write final generated outputs (or print if --stdout)
    if args.stdout:
        # Deterministic order
        for path in sorted(res.outputs.keys()):
            sys.stdout.write(f"\n===== {path} =====\n")
            sys.stdout.write(res.outputs[path])
        return 0

    out_base = args.out or "."
    for rel, text in res.outputs.items():
        out_path = os.path.join(out_base, rel)
        _write_text(out_path, text)
        print(f"Wrote {out_path}")

    return 0


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(prog="pipeline.py", description="sub8 codegen pipeline (stage1+stage2+stage3)")
    sub = ap.add_subparsers(dest="cmd", required=True)

    # stage1
    ap1 = sub.add_parser("stage1", help="Stage1: merge/import/substitute/normalize/prune")
    ap1.add_argument("root", help="Root YAML (e.g. example/example.yaml)")
    ap1.add_argument("-o", "--out", default="", help="Output path (default: stdout)")
    ap1.add_argument("--no-prune", action="store_true", help="Disable emit_only_if_referenced pruning")
    ap1.add_argument("--json", action="store_true", help="Emit JSON instead of YAML")
    ap1.set_defaults(_fn=_cmd_stage1)

    # stage2
    ap2 = sub.add_parser("stage2", help="Stage2: merged YAML -> codegen_plan")
    ap2.add_argument("merged", help="merged.yaml (output of stage1)")
    ap2.add_argument("-o", "--out", default="", help="Output path (default: stdout)")
    ap2.add_argument("--json", action="store_true", help="Emit JSON instead of YAML")
    ap2.set_defaults(_fn=_cmd_stage2)

    # stage3
    ap3 = sub.add_parser("stage3", help="Stage3: codegen_plan -> generated files")
    ap3.add_argument("plan", help="codegen_plan.yaml (output of stage2)")
    ap3.add_argument("--repo-root", default=".", help="Repo root for inline includes")
    ap3.add_argument("-o", "--out", default=".", help="Output base directory")
    ap3.set_defaults(_fn=_cmd_stage3)

    # all
    apA = sub.add_parser("all", help="Run the full pipeline in-memory and write only final outputs")
    apA.add_argument("root", help="Root YAML (e.g. example/example.yaml)")
    apA.add_argument("--repo-root", default=".", help="Repo root for inline includes")
    apA.add_argument("-o", "--out", default=".", help="Output base directory for generated files")
    apA.add_argument("--no-prune", action="store_true", help="Disable emit_only_if_referenced pruning")
    apA.add_argument("--emit-merged", default="", help="Also write merged.yaml to this path")
    apA.add_argument("--emit-plan", default="", help="Also write codegen_plan.yaml to this path")
    apA.add_argument("--json", action="store_true", help="Emit intermediates as JSON instead of YAML")
    apA.add_argument("--stdout", action="store_true", help="Print generated files to stdout instead of writing")
    apA.set_defaults(_fn=_cmd_all)

    args = ap.parse_args(argv)
    return args._fn(args)


if __name__ == "__main__":
    raise SystemExit(main())
