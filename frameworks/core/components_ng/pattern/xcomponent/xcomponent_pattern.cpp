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

#include "core/components_ng/pattern/xcomponent/xcomponent_pattern.h"

#include "base/geometry/ng/size_t.h"
#include "base/ressched/ressched_report.h"
#include "base/utils/utils.h"
#include "core/components_ng/event/input_event.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_event_hub.h"
#include "core/event/mouse_event.h"
#include "core/event/touch_event.h"
#include "core/pipeline/pipeline_context.h"
#include "frameworks/bridge/declarative_frontend/declarative_frontend.h"

namespace OHOS::Ace::NG {
namespace {
#ifdef OHOS_PLATFORM
constexpr int64_t INCREASE_CPU_TIME_ONCE = 4000000000; // 4s(unit: ns)
#endif
OH_NativeXComponent_TouchEventType ConvertNativeXComponentTouchEvent(const TouchType& touchType)
{
    switch (touchType) {
        case TouchType::DOWN:
            return OH_NativeXComponent_TouchEventType::OH_NATIVEXCOMPONENT_DOWN;
        case TouchType::UP:
            return OH_NativeXComponent_TouchEventType::OH_NATIVEXCOMPONENT_UP;
        case TouchType::MOVE:
            return OH_NativeXComponent_TouchEventType::OH_NATIVEXCOMPONENT_MOVE;
        case TouchType::CANCEL:
            return OH_NativeXComponent_TouchEventType::OH_NATIVEXCOMPONENT_CANCEL;
        default:
            return OH_NativeXComponent_TouchEventType::OH_NATIVEXCOMPONENT_UNKNOWN;
    }
}

OH_NativeXComponent_TouchPointToolType ConvertNativeXComponentTouchToolType(const SourceTool& toolType)
{
    switch (toolType) {
        case SourceTool::FINGER:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_FINGER;
        case SourceTool::PEN:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_PEN;
        case SourceTool::RUBBER:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_RUBBER;
        case SourceTool::BRUSH:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_BRUSH;
        case SourceTool::PENCIL:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_PENCIL;
        case SourceTool::AIRBRUSH:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_AIRBRUSH;
        case SourceTool::MOUSE:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_MOUSE;
        case SourceTool::LENS:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_LENS;
        default:
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_UNKNOWN;
    }
}
} // namespace

XComponentPattern::XComponentPattern(const std::string& id, XComponentType type, const std::string& libraryname,
    const RefPtr<XComponentController>& xcomponentController)
    : id_(id), type_(type), libraryname_(libraryname), xcomponentController_(xcomponentController)
{}

void XComponentPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    auto renderContext = host->GetRenderContext();
    if (type_ == XComponentType::SURFACE) {
        renderContextForSurface_ = RenderContext::Create();
        renderContextForSurface_->InitContext(false, id_ + "Surface");
        renderSurface_ = RenderSurface::Create();
        renderSurface_->SetRenderContext(renderContextForSurface_);
        renderSurface_->InitSurface();
        renderSurface_->UpdateXComponentConfig();
        renderContextForSurface_->UpdateBackgroundColor(Color::BLACK);
        InitEvent();
        SetMethodCall();
    }
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);
}

void XComponentPattern::OnRebuildFrame()
{
    if (type_ == XComponentType::COMPONENT) {
        return;
    }
    if (!renderSurface_->IsSurfaceValid()) {
        LOGE("surface not valid");
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->AddChild(renderContextForSurface_, 0);
}

void XComponentPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    CHECK_NULL_VOID(frameNode);
    if (!hasXComponentInit_) {
        return;
    }
    if (type_ == XComponentType::SURFACE) {
        NativeXComponentDestroy();
        auto eventHub = frameNode->GetEventHub<XComponentEventHub>();
        CHECK_NULL_VOID(eventHub);
        eventHub->FireDestroyEvent();
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto taskExecutor = pipelineContext->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostTask(
            [eventHub] {
                CHECK_NULL_VOID(eventHub);
                eventHub->FireSurfaceDestroyEvent();
            },
            TaskExecutor::TaskType::JS);
    }
}

