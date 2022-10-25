/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/pipeline_ng/pipeline_context.h"

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <memory>

#include "base/geometry/ng/offset_t.h"
#include "base/log/ace_trace.h"
#include "base/log/ace_tracker.h"
#include "base/log/event_report.h"
#include "base/log/frame_report.h"
#include "base/memory/referenced.h"
#include "base/thread/task_executor.h"
#include "base/utils/utils.h"
#include "core/animation/scheduler.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/common/manager_interface.h"
#include "core/common/text_field_manager.h"
#include "core/common/thread_checker.h"
#include "core/common/window.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/container_modal/container_modal_view.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace {
constexpr int32_t TIME_THRESHOLD = 2 * 1000000; // 3 millisecond
}

namespace OHOS::Ace::NG {

PipelineContext::PipelineContext(std::unique_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
    RefPtr<AssetManager> assetManager, RefPtr<PlatformResRegister> platformResRegister,
    const RefPtr<Frontend>& frontend, int32_t instanceId)
    : PipelineBase(std::move(window), std::move(taskExecutor), std::move(assetManager), frontend, instanceId)
{}

PipelineContext::PipelineContext(std::unique_ptr<Window> window, RefPtr<TaskExecutor> taskExecutor,
    RefPtr<AssetManager> assetManager, const RefPtr<Frontend>& frontend, int32_t instanceId)
    : PipelineBase(std::move(window), std::move(taskExecutor), std::move(assetManager), frontend, instanceId)
{}

RefPtr<PipelineContext> PipelineContext::GetCurrentContext()
{
    auto currentContainer = Container::Current();
    CHECK_NULL_RETURN(currentContainer, nullptr);
    return DynamicCast<PipelineContext>(currentContainer->GetPipelineContext());
}

float PipelineContext::GetCurrentRootWidth()
{
    auto context = GetCurrentContext();
    CHECK_NULL_RETURN(context, 0.0f);
    return static_cast<float>(context->rootWidth_);
}

float PipelineContext::GetCurrentRootHeight()
{
    auto context = GetCurrentContext();
    CHECK_NULL_RETURN(context, 0.0f);
    return static_cast<float>(context->rootHeight_);
}

void PipelineContext::AddDirtyCustomNode(const RefPtr<UINode>& dirtyNode)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirtyNode);
    dirtyNodes_.emplace(dirtyNode);
    hasIdleTasks_ = true;
    window_->RequestFrame();
}

void PipelineContext::AddDirtyLayoutNode(const RefPtr<FrameNode>& dirty)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirty);
    taskScheduler_.AddDirtyLayoutNode(dirty);
    ForceLayoutForImplicitAnimation();
    hasIdleTasks_ = true;
    window_->RequestFrame();
}

void PipelineContext::AddDirtyRenderNode(const RefPtr<FrameNode>& dirty)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(dirty);
    taskScheduler_.AddDirtyRenderNode(dirty);
    hasIdleTasks_ = true;
    window_->RequestFrame();
}

void PipelineContext::FlushDirtyNodeUpdate()
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().BeginFlushBuild();
    }

    decltype(dirtyNodes_) dirtyNodes(std::move(dirtyNodes_));
    for (const auto& node : dirtyNodes) {
        if (AceType::InstanceOf<NG::CustomNode>(node)) {
            auto customNode = AceType::DynamicCast<NG::CustomNode>(node);
            customNode->Update();
        } else if (AceType::InstanceOf<NG::CustomMeasureLayoutNode>(node)) {
            auto customNode = AceType::DynamicCast<NG::CustomMeasureLayoutNode>(node);
            customNode->Update();
        }
    }

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndFlushBuild();
    }
}

uint32_t PipelineContext::AddScheduleTask(const RefPtr<ScheduleTask>& task)
{
    CHECK_RUN_ON(UI);
    scheduleTasks_.try_emplace(++nextScheduleTaskId_, task);
    window_->RequestFrame();
    return nextScheduleTaskId_;
}

