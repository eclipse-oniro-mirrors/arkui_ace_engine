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
#include "gtest/gtest.h"

#define protected public
#define private public
#include "base/geometry/shape.h"
#include "base/subwindow/subwindow_manager.h"
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/unittest/core/base/view_abstract_test_ng.h"

#include "core/components/theme/theme_constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/property/edgelight_property.h"
#include "core/components_ng/property/position_property.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class ViewAbstractModelStaticTestThreeNg : public testing::Test {};

namespace {
constexpr int32_t STATIC_THREE_NODE_ID = 20001;
constexpr float STATIC_THREE_SCALE_HALF = 0.5f;
constexpr float STATIC_THREE_SCALE_NINE = 0.9f;
constexpr float STATIC_THREE_BLOOM = 0.6f;
constexpr uint32_t STATIC_THREE_ILLUMINATED = 3;
constexpr double STATIC_THREE_MAX_WIDTH = 100.0;
constexpr double STATIC_THREE_MAX_HEIGHT = 200.0;
constexpr char STATIC_THREE_CALC_WIDTH[] = "calc(100% - 10vp)";
constexpr char STATIC_THREE_CALC_HEIGHT[] = "calc(50% + 20vp)";
constexpr char STATIC_THREE_BORDER_SRC[] = "resource://border.png";
constexpr char STATIC_THREE_EXTRA_INFO[] = "previewExtra";

RefPtr<FrameNode> CreateStaticThreeFrameNode(int32_t nodeId = STATIC_THREE_NODE_ID)
{
    return AceType::MakeRefPtr<FrameNode>("staticThreeTest", nodeId, AceType::MakeRefPtr<Pattern>());
}

RefPtr<PipelineContext> AttachStaticThreePipeline(const RefPtr<FrameNode>& frameNode)
{
    auto pipeline = AceType::MakeRefPtr<NG::PipelineContext>();
    pipeline->eventManager_ = AceType::MakeRefPtr<EventManager>();
    auto rootNode = AceType::MakeRefPtr<FrameNode>("root", -1, AceType::MakeRefPtr<Pattern>());
    pipeline->rootNode_ = rootNode;
    pipeline->overlayManager_ = AceType::MakeRefPtr<NG::OverlayManager>(rootNode);
    frameNode->context_ = AceType::RawPtr(pipeline);
    return pipeline;
}
} // namespace

/**
 * @tc.name: SetBackgroundColorOptionalBranches001
 * @tc.desc: Test SetBackgroundColor with value and empty optional.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetBackgroundColorOptionalBranches001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node and render context.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    /**
     * @tc.steps: step2. set background color with value, then with empty optional.
     * @tc.expected: value branch stores red, empty branch stores transparent.
     */
    ViewAbstractModelStatic::SetBackgroundColor(AceType::RawPtr(frameNode), Color::RED);
    EXPECT_EQ(renderContext->GetBackgroundColor().value_or(Color::TRANSPARENT), Color::RED);

    ViewAbstractModelStatic::SetBackgroundColor(AceType::RawPtr(frameNode), std::nullopt);
    EXPECT_EQ(renderContext->GetBackgroundColor().value_or(Color::BLACK), Color::TRANSPARENT);
}

/**
 * @tc.name: SetBorderImageBitsetCombos001
 * @tc.desc: Test SetBorderImage with none and outset bit combos.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetBorderImageBitsetCombos001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. combo with no bit set, all flags keep default.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto borderImage = AceType::MakeRefPtr<BorderImage>(STATIC_THREE_BORDER_SRC);
    ASSERT_NE(borderImage, nullptr);

    ViewAbstractModelStatic::SetBorderImage(
        AceType::RawPtr(frameNode), borderImage, 0);
    EXPECT_FALSE(renderContext->GetHasBorderImageOutset().value_or(false));
    EXPECT_FALSE(renderContext->GetHasBorderImageSlice().value_or(false));
    EXPECT_FALSE(renderContext->GetHasBorderImageRepeat().value_or(false));
    EXPECT_FALSE(renderContext->GetHasBorderImageWidth().value_or(false));
    EXPECT_FALSE(renderContext->GetBorderImageSource().has_value());

    /**
     * @tc.steps: step2. combo with only outset bit.
     * @tc.expected: only outset flag is true.
     */
    auto outsetNode = CreateStaticThreeFrameNode(STATIC_THREE_NODE_ID + 1);
    ASSERT_NE(outsetNode, nullptr);
    auto outsetContext = outsetNode->GetRenderContext();
    ASSERT_NE(outsetContext, nullptr);
    ViewAbstractModelStatic::SetBorderImage(
        AceType::RawPtr(outsetNode), borderImage, BorderImage::OUTSET_BIT);
    EXPECT_TRUE(outsetContext->GetHasBorderImageOutset().value_or(false));
    EXPECT_FALSE(outsetContext->GetHasBorderImageSlice().value_or(false));
    EXPECT_FALSE(outsetContext->GetHasBorderImageRepeat().value_or(false));
    EXPECT_FALSE(outsetContext->GetHasBorderImageWidth().value_or(false));
    EXPECT_FALSE(outsetContext->GetBorderImageSource().has_value());
}

