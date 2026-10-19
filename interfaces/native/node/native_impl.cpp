/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "interfaces/native/native_error_message_macros.h"
#include "native_interface.h"
#include "node/animate_impl.h"
#include "node/config_manager.h"
#include "node/dialog_model.h"
#include "node/gesture_impl.h"
#include "node/node_model.h"
#include "node/node_model_safely.h"
#include "node_extened.h"

#include "base/log/log_wrapper.h"

namespace {

constexpr int32_t CURRENT_NATIVE_NODE_API_VERSION = 1;
constexpr int32_t NATIVE_DIALOG_VERSION_0 = 0;
constexpr int32_t NATIVE_DIALOG_VERSION_1 = 1;
constexpr int32_t NATIVE_DIALOG_VERSION_2 = 2;
constexpr int32_t NATIVE_DIALOG_VERSION_3 = 3;

ArkUI_NativeNodeAPI_Compatible nodeImpl_compatible = {
    CURRENT_NATIVE_NODE_API_VERSION,
    OHOS::Ace::NodeModel::CreateNode,
    OHOS::Ace::NodeModel::DisposeNode,
    OHOS::Ace::NodeModel::AddChild,
    OHOS::Ace::NodeModel::RemoveChild,
    OHOS::Ace::NodeModel::InsertChildAfter,
    OHOS::Ace::NodeModel::InsertChildBefore,
    OHOS::Ace::NodeModel::InsertChildAt,
    OHOS::Ace::NodeModel::SetAttribute,
    OHOS::Ace::NodeModel::GetAttribute,
    OHOS::Ace::NodeModel::ResetAttribute,
    OHOS::Ace::NodeModel::RegisterNodeEvent,
    OHOS::Ace::NodeModel::UnregisterNodeEvent,
    OHOS::Ace::NodeModel::RegisterOnEvent,
    OHOS::Ace::NodeModel::UnregisterOnEvent,
    OHOS::Ace::NodeModel::MarkDirty,
};

ArkUI_NativeNodeAPI_1 nodeImpl_1 = {
    CURRENT_NATIVE_NODE_API_VERSION,
    [](ArkUI_NodeType type) -> ArkUI_NodeHandle {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.createNode");
        return OHOS::Ace::NodeModel::CreateNode(type);
    },
    [](ArkUI_NodeHandle node) -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.disposeNode");
        OHOS::Ace::NodeModel::DisposeNode(node);
    },
    [](ArkUI_NodeHandle parent, ArkUI_NodeHandle child) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.addChild");
        return OHOS::Ace::NodeModel::AddChild(parent, child);
    },
    [](ArkUI_NodeHandle parent, ArkUI_NodeHandle child) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.removeChild");
        return OHOS::Ace::NodeModel::RemoveChild(parent, child);
    },
    [](ArkUI_NodeHandle parent, ArkUI_NodeHandle child, ArkUI_NodeHandle sibling) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.insertChildAfter");
        return OHOS::Ace::NodeModel::InsertChildAfter(parent, child, sibling);
    },
    [](ArkUI_NodeHandle parent, ArkUI_NodeHandle child, ArkUI_NodeHandle sibling) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.insertChildBefore");
        return OHOS::Ace::NodeModel::InsertChildBefore(parent, child, sibling);
    },
    [](ArkUI_NodeHandle parent, ArkUI_NodeHandle child, int32_t position) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.insertChildAt");
        return OHOS::Ace::NodeModel::InsertChildAt(parent, child, position);
    },
    [](ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute, const ArkUI_AttributeItem* item) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.setAttribute");
        return OHOS::Ace::NodeModel::SetAttribute(node, attribute, item);
    },
    [](ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute) -> const ArkUI_AttributeItem* {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getAttribute");
        return OHOS::Ace::NodeModel::GetAttribute(node, attribute);
    },
    [](ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.resetAttribute");
        return OHOS::Ace::NodeModel::ResetAttribute(node, attribute);
    },
    [](ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType, int32_t targetId, void* userData) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.registerNodeEvent");
        return OHOS::Ace::NodeModel::RegisterNodeEvent(nodePtr, eventType, targetId, userData);
    },
    [](ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType) -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.unregisterNodeEvent");
        OHOS::Ace::NodeModel::UnregisterNodeEvent(nodePtr, eventType);
    },
    [](void (*eventReceiver)(ArkUI_NodeEvent* event)) -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.registerNodeEventReceiver");
        OHOS::Ace::NodeModel::RegisterOnEvent(eventReceiver);
    },
    []() -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.unregisterNodeEventReceiver");
        OHOS::Ace::NodeModel::UnregisterOnEvent();
    },
    [](ArkUI_NodeHandle node, ArkUI_NodeDirtyFlag dirtyFlag) -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.markDirty");
        OHOS::Ace::NodeModel::MarkDirty(node, dirtyFlag);
    },
    [](ArkUI_NodeHandle node) -> uint32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getTotalChildCount");
        return OHOS::Ace::NodeModel::GetTotalChildCount(node);
    },
    [](ArkUI_NodeHandle node, int32_t position) -> ArkUI_NodeHandle {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getChildAt");
        return OHOS::Ace::NodeModel::GetChildAt(node, position);
    },
    [](ArkUI_NodeHandle node) -> ArkUI_NodeHandle {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getFirstChild");
        return OHOS::Ace::NodeModel::GetFirstChild(node);
    },
    [](ArkUI_NodeHandle node) -> ArkUI_NodeHandle {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getLastChild");
        return OHOS::Ace::NodeModel::GetLastChild(node);
    },
    [](ArkUI_NodeHandle node) -> ArkUI_NodeHandle {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getPreviousSibling");
        return OHOS::Ace::NodeModel::GetPreviousSibling(node);
    },
    [](ArkUI_NodeHandle node) -> ArkUI_NodeHandle {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getNextSibling");
        return OHOS::Ace::NodeModel::GetNextSibling(node);
    },
    [](ArkUI_NodeHandle node, ArkUI_NodeCustomEventType eventType, int32_t targetId, void* userData) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.registerNodeCustomEvent");
        return OHOS::Ace::NodeModel::RegisterNodeCustomEvent(node, eventType, targetId, userData);
    },
    [](ArkUI_NodeHandle node, ArkUI_NodeCustomEventType eventType) -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.unregisterNodeCustomEvent");
        OHOS::Ace::NodeModel::UnregisterNodeCustomEvent(node, eventType);
    },
    [](void (*eventReceiver)(ArkUI_NodeCustomEvent* event)) -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.registerNodeCustomEventReceiver");
        OHOS::Ace::NodeModel::RegisterNodeCustomReceiver(eventReceiver);
    },
    []() -> void {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.unregisterNodeCustomEventReceiver");
        OHOS::Ace::NodeModel::UnregisterNodeCustomEventReceiver();
    },
    OHOS::Ace::NodeModel::SetMeasuredSize,
    OHOS::Ace::NodeModel::SetLayoutPosition,
    OHOS::Ace::NodeModel::GetMeasuredSize,
    OHOS::Ace::NodeModel::GetLayoutPosition,
    OHOS::Ace::NodeModel::MeasureNode,
    OHOS::Ace::NodeModel::LayoutNode,
    OHOS::Ace::NodeModel::AddNodeEventReceiver,
    OHOS::Ace::NodeModel::RemoveNodeEventReceiver,
    OHOS::Ace::NodeModel::AddNodeCustomEventReceiver,
    OHOS::Ace::NodeModel::RemoveNodeCustomEventReceiver,
    OHOS::Ace::NodeModel::SetUserData,
    OHOS::Ace::NodeModel::GetUserData,
    [](ArkUI_NodeHandle node, ArkUI_LengthMetricUnit unit) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.setLengthMetricUnit");
        return OHOS::Ace::NodeModel::SetLengthMetricUnit(node, unit);
    },
    [](ArkUI_NodeHandle node) -> ArkUI_NodeHandle {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.getParent");
        return OHOS::Ace::NodeModel::GetParent(node);
    },
    [](ArkUI_NodeHandle parent) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeNodeAPI_1.removeAllChildren");
        return OHOS::Ace::NodeModel::RemoveAllChildren(parent);
    },
};