void PipelineContext::FlushVsync(uint64_t nanoTimestamp, uint32_t frameCount)
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    static const std::string abilityName = AceApplicationInfo::GetInstance().GetProcessName().empty()
                                               ? AceApplicationInfo::GetInstance().GetPackageName()
                                               : AceApplicationInfo::GetInstance().GetProcessName();
    window_->RecordFrameTime(nanoTimestamp, abilityName);
    FlushAnimation(GetTimeFromExternalTimer());
    FlushBuild();
    if (isEtsCard_ && drawDelegate_) {
        auto renderContext = AceType::DynamicCast<NG::RenderContext>(rootNode_->GetRenderContext());
        drawDelegate_->DrawRSFrame(renderContext);
        drawDelegate_ = nullptr;
    }
    FlushTouchEvents();
    taskScheduler_.FlushTask();
    auto hasAninmation = window_->FlushCustomAnimation(nanoTimestamp);
    if (hasAninmation) {
        RequestFrame();
    }
    FlushMessages();
    FlushFocus();
}

void PipelineContext::FlushAnimation(uint64_t nanoTimestamp)
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    if (scheduleTasks_.empty()) {
        return;
    }

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().BeginFlushAnimation();
    }

    decltype(scheduleTasks_) temp(std::move(scheduleTasks_));
    for (const auto& [id, weakTask] : temp) {
        auto task = weakTask.Upgrade();
        if (task) {
            task->OnFrame(nanoTimestamp);
        }
    }

    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndFlushAnimation();
    }
}

void PipelineContext::FlushMessages()
{
    ACE_FUNCTION_TRACE();
    window_->FlushTasks();
}

void PipelineContext::FlushFocus()
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACK();
    ACE_FUNCTION_TRACE();
    auto focusNode = dirtyFocusNode_.Upgrade();
    if (!focusNode || focusNode->GetFocusType() != FocusType::NODE) {
        dirtyFocusNode_.Reset();
    } else {
        auto focusNodeHub = focusNode->GetFocusHub();
        if (focusNodeHub) {
            focusNodeHub->RequestFocusImmediately();
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        return;
    }
    auto focusScope = dirtyFocusScope_.Upgrade();
    if (!focusScope || focusScope->GetFocusType() != FocusType::SCOPE) {
        dirtyFocusScope_.Reset();
    } else {
        auto focusScopeHub = focusScope->GetFocusHub();
        if (focusScopeHub) {
            focusScopeHub->RequestFocusImmediately();
        }
        dirtyFocusNode_.Reset();
        dirtyFocusScope_.Reset();
        return;
    }
    if (!RequestDefaultFocus()) {
        if (rootNode_ && !rootNode_->GetFocusHub()->IsCurrentFocus()) {
            rootNode_->GetFocusHub()->RequestFocusImmediately();
        }
    }
}

void PipelineContext::FlushPipelineImmediately()
{
    CHECK_RUN_ON(UI);
    ACE_FUNCTION_TRACE();
    FlushPipelineWithoutAnimation();
}

void PipelineContext::FlushPipelineWithoutAnimation()
{
    FlushBuild();
    FlushTouchEvents();
    taskScheduler_.FlushTask();
    FlushMessages();
    FlushFocus();
}

void PipelineContext::FlushBuild()
{
    FlushDirtyNodeUpdate();
    FlushBuildFinishCallbacks();
}

void PipelineContext::FlushBuildFinishCallbacks()
{
    decltype(buildFinishCallbacks_) buildFinishCallbacks(std::move(buildFinishCallbacks_));
    for (const auto& func : buildFinishCallbacks) {
        if (func) {
            func();
        }
    }
}

