/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_INTERFACES_OBSERVER_LISTENER_H
#define FOUNDATION_ACE_INTERFACES_OBSERVER_LISTENER_H

#include <cstdint>
#include <string>

#include "napi/native_api.h"

#include "common/napi_observer_callback.h"

namespace OHOS::Ace {
class BaseEventInfo;
class ClickInfo;
class GestureEvent;
struct FingerInfo;
struct WindowSizeBreakpoint;
enum class Axis;
template<typename T>
class RefPtr;
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
class FrameNode;
class NGGestureRecognizer;
class PanRecognizer;
struct NavDestinationInfo;
struct NavDestinationSwitchInfo;
struct RouterPageInfoNG;
struct SwiperContentInfo;
struct TabContentInfo;
struct TextChangeEventInfo;
enum class GestureActionPhase;
enum class GestureListenerType;
enum class NodeRenderState;
enum class ScrollEventType;
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::Napi {
class UIObserverListener {
public:
    UIObserverListener(napi_env env, napi_value callback) : env_(env), callback_(env, callback) {}
    ~UIObserverListener() = default;
    bool NapiEqual(napi_value callback) const
    {
        return callback_.Equals(callback);
    }

    void OnNavigationStateChange(const NG::NavDestinationInfo& info);
    void OnScrollEventStateChange(
        const std::string& id, int32_t uniqueId, NG::ScrollEventType eventType, float offset, Ace::Axis axis);
    void OnRouterPageStateChange(const NG::RouterPageInfoNG& pageInfo, napi_value context = nullptr);
    void OnDensityChange(double density);
    void OnWinSizeLayoutBreakpointChange(const WindowSizeBreakpoint info);
    void OnWillClick(const GestureEvent& gestureEventInfo, const ClickInfo& clickInfo,
        const RefPtr<NG::FrameNode> frameNode);
    void OnDidClick(const GestureEvent& gestureEventInfo, const ClickInfo& clickInfo,
        const RefPtr<NG::FrameNode> frameNode);
    void OnPanGestureStateChange(const GestureEvent& gestureEventInfo, const RefPtr<NG::PanRecognizer>& current,
        const RefPtr<NG::FrameNode> frameNode);
    void OnGestureStateChange(NG::GestureListenerType gestureListenerType, const GestureEvent& gestureEventInfo,
        const RefPtr<NG::NGGestureRecognizer>& current, const RefPtr<NG::FrameNode> frameNode,
        NG::GestureActionPhase phase);
    void OnTabContentStateChange(const NG::TabContentInfo& tabContentInfo);
    void OnTabChange(const NG::TabContentInfo& tabContentInfo);
    void OnNodeRenderStateChange(NG::FrameNode* frameNode, NG::NodeRenderState nodeRenderState);
    void OnNavDestinationSwitch(const NG::NavDestinationSwitchInfo& switchInfo, napi_value context);
    void OnTextChangeEvent(const NG::TextChangeEventInfo& info);
    void OnRouterPageSizeChange(const NG::RouterPageInfoNG& info, napi_value context = nullptr);
    void OnNavDestinationSizeChange(const NG::NavDestinationInfo& info);
    void HandleSwiperContentUpdate(const NG::SwiperContentInfo& info);

private:
    napi_value CreateNavDestinationSwitchInfoObj(const NG::NavDestinationSwitchInfo& switchInfo, napi_value context);
    napi_value CreateNavDestinationInfoObj(const NG::NavDestinationInfo& info);
    napi_value CreateRouterPageInfoObj(const NG::RouterPageInfoNG& info, napi_value context);
    napi_value GetFrameNodeObject(const RefPtr<NG::FrameNode>& frameNode);
    static napi_valuetype GetValueType(napi_env env, napi_value value);
    static napi_value GetNamedProperty(napi_env env, napi_value object, const std::string& propertyName);
    void AddBaseEventInfo(napi_value objValueEvent, const BaseEventInfo& baseEventInfo);
    void AddGestureEventInfoOne(napi_value objValueEvent, const GestureEvent& gestureEventInfo);
    void AddGestureEventInfoTwo(napi_value objValueEvent, const GestureEvent& gestureEventInfo);
    void AddGestureEventInfoThree(napi_value objValueEvent, const GestureEvent& gestureEventInfo);
    void AddGestureEventInfoFour(napi_value objValueEvent, const GestureEvent& gestureEventInfo);
    void AddTapLocationInfo(napi_value objTapGestureEventInfo, const GestureEvent& gestureEventInfo);
    void AddFingerListInfo(napi_value objValueClickEvent, const GestureEvent& gestureEventInfo);
    void AddFingerInfosInfo(napi_value objValueClickEvent, const GestureEvent& gestureEventInfo);
    void AddClickEventInfoOne(napi_value objValueClickEvent, const ClickInfo& clickInfo);
    void AddClickEventInfoTwo(napi_value objValueClickEvent, const ClickInfo& clickInfo);
    void AddFingerObjectInfo(napi_value napiFinger, const FingerInfo& finger);
    void AddTargetObject(napi_value objValueEvent, const BaseEventInfo& baseEventInfo);

    napi_env env_ = nullptr;
    NapiObserverCallback callback_;
};
} // namespace OHOS::Ace::Napi

#endif // FOUNDATION_ACE_INTERFACES_OBSERVER_LISTENER_H
