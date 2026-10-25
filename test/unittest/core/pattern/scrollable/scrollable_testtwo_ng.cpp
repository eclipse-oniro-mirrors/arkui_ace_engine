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

#include "interfaces/inner_api/ui_session/param_config.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_event_hub.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_layout_property.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_static.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

class ScrollableTestNg : public TestNG {
public:
};

namespace {
void ExpectUniqueDumpProperty(const JsonValue& json, const char* key, const std::string& expected)
{
    SCOPED_TRACE(key);
    EXPECT_EQ(json.GetString(key), expected);
    int count = 0;
    for (auto item = json.GetChild(); item && item->IsValid(); item = item->GetNext()) {
        count += item->GetKey() == key;
    }
    EXPECT_EQ(count, 1);
}
} // namespace

/**
 * @tc.name: GetEdgeEffectDumpInfo001
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo001, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->edgeEffect_ = EdgeEffect::NONE;
    scrollablePattern->GetEdgeEffectDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "edgeEffect: NONE\n");
}

/**
 * @tc.name: GetEdgeEffectDumpInfo002
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo002, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->edgeEffect_ = EdgeEffect::SPRING;
    scrollablePattern->GetEdgeEffectDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "edgeEffect: SPRING\n");
}

/**
 * @tc.name: GetEdgeEffectDumpInfo003
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo003, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->edgeEffect_ = EdgeEffect::FADE;
    scrollablePattern->GetEdgeEffectDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "edgeEffect: FADE\n");
}

/**
 * @tc.name: GetEdgeEffectDumpInfo004
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo004, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    int32_t number = 6;
    scrollablePattern->edgeEffect_ = static_cast<EdgeEffect>(number);
    scrollablePattern->GetEdgeEffectDumpInfo();
    EXPECT_NE(DumpLog::GetInstance().description_[0], "edgeEffect: SPRING\n");
}

/**
 * @tc.name: GetAxisDumpInfo001
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo001, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    scrollablePattern->GetAxisDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "Axis: NONE\n");
}

/**
 * @tc.name: GetAxisDumpInfo002
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo002, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::VERTICAL;
    scrollablePattern->GetAxisDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "Axis: VERTICAL\n");
}

/**
 * @tc.name: GetAxisDumpInfo003
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo003, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::HORIZONTAL;
    scrollablePattern->GetAxisDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "Axis: HORIZONTAL\n");
}

/**
 * @tc.name: GetAxisDumpInfo004
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo004, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::FREE;
    scrollablePattern->GetAxisDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "Axis: FREE\n");
}

/**
 * @tc.name: GetAxisDumpInfo005
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo005, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    int32_t number = 6;
    scrollablePattern->axis_ = static_cast<Axis>(number);
    scrollablePattern->GetAxisDumpInfo();
    EXPECT_NE(DumpLog::GetInstance().description_[0], "Axis: VERTICAL\n");
}

/**
 * @tc.name: GetPanDirectionDumpInfo001
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo001, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::NONE;
    scrollablePattern->GetPanDirectionDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "ScrollablePanDirection:NONE\n");
}

/**
 * @tc.name: GetPanDirectionDumpInfo002
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo002, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::VERTICAL;
    scrollablePattern->GetPanDirectionDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "ScrollablePanDirection:VERTICAL\n");
}

/**
 * @tc.name: GetPanDirectionDumpInfo003
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo003, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::HORIZONTAL;
    scrollablePattern->GetPanDirectionDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "ScrollablePanDirection:HORIZONTAL\n");
}

/**
 * @tc.name: GetPanDirectionDumpInfo004
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo004, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::ALL;
    scrollablePattern->GetPanDirectionDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "ScrollablePanDirection:FREE\n");
}

/**
 * @tc.name: GetPaintPropertyDumpInfo001
 * @tc.desc: Test ScrollablePattern GetPaintPropertyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPaintPropertyDumpInfo001, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<PaintProperty> paintProperty = AceType::MakeRefPtr<ScrollablePaintProperty>();
    frameNode->paintProperty_ = paintProperty;
    scrollablePattern->frameNode_ = frameNode;
    scrollablePattern->GetPaintPropertyDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "innerScrollBarState: OFF\n");
}

/**
 * @tc.name: GetPaintPropertyDumpInfo002
 * @tc.desc: Test ScrollablePattern GetPaintPropertyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPaintPropertyDumpInfo002, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<PaintProperty> paintProperty = AceType::MakeRefPtr<ScrollablePaintProperty>();
    frameNode->paintProperty_ = paintProperty;
    scrollablePattern->frameNode_ = frameNode;
    scrollablePattern->GetPaintPropertyDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_.size(), 8);
    EXPECT_EQ(DumpLog::GetInstance().description_[0], "innerScrollBarState: OFF\n");
    EXPECT_EQ(DumpLog::GetInstance().description_[1], "scrollBarHeight: None\n");
    EXPECT_EQ(DumpLog::GetInstance().description_[2], "scrollBarWidth: None\n");
}

/**
 * @tc.name: GetEventDumpInfo001
 * @tc.desc: Test ScrollBarPattern GetEventDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEventDumpInfo001, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<ScrollEventHub> eventHub = AceType::MakeRefPtr<ScrollEventHub>();
    eventHub->onScrollStartEvent_ = []() {};
    eventHub->onScrollStopEvent_ = []() {};
    frameNode->eventHub_ = eventHub;
    scrollablePattern->frameNode_ = frameNode;
    scrollablePattern->GetEventDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[4], "hasOnWillScroll: false\n");
}

/**
 * @tc.name: GetEventDumpInfo002
 * @tc.desc: Test ScrollBarPattern GetEventDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEventDumpInfo002, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<ScrollableEventHub> eventHub = AceType::MakeRefPtr<ScrollableEventHub>();
    eventHub->onScrollStartEvent_ = []() {};
    eventHub->onScrollStopEvent_ = []() {};
    frameNode->eventHub_ = eventHub;
    scrollablePattern->frameNode_ = frameNode;
    scrollablePattern->GetEventDumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_[4], "hasOnWillScroll: false\n");
}

/**
 * @tc.name: GetPaintPropertyDumpInfo_Parameter001
 * @tc.desc: Test ScrollablePattern GetPaintPropertyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPaintPropertyDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<PaintProperty> paintProperty = AceType::MakeRefPtr<ScrollablePaintProperty>();
    frameNode->paintProperty_ = paintProperty;
    scrollablePattern->frameNode_ = frameNode;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetPaintPropertyDumpInfo(json);
    EXPECT_EQ(json->GetString("innerScrollBarState"), "OFF");
}

/**
 * @tc.name: GetPaintPropertyDumpInfo_Parameter002
 * @tc.desc: Test ScrollablePattern GetPaintPropertyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPaintPropertyDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetPaintPropertyDumpInfo(json);
    EXPECT_NE(json->GetString("innerScrollBarState"), "OFF");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter001
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("Axis"), "NONE");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter002
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::VERTICAL;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("Axis"), "VERTICAL");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter003
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::HORIZONTAL;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("Axis"), "HORIZONTAL");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter004
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::FREE;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetAxisDumpInfo(json);
    EXPECT_EQ(json->GetString("Axis"), "FREE");
}

/**
 * @tc.name: GetAxisDumpInfo_Parameter005
 * @tc.desc: Test ScrollablePattern GetAxisDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetAxisDumpInfo_Parameter005, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    int32_t number = 6;
    scrollablePattern->axis_ = static_cast<Axis>(number);
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetAxisDumpInfo(json);
    EXPECT_NE(json->GetString("Axis"), "VERTICAL");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter001
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::NONE;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("ScrollablePanDirection"), "NONE");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter002
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::VERTICAL;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("ScrollablePanDirection"), "VERTICAL");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter003
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::HORIZONTAL;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("ScrollablePanDirection"), "HORIZONTAL");
}

/**
 * @tc.name: GetPanDirectionDumpInfo_Parameter004
 * @tc.desc: Test ScrollablePattern GetPanDirectionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetPanDirectionDumpInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->axis_ = Axis::NONE;
    RefPtr<ScrollableEvent> scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(Axis::NONE);
    scrollablePattern->scrollableEvent_ = scrollableEvent;
    ScrollPositionCallback callback = [](double value, int32_t source) {
        value = 2.F;
        source = 1;
        return false;
    };
    RefPtr<Scrollable> scrollable = AceType::MakeRefPtr<Scrollable>(callback, Axis::NONE);
    scrollablePattern->scrollableEvent_->scrollable_ = scrollable;
    RefPtr<PanGestureOption> panGestureOption = AceType::MakeRefPtr<PanGestureOption>();
    RefPtr<NG::PanRecognizer> panRecognizerNG = AceType::MakeRefPtr<NG::PanRecognizer>(panGestureOption);
    scrollable->panRecognizerNG_ = panRecognizerNG;
    panRecognizerNG->direction_.type = PanDirection::ALL;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetPanDirectionDumpInfo(json);
    EXPECT_EQ(json->GetString("ScrollablePanDirection"), "FREE");
}

/**
 * @tc.name: GetEventDumpInfo_Parameter001
 * @tc.desc: Test ScrollBarPattern GetEventDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEventDumpInfo_Parameter001, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<ScrollEventHub> eventHub = AceType::MakeRefPtr<ScrollEventHub>();
    eventHub->onScrollStartEvent_ = []() {};
    eventHub->onScrollStopEvent_ = []() {};
    frameNode->eventHub_ = eventHub;
    scrollablePattern->frameNode_ = frameNode;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetEventDumpInfo(json);
    EXPECT_EQ(json->GetString("hasOnWillScroll"), "false");
}

/**
 * @tc.name: GetEventDumpInfo_Parameter002
 * @tc.desc: Test ScrollBarPattern GetEventDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEventDumpInfo_Parameter002, TestSize.Level1)
{
    DumpLog::GetInstance().description_.clear();
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<ScrollableEventHub> eventHub = AceType::MakeRefPtr<ScrollableEventHub>();
    eventHub->onScrollStartEvent_ = []() {};
    eventHub->onScrollStopEvent_ = []() {};
    frameNode->eventHub_ = eventHub;
    scrollablePattern->frameNode_ = frameNode;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetEventDumpInfo(json);
    EXPECT_EQ(json->GetString("hasOnWillScroll"), "false");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter001
 * @tc.desc: Test ScrollBarPattern DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    frameNode->nodeId_ = 5;
    frameNode->tag_ = "tag";
    scrollablePattern->frameNode_ = frameNode;
    scrollablePattern->parent_ = scrollablePattern;
    auto json = JsonUtil::Create(true);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("nestedScrollParent"), "");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter002
 * @tc.desc: Test ScrollBarPattern DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    scrollablePattern->parent_ = scrollablePattern;
    auto json = JsonUtil::Create(true);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("nestedScrollParent"), "null");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter003
 * @tc.desc: Test ScrollBarPattern DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto json = JsonUtil::Create(true);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("nestedScrollParent"), "null");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter004
 * @tc.desc: Test ScrollBarPattern DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto json = JsonUtil::Create(true);
    auto scrollBar = AceType::MakeRefPtr<ScrollBar>();
    scrollablePattern->scrollBar_ = scrollBar;
    scrollablePattern->scrollBar_->activeRect_ = Rect(2, 6, 5, 10);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("activeRect"), "Rect (2.00, 6.00) - [5.00 x 10.00]");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter005
 * @tc.desc: Test ScrollBarPattern DumpAdvanceInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter005, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto json = JsonUtil::Create(true);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("inner ScrollBar"), "null");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter006
 * @tc.desc: Test ScrollablePattern DumpAdvanceInfo with fadingEdge properties
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter006, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<PaintProperty> paintProperty = AceType::MakeRefPtr<ScrollablePaintProperty>();
    frameNode->paintProperty_ = paintProperty;
    scrollablePattern->frameNode_ = frameNode;
    auto json = JsonUtil::Create(true);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("fadingEdge"), "None");
    EXPECT_EQ(json->GetString("fadingEdgeLength"), "None");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter007
 * @tc.desc: Test ScrollablePattern DumpAdvanceInfo with fadingEdge set to true
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter007, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<ScrollablePaintProperty> paintProperty = AceType::MakeRefPtr<ScrollablePaintProperty>();
    paintProperty->UpdateFadingEdge(true);
    frameNode->paintProperty_ = paintProperty;
    scrollablePattern->frameNode_ = frameNode;
    auto json = JsonUtil::Create(true);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("fadingEdge"), "true");
}

/**
 * @tc.name: DumpAdvanceInfo_Parameter008
 * @tc.desc: Test ScrollablePattern DumpAdvanceInfo with fadingEdgeLength set
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpAdvanceInfo_Parameter008, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    ASSERT_NE(scrollablePattern, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, scrollablePattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<ScrollablePaintProperty> paintProperty = AceType::MakeRefPtr<ScrollablePaintProperty>();
    paintProperty->UpdateFadingEdgeLength(20.0_vp);
    frameNode->paintProperty_ = paintProperty;
    scrollablePattern->frameNode_ = frameNode;
    auto json = JsonUtil::Create(true);
    scrollablePattern->DumpAdvanceInfo(json);
    EXPECT_EQ(json->GetString("fadingEdgeLength"), "20.00vp");
}

/**
 * @tc.name: GetEdgeEffectDumpInfo_Parameter001
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo_Parameter001, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->edgeEffect_ = EdgeEffect::NONE;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetEdgeEffectDumpInfo(json);
    EXPECT_EQ(json->GetString("edgeEffect"), "NONE");
}

/**
 * @tc.name: GetEdgeEffectDumpInfo_Parameter002
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo_Parameter002, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->edgeEffect_ = EdgeEffect::SPRING;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetEdgeEffectDumpInfo(json);
    EXPECT_EQ(json->GetString("edgeEffect"), "SPRING");
}

/**
 * @tc.name: GetEdgeEffectDumpInfo_Parameter003
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo_Parameter003, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    scrollablePattern->edgeEffect_ = EdgeEffect::FADE;
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetEdgeEffectDumpInfo(json);
    EXPECT_EQ(json->GetString("edgeEffect"), "FADE");
}

/**
 * @tc.name: GetEdgeEffectDumpInfo_Parameter004
 * @tc.desc: Test ScrollablePattern GetEdgeEffectDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, GetEdgeEffectDumpInfo_Parameter004, TestSize.Level1)
{
    RefPtr<ScrollablePattern> scrollablePattern = AceType::MakeRefPtr<ListPattern>();
    int32_t number = 6;
    scrollablePattern->edgeEffect_ = static_cast<EdgeEffect>(number);
    auto json = JsonUtil::Create(true);
    scrollablePattern->GetEdgeEffectDumpInfo(json);
    EXPECT_NE(json->GetString("edgeEffect"), "SPRING");
}

/**
 * @tc.name: ScrollableModelStatic_SetEdgeEffect001
 * @tc.desc: Test ScrollableModelStatic SetEdgeEffect
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableModelStatic_SetEdgeEffect001, TestSize.Level1)
{
    auto frameNodeList = FrameNode::GetOrCreateFrameNode(V2::LIST_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ListPattern>(); });
    ASSERT_NE(frameNodeList, nullptr);
    ScrollableModelStatic::SetEdgeEffect(AceType::RawPtr(frameNodeList), std::nullopt, false, EffectEdge::ALL);
    auto listEffect = ScrollableModelNG::GetEdgeEffect(AceType::RawPtr(frameNodeList));
    EXPECT_EQ(listEffect, 0);
    auto frameNodeOther = FrameNode::GetOrCreateFrameNode(V2::SCROLL_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ScrollPattern>(); });
    ASSERT_NE(frameNodeOther, nullptr);
    ScrollableModelStatic::SetEdgeEffect(AceType::RawPtr(frameNodeOther), std::nullopt, false, EffectEdge::ALL);
    auto scrollEffect = ScrollableModelNG::GetEdgeEffect(AceType::RawPtr(frameNodeOther));
    EXPECT_EQ(scrollEffect, 2);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone001
 * @tc.desc: Test ScrollableLayoutProperty Clone with all properties set
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone001, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set all properties
     * @tc.expected: Properties are set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentStartOffset(10.5f);
    layoutProperty->UpdateContentEndOffset(20.5f);
    layoutProperty->UpdateSupportLazyLoadingEmptyBranch(true);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful with valid object creation
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify all cloned properties match original values
     * @tc.expected: All cloned properties maintain correct values
     */
    EXPECT_TRUE(clonedProperty->HasContentStartOffset());
    EXPECT_TRUE(clonedProperty->HasContentEndOffset());
    EXPECT_TRUE(clonedProperty->HasSupportLazyLoadingEmptyBranch());
    EXPECT_EQ(clonedProperty->GetContentStartOffsetValue(), 10.5f);
    EXPECT_EQ(clonedProperty->GetContentEndOffsetValue(), 20.5f);
    EXPECT_EQ(clonedProperty->GetSupportLazyLoadingEmptyBranchValue(), true);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone002
 * @tc.desc: Test ScrollableLayoutProperty Clone with partial properties set
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone002, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set partial properties
     * @tc.expected: Properties are set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentStartOffset(15.0f);
    // ContentEndOffset not set
    layoutProperty->UpdateSupportLazyLoadingEmptyBranch(false);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful with valid object creation
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify cloned properties match original values
     * @tc.expected: Set properties are cloned, unset properties remain unset
     */
    EXPECT_TRUE(clonedProperty->HasContentStartOffset());
    EXPECT_FALSE(clonedProperty->HasContentEndOffset());
    EXPECT_TRUE(clonedProperty->HasSupportLazyLoadingEmptyBranch());
    EXPECT_EQ(clonedProperty->GetContentStartOffsetValue(), 15.0f);
    EXPECT_EQ(clonedProperty->GetSupportLazyLoadingEmptyBranchValue(), false);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone003
 * @tc.desc: Test ScrollableLayoutProperty Clone with no properties set
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone003, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty without setting any properties
     * @tc.expected: Object created successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful with valid object creation
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify all cloned properties are unset
     * @tc.expected: No properties should have values
     */
    EXPECT_FALSE(clonedProperty->HasContentStartOffset());
    EXPECT_FALSE(clonedProperty->HasContentEndOffset());
    EXPECT_FALSE(clonedProperty->HasSupportLazyLoadingEmptyBranch());
}

