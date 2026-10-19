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

"""Validate a real UISession callback capture, without manufacturing a result."""
import argparse
import json
from pathlib import Path
import re
import sys
import time

def validate(capture, case, regions, providers, accessibility):
    assertions = 0
    failures = []

    def check(condition, message):
        nonlocal assertions
        assertions += 1
        if not condition:
            failures.append(message)

    outer = json.loads(capture)
    check(type(outer) is dict and type(outer.get("0")) is str, "callback envelope must contain string 0")
    if failures:
        return assertions, failures
    body = outer["0"]
    tree = json.loads(body)
    app = tree.get("appInfo", {})
    check(type(app.get("PID")) is int and app["PID"] > 0, "app PID must identify the target process")
    check(type(app.get("WindowTitle")) is str, "WindowTitle must be a string")
    check(type(app.get("WindowID")) is int, "WindowID must remain an integer")
    nodes = []
    pending = [tree]
    while pending:
        node = pending.pop()
        check(type(node) is dict, "native node must be an object")
        if type(node) is not dict:
            continue
        nodes.append(node)
        pending.extend(node.get("$children", []))
    by_id = {node.get("compid"): node for node in nodes if node.get("compid")}
    hosts = []
    for node in nodes:
        tag = node.get("$type", "")
        if "accessibilityId" in node:
            value = node["accessibilityId"]
            check(accessibility, "accessibilityId present with accessibilityInfo disabled")
            check(type(value) is str and re.fullmatch(r"0|[1-9][0-9]*", value) is not None,
                  "accessibilityId must be a canonical decimal string")
        for key in ("touchable", "checkable", "checked", "selected", "accessibilityGroup"):
            if key in node:
                check(type(node[key]) is bool, key + " must preserve boolean type")
        if node.get("checkable") is False:
            check("checked" not in node, "non-checkable node must omit checked")
        attrs = node.get("$attrs", {})
        for key in ("password", "multiLine", "expanded"):
            if key in attrs:
                check(type(attrs[key]) is bool, key + " must be boolean")
        for key in ("selectionStart", "selectionEnd", "inputType", "textLengthLimit"):
            if key in attrs:
                check(type(attrs[key]) is int, key + " must be integer")
        for key in ("responseRegionMap", "responseRegion", "mouseResponseRegion"):
            if key in node:
                check(regions, key + " present with collection disabled")
                values = node[key]
                if key == "responseRegionMap":
                    check(type(values) is dict and set(values) <= {"all", "finger", "pen", "mouse"},
                          "invalid tool map")
                    values = list(values.values()) if type(values) is dict else []
                else:
                    values = [values]
                for rectangles in values:
                    check(type(rectangles) is list and all(type(item) is str for item in rectangles),
                          "response region must be a rectangle string array")
        check(not any(key in node for key in
                      ("accessibilityTree", "accessibilityTreeComplete", "errorMessage")),
              "legacy third-provider wrapper must not be emitted")
        if "$children-third" in node:
            check(providers, "provider result present with collection disabled")
            hosts.append(node)
            third = [node["$children-third"]]
            while third:
                item = third.pop()
                check(type(item) is dict, "third-provider node must be an object")
                if type(item) is not dict:
                    continue
                check(type(item.get("$rect")) is str, "missing provider window rectangle")
                check(item.get("type") == "third", "missing third-provider type")
                check(type(item.get("$ID")) is int, "provider ID must follow exporter format")
                children = item.get("$children", [])
                check(type(children) is list, "provider children must follow exporter hierarchy")
                if type(children) is list:
                    third.extend(children)
    for required in case.get("requiredIds", []):
        check(required in by_id, "missing fixture node: " + required)
    for required in case.get("emptyContentIds", []):
        check(required in by_id and by_id[required].get("$attrs", {}).get("content") == "",
              "protected input not empty: " + required)
    for key, expected in case.get("text", {}).items():
        check(key in by_id and by_id[key].get("$attrs", {}).get("content") == expected,
              "ordinary label changed: " + key)
    if "regionTools" in case:
        found = by_id.get("tool-regions", {}).get("responseRegionMap", {})
        check(set(found) == set(case["regionTools"]), "configured tool keys not retained")
        check(not any(k in by_id.get("no-region", {}) for k in
                      ("responseRegionMap", "responseRegion", "mouseResponseRegion")), "unconfigured region fabricated")
    if case.get("providerNonempty"):
        check(bool(hosts), "nonempty provider fixture not returned")
    if case.get("providerOmitted"):
        check(not hosts, "exporter must omit empty/failed/oversized provider data")
    return assertions, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--case", required=True)
    parser.add_argument("--with-response-region", action="store_true")
    parser.add_argument("--with-xcomponent", action="store_true")
    parser.add_argument("--with-third-provider", action="store_true")
    parser.add_argument("--accessibility-info", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--device-build", required=True)
    args = parser.parse_args()
    cases = json.loads(Path(__file__).with_name("inspector_tree_cases.json").read_text())["cases"]
    if args.case not in cases:
        parser.error("unknown case")
    start = time.time_ns() // 1000000
    try:
        count, failures = validate(args.capture.read_text(), cases[args.case], args.with_response_region,
                                   args.with_xcomponent or args.with_third_provider, args.accessibility_info)
    except (ValueError, TypeError, KeyError, IndexError, OSError, RecursionError) as error:
        count, failures = 1, ["capture parsing/validation failed: " + type(error).__name__]
    report = {"caseId": args.case, "deviceBuild": args.device_build, "capture": str(args.capture),
              "startMs": start, "endMs": time.time_ns() // 1000000, "assertions": count,
              "failed": len(failures), "failures": failures}
    args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    print(json.dumps({"assertions": count, "failed": len(failures), "report": str(args.report)}))
    return 1 if failures or count == 0 else 0


if __name__ == "__main__":
    sys.exit(main())
