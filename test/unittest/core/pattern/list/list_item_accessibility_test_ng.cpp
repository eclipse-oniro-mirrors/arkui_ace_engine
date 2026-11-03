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

#include <tuple>
#include <vector>

#include "gtest/gtest.h"
#include "list_test_ng.h"

#include "core/components/list/list_item_theme.h"
#define protected public // NOLINT
#include "core/components_ng/pattern/list/list_item_accessibility_property.h"
#undef protected
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/syntax/shallow_builder.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

class ListItemAccessibilityTestNg : public ListTestNg {};

class ListItemSelectionRenderContextMock : public MockRenderContext {
public:
    void OnMouseSelectUpdate(bool isSelected, const Color& fillColor, const Color& strokeColor) override
    {
        selectUpdates_.emplace_back(isSelected, fillColor, strokeColor);
    }

    std::vector<std::tuple<bool, Color, Color>> selectUpdates_;
};

/**
 * @tc.name: SetSpecificSupportAction001
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction001, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_START;
    listItemPattern->swipeActionState_ = SwipeActionState::EXPANDED;
    listItemPattern->startNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: SetSpecificSupportAction002
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction002, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_START;
    listItemPattern->swipeActionState_ = SwipeActionState::COLLAPSED;
    listItemPattern->startNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: SetSpecificSupportAction003
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction003, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_ACTION;
    listItemPattern->swipeActionState_ = SwipeActionState::EXPANDED;
    listItemPattern->startNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: SetSpecificSupportAction004
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction004, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_ACTION;
    listItemPattern->swipeActionState_ = SwipeActionState::COLLAPSED;
    listItemPattern->startNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: SetSpecificSupportAction005
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction005, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_END;
    listItemPattern->swipeActionState_ = SwipeActionState::EXPANDED;
    listItemPattern->endNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: SetSpecificSupportAction006
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction006, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_END;
    listItemPattern->swipeActionState_ = SwipeActionState::COLLAPSED;
    listItemPattern->endNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: SetSpecificSupportAction007
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction007, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_ACTION;
    listItemPattern->swipeActionState_ = SwipeActionState::EXPANDED;
    listItemPattern->endNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: SetSpecificSupportAction008
 * @tc.desc: Test ListItemAccessibilityProperty SetSpecificSupportAction
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, SetSpecificSupportAction008, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemPattern> listItemPattern =
        AceType::MakeRefPtr<ListItemPattern>(shallowBuilder, V2::ListItemStyle::CARD);
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 2, listItemPattern);
    WeakPtr<FrameNode> node = frameNode;
    RefPtr<ListItemAccessibilityProperty> listItemAccessibilityProperty =
        AceType::MakeRefPtr<ListItemAccessibilityProperty>();
    listItemAccessibilityProperty->host_ = node;
    listItemPattern->swiperIndex_ = ListItemSwipeIndex::SWIPER_ACTION;
    listItemPattern->swipeActionState_ = SwipeActionState::COLLAPSED;
    listItemPattern->endNodeIndex_ = 2;
    listItemAccessibilityProperty->SetSpecificSupportAction();
    uint64_t expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    EXPECT_EQ(listItemAccessibilityProperty->supportActions_, expectActions);
}

/**
 * @tc.name: ActActionSelectThemeColor001
 * @tc.desc: Test ListItemPattern SetAccessibilityAction ACTION_SELECT uses ListItemTheme selected color.
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, ActActionSelectThemeColor001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with one ListItem and replace render context to capture select updates.
     */
    CreateList();
    CreateListItems(1);
    CreateDone();
    auto listItem = GetChildFrameNode(frameNode_, 0);
    ASSERT_NE(listItem, nullptr);
    auto pattern = listItem->GetPattern<ListItemPattern>();
    ASSERT_NE(pattern, nullptr);
    auto accessibilityProperty = listItem->GetAccessibilityProperty<ListItemAccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    auto renderContext = AceType::MakeRefPtr<ListItemSelectionRenderContextMock>();
    ASSERT_NE(renderContext, nullptr);
    listItem->renderContext_ = renderContext;

    /**
     * @tc.steps: step2. Set theme selected colors, selectedColor_ and selectedColorWithAlpha_ are different.
     */
    auto theme = MockPipelineContext::pipeline_->GetTheme<ListItemTheme>();
    ASSERT_NE(theme, nullptr);
    theme->selectedColor_ = Color::RED;
    theme->selectedColorWithAlpha_ = Color::BLUE;

    /**
     * @tc.steps: step3. Trigger ACTION_SELECT.
     * @tc.expected: Both select updates use GetItemSelectedColor(), and item is selected.
     */
    pattern->SetSelectable(true);
    EXPECT_TRUE(accessibilityProperty->ActActionSelect());
    EXPECT_TRUE(pattern->IsSelected());
    ASSERT_EQ(renderContext->selectUpdates_.size(), 2U);
    EXPECT_FALSE(std::get<0>(renderContext->selectUpdates_[0]));
    EXPECT_EQ(std::get<1>(renderContext->selectUpdates_[0]), Color::BLUE);
    EXPECT_EQ(std::get<2>(renderContext->selectUpdates_[0]), Color::BLUE);
    EXPECT_TRUE(std::get<0>(renderContext->selectUpdates_[1]));
    EXPECT_EQ(std::get<1>(renderContext->selectUpdates_[1]), Color::BLUE);
    EXPECT_EQ(std::get<2>(renderContext->selectUpdates_[1]), Color::BLUE);
    EXPECT_EQ(pattern->itemSelectedColor_, Color::BLUE);
}