ArkUI_NativeNodeAPI_1 multiThreadNodeImpl_1 = {
    .version = CURRENT_NATIVE_NODE_API_VERSION,
    .createNode = OHOS::Ace::NodeModel::CreateNodeSafely,
    .disposeNode = OHOS::Ace::NodeModel::DisposeNodeSafely,
    .addChild = OHOS::Ace::NodeModel::AddChildSafely,
    .removeChild = OHOS::Ace::NodeModel::RemoveChildSafely,
    .insertChildAfter = OHOS::Ace::NodeModel::InsertChildAfterSafely,
    .insertChildBefore = OHOS::Ace::NodeModel::InsertChildBeforeSafely,
    .insertChildAt = OHOS::Ace::NodeModel::InsertChildAtSafely,
    .setAttribute = OHOS::Ace::NodeModel::SetAttributeSafely,
    .getAttribute = OHOS::Ace::NodeModel::GetAttributeSafely,
    .resetAttribute = OHOS::Ace::NodeModel::ResetAttributeSafely,
    .registerNodeEvent = OHOS::Ace::NodeModel::RegisterNodeEventSafely,
    .unregisterNodeEvent = OHOS::Ace::NodeModel::UnregisterNodeEventSafely,
    .registerNodeEventReceiver = OHOS::Ace::NodeModel::RegisterOnEventSafely,
    .unregisterNodeEventReceiver = OHOS::Ace::NodeModel::UnregisterOnEventSafely,
    .markDirty = OHOS::Ace::NodeModel::MarkDirtySafely,
    .getTotalChildCount = OHOS::Ace::NodeModel::GetTotalChildCountSafely,
    .getChildAt = OHOS::Ace::NodeModel::GetChildAtSafely,
    .getFirstChild = OHOS::Ace::NodeModel::GetFirstChildSafely,
    .getLastChild = OHOS::Ace::NodeModel::GetLastChildSafely,
    .getPreviousSibling = OHOS::Ace::NodeModel::GetPreviousSiblingSafely,
    .getNextSibling = OHOS::Ace::NodeModel::GetNextSiblingSafely,
    .registerNodeCustomEvent = OHOS::Ace::NodeModel::RegisterNodeCustomEventSafely,
    .unregisterNodeCustomEvent = OHOS::Ace::NodeModel::UnregisterNodeCustomEventSafely,
    .registerNodeCustomEventReceiver = OHOS::Ace::NodeModel::RegisterNodeCustomReceiverSafely,
    .unregisterNodeCustomEventReceiver = OHOS::Ace::NodeModel::UnregisterNodeCustomEventReceiverSafely,
    .setMeasuredSize = OHOS::Ace::NodeModel::SetMeasuredSizeSafely,
    .setLayoutPosition = OHOS::Ace::NodeModel::SetLayoutPositionSafely,
    .getMeasuredSize = OHOS::Ace::NodeModel::GetMeasuredSizeSafely,
    .getLayoutPosition = OHOS::Ace::NodeModel::GetLayoutPositionSafely,
    .measureNode = OHOS::Ace::NodeModel::MeasureNodeSafely,
    .layoutNode = OHOS::Ace::NodeModel::LayoutNodeSafely,
    .addNodeEventReceiver = OHOS::Ace::NodeModel::AddNodeEventReceiverSafely,
    .removeNodeEventReceiver = OHOS::Ace::NodeModel::RemoveNodeEventReceiverSafely,
    .addNodeCustomEventReceiver = OHOS::Ace::NodeModel::AddNodeCustomEventReceiverSafely,
    .removeNodeCustomEventReceiver = OHOS::Ace::NodeModel::RemoveNodeCustomEventReceiverSafely,
    .setUserData = OHOS::Ace::NodeModel::SetUserDataSafely,
    .getUserData = OHOS::Ace::NodeModel::GetUserDataSafely,
    .setLengthMetricUnit = OHOS::Ace::NodeModel::SetLengthMetricUnitSafely,
    .getParent = OHOS::Ace::NodeModel::GetParentSafely,
    .removeAllChildren = OHOS::Ace::NodeModel::RemoveAllChildrenSafely,
};

