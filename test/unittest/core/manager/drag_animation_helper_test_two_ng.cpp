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

#include "test/unittest/core/manager/drag_animation_helper_test_ng.h"
#include "core/components_ng/manager/drag_drop/drag_drop_func_wrapper.h"
#include "core/components_ng/syntax/shallow_builder.h"

#include "test/mock/frameworks/base/image/mock_pixel_map.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/relative_container/relative_container_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/common/ace_engine.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/pattern/image/image_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float EPSILON = 0.1f;
constexpr float DRAG_PREVIEW_SCALE = 1.05f;
constexpr float LIFTING_ANIMATION_SCALE = 0.95f;
constexpr float DEFAULT_GRAYED_OPACITY = 0.4f;
constexpr float BADGE_PREVIEW_SCALE = 1.0f;
constexpr float PRE_GRAYED_OPACITY = 0.6f;
constexpr int32_t BADGE_CHILD_SIZE = 2;
constexpr int32_t BIG_BADGE_NUMBER = 3;
constexpr int32_t MOCK_PIXELMAP_WIDTH = 100;
constexpr int32_t MOCK_PIXELMAP_HEIGHT = 200;
constexpr int32_t BIG_MOCK_PIXELMAP_WIDTH = 200;
constexpr int32_t BIG_MOCK_PIXELMAP_HEIGHT = 300;
constexpr Dimension BADGE_RELATIVE_OFFSET = 8.0_vp;
} // namespace

/**
 * @tc.name: CreatePreviewNode001
 * @tc.desc: test CreatePreviewNode with null frameNode and frameNode without pixelMap.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, CreatePreviewNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call CreatePreviewNode with null frameNode.
     * @tc.expected: imageNode is nullptr.
     */
    RefPtr<FrameNode> imageNode = nullptr;
    PreparedInfoForDrag data;
    DragAnimationHelper::CreatePreviewNode(nullptr, imageNode, DRAG_PREVIEW_SCALE, data);
    EXPECT_EQ(imageNode, nullptr);

    /**
     * @tc.steps: step2. call CreatePreviewNode with frameNode which has no pixelMap.
     * @tc.expected: imageNode is nullptr.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    DragAnimationHelper::CreatePreviewNode(frameNode, imageNode, DRAG_PREVIEW_SCALE, data);
    EXPECT_EQ(imageNode, nullptr);
}

/**
 * @tc.name: CreatePreviewNode002
 * @tc.desc: test CreatePreviewNode with DEFAULT sizeChangeEffect and badgeNumber <= 1.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, CreatePreviewNode002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode with pixelMap 100 x 200.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto mockPixelMap = AceType::MakeRefPtr<MockPixelMap>();
    ASSERT_NE(mockPixelMap, nullptr);
    ON_CALL(*mockPixelMap, GetWidth()).WillByDefault(Return(MOCK_PIXELMAP_WIDTH));
    ON_CALL(*mockPixelMap, GetHeight()).WillByDefault(Return(MOCK_PIXELMAP_HEIGHT));
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    gestureHub->SetPixelMap(mockPixelMap);

    /**
     * @tc.steps: step2. call CreatePreviewNode with DEFAULT effect and badgeNumber = 1.
     */
    PreparedInfoForDrag data;
    data.sizeChangeEffect = DraggingSizeChangeEffect::DEFAULT;
    data.badgeNumber = 1;
    RefPtr<FrameNode> imageNode = nullptr;
    DragAnimationHelper::CreatePreviewNode(frameNode, imageNode, DRAG_PREVIEW_SCALE, data);

    /**
     * @tc.steps: step3. check imageNode position and scale are updated by pixelMap size.
     */
    ASSERT_NE(imageNode, nullptr);
    auto renderContext = imageNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto transformScale = renderContext->GetTransformScale();
    ASSERT_TRUE(transformScale.has_value());
    EXPECT_NEAR(transformScale->x, DRAG_PREVIEW_SCALE, EPSILON);
    EXPECT_NEAR(transformScale->y, DRAG_PREVIEW_SCALE, EPSILON);
    auto position = renderContext->GetPosition();
    ASSERT_TRUE(position.has_value());
    auto center = DragDropFuncWrapper::GetPaintRectCenter(frameNode);
    EXPECT_NEAR(position->GetX().Value(), center.GetX() - MOCK_PIXELMAP_WIDTH / 2.0f, EPSILON);
    EXPECT_NEAR(position->GetY().Value(), center.GetY() - MOCK_PIXELMAP_HEIGHT / 2.0f, EPSILON);
}