/**
 * @tc.name: ScrollableLayoutPropertyClone004
 * @tc.desc: Test ScrollableLayoutProperty Clone independence
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone004, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set properties
     * @tc.expected: Properties are set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentStartOffset(30.0f);
    layoutProperty->UpdateContentEndOffset(40.0f);
    layoutProperty->UpdateSupportLazyLoadingEmptyBranch(true);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Modify original properties after cloning
     * @tc.expected: Original values are preserved in clone
     */
    EXPECT_EQ(clonedProperty->GetContentStartOffsetValue(), 30.0f);
    EXPECT_EQ(clonedProperty->GetContentEndOffsetValue(), 40.0f);
    EXPECT_EQ(clonedProperty->GetSupportLazyLoadingEmptyBranchValue(), true);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone005
 * @tc.desc: Test ScrollableLayoutProperty Clone with edge values
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone005, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set edge values
     * @tc.expected: Properties are set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentStartOffset(0.0f);
    layoutProperty->UpdateContentEndOffset(-10.5f);
    layoutProperty->UpdateSupportLazyLoadingEmptyBranch(false);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful with edge values preserved
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify edge values are correctly cloned
     * @tc.expected: Edge values match exactly
     */
    EXPECT_EQ(clonedProperty->GetContentStartOffsetValue(), 0.0f);
    EXPECT_EQ(clonedProperty->GetContentEndOffsetValue(), -10.5f);
    EXPECT_EQ(clonedProperty->GetSupportLazyLoadingEmptyBranchValue(), false);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone006
 * @tc.desc: Test ScrollableLayoutProperty Clone with large values
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone006, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set large float values
     * @tc.expected: Properties are set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentStartOffset(999999.99f);
    layoutProperty->UpdateContentEndOffset(-999999.99f);
    layoutProperty->UpdateSupportLazyLoadingEmptyBranch(true);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful with large values preserved
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify large values are correctly cloned
     * @tc.expected: Large values match exactly
     */
    EXPECT_FLOAT_EQ(clonedProperty->GetContentStartOffsetValue(), 999999.99f);
    EXPECT_FLOAT_EQ(clonedProperty->GetContentEndOffsetValue(), -999999.99f);
    EXPECT_EQ(clonedProperty->GetSupportLazyLoadingEmptyBranchValue(), true);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone007
 * @tc.desc: Test ScrollableLayoutProperty Clone only ContentStartOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone007, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set only ContentStartOffset
     * @tc.expected: Property is set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentStartOffset(25.5f);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify only ContentStartOffset is set in clone
     * @tc.expected: Only ContentStartOffset has value
     */
    EXPECT_TRUE(clonedProperty->HasContentStartOffset());
    EXPECT_FALSE(clonedProperty->HasContentEndOffset());
    EXPECT_FALSE(clonedProperty->HasSupportLazyLoadingEmptyBranch());
    EXPECT_EQ(clonedProperty->GetContentStartOffsetValue(), 25.5f);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone008
 * @tc.desc: Test ScrollableLayoutProperty Clone only ContentEndOffset
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone008, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set only ContentEndOffset
     * @tc.expected: Property is set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentEndOffset(35.75f);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify only ContentEndOffset is set in clone
     * @tc.expected: Only ContentEndOffset has value
     */
    EXPECT_FALSE(clonedProperty->HasContentStartOffset());
    EXPECT_TRUE(clonedProperty->HasContentEndOffset());
    EXPECT_FALSE(clonedProperty->HasSupportLazyLoadingEmptyBranch());
    EXPECT_EQ(clonedProperty->GetContentEndOffsetValue(), 35.75f);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone009
 * @tc.desc: Test ScrollableLayoutProperty Clone only SupportLazyLoadingEmptyBranch
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone009, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set only SupportLazyLoadingEmptyBranch
     * @tc.expected: Property is set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateSupportLazyLoadingEmptyBranch(true);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Verify only SupportLazyLoadingEmptyBranch is set in clone
     * @tc.expected: Only SupportLazyLoadingEmptyBranch has value
     */
    EXPECT_FALSE(clonedProperty->HasContentStartOffset());
    EXPECT_FALSE(clonedProperty->HasContentEndOffset());
    EXPECT_TRUE(clonedProperty->HasSupportLazyLoadingEmptyBranch());
    EXPECT_EQ(clonedProperty->GetSupportLazyLoadingEmptyBranchValue(), true);
}

