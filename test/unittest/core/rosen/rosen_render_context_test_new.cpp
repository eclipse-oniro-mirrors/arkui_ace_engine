/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <array>
#include <chrono>

#define private public
#define protected public

#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/unittest/core/rosen/rosen_render_context_test.h"
#include "test/unittest/core/rosen/mock_animated_image.h"
#include "core/components/common/properties/border_image.h"
#include "core/components_ng/pattern/render_node/render_node_properties.h"
#include "core/components_ng/render/adapter/pixelmap_image.h"
#include "core/components_ng/render/drawing.h"

#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_pattern.h"
#include "core/components_ng/base/mount_policy.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/common/event_manager.h"
#include "modifier_ng/background/rs_background_shader_modifier.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
namespace {
constexpr uint64_t MAX_WAITING_TIME_FOR_TASKS = 1000; // 1000ms
} // namespace

namespace OHOS::Ace::NG {
namespace {} // namespace

/**
 * @tc.name: RosenRenderContextTestNew001
 * @tc.desc: OnAttractionEffectUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    auto pipelineContext = rosenRenderContext->GetPipelineContext();
    EXPECT_NE(pipelineContext, nullptr);
    rosenRenderContext->SuggestOpIncNode(false, true);
    AttractionEffect effect;
    effect.fraction = 1.0f;
    effect.destinationX = Dimension(1.0, DimensionUnit::PX);
    effect.destinationY = Dimension(1.0, DimensionUnit::PX);
    rosenRenderContext->OnAttractionEffectUpdate(effect);
    Rosen::Vector2f dstPoint = { 1.0, 1.0 };
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetAttractionFractionValue(), 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetAttractionDstPointValue(), dstPoint);
}

/**
 * @tc.name: RosenRenderContextSafeAreaInsetMixedMount001
 * @tc.desc: Verify the dedicated safeAreaInset slot is retained by the mixed-mount RS target list.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextSafeAreaInsetMixedMount001, TestSize.Level0)
{
    auto host = FrameNode::CreateFrameNode("safeAreaInsetHost", -101, AceType::MakeRefPtr<Pattern>());
    auto inset = FrameNode::CreateFrameNode("safeAreaInsetSlot", -102, AceType::MakeRefPtr<Pattern>());
    auto overlay = FrameNode::CreateFrameNode("safeAreaInsetOverlay", -103, AceType::MakeRefPtr<Pattern>());
    auto focus = FrameNode::CreateFrameNode("safeAreaInsetFocus", -104, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(host, nullptr);
    ASSERT_NE(inset, nullptr);
    ASSERT_NE(overlay, nullptr);
    ASSERT_NE(focus, nullptr);
    auto hostContext = InitRosenRenderContext(host);
    auto insetContext = InitRosenRenderContext(inset);
    auto overlayContext = InitRosenRenderContext(overlay);
    auto focusContext = InitRosenRenderContext(focus);
    ASSERT_NE(hostContext, nullptr);
    ASSERT_NE(insetContext, nullptr);
    ASSERT_NE(overlayContext, nullptr);
    ASSERT_NE(focusContext, nullptr);
    host->renderContext_ = hostContext;
    inset->renderContext_ = insetContext;
    overlay->renderContext_ = overlayContext;
    focus->renderContext_ = focusContext;
    host->SetMountPolicy(MountPolicy::MIXED);
    host->SetSafeAreaInsetNode(inset);
    host->SetOverlayNode(overlay);
    host->SetFocusPaintNode(focus);

    auto targetNodes = hostContext->BuildMixedTargetRSNodes(AceType::RawPtr(host));
    auto insetIter = std::find(targetNodes.begin(), targetNodes.end(), insetContext->GetRSNode());
    auto overlayIter = std::find(targetNodes.begin(), targetNodes.end(), overlayContext->GetRSNode());
    auto focusIter = std::find(targetNodes.begin(), targetNodes.end(), focusContext->GetRSNode());
    ASSERT_NE(insetIter, targetNodes.end());
    ASSERT_NE(overlayIter, targetNodes.end());
    ASSERT_NE(focusIter, targetNodes.end());
    EXPECT_LT(insetIter, overlayIter);
    EXPECT_LT(overlayIter, focusIter);

    std::vector<RefPtr<FrameNode>> liveChildren;
    hostContext->GetLiveChildren(host, liveChildren);
    auto liveInset = std::find(liveChildren.begin(), liveChildren.end(), inset);
    auto liveOverlay = std::find(liveChildren.begin(), liveChildren.end(), overlay);
    auto liveFocus = std::find(liveChildren.begin(), liveChildren.end(), focus);
    ASSERT_NE(liveInset, liveChildren.end());
    ASSERT_NE(liveOverlay, liveChildren.end());
    ASSERT_NE(liveFocus, liveChildren.end());
    EXPECT_LT(liveInset, liveOverlay);
    EXPECT_LT(liveInset, liveFocus);
    EXPECT_LT(liveFocus, liveOverlay);
}

// The fixture supplies ordinary parent-relative geometry; layout computation is
// covered by SafeAreaInsetTestNg. Rendering and input use real Rosen contexts
// and EventManager rather than invoking application callbacks directly.
class SafeAreaInsetRosenTest : public RosenRenderContextTest {
public:
    void TearDown() override
    {
        if (host_) {
            host_->DetachFromMainTree(true);
            host_.Reset();
        }
        RosenRenderContextTest::TearDown();
    }

    RefPtr<FrameNode> MakeNode(const char* tag, const RectF& rect, bool button = false)
    {
        RefPtr<Pattern> pattern = AceType::MakeRefPtr<Pattern>();
        if (button) {
            pattern = AceType::MakeRefPtr<ButtonPattern>();
        }
        auto node = FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(), pattern);
        node->renderContext_ = InitRosenRenderContext(node);
        SetRect(node, rect);
        node->SetActive(true);
        return node;
    }

    void SetRect(const RefPtr<FrameNode>& node, const RectF& rect)
    {
        node->GetGeometryNode()->SetFrameOffset(rect.GetOffset());
        node->GetGeometryNode()->SetFrameSize(rect.GetSize());
        auto context = AceType::DynamicCast<RosenRenderContext>(node->GetRenderContext());
        ASSERT_NE(context, nullptr);
        context->UpdatePaintRect(rect);
        context->SyncGeometryProperties(rect);
    }

    void Install(const SafeAreaInsetOptions& options, const SafeAreaInsetContentArray& contents)
    {
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host_), options, contents);
        auto container = host_->GetSafeAreaInsetNode();
        ASSERT_NE(container, nullptr);
        container->renderContext_ = InitRosenRenderContext(container);
        SetRect(container, RectF(10.0f, 10.0f, 280.0f, 220.0f));
        container->SetActive(true);
        container->MarkNeedSyncRenderTree(true);
        container->RebuildRenderContextTree();
        host_->MarkNeedSyncRenderTree(true);
        host_->RebuildRenderContextTree();
    }

    void Tap(float x, float y)
    {
        auto manager = PipelineContext::GetCurrentContext()->GetEventManager();
        ASSERT_NE(manager, nullptr);
        TouchEvent event;
        event.id = ++touchId_;
        event.originalId = event.id;
        event.x = x;
        event.y = y;
        event.screenX = x;
        event.screenY = y;
        event.sourceType = SourceType::TOUCH;
        event.sourceTool = SourceTool::FINGER;
        event.type = TouchType::DOWN;
        event.time = TimeStamp(std::chrono::milliseconds(touchId_ * 100));
        TouchPoint pointer;
        pointer.id = event.id;
        pointer.originalId = event.id;
        pointer.x = x;
        pointer.y = y;
        pointer.screenX = x;
        pointer.screenY = y;
        pointer.downTime = event.time;
        pointer.sourceTool = SourceTool::FINGER;
        pointer.isPressed = true;
        event.pointers.push_back(pointer);
        TouchRestrict restrict;
        restrict.sourceType = SourceType::TOUCH;
        restrict.sourceTool = SourceTool::FINGER;
        restrict.touchEvent = event;
        manager->TouchTest(event, host_, restrict);
        manager->DispatchTouchEvent(event);
        event.type = TouchType::MOVE;
        event.time += std::chrono::milliseconds(10);
        manager->DispatchTouchEvent(event);
        event.type = TouchType::UP;
        event.time += std::chrono::milliseconds(10);
        event.pointers.front().isPressed = false;
        manager->DispatchTouchEvent(event);
    }

    RefPtr<FrameNode> host_;
    int32_t touchId_ = 0;
};

/**
 * @tc.name: OrdinaryButtonControl001
 * @tc.desc: Verify the same real input fixture can click an ordinary Button.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetRosenTest, OrdinaryButtonControl001, TestSize.Level0)
{
    host_ = MakeNode("ordinaryHost", RectF(20.0f, 30.0f, 300.0f, 240.0f));
    auto button = MakeNode("Button", RectF(90.0f, 200.0f, 40.0f, 20.0f), true);
    button->MountToParent(host_);
    host_->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    int32_t clicks = 0;
    button->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent& info) {
        ++clicks;
        EXPECT_NEAR(info.GetLocalLocation().GetX(), 5.0, 0.01);
        EXPECT_NEAR(info.GetLocalLocation().GetY(), 5.0, 0.01);
    });
    host_->MarkNeedSyncRenderTree(true);
    host_->RebuildRenderContextTree();
    Tap(115.0f, 235.0f);
    EXPECT_EQ(clicks, 1);
}

/**
 * @tc.name: DirectContentPositionAndClick001
 * @tc.desc: Click all four direct content roots at their visible positions and verify local touch coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetRosenTest, DirectContentPositionAndClick001, TestSize.Level0)
{
    host_ = MakeNode("insetHost", RectF(20.0f, 30.0f, 300.0f, 240.0f));
    host_->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    const std::array<OffsetF, SAFE_AREA_INSET_EDGE_COUNT> offsets = {
        OffsetF(80.0f, 0.0f), OffsetF(240.0f, 50.0f), OffsetF(90.0f, 190.0f), OffsetF(0.0f, 60.0f) };
    for (size_t index = 0; index < offsets.size(); ++index) {
        auto rect = RectF(offsets[index], SizeF(30.0f, 20.0f));
        auto button = MakeNode("Button", rect, true);
        int32_t clicks = 0;
        std::vector<TouchType> touches;
        button->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent& info) {
            ++clicks;
            EXPECT_NEAR(info.GetLocalLocation().GetX(), 5.0, 0.01);
            EXPECT_NEAR(info.GetLocalLocation().GetY(), 5.0, 0.01);
            EXPECT_NEAR(info.GetCurrentLocalLocation().GetX(), 5.0, 0.01);
            EXPECT_NEAR(info.GetCurrentLocalLocation().GetY(), 5.0, 0.01);
        });
        button->GetOrCreateGestureEventHub()->AddTouchEvent(AceType::MakeRefPtr<TouchEventImpl>(
            [&](TouchEventInfo& info) {
                ASSERT_FALSE(info.GetChangedTouches().empty());
                const auto& touch = info.GetChangedTouches().front();
                touches.push_back(touch.GetTouchType());
                EXPECT_NEAR(touch.GetLocalLocation().GetX(), 5.0, 0.01);
                EXPECT_NEAR(touch.GetLocalLocation().GetY(), 5.0, 0.01);
            }));
        SafeAreaInsetOptions options;
        options.edges[index] = SafeAreaInsetEdgeOptions {};
        SafeAreaInsetContentArray contents;
        contents[index] = button;
        Install(options, contents);
        auto container = host_->GetSafeAreaInsetNode();
        EXPECT_EQ(button->GetParent(), container);
        auto context = AceType::DynamicCast<RosenRenderContext>(button->GetRenderContext());
        EXPECT_FALSE(context->extraOffset_.has_value());
        auto bounds = context->GetRSNode()->GetStagingProperties().GetBounds();
        EXPECT_FLOAT_EQ(bounds.data_[0], rect.Left());
        EXPECT_FLOAT_EQ(bounds.data_[1], rect.Top());
        EXPECT_EQ(context->GetPaintRectWithoutTransform(), rect);
        Tap(30.0f + rect.Left() + 5.0f, 40.0f + rect.Top() + 5.0f);
        EXPECT_EQ(clicks, 1);
        EXPECT_EQ(touches, (std::vector<TouchType> { TouchType::DOWN, TouchType::MOVE, TouchType::UP }));
        ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host_));
    }
}

/**
 * @tc.name: NestedSingleRootClick001
 * @tc.desc: A Button inside one developer-owned root container uses normal ancestor coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetRosenTest, NestedSingleRootClick001, TestSize.Level0)
{
    host_ = MakeNode("insetHost", RectF(20.0f, 30.0f, 300.0f, 240.0f));
    host_->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    auto root = MakeNode("singleContainer", RectF(80.0f, 190.0f, 80.0f, 30.0f));
    auto button = MakeNode("Button", RectF(10.0f, 5.0f, 40.0f, 20.0f), true);
    button->MountToParent(root);
    int32_t clicks = 0;
    button->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent& info) {
        ++clicks;
        EXPECT_NEAR(info.GetLocalLocation().GetX(), 5.0, 0.01);
        EXPECT_NEAR(info.GetLocalLocation().GetY(), 5.0, 0.01);
    });
    SafeAreaInsetOptions options;
    options.edges[2] = SafeAreaInsetEdgeOptions {};
    SafeAreaInsetContentArray contents;
    contents[2] = root;
    Install(options, contents);
    root->MarkNeedSyncRenderTree(true);
    root->RebuildRenderContextTree();
    Tap(125.0f, 240.0f);
    EXPECT_EQ(clicks, 1);
}

/**
 * @tc.name: DirectContentZIndexAndBlank001
 * @tc.desc: Root zIndex determines the target and carrier-only hits do not block business content.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetRosenTest, DirectContentZIndexAndBlank001, TestSize.Level0)
{
    host_ = MakeNode("insetHost", RectF(20.0f, 30.0f, 300.0f, 240.0f));
    auto business = MakeNode("Button", RectF(0.0f, 0.0f, 300.0f, 240.0f), true);
    business->MountToParent(host_);
    host_->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    int32_t businessClicks = 0;
    business->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent&) { ++businessClicks; });
    auto first = MakeNode("Button", RectF(80.0f, 80.0f, 40.0f, 30.0f), true);
    auto last = MakeNode("Button", RectF(80.0f, 80.0f, 40.0f, 30.0f), true);
    int32_t firstClicks = 0;
    int32_t lastClicks = 0;
    first->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent&) { ++firstClicks; });
    last->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent&) { ++lastClicks; });
    SafeAreaInsetOptions options;
    options.edges[0] = SafeAreaInsetEdgeOptions {};
    options.edges[3] = SafeAreaInsetEdgeOptions {};
    SafeAreaInsetContentArray contents;
    contents[0] = first;
    contents[3] = last;
    Install(options, contents);
    Tap(115.0f, 125.0f);
    EXPECT_EQ(firstClicks, 0);
    EXPECT_EQ(lastClicks, 1);
    first->GetRenderContext()->UpdateZIndex(5);
    auto container = host_->GetSafeAreaInsetNode();
    container->MarkNeedSyncRenderTree(true);
    container->RebuildRenderContextTree();
    Tap(115.0f, 125.0f);
    EXPECT_EQ(firstClicks, 1);
    EXPECT_EQ(lastClicks, 1);
    Tap(40.0f, 50.0f);
    EXPECT_EQ(businessClicks, 1);
    EXPECT_EQ(container->GetHitTestMode(), HitTestMode::HTMTRANSPARENT_SELF);
}

/**
 * @tc.name: DirectContentMoveReplaceReset001
 * @tc.desc: Moved, disabled, hidden, replaced and cleared content follows ordinary input rules.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetRosenTest, DirectContentMoveReplaceReset001, TestSize.Level0)
{
    host_ = MakeNode("insetHost", RectF(20.0f, 30.0f, 300.0f, 240.0f));
    host_->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    auto button = MakeNode("Button", RectF(80.0f, 190.0f, 40.0f, 20.0f), true);
    int32_t clicks = 0;
    button->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent&) { ++clicks; });
    SafeAreaInsetOptions options;
    options.edges[2] = SafeAreaInsetEdgeOptions {};
    SafeAreaInsetContentArray contents;
    contents[2] = button;
    Install(options, contents);
    Tap(115.0f, 235.0f);
    EXPECT_EQ(clicks, 1);
    SetRect(button, RectF(150.0f, 190.0f, 40.0f, 20.0f));
    Tap(115.0f, 235.0f);
    EXPECT_EQ(clicks, 1);
    Tap(185.0f, 235.0f);
    EXPECT_EQ(clicks, 2);
    button->GetEventHub<EventHub>()->SetEnabled(false);
    Tap(185.0f, 235.0f);
    EXPECT_EQ(clicks, 2);
    auto replacement = MakeNode("Button", RectF(150.0f, 190.0f, 40.0f, 20.0f), true);
    int32_t replacementClicks = 0;
    replacement->GetOrCreateGestureEventHub()->SetUserOnClick([&](GestureEvent&) { ++replacementClicks; });
    contents[2] = replacement;
    Install(options, contents);
    EXPECT_EQ(button->GetParent(), nullptr);
    Tap(185.0f, 235.0f);
    EXPECT_EQ(clicks, 2);
    EXPECT_EQ(replacementClicks, 1);
    auto container = host_->GetSafeAreaInsetNode();
    replacement->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    container->MarkNeedSyncRenderTree(true);
    container->RebuildRenderContextTree();
    Tap(185.0f, 235.0f);
    EXPECT_EQ(replacementClicks, 1);
    replacement->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    container->MarkNeedSyncRenderTree(true);
    container->RebuildRenderContextTree();
    Tap(185.0f, 235.0f);
    EXPECT_EQ(replacementClicks, 2);
    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host_));
    host_->RebuildRenderContextTree();
    Tap(185.0f, 235.0f);
    EXPECT_EQ(replacementClicks, 2);
}

/**
 * @tc.name: DirectContentSingleMixedMount001
 * @tc.desc: Direct content remains mounted in the common container in both render modes.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetRosenTest, DirectContentSingleMixedMount001, TestSize.Level0)
{
    host_ = MakeNode("insetHost", RectF(0.0f, 0.0f, 300.0f, 240.0f));
    host_->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    auto button = MakeNode("Button", RectF(80.0f, 190.0f, 40.0f, 20.0f), true);
    SafeAreaInsetOptions options;
    options.edges[2] = SafeAreaInsetEdgeOptions {};
    SafeAreaInsetContentArray contents;
    contents[2] = button;
    Install(options, contents);
    auto container = host_->GetSafeAreaInsetNode();
    auto context = AceType::DynamicCast<RosenRenderContext>(container->GetRenderContext());
    for (auto policy : { MountPolicy::SINGLE_IF_RENDER_NODE, MountPolicy::MIXED }) {
        container->SetMountPolicy(policy);
        container->MarkNeedSyncRenderTree(true);
        container->RebuildRenderContextTree();
        EXPECT_EQ(context->GetRSNode()->GetChildren().size(), 1);
        auto targets = context->BuildMixedTargetRSNodes(AceType::RawPtr(container));
        ASSERT_EQ(targets.size(), 1);
        EXPECT_EQ(targets.front(), AceType::DynamicCast<RosenRenderContext>(button->GetRenderContext())->GetRSNode());
    }
}

/**
 * @tc.name: RosenRenderContextSafeAreaInsetAbsentPosition001
 * @tc.desc: Non-inset layout nodes retain their paint and deferred extra-offset sync behavior (AC-6.14).
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextSafeAreaInsetAbsentPosition001, TestSize.Level0)
{
    auto wrapper = FrameNode::CreateCommonNode("legacyLayoutNode", -130, true, AceType::MakeRefPtr<Pattern>());
    auto edgeContext = AceType::MakeRefPtr<RosenRenderContext>();
    edgeContext->SetHostNode(wrapper);
    wrapper->renderContext_ = edgeContext;
    wrapper->GetGeometryNode()->SetFrameOffset(OffsetF(80.0f, 60.0f));
    wrapper->GetGeometryNode()->SetPixelGridRoundOffset(OffsetF(11.0f, 12.0f));
    EXPECT_EQ(edgeContext->AdjustPaintRectInner(wrapper), RectF());
    EXPECT_EQ(wrapper->GetGeometryNode()->GetPixelGridRoundOffset(), OffsetF(11.0f, 12.0f));

    auto content = FrameNode::CreateFrameNode("legacyContent", -131, AceType::MakeRefPtr<Pattern>());
    content->MountToParent(wrapper);
    auto context = InitRosenRenderContext(content);
    content->renderContext_ = context;
    const RectF localRect(3.0f, 4.0f, 50.0f, 10.0f);
    context->UpdatePaintRect(localRect);
    context->SyncGeometryProperties(localRect);
    const auto bounds = context->GetRSNode()->GetStagingProperties().GetBounds();
    context->SetExtraOffset(OffsetF(80.0f, 60.0f));
    EXPECT_EQ(context->GetRSNode()->GetStagingProperties().GetBounds(), bounds);
}

/**
 * @tc.name: RosenRenderContextTestNew002
 * @tc.desc: NotifyHostTransformUpdated().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew002, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SyncPartialRsProperties();
    std::shared_ptr<Rosen::RectF> drawRect = std::make_shared<Rosen::RectF>(1.0, 1.0, 1.0, 1.0);
    rosenRenderContext->UpdateDrawRegion(5, drawRect); // 5 is the index of drawRegionRects_
    rosenRenderContext->NotifyHostTransformUpdated(false);
    EXPECT_FALSE(frameNode->isTransformNotChanged_);
}

/**
 * @tc.name: RosenRenderContextTestNew003
 * @tc.desc: SavePaintRect().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew003, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->ResetSurface(1, 0);
    rosenRenderContext->SetMarkNodeGroup(false);
    rosenRenderContext->SavePaintRect(true, 0);
    EXPECT_EQ(rosenRenderContext->paintRect_, RectF(0.0, 0.0, 0.0, 0.0));
    rosenRenderContext->SavePaintRect(false, 0);
    EXPECT_EQ(rosenRenderContext->paintRect_, RectF(0.0, 0.0, 0.0, 0.0));
    int32_t rollbackApiVersion = MockContainer::Current()->GetApiTargetVersion();
    int32_t setApiVersion = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    MockContainer::Current()->SetApiTargetVersion(setApiVersion);
    rosenRenderContext->SavePaintRect(true, 0);
    EXPECT_EQ(rosenRenderContext->paintRect_, RectF(0.0, 0.0, 0.0, 0.0));
    rosenRenderContext->SavePaintRect(false, 0);
    MockContainer::Current()->SetApiTargetVersion(rollbackApiVersion);
    EXPECT_EQ(rosenRenderContext->paintRect_, RectF(0.0, 0.0, 0.0, 0.0));
}

/**
 * @tc.name: RosenRenderContextTestNew004
 * @tc.desc: UpdatePaintRect().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew004, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->UpdatePaintRect(RectF(1.0, 1.0, 1.0, 1.0));
    EXPECT_EQ(rosenRenderContext->paintRect_, RectF(1.0, 1.0, 1.0, 1.0));
}

/**
 * @tc.name: RosenRenderContextTestNew005
 * @tc.desc: SetCommandPathMask().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew005, TestSize.Level1)
{
    std::string commands = "abcd";
    RSPath path;
    path.BuildFromSVGString(commands);
    ShapeMaskProperty property;
    property.fillColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeWidth = 0.0f;
    RSColor color;
    color.SetColorQuad(property.fillColor);
    RSBrush brush(color);
    RSColor rsStrokeColor;
    rsStrokeColor.SetColorQuad(property.strokeColor);
    RSPen pen;
    pen.SetColor(rsStrokeColor);
    pen.SetWidth(property.strokeWidth);
    std::shared_ptr<Rosen::RSMask> mask = Rosen::RSMask::CreatePathMask(path, pen, brush);
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetCommandPathMask(commands, property);
    rosenRenderContext->SetClipBoundsWithCommands(commands);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetMask()->GetMaskPen().GetColor(),
        mask->GetMaskPen().GetColor());
}

/**
 * @tc.name: RosenRenderContextTestNew006
 * @tc.desc: SetOvalMask().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew006, TestSize.Level1)
{
    auto rect = RectF(1.0, 1.0, 1.0, 1.0);
    RSRect rsRect(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
    RSPath path;
    path.AddOval(rsRect);
    ShapeMaskProperty property;
    property.fillColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeWidth = 0.0f;
    RSColor color;
    color.SetColorQuad(property.fillColor);
    RSBrush brush(color);
    RSColor rsStrokeColor;
    rsStrokeColor.SetColorQuad(property.strokeColor);
    RSPen pen;
    pen.SetColor(rsStrokeColor);
    pen.SetWidth(property.strokeWidth);
    std::shared_ptr<Rosen::RSMask> mask = Rosen::RSMask::CreatePathMask(path, pen, brush);
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetOvalMask(rect, property);
    rosenRenderContext->ClipWithRect(rect);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetMask()->GetMaskPen().GetColor(),
        mask->GetMaskPen().GetColor());
}

/**
 * @tc.name: RosenRenderContextTestNew007
 * @tc.desc: SetRoundRectMask().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew007, TestSize.Level1)
{
    auto rect = RectF(1.0, 1.0, 1.0, 1.0);
    RSRoundRect rsRoundRect;
    RoundRect roundRect;
    roundRect.SetRect(rect);
    roundRect.SetCornerRadius(1.0);
    RSRect rsRect(roundRect.GetRect().Left(), roundRect.GetRect().Top(), roundRect.GetRect().Right(),
        roundRect.GetRect().Bottom());
    rsRoundRect.SetRect(rsRect);
    EdgeF edge = roundRect.GetCornerRadius(RoundRect::TOP_LEFT_POS);
    rsRoundRect.SetCornerRadius(RSRoundRect::TOP_LEFT_POS, edge.x, edge.y);
    edge = roundRect.GetCornerRadius(RoundRect::TOP_RIGHT_POS);
    rsRoundRect.SetCornerRadius(RSRoundRect::TOP_RIGHT_POS, edge.x, edge.y);
    edge = roundRect.GetCornerRadius(RoundRect::BOTTOM_LEFT_POS);
    rsRoundRect.SetCornerRadius(RSRoundRect::BOTTOM_LEFT_POS, edge.x, edge.y);
    edge = roundRect.GetCornerRadius(RoundRect::BOTTOM_RIGHT_POS);
    rsRoundRect.SetCornerRadius(RSRoundRect::BOTTOM_RIGHT_POS, edge.x, edge.y);
    RSPath path;
    path.AddRoundRect(rsRoundRect);
    ShapeMaskProperty property;
    property.fillColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeWidth = 0.0f;
    RSColor color;
    color.SetColorQuad(property.fillColor);
    RSBrush brush(color);
    RSColor rsStrokeColor;
    rsStrokeColor.SetColorQuad(property.strokeColor);
    RSPen pen;
    pen.SetColor(rsStrokeColor);
    pen.SetWidth(property.strokeWidth);
    std::shared_ptr<Rosen::RSMask> mask = Rosen::RSMask::CreatePathMask(path, pen, brush);
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetRoundRectMask(roundRect, property);
    rosenRenderContext->ClipWithRoundRect(roundRect);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetMask()->GetMaskPen().GetColor(),
        mask->GetMaskPen().GetColor());
}

/**
 * @tc.name: RosenRenderContextTestNew008
 * @tc.desc: SetCircleMask().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew008, TestSize.Level1)
{
    Circle circle;
    circle.SetAxisX(Dimension(1.0));
    circle.SetAxisY(Dimension(0.0));
    circle.SetRadius(Dimension(1.0));
    RSPath path;
    path.AddCircle(circle.GetAxisX().Value(), circle.GetAxisY().Value(), circle.GetRadius().Value());
    ShapeMaskProperty property;
    property.fillColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeWidth = 0.0f;
    RSColor color;
    color.SetColorQuad(property.fillColor);
    RSBrush brush(color);
    RSColor rsStrokeColor;
    rsStrokeColor.SetColorQuad(property.strokeColor);
    RSPen pen;
    pen.SetColor(rsStrokeColor);
    pen.SetWidth(property.strokeWidth);
    std::shared_ptr<Rosen::RSMask> mask = Rosen::RSMask::CreatePathMask(path, pen, brush);
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetCircleMask(circle, property);
    rosenRenderContext->SetOpacityMultiplier(1.0);
    rosenRenderContext->ClipWithCircle(circle);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetMask()->GetMaskPen().GetColor(),
        mask->GetMaskPen().GetColor());
}

/**
 * @tc.name: RosenRenderContextTestNew009
 * @tc.desc: SetRectMask().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew009, TestSize.Level1)
{
    auto rect = RectF(1.0, 1.0, 1.0, 1.0);
    RSPath path;
    path.AddRect(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
    ShapeMaskProperty property;
    property.fillColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeColor = SHAPE_MASK_DEFAULT_COLOR;
    property.strokeWidth = 0.0f;
    RSColor color;
    color.SetColorQuad(property.fillColor);
    RSBrush brush(color);
    RSColor rsStrokeColor;
    rsStrokeColor.SetColorQuad(property.strokeColor);
    RSPen pen;
    pen.SetColor(rsStrokeColor);
    pen.SetWidth(property.strokeWidth);
    std::shared_ptr<Rosen::RSMask> mask = Rosen::RSMask::CreatePathMask(path, pen, brush);
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetRectMask(rect, property);
    rosenRenderContext->SetTranslate(0.0, 1.0, 0.0);
    rosenRenderContext->ClipWithOval(rect);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetMask()->GetMaskPen().GetColor(),
        mask->GetMaskPen().GetColor());
}

/**
 * @tc.name: RosenRenderContextTestNew010
 * @tc.desc: SetTransitionUserCallback().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew010, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    auto parentNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    parentNode->AddChild(frameNode);

    bool userFlag = false;
    TransitionFinishCallback userCallback = [&userFlag](bool ret) { userFlag = !userFlag; };
    rosenRenderContext->SetTransitionUserCallback(std::move(userCallback));
    rosenRenderContext->FireTransitionUserCallback(true);
    rosenRenderContext->InitEventClickEffect();
    EXPECT_TRUE(userFlag);

    bool outFlag = false;
    auto outCallback = [&outFlag]() { outFlag = !outFlag; };
    rosenRenderContext->SetTransitionOutCallback(std::move(outCallback));
    auto property = AceType::MakeRefPtr<NodeAnimatablePropertyBase>();
    rosenRenderContext->AttachNodeAnimatableProperty(property);
    auto layoutProperty = AceType::MakeRefPtr<LayoutProperty>();
    layoutProperty->propVisibility_ = VisibleType::INVISIBLE;
    frameNode->SetLayoutProperty(layoutProperty);
    frameNode->isDisappearing_ = false;
    frameNode->onMainTree_ = true;
    rosenRenderContext->DetachNodeAnimatableProperty(property);
    rosenRenderContext->OnTransitionOutFinish();
    EXPECT_TRUE(outFlag);

    bool inFlag = false;
    auto inCallback = [&inFlag]() { inFlag = !inFlag; };
    rosenRenderContext->SetTransitionInCallback(std::move(inCallback));
    rosenRenderContext->PostTransitionUserOutCallback();
    frameNode->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    rosenRenderContext->OnTransitionInFinish();
    EXPECT_TRUE(inFlag);
}

/**
 * @tc.name: RosenRenderContextTestNew011
 * @tc.desc: SetOpacity().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew011, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    EXPECT_FALSE(rosenRenderContext->IsUniRenderEnabled());
    rosenRenderContext->SetOpacity(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetAlpha(), 1.0);
    rosenRenderContext->SetFrame(0.0, 1.0, 0.0, 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetFrame().data_[1], 1.0);
    rosenRenderContext->SetRenderPivot(1.0, 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetPivot().x_, 1.0);

    rosenRenderContext->SetScale(1.0, 0.0);
    rosenRenderContext->PaintPixmapBgImage();
    rosenRenderContext->PaintRSBgImage();
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetScale().data_[0], 1.0);
    rosenRenderContext->SetRenderFrameOffset(OffsetF(1.0, 0.0));
    EXPECT_EQ(rosenRenderContext->frameOffset_, OffsetF(1.0, 0.0));
    rosenRenderContext->SetShadowRadius(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowRadius(), 1.0);
    rosenRenderContext->SetShadowElevation(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowElevation(), 1.0);
    rosenRenderContext->SetShadowAlpha(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowAlpha(), 1.0);
    rosenRenderContext->SetShadowOffset(1.0, 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowOffsetX(), 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowOffsetY(), 0.0);
    rosenRenderContext->SetShadowColor(SHAPE_MASK_DEFAULT_COLOR);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowColor(),
        OHOS::Rosen::RSColor::FromArgbInt(SHAPE_MASK_DEFAULT_COLOR));
    rosenRenderContext->SetRotation(1.0, 0.0, 1.0);
    rosenRenderContext->DumpInfo();
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetRotationX(), 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetRotationY(), 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetRotation(), 1.0);
}

/**
 * @tc.name: RosenRenderContextTestNew012
 * @tc.desc: SetContentRectToFrame().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew012, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->OnRenderGroupUpdate(true);
    rosenRenderContext->OnSuggestedRenderGroupUpdate(true);
    rosenRenderContext->UpdateRenderGroup(true, false, true);
    auto updateCallback = [](std::vector<float>& param) {};
    rosenRenderContext->AddUpdateCallBack(std::move(updateCallback));
    auto attachCallback = [](int64_t param, bool ret) {};
    rosenRenderContext->AddAttachCallBack(std::move(attachCallback));
    rosenRenderContext->MarkNewFrameAvailable(nullptr);
    rosenRenderContext->OnRenderFitUpdate(RenderFit::BOTTOM_LEFT);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetFrameGravity(), Rosen::Gravity::BOTTOM_LEFT);
    std::string id = "id";
    rosenRenderContext->OnNodeNameUpdate(id);
    rosenRenderContext->SetContentRectToFrame(RectF(0.0, 1.0, 0.0, 1.0));
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetFrame().data_[1], 1.0);
}

/**
 * @tc.name: RosenRenderContextTestNew013
 * @tc.desc: UpdatePlayAnimationValue().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew013, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    auto context = AceType::MakeRefPtr<RenderContext>();
    rosenRenderContext->RegisterSharedTransition(context, true);
    float scaleValue = 1.0; // 4.0 is the value of scaleValue
    rosenRenderContext->UpdatePlayAnimationValue(ClickEffectLevel::LIGHT, scaleValue);
    EXPECT_EQ(scaleValue, 1.0);
    ClickEffectInfo effectInfo;
    effectInfo.level = ClickEffectLevel::LIGHT;
    rosenRenderContext->UpdateClickEffectLevel(effectInfo);
    rosenRenderContext->ClickEffectPlayAnimation(TouchType::DOWN);
    EXPECT_FALSE(rosenRenderContext->isTouchUpFinished_);
    rosenRenderContext->UnregisterSharedTransition(context);
    BorderRadiusProperty property;
    property.SetRadius(Dimension(1.0_px));
    rosenRenderContext->UpdateBorderRadius(property);
    rosenRenderContext->SetActualForegroundColor(Color(SHAPE_MASK_DEFAULT_COLOR));
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetForegroundColor(),
        OHOS::Rosen::RSColor::FromArgbInt(SHAPE_MASK_DEFAULT_COLOR));
}

/**
 * @tc.name: RosenRenderContextTestNew014
 * @tc.desc: UpdateChainedTransition().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew014, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->transitionEffect_ = AceType::MakeRefPtr<RosenTransitionEffect>();
    rosenRenderContext->NotifyTransition(true);
    rosenRenderContext->OnMouseSelectUpdate(true, Color(SHAPE_MASK_DEFAULT_COLOR), Color(SHAPE_MASK_DEFAULT_COLOR));
    rosenRenderContext->MarkDrivenRenderFramePaintState(true);
    rosenRenderContext->MarkDrivenRenderItemIndex(1);
    rosenRenderContext->MarkDrivenRender(true);
    rosenRenderContext->MarkContentChanged(true);
    RefPtr<NG::ChainedTransitionEffect> effect = AceType::MakeRefPtr<NG::ChainedOpacityEffect>(1.0);
    rosenRenderContext->UpdateChainedTransition(effect);
    EXPECT_FALSE(rosenRenderContext->hasDefaultTransition_);
}

/**
 * @tc.name: RosenRenderContextTestNew015
 * @tc.desc: SetFrameGravity().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew015, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->UpdateBackgroundAlign(Alignment::BOTTOM_LEFT);
    rosenRenderContext->UpdateLightUpEffect(1);
    rosenRenderContext->DumpAdvanceInfo();
    std::shared_ptr<RSNode> externalNode = rosenRenderContext->GetRSNode();
    rosenRenderContext->SetRSNode(externalNode);
    ASSERT_NE(rosenRenderContext->GetRSNode(), nullptr);
    rosenRenderContext->ClearDrawCommands();
    rosenRenderContext->SetSurfaceRotation(false);
    rosenRenderContext->rsTextureExport_ = std::make_shared<Rosen::RSTextureExport>(externalNode, 1);
    EXPECT_TRUE(rosenRenderContext->StopTextureExport());
    EXPECT_FALSE(rosenRenderContext->DoTextureExport(1));
    std::string scene = "scene";
    EXPECT_EQ(rosenRenderContext->CalcExpectedFrameRate(scene, 1.0), 0);
    rosenRenderContext->SetFrameGravity(OHOS::Rosen::Gravity::BOTTOM_LEFT);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetFrameGravity(), Rosen::Gravity::BOTTOM_LEFT);
}

/**
 * @tc.name: RosenRenderContextTestNew016
 * @tc.desc: SetBounds().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew016, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetUsingContentRectForRenderFrame(false, true);
    EXPECT_TRUE(rosenRenderContext->adjustRSFrameByContentRect_);
    EXPECT_FALSE(rosenRenderContext->useContentRectForRSFrame_);
    rosenRenderContext->SetSharedTranslate(1.0, 1.0);
    rosenRenderContext->ResetSharedTranslate();
    auto childContextOne = AceType::MakeRefPtr<RosenRenderContext>();
    auto childContextTwo = AceType::MakeRefPtr<RosenRenderContext>();
    rosenRenderContext->AddChild(childContextOne, 0);
    rosenRenderContext->AddChild(childContextTwo, 1);
    rosenRenderContext->RemoveChild(childContextOne);
    rosenRenderContext->ClearChildren();
    EXPECT_EQ(rosenRenderContext->GetRSNode()->children_.size(), 0);
    rosenRenderContext->SetBounds(1.0, 0.0, 1.0, 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetBounds().data_[0], 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetBounds().data_[1], 0.0);
}

/**
 * @tc.name: RosenRenderContextTestNew017
 * @tc.desc: OnBloomUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew017, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    SafeAreaInsets::Inset left { 0, 1 };
    SafeAreaInsets::Inset top { 0, 1 };
    SafeAreaInsets::Inset right { 0, 1 };
    SafeAreaInsets::Inset bottom { 0, 1 };
    SafeAreaInsets safeAreaInsets(left, top, right, bottom);
    auto context = PipelineContext::GetCurrentContext();
    context->UpdateSystemSafeArea(safeAreaInsets);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    OverlayOptions overlay;
    overlay.x = Dimension(1.0_px);
    rosenRenderContext->OnOverlayTextUpdate(overlay);
    rosenRenderContext->OnBloomUpdate(false);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetBloom(), false);
    rosenRenderContext->OnIlluminatedBorderWidthUpdate(Dimension(1.0_px));
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetIlluminatedBorderWidth(), 1.0);
    rosenRenderContext->OnLightIlluminatedUpdate(1);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetIlluminatedType(), 1);
    rosenRenderContext->OnLightColorUpdate(Color(SHAPE_MASK_DEFAULT_COLOR));
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetLightColor(),
        OHOS::Rosen::RSColor::FromArgbInt(SHAPE_MASK_DEFAULT_COLOR));
    MotionPathOption motionPathOption("test");
    rosenRenderContext->OnMotionPathUpdate(motionPathOption);
    rosenRenderContext->OnLightIntensityUpdate(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetLightIntensity(), 1.0);
    TranslateOptions translate { Dimension(1.0_px), Dimension(0.0_px), Dimension(1.0_px) };
    rosenRenderContext->OnLightPositionUpdate(translate);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetLightPosition().data_[0], 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetLightPosition().data_[1], 0.0);
}

/**
 * @tc.name: RosenRenderContextTestNew018
 * @tc.desc: OnClipEdgeUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew018, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    auto pattern = frameNode->GetPattern<PagePattern>();
    PageTransitionOption option;
    option.routeType = RouteType::PUSH;
    auto effect = AceType::MakeRefPtr<PageTransitionEffect>(PageTransitionType::EXIT, option);
    pattern->AddPageTransition(effect);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    auto basicShape = AceType::MakeRefPtr<BasicShape>(BasicShapeType::CIRCLE);
    rosenRenderContext->UpdateClipShape(basicShape);
    rosenRenderContext->UpdateClipMask(basicShape);
    rosenRenderContext->OnClipMaskUpdate(basicShape);
    rosenRenderContext->OnClipShapeUpdate(basicShape);
    rosenRenderContext->PaintClip(SizeF(1.0, 1.0));
    rosenRenderContext->RemoveClipWithRRect();
    RefPtr<NG::ProgressMaskProperty> progress = AceType::MakeRefPtr<NG::ProgressMaskProperty>();
    progress->SetColor(Color::BLUE);
    rosenRenderContext->UpdateProgressMask(progress);
    rosenRenderContext->OnProgressMaskUpdate(progress);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetClipToBounds(), true);
    rosenRenderContext->OnClipEdgeUpdate(false);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetClipToBounds(), false);
}

/**
 * @tc.name: RosenRenderContextTestNew019
 * @tc.desc: PaintGradient().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew019, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    NG::Gradient gradient;
    GradientColor color(Color::BLACK);
    gradient.AddColor(color);
    rosenRenderContext->UpdateLinearGradient(gradient);
    rosenRenderContext->UpdateSweepGradient(gradient);
    rosenRenderContext->UpdateRadialGradient(gradient);
    rosenRenderContext->SetOpacity(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetAlpha(), 1.0);
    rosenRenderContext->OnLinearGradientUpdate(gradient);
    rosenRenderContext->OnSweepGradientUpdate(gradient);
    rosenRenderContext->OnRadialGradientUpdate(gradient);
    rosenRenderContext->PaintGradient(SizeF(1.0, 0.0));
    auto shader = std::make_shared<Rosen::RSShader>();
    rosenRenderContext->SetBackgroundShader(shader);
    TransitionOptions options;
    options.Type = TransitionType::APPEARING;
    auto optionsPtr = std::make_unique<TransitionOptions>(options);
    rosenRenderContext->UpdateTransition(options);
    EXPECT_NE(rosenRenderContext->GetRSTransitionWithoutType(optionsPtr, SizeF(0.0, 1.0)), nullptr);
    rosenRenderContext->CleanTransition();
    rosenRenderContext->SetShadowAlpha(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowAlpha(), 1.0);
    rosenRenderContext->SetShadowOffset(1.0, 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowOffsetX(), 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowOffsetY(), 0.0);
}

using RSModifier = Rosen::ModifierNG::RSModifier;

/**
 * @tc.name: RosenRenderContextTestNew020
 * @tc.desc: OnClipEdgeUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew020, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    auto property = std::make_shared<Rosen::RSProperty<bool>>();
    std::shared_ptr<RSModifier> modifier = std::make_shared<Rosen::ModifierNG::RSBackgroundShaderModifier>();
    modifier->AttachProperty(Rosen::ModifierNG::RSPropertyType::BACKGROUND_SHADER, property);
    rosenRenderContext->AddModifier(modifier);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    BrightnessOption option;
    std::optional<BrightnessOption> brightnessOption = std::make_optional(option);
    rosenRenderContext->OnFgDynamicBrightnessOptionUpdate(brightnessOption);
    rosenRenderContext->OnBgDynamicBrightnessOptionUpdate(brightnessOption);
    rosenRenderContext->OnDynamicLightUpDegreeUpdate(0.0);
    rosenRenderContext->OnDynamicLightUpRateUpdate(1.0);
    rosenRenderContext->OnDynamicDimDegreeUpdate(0.0);
    rosenRenderContext->SetRenderFrameOffset(OffsetF(1.0, 0.0));
    EXPECT_EQ(rosenRenderContext->frameOffset_, OffsetF(1.0, 0.0));
    std::vector<std::pair<float, float>> fractionStops;
    fractionStops.push_back(std::pair<float, float>(0.0f, 1.0f));
    CalcDimension dimensionRadius(0.0);
    NG::LinearGradientBlurPara blurPara(dimensionRadius, fractionStops, NG::GradientDirection::LEFT);
    rosenRenderContext->OnLinearGradientBlurUpdate(blurPara);
    rosenRenderContext->SetShadowRadius(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowRadius(), 1.0);
    rosenRenderContext->OnSystemBarEffectUpdate(false);
    rosenRenderContext->RemoveModifier(modifier);
    rosenRenderContext->UpdateFrontGrayScale(1.0_vp);
    rosenRenderContext->UpdateFrontBrightness(1.0_vp);
    rosenRenderContext->UpdateFrontContrast(1.0_vp);
    rosenRenderContext->UpdateFrontSaturate(1.0_vp);
    rosenRenderContext->UpdateFrontSepia(1.0_vp);
    InvertVariant invert = 0.0f;
    rosenRenderContext->UpdateFrontInvert(invert);
    rosenRenderContext->UpdateFrontHueRotate(0.0);
    rosenRenderContext->UpdateFrontColorBlend(Color::GRAY);
    rosenRenderContext->PaintGraphics();
    rosenRenderContext->SetRotation(1.0, 0.0, 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetRotationX(), 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetRotationY(), 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetRotation(), 1.0);
}

/**
 * @tc.name: RosenRenderContextTestNew021
 * @tc.desc: OnBackBlendApplyTypeUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew021, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    auto property = std::make_shared<Rosen::RSProperty<bool>>();
    std::shared_ptr<RSModifier> modifier = std::make_shared<Rosen::ModifierNG::RSBackgroundShaderModifier>();
    modifier->AttachProperty(Rosen::ModifierNG::RSPropertyType::BACKGROUND_SHADER, property);
    rosenRenderContext->AddModifier(modifier);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    rosenRenderContext->OnBackBlendApplyTypeUpdate(BlendApplyType::OFFSCREEN);
    rosenRenderContext->OnBackBlendModeUpdate(BlendMode::SATURATION);
    rosenRenderContext->AnimateHoverEffectScale(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredScale_);
    MotionBlurOption motionBlurOption;
    motionBlurOption.radius = 5;
    rosenRenderContext->UpdateMotionBlur(motionBlurOption);
    rosenRenderContext->AnimateHoverEffectBoard(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredBoard_);
    rosenRenderContext->UpdateFrontBlurRadius(0.0_px);
    EXPECT_EQ(rosenRenderContext->GetFrontBlurRadius().value(), 0.0_px);
    BlurOption blurOption;
    blurOption.grayscale.emplace_back(0.0);
    blurOption.grayscale.emplace_back(1.0);
    rosenRenderContext->UpdateFrontBlur(1.0_px, blurOption);
    EXPECT_EQ(rosenRenderContext->GetFrontBlurRadius().value(), 1.0_px);
    rosenRenderContext->UpdateBackBlurRadius(0.0_px);
    EXPECT_EQ(rosenRenderContext->GetBackBlurRadius().value(), 0.0_px);
    rosenRenderContext->UpdateBackBlur(1.0_px, blurOption);
    EXPECT_EQ(rosenRenderContext->GetBackBlurRadius().value(), 1.0_px);
}

/**
 * @tc.name: RosenRenderContextTestNew022
 * @tc.desc: OnBackShadowUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew022, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    auto childNode =
        FrameNode::GetOrCreateFrameNode("child", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    auto childRenderContext = InitRosenRenderContext(childNode);
    auto nodeOne = FrameNode::GetOrCreateFrameNode("nodeOne", 0, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto nodeTwo = FrameNode::GetOrCreateFrameNode("nodeTwo", 1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto nodeThree = FrameNode::GetOrCreateFrameNode("nodeThree", 0, []() { return AceType::MakeRefPtr<Pattern>(); });
    std::vector<RefPtr<FrameNode>> childrenOne;
    childrenOne.emplace_back(nodeOne);
    childrenOne.emplace_back(nodeTwo);
    std::vector<RefPtr<FrameNode>> childrenTwo;
    childrenTwo.emplace_back(nodeTwo);
    childrenTwo.emplace_back(nodeThree);
    rosenRenderContext->AddFrameChildren(nullptr, childrenOne);
    rosenRenderContext->RebuildFrame(nullptr, childrenTwo);
    rosenRenderContext->RemoveFrameChildren(nullptr, childrenTwo);
    EXPECT_NE(childRenderContext, nullptr);
    rosenRenderContext->MoveFrame(nullptr, childNode, 0);
    Shadow shadow(1.0, Offset(1.0, 0.0), Color(SHAPE_MASK_DEFAULT_COLOR), ShadowStyle::None);
    shadow.SetIsFilled(true);
    shadow.SetShadowType(ShadowType::COLOR);
    shadow.SetShadowColorStrategy(ShadowColorStrategy::NONE);
    rosenRenderContext->OnBackShadowUpdate(shadow);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowOffsetX(), 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowOffsetY(), 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowColor(),
        OHOS::Rosen::RSColor::FromArgbInt(SHAPE_MASK_DEFAULT_COLOR));
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowMask(), false);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowIsFilled(), true);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowColorStrategy(),
        Rosen::SHADOW_COLOR_STRATEGY::COLOR_STRATEGY_NONE);
}

/**
 * @tc.name: RosenRenderContextTestNew023
 * @tc.desc: OnBackBlendApplyTypeUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew023, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    auto fgModifier = AceType::MakeRefPtr<Modifier>();
    rosenRenderContext->FlushForegroundModifier(fgModifier);
    auto overlayModifier = AceType::MakeRefPtr<Modifier>();
    rosenRenderContext->FlushOverlayModifier(overlayModifier);
    std::function<void(RSCanvas & canvas)> func = [](RSCanvas& canvas) {};
    rosenRenderContext->FlushOverlayDrawFunction(std::move(func));
    auto rect = RectF(1.0, 1.0, 1.0, 1.0);
    RoundRect roundRect;
    roundRect.SetRect(rect);
    roundRect.SetCornerRadius(1.0);
    rosenRenderContext->PaintFocusState(roundRect, Color::BLUE, 1.0_vp, true, true);
    rosenRenderContext->SetShadowRadius(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowRadius(), 1.0);
    rosenRenderContext->SetShadowElevation(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetShadowElevation(), 1.0);
    rosenRenderContext->FlushForegroundDrawFunction(std::move(func));
    auto contentModifier = AceType::MakeRefPtr<Modifier>();
    rosenRenderContext->FlushContentModifier(contentModifier);
    rosenRenderContext->FlushContentDrawFunction(std::move(func));
    rosenRenderContext->ClearFocusState();
    rosenRenderContext->PaintFocusState(1.0_vp, Color::BLACK, 0.0_vp, false);
    EXPECT_FALSE(rosenRenderContext->IsUniRenderEnabled());
    rosenRenderContext->SetOpacity(1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetAlpha(), 1.0);
    rosenRenderContext->PaintAccessibilityFocus(false);
    rosenRenderContext->PaintFocusState(1.0_vp, Color::BLACK, 0.0_vp, true);
    rosenRenderContext->SetFrame(0.0, 1.0, 0.0, 1.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetFrame().data_[1], 1.0);
    rosenRenderContext->SetRenderPivot(1.0, 0.0);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetPivot().x_, 1.0);
}

/**
 * @tc.name: RosenRenderContextTestNew024
 * @tc.desc: AdjustPaintRect().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew024, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    auto parentNode =
        FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    parentNode->AddChild(frameNode);
    rosenRenderContext->OnZIndexUpdate(1);
    auto layoutProperty = AceType::MakeRefPtr<LayoutProperty>();
    layoutProperty->SetUsingPosition(true);
    frameNode->SetLayoutProperty(layoutProperty);
    rosenRenderContext->RecalculatePosition();
    auto geometryNode = frameNode->GetGeometryNode();
    geometryNode->frame_.rect_ = RectF(1.0, 1.0, 1.0, 1.0);
    auto offset = OffsetT<Dimension>(Dimension(1), Dimension(1));
    EdgesParam edges;
    CalcDimension top(1.0, DimensionUnit::VP);
    edges.SetTop(top);
    rosenRenderContext->UpdateOffsetEdges(edges);
    rosenRenderContext->OnPositionEdgesUpdate(edges);
    rosenRenderContext->UpdateOffset(offset);
    rosenRenderContext->RecalculatePosition();
    rosenRenderContext->ResetBlendBorderColor();
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetBorderColor().data_[0],
        OHOS::Rosen::RSColor::FromArgbInt((Color::TRANSPARENT).GetValue()));
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetBorderColor().data_[1],
        OHOS::Rosen::RSColor::FromArgbInt((Color::TRANSPARENT).GetValue()));
    rosenRenderContext->UpdatePositionEdges(edges);
    rosenRenderContext->OnOffsetEdgesUpdate(edges);
    rosenRenderContext->UpdatePosition(offset);
    rosenRenderContext->OnAnchorUpdate(offset);
    rosenRenderContext->BlendBorderColor(Color::BLUE);
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetBorderColor().data_[0],
        OHOS::Rosen::RSColor::FromArgbInt((Color::BLUE).GetValue()));
    EXPECT_EQ(rosenRenderContext->GetRSNode()->GetStagingProperties().GetBorderColor().data_[1],
        OHOS::Rosen::RSColor::FromArgbInt((Color::BLUE).GetValue()));
}

/**
 * @tc.name: RosenRenderContextTestNew025
 * @tc.desc: AdjustPaintRect().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew025, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    RectF rect(10.0f, 10.0f, 10.0f, 10.0f);
    float xRad = 10.0;
    float yRad = 10.0f;
    RoundRect roundRect(rect, xRad, yRad);
    rosenRenderContext->InitAccessibilityFocusModidifer(roundRect, Color::BLACK, 20.0f);
    EXPECT_NE(rosenRenderContext->accessibilityFocusStateModifier_, nullptr);
}

/**
 * @tc.name: RosenRenderContextTestNew027
 * @tc.desc: SetChildBounds().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew027, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    RectF rect(10.0f, 10.0f, 10.0f, 10.0f);
    rosenRenderContext->SetChildBounds(rect);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto bounds = stagingProperties.GetBounds();
    EXPECT_NEAR(bounds[3], 0.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew028
 * @tc.desc: SetFrameGravity().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew028, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetFrameGravity(Rosen::Gravity::LEFT);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto frame = stagingProperties.GetFrameGravity();
    EXPECT_EQ(frame, Rosen::Gravity::LEFT);
}

/**
 * @tc.name: RosenRenderContextTestNew029
 * @tc.desc: GetPositionZ().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew029, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->OnZIndexUpdate(3);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto positionZ = stagingProperties.GetPositionZ();
    EXPECT_NEAR(positionZ, 3.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew030
 * @tc.desc: GetPositionZ().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew030, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->OnZIndexUpdate(3);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto positionZ = stagingProperties.GetPositionZ();
    EXPECT_NEAR(positionZ, 3.0f, 0.01f);
}


/**
 * @tc.name: RosenRenderContextTestNew031
 * @tc.desc: SetRenderPivot().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew031, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetRenderPivot(3, 4);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto pivot = stagingProperties.GetPivot();
    EXPECT_NEAR(pivot[0], 3.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew032
 * @tc.desc: SetPivotZ().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew032, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetPivot(1.0f, 2.0f, 3.0f);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto pivotZ = stagingProperties.GetPivotZ();
    EXPECT_NEAR(pivotZ, 3.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew033
 * @tc.desc: SetPivotZ().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew033, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetPivot(1.0f, 2.0f, 3.0f);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto pivotZ = stagingProperties.GetPivotZ();
    EXPECT_NEAR(pivotZ, 3.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew034
 * @tc.desc: SetRotation().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew034, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetRotation(1.0f, 2.0f, 3.0f);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto rotation = stagingProperties.GetRotation();
    EXPECT_NEAR(rotation, 3.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew035
 * @tc.desc: SetAlphaOffscreen().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew035, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->SetAlphaOffscreen(false);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto alphaOffscreen = stagingProperties.GetAlphaOffscreen();
    EXPECT_EQ(false, alphaOffscreen);
}

/**
 * @tc.name: RosenRenderContextTestNew036
 * @tc.desc: SetCornerRadius().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew036, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    Dimension defaultDimension(10);
    BorderRadiusProperty borderRadius = { defaultDimension, defaultDimension, defaultDimension, defaultDimension };
    rosenRenderContext->SetBorderRadius(borderRadius);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto radius = stagingProperties.GetCornerRadius();
    EXPECT_NEAR(radius[0], 10.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew037
 * @tc.desc: SetCornerRadius().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew037, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    Dimension defaultDimension(10);
    BorderRadiusProperty borderRadius = { defaultDimension, defaultDimension, defaultDimension, defaultDimension };
    rosenRenderContext->SetBorderRadius(borderRadius);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto radius = stagingProperties.GetCornerRadius();
    EXPECT_NEAR(radius[0], 10.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew038
 * @tc.desc: SetBorderWidth().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew038, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    BorderWidthProperty borderWidthProperty = { 1.0_vp, 1.0_vp, 1.0_vp, 1.0_vp };
    rosenRenderContext->SetBorderWidth(borderWidthProperty);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto borderWidth = stagingProperties.GetBorderWidth();
    EXPECT_NEAR(borderWidth[0], 1.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew039
 * @tc.desc: OnBorderStyleUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew039, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    std::optional<BorderStyle> borderStyle = BorderStyle::DASHED;
    auto borderStyleProperty = NG::BorderStyleProperty({ borderStyle, borderStyle, borderStyle, borderStyle });
    rosenRenderContext->OnBorderStyleUpdate(borderStyleProperty);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto borderStyleRet = stagingProperties.GetBorderStyle();
    EXPECT_EQ(borderStyleRet[0], 1);
}

/**
 * @tc.name: RosenRenderContextTestNew040
 * @tc.desc: GetBorderDashWidth().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew040, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    BorderWidthProperty borderWidth({ 1.0_vp, 1.0_vp, 1.0_vp, 1.0_vp });
    rosenRenderContext->SetDashWidth(borderWidth);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto borderDashWidth = stagingProperties.GetBorderDashWidth();
    EXPECT_NEAR(borderDashWidth[0], 1.0f, 0.01f);
}

/**
 * @tc.name: RSUIContext002
 * @tc.desc: Test RSUIContext001 Func.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RSUIContext002, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    ASSERT_NE(frameNode, nullptr);
    ComponentSnapshot snapshot;
    snapshot.SetRSUIContext(frameNode, nullptr);
    auto pipeline = MockPipelineContext::GetCurrentContext();
    auto rsUIContext = snapshot.GetRSUIContext(pipeline);
    EXPECT_EQ(rsUIContext, nullptr);
}

/**
 * @tc.name: RosenRenderContextTestNew041
 * @tc.desc: GetClipToFrame().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew041, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->OnClipEdgeUpdate(false);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto clipToFrame = stagingProperties.GetClipToFrame();
    EXPECT_EQ(clipToFrame, false);
}

/**
 * @tc.name: RosenRenderContextTestNew042
 * @tc.desc: GetClipToFrame().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew042, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->OnClipEdgeUpdate(false);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto clipToFrame = stagingProperties.GetClipToFrame();
    EXPECT_EQ(clipToFrame, false);
}

/**
 * @tc.name: RosenRenderContextTestNew043
 * @tc.desc: OnSphericalEffectUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew043, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->OnSphericalEffectUpdate(10);
    auto rsNdoe = rosenRenderContext->GetRSNode();
    auto stagingProperties = rsNdoe->GetStagingProperties();
    auto spherizeDegree = stagingProperties.GetSpherizeDegree();
    EXPECT_NEAR(spherizeDegree, 10.0f, 0.01f);
}

/**
 * @tc.name: RosenRenderContextTestNew045
 * @tc.desc: UpdateFrontBlurStyle().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew044, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    g_isConfigChangePerform = true;
    BlurStyleOption blur;
    blur.blurStyle = BlurStyle::NO_MATERIAL;
    SysOptions sysOptions;
    auto pattern = frameNode->GetPattern<Pattern>();
    ASSERT_NE(pattern, nullptr);
    rosenRenderContext->UpdateFrontBlurStyle(blur, sysOptions);
    g_isConfigChangePerform = false;
    std::string blurStyleStr = pattern->GetResCacheMapByKey("foregroundBlurStyle.blurStyle");
    EXPECT_EQ(blurStyleStr, "");
}

/**
 * @tc.name: RosenRenderContextTestNew045
 * @tc.desc: CanNodeBeDeleted().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew045, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);

    bool canBeDeleted = false;
    auto pageNode = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>(), true);
    canBeDeleted = rosenRenderContext->CanNodeBeDeleted(pageNode);
    EXPECT_EQ(canBeDeleted, false);
    canBeDeleted = rosenRenderContext->CanNodeBeDeleted(nullptr);
    EXPECT_EQ(canBeDeleted, false);
}

/**
 * @tc.name: RosenRenderContextTestNew046
 * @tc.desc: CancelTask().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew046, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    EXPECT_EQ(rosenRenderContext->CancelDynamicImageLoadingTasks(), true);
    rosenRenderContext->pendingDecodeTask_.Reset([]() {});
    rosenRenderContext->pendingUITask_.Reset([]() {});
    rosenRenderContext->OnPaintBackgroundDynamic();
    EXPECT_EQ(rosenRenderContext->CancelDynamicImageLoadingTasks(), true);
    {
        if (!rosenRenderContext->taskMtx_.try_lock_for(std::chrono::milliseconds(MAX_WAITING_TIME_FOR_TASKS))) {
            return;
        }
        // Adopt the already acquired lock
        std::scoped_lock lock(std::adopt_lock, rosenRenderContext->taskMtx_);
        rosenRenderContext->OnPaintBackgroundDynamic();
        EXPECT_EQ(rosenRenderContext->CancelDynamicImageLoadingTasks(), false);
    }
}

/**
 * @tc.name: RosenRenderContextTestNew047
 * @tc.desc: ScheduleBackgroundPaint().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew047, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->rsNode_ = nullptr;
    rosenRenderContext->pendingDecodeTask_.Reset({});
    rosenRenderContext->bgImage_ = AceType::MakeRefPtr<PixelMapImage>();
    rosenRenderContext->ScheduleBackgroundPaint();
    EXPECT_FALSE(rosenRenderContext->pendingDecodeTask_);
    auto bgImage = AceType::MakeRefPtr<MockAnimatedImage>();
    rosenRenderContext->bgImage_ = bgImage;
    rosenRenderContext->UpdateBackgroundImageSyncMode(true);
    bgImage->firstPixelMap = nullptr;
    rosenRenderContext->ScheduleBackgroundPaint();
    EXPECT_FALSE(rosenRenderContext->pendingDecodeTask_);
    bgImage->firstPixelMap = AceType::MakeRefPtr<MockPixelMap>();
    rosenRenderContext->ScheduleBackgroundPaint();
    EXPECT_FALSE(rosenRenderContext->pendingDecodeTask_);
    rosenRenderContext->UpdateBackgroundImageSyncMode(false);
    rosenRenderContext->ScheduleBackgroundPaint();
    EXPECT_TRUE(rosenRenderContext->pendingDecodeTask_);
}

/**
 * @tc.name: RosenRenderContextTestNew048
 * @tc.desc: OnBackBlendApplyTypeUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew048, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    auto property = std::make_shared<Rosen::RSProperty<bool>>();
    std::shared_ptr<RSModifier> modifier = std::make_shared<Rosen::ModifierNG::RSBackgroundShaderModifier>();
    modifier->AttachProperty(Rosen::ModifierNG::RSPropertyType::BACKGROUND_SHADER, property);
    rosenRenderContext->AddModifier(modifier);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    rosenRenderContext->OnBackBlendApplyTypeUpdate(BlendApplyType::FAST);
    rosenRenderContext->OnBackBlendModeUpdate(BlendMode::SATURATION);
    rosenRenderContext->AnimateHoverEffectScale(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredScale_);
    MotionBlurOption motionBlurOption;
    motionBlurOption.radius = 5;
    rosenRenderContext->UpdateMotionBlur(motionBlurOption);
    rosenRenderContext->AnimateHoverEffectBoard(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredBoard_);
    rosenRenderContext->UpdateFrontBlurRadius(0.0_px);
    EXPECT_EQ(rosenRenderContext->GetFrontBlurRadius().value(), 0.0_px);
}

/**
 * @tc.name: RosenRenderContextTestNew049
 * @tc.desc: OnBackBlendApplyTypeUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew049, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    auto property = std::make_shared<Rosen::RSProperty<bool>>();
    std::shared_ptr<RSModifier> modifier = std::make_shared<Rosen::ModifierNG::RSBackgroundShaderModifier>();
    modifier->AttachProperty(Rosen::ModifierNG::RSPropertyType::BACKGROUND_SHADER, property);
    rosenRenderContext->AddModifier(modifier);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    rosenRenderContext->OnBackBlendApplyTypeUpdate(BlendApplyType::OFFSCREEN_WITH_BACKGROUND);
    rosenRenderContext->OnBackBlendModeUpdate(BlendMode::SATURATION);
    rosenRenderContext->AnimateHoverEffectScale(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredScale_);
    MotionBlurOption motionBlurOption;
    motionBlurOption.radius = 5;
    rosenRenderContext->UpdateMotionBlur(motionBlurOption);
    rosenRenderContext->AnimateHoverEffectBoard(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredBoard_);
    rosenRenderContext->UpdateFrontBlurRadius(0.0_px);
    EXPECT_EQ(rosenRenderContext->GetFrontBlurRadius().value(), 0.0_px);
}

/**
 * @tc.name: RosenRenderContextTestNew050
 * @tc.desc: OnBackBlendApplyTypeUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew050, TestSize.Level1)
{
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    auto property = std::make_shared<Rosen::RSProperty<bool>>();
    std::shared_ptr<RSModifier> modifier = std::make_shared<Rosen::ModifierNG::RSBackgroundShaderModifier>();
    modifier->AttachProperty(Rosen::ModifierNG::RSPropertyType::BACKGROUND_SHADER, property);
    auto container = MockContainer::Current();
    ASSERT_NE(container, nullptr);
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_EIGHTEEN));
    rosenRenderContext->AddModifier(modifier);
    rosenRenderContext->paintRect_ = RectF(1.0, 1.0, 1.0, 1.0);
    rosenRenderContext->OnBackBlendApplyTypeUpdate(BlendApplyType::MAX);
    rosenRenderContext->OnBackBlendModeUpdate(BlendMode::SATURATION);
    rosenRenderContext->AnimateHoverEffectScale(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredScale_);
    MotionBlurOption motionBlurOption;
    motionBlurOption.radius = 5;
    rosenRenderContext->UpdateMotionBlur(motionBlurOption);
    rosenRenderContext->AnimateHoverEffectBoard(true);
    EXPECT_FALSE(rosenRenderContext->isHoveredBoard_);
    rosenRenderContext->UpdateFrontBlurRadius(0.0_px);
    EXPECT_EQ(rosenRenderContext->GetFrontBlurRadius().value(), 0.0_px);
}

/**
 * @tc.name: RosenRenderContextTestNew051
 * @tc.desc: PaintOverlayText should refresh draw region when overlay content is cleared.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RosenRenderContextTestNew051, TestSize.Level1)
{
    constexpr uint32_t DRAW_REGION_OVERLAY_TEXT_INDEX = 4;
    constexpr float PAINT_WIDTH = 200.0f;
    constexpr float PAINT_HEIGHT = 100.0f;
    auto frameNode =
        FrameNode::GetOrCreateFrameNode("frame", -1, []() { return AceType::MakeRefPtr<PagePattern>(nullptr); });
    RefPtr<RosenRenderContext> rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, PAINT_WIDTH, PAINT_HEIGHT);

    OverlayOptions overlay;
    overlay.content = "overlay-text";
    overlay.align = Alignment::TOP_LEFT;
    overlay.x = Dimension(10.0_px);
    overlay.y = Dimension(5.0_px);
    RefPtr<RenderContext> renderContext = AceType::DynamicCast<RenderContext>(rosenRenderContext);
    ASSERT_NE(renderContext, nullptr);
    renderContext->UpdateOverlayText(overlay);
    ASSERT_NE(rosenRenderContext->overlayTextModifier_, nullptr);
    ASSERT_NE(rosenRenderContext->drawRegionRects_[DRAW_REGION_OVERLAY_TEXT_INDEX], nullptr);
    auto expectedTextRect = std::make_shared<Rosen::RectF>(
        *rosenRenderContext->drawRegionRects_[DRAW_REGION_OVERLAY_TEXT_INDEX]);

    auto expectedClearRect = std::make_shared<Rosen::RectF>(0.0f, 0.0f, PAINT_WIDTH, PAINT_HEIGHT);
    EXPECT_FALSE(*rosenRenderContext->drawRegionRects_[DRAW_REGION_OVERLAY_TEXT_INDEX] == *expectedClearRect);

    overlay.content = "";
    renderContext->UpdateOverlayText(overlay);
    ASSERT_NE(rosenRenderContext->overlayTextModifier_, nullptr);
    ASSERT_NE(rosenRenderContext->drawRegionRects_[DRAW_REGION_OVERLAY_TEXT_INDEX], nullptr);
    EXPECT_TRUE(*rosenRenderContext->drawRegionRects_[DRAW_REGION_OVERLAY_TEXT_INDEX] == *expectedTextRect);
    EXPECT_FALSE(*rosenRenderContext->drawRegionRects_[DRAW_REGION_OVERLAY_TEXT_INDEX] == *expectedClearRect);
}
} // namespace OHOS::Ace::NG