/**
 * @tc.name: SetBorderImageBitsetCombos002
 * @tc.desc: Test SetBorderImage with source, combined bits and null border image.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetBorderImageBitsetCombos002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. combo with only source bit.
     * @tc.expected: border image source is stored with flag.
     */
    auto sourceNode = CreateStaticThreeFrameNode(STATIC_THREE_NODE_ID + 2);
    ASSERT_NE(sourceNode, nullptr);
    auto sourceContext = sourceNode->GetRenderContext();
    ASSERT_NE(sourceContext, nullptr);
    auto borderImage = AceType::MakeRefPtr<BorderImage>(STATIC_THREE_BORDER_SRC);
    ASSERT_NE(borderImage, nullptr);
    ViewAbstractModelStatic::SetBorderImage(
        AceType::RawPtr(sourceNode), borderImage, BorderImage::SOURCE_BIT);
    ASSERT_TRUE(sourceContext->GetBorderImageSource().has_value());
    EXPECT_EQ(sourceContext->GetBorderImageSource()->GetSrc(), STATIC_THREE_BORDER_SRC);
    EXPECT_TRUE(sourceContext->GetBorderSourceFromImage().value_or(false));
    EXPECT_FALSE(sourceContext->GetHasBorderImageOutset().value_or(false));

    /**
     * @tc.steps: step2. combo with slice, repeat and width bits.
     * @tc.expected: slice, repeat and width flags are true, source flag keeps default.
     */
    auto comboNode = CreateStaticThreeFrameNode(STATIC_THREE_NODE_ID + 3);
    ASSERT_NE(comboNode, nullptr);
    auto comboContext = comboNode->GetRenderContext();
    ASSERT_NE(comboContext, nullptr);
    ViewAbstractModelStatic::SetBorderImage(AceType::RawPtr(comboNode), borderImage,
        BorderImage::SLICE_BIT | BorderImage::REPEAT_BIT | BorderImage::WIDTH_BIT);
    EXPECT_TRUE(comboContext->GetHasBorderImageSlice().value_or(false));
    EXPECT_TRUE(comboContext->GetHasBorderImageRepeat().value_or(false));
    EXPECT_TRUE(comboContext->GetHasBorderImageWidth().value_or(false));
    EXPECT_FALSE(comboContext->GetHasBorderImageOutset().value_or(false));
    EXPECT_FALSE(comboContext->GetBorderImageSource().has_value());

    /**
     * @tc.steps: step3. null border image returns early.
     * @tc.expected: no flag is updated.
     */
    auto nullNode = CreateStaticThreeFrameNode(STATIC_THREE_NODE_ID + 4);
    ASSERT_NE(nullNode, nullptr);
    auto nullContext = nullNode->GetRenderContext();
    ASSERT_NE(nullContext, nullptr);
    ViewAbstractModelStatic::SetBorderImage(
        AceType::RawPtr(nullNode), nullptr, BorderImage::SOURCE_BIT | BorderImage::OUTSET_BIT);
    EXPECT_FALSE(nullContext->GetHasBorderImageOutset().value_or(false));
    EXPECT_FALSE(nullContext->GetBorderImageSource().has_value());
}