ArkUI_NativeDialogAPI_1 dialogImpl_1 = {
    OHOS::Ace::DialogModel::Create,
    OHOS::Ace::DialogModel::Dispose,
    OHOS::Ace::DialogModel::SetContent,
    OHOS::Ace::DialogModel::RemoveContent,
    OHOS::Ace::DialogModel::SetContentAlignment,
    OHOS::Ace::DialogModel::ResetContentAlignment,
    OHOS::Ace::DialogModel::SetModalMode,
    OHOS::Ace::DialogModel::SetAutoCancel,
    OHOS::Ace::DialogModel::SetMask,
    OHOS::Ace::DialogModel::SetBackgroundColor,
    OHOS::Ace::DialogModel::SetCornerRadius,
    OHOS::Ace::DialogModel::SetGridColumnCount,
    OHOS::Ace::DialogModel::EnableCustomStyle,
    OHOS::Ace::DialogModel::EnableCustomAnimation,
    OHOS::Ace::DialogModel::RegisterOnWillDismiss,
    OHOS::Ace::DialogModel::Show,
    OHOS::Ace::DialogModel::Close,
    OHOS::Ace::DialogModel::RegisterOnWillDismissWithUserData,
};

ArkUI_NativeDialogAPI_2 dialogImpl_2 = {
    dialogImpl_1,
    OHOS::Ace::DialogModel::SetKeyboardAvoidDistance,
    OHOS::Ace::DialogModel::SetLevelMode,
    OHOS::Ace::DialogModel::SetLevelUniqueId,
    OHOS::Ace::DialogModel::SetImmersiveMode,
};