/**
 * @tc.name: CreatePreviewNode003
 * @tc.desc: test CreatePreviewNode with DEFAULT sizeChangeEffect and badgeNumber > 1.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, CreatePreviewNode003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode with pixelMap 200 x 300.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto mockPixelMap = AceType::MakeRefPtr<MockPixelMap>();
    ASSERT_NE(mockPixelMap, nullptr);
    ON_CALL(*mockPixelMap, GetWidth()).WillByDefault(Return(BIG_MOCK_PIXELMAP_WIDTH));
    ON_CALL(*mockPixelMap, GetHeight()).WillByDefault(Return(BIG_MOCK_PIXELMAP_HEIGHT));
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    gestureHub->SetPixelMap(mockPixelMap);
    auto relativeContainerNode = FrameNode::CreateFrameNode(
        V2::RELATIVE_CONTAINER_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<RelativeContainerPattern>());
    ASSERT_NE(relativeContainerNode, nullptr);

    /**
     * @tc.steps: step2. call CreatePreviewNode with DEFAULT effect and badgeNumber = 3.
     */
    PreparedInfoForDrag data;
    data.sizeChangeEffect = DraggingSizeChangeEffect::DEFAULT;
    data.badgeNumber = BIG_BADGE_NUMBER;
    data.originPreviewRect = RectF(0.0f, 0.0f, 100.0f, 200.0f);
    data.relativeContainerNode = relativeContainerNode;
    RefPtr<FrameNode> imageNode = nullptr;
    DragAnimationHelper::CreatePreviewNode(frameNode, imageNode, DRAG_PREVIEW_SCALE, data);
    ASSERT_NE(imageNode, nullptr);

    /**
     * @tc.steps: step3. check relativeContainerNode position is updated by originPreviewRect.
     */
    auto relRenderContext = relativeContainerNode->GetRenderContext();
    ASSERT_NE(relRenderContext, nullptr);
    auto position = relRenderContext->GetPosition();
    ASSERT_TRUE(position.has_value());
    auto center = DragDropFuncWrapper::GetPaintRectCenter(frameNode);
    EXPECT_NEAR(position->GetX().Value(), center.GetX() - data.originPreviewRect.Width() / 2.0f, EPSILON);
    EXPECT_NEAR(position->GetY().Value(), center.GetY() - data.originPreviewRect.Height() / 2.0f, EPSILON);

    /**
     * @tc.steps: step4. check imageNode ideal size is overwritten by originPreviewRect size.
     */
    const auto& calcLayoutConstraint = imageNode->GetLayoutProperty()->GetCalcLayoutConstraint();
    ASSERT_NE(calcLayoutConstraint, nullptr);
    ASSERT_TRUE(calcLayoutConstraint->selfIdealSize.has_value());
    EXPECT_EQ(calcLayoutConstraint->selfIdealSize->Width().value().GetDimension().Value(), 100.0);
    EXPECT_EQ(calcLayoutConstraint->selfIdealSize->Height().value().GetDimension().Value(), 200.0);
}

