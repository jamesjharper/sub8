from __future__ import annotations

import argparse
import json
import os
import sys

try:
    import yaml  # pip install pyyaml
except ImportError:  # pragma: no cover
    print("Missing dependency: pyyaml. Install with: pip install pyyaml", file=sys.stderr)
    raise

from .stage2 import stage2_process


def main() -> int:
    ap = argparse.ArgumentParser(description="Stage2: merged YAML -> op-plan YAML")
    ap.add_argument(
        "root",
        help="Merged YAML file (e.g. build/merged.yaml). Use '-' to read from stdin.",
    )
    ap.add_argument("-o", "--out", default="", help="Output path (default: stdout)")
    ap.add_argument("--json", action="store_true", help="Output JSON instead of YAML")
    args = ap.parse_args()

    if args.root == "-":
        doc = yaml.safe_load(sys.stdin.read() or "")
    else:
        with open(args.root, "r", encoding="utf-8") as f:
            doc = yaml.safe_load(f)

    plan = stage2_process(doc)

    if args.json:
        text = json.dumps(plan, indent=2, sort_keys=True) + "\n"
    else:
        text = yaml.safe_dump(plan, sort_keys=False)

    if args.out:
        os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
        with open(args.out, "w", encoding="utf-8") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    return 0