/**
 * @tc.name: SetMaxWidthCalcAndNumeric001
 * @tc.desc: Test SetMaxWidth with CALC unit and numeric unit.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetMaxWidthCalcAndNumeric001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set max width with CALC dimension.
     * @tc.expected: calc string is stored in max size width.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);

    ViewAbstractModelStatic::SetMaxWidth(
        AceType::RawPtr(frameNode), CalcDimension(STATIC_THREE_CALC_WIDTH, DimensionUnit::CALC));
    auto&& layoutConstraint = layoutProperty->GetCalcLayoutConstraint();
    ASSERT_NE(layoutConstraint, nullptr);
    ASSERT_TRUE(layoutConstraint->maxSize.has_value());
    EXPECT_EQ(layoutConstraint->maxSize->Width(), NG::CalcLength(STATIC_THREE_CALC_WIDTH));

    /**
     * @tc.steps: step2. set max width with numeric dimension.
     * @tc.expected: numeric value is stored in max size width.
     */
    ViewAbstractModelStatic::SetMaxWidth(
        AceType::RawPtr(frameNode), CalcDimension(STATIC_THREE_MAX_WIDTH, DimensionUnit::VP));
    ASSERT_TRUE(layoutConstraint->maxSize.has_value());
    EXPECT_EQ(layoutConstraint->maxSize->Width(),
        NG::CalcLength(Dimension(STATIC_THREE_MAX_WIDTH, DimensionUnit::VP)));
}

/**
 * @tc.name: SetMaxHeightCalcAndNumeric001
 * @tc.desc: Test SetMaxHeight with CALC unit and numeric unit.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetMaxHeightCalcAndNumeric001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set max height with CALC dimension.
     * @tc.expected: calc string is stored in max size height.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);

    ViewAbstractModelStatic::SetMaxHeight(
        AceType::RawPtr(frameNode), CalcDimension(STATIC_THREE_CALC_HEIGHT, DimensionUnit::CALC));
    auto&& layoutConstraint = layoutProperty->GetCalcLayoutConstraint();
    ASSERT_NE(layoutConstraint, nullptr);
    ASSERT_TRUE(layoutConstraint->maxSize.has_value());
    EXPECT_EQ(layoutConstraint->maxSize->Height(), NG::CalcLength(STATIC_THREE_CALC_HEIGHT));

    /**
     * @tc.steps: step2. set max height with numeric dimension.
     * @tc.expected: numeric value is stored in max size height.
     */
    ViewAbstractModelStatic::SetMaxHeight(
        AceType::RawPtr(frameNode), CalcDimension(STATIC_THREE_MAX_HEIGHT, DimensionUnit::VP));
    ASSERT_TRUE(layoutConstraint->maxSize.has_value());
    EXPECT_EQ(layoutConstraint->maxSize->Height(),
        NG::CalcLength(Dimension(STATIC_THREE_MAX_HEIGHT, DimensionUnit::VP)));
}

/**
 * @tc.name: SetClickEffectLevelFourStates001
 * @tc.desc: Test SetClickEffectLevel with four optional state combos.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetClickEffectLevelFourStates001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node and render context.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    /**
     * @tc.steps: step2. both optionals are empty.
     * @tc.expected: level is undefined with default scale number.
     */
    ViewAbstractModelStatic::SetClickEffectLevel(AceType::RawPtr(frameNode), std::nullopt, std::nullopt);
    ASSERT_TRUE(renderContext->GetClickEffectLevel().has_value());
    EXPECT_EQ(renderContext->GetClickEffectLevel()->level, ClickEffectLevel::UNDEFINED);
    EXPECT_EQ(renderContext->GetClickEffectLevel()->scaleNumber, 0.0f);

    /**
     * @tc.steps: step3. only level has value.
     * @tc.expected: level is heavy with default scale number.
     */
    ViewAbstractModelStatic::SetClickEffectLevel(AceType::RawPtr(frameNode), ClickEffectLevel::HEAVY, std::nullopt);
    ASSERT_TRUE(renderContext->GetClickEffectLevel().has_value());
    EXPECT_EQ(renderContext->GetClickEffectLevel()->level, ClickEffectLevel::HEAVY);
    EXPECT_EQ(renderContext->GetClickEffectLevel()->scaleNumber, 0.0f);

    /**
     * @tc.steps: step4. only scale value has value.
     * @tc.expected: level keeps default light with given scale number.
     */
    ViewAbstractModelStatic::SetClickEffectLevel(
        AceType::RawPtr(frameNode), std::nullopt, STATIC_THREE_SCALE_HALF);
    ASSERT_TRUE(renderContext->GetClickEffectLevel().has_value());
    EXPECT_EQ(renderContext->GetClickEffectLevel()->level, ClickEffectLevel::LIGHT);
    EXPECT_EQ(renderContext->GetClickEffectLevel()->scaleNumber, STATIC_THREE_SCALE_HALF);

    /**
     * @tc.steps: step5. both level and scale value have value.
     * @tc.expected: level is middle with given scale number.
     */
    ViewAbstractModelStatic::SetClickEffectLevel(
        AceType::RawPtr(frameNode), ClickEffectLevel::MIDDLE, STATIC_THREE_SCALE_NINE);
    ASSERT_TRUE(renderContext->GetClickEffectLevel().has_value());
    EXPECT_EQ(renderContext->GetClickEffectLevel()->level, ClickEffectLevel::MIDDLE);
    EXPECT_EQ(renderContext->GetClickEffectLevel()->scaleNumber, STATIC_THREE_SCALE_NINE);
}