void XComponentPattern::SetMethodCall()
{
    CHECK_NULL_VOID(xcomponentController_);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto uiTaskExecutor = SingleTaskExecutor::Make(pipelineContext->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    xcomponentController_->SetConfigSurfaceImpl(
        [weak = WeakClaim(this), uiTaskExecutor](uint32_t surfaceWidth, uint32_t surfaceHeight) {
            uiTaskExecutor.PostSyncTask([weak, surfaceWidth, surfaceHeight]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->ConfigSurface(surfaceWidth, surfaceHeight);
            });
        });

    xcomponentController_->surfaceId_ = renderSurface_->GetUniqueId();
}

void XComponentPattern::ConfigSurface(uint32_t surfaceWidth, uint32_t surfaceHeight)
{
    renderSurface_->ConfigSurface(surfaceWidth, surfaceHeight);
}

bool XComponentPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (type_ == XComponentType::COMPONENT || config.skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }
    auto geometryNode = dirty->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);

    if (!hasXComponentInit_) {
        auto position = geometryNode->GetContentOffset() + geometryNode->GetFrameOffset();
        auto drawSize = geometryNode->GetContentSize();
        XComponentSizeInit(drawSize.Width(), drawSize.Height());
        NativeXComponentOffset(position.GetX(), position.GetY());
        hasXComponentInit_ = true;
    } else {
        if (config.frameOffsetChange || config.contentOffsetChange) {
            auto position = geometryNode->GetContentOffset() + geometryNode->GetFrameOffset();
            NativeXComponentOffset(position.GetX(), position.GetY());
        }
        if (config.contentSizeChange) {
            auto drawSize = geometryNode->GetContentSize();
            XComponentSizeChange(drawSize.Width(), drawSize.Height());
        }
    }

    auto size = geometryNode->GetContentSize();
    auto offset = geometryNode->GetContentOffset();
    renderContextForSurface_->SetBounds(offset.GetX(), offset.GetY(), size.Width(), size.Height());
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkNeedSyncRenderTree();
    return false;
}

void XComponentPattern::OnPaint()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    renderContext->UpdateBackgroundColor(Color::BLACK);
}

void XComponentPattern::NativeXComponentChange(float width, float height)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto taskExecutor = pipelineContext->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [nXCompImpl = nativeXComponentImpl_, &nXComp = nativeXComponent_, width, height] {
            CHECK_NULL_VOID(nXComp && nXCompImpl);
            nXCompImpl->SetXComponentWidth(static_cast<int>(width));
            nXCompImpl->SetXComponentHeight(static_cast<int>(height));
            auto* surface = const_cast<void*>(nXCompImpl->GetSurface());
            const auto* callback = nXCompImpl->GetCallback();
            CHECK_NULL_VOID_NOLOG(callback && callback->OnSurfaceChanged);
            callback->OnSurfaceChanged(nXComp, surface);
        },
        TaskExecutor::TaskType::JS);
}

void XComponentPattern::NativeXComponentDestroy()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto taskExecutor = pipelineContext->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [nXCompImpl = nativeXComponentImpl_, &nXComp = nativeXComponent_] {
            CHECK_NULL_VOID(nXComp && nXCompImpl);
            auto* surface = const_cast<void*>(nXCompImpl->GetSurface());
            const auto* callback = nXCompImpl->GetCallback();
            CHECK_NULL_VOID_NOLOG(callback && callback->OnSurfaceDestroyed);
            callback->OnSurfaceDestroyed(nXComp, surface);
            delete nXComp;
            nXComp = nullptr;
        },
        TaskExecutor::TaskType::JS);
}

void XComponentPattern::NativeXComponentOffset(double x, double y)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    float scale = pipelineContext->GetViewScale();
    auto taskExecutor = pipelineContext->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [nXCompImpl = nativeXComponentImpl_, &nXComp = nativeXComponent_, x, y, scale] {
            CHECK_NULL_VOID(nXComp && nXCompImpl);
            nXCompImpl->SetXComponentOffsetX(x * scale);
            nXCompImpl->SetXComponentOffsetY(y * scale);
        },
        TaskExecutor::TaskType::JS);
}

