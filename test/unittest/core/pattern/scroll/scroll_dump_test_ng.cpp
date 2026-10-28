/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstdint>

#include "interfaces/inner_api/ui_session/param_config.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

class ScrollDumpTestNg : public TestNG {
public:
};

/**
 * @tc.name: GetShapeModeDumpInfo_Parameter001
 * @tc.desc: Test ScrollBar GetShapeModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetShapeModeDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->shapeMode_ = ShapeMode::RECT;
    auto json = JsonUtil::Create(true);
    scrollbar->GetShapeModeDumpInfo(json);
    EXPECT_EQ(json->GetString("shapeMode"), "RECT");
}

/**
 * @tc.name: GetShapeModeDumpInfo_Parameter002
 * @tc.desc: Test ScrollBar GetShapeModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetShapeModeDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->shapeMode_ = ShapeMode::ROUND;
    auto json = JsonUtil::Create(true);
    scrollbar->GetShapeModeDumpInfo(json);
    EXPECT_EQ(json->GetString("shapeMode"), "ROUND");
}

/**
 * @tc.name: GetShapeModeDumpInfo_Parameter003
 * @tc.desc: Test ScrollBar GetShapeModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetShapeModeDumpInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->shapeMode_ = ShapeMode::DEFAULT;
    auto json = JsonUtil::Create(true);
    scrollbar->GetShapeModeDumpInfo(json);
    EXPECT_EQ(json->GetString("shapeMode"), "DEFAULT");
}

/**
 * @tc.name: GetShapeModeDumpInfo_Parameter004
 * @tc.desc: Test ScrollBar GetShapeModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetShapeModeDumpInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    int32_t number = 6;
    scrollbar->shapeMode_ = static_cast<ShapeMode>(number);
    auto json = JsonUtil::Create(true);
    scrollbar->GetShapeModeDumpInfo(json);
    EXPECT_NE(json->GetString("shapeMode"), "RECT");
}

/**
 * @tc.name: GetPositionModeDumpInfo_Parameter001
 * @tc.desc: Test ScrollBar GetPositionModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPositionModeDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->positionMode_ = PositionMode::RIGHT;
    Edge padding(4.F);
    scrollbar->padding_ = padding;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPositionModeDumpInfo(json);
    EXPECT_EQ(json->GetString("padding.right"), "4.00px");
}

/**
 * @tc.name: GetPositionModeDumpInfo_Parameter002
 * @tc.desc: Test ScrollBar GetPositionModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPositionModeDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->positionMode_ = PositionMode::LEFT;
    Edge padding(4.F);
    scrollbar->padding_ = padding;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPositionModeDumpInfo(json);
    EXPECT_EQ(json->GetString("padding.left"), "4.00px");
}

/**
 * @tc.name: GetPositionModeDumpInfo_Parameter003
 * @tc.desc: Test ScrollBar GetPositionModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPositionModeDumpInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->positionMode_ = PositionMode::BOTTOM;
    Edge padding(4.F);
    scrollbar->padding_ = padding;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPositionModeDumpInfo(json);
    EXPECT_EQ(json->GetString("padding.bottom"), "4.00px");
}

/**
 * @tc.name: GetPositionModeDumpInfo_Parameter004
 * @tc.desc: Test ScrollBar GetPositionModeDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPositionModeDumpInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    int32_t number = 6;
    scrollbar->positionMode_ = static_cast<PositionMode>(number);
    Edge padding(4.F);
    scrollbar->padding_ = padding;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPositionModeDumpInfo(json);
    EXPECT_NE(json->GetString("padding.left"), "RIGHT");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter001
 * @tc.desc: Test ScrollBar GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetAxisDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->axis_ = Axis::NONE;
    auto json = JsonUtil::Create(true);
    scrollbar->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("axis"), "NONE");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter002
 * @tc.desc: Test ScrollBar GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetAxisDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->axis_ = Axis::VERTICAL;
    auto json = JsonUtil::Create(true);
    scrollbar->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("axis"), "VERTICAL");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter003
 * @tc.desc: Test ScrollBar GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetAxisDumpInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->axis_ = Axis::HORIZONTAL;
    auto json = JsonUtil::Create(true);
    scrollbar->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("axis"), "HORIZONTAL");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter004
 * @tc.desc: Test ScrollBar GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetAxisDumpInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->axis_ = Axis::FREE;
    auto json = JsonUtil::Create(true);
    scrollbar->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("axis"), "FREE");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter005
 * @tc.desc: Test ScrollBar GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetAxisDumpInfo_Parameter005, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    int32_t number = 6;
    scrollbar->axis_ = static_cast<Axis>(number);
    auto json = JsonUtil::Create(true);
    scrollbar->GetAxisDumpInfo(json);
    EXPECT_NE(json->GetString("axis"), "VERTICAL");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter001
 * @tc.desc: Test ScrollBar GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPanDirectionDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<PanRecognizer> panRecognizer = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    scrollbar->panRecognizer_ = panRecognizer;
    scrollbar->panRecognizer_->direction_.type = PanDirection::NONE;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("panDirection"), "NONE");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter002
 * @tc.desc: Test ScrollBar GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPanDirectionDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<PanRecognizer> panRecognizer = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    scrollbar->panRecognizer_ = panRecognizer;
    scrollbar->panRecognizer_->direction_.type = PanDirection::VERTICAL;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("panDirection"), "VERTICAL");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter003
 * @tc.desc: Test ScrollBar GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPanDirectionDumpInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<PanRecognizer> panRecognizer = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    scrollbar->panRecognizer_ = panRecognizer;
    scrollbar->panRecognizer_->direction_.type = PanDirection::HORIZONTAL;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("panDirection"), "HORIZONTAL");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter004
 * @tc.desc: Test ScrollBar GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPanDirectionDumpInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<PanRecognizer> panRecognizer = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    scrollbar->panRecognizer_ = panRecognizer;
    scrollbar->panRecognizer_->direction_.type = PanDirection::ALL;
    auto json = JsonUtil::Create(true);
    scrollbar->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("panDirection"), "FREE");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter005
 * @tc.desc: Test ScrollBar GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, GetPanDirectionDumpInfo_Parameter005, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    auto json = JsonUtil::Create(true);
    scrollbar->GetPanDirectionDumpInfo(json);
    EXPECT_NE(json->GetString("panDirection"), "FREE");
}

/**
 * @tc.name: DumpAdvanceInfo001
 * @tc.desc: Test ScrollBar DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, DumpAdvanceInfo001, TestSize.Level1)
{
    RefPtr<ScrollBar> scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->isReverse_ = true;
    auto json = JsonUtil::Create(true);
    scrollbar->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("isReverse"), "1");
}

/**
 * @tc.name: DumpAdvanceInfoPublicProperties
 * @tc.desc: Public property filters preserve boolean reverse values and exclude diagnostics.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, DumpAdvanceInfoPublicProperties, TestSize.Level1)
{
    auto scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    auto panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    scrollbar->panRecognizer_ = AceType::MakeRefPtr<PanRecognizer>(panGestureOption);
    scrollbar->panRecognizer_->direction_.type = PanDirection::VERTICAL;
    for (bool reverse : { false, true }) {
        scrollbar->SetReverse(reverse);
        const auto axis = reverse ? Axis::HORIZONTAL : Axis::VERTICAL;
        scrollbar->SetAxis(axis);
        for (bool visual : { false, true }) {
            for (bool functional : { false, true }) {
                ParamConfig config;
                config.dumpMode = TreeDumpMode::PUBLIC_TREE;
                config.visualProperties = visual;
                config.functionalProperties = functional;
                auto json = JsonUtil::Create(true);
                scrollbar->DumpAdvanceInfo(json, config);
                EXPECT_EQ(json->Contains("activeRect"), visual);
                EXPECT_EQ(json->Contains("scrollBarHeight"), visual);
                EXPECT_FALSE(json->Contains("scrollBarMargin"));
                EXPECT_FALSE(json->Contains("axis"));
                EXPECT_FALSE(json->Contains("panDirection"));
                EXPECT_FALSE(json->Contains("innerScrollBarLayoutInfos"));
                EXPECT_EQ(json->Contains("isReverse"), functional);
                if (functional) {
                    EXPECT_TRUE(json->GetValue("isReverse")->IsBool());
                    EXPECT_EQ(json->GetBool("isReverse"), reverse);
                }
            }
        }
        auto legacy = JsonUtil::Create(true);
        scrollbar->DumpAdvanceInfo(legacy);
        EXPECT_EQ(legacy->GetString("isReverse"), reverse ? "1" : "0");
        EXPECT_EQ(legacy->GetString("axis"), axis == Axis::HORIZONTAL ? "HORIZONTAL" : "VERTICAL");
        EXPECT_TRUE(legacy->Contains("panDirection"));
        EXPECT_TRUE(legacy->Contains("scrollBarMargin"));
        EXPECT_TRUE(legacy->Contains("innerScrollBarLayoutInfos"));
    }
}

/**
 * @tc.name: DumpAdvanceInfoPublicPreservesHostProperties
 * @tc.desc: Public scrollbar output retains host height and margin without duplicate keys.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollDumpTestNg, DumpAdvanceInfoPublicPreservesHostProperties, TestSize.Level1)
{
    auto scrollbar = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    scrollbar->scrollBarMargin_ = ScrollBarMargin();
    ParamConfig config;
    config.dumpMode = TreeDumpMode::PUBLIC_TREE;
    config.visualProperties = true;
    auto json = JsonUtil::Create(true);
    scrollbar->DumpAdvanceInfo(json, config);
    EXPECT_TRUE(json->Contains("scrollBarMargin"));

    json = JsonUtil::Create(true);
    json->Put("scrollBarHeight", "hostHeight");
    json->Put("scrollBarMargin", "hostMargin");
    scrollbar->DumpAdvanceInfo(json, config);
    EXPECT_EQ(json->GetString("scrollBarHeight"), "hostHeight");
    EXPECT_EQ(json->GetString("scrollBarMargin"), "hostMargin");
    int heightCount = 0;
    int marginCount = 0;
    for (auto item = json->GetChild(); item && item->IsValid(); item = item->GetNext()) {
        heightCount += item->GetKey() == "scrollBarHeight";
        marginCount += item->GetKey() == "scrollBarMargin";
    }
    EXPECT_EQ(heightCount, 1);
    EXPECT_EQ(marginCount, 1);
}
} // namespace OHOS::Ace::NG