void PipelineContext::SetupRootElement()
{
    CHECK_RUN_ON(UI);
    rootNode_ = FrameNode::CreateFrameNodeWithTree(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<RootPattern>());
    rootNode_->SetHostRootId(GetInstanceId());
    rootNode_->SetHostPageId(-1);
    CalcSize idealSize { CalcLength(rootWidth_), CalcLength(rootHeight_) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    rootNode_->UpdateLayoutConstraint(layoutConstraint);
    auto rootFocusHub = rootNode_->GetOrCreateFocusHub();
    rootFocusHub->SetFocusType(FocusType::SCOPE);
    rootFocusHub->SetFocusable(true);
    window_->SetRootFrameNode(rootNode_);
    rootNode_->AttachToMainTree();

    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), MakeRefPtr<StagePattern>());
    if (windowModal_ == WindowModal::CONTAINER_MODAL) {
        rootNode_->AddChild(ContainerModalView::Create(stageNode));
    } else {
        rootNode_->AddChild(stageNode);
    }
    stageManager_ = MakeRefPtr<StageManager>(stageNode);
    overlayManager_ = MakeRefPtr<OverlayManager>(rootNode_);
    fullScreenManager_ = MakeRefPtr<FullScreenManager>(rootNode_);
    selectOverlayManager_ = MakeRefPtr<SelectOverlayManager>(rootNode_);
    dragDropManager_ = MakeRefPtr<DragDropManager>();
    LOGI("SetupRootElement success!");
}

const RefPtr<StageManager>& PipelineContext::GetStageManager()
{
    return stageManager_;
}

const RefPtr<OverlayManager>& PipelineContext::GetOverlayManager()
{
    return overlayManager_;
}

const RefPtr<FullScreenManager>& PipelineContext::GetFullScreenManager()
{
    return fullScreenManager_;
}

void PipelineContext::SetRootRect(double width, double height, double offset)
{
    LOGI("SetRootRect width %{public}f, height %{public}f, %{public}f", width, height, offset);
    CHECK_RUN_ON(UI);
    UpdateRootSizeAndScale(width, height);
    if (!rootNode_) {
        LOGE("rootNode_ is nullptr");
        return;
    }
    GridSystemManager::GetInstance().SetWindowInfo(rootWidth_, density_, dipScale_);
    GridSystemManager::GetInstance().OnSurfaceChanged(width);
    SizeF sizeF { static_cast<float>(width), static_cast<float>(height) };
    if (rootNode_->GetGeometryNode()->GetFrameSize() != sizeF) {
        CalcSize idealSize { CalcLength(width), CalcLength(height) };
        MeasureProperty layoutConstraint;
        layoutConstraint.selfIdealSize = idealSize;
        layoutConstraint.maxSize = idealSize;
        rootNode_->UpdateLayoutConstraint(layoutConstraint);
        rootNode_->MarkDirtyNode();
    }
    if (rootNode_->GetGeometryNode()->GetFrameOffset().GetY() != offset) {
        OffsetF newOffset = rootNode_->GetGeometryNode()->GetFrameOffset();
        newOffset.SetY(static_cast<float>(offset));
        rootNode_->GetGeometryNode()->SetMarginFrameOffset(newOffset);
        auto rootContext = rootNode_->GetRenderContext();
        rootContext->SyncGeometryProperties(RawPtr(rootNode_->GetGeometryNode()));
        RequestFrame();
    }
}

void PipelineContext::OnVirtualKeyboardHeightChange(float keyboardHeight)
{
    CHECK_RUN_ON(UI);
    float positionY = 0;
    auto manager = PipelineBase::GetTextFieldManager();
    if (manager) {
        positionY = static_cast<float>(manager->GetClickPosition().GetY());
    }
    auto rootSize = rootNode_->GetGeometryNode()->GetFrameSize();
    float offsetFix = (rootSize.Height() - positionY) > 100.0 ? keyboardHeight - (rootSize.Height() - positionY) / 2.0
                                                              : keyboardHeight;
    LOGI("OnVirtualKeyboardAreaChange positionY:%{public}f safeArea:%{public}f offsetFix:%{public}f", positionY,
        (rootSize.Height() - keyboardHeight), offsetFix);
    if (NearZero(keyboardHeight)) {
        SetRootRect(rootSize.Width(), rootSize.Height(), 0);
    } else if (positionY > (rootSize.Height() - keyboardHeight) && offsetFix > 0.0) {
        SetRootRect(rootSize.Width(), rootSize.Height(), -offsetFix);
    }
}