/**
 * @tc.name: ScrollableLayoutPropertyClone010
 * @tc.desc: Test ScrollableLayoutProperty Clone with Reset and Update
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, ScrollableLayoutPropertyClone010, TestSize.Level1)
{
    /**
     * @tc.step: step1. Create ScrollableLayoutProperty and set properties
     * @tc.expected: Properties are set successfully
     */
    auto layoutProperty = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    layoutProperty->UpdateContentStartOffset(100.0f);
    layoutProperty->UpdateContentEndOffset(200.0f);
    layoutProperty->UpdateSupportLazyLoadingEmptyBranch(true);

    /**
     * @tc.step: step2. Clone the layout property
     * @tc.expected: Clone successful
     */
    auto clonedProperty = AceType::DynamicCast<ScrollableLayoutProperty>(layoutProperty->Clone());
    ASSERT_NE(clonedProperty, nullptr);

    /**
     * @tc.step: step3. Reset original property and verify clone is unaffected
     * @tc.expected: Clone maintains its values
     */
    layoutProperty->Reset();

    EXPECT_TRUE(clonedProperty->HasContentStartOffset());
    EXPECT_TRUE(clonedProperty->HasContentEndOffset());
    EXPECT_TRUE(clonedProperty->HasSupportLazyLoadingEmptyBranch());
    EXPECT_EQ(clonedProperty->GetContentStartOffsetValue(), 100.0f);
    EXPECT_EQ(clonedProperty->GetContentEndOffsetValue(), 200.0f);
    EXPECT_EQ(clonedProperty->GetSupportLazyLoadingEmptyBranchValue(), true);

    /**
     * @tc.step: step4. Verify original property is reset
     * @tc.expected: Original property has no values
     */
    EXPECT_FALSE(layoutProperty->HasContentStartOffset());
    EXPECT_FALSE(layoutProperty->HasContentEndOffset());
    EXPECT_FALSE(layoutProperty->HasSupportLazyLoadingEmptyBranch());
}