/**
 * @tc.name: CreatePreviewNode004
 * @tc.desc: test CreatePreviewNode with SIZE_TRANSITION and SIZE_CONTENT_TRANSITION effects.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, CreatePreviewNode004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode with pixelMap 100 x 200.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto mockPixelMap = AceType::MakeRefPtr<MockPixelMap>();
    ASSERT_NE(mockPixelMap, nullptr);
    ON_CALL(*mockPixelMap, GetWidth()).WillByDefault(Return(MOCK_PIXELMAP_WIDTH));
    ON_CALL(*mockPixelMap, GetHeight()).WillByDefault(Return(MOCK_PIXELMAP_HEIGHT));
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    gestureHub->SetPixelMap(mockPixelMap);

    /**
     * @tc.steps: step2. call CreatePreviewNode with SIZE_TRANSITION and badgeNumber = 1.
     */
    auto relativeContainerNode = FrameNode::CreateFrameNode(
        V2::RELATIVE_CONTAINER_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<RelativeContainerPattern>());
    ASSERT_NE(relativeContainerNode, nullptr);
    PreparedInfoForDrag data;
    data.sizeChangeEffect = DraggingSizeChangeEffect::SIZE_TRANSITION;
    data.badgeNumber = 1;
    data.originPreviewRect = RectF(0.0f, 0.0f, 100.0f, 200.0f);
    data.relativeContainerNode = relativeContainerNode;
    RefPtr<FrameNode> imageNode = nullptr;
    DragAnimationHelper::CreatePreviewNode(frameNode, imageNode, DRAG_PREVIEW_SCALE, data);
    ASSERT_NE(imageNode, nullptr);
    auto position = relativeContainerNode->GetRenderContext()->GetPosition();
    ASSERT_TRUE(position.has_value());

    /**
     * @tc.steps: step3. badge branch needs DEFAULT effect, imageNode ideal size keeps pixelMap size.
     */
    const auto& calcLayoutConstraint = imageNode->GetLayoutProperty()->GetCalcLayoutConstraint();
    ASSERT_NE(calcLayoutConstraint, nullptr);
    ASSERT_TRUE(calcLayoutConstraint->selfIdealSize.has_value());
    EXPECT_EQ(calcLayoutConstraint->selfIdealSize->Width().value().GetDimension().Value(), MOCK_PIXELMAP_WIDTH);

    /**
     * @tc.steps: step4. call CreatePreviewNode with SIZE_CONTENT_TRANSITION and badgeNumber = 2.
     */
    auto relativeContainerNode2 = FrameNode::CreateFrameNode(
        V2::RELATIVE_CONTAINER_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<RelativeContainerPattern>());
    ASSERT_NE(relativeContainerNode2, nullptr);
    PreparedInfoForDrag data2;
    data2.sizeChangeEffect = DraggingSizeChangeEffect::SIZE_CONTENT_TRANSITION;
    data2.badgeNumber = BADGE_CHILD_SIZE;
    data2.originPreviewRect = RectF(0.0f, 0.0f, 50.0f, 60.0f);
    data2.relativeContainerNode = relativeContainerNode2;
    RefPtr<FrameNode> imageNode2 = nullptr;
    DragAnimationHelper::CreatePreviewNode(frameNode, imageNode2, DRAG_PREVIEW_SCALE, data2);
    ASSERT_NE(imageNode2, nullptr);
    ASSERT_TRUE(relativeContainerNode2->GetRenderContext()->GetPosition().has_value());

    /**
     * @tc.steps: step5. badge branch needs DEFAULT effect, imageNode ideal size keeps pixelMap size.
     */
    const auto& calcLayoutConstraint2 = imageNode2->GetLayoutProperty()->GetCalcLayoutConstraint();
    ASSERT_NE(calcLayoutConstraint2, nullptr);
    ASSERT_TRUE(calcLayoutConstraint2->selfIdealSize.has_value());
    EXPECT_EQ(calcLayoutConstraint2->selfIdealSize->Width().value().GetDimension().Value(), MOCK_PIXELMAP_WIDTH);
}

/**
 * @tc.name: UpdateStartAnimation001
 * @tc.desc: test UpdateStartAnimation with null animateProperty and null relativeContainerNode.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, UpdateStartAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare overlayManager and DragPreviewInfo without relativeContainerNode.
     */
    auto pipelineContext = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    auto overlayManager = pipelineContext->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);
    auto imageNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(imageNode, nullptr);
    DragDropManager::DragPreviewInfo info;
    info.imageNode = imageNode;
    info.relativeContainerNode = nullptr;

    /**
     * @tc.steps: step2. call UpdateStartAnimation with null animateProperty.
     * @tc.expected: return after gather node updates without crash.
     */
    RefPtr<NodeAnimatablePropertyFloat> nullAnimateProperty;
    DragAnimationHelper::UpdateStartAnimation(
        overlayManager, nullAnimateProperty, Point(5, 5), info, Offset(10.0, 10.0));

    /**
     * @tc.steps: step3. call UpdateStartAnimation with non-null animateProperty.
     * @tc.expected: animateProperty is set to 1.0f before relativeContainerNode check.
     */
    auto animateProperty = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(-1.0f, [](const float& value) {});
    ASSERT_NE(animateProperty, nullptr);
    DragAnimationHelper::UpdateStartAnimation(overlayManager, animateProperty, Point(5, 5), info, Offset(10.0, 10.0));
    EXPECT_FLOAT_EQ(animateProperty->Get(), 1.0f);
}