ArkUI_NativeDialogAPI_3 dialogImpl_3 = {
    dialogImpl_1,
    dialogImpl_2,
    OHOS::Ace::DialogModel::SetLevelOrder,
    OHOS::Ace::DialogModel::RegisterOnWillAppear,
    OHOS::Ace::DialogModel::RegisterOnDidAppear,
    OHOS::Ace::DialogModel::RegisterOnWillDisappear,
    OHOS::Ace::DialogModel::RegisterOnDidDisappear,
    OHOS::Ace::DialogModel::SetBorderWidth,
    OHOS::Ace::DialogModel::SetBorderColor,
    OHOS::Ace::DialogModel::SetBorderStyle,
    OHOS::Ace::DialogModel::SetWidth,
    OHOS::Ace::DialogModel::SetHeight,
    OHOS::Ace::DialogModel::SetShadow,
    OHOS::Ace::DialogModel::SetCustomShadow,
    OHOS::Ace::DialogModel::SetBackgroundBlurStyle,
    OHOS::Ace::DialogModel::SetKeyboardAvoidMode,
    OHOS::Ace::DialogModel::EnableHoverMode,
    OHOS::Ace::DialogModel::SetHoverModeArea,
    OHOS::Ace::DialogModel::SetFocusable,
    OHOS::Ace::DialogModel::SetBackgroundBlurStyleOptions,
    OHOS::Ace::DialogModel::SetBackgroundEffect,
};

constexpr int32_t CURRENT_NATIVE_GESTURE_API_VERSION = 1;
ArkUI_NativeGestureAPI_1 gestureImpl_1 = {
    CURRENT_NATIVE_GESTURE_API_VERSION,
    [](int32_t count, int32_t fingers) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createTapGesture");
        return OHOS::Ace::GestureModel::CreateTapGesture(count, fingers);
    },
    [](int32_t fingers, bool repeatResult, int32_t duration) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createLongPressGesture");
        return OHOS::Ace::GestureModel::CreateLongPressGesture(fingers, repeatResult, duration);
    },
    [](int32_t fingersNum, ArkUI_GestureDirectionMask mask, double distanceNum) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createPanGesture");
        return OHOS::Ace::GestureModel::CreatePanGesture(fingersNum, mask, distanceNum);
    },
    [](int32_t fingers, double distance) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createPinchGesture");
        return OHOS::Ace::GestureModel::CreatePinchGesture(fingers, distance);
    },
    [](int32_t fingers, double angle) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createRotationGesture");
        return OHOS::Ace::GestureModel::CreateRotationGesture(fingers, angle);
    },
    [](int32_t fingers, ArkUI_GestureDirectionMask directions, double speed) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createSwipeGesture");
        return OHOS::Ace::GestureModel::CreateSwipeGesture(fingers, directions, speed);
    },
    [](ArkUI_GroupGestureMode gestureMode) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createGroupGesture");
        return OHOS::Ace::GestureModel::CreateGroupGesture(gestureMode);
    },
    [](ArkUI_GestureRecognizer* recognizer) -> void {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.dispose");
        OHOS::Ace::GestureModel::DisposeGesture(recognizer);
    },
    [](ArkUI_GestureRecognizer* group, ArkUI_GestureRecognizer* child) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.addChildGesture");
        return OHOS::Ace::GestureModel::AddChildGesture(group, child);
    },
    [](ArkUI_GestureRecognizer* group, ArkUI_GestureRecognizer* child) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.removeChildGesture");
        return OHOS::Ace::GestureModel::RemoveChildGesture(group, child);
    },
    [](ArkUI_GestureRecognizer* recognizer, ArkUI_GestureEventActionTypeMask mask, void* extraParam,
        void (*targetReceiver)(ArkUI_GestureEvent* event, void* extraParam)) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.setGestureEventTarget");
        return OHOS::Ace::GestureModel::SetGestureEventTarget(recognizer, mask, extraParam, targetReceiver);
    },
    [](ArkUI_NodeHandle node, ArkUI_GestureRecognizer* recognizer, ArkUI_GesturePriority priorityNum,
        ArkUI_GestureMask mask) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.addGestureToNode");
        return OHOS::Ace::GestureModel::AddGestureToNode(node, recognizer, priorityNum, mask);
    },
    [](ArkUI_NodeHandle node, ArkUI_GestureRecognizer* recognizer) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.removeGestureFromNode");
        return OHOS::Ace::GestureModel::RemoveGestureFromNode(node, recognizer);
    },
    [](ArkUI_NodeHandle node, ArkUI_GestureInterruptResult (*interrupter)(ArkUI_GestureInterruptInfo* info)) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.setGestureInterrupterToNode");
        return OHOS::Ace::GestureModel::SetGestureInterrupterToNode(node, interrupter);
    },
    OHOS::Ace::GestureModel::GetGestureType,
    [](ArkUI_NodeHandle node, void* userData,
        ArkUI_GestureRecognizer* (*parallelInnerGesture)(ArkUI_ParallelInnerGestureEvent* event)) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.setInnerGestureParallelTo");
        return OHOS::Ace::GestureModel::SetInnerGestureParallelTo(node, userData, parallelInnerGesture);
    },
    [](int32_t count, int32_t fingers, double distanceThreshold) -> ArkUI_GestureRecognizer* {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_1.createTapGestureWithDistanceThreshold");
        return OHOS::Ace::GestureModel::CreateTapGestureWithDistanceThreshold(count, fingers, distanceThreshold);
    },
};