/**
 * @tc.name: SetKeyboardShortcutClearAndSingleKey001
 * @tc.desc: Test SetKeyboardShortcut with empty value clear and no-modifier single key.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetKeyboardShortcutClearAndSingleKey001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node with pipeline and event manager.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pipeline = AttachStaticThreePipeline(frameNode);
    ASSERT_NE(pipeline->GetEventManager(), nullptr);

    /**
     * @tc.steps: step2. register shortcut with ctrl modifier.
     * @tc.expected: shortcut is stored on event hub.
     */
    std::vector<ModifierKey> keys = { ModifierKey::CTRL };
    std::function<void()> action = []() {};
    ViewAbstractModelStatic::SetKeyboardShortcut(AceType::RawPtr(frameNode), "C", keys, std::move(action));
    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    EXPECT_TRUE(eventHub->HasKeyboardShortcut());
    const auto& shortcuts = eventHub->GetKeyboardShortcut();
    ASSERT_EQ(shortcuts.size(), 1);
    EXPECT_EQ(shortcuts[0].value, "C");
    EXPECT_NE(shortcuts[0].keys, 0);

    /**
     * @tc.steps: step3. register shortcut with empty value.
     * @tc.expected: single shortcut is cleared.
     */
    std::function<void()> emptyAction = []() {};
    ViewAbstractModelStatic::SetKeyboardShortcut(AceType::RawPtr(frameNode), "", keys, std::move(emptyAction));
    EXPECT_FALSE(eventHub->HasKeyboardShortcut());

    /**
     * @tc.steps: step4. register single char without modifier keys.
     * @tc.expected: nothing is registered because key value is zero.
     */
    std::vector<ModifierKey> emptyKeys;
    std::function<void()> singleAction = []() {};
    ViewAbstractModelStatic::SetKeyboardShortcut(
        AceType::RawPtr(frameNode), "a", emptyKeys, std::move(singleAction));
    EXPECT_FALSE(eventHub->HasKeyboardShortcut());
}

/**
 * @tc.name: SetKeyboardShortcutDuplicateDedup001
 * @tc.desc: Test SetKeyboardShortcut duplicate registration keeps first action.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetKeyboardShortcutDuplicateDedup001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node with pipeline and event manager.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pipeline = AttachStaticThreePipeline(frameNode);
    ASSERT_NE(pipeline->GetEventManager(), nullptr);

    /**
     * @tc.steps: step2. register shortcut twice with same value and keys.
     * @tc.expected: duplicate registration returns early, only one shortcut kept with first action.
     */
    bool firstActionCalled = false;
    bool secondActionCalled = false;
    std::vector<ModifierKey> keys = { ModifierKey::CTRL };
    std::function<void()> firstAction = [&firstActionCalled]() { firstActionCalled = true; };
    ViewAbstractModelStatic::SetKeyboardShortcut(AceType::RawPtr(frameNode), "C", keys, std::move(firstAction));
    std::function<void()> secondAction = [&secondActionCalled]() { secondActionCalled = true; };
    ViewAbstractModelStatic::SetKeyboardShortcut(AceType::RawPtr(frameNode), "C", keys, std::move(secondAction));

    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    const auto& shortcuts = eventHub->GetKeyboardShortcut();
    ASSERT_EQ(shortcuts.size(), 1);
    ASSERT_TRUE(shortcuts[0].onKeyboardShortcutAction);
    shortcuts[0].onKeyboardShortcutAction();
    EXPECT_TRUE(firstActionCalled);
    EXPECT_FALSE(secondActionCalled);
}

