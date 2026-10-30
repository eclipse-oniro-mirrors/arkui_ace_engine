#!/usr/bin/env python3
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Check branch outcomes on lines changed by the page-level split feature.

Consume LCOV tracefiles or full `llvm-cov export` JSON, including branches. Multiple reports may
represent different test binaries; identical branch regions are merged by hit
count rather than counted twice. Missing source files or branch data fail closed.
The default baseline is the second parent of the feature's merge commit.
"""

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys


SOURCES = (
    "adapter/ohos/entrance/ui_content_impl.cpp",
    "frameworks/bridge/declarative_frontend/ng/force_split/parallel_page_router_manager.cpp",
    "frameworks/core/common/force_split/force_split_utils.cpp",
    "frameworks/core/components_ng/manager/force_split/force_split_manager.cpp",
    "frameworks/core/components_ng/manager/force_split/force_split_manager.h",
    "frameworks/core/components_ng/pattern/navigation/navigation_group_node.cpp",
    "frameworks/core/components_ng/pattern/navigation/navigation_pattern.cpp",
    "frameworks/core/components_ng/pattern/stage/force_split/parallel_stage_manager.cpp",
    "frameworks/core/components_ng/pattern/stage/force_split/parallel_stage_pattern.cpp",
)


def git(root, *args):
    return subprocess.check_output(
        ["git", "-C", str(root), *args], encoding="utf-8", errors="strict"
    )


def changed_lines(root, base):
    changes = {}
    for source in SOURCES:
        diff = git(root, "diff", "--no-ext-diff", "--unified=0", base, "--", source)
        lines = set()
        for match in re.finditer(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@", diff, re.M):
            first = int(match[1])
            size = int(match[2]) if match[2] is not None else 1
            lines.update(range(first, first + size))
        if lines:
            changes[source] = lines
    return changes


def normalize_source(filename):
    filename = filename.replace("\\", "/")
    return next((source for source in SOURCES
                 if filename == source or filename.endswith("/" + source)), None)


def read_branches(reports):
    files = set()
    branches = {}
    formats = set()
    for report in reports:
        text = Path(report).read_text(encoding="utf-8")
        if not text.lstrip().startswith("{"):
            formats.add("lcov")
            source = None
            for line in text.splitlines():
                if line.startswith("SF:"):
                    source = normalize_source(line[3:])
                    if source:
                        files.add(source)
                elif line == "end_of_record":
                    source = None
                elif source and line.startswith("BRDA:"):
                    number, block, branch, taken = line[5:].split(",")
                    key = (source, int(number), block, branch)
                    count = 0 if taken == "-" else int(taken)
                    branches.setdefault(key, [0])[0] += count
            continue
        formats.add("llvm")
        document = json.loads(text)
        if document.get("type") != "llvm.coverage.json.export":
            raise ValueError(f"{report}: expected full llvm-cov export JSON")
        for data in document["data"]:
            for file in data["files"]:
                source = normalize_source(file["filename"])
                if source is None:
                    continue
                files.add(source)
                for branch in file.get("branches", []):
                    if len(branch) < 6:
                        raise ValueError(f"{report}: unsupported branch record: {branch!r}")
                    # LLVM: start line/column, end line/column, true/false counts, ...
                    key = (source, *branch[:4])
                    previous = branches.setdefault(key, [0, 0])
                    previous[0] += branch[4]
                    previous[1] += branch[5]
    if len(formats) > 1:
        raise ValueError("Use either LCOV or LLVM reports in one run, not both.")
    return files, branches


def evaluate(changes, files, branches, minimum):
    results = []
    failures = []
    for source, lines in changes.items():
        if source not in files:
            failures.append(f"No coverage mapping for changed source: {source}")
            continue
        regions = [(key, counts) for key, counts in branches.items()
                   if key[0] == source and any(
                       key[1] <= line <= (key[3] if len(key) == 5 else key[1]) for line in lines)]
        if not regions:
            failures.append(f"No changed-line branch data: {source}")
            continue
        total = sum(len(counts) for _, counts in regions)
        covered = sum(count > 0 for _, counts in regions for count in counts)
        percent = 100.0 * covered / total
        uncovered = [{"region": list(key[1:]), "outcome": outcome}
                     for key, counts in regions
                     for outcome, count in zip(("true", "false") if len(counts) == 2 else ("branch",), counts)
                     if count == 0]
        results.append({"file": source, "covered": covered, "total": total,
                        "percent": percent, "uncovered": uncovered})
        if percent < minimum:
            failures.append(f"{source}: {percent:.2f}% < {minimum:.2f}%")
    covered = sum(result["covered"] for result in results)
    total = sum(result["total"] for result in results)
    percent = 100.0 * covered / total if total else 0.0
    if not changes:
        failures.append("No feature changes found; check --base and the checkout.")
    if not total or percent < minimum:
        failures.append(f"Overall changed-line branches: {percent:.2f}% < {minimum:.2f}%")
    return {"passed": not failures, "covered": covered, "total": total,
            "percent": percent, "files": results, "failures": failures}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reports", nargs="+", help="LCOV tracefiles or full llvm-cov export JSON files")
    parser.add_argument("--base", default="15aeee621d303eca5f8de196ec6ae8320f1edd3c^2")
    parser.add_argument("--minimum", type=float, default=90.0)
    parser.add_argument("--output", type=Path, help="Optional machine-readable result")
    args = parser.parse_args()
    if not 0 < args.minimum <= 100:
        parser.error("--minimum must be in (0, 100]")
    root = git(Path(__file__).resolve().parent, "rev-parse", "--show-toplevel").strip()
    files, branches = read_branches(args.reports)
    result = evaluate(changed_lines(root, args.base), files, branches, args.minimum)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    if args.output:
        args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        print(f"Coverage check failed: {error}", file=sys.stderr)
        sys.exit(2)