void XComponentPattern::NativeXComponentDispatchTouchEvent(
    const OH_NativeXComponent_TouchEvent& touchEvent, const std::vector<XComponentTouchPoint>& xComponentTouchPoints)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto taskExecutor = pipelineContext->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [nXCompImpl = nativeXComponentImpl_, &nXComp = nativeXComponent_, touchEvent, xComponentTouchPoints] {
            CHECK_NULL_VOID(nXComp && nXCompImpl);
            nXCompImpl->SetTouchEvent(touchEvent);
            nXCompImpl->SetTouchPoint(xComponentTouchPoints);
            auto* surface = const_cast<void*>(nXCompImpl->GetSurface());
            const auto* callback = nXCompImpl->GetCallback();
            CHECK_NULL_VOID_NOLOG(callback && callback->DispatchTouchEvent);
            callback->DispatchTouchEvent(nXComp, surface);
        },
        TaskExecutor::TaskType::JS);
}

void XComponentPattern::XComponentSizeInit(float textureWidth, float textureHeight)
{
    auto host = GetHost();
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    if (renderSurface_->IsSurfaceValid() && type_ == XComponentType::SURFACE) {
        float viewScale = context->GetViewScale();
        renderSurface_->CreateNativeWindow();
        renderSurface_->AdjustNativeWindowSize(
            static_cast<int>(textureWidth * viewScale), static_cast<int>(textureHeight * viewScale));
    }

    auto platformTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::JS);
    platformTaskExecutor.PostTask([weak = WeakClaim(this)] {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        auto xcId = pattern->GetId();
        auto eventHub = host->GetEventHub<XComponentEventHub>();
        eventHub->FireSurfaceInitEvent(xcId, host->GetId());
        eventHub->FireLoadEvent(xcId);
    });
}

void XComponentPattern::XComponentSizeChange(float textureWidth, float textureHeight)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto viewScale = context->GetViewScale();
    renderSurface_->AdjustNativeWindowSize(
        static_cast<int>(textureWidth * viewScale), static_cast<int>(textureHeight * viewScale));
    NativeXComponentChange(textureWidth, textureHeight);
}

void XComponentPattern::InitEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<XComponentEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnSurfaceInitEvent(CreateExternalEvent());
    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    InitTouchEvent(gestureHub);
    auto inputHub = eventHub->GetOrCreateInputEventHub();
    InitMouseEvent(inputHub);
    InitMouseHoverEvent(inputHub);
}

void XComponentPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID_NOLOG(!touchEvent_);

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleTouchEvent(info);
    };

    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void XComponentPattern::InitMouseEvent(const RefPtr<InputEventHub>& inputHub)
{
    CHECK_NULL_VOID_NOLOG(!mouseEvent_);

    auto mouseTask = [weak = WeakClaim(this)](const MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleMouseEvent(info);
    };

    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void XComponentPattern::InitMouseHoverEvent(const RefPtr<InputEventHub>& inputHub)
{
    CHECK_NULL_VOID_NOLOG(!mouseHoverEvent_);
    auto mouseHoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleMouseHoverEvent(isHover);
    };
    mouseHoverEvent_ = MakeRefPtr<InputEvent>(std::move(mouseHoverTask));
    inputHub->AddOnHoverEvent(mouseHoverEvent_);
}

void XComponentPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    auto touchInfoList = info.GetChangedTouches();
    if (touchInfoList.empty()) {
        return;
    }
    const auto& locationInfo = touchInfoList.front();
    const auto& screenOffset = locationInfo.GetGlobalLocation();
    const auto& localOffset = locationInfo.GetLocalLocation();
    touchEventPoint_.id = locationInfo.GetFingerId();
    touchEventPoint_.screenX = static_cast<float>(screenOffset.GetX());
    touchEventPoint_.screenY = static_cast<float>(screenOffset.GetY());
    touchEventPoint_.x = static_cast<float>(localOffset.GetX());
    touchEventPoint_.y = static_cast<float>(localOffset.GetY());
    touchEventPoint_.size = locationInfo.GetSize();
    touchEventPoint_.force = locationInfo.GetForce();
    touchEventPoint_.deviceId = locationInfo.GetTouchDeviceId();
    const auto timeStamp = info.GetTimeStamp().time_since_epoch().count();
    touchEventPoint_.timeStamp = timeStamp;
    auto touchType = touchInfoList.front().GetTouchType();
    touchEventPoint_.type = ConvertNativeXComponentTouchEvent(touchType);