bool PipelineContext::OnBackPressed()
{
    LOGD("OnBackPressed");
    CHECK_RUN_ON(PLATFORM);
    auto frontend = weakFrontend_.Upgrade();
    if (!frontend) {
        // return back.
        return false;
    }

    auto result = false;
    taskExecutor_->PostSyncTask(
        [weakFrontend = weakFrontend_, weakPipelineContext = WeakClaim(this), &result]() {
            auto frontend = weakFrontend.Upgrade();
            if (!frontend) {
                LOGW("frontend is nullptr");
                result = false;
                return;
            }
            result = frontend->OnBackPressed();
        },
        TaskExecutor::TaskType::JS);

    if (result) {
        // user accept
        LOGI("CallRouterBackToPopPage(): frontend accept");
        return true;
    }
    LOGI("CallRouterBackToPopPage(): return platform consumed");
    return false;
}

void PipelineContext::OnTouchEvent(const TouchEvent& point, bool isSubPipe)
{
    CHECK_RUN_ON(UI);
    auto scalePoint = point.CreateScalePoint(GetViewScale());
    LOGD("AceTouchEvent: x = %{public}f, y = %{public}f, type = %{public}zu", scalePoint.x, scalePoint.y,
        scalePoint.type);
    if (scalePoint.type == TouchType::DOWN) {
        LOGD("receive touch down event, first use touch test to collect touch event target");
        TouchRestrict touchRestrict { TouchRestrict::NONE };
        touchRestrict.sourceType = point.sourceType;
        eventManager_->TouchTest(scalePoint, rootNode_, touchRestrict, GetPluginEventOffset(), viewScale_, isSubPipe);

        for (const auto& weakContext : touchPluginPipelineContext_) {
            auto pipelineContext = DynamicCast<OHOS::Ace::PipelineBase>(weakContext.Upgrade());
            if (!pipelineContext) {
                continue;
            }
            auto pluginPoint =
                point.UpdateScalePoint(viewScale_, static_cast<float>(pipelineContext->GetPluginEventOffset().GetX()),
                    static_cast<float>(pipelineContext->GetPluginEventOffset().GetY()), point.id);
            auto eventManager = pipelineContext->GetEventManager();
            if (eventManager) {
                eventManager->SetInstanceId(pipelineContext->GetInstanceId());
            }

            pipelineContext->OnTouchEvent(pluginPoint, true);
        }
    }
    if (isSubPipe) {
        return;
    }

    if (scalePoint.type == TouchType::MOVE) {
        touchEvents_.emplace_back(point);
        hasIdleTasks_ = true;
        window_->RequestFrame();
        return;
    }

    std::optional<TouchEvent> lastMoveEvent;
    if (scalePoint.type == TouchType::UP && !touchEvents_.empty()) {
        for (auto iter = touchEvents_.begin(); iter != touchEvents_.end(); ++iter) {
            auto movePoint = (*iter).CreateScalePoint(GetViewScale());
            if (scalePoint.id == movePoint.id) {
                lastMoveEvent = movePoint;
                touchEvents_.erase(iter++);
            }
        }
        if (lastMoveEvent.has_value()) {
            eventManager_->DispatchTouchEvent(lastMoveEvent.value());
        }
    }

    eventManager_->DispatchTouchEvent(scalePoint);
    hasIdleTasks_ = true;
    window_->RequestFrame();
}

void PipelineContext::OnSurfaceDensityChanged(double density)
{
    CHECK_RUN_ON(UI);
    LOGI("OnSurfaceDensityChanged density_(%{public}lf)", density_);
    LOGI("OnSurfaceDensityChanged dipScale_(%{public}lf)", dipScale_);
    density_ = density;
    if (!NearZero(viewScale_)) {
        LOGI("OnSurfaceDensityChanged viewScale_(%{public}lf)", viewScale_);
        dipScale_ = density_ / viewScale_;
    }
}