/**
 * @tc.name: UpdateStartAnimation002
 * @tc.desc: test UpdateStartAnimation with menuWrapperNode and full DragPreviewInfo.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, UpdateStartAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare overlayManager and dragDropManager.
     */
    auto pipelineContext = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    auto overlayManager = pipelineContext->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);
    auto dragDropManager = pipelineContext->GetDragDropManager();
    ASSERT_NE(dragDropManager, nullptr);

    /**
     * @tc.steps: step2. set menuWrapperNode with MenuWrapperPattern and a menu child.
     */
    auto menuWrapperNode = FrameNode::CreateFrameNode(
        V2::MENU_WRAPPER_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<MenuWrapperPattern>(GetElmtId()));
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(),
        AceType::MakeRefPtr<MenuPattern>(GetElmtId(), V2::COLUMN_ETS_TAG, MenuType::MENU));
    ASSERT_NE(menuNode, nullptr);
    menuWrapperNode->AddChild(menuNode);
    dragDropManager->SetMenuWrapperNode(menuWrapperNode);

    /**
     * @tc.steps: step3. prepare full DragPreviewInfo with all frame nodes.
     */
    auto imageNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(imageNode, nullptr);
    auto relativeContainerNode = FrameNode::CreateFrameNode(
        V2::RELATIVE_CONTAINER_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<RelativeContainerPattern>());
    ASSERT_NE(relativeContainerNode, nullptr);
    auto stackNode = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<StackPattern>());
    ASSERT_NE(stackNode, nullptr);
    auto menuPreviewImageNode =
        FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(menuPreviewImageNode, nullptr);
    DragDropManager::DragPreviewInfo info;
    info.imageNode = imageNode;
    info.relativeContainerNode = relativeContainerNode;
    info.stackNode = stackNode;
    info.menuPreviewImageNode = menuPreviewImageNode;
    info.sizeChangeEffect = DraggingSizeChangeEffect::DEFAULT;

    /**
     * @tc.steps: step4. call UpdateStartAnimation.
     * @tc.expected: animateProperty is set to 1.0f and relativeContainerNode translate is updated.
     */
    auto animateProperty = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(-1.0f, [](const float& value) {});
    ASSERT_NE(animateProperty, nullptr);
    DragAnimationHelper::UpdateStartAnimation(overlayManager, animateProperty, Point(5, 5), info, Offset(10.0, 20.0));
    EXPECT_FLOAT_EQ(animateProperty->Get(), 1.0f);
    auto translate = relativeContainerNode->GetRenderContext()->GetTransformTranslate();
    ASSERT_TRUE(translate.has_value());
    TranslateOptions expectValue { 10.0f, 20.0f, 0.0f };
    EXPECT_EQ(translate->x.CalcValue(), expectValue.x.CalcValue());
    EXPECT_EQ(translate->y.CalcValue(), expectValue.y.CalcValue());

    /**
     * @tc.steps: step5. clean menuWrapperNode.
     */
    RefPtr<FrameNode> nullWrapperNode;
    dragDropManager->SetMenuWrapperNode(nullWrapperNode);
}

/**
 * @tc.name: UpdateStartTransitionOptionAnimation001
 * @tc.desc: test UpdateStartTransitionOptionAnimation with borderRadius and shadow options.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, UpdateStartTransitionOptionAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call with null stackNode, return early.
     */
    DragDropManager::DragPreviewInfo info;
    info.stackNode = nullptr;
    DragAnimationHelper::UpdateStartTransitionOptionAnimation(info);

    /**
     * @tc.steps: step2. call with imageNode which has borderRadius option.
     * @tc.expected: stackNode borderRadius is updated by option value.
     */
    auto imageNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(imageNode, nullptr);
    DragPreviewOption previewOption;
    previewOption.options.borderRadius = BorderRadiusProperty(10.0_vp);
    imageNode->SetDragPreviewOptions(previewOption);
    auto stackNode = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<StackPattern>());
    ASSERT_NE(stackNode, nullptr);
    info.imageNode = imageNode;
    info.stackNode = stackNode;
    info.menuPreviewImageNode = nullptr;
    DragAnimationHelper::UpdateStartTransitionOptionAnimation(info);
    auto stackRadius = stackNode->GetRenderContext()->GetBorderRadius();
    ASSERT_TRUE(stackRadius.has_value());
    EXPECT_EQ(stackRadius->radiusTopLeft.value().Value(), 10.0);
    EXPECT_EQ(stackRadius->radiusTopLeft.value().Unit(), DimensionUnit::VP);

    /**
     * @tc.steps: step3. call without borderRadius option but with shadow option.
     * @tc.expected: stackNode borderRadius falls back to 0.0_vp and backShadow is set,
     *               menuPreviewImageNode is null so function returns early.
     */
    auto imageNode2 = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(imageNode2, nullptr);
    DragPreviewOption previewOption2;
    previewOption2.options.shadow = Shadow::CreateShadow(ShadowStyle::None);
    imageNode2->SetDragPreviewOptions(previewOption2);
    auto stackNode2 = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<StackPattern>());
    ASSERT_NE(stackNode2, nullptr);
    info.imageNode = imageNode2;
    info.stackNode = stackNode2;
    info.menuPreviewImageNode = nullptr;
    DragAnimationHelper::UpdateStartTransitionOptionAnimation(info);
    auto stackRadius2 = stackNode2->GetRenderContext()->GetBorderRadius();
    ASSERT_TRUE(stackRadius2.has_value());
    EXPECT_EQ(stackRadius2->radiusTopLeft.value().Value(), 0.0);
    EXPECT_TRUE(stackNode2->GetRenderContext()->GetBackShadow().has_value());
}