#ifdef OHOS_PLATFORM
    // increase cpu frequency
    if (touchType == TouchType::MOVE) {
        auto currentTime = GetSysTimestamp();
        auto increaseCpuTime = currentTime - startIncreaseTime_;
        if (increaseCpuTime >= INCREASE_CPU_TIME_ONCE) {
            LOGD("HandleTouchEvent increase cpu frequency");
            startIncreaseTime_ = currentTime;
            ResSchedReport::GetInstance().ResSchedDataReport("slide_on");
        }
    } else if (touchType == TouchType::UP) {
        startIncreaseTime_ = 0;
        ResSchedReport::GetInstance().ResSchedDataReport("slide_off");
    }
#endif
    SetTouchPoint(info.GetTouches(), timeStamp, touchType);

    NativeXComponentDispatchTouchEvent(touchEventPoint_, nativeXComponentTouchPoints_);
}

void XComponentPattern::HandleMouseEvent(const MouseInfo& info)
{
    OH_NativeXComponent_MouseEvent mouseEventPoint;
    mouseEventPoint.x = static_cast<float>(info.GetLocalLocation().GetX());
    mouseEventPoint.y = static_cast<float>(info.GetLocalLocation().GetY());
    mouseEventPoint.screenX = static_cast<float>(info.GetScreenLocation().GetX());
    mouseEventPoint.screenY = static_cast<float>(info.GetScreenLocation().GetY());
    switch (info.GetAction()) {
        case MouseAction::PRESS:
            mouseEventPoint.action = OH_NativeXComponent_MouseEventAction::OH_NATIVEXCOMPONENT_MOUSE_PRESS;
            break;
        case MouseAction::RELEASE:
            mouseEventPoint.action = OH_NativeXComponent_MouseEventAction::OH_NATIVEXCOMPONENT_MOUSE_RELEASE;
            break;
        case MouseAction::MOVE:
            mouseEventPoint.action = OH_NativeXComponent_MouseEventAction::OH_NATIVEXCOMPONENT_MOUSE_MOVE;
            break;
        default:
            mouseEventPoint.action = OH_NativeXComponent_MouseEventAction::OH_NATIVEXCOMPONENT_MOUSE_NONE;
            break;
    }
    switch (info.GetButton()) {
        case MouseButton::LEFT_BUTTON:
            mouseEventPoint.button = OH_NativeXComponent_MouseEventButton::OH_NATIVEXCOMPONENT_LEFT_BUTTON;
            break;
        case MouseButton::RIGHT_BUTTON:
            mouseEventPoint.button = OH_NativeXComponent_MouseEventButton::OH_NATIVEXCOMPONENT_RIGHT_BUTTON;
            break;
        case MouseButton::MIDDLE_BUTTON:
            mouseEventPoint.button = OH_NativeXComponent_MouseEventButton::OH_NATIVEXCOMPONENT_MIDDLE_BUTTON;
            break;
        case MouseButton::BACK_BUTTON:
            mouseEventPoint.button = OH_NativeXComponent_MouseEventButton::OH_NATIVEXCOMPONENT_BACK_BUTTON;
            break;
        case MouseButton::FORWARD_BUTTON:
            mouseEventPoint.button = OH_NativeXComponent_MouseEventButton::OH_NATIVEXCOMPONENT_FORWARD_BUTTON;
            break;
        default:
            mouseEventPoint.button = OH_NativeXComponent_MouseEventButton::OH_NATIVEXCOMPONENT_NONE_BUTTON;
            break;
    }
    mouseEventPoint.timestamp = info.GetTimeStamp().time_since_epoch().count();
    NativeXComponentDispatchMouseEvent(mouseEventPoint);
}

void XComponentPattern::HandleMouseHoverEvent(bool isHover)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [nXCompImpl = nativeXComponentImpl_, &nXComp = nativeXComponent_, isHover] {
            CHECK_NULL_VOID(nXComp && nXCompImpl);
            const auto* callback = nXCompImpl->GetMouseEventCallback();
            CHECK_NULL_VOID_NOLOG(callback && callback->DispatchHoverEvent);
            callback->DispatchHoverEvent(nXComp, isHover);
        },
        TaskExecutor::TaskType::JS);
}