bool PipelineContext::OnDumpInfo(const std::vector<std::string>& params) const
{
    ACE_DCHECK(!params.empty());

    if (params[0] == "-element") {
        if (params.size() > 1 && params[1] == "-lastpage") {
            auto lastPage = stageManager_->GetLastPage();
            if (lastPage) {
                lastPage->DumpTree(0);
            }
        } else {
            rootNode_->DumpTree(0);
        }
    } else if (params[0] == "-render") {
    } else if (params[0] == "-focus") {
        if (rootNode_->GetFocusHub()) {
            rootNode_->GetFocusHub()->DumpFocusTree(0);
        }
    } else if (params[0] == "-layer") {
    } else if (params[0] == "-frontend") {
#ifndef WEARABLE_PRODUCT
    } else if (params[0] == "-multimodal") {
#endif
    } else if (params[0] == "-accessibility" || params[0] == "-inspector") {
        auto accessibilityManager = GetAccessibilityManager();
        if (accessibilityManager) {
            accessibilityManager->OnDumpInfo(params);
        }
    } else if (params[0] == "-rotation" && params.size() >= 2) {
    } else if (params[0] == "-animationscale" && params.size() >= 2) {
    } else if (params[0] == "-velocityscale" && params.size() >= 2) {
    } else if (params[0] == "-scrollfriction" && params.size() >= 2) {
    } else if (params[0] == "-threadstuck" && params.size() >= 3) {
    } else {
        return false;
    }
    return true;
}

void PipelineContext::FlushTouchEvents()
{
    CHECK_RUN_ON(UI);
    if (!rootNode_) {
        LOGE("root node is nullptr");
        return;
    }
    {
        eventManager_->FlushTouchEventsBegin(touchEvents_);
        std::unordered_set<int32_t> moveEventIds;
        decltype(touchEvents_) touchEvents(std::move(touchEvents_));
        if (touchEvents.empty()) {
            return;
        }
        std::list<TouchEvent> touchPoints;
        for (auto iter = touchEvents.rbegin(); iter != touchEvents.rend(); ++iter) {
            auto scalePoint = (*iter).CreateScalePoint(GetViewScale());
            auto result = moveEventIds.emplace(scalePoint.id);
            if (result.second) {
                touchPoints.emplace_front(scalePoint);
            }
        }
        auto maxSize = touchPoints.size();
        for (auto iter = touchPoints.rbegin(); iter != touchPoints.rend(); ++iter) {
            maxSize--;
            if (maxSize == 0) {
                eventManager_->FlushTouchEventsEnd(touchPoints);
            }
            eventManager_->DispatchTouchEvent(*iter);
        }
    }
}

void PipelineContext::OnMouseEvent(const MouseEvent& event)
{
    CHECK_RUN_ON(UI);

    if ((event.action == MouseAction::RELEASE || event.action == MouseAction::PRESS ||
            event.action == MouseAction::MOVE) &&
        (event.button == MouseButton::LEFT_BUTTON || event.pressedButtons == MOUSE_PRESS_LEFT)) {
        auto touchPoint = event.CreateTouchPoint();
        LOGD("Mouse event to touch: button is %{public}d, action is %{public}d", event.button, event.action);
        OnTouchEvent(touchPoint);
    }

    CHECK_NULL_VOID(rootNode_);
    auto scaleEvent = event.CreateScaleEvent(viewScale_);
    LOGD(
        "MouseEvent (x,y): (%{public}f,%{public}f), button: %{public}d, action: %{public}d, pressedButtons: %{public}d",
        scaleEvent.x, scaleEvent.y, scaleEvent.action, scaleEvent.button, scaleEvent.pressedButtons);
    eventManager_->MouseTest(scaleEvent, rootNode_);
    eventManager_->DispatchMouseEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverEventNG(scaleEvent);
    eventManager_->DispatchMouseHoverAnimationNG(scaleEvent);
    window_->RequestFrame();
}

bool PipelineContext::OnKeyEvent(const KeyEvent& event)
{
    // Need update while key tab pressed
    if (!isNeedShowFocus_ && event.action == KeyAction::DOWN &&
        (event.code == KeyCode::KEY_TAB || event.code == KeyCode::KEY_DPAD_UP || event.code == KeyCode::KEY_DPAD_LEFT ||
            event.code == KeyCode::KEY_DPAD_DOWN || event.code == KeyCode::KEY_DPAD_RIGHT)) {
        isNeedShowFocus_ = true;
        FlushFocus();
        return true;
    }
    auto lastPage = stageManager_->GetLastPage();
    CHECK_NULL_RETURN(lastPage, false);
    if (!eventManager_->DispatchTabIndexEventNG(event, rootNode_, lastPage)) {
        return eventManager_->DispatchKeyEventNG(event, rootNode_);
    }
    return true;
}

