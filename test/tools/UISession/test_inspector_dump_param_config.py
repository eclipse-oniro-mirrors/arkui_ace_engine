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

"""Host checks for the production UIExtension dump parameter codec.

Compile the sender's parameter-building block and the receiver's actual parser
with the production ParamConfig and numeric conversion. This checks their wire
format independently of the device-only UIExtension session/lifecycle fixture.
Source extraction fails if the expected function boundaries change.
"""

import itertools
from pathlib import Path
import re
import subprocess
import tempfile
import unittest


REPO = Path(__file__).resolve().parents[3]


def function(source, signature):
    start = source.index(signature + "\n{")
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if depth == 0:
            return source[start:index + 1]
    raise ValueError("Unclosed function: " + signature)


class InspectorDumpParamConfigTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pipeline = (REPO / "frameworks/core/pipeline_ng/pipeline_context.cpp").read_text()
        sender = (REPO / "frameworks/core/components_ng/pattern/ui_extension/"
                         "ui_extension_component/ui_extension_pattern.cpp").read_text()
        strings = (REPO / "frameworks/base/utils/string_utils.cpp").read_text()
        string_header = (REPO / "frameworks/base/utils/string_utils.h").read_text()
        send_body = function(sender, "void UIExtensionPattern::AddExtraInfoWithParamConfig("
                                     "std::shared_ptr<JsonValue>& json, ParamConfig config)")
        send_start = send_body.index("    std::vector<std::string> params;")
        send_end = send_body.index("    auto host = GetHost();", send_start)
        constants = "\n".join(re.findall(r"constexpr size_t DUMP_PARAM_\w+\s*=\s*[^;]+;", pipeline))
        pid_flag = re.search(r"constexpr char PID_FLAG\[\]\s*=\s*[^;]+;", pipeline).group()
        percent = re.search(r"constexpr double PERCENT_VALUE\s*=\s*[^;]+;", string_header).group()
        source = '''#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include "interfaces/inner_api/ui_session/param_config.h"
namespace OHOS::Ace::StringUtils {
''' + percent + "\n" + function(strings, "bool StringToDouble(const std::string& value, double& result)") + '''
}
using namespace OHOS::Ace;
''' + constants + "\n" + pid_flag + "\n"
        for signature in ("bool ParseBoolParam(const std::string& param)",
                          "float ParseMinOpacityParam(const std::string& param)",
                          "ParamConfig ParseDumpParamConfig(const std::vector<std::string>& params, size_t startIndex)",
                          "ParamConfig ParseDumpParamConfig(const std::vector<std::string>& params)"):
            source += function(pipeline, signature) + "\n"
        source += "std::vector<std::string> Encode(ParamConfig config)\n{\n"
        source += send_body[send_start:send_end] + "    return params;\n}\n"
        source += '''int main(int argc, char** argv)
{
    if (argc != 4) { return 2; }
    ParamConfig config;
    config.interactionInfo = true;
    config.withUIExtension = true;
    config.rectCulling = true;
    config.minOpacity = 0.25f;
    config.withResponseRegion = std::string(argv[1]) == "1";
    config.withCustomContent = std::string(argv[2]) == "1";
    auto params = Encode(config);
    const std::string mode = argv[3];
    if (mode == "legacy") { params.resize(9); }
    if (mode == "response-only") { params.resize(10); }
    if (mode == "visible") {
        params[0] = "-visibleInfoWithModifiedParam";
        params.erase(params.begin() + 1);
    }
    if (mode != "nested") { params.push_back(PID_FLAG); }
    params.push_back("1"); // A PID that would parse as true if mistaken for a flag.
    auto decoded = mode == "visible" ? ParseDumpParamConfig(params, DUMP_PARAM_VISIBLE_INFO_START_INDEX) :
        ParseDumpParamConfig(params);
    std::cout << decoded.interactionInfo << ' ' << decoded.accessibilityInfo << ' ' << decoded.cacheNodes << ' '
              << decoded.withWeb << ' ' << decoded.withUIExtension << ' ' << decoded.rectCulling << ' '
              << decoded.minOpacity << ' ' << decoded.withResponseRegion << ' ' << decoded.withCustomContent;
}
'''
        cls.directory = tempfile.TemporaryDirectory(prefix="inspector-dump-config-")
        cls.addClassCleanup(cls.directory.cleanup)
        cpp = Path(cls.directory.name) / "roundtrip.cpp"
        cpp.write_text(source)
        cls.binary = cpp.with_suffix("")
        build = subprocess.run(["c++", "-std=c++17", "-Wall", "-Wextra", "-Werror", "-I", str(REPO),
                                str(cpp), "-o", str(cls.binary)], capture_output=True, text=True)
        if build.returncode:
            raise RuntimeError(build.stderr)

    def check_roundtrip(self, mode, response, custom, expected):
        result = subprocess.run([str(self.binary), str(response), str(custom), mode],
                                check=True, capture_output=True, text=True)
        self.assertEqual(result.stdout.split(), ["1", "0", "1", "0", "1", "1", "0.25", *map(str, expected)])

    def test_sender_receiver_roundtrip(self):
        for response, custom in itertools.product((0, 1), repeat=2):
            with self.subTest(response=response, custom=custom):
                self.check_roundtrip("full", response, custom, (response, custom))

    def test_nested_extension_pid_is_not_a_config_flag(self):
        for response, custom in itertools.product((0, 1), repeat=2):
            with self.subTest(response=response, custom=custom):
                self.check_roundtrip("nested", response, custom, (response, custom))

    def test_visible_dump_uses_its_own_prefix_length(self):
        for response, custom in itertools.product((0, 1), repeat=2):
            with self.subTest(response=response, custom=custom):
                self.check_roundtrip("visible", response, custom, (response, custom))

    def test_legacy_config_stops_at_pid_flag(self):
        self.check_roundtrip("legacy", 1, 1, (0, 0))

    def test_missing_custom_content_defaults_to_false(self):
        self.check_roundtrip("response-only", 1, 1, (1, 0))


if __name__ == "__main__":
    unittest.main(verbosity=2)