/**
 * @tc.name: SetFreezeOptionalForwarding001
 * @tc.desc: Test SetFreeze with value and empty optional.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetFreezeOptionalForwarding001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node and render context.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    /**
     * @tc.steps: step2. set freeze with true, then with empty optional.
     * @tc.expected: freeze is true then falls back to false.
     */
    ViewAbstractModelStatic::SetFreeze(AceType::RawPtr(frameNode), true);
    ASSERT_TRUE(renderContext->GetFreeze().has_value());
    EXPECT_TRUE(renderContext->GetFreeze().value());

    ViewAbstractModelStatic::SetFreeze(AceType::RawPtr(frameNode), std::nullopt);
    ASSERT_TRUE(renderContext->GetFreeze().has_value());
    EXPECT_FALSE(renderContext->GetFreeze().value());
}

/**
 * @tc.name: SetAllowDropAndDragPreview001
 * @tc.desc: Test SetAllowDrop and SetDragPreview with value and empty optional.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetAllowDropAndDragPreview001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. set allow drop with value, then with empty optional.
     * @tc.expected: forced disallow flag flips with optional state.
     */
    std::set<std::string> allowDrop = { "general.unlock-screen" };
    ViewAbstractModelStatic::SetAllowDrop(AceType::RawPtr(frameNode), allowDrop);
    EXPECT_FALSE(frameNode->GetDisallowDropForcedly());
    EXPECT_EQ(frameNode->GetAllowDrop().size(), 1);
    EXPECT_EQ(frameNode->GetAllowDrop().count("general.unlock-screen"), 1);

    ViewAbstractModelStatic::SetAllowDrop(AceType::RawPtr(frameNode), std::nullopt);
    EXPECT_TRUE(frameNode->GetDisallowDropForcedly());
    EXPECT_TRUE(frameNode->GetAllowDrop().empty());

    /**
     * @tc.steps: step3. set drag preview with value, then with empty optional.
     * @tc.expected: extra info is stored then reset to empty.
     */
    DragDropInfo dragDropInfo;
    dragDropInfo.extraInfo = STATIC_THREE_EXTRA_INFO;
    ViewAbstractModelStatic::SetDragPreview(AceType::RawPtr(frameNode), dragDropInfo);
    EXPECT_EQ(frameNode->GetDragPreview().extraInfo, STATIC_THREE_EXTRA_INFO);

    ViewAbstractModelStatic::SetDragPreview(AceType::RawPtr(frameNode), std::nullopt);
    EXPECT_EQ(frameNode->GetDragPreview().extraInfo, "");
}

/**
 * @tc.name: SetPrivacySensitiveAndMask001
 * @tc.desc: Test SetPrivacySensitive and SetMask with value and empty state.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetPrivacySensitiveAndMask001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node. Keep it detached from a pipeline so
     *            the LPX mask callback registration only touches node-local state.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    /**
     * @tc.steps: step2. set privacy sensitive with value, then with empty optional.
     * @tc.expected: flag is true then falls back to false.
     */
    ViewAbstractModelStatic::SetPrivacySensitive(AceType::RawPtr(frameNode), true);
    EXPECT_TRUE(frameNode->IsPrivacySensitive());

    ViewAbstractModelStatic::SetPrivacySensitive(AceType::RawPtr(frameNode), std::nullopt);
    EXPECT_FALSE(frameNode->IsPrivacySensitive());

    /**
     * @tc.steps: step3. set mask with basic shape, then with null shape.
     * @tc.expected: clip mask is stored then reset.
     */
    auto basicShape = AceType::MakeRefPtr<BasicShape>(BasicShapeType::CIRCLE);
    ASSERT_NE(basicShape, nullptr);
    ViewAbstract::SetMask(AceType::RawPtr(frameNode), basicShape);
    ASSERT_TRUE(renderContext->GetClipMask().has_value());
    EXPECT_EQ(renderContext->GetClipMask().value(), basicShape);

    ViewAbstract::SetMask(AceType::RawPtr(frameNode), nullptr);
    // UpdateClipMask(nullptr) stores an empty shape instead of resetting the
    // optional, so the property stays present with a null value.
    ASSERT_TRUE(renderContext->GetClipMask().has_value());
    EXPECT_EQ(renderContext->GetClipMask().value(), nullptr);
}

