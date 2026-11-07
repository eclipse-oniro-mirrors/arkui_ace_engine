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

"""Run the production clipping loop with real RectF; observe JSON builder calls with a spy.

These host checks do not replace FrameNode/device integration tests.
"""

from pathlib import Path
import subprocess
import tempfile
import unittest


REPO = Path(__file__).resolve().parents[3]


class InspectorRegionClippingTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        frame = (REPO / "frameworks/core/components_ng/base/frame_node.cpp").read_text()
        method = frame.index("void FrameNode::DumpInspectorResponseRegions(")
        begin = frame.index("            std::vector<RectF> clipped;", method)
        end_marker = "            effective = std::move(clipped);"
        end = frame.index(end_marker, begin) + len(end_marker)
        clipping = frame[begin:end]
        begin = frame.index("        auto array = JsonUtil::CreateArray();", end)
        end = frame.index("        return array;", begin) + len("        return array;")
        encoding = frame[begin:end]
        source = r'''#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "base/geometry/ng/rect_t.h"
using OHOS::Ace::NG::RectF;
void Clip(std::vector<RectF>& effective, const std::vector<RectF>& clips)
{
''' + clipping + r'''
}
struct JsonSpy {
    size_t count = 0;
    size_t sizeReads = 0;
    bool validKeys = true;
    size_t GetArraySize() { ++sizeReads; return count; }
    void Put(const char* key, const char* value)
    {
        validKeys = validKeys && std::string(key) == std::to_string(count) && std::string(value) == "rect";
        ++count;
    }
};
struct JsonFactory {
    static auto CreateArray() { return std::make_unique<JsonSpy>(); }
};
struct EncodedRegion {
    std::string ToBounds() const { return "rect"; }
};
template<typename JsonUtil>
auto Encode(const std::vector<EncodedRegion>& effective)
{
''' + encoding + r'''
}
std::vector<RectF> ReadRects()
{
    size_t count = 0;
    std::cin >> count;
    std::vector<RectF> result;
    for (size_t i = 0; i < count; ++i) {
        float x, y, width, height;
        std::cin >> x >> y >> width >> height;
        result.emplace_back(x, y, width, height);
    }
    return result;
}
int main(int argc, char** argv)
{
    if (argc > 1) {
        auto array = Encode<JsonFactory>(std::vector<EncodedRegion>(std::stoul(argv[1])));
        std::cout << array->count << ' ' << array->sizeReads << ' ' << array->validKeys << '\n';
        return 0;
    }
    auto effective = ReadRects();
    size_t layers = 0;
    std::cin >> layers;
    for (size_t i = 0; i < layers; ++i) {
        Clip(effective, ReadRects());
    }
    std::cout << effective.size() << '\n' << std::setprecision(9);
    // Keep failures small even if a regression produces millions of rectangles.
    for (size_t i = 0; i < std::min(effective.size(), size_t(32)); ++i) {
        const auto& rect = effective[i];
        std::cout << rect.Left() << ' ' << rect.Top() << ' ' << rect.Width() << ' ' << rect.Height() << '\n';
    }
}
'''
        cls.directory = tempfile.TemporaryDirectory(prefix="inspector-region-test-")
        cls.addClassCleanup(cls.directory.cleanup)
        temporary = Path(cls.directory.name)
        cpp = temporary / "clipping.cpp"
        cpp.write_text(source)
        cls.binary = temporary / "clipping"
        compiler = REPO.parents[2] / "prebuilts/clang/ohos/linux-x86_64/llvm/bin/clang++"
        subprocess.run([str(compiler), "-std=c++17", "-O2", "-I" + str(REPO / "frameworks"),
                        "-I" + str(REPO / "interfaces/inner_api/ace_kit/include"), "-I" + str(REPO),
                        str(cpp), "-o", str(cls.binary)], check=True, capture_output=True, text=True)

    def clip(self, regions, layers):
        def encode(rects):
            return str(len(rects)) + "\n" + "\n".join(" ".join(map(str, rect)) for rect in rects) + "\n"

        data = encode(regions) + str(len(layers)) + "\n" + "".join(encode(layer) for layer in layers)
        result = subprocess.run([str(self.binary)], input=data, text=True, capture_output=True, check=True, timeout=10)
        lines = result.stdout.splitlines()
        return int(lines[0]), [tuple(map(float, line.split())) for line in lines[1:]]

    def test_twenty_layers_keep_one_intersection(self):
        clips = [(-5, -1, 20, 12), (-1, -5, 12, 20)]
        self.assertEqual(self.clip([(0, 0, 10, 10)], [clips] * 20), (1, [(0, 0, 10, 10)]))

    def test_nonconsecutive_duplicates_are_removed(self):
        clips = [(8, 0, 2, 10), (0, 0, 2, 10), (8, 0, 2, 10)]
        self.assertEqual(self.clip([(0, 0, 10, 10)], [clips]), (2, [(0, 0, 2, 10), (8, 0, 2, 10)]))

    def test_partial_overlaps_are_not_merged(self):
        clips = [(0, 0, 8, 10), (2, 0, 8, 10), (0, 0, 8, 10)]
        self.assertEqual(self.clip([(0, 0, 10, 10)], [clips]), (2, [(0, 0, 8, 10), (2, 0, 8, 10)]))

    def test_multiple_layers_preserve_gaps(self):
        clips = [[(0, 0, 5, 10), (15, 0, 5, 10)], [(2, 0, 16, 10), (2, 0, 16, 10)]]
        self.assertEqual(self.clip([(0, 0, 20, 10)], clips), (2, [(2, 0, 3, 10), (15, 0, 3, 10)]))

    def test_near_equal_rectangles_remain_distinct(self):
        shifted = 1 + 2 ** -23
        count, rects = self.clip([(0, 0, 10, 10)], [[(1, 0, 1, 10), (shifted, 0, shifted, 10)]])
        self.assertEqual(count, 2)
        self.assertNotEqual(rects[0], rects[1])

    def test_touching_edges_remain_valid(self):
        self.assertEqual(self.clip([(0, 0, 10, 10)], [[(10, 0, 10, 10)] * 2]), (1, [(10, 0, 0, 10)]))

    def test_empty_intersection(self):
        self.assertEqual(self.clip([(0, 0, 10, 10)], [[(20, 0, 10, 10)]]), (0, []))

    def test_no_ancestors_preserve_original_regions(self):
        regions = [(5, 0, 1, 1), (0, 0, 1, 1), (5, 0, 1, 1)]
        self.assertEqual(self.clip(regions, []), (len(regions), regions))

    def test_json_encoding_uses_sequential_keys_without_size_queries(self):
        for count in (0, 1, 32, 4096):
            with self.subTest(count=count):
                output = subprocess.check_output([str(self.binary), str(count)], text=True)
                self.assertEqual(list(map(int, output.split())), [count, 0, 1])


if __name__ == "__main__":
    unittest.main()
