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
#include <vector>

#include "gtest/gtest.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/list/list_model_ng.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/scroll_model_ng.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/property/measure_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float HOST_WIDTH = 300.0f;
constexpr float HOST_HEIGHT = 200.0f;
constexpr float SPACING = 8.0f;

struct InsetItem {
    SafeAreaInsetEdge edge;
    float main;
    float cross;
};

SafeAreaInsetEdgeOptions MakeEdgeOptions(float spacing = SPACING)
{
    return {
        .spacing = CalcLength(spacing, DimensionUnit::PX)
    };
}

SafeAreaInsetOptions BuildOptions(const std::vector<InsetItem>& items)
{
    SafeAreaInsetOptions options;
    for (const auto& item : items) {
        options.GetEdge(item.edge) = MakeEdgeOptions();
    }
    return options;
}

RefPtr<FrameNode> MakeContent(const char* tag, float width, float height)
{
    auto node = FrameNode::CreateFrameNode(
        tag, ViewStackProcessor::GetInstance()->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
    CHECK_NULL_RETURN(node, nullptr);
    node->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(width, DimensionUnit::PX), CalcLength(height, DimensionUnit::PX)));
    return node;
}

SafeAreaInsetContentArray BuildContents(const std::vector<InsetItem>& items)
{
    SafeAreaInsetContentArray contents;
    for (const auto& item : items) {
        bool horizontal = item.edge == SafeAreaInsetEdge::START || item.edge == SafeAreaInsetEdge::END;
        float width = horizontal ? item.main : item.cross;
        float height = horizontal ? item.cross : item.main;
        contents[SafeAreaInsetOptions::ToIndex(item.edge)] = MakeContent("insetContent", width, height);
    }
    return contents;
}

void SetHostSize()
{
    ViewAbstract::SetWidth(CalcLength(HOST_WIDTH, DimensionUnit::PX));
    ViewAbstract::SetHeight(CalcLength(HOST_HEIGHT, DimensionUnit::PX));
}

void SetRawSafeAreaPadding(float value)
{
    PaddingProperty safeAreaPadding;
    safeAreaPadding.SetEdges(CalcLength(value, DimensionUnit::PX));
    ViewAbstract::SetSafeAreaPadding(safeAreaPadding);
}

void ApplyInset(const std::vector<InsetItem>& items)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    ViewAbstract::SetSafeAreaInset(frameNode, BuildOptions(items), BuildContents(items));
}

RefPtr<FrameNode> CreateScrollHost(const std::function<void(ScrollModelNG&)>& callback)
{
    ScrollModelNG model;
    model.Create();
    if (callback) {
        callback(model);
    }
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
    ViewStackProcessor::GetInstance()->PopContainer();
    return AceType::DynamicCast<FrameNode>(element);
}

RefPtr<FrameNode> CreateListHost(const std::function<void(ListModelNG&)>& callback)
{
    ListModelNG model;
    model.Create();
    if (callback) {
        callback(model);
    }
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
    ViewStackProcessor::GetInstance()->PopContainer();
    return AceType::DynamicCast<FrameNode>(element);
}

void ExpectNoExtraContentOffset(const RefPtr<ScrollablePattern>& pattern)
{
    ASSERT_NE(pattern, nullptr);
    EXPECT_FLOAT_EQ(pattern->GetContentStartOffset(), 0.0f);
    EXPECT_FLOAT_EQ(pattern->GetContentEndOffset(), 0.0f);
}
} // namespace