/**
 * @tc.name: UpdateStartTransitionOptionAnimation002
 * @tc.desc: test UpdateStartTransitionOptionAnimation with SIZE_CONTENT_TRANSITION effect.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, UpdateStartTransitionOptionAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare imageNode with custom opacity option.
     */
    auto imageNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(imageNode, nullptr);
    DragPreviewOption previewOption;
    previewOption.options.opacity = 0.5f;
    imageNode->SetDragPreviewOptions(previewOption);
    auto stackNode = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<StackPattern>());
    ASSERT_NE(stackNode, nullptr);
    auto menuPreviewImageNode =
        FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(menuPreviewImageNode, nullptr);

    /**
     * @tc.steps: step2. call with SIZE_CONTENT_TRANSITION effect.
     * @tc.expected: imageNode opacity is option opacity and menuPreviewImageNode opacity is 0.
     */
    DragDropManager::DragPreviewInfo info;
    info.imageNode = imageNode;
    info.stackNode = stackNode;
    info.menuPreviewImageNode = menuPreviewImageNode;
    info.sizeChangeEffect = DraggingSizeChangeEffect::SIZE_CONTENT_TRANSITION;
    DragAnimationHelper::UpdateStartTransitionOptionAnimation(info);

    auto imageOpacity = imageNode->GetRenderContext()->GetOpacity();
    ASSERT_TRUE(imageOpacity.has_value());
    EXPECT_NEAR(imageOpacity.value(), 0.5f, EPSILON);
    auto menuPreviewOpacity = menuPreviewImageNode->GetRenderContext()->GetOpacity();
    ASSERT_TRUE(menuPreviewOpacity.has_value());
    EXPECT_NEAR(menuPreviewOpacity.value(), 0.0f, EPSILON);
}

/**
 * @tc.name: HideDragNodeCopyWithAnimation002
 * @tc.desc: test HideDragNodeCopyWithAnimation with dragNodeCopy and defaultAnimationBeforeLifting true.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, HideDragNodeCopyWithAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare overlayManager and frameNode with defaultAnimationBeforeLifting true.
     */
    auto pipelineContext = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    auto overlayManager = pipelineContext->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);

    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    DragPreviewOption option;
    option.defaultAnimationBeforeLifting = true;
    frameNode->SetDragPreviewOptions(option);
    frameNode->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);

    auto dragNodeCopy =
        FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(dragNodeCopy, nullptr);
    dragNodeCopy->AttachContext(AceType::RawPtr(pipelineContext), true);
    overlayManager->SetDragNodeCopy(dragNodeCopy);

    /**
     * @tc.steps: step2. call HideDragNodeCopyWithAnimation with dragNodeCopy.
     * @tc.expected: frameNode visibility is restored and dragNodeCopy opacity is set to 0.
     */
    DragAnimationHelper::HideDragNodeCopyWithAnimation(overlayManager, frameNode);
    auto visibility = frameNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::INVISIBLE);
    EXPECT_EQ(visibility, VisibleType::VISIBLE);
    auto copyOpacity = dragNodeCopy->GetRenderContext()->GetOpacity();
    ASSERT_TRUE(copyOpacity.has_value());
    EXPECT_NEAR(copyOpacity.value(), 0.0f, EPSILON);
    auto copyScale = dragNodeCopy->GetRenderContext()->GetTransformScale();
    ASSERT_TRUE(copyScale.has_value());
    EXPECT_NEAR(copyScale->x, 1.0f, EPSILON);

    overlayManager->SetDragNodeCopy(nullptr);
}