/**
 * @tc.name: PublicDumpPreservesFlatScrollProperties
 * @tc.desc: Property and scrollbar readers keep flat values and do not duplicate host keys.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, PublicDumpPreservesFlatScrollProperties, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    auto node = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 1, pattern);
    auto property = node->GetPaintProperty<ScrollablePaintProperty>();
    ASSERT_NE(property, nullptr);
    const Dimension hostHeight(11.0, DimensionUnit::VP);
    const ScrollBarMargin hostMargin { Dimension(2.0, DimensionUnit::VP), Dimension(3.0, DimensionUnit::VP) };
    property->UpdateScrollBarHeight(hostHeight);
    property->UpdateScrollBarMargin(hostMargin);
    pattern->scrollBar_ = AceType::MakeRefPtr<ScrollBar>();
    pattern->scrollBar_->touchRegion_ = Rect(1, 2, 30, 40);
    pattern->scrollBar_->hoverRegion_ = Rect(3, 4, 50, 60);
    pattern->scrollBar_->touchWidth_ = Dimension(12.0, DimensionUnit::VP);
    pattern->scrollBar_->hoverWidth_ = Dimension(16.0, DimensionUnit::VP);
    pattern->scrollBar_->scrollBarHeight_ = Dimension(22.0, DimensionUnit::VP);
    pattern->scrollBar_->scrollBarMargin_ = ScrollBarMargin {};
    auto legacy = JsonUtil::Create();
    pattern->ScrollablePattern::DumpAdvanceInfo(legacy);
    for (bool visual : { false, true }) {
        for (bool functional : { false, true }) {
            ParamConfig config;
            config.dumpMode = TreeDumpMode::PUBLIC_TREE;
            config.visualProperties = visual;
            config.functionalProperties = functional;
            auto json = JsonUtil::Create();
            pattern->ScrollablePattern::DumpAdvanceInfo(json, config);
            EXPECT_FALSE(json->Contains("paintProperty"));
            EXPECT_FALSE(json->Contains("scrollBar"));
            EXPECT_EQ(json->Contains("activeRect"), visual);
            for (const char* key : { "touchRegion", "hoverRegion", "touchWidth", "hoverWidth" }) {
                EXPECT_EQ(json->Contains(key), visual);
                if (visual) {
                    EXPECT_EQ(json->GetString(key), legacy->GetString(key));
                }
            }
            EXPECT_EQ(json->Contains("autoAdjustScrollBarMargin"), functional);
            EXPECT_FALSE(json->Contains("innerScrollBarLayoutInfos"));
            if (visual) {
                ExpectUniqueDumpProperty(*json, "scrollBarHeight", legacy->GetString("scrollBarHeight"));
                ExpectUniqueDumpProperty(*json, "scrollBarMargin", legacy->GetString("scrollBarMargin"));
            }
        }
    }
    auto after = JsonUtil::Create();
    pattern->ScrollablePattern::DumpAdvanceInfo(after);
    EXPECT_EQ(after->ToString(), legacy->ToString());
}


/**
 * @tc.name: LegacyDumpEventSlotsMatchBaselineSnapshot
 * @tc.desc: Preserve separate ordinary and JSFrameNode event fields and their order.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, LegacyDumpEventSlotsMatchBaselineSnapshot, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    auto host = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 1, pattern);
    auto hub = host->GetEventHub<ScrollableEventHub>();
    ASSERT_NE(hub, nullptr);
    hub->SetOnScrollStart([]() {});
    hub->SetJSFrameNodeOnScrollStop([]() {});
    // Frozen from fdf7d63988ed9fa338051a11c19fadf1f86653dc; compare serialized key order and types.
    const std::string expected = R"({"hasOnScrollStart":"true","nodeOnScrollStart":"false",)"
        R"("hasOnScrollStop":"false","nodeOnScrollStop":"true","hasOnWillScroll":"false",)"
        R"("nodeOnWillScroll":"false","hasOnDidScroll":"false","nodeOnDidScroll":"false",)"
        R"("hasOnScrollBegin":"false","nodeOnScrollBegin":"false","hasOnReachStart":"false",)"
        R"("nodeOnReachStart":"false","hasOnReachEnd":"false","nodeOnReachEnd":"false"})";
    auto json = JsonUtil::Create();
    pattern->ScrollablePattern::GetEventDumpInfo(json);
    EXPECT_EQ(json->ToString(), expected);
}

/**
 * @tc.name: DumpContentOffsetUnits
 * @tc.desc: Public configured offsets retain vp units; legacy strings, missing values and order stay unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpContentOffsetUnits, TestSize.Level1)
{
    auto property = AceType::MakeRefPtr<ScrollableLayoutProperty>();
    ParamConfig config;
    config.dumpMode = TreeDumpMode::PUBLIC_TREE;
    auto unset = JsonUtil::Create();
    property->DumpInfo(unset, config);
    EXPECT_EQ(unset->ToString(), "{}");
    auto missing = JsonUtil::Create();
    property->DumpInfo(missing);
    EXPECT_EQ(missing->ToString(), "{\"contentStartOffset\":\"None\",\"contentEndOffset\":\"None\"}");
    property->UpdateContentStartOffset(10.25f);
    property->UpdateContentEndOffset(-2.5f);
    auto json = JsonUtil::Create();
    property->DumpInfo(json, config);
    EXPECT_EQ(json->ToString(), "{\"contentStartOffset\":\"10.25vp\",\"contentEndOffset\":\"-2.50vp\"}");
    auto legacy = JsonUtil::Create();
    property->DumpInfo(legacy);
    EXPECT_EQ(legacy->ToString(), "{\"contentStartOffset\":\"10.250000\",\"contentEndOffset\":\"-2.500000\"}");
    property->ResetContentStartOffset();
    property->UpdateContentEndOffset(0.0f);
    auto endOnly = JsonUtil::Create();
    property->DumpInfo(endOnly, config);
    EXPECT_EQ(endOnly->ToString(), "{\"contentEndOffset\":\"0.00vp\"}");
}

/**
 * @tc.name: DumpContentOffsetSources
 * @tc.desc: Scroll exposes configured vp values, while List retains resolved px numbers and visual filtering.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableTestNg, DumpContentOffsetSources, TestSize.Level1)
{
    auto scroll = AceType::MakeRefPtr<ScrollPattern>();
    auto scrollNode = FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG, 1, scroll);
    auto list = AceType::MakeRefPtr<ListPattern>();
    auto listNode = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, 2, list);
    for (const auto& node : { scrollNode, listNode }) {
        ScrollableModelNG::SetContentStartOffset(AceType::RawPtr(node), 10.0f);
        ScrollableModelNG::SetContentEndOffset(AceType::RawPtr(node), 20.0f);
    }
    list->contentStartOffset_ = 20.0f;
    list->contentEndOffset_ = 40.0f;
    ParamConfig config;
    config.dumpMode = TreeDumpMode::PUBLIC_TREE;
    config.visualProperties = true;
    auto scrollJson = JsonUtil::Create();
    scroll->DumpAdvanceInfo(scrollJson, config);
    ExpectUniqueDumpProperty(*scrollJson, "contentStartOffset", "10.00vp");
    ExpectUniqueDumpProperty(*scrollJson, "contentEndOffset", "20.00vp");
    auto listJson = JsonUtil::Create();
    list->DumpAdvanceInfo(listJson, config);
    EXPECT_TRUE(listJson->GetValue("contentStartOffset")->IsNumber());
    EXPECT_TRUE(listJson->GetValue("contentEndOffset")->IsNumber());
    EXPECT_EQ(listJson->GetDouble("contentStartOffset"), 20.0);
    EXPECT_EQ(listJson->GetDouble("contentEndOffset"), 40.0);
    config.visualProperties = false;
    for (const auto& node : { scrollNode, listNode }) {
        auto disabled = JsonUtil::Create();
        node->GetPattern()->DumpAdvanceInfo(disabled, config);
        EXPECT_FALSE(disabled->Contains("contentStartOffset"));
        EXPECT_FALSE(disabled->Contains("contentEndOffset"));
    }
}
} // namespace OHOS::Ace::NG