void XComponentPattern::NativeXComponentDispatchMouseEvent(const OH_NativeXComponent_MouseEvent& mouseEvent)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [nXCompImpl = nativeXComponentImpl_, nXComp = nativeXComponent_, mouseEvent] {
            CHECK_NULL_VOID(nXComp && nXCompImpl);
            nXCompImpl->SetMouseEvent(mouseEvent);
            auto* surface = const_cast<void*>(nXCompImpl->GetSurface());
            const auto* callback = nXCompImpl->GetMouseEventCallback();
            CHECK_NULL_VOID_NOLOG(callback && callback->DispatchMouseEvent);
            callback->DispatchMouseEvent(nXComp, surface);
        },
        TaskExecutor::TaskType::JS);
}

void XComponentPattern::SetTouchPoint(
    const std::list<TouchLocationInfo>& touchInfoList, const int64_t timeStamp, const TouchType& touchType)
{
    touchEventPoint_.numPoints =
        touchInfoList.size() <= OH_MAX_TOUCH_POINTS_NUMBER ? touchInfoList.size() : OH_MAX_TOUCH_POINTS_NUMBER;
    nativeXComponentTouchPoints_.clear();
    uint32_t index = 0;
    for (auto iterator = touchInfoList.begin(); iterator != touchInfoList.end() && index < OH_MAX_TOUCH_POINTS_NUMBER;
         iterator++) {
        OH_NativeXComponent_TouchPoint ohTouchPoint;
        const auto& pointTouchInfo = *iterator;
        const auto& pointScreenOffset = pointTouchInfo.GetGlobalLocation();
        const auto& pointLocalOffset = pointTouchInfo.GetLocalLocation();
        ohTouchPoint.id = pointTouchInfo.GetFingerId();
        ohTouchPoint.screenX = static_cast<float>(pointScreenOffset.GetX());
        ohTouchPoint.screenY = static_cast<float>(pointScreenOffset.GetY());
        ohTouchPoint.x = static_cast<float>(pointLocalOffset.GetX());
        ohTouchPoint.y = static_cast<float>(pointLocalOffset.GetY());
        ohTouchPoint.type = ConvertNativeXComponentTouchEvent(touchType);
        ohTouchPoint.size = pointTouchInfo.GetSize();
        ohTouchPoint.force = pointTouchInfo.GetForce();
        ohTouchPoint.timeStamp = timeStamp;
        ohTouchPoint.isPressed = (touchType == TouchType::DOWN);
        touchEventPoint_.touchPoints[index++] = ohTouchPoint;
        // set tiltX, tiltY and sourceToolType
        XComponentTouchPoint xcomponentTouchPoint;
        xcomponentTouchPoint.tiltX = pointTouchInfo.GetTiltX().value_or(0.0f);
        xcomponentTouchPoint.tiltY = pointTouchInfo.GetTiltY().value_or(0.0f);
        xcomponentTouchPoint.sourceToolType = ConvertNativeXComponentTouchToolType(pointTouchInfo.GetSourceTool());
        nativeXComponentTouchPoints_.emplace_back(xcomponentTouchPoint);
    }
    while (index < OH_MAX_TOUCH_POINTS_NUMBER) {
        OH_NativeXComponent_TouchPoint ohTouchPoint;
        ohTouchPoint.id = 0;
        ohTouchPoint.screenX = 0;
        ohTouchPoint.screenY = 0;
        ohTouchPoint.x = 0;
        ohTouchPoint.y = 0;
        ohTouchPoint.type = OH_NativeXComponent_TouchEventType::OH_NATIVEXCOMPONENT_UNKNOWN;
        ohTouchPoint.size = 0;
        ohTouchPoint.force = 0;
        ohTouchPoint.timeStamp = 0;
        ohTouchPoint.isPressed = false;
        touchEventPoint_.touchPoints[index++] = ohTouchPoint;
    }
}

ExternalEvent XComponentPattern::CreateExternalEvent()
{
    return
        [weak = AceType::WeakClaim(this)](const std::string& componentId, const uint32_t nodeId, const bool isDestroy) {
            auto context = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(context);
            auto frontEnd = AceType::DynamicCast<DeclarativeFrontend>(context->GetFrontend());
            CHECK_NULL_VOID(frontEnd);
            auto jsEngine = frontEnd->GetJsEngine();
            jsEngine->FireExternalEvent(componentId, nodeId, isDestroy);
        };
}
} // namespace OHOS::Ace::NG