/**
 * @tc.name: HideDragNodeCopyWithAnimation003
 * @tc.desc: test HideDragNodeCopyWithAnimation with defaultAnimationBeforeLifting false and no copy.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, HideDragNodeCopyWithAnimation003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call with defaultAnimationBeforeLifting false.
     * @tc.expected: frameNode visibility keeps INVISIBLE but dragNodeCopy opacity is still 0.
     */
    auto pipelineContext = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    auto overlayManager = pipelineContext->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);
    auto frameNode2 = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode2, nullptr);
    DragPreviewOption optionFalse;
    optionFalse.defaultAnimationBeforeLifting = false;
    frameNode2->SetDragPreviewOptions(optionFalse);
    frameNode2->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    auto dragNodeCopy2 =
        FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(dragNodeCopy2, nullptr);
    dragNodeCopy2->AttachContext(AceType::RawPtr(pipelineContext), true);
    overlayManager->SetDragNodeCopy(dragNodeCopy2);
    DragAnimationHelper::HideDragNodeCopyWithAnimation(overlayManager, frameNode2);
    auto visibility2 = frameNode2->GetLayoutProperty()->GetVisibilityValue(VisibleType::INVISIBLE);
    EXPECT_EQ(visibility2, VisibleType::INVISIBLE);
    auto copyOpacity2 = dragNodeCopy2->GetRenderContext()->GetOpacity();
    ASSERT_TRUE(copyOpacity2.has_value());
    EXPECT_NEAR(copyOpacity2.value(), 0.0f, EPSILON);

    /**
     * @tc.steps: step2. call without dragNodeCopy and defaultAnimationBeforeLifting false.
     * @tc.expected: frameNode visibility keeps INVISIBLE.
     */
    auto frameNode3 = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode3, nullptr);
    frameNode3->SetDragPreviewOptions(optionFalse);
    frameNode3->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    overlayManager->SetDragNodeCopy(nullptr);
    DragAnimationHelper::HideDragNodeCopyWithAnimation(overlayManager, frameNode3);
    auto visibility3 = frameNode3->GetLayoutProperty()->GetVisibilityValue(VisibleType::INVISIBLE);
    EXPECT_EQ(visibility3, VisibleType::INVISIBLE);

    overlayManager->SetDragNodeCopy(nullptr);
}

/**
 * @tc.name: ShowDragNodeCopyAnimation002
 * @tc.desc: test ShowDragNodeCopyAnimation with defaultAnimationBeforeLifting true.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, ShowDragNodeCopyAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare overlayManager and frameNode with defaultAnimationBeforeLifting true.
     */
    auto pipelineContext = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    auto overlayManager = pipelineContext->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);

    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    DragPreviewOption option;
    option.defaultAnimationBeforeLifting = true;
    frameNode->SetDragPreviewOptions(option);

    /**
     * @tc.steps: step2. call without dragNodeCopy.
     * @tc.expected: return early and frameNode visibility keeps VISIBLE.
     */
    overlayManager->SetDragNodeCopy(nullptr);
    DragAnimationHelper::ShowDragNodeCopyAnimation(overlayManager, frameNode);
    auto visibility = frameNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE);
    EXPECT_EQ(visibility, VisibleType::VISIBLE);

    /**
     * @tc.steps: step3. call with dragNodeCopy.
     * @tc.expected: frameNode visibility becomes INVISIBLE and dragNodeCopy scale is animated to 0.95.
     */
    auto dragNodeCopy =
        FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<ImagePattern>());
    ASSERT_NE(dragNodeCopy, nullptr);
    dragNodeCopy->AttachContext(AceType::RawPtr(pipelineContext), true);
    overlayManager->SetDragNodeCopy(dragNodeCopy);
    DragAnimationHelper::ShowDragNodeCopyAnimation(overlayManager, frameNode);
    auto visibility2 = frameNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE);
    EXPECT_EQ(visibility2, VisibleType::INVISIBLE);
    auto copyScale = dragNodeCopy->GetRenderContext()->GetTransformScale();
    ASSERT_TRUE(copyScale.has_value());
    EXPECT_NEAR(copyScale->x, LIFTING_ANIMATION_SCALE, EPSILON);
    EXPECT_NEAR(copyScale->y, LIFTING_ANIMATION_SCALE, EPSILON);

    overlayManager->SetDragNodeCopy(nullptr);
}

