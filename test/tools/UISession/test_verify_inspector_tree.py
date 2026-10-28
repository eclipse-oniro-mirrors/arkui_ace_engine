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

"""Test the validator with synthetic inputs, independently of device captures."""

import json
from pathlib import Path
import unittest

from verify_inspector_tree import validate


CASES = json.loads(Path(__file__).with_name("inspector_tree_cases.json").read_text())["cases"]
PROVIDER_NODE = {
    "elementId": "100", "parentId": "-1", "childNodeIds": [],
    "screenRect": {"leftTopX": 0, "leftTopY": 0, "rightBottomX": 10, "rightBottomY": 10},
    "$rect": "[0.00, 0.00],[10.00,10.00]",
}


class InspectorTreeValidatorTest(unittest.TestCase):
    def nodes(self, case_id):
        return [{"compid": node_id} for node_id in CASES[case_id]["requiredIds"]]

    def failures(self, case_id, nodes, regions=False, providers=False):
        tree = {"appInfo": {"PID": 42, "WindowID": 1, "WindowTitle": "validator-test"}, "$children": nodes}
        if regions or providers:
            tree["errorMessage"] = "no error"
        _, failures = validate(json.dumps({"0": json.dumps(tree)}), CASES[case_id], regions, providers, False)
        return failures

    def test_disabled_regions_accept_omitted_fields(self):
        self.assertEqual(self.failures("response_regions", self.nodes("response_regions")), [])

    def test_disabled_regions_reject_present_fields(self):
        nodes = self.nodes("response_regions")
        nodes[0]["responseRegion"] = ["[0,0],[10,10]"]
        self.assertIn("responseRegion present with collection disabled", self.failures("response_regions", nodes))

    def test_enabled_regions_require_all_configured_tools(self):
        nodes = self.nodes("response_regions")
        nodes[0]["responseRegionMap"] = {tool: ["[0,0],[10,10]"] for tool in CASES["response_regions"]["regionTools"]}
        self.assertEqual(self.failures("response_regions", nodes, regions=True), [])
        del nodes[0]["responseRegionMap"]["pen"]
        self.assertIn("configured tool keys not retained", self.failures("response_regions", nodes, regions=True))

    def test_unconfigured_regions_remain_omitted(self):
        nodes = self.nodes("response_regions")
        nodes[0]["responseRegionMap"] = {tool: [] for tool in CASES["response_regions"]["regionTools"]}
        nodes[-1]["responseRegion"] = ["[0,0],[10,10]"]
        self.assertIn("unconfigured region fabricated", self.failures("response_regions", nodes, regions=True))

    def test_normal_providers_reject_empty_arrays(self):
        nodes = self.nodes("provider_normal")
        for node in nodes:
            node["accessibilityTree"] = []
        self.assertIn("nonempty provider fixture not returned",
                      self.failures("provider_normal", nodes, providers=True))

    def test_normal_providers_reject_one_empty_array(self):
        nodes = self.nodes("provider_normal")
        nodes[0]["accessibilityTree"] = [PROVIDER_NODE]
        nodes[1]["accessibilityTree"] = []
        self.assertIn("nonempty provider fixture not returned",
                      self.failures("provider_normal", nodes, providers=True))

    def test_normal_providers_accept_nonempty_arrays(self):
        nodes = self.nodes("provider_normal")
        for node in nodes:
            node["accessibilityTree"] = [PROVIDER_NODE]
        self.assertEqual(self.failures("provider_normal", nodes, providers=True), [])

    def test_disabled_providers_accept_omitted_fields(self):
        for case_id in ("provider_normal", "provider_empty"):
            for regions in (False, True):
                with self.subTest(case=case_id, regions=regions):
                    self.assertEqual(self.failures(case_id, self.nodes(case_id), regions=regions), [])

    def test_disabled_providers_reject_present_fields(self):
        nodes = self.nodes("provider_normal")
        nodes[0]["accessibilityTree"] = [PROVIDER_NODE]
        self.assertIn("provider result present with collection disabled", self.failures("provider_normal", nodes))

    def test_empty_providers_accept_empty_arrays(self):
        nodes = self.nodes("provider_empty")
        for node in nodes:
            node["accessibilityTree"] = []
        self.assertEqual(self.failures("provider_empty", nodes, providers=True), [])

    def test_empty_providers_reject_nonempty_arrays(self):
        nodes = self.nodes("provider_empty")
        for node in nodes:
            node["accessibilityTree"] = [PROVIDER_NODE]
        self.assertIn("successful empty providers must return empty arrays",
                      self.failures("provider_empty", nodes, providers=True))


if __name__ == "__main__":
    unittest.main()