ArkUI_NativeGestureAPI_2 gestureImpl_2 = {
    &gestureImpl_1,
    [](ArkUI_NodeHandle node, void* userData,
        ArkUI_GestureInterruptResult (*interrupter)(ArkUI_GestureInterruptInfo* info)) -> int32_t {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_2.setGestureInterrupterToNode");
        return OHOS::Ace::GestureModel::SetGestureInterrupterToNodeWithUserData(node, userData, interrupter);
    },
};

ArkUI_NativeGestureAPI_3 gestureImpl_3 = {
    &gestureImpl_2,
    [](ArkUI_NodeHandle node, void* userData,
        ArkUI_GestureRecognizer* (*parallelGesture)(ArkUI_ParallelGestureEvent* event)) -> ArkUI_ErrorCode {
        CHECK_UI_THREAD("ArkUI_NativeGestureAPI_3.setGestureParallelTo");
        return OHOS::Ace::GestureModel::SetGestureParallelTo(node, userData, parallelGesture);
    },
};

ArkUI_NativeAnimateAPI_1 animateImpl_1 = {
    OHOS::Ace::AnimateModel::AnimateTo,
    OHOS::Ace::AnimateModel::KeyframeAnimateTo,
    OHOS::Ace::AnimateModel::CreateAnimator,
    OHOS::Ace::AnimateModel::DisposeAnimator,
};

} // namespace