/**
 * @tc.name: GetOrCreateGatherNode003
 * @tc.desc: test GetOrCreateGatherNode early return branches.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, GetOrCreateGatherNode003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare overlayManager and collect item nodes.
     */
    auto pipelineContext = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    auto overlayManager = pipelineContext->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->RemoveGatherNode();
    std::vector<RefPtr<FrameNode>> itemNodes;
    for (const auto& child : childNodes_) {
        itemNodes.push_back(child);
    }
    ASSERT_GE(itemNodes.size(), 3u);
    std::vector<GatherNodeChildInfo> gatherNodeInfo;

    /**
     * @tc.steps: step2. call with null actuator.
     * @tc.expected: returns nullptr.
     */
    EXPECT_EQ(DragAnimationHelper::GetOrCreateGatherNode(overlayManager, nullptr, gatherNodeInfo), nullptr);

    /**
     * @tc.steps: step3. call with actuator which has no itemParentNode, IsNeedGather is false.
     * @tc.expected: returns nullptr.
     */
    auto gestureHub = itemNodes[0]->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    auto actuatorNoParent = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureHub)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(actuatorNoParent, nullptr);
    EXPECT_EQ(DragAnimationHelper::GetOrCreateGatherNode(overlayManager, actuatorNoParent, gatherNodeInfo), nullptr);

    /**
     * @tc.steps: step4. call with actuator whose frameNode is null.
     * @tc.expected: returns nullptr.
     */
    auto eventHub = AceType::MakeRefPtr<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto gestureHubNoHost =
        AceType::MakeRefPtr<GestureEventHub>(AceType::WeakClaim(AceType::RawPtr(eventHub)));
    ASSERT_NE(gestureHubNoHost, nullptr);
    auto actuatorNoHost = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureHubNoHost)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(actuatorNoHost, nullptr);
    actuatorNoHost->itemParentNode_ = parentNode_;
    actuatorNoHost->isSelectedItemNode_ = true;
    EXPECT_EQ(DragAnimationHelper::GetOrCreateGatherNode(overlayManager, actuatorNoHost, gatherNodeInfo), nullptr);

    /**
     * @tc.steps: step5. call with frameNode whose isMultiSelectionEnabled is false.
     * @tc.expected: returns nullptr.
     */
    DragPreviewOption previewOption;
    itemNodes[1]->SetDragPreviewOptions(previewOption);
    auto gestureHub2 = itemNodes[1]->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub2, nullptr);
    auto actuatorNoMulti = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureHub2)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(actuatorNoMulti, nullptr);
    actuatorNoMulti->itemParentNode_ = parentNode_;
    actuatorNoMulti->isSelectedItemNode_ = true;
    EXPECT_EQ(DragAnimationHelper::GetOrCreateGatherNode(overlayManager, actuatorNoMulti, gatherNodeInfo), nullptr);

    /**
     * @tc.steps: step6. call with null overlayManager.
     * @tc.expected: returns nullptr.
     */
    auto gestureHub3 = itemNodes[2]->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub3, nullptr);
    auto actuatorNoOverlay = AceType::MakeRefPtr<DragEventActuator>(
        AceType::WeakClaim(AceType::RawPtr(gestureHub3)), DRAG_DIRECTION, FINGERS_NUMBER, DISTANCE);
    ASSERT_NE(actuatorNoOverlay, nullptr);
    actuatorNoOverlay->itemParentNode_ = parentNode_;
    actuatorNoOverlay->isSelectedItemNode_ = true;
    EXPECT_EQ(DragAnimationHelper::GetOrCreateGatherNode(nullptr, actuatorNoOverlay, gatherNodeInfo), nullptr);
}