bool PipelineContext::RequestDefaultFocus()
{
    auto lastPage = stageManager_->GetLastPage();
    CHECK_NULL_RETURN(lastPage, false);
    auto lastPageFocusHub = lastPage->GetFocusHub();
    CHECK_NULL_RETURN(lastPageFocusHub, false);
    if (lastPageFocusHub->IsDefaultHasFocused()) {
        return false;
    }
    auto defaultFocusNode = lastPageFocusHub->GetChildFocusNodeByType();
    if (!defaultFocusNode) {
        return false;
    }
    if (!defaultFocusNode->IsFocusableWholePath()) {
        return false;
    }
    lastPageFocusHub->SetIsDefaultHasFocused(true);
    LOGD("Focus: request default focus node %{public}s", defaultFocusNode->GetFrameName().c_str());
    return defaultFocusNode->RequestFocusImmediately();
}

bool PipelineContext::RequestFocus(const std::string& targetNodeId)
{
    CHECK_NULL_RETURN(rootNode_, false);
    auto focusHub = rootNode_->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->RequestFocusImmediatelyById(targetNodeId);
}

void PipelineContext::AddDirtyFocus(const RefPtr<FrameNode>& node)
{
    CHECK_RUN_ON(UI);
    CHECK_NULL_VOID(node);
    if (node->GetFocusType() == FocusType::NODE) {
        dirtyFocusNode_ = WeakClaim(RawPtr(node));
    } else {
        dirtyFocusScope_ = WeakClaim(RawPtr(node));
    }
    window_->RequestFrame();
}

void PipelineContext::OnAxisEvent(const AxisEvent& event)
{
    // Need develop here: CTRL+AXIS = Pinch event

    auto scaleEvent = event.CreateScaleEvent(viewScale_);
    LOGD("AxisEvent (x,y): (%{public}f,%{public}f), horizontalAxis: %{public}f, verticalAxis: %{public}f, action: "
         "%{public}d",
        scaleEvent.x, scaleEvent.y, scaleEvent.horizontalAxis, scaleEvent.verticalAxis, scaleEvent.action);

    // Need update here: zoom(ctrl+axis) event

    if (event.action == AxisAction::BEGIN) {
        TouchRestrict touchRestrict { TouchRestrict::NONE };
        eventManager_->TouchTest(scaleEvent, rootNode_, touchRestrict);
    }
    eventManager_->DispatchTouchEvent(scaleEvent);

    if (event.action == AxisAction::BEGIN || event.action == AxisAction::UPDATE) {
        eventManager_->AxisTest(scaleEvent, rootNode_);
        eventManager_->DispatchAxisEvent(scaleEvent);
    }
}

void PipelineContext::OnShow()
{
    CHECK_RUN_ON(UI);
    window_->OnShow();
    window_->RequestFrame();
    FlushWindowStateChangedCallback(true);
}

void PipelineContext::OnHide()
{
    CHECK_RUN_ON(UI);
    window_->RequestFrame();
    window_->OnHide();
    OnVirtualKeyboardAreaChange(Rect());
    FlushWindowStateChangedCallback(false);
}

void PipelineContext::WindowFocus(bool isFocus)
{
    CHECK_RUN_ON(UI);
    if (!isFocus) {
        NotifyPopupDismiss();
        OnVirtualKeyboardAreaChange(Rect());
    }
    FlushWindowFocusChangedCallback(isFocus);
}

void PipelineContext::Destroy()
{
    taskScheduler_.CleanUp();
    scheduleTasks_.clear();
    dirtyNodes_.clear();
    rootNode_.Reset();
    stageManager_.Reset();
    overlayManager_.Reset();
}

void PipelineContext::AddBuildFinishCallBack(std::function<void()>&& callback)
{
    buildFinishCallbacks_.emplace_back(std::move(callback));
}