class ScrollableSafeAreaInsetTestNg : public TestNG {};

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid001
 * @tc.desc: Scrollbar avoid offset consumes top inset contribution when only inset is configured.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid001, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG&) {
        SetHostSize();
        ApplyInset({
            { SafeAreaInsetEdge::TOP, 20.0f, 100.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 28.0f); // content 20 + spacing 8
    EXPECT_FLOAT_EQ(avoid.second, 0.0f);
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid002
 * @tc.desc: Scrollbar avoid offset consumes top and bottom inset contributions independently.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid002, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG&) {
        SetHostSize();
        ApplyInset({
            { SafeAreaInsetEdge::TOP, 20.0f, 100.0f },
            { SafeAreaInsetEdge::BOTTOM, 30.0f, 100.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 28.0f);
    EXPECT_FLOAT_EQ(avoid.second, 38.0f); // content 30 + spacing 8
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid003
 * @tc.desc: Same-edge inset larger than raw safeAreaPadding takes max, not sum.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid003, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG&) {
        SetHostSize();
        SetRawSafeAreaPadding(10.0f);
        ApplyInset({
            { SafeAreaInsetEdge::TOP, 20.0f, 100.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 28.0f); // max(10, 28), not 38
    EXPECT_FLOAT_EQ(avoid.second, 0.0f);
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid004
 * @tc.desc: Same-edge raw safeAreaPadding larger than inset contribution keeps its value.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid004, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG&) {
        SetHostSize();
        SetRawSafeAreaPadding(40.0f);
        ApplyInset({
            { SafeAreaInsetEdge::TOP, 20.0f, 100.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 40.0f); // max(40, 28)
    EXPECT_FLOAT_EQ(avoid.second, 0.0f);
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid005
 * @tc.desc: Without inset the raw safeAreaPadding path keeps legacy avoid offset values.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid005, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG&) {
        SetHostSize();
        SetRawSafeAreaPadding(10.0f);
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 10.0f);
    EXPECT_FLOAT_EQ(avoid.second, 0.0f);
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid006
 * @tc.desc: Reset removes only the inset source; independent raw safeAreaPadding keeps working.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid006, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG&) {
        SetHostSize();
        SetRawSafeAreaPadding(6.0f);
        ApplyInset({
            { SafeAreaInsetEdge::TOP, 20.0f, 100.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 28.0f);
    EXPECT_FLOAT_EQ(avoid.second, 0.0f);

    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    FlushUITasks(host);
    avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 6.0f);
    EXPECT_FLOAT_EQ(avoid.second, 0.0f);
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid007
 * @tc.desc: Horizontal LTR scroll maps START/END insets to physical left/right avoid offsets.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid007, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG& model) {
        SetHostSize();
        model.SetAxis(Axis::HORIZONTAL);
        ApplyInset({
            { SafeAreaInsetEdge::START, 20.0f, 60.0f },
            { SafeAreaInsetEdge::END, 60.0f, 60.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    EXPECT_FALSE(pattern->IsReverse());
    auto effective = host->GetLayoutProperty()->GetOrCreateSafeAreaPadding(true);
    EXPECT_FLOAT_EQ(effective.left.value_or(0.0f), 28.0f);
    EXPECT_FLOAT_EQ(effective.right.value_or(0.0f), 68.0f);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 28.0f);  // physical left
    EXPECT_FLOAT_EQ(avoid.second, 68.0f); // physical right
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid008
 * @tc.desc: Horizontal RTL scroll mirrors START/END contributions to physical sides.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid008, TestSize.Level1)
{
    auto host = CreateScrollHost([](ScrollModelNG& model) {
        SetHostSize();
        model.SetAxis(Axis::HORIZONTAL);
        ViewAbstract::SetLayoutDirection(TextDirection::RTL);
        ApplyInset({
            { SafeAreaInsetEdge::START, 20.0f, 60.0f },
            { SafeAreaInsetEdge::END, 60.0f, 60.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ScrollPattern>();
    ExpectNoExtraContentOffset(pattern);
    EXPECT_TRUE(pattern->IsReverse());
    auto effective = host->GetLayoutProperty()->GetOrCreateSafeAreaPadding(true);
    EXPECT_FLOAT_EQ(effective.left.value_or(0.0f), 68.0f);  // END mirrored to left
    EXPECT_FLOAT_EQ(effective.right.value_or(0.0f), 28.0f); // START mirrored to right
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 68.0f);  // physical left
    EXPECT_FLOAT_EQ(avoid.second, 28.0f); // physical right
}

/**
 * @tc.name: ScrollableSafeAreaInsetAvoid009
 * @tc.desc: List as derived scrollable inherits the effective-value scrollbar avoidance.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableSafeAreaInsetTestNg, ScrollableSafeAreaInsetAvoid009, TestSize.Level1)
{
    auto host = CreateListHost([](ListModelNG&) {
        SetHostSize();
        ApplyInset({
            { SafeAreaInsetEdge::TOP, 20.0f, 100.0f }
        });
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    auto pattern = host->GetPattern<ListPattern>();
    ExpectNoExtraContentOffset(pattern);
    auto avoid = pattern->GetAutoAdjustAvoidOffset();
    EXPECT_FLOAT_EQ(avoid.first, 28.0f);
    EXPECT_FLOAT_EQ(avoid.second, 0.0f);
}
} // namespace OHOS::Ace::NG