/**
 * @tc.name: UpdateBadgeTextNodePosition002
 * @tc.desc: test UpdateBadgeTextNodePosition early returns and default previewOffset in RTL and LTR.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, UpdateBadgeTextNodePosition002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create frameNode with pixelMap 100 x 200.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto mockPixelMap = AceType::MakeRefPtr<MockPixelMap>();
    ASSERT_NE(mockPixelMap, nullptr);
    ON_CALL(*mockPixelMap, GetWidth()).WillByDefault(Return(MOCK_PIXELMAP_WIDTH));
    ON_CALL(*mockPixelMap, GetHeight()).WillByDefault(Return(MOCK_PIXELMAP_HEIGHT));
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    gestureHub->SetPixelMap(mockPixelMap);

    /**
     * @tc.steps: step2. call with childSize <= 1.
     * @tc.expected: return early and textNode position is not set.
     */
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    DragAnimationHelper::UpdateBadgeTextNodePosition(frameNode, textNode, 1, BADGE_PREVIEW_SCALE);
    EXPECT_FALSE(textNode->GetRenderContext()->GetPosition().has_value());

    /**
     * @tc.steps: step3. call with frameNode which has no pixelMap.
     * @tc.expected: return early and textNode position is not set.
     */
    auto frameNodeNoPixelMap =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNodeNoPixelMap, nullptr);
    auto textNode2 = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode2, nullptr);
    DragAnimationHelper::UpdateBadgeTextNodePosition(
        frameNodeNoPixelMap, textNode2, BADGE_CHILD_SIZE, BADGE_PREVIEW_SCALE);
    EXPECT_FALSE(textNode2->GetRenderContext()->GetPosition().has_value());

    /**
     * @tc.steps: step4. call with null textNode.
     * @tc.expected: return early without crash.
     */
    DragAnimationHelper::UpdateBadgeTextNodePosition(frameNode, nullptr, BADGE_CHILD_SIZE, BADGE_PREVIEW_SCALE);

    /**
     * @tc.steps: step5. call with default previewOffset in RTL mode.
     * @tc.expected: textNode position is (-BADGE_RELATIVE_OFFSET, -BADGE_RELATIVE_OFFSET).
     */
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    DragAnimationHelper::UpdateBadgeTextNodePosition(frameNode, textNode, BADGE_CHILD_SIZE, BADGE_PREVIEW_SCALE);
    AceApplicationInfo::GetInstance().isRightToLeft_ = false;
    auto position = textNode->GetRenderContext()->GetPosition();
    ASSERT_TRUE(position.has_value());
    float badgeOffset = BADGE_RELATIVE_OFFSET.ConvertToPx();
    // GetFrameNodeOffsetToWindow centers the preview: offset = (-width/2, -height/2).
    EXPECT_NEAR(position->GetX().Value(), -badgeOffset - MOCK_PIXELMAP_WIDTH / 2.0f, EPSILON);
    EXPECT_NEAR(position->GetY().Value(), -badgeOffset - MOCK_PIXELMAP_HEIGHT / 2.0f, EPSILON);

    /**
     * @tc.steps: step6. call with default previewOffset in LTR mode.
     * @tc.expected: textNode position is computed by pixelMap size and badge offset.
     */
    auto textNode3 = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode3, nullptr);
    DragAnimationHelper::UpdateBadgeTextNodePosition(frameNode, textNode3, BADGE_CHILD_SIZE, BADGE_PREVIEW_SCALE);
    auto position3 = textNode3->GetRenderContext()->GetPosition();
    ASSERT_TRUE(position3.has_value());
    float badgeLength = 1.0f;
    float expectX = MOCK_PIXELMAP_WIDTH * (BADGE_PREVIEW_SCALE + 1) / 2.0f - badgeOffset -
                    badgeOffset * badgeLength - MOCK_PIXELMAP_WIDTH / 2.0f;
    float expectY = -badgeOffset - MOCK_PIXELMAP_HEIGHT / 2.0f;
    EXPECT_NEAR(position3->GetX().Value(), expectX, EPSILON);
    EXPECT_NEAR(position3->GetY().Value(), expectY, EPSILON);
}

/**
 * @tc.name: DoDragStartGrayedAnimation002
 * @tc.desc: test DoDragStartGrayedAnimation when renderContext has no opacity, DoGrayedAnimation null branch
 *           and SetPreOpacity with pre-set grayed opacity.
 * @tc.type: FUNC
 */
HWTEST_F(DragAnimationHelperTestNg, DoDragStartGrayedAnimation002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call DoGrayedAnimation with null frameNode.
     * @tc.expected: return directly without crash.
     */
    auto pipelineContext = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    const RefPtr<InterpolatingSpring> curve = AceType::MakeRefPtr<InterpolatingSpring>(0.0f, 0.0f, 380.0f, 34.0f);
    DragAnimationHelper::DoGrayedAnimation(nullptr, DEFAULT_GRAYED_OPACITY, curve);

    /**
     * @tc.steps: step2. call DoDragStartGrayedAnimation when renderContext has no opacity.
     * @tc.expected: preGrayedOpacity keeps default 1.0f and opacity becomes 1.0f * DEFAULT_GRAYED.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    frameNode->AttachContext(AceType::RawPtr(pipelineContext), true);
    DragAnimationHelper::DoDragStartGrayedAnimation(frameNode);
    EXPECT_FLOAT_EQ(frameNode->GetPreGrayedOpacity(), 1.0f);
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto opacity = renderContext->GetOpacity();
    ASSERT_TRUE(opacity.has_value());
    EXPECT_NEAR(opacity.value(), DEFAULT_GRAYED_OPACITY, EPSILON);

    /**
     * @tc.steps: step3. call SetPreOpacity with pre-set grayed opacity.
     * @tc.expected: renderContext opacity is set to pre-set grayed opacity.
     */
    auto preNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, GetElmtId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(preNode, nullptr);
    preNode->AttachContext(AceType::RawPtr(pipelineContext), true);
    preNode->SetPreGrayedOpacity(PRE_GRAYED_OPACITY);
    DragAnimationHelper::SetPreOpacity(preNode);
    auto preRenderContext = preNode->GetRenderContext();
    ASSERT_NE(preRenderContext, nullptr);
    auto appliedOpacity = preRenderContext->GetOpacity();
    ASSERT_TRUE(appliedOpacity.has_value());
    EXPECT_NEAR(appliedOpacity.value(), PRE_GRAYED_OPACITY, EPSILON);
}
} // namespace OHOS::Ace::NG