void PipelineContext::AddWindowStateChangedCallback(int32_t nodeId)
{
    onWindowStateChangedCallbacks_.emplace_back(nodeId);
}

void PipelineContext::RemoveWindowStateChangedCallback(int32_t nodeId)
{
    onWindowStateChangedCallbacks_.remove(nodeId);
}

void PipelineContext::FlushWindowStateChangedCallback(bool isShow)
{
    auto iter = onWindowStateChangedCallbacks_.begin();
    while (iter != onWindowStateChangedCallbacks_.end()) {
        auto node = ElementRegister::GetInstance()->GetUINodeById(*iter);
        if (!node) {
            iter = onWindowStateChangedCallbacks_.erase(iter);
        } else {
            if (isShow) {
                node->OnWindowShow();
            } else {
                node->OnWindowHide();
            }
            ++iter;
        }
    }
}

void PipelineContext::AddWindowFocusChangedCallback(int32_t nodeId)
{
    onWindowFocusChangedCallbacks_.emplace_back(nodeId);
}

void PipelineContext::RemoveWindowFocusChangedCallback(int32_t nodeId)
{
    onWindowFocusChangedCallbacks_.remove(nodeId);
}

void PipelineContext::FlushWindowFocusChangedCallback(bool isFocus)
{
    auto iter = onWindowFocusChangedCallbacks_.begin();
    while (iter != onWindowFocusChangedCallbacks_.end()) {
        auto node = ElementRegister::GetInstance()->GetUINodeById(*iter);
        if (!node) {
            iter = onWindowFocusChangedCallbacks_.erase(iter);
        } else {
            if (isFocus) {
                node->OnWindowFocused();
            } else {
                node->OnWindowUnfocused();
            }
            ++iter;
        }
    }
}

void PipelineContext::OnDragEvent(int32_t x, int32_t y, DragEventAction action)
{
    if (isDragged_) {
        return;
    }

    auto manager = GetDragDropManager();
    CHECK_NULL_VOID(manager);

    std::string extraInfo;
    manager->GetExtraInfoFromClipboard(extraInfo);

    if (action == DragEventAction::DRAG_EVENT_END) {
        manager->OnDragEnd(static_cast<float>(x), static_cast<float>(y), extraInfo);
        manager->RestoreClipboardData();
        return;
    }

    manager->OnDragMove(static_cast<float>(x), static_cast<float>(y), extraInfo);
}

void PipelineContext::AddNodesToNotifyMemoryLevel(int32_t nodeId)
{
    nodesToNotifyMemoryLevel_.emplace_back(nodeId);
}

void PipelineContext::RemoveNodesToNotifyMemoryLevel(int32_t nodeId)
{
    nodesToNotifyMemoryLevel_.remove(nodeId);
}

void PipelineContext::NotifyMemoryLevel(int32_t level)
{
    LOGI("PipelineContext: NotifyMemoryLevel called");
    auto iter = nodesToNotifyMemoryLevel_.begin();
    while (iter != nodesToNotifyMemoryLevel_.end()) {
        auto node = ElementRegister::GetInstance()->GetUINodeById(*iter);
        if (!node) {
            iter = nodesToNotifyMemoryLevel_.erase(iter);
        } else {
            node->OnNotifyMemoryLevel(level);
        }
        ++iter;
    }
}
void PipelineContext::AddPredictTask(PredictTask&& task)
{
    taskScheduler_.AddPredictTask(std::move(task));
    window_->RequestFrame();
}

void PipelineContext::OnIdle(int64_t deadline)
{
    if (deadline == 0) {
        return;
    }
    CHECK_RUN_ON(UI);
    ACE_SCOPED_TRACE("OnIdle, targettime:%" PRId64 "", deadline);
    taskScheduler_.FlushPredictTask(deadline - TIME_THRESHOLD);
}

void PipelineContext::Finish(bool /*autoFinish*/) const
{
    CHECK_RUN_ON(UI);
    if (finishEventHandler_) {
        LOGI("call finishEventHandler");
        finishEventHandler_();
    } else {
        LOGE("fail to finish current context due to handler is nullptr");
    }
}

} // namespace OHOS::Ace::NG