#ifdef __cplusplus
extern "C" {
#endif

void* OH_ArkUI_QueryModuleInterface(ArkUI_NativeAPIVariantKind type, int32_t version)
{
    if (!OHOS::Ace::NodeModel::InitialFullImpl()) {
        TAG_LOGE(OHOS::Ace::AceLogTag::ACE_NATIVE_NODE,
            "fail to get %{public}d node api family of %{public}d version, impl library is not found", type, version);
        return nullptr;
    }
    switch (type) {
        case ARKUI_NATIVE_NODE: {
            switch (version) {
                case 0:
                case 1:
                    return &nodeImpl_compatible;
                default: {
                    TAG_LOGE(OHOS::Ace::AceLogTag::ACE_NATIVE_NODE,
                        "fail to get basic node api family, version is incorrect: %{public}d", version);
                    return nullptr;
                }
            }
            break;
        }
        case ARKUI_NATIVE_DIALOG: {
            switch (version) {
                case NATIVE_DIALOG_VERSION_0:
                case NATIVE_DIALOG_VERSION_1:
                    return &dialogImpl_1;
                case NATIVE_DIALOG_VERSION_2:
                    return &dialogImpl_2;
                case NATIVE_DIALOG_VERSION_3:
                    return &dialogImpl_3;
                default: {
                    TAG_LOGE(OHOS::Ace::AceLogTag::ACE_NATIVE_NODE,
                        "fail to get dialog api family, version is incorrect: %{public}d", version);
                    return nullptr;
                }
            }
            break;
        }
        case ARKUI_NATIVE_GESTURE: {
            switch (version) {
                case 0:
                case 1:
                    return &gestureImpl_1;
                case 2:
                    return &gestureImpl_2;
                case 3:
                    return &gestureImpl_3;
                default: {
                    TAG_LOGE(OHOS::Ace::AceLogTag::ACE_NATIVE_NODE,
                        "fail to get gesture api family, version is incorrect: %{public}d", version);
                    return nullptr;
                }
            }
            break;
        }
        default: {
            TAG_LOGE(OHOS::Ace::AceLogTag::ACE_NATIVE_NODE,
                "fail to get %{public}d node api family, version is incorrect: %{public}d", type, version);
            return nullptr;
        }
    }
}

void* OH_ArkUI_GetNativeAPI(ArkUI_NativeAPIVariantKind type, int32_t version)
{
    return OH_ArkUI_QueryModuleInterface(type, version);
}

void* OH_ArkUI_QueryModuleInterfaceByName(ArkUI_NativeAPIVariantKind type, const char* structName)
{
    if (structName == nullptr) {
        TAG_LOGE(
            OHOS::Ace::AceLogTag::ACE_NATIVE_NODE, "fail to get %{public}d node api family, struct name is null", type);
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "struct name is null");
        return nullptr;
    }
    if (!OHOS::Ace::NodeModel::InitialFullImpl()) {
        TAG_LOGE(OHOS::Ace::AceLogTag::ACE_NATIVE_NODE,
            "fail to get %{public}d node api family, impl library is not found", type);
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID, __FUNCTION__, "impl library is not found");
        return nullptr;
    }
    switch (type) {
        case ARKUI_NATIVE_NODE:
            if (strcmp(structName, "ArkUI_NativeNodeAPI_1") == 0) {
                auto impl = OHOS::Ace::NodeModel::GetFullImpl();
                if (impl && impl->getMultiThreadManagerAPI() &&
                    impl->getMultiThreadManagerAPI()->debugThreadSafeNodeEnabled()) {
                    return &multiThreadNodeImpl_1;
                }
                return &nodeImpl_1;
            }
            break;
        case ARKUI_NATIVE_DIALOG:
            if (strcmp(structName, "ArkUI_NativeDialogAPI_1") == 0) {
                return &dialogImpl_1;
            } else if (strcmp(structName, "ArkUI_NativeDialogAPI_2") == 0) {
                return &dialogImpl_2;
            } else if (strcmp(structName, "ArkUI_NativeDialogAPI_3") == 0) {
                return &dialogImpl_3;
            }
            break;
        case ARKUI_NATIVE_GESTURE:
            if (strcmp(structName, "ArkUI_NativeGestureAPI_1") == 0) {
                return &gestureImpl_1;
            } else if (strcmp(structName, "ArkUI_NativeGestureAPI_2") == 0) {
                return &gestureImpl_2;
            } else if (strcmp(structName, "ArkUI_NativeGestureAPI_3") == 0) {
                return &gestureImpl_3;
            }
            break;
        case ARKUI_NATIVE_ANIMATE:
            if (strcmp(structName, "ArkUI_NativeAnimateAPI_1") == 0) {
                return &animateImpl_1;
            }
            break;
        case ARKUI_MULTI_THREAD_NATIVE_NODE:
            if (strcmp(structName, "ArkUI_NativeNodeAPI_1") == 0) {
                return &multiThreadNodeImpl_1;
            }
            break;
        default:
            break;
    }
    return nullptr;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_SetRuntimeCheckMode(
    OH_ArkUI_NativeModule_RuntimeCheckType checkType, OH_ArkUI_NativeModule_RuntimeCheckMode mode)
{
    // Ensures the internal bridge is installed so the UI thread predicate and the
    // application debug flag are available to the config manager.
    OHOS::Ace::NodeModel::GetOrCreateFullImpl();
    bool success = OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        static_cast<int32_t>(checkType), static_cast<int32_t>(mode));
    return success ? ARKUI_ERROR_CODE_NO_ERROR : ARKUI_ERROR_CODE_PARAM_INVALID;
}

#ifdef __cplusplus
};
#endif