/**
 * @tc.name: ActActionClearSelectionThemeColor001
 * @tc.desc: Test ListItemPattern SetAccessibilityAction ACTION_CLEAR_SELECTION uses ListItemTheme selected color.
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, ActActionClearSelectionThemeColor001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with one ListItem and replace render context to capture select updates.
     */
    CreateList();
    CreateListItems(1);
    CreateDone();
    auto listItem = GetChildFrameNode(frameNode_, 0);
    ASSERT_NE(listItem, nullptr);
    auto pattern = listItem->GetPattern<ListItemPattern>();
    ASSERT_NE(pattern, nullptr);
    auto accessibilityProperty = listItem->GetAccessibilityProperty<ListItemAccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    auto renderContext = AceType::MakeRefPtr<ListItemSelectionRenderContextMock>();
    ASSERT_NE(renderContext, nullptr);
    listItem->renderContext_ = renderContext;

    auto theme = MockPipelineContext::pipeline_->GetTheme<ListItemTheme>();
    ASSERT_NE(theme, nullptr);
    theme->selectedColorWithAlpha_ = Color::GREEN;

    /**
     * @tc.steps: step2. Select item, then trigger ACTION_CLEAR_SELECTION.
     * @tc.expected: Clear selection uses the theme selected color and item is not selected.
     */
    pattern->SetSelectable(true);
    EXPECT_TRUE(accessibilityProperty->ActActionSelect());
    renderContext->selectUpdates_.clear();
    EXPECT_TRUE(accessibilityProperty->ActActionClearSelection());
    EXPECT_FALSE(pattern->IsSelected());
    ASSERT_EQ(renderContext->selectUpdates_.size(), 1U);
    EXPECT_FALSE(std::get<0>(renderContext->selectUpdates_[0]));
    EXPECT_EQ(std::get<1>(renderContext->selectUpdates_[0]), Color::GREEN);
    EXPECT_EQ(std::get<2>(renderContext->selectUpdates_[0]), Color::GREEN);
    EXPECT_EQ(pattern->itemSelectedColor_, Color::GREEN);
}

/**
 * @tc.name: ActActionSelectAndClearSelectionNotSelectable001
 * @tc.desc: Test ListItemPattern SetAccessibilityAction select actions return early when item is not selectable.
 * @tc.type: FUNC
 */
HWTEST_F(ListItemAccessibilityTestNg, ActActionSelectAndClearSelectionNotSelectable001, TestSize.Level1)
{
    CreateList();
    CreateListItems(1);
    CreateDone();
    auto listItem = GetChildFrameNode(frameNode_, 0);
    ASSERT_NE(listItem, nullptr);
    auto pattern = listItem->GetPattern<ListItemPattern>();
    ASSERT_NE(pattern, nullptr);
    auto accessibilityProperty = listItem->GetAccessibilityProperty<ListItemAccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);
    auto renderContext = AceType::MakeRefPtr<ListItemSelectionRenderContextMock>();
    ASSERT_NE(renderContext, nullptr);
    listItem->renderContext_ = renderContext;

    /**
     * @tc.steps: step1. Set item not selectable and trigger both actions.
     * @tc.expected: No select update is performed and item stays unselected.
     */
    pattern->SetSelectable(false);
    EXPECT_TRUE(accessibilityProperty->ActActionSelect());
    EXPECT_FALSE(pattern->IsSelected());
    EXPECT_TRUE(renderContext->selectUpdates_.empty());
    EXPECT_TRUE(accessibilityProperty->ActActionClearSelection());
    EXPECT_FALSE(pattern->IsSelected());
    EXPECT_TRUE(renderContext->selectUpdates_.empty());
}
} // namespace OHOS::Ace::NG
