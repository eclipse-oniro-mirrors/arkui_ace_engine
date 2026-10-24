/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <limits>

#include "test/unittest/core/pattern/test_ng.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_pattern.h"
#include "core/components_ng/syntax/static/detached_free_root_proxy_node.h"
#include "core/interfaces/native/ani/safe_area_inset_ani_modifier.h"
#include "core/components_ng/syntax/if_else_node.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t INVALID_ENUM_VALUE = 999;
constexpr size_t TOP_INDEX = SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP);
constexpr size_t END_INDEX = SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::END);
constexpr size_t BOTTOM_INDEX = SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::BOTTOM);
constexpr size_t START_INDEX = SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::START);
} // namespace

class SafeAreaInsetAniModifierTest : public TestNG {};

HWTEST_F(SafeAreaInsetAniModifierTest, SnapshotReplaceReset, TestSize.Level1)
{
    auto host = FrameNode::CreateFrameNode("host", 2100, AceType::MakeRefPtr<Pattern>());
    auto business = FrameNode::CreateFrameNode("business", 2101, AceType::MakeRefPtr<Pattern>());
    business->MountToParent(host);
    std::array<RefPtr<FrameNode>, SAFE_AREA_INSET_EDGE_COUNT> roots;
    std::array<ArkUIAniSafeAreaInsetEdge, SAFE_AREA_INSET_EDGE_COUNT> edges;
    for (size_t i = 0; i < roots.size(); ++i) {
        roots[i] = FrameNode::CreateFrameNode("content", 2110 + i, AceType::MakeRefPtr<Pattern>());
        edges[i].content = reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(roots[i]));
    }
    auto node = reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(host));
    SetSafeAreaInsetStatic(node, edges.data(), edges.size());
    ASSERT_NE(host->GetSafeAreaInsetNode(), nullptr);
    auto pattern = host->GetSafeAreaInsetNode()->GetPattern<SafeAreaInsetPattern>();
    for (size_t i = 0; i < roots.size(); ++i) {
        auto proxy = pattern->GetEdgeContent(static_cast<SafeAreaInsetEdge>(i));
        ASSERT_NE(AceType::DynamicCast<DetachedFreeRootProxyNode>(proxy), nullptr);
        EXPECT_EQ(proxy->GetFirstChild(), roots[i]);
    }
    EXPECT_EQ(host->GetChildren().size(), 1);
    auto oldCarrier = host->GetSafeAreaInsetNode();
    SetSafeAreaInsetStatic(node, edges.data(), edges.size() - 1);
    EXPECT_EQ(host->GetSafeAreaInsetNode(), oldCarrier);
    auto oldTopProxy = pattern->GetEdgeContent(SafeAreaInsetEdge::TOP);
    auto customRoot = IfElseNode::GetOrCreateIfElseNode(2129);
    auto nested = FrameNode::CreateFrameNode("nested", 2139, AceType::MakeRefPtr<Pattern>());
    nested->MountToParent(customRoot);
    std::array<ArkUIAniSafeAreaInsetEdge, SAFE_AREA_INSET_EDGE_COUNT> replacement;
    replacement[BOTTOM_INDEX].content = reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(customRoot));
    SetSafeAreaInsetStatic(node, replacement.data(), replacement.size());
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP), nullptr);
    EXPECT_EQ(oldTopProxy->GetParent(), nullptr);
    auto newProxy = pattern->GetEdgeContent(SafeAreaInsetEdge::BOTTOM);
    ASSERT_NE(newProxy, nullptr);
    EXPECT_EQ(newProxy->GetFirstChild(), customRoot);
    EXPECT_EQ(customRoot->FrameCount(), 1);
    std::array<ArkUIAniSafeAreaInsetEdge, SAFE_AREA_INSET_EDGE_COUNT> empty;
    SetSafeAreaInsetStatic(node, empty.data(), empty.size());
    EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
    EXPECT_EQ(oldCarrier->GetParent(), nullptr);
    EXPECT_EQ(host->GetChildren().front(), business);
}

HWTEST_F(SafeAreaInsetAniModifierTest, OptionsValidationAndHostIsolation, TestSize.Level1)
{
    auto host = FrameNode::CreateFrameNode("host", 2120, AceType::MakeRefPtr<Pattern>());
    auto other = FrameNode::CreateFrameNode("other", 2121, AceType::MakeRefPtr<Pattern>());
    const std::array<double, 5> values { 6.0, -1.0, std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN(), 0.0 };
    for (double value : values) {
        auto root = FrameNode::CreateFrameNode("content", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<Pattern>());
        std::array<ArkUIAniSafeAreaInsetEdge, SAFE_AREA_INSET_EDGE_COUNT> edges;
        edges[END_INDEX] = { reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(root)), value, static_cast<int32_t>(DimensionUnit::VP), INVALID_ENUM_VALUE };
        SetSafeAreaInsetStatic(reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(host)), edges.data(), edges.size());
        const auto& options = host->GetLayoutProperty()->GetSafeAreaInset();
        ASSERT_TRUE(options.has_value());
        EXPECT_FALSE(options->edges[TOP_INDEX].has_value());
        ASSERT_TRUE(options->edges[END_INDEX].has_value());
        EXPECT_DOUBLE_EQ(options->edges[END_INDEX]->spacing.GetDimension().Value(), value == 6.0 ? 6.0 : 0.0);
        EXPECT_EQ(options->edges[END_INDEX]->alignment, SafeAreaInsetAlignment::CENTER);
        EXPECT_EQ(other->GetSafeAreaInsetNode(), nullptr);
        ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    }
}

HWTEST_F(SafeAreaInsetAniModifierTest, UnitsAlignmentAndEmptyEdges, TestSize.Level1)
{
    auto host = FrameNode::CreateFrameNode("host", 2130, AceType::MakeRefPtr<Pattern>());
    for (auto unit : { DimensionUnit::PX, DimensionUnit::VP, DimensionUnit::FP,
        DimensionUnit::PERCENT, DimensionUnit::LPX, static_cast<DimensionUnit>(INVALID_ENUM_VALUE) }) {
        auto root = FrameNode::CreateFrameNode("content", ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<Pattern>());
        std::array<ArkUIAniSafeAreaInsetEdge, SAFE_AREA_INSET_EDGE_COUNT> edges;
        edges[START_INDEX] = { reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(root)), 7.0, static_cast<int32_t>(unit),
            static_cast<int32_t>(SafeAreaInsetAlignment::END) };
        auto node = reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(host));
        SetSafeAreaInsetStatic(node, edges.data(), edges.size());
        const auto& options = host->GetLayoutProperty()->GetSafeAreaInset();
        ASSERT_TRUE(options.has_value());
        const auto& edge = options->edges[START_INDEX];
        ASSERT_TRUE(edge.has_value());
        bool valid = unit == DimensionUnit::PX || unit == DimensionUnit::VP ||
            unit == DimensionUnit::FP || unit == DimensionUnit::LPX;
        EXPECT_DOUBLE_EQ(edge->spacing.GetDimension().Value(), valid ? 7.0 : 0.0);
        EXPECT_EQ(edge->alignment, SafeAreaInsetAlignment::END);
        edges[START_INDEX].content = nullptr;
        SetSafeAreaInsetStatic(node, edges.data(), edges.size());
        EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
    }
}
} // namespace OHOS::Ace::NG