/**
 * @tc.name: SetEdgeLightParamSetAndReset001
 * @tc.desc: Test SetEdgeLightParam with value and empty optional.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetEdgeLightParamSetAndReset001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node and render context.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    /**
     * @tc.steps: step2. set edge light param with value.
     * @tc.expected: param is stored on render context.
     */
    EdgeLightParam param;
    param.edgeLightPosition = EdgeLightPosition::TOP;
    param.length = CalcDimension(10.0, DimensionUnit::VP);
    param.intensity = STATIC_THREE_SCALE_HALF;
    param.thickness = CalcDimension(2.0, DimensionUnit::VP);
    param.color = Color::RED;
    ViewAbstractModelStatic::SetEdgeLightParam(AceType::RawPtr(frameNode), param);
    ASSERT_TRUE(renderContext->GetEdgeLightParam().has_value());
    EXPECT_EQ(renderContext->GetEdgeLightParam().value(), param);

    /**
     * @tc.steps: step3. set edge light param with empty optional.
     * @tc.expected: edge light param is reset.
     */
    ViewAbstractModelStatic::SetEdgeLightParam(AceType::RawPtr(frameNode), std::nullopt);
    EXPECT_FALSE(renderContext->GetEdgeLightParam().has_value());
}

/**
 * @tc.name: SetBloomWithMockTheme001
 * @tc.desc: Test SetBloom with value and empty optional using mock theme constants.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetBloomWithMockTheme001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node and mock theme constants.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeConstants = AceType::MakeRefPtr<ThemeConstants>(nullptr);
    ASSERT_NE(themeConstants, nullptr);

    /**
     * @tc.steps: step2. set bloom with value, then with empty optional.
     * @tc.expected: bloom value is stored then reset, back shadow is always set.
     */
    ViewAbstractModelStatic::SetBloom(AceType::RawPtr(frameNode), STATIC_THREE_BLOOM, themeConstants);
    ASSERT_TRUE(renderContext->GetBloom().has_value());
    EXPECT_EQ(renderContext->GetBloom().value(), STATIC_THREE_BLOOM);
    EXPECT_TRUE(renderContext->GetBackShadow().has_value());

    ViewAbstractModelStatic::SetBloom(AceType::RawPtr(frameNode), std::nullopt, themeConstants);
    EXPECT_FALSE(renderContext->GetBloom().has_value());
    EXPECT_TRUE(renderContext->GetBackShadow().has_value());
}

/**
 * @tc.name: SetLightIlluminatedAndBorderWidth001
 * @tc.desc: Test SetLightIlluminated and SetIlluminatedBorderWidth with mock theme constants.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelStaticTestThreeNg, SetLightIlluminatedAndBorderWidth001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frame node and mock theme constants.
     */
    auto frameNode = CreateStaticThreeFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto themeConstants = AceType::MakeRefPtr<ThemeConstants>(nullptr);
    ASSERT_NE(themeConstants, nullptr);

    /**
     * @tc.steps: step2. set light illuminated with value, then with empty optional.
     * @tc.expected: illuminated value is stored then reset, border width comes from theme.
     */
    ViewAbstractModelStatic::SetLightIlluminated(
        AceType::RawPtr(frameNode), STATIC_THREE_ILLUMINATED, themeConstants);
    ASSERT_TRUE(renderContext->GetLightIlluminated().has_value());
    EXPECT_EQ(renderContext->GetLightIlluminated().value(), STATIC_THREE_ILLUMINATED);
    ASSERT_TRUE(renderContext->GetIlluminatedBorderWidth().has_value());
    EXPECT_EQ(renderContext->GetIlluminatedBorderWidth().value(), Dimension(10.0, DimensionUnit::PX));

    ViewAbstractModelStatic::SetLightIlluminated(AceType::RawPtr(frameNode), std::nullopt, themeConstants);
    EXPECT_FALSE(renderContext->GetLightIlluminated().has_value());

    /**
     * @tc.steps: step3. set illuminated border width directly.
     * @tc.expected: border width is stored on render context.
     */
    ViewAbstractModelStatic::SetIlluminatedBorderWidth(
        AceType::RawPtr(frameNode), Dimension(5.0, DimensionUnit::VP));
    ASSERT_TRUE(renderContext->GetIlluminatedBorderWidth().has_value());
    EXPECT_EQ(renderContext->GetIlluminatedBorderWidth().value(), Dimension(5.0, DimensionUnit::VP));
}
} // namespace OHOS::Ace::NG
