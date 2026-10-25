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
    check(type(tree) is dict, "decoded tree must be an object")
    if type(tree) is not dict:
        return assertions, failures
    expected_error = case.get("errorMessage") if providers else None
    if expected_error:
        expected = {"errorMessage": expected_error} if regions or providers else {}
        check(tree == expected, "failed request must discard the whole tree and report its first error")
        return assertions, failures
    check((tree.get("errorMessage") == "no error") if regions or providers else
          "errorMessage" not in tree, "root errorMessage gate does not match the request")
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
        check("accessibilityTreeComplete" not in node, "obsolete host completeness field")
        if node is not tree:
            check("errorMessage" not in node, "errorMessage must only appear at the tree root")
        if "accessibilityTree" in node:
            check(providers, "provider result present with collection disabled")
            hosts.append(node)
            third = node["accessibilityTree"]
            check(type(third) is list, "provider result must be a flat array")
            if type(third) is not list:
                continue
            for item in third:
                check(type(item) is dict, "third-provider node must be an object")
                if type(item) is not dict:
                    continue
                check(type(item.get("$rect")) is str, "missing provider window rectangle")
                check(type(item.get("screenRect")) is dict, "missing original provider screen rectangle")
                check(type(item.get("childNodeIds")) is list, "missing original child IDs")
                for key in ("elementId", "parentId"):
                    check(type(item.get(key)) is str and re.fullmatch(r"0|-?[1-9][0-9]*", item[key]) is not None,
                          "provider ID must be a canonical decimal string")
                check("$children" not in item, "provider list must not become a nested tree")
    for required in case.get("requiredIds", []):
        check(required in by_id, "missing fixture node: " + required)
    for required in case.get("emptyContentIds", []):
        check(required in by_id and by_id[required].get("$attrs", {}).get("content") == "",
              "protected input not empty: " + required)
    for key, expected in case.get("text", {}).items():
        check(key in by_id and by_id[key].get("$attrs", {}).get("content") == expected,
              "ordinary label changed: " + key)
    if regions and "regionTools" in case:
        found = by_id.get("tool-regions", {}).get("responseRegionMap", {})
        check(set(found) == set(case["regionTools"]), "configured tool keys not retained")
        check(not any(k in by_id.get("no-region", {}) for k in
                      ("responseRegionMap", "responseRegion", "mouseResponseRegion")), "unconfigured region fabricated")
    if providers and case.get("providerNonempty"):
        check(bool(hosts) and all(host["accessibilityTree"] for host in hosts),
              "nonempty provider fixture not returned")
    if providers and case.get("providerEmpty"):
        check(bool(hosts) and all(host["accessibilityTree"] == [] for host in hosts),
              "successful empty providers must return empty arrays")
    return assertions, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--case", required=True)
    parser.add_argument("--with-response-region", action="store_true")
    parser.add_argument("--with-custom-content", action="store_true")
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
                                   args.with_custom_content, args.accessibility_info)
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
