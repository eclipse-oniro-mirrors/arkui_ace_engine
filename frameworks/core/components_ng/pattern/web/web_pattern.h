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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_PATTERN_H

#include <optional>
#include <string>
#include <utility>

#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/web/web_property.h"
#include "core/components_ng/manager/select_overlay/select_overlay_manager.h"
#include "core/components_ng/manager/select_overlay/select_overlay_proxy.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/components_ng/pattern/web/web_pattern_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/render_surface.h"
#include "nweb_handler.h"

namespace OHOS::Ace::NG {
namespace {

struct MouseClickInfo {
    double x = -1;
    double y = -1;
    TimeStamp start;
};

#ifdef OHOS_STANDARD_SYSTEM
struct TouchInfo {
    double x = -1;
    double y = -1;
    int32_t id = -1;
};

struct TouchHandleState {
    int32_t id = -1;
    int32_t x = -1;
    int32_t y = -1;
    int32_t edge_height = 0;
};

enum WebOverlayType { INSERT_OVERLAY, SELECTION_OVERLAY, INVALID_OVERLAY };
#endif
} // namespace

class WebPattern : public Pattern {
    DECLARE_ACE_TYPE(WebPattern, Pattern);

public:
    using SetWebIdCallback = std::function<void(int32_t)>;
    using JsProxyCallback = std::function<void()>;

    WebPattern();
    WebPattern(std::string webSrc, const RefPtr<WebController>& webController);
    WebPattern(std::string webSrc, const SetWebIdCallback& setWebIdCallback);

    ~WebPattern() override = default;

    enum class VkState {
        VK_NONE,
        VK_SHOW,
        VK_HIDE
    };

    std::optional<std::string> GetSurfaceNodeName() const override
    {
        return "RosenWeb";
    }

    bool IsAtomicNode() const override
    {
        return true;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<WebEventHub>();
    }

    void OnModifyDone() override;

    void SetWebSrc(const std::string& webSrc)
    {
        if (webSrc_ != webSrc_) {
            OnWebSrcUpdate();
            webSrc_ = webSrc;
        }
    }

    const std::optional<std::string>& GetWebSrc() const
    {
        return webSrc_;
    }

    void SetWebData(const std::string& webData)
    {
        if (webData_ != webData) {
            OnWebDataUpdate();
            webData_ = webData;
        }
    }

    const std::optional<std::string>& GetWebData() const
    {
        return webData_;
    }

    void SetCustomScheme(const std::string& scheme)
    {
        customScheme_ = scheme;
    }

    const std::optional<std::string>& GetCustomScheme() const
    {
        return customScheme_;
    }

    void SetWebController(const RefPtr<WebController>& webController)
    {
        // TODO: add web controller diff function.
        webController_ = webController;
    }

    RefPtr<WebController> GetWebController() const
    {
        return webController_;
    }

    void SetSetWebIdCallback(SetWebIdCallback&& SetIdCallback)
    {
        setWebIdCallback_ = std::move(SetIdCallback);
    }

    SetWebIdCallback GetSetWebIdCallback() const
    {
        return setWebIdCallback_;
    }

    void SetJsProxyCallback(JsProxyCallback&& jsProxyCallback)
    {
        jsProxyCallback_ = std::move(jsProxyCallback);
    }

    void CallJsProxyCallback()
    {
        if (jsProxyCallback_) {
            jsProxyCallback_();
        }
    }

    RefPtr<WebEventHub> GetWebEventHub()
    {
        return GetEventHub<WebEventHub>();
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true };
    }

    ACE_DEFINE_PROPERTY_GROUP(WebProperty, WebPatternProperty);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, JsEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MediaPlayGestureAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OnLineImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DomStorageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MixedMode, MixedModeContent);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ZoomAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, GeolocationAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, UserAgent, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, CacheMode, WebCacheMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverviewModeAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileFromUrlAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DatabaseAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, TextZoomRatio, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebDebuggingAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BackgroundColor, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, InitialScale, float);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, PinchSmoothModeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MultiWindowAccessEnabled, bool);

    void RequestFullScreen();
    void ExitFullScreen();
    bool RunQuickMenu(std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> callback);
    void OnQuickMenuDismissed();
    void OnTouchSelectionChanged(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle);
    void UpdateTouchHandleForOverlay();
    void UpdateLocale();

private:
    void RegistVirtualKeyBoardListener();
    bool ProcessVirtualKeyBoard(int32_t width, int32_t height, double keyboard);
    void UpdateWebLayoutSize();
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void OnAttachToFrameNode() override;
    void OnWindowShow() override;
    void OnWindowHide() override;
    void OnInActive() override;
    void OnActive() override;

    void OnWebSrcUpdate();
    void OnWebDataUpdate();
    void OnJsEnabledUpdate(bool value);
    void OnMediaPlayGestureAccessUpdate(bool value);
    void OnFileAccessEnabledUpdate(bool value);
    void OnOnLineImageAccessEnabledUpdate(bool value);
    void OnDomStorageAccessEnabledUpdate(bool value);
    void OnImageAccessEnabledUpdate(bool value);
    void OnMixedModeUpdate(MixedModeContent value);
    void OnZoomAccessEnabledUpdate(bool value);
    void OnGeolocationAccessEnabledUpdate(bool value);
    void OnUserAgentUpdate(const std::string& value);
    void OnCacheModeUpdate(WebCacheMode value);
    void OnOverviewModeAccessEnabledUpdate(bool value);
    void OnFileFromUrlAccessEnabledUpdate(bool value);
    void OnDatabaseAccessEnabledUpdate(bool value);
    void OnTextZoomRatioUpdate(int32_t value);
    void OnWebDebuggingAccessEnabledUpdate(bool value);
    void OnPinchSmoothModeEnabledUpdate(bool value);
    void OnBackgroundColorUpdate(int32_t value);
    void OnInitialScaleUpdate(float value);
    void OnMultiWindowAccessEnabledUpdate(bool value);

    void InitEvent();
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitMouseEvent(const RefPtr<InputEventHub>& inputHub);
    void InitCommonDragDropEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitDragEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleDragStart(const GestureEvent& info);
    void HandleDragUpdate(const GestureEvent& info);
    void HandleDragEnd(const GestureEvent& info);
    void HandleDragCancel();
    bool GenerateDragDropInfo(NG::DragDropInfo& dragDropInfo);
    void HandleMouseEvent(MouseInfo& info);
    void HandleAxisEvent(AxisInfo& info);
    void WebOnMouseEvent(const MouseInfo& info);
    bool HandleDoubleClickEvent(const MouseInfo& info);
    void SendDoubleClickEvent(const MouseClickInfo& info);
    void InitFocusEvent(const RefPtr<FocusHub>& focusHub);
    void HandleFocusEvent();
    void HandleBlurEvent();
    bool HandleKeyEvent(const KeyEvent& keyEvent);
    bool WebOnKeyEvent(const KeyEvent& keyEvent);
    void WebRequestFocus();

    void HandleTouchDown(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchUp(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchMove(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchCancel(const TouchEventInfo& info);

    bool IsTouchHandleValid(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> handle);
    bool IsTouchHandleShow(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> handle);
#ifdef OHOS_STANDARD_SYSTEM
    WebOverlayType GetTouchHandleOverlayType(
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle,
        std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle);
#endif
    void RegisterSelectOverlayCallback(SelectOverlayInfo& selectInfo,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
        std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> callback);
    void CloseSelectOverlay();
    RectF ComputeTouchHandleRect(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> touchHandle);
    std::optional<OffsetF> GetCoordinatePoint();

    struct TouchInfo {
        float x = -1.0f;
        float y = -1.0f;
        int32_t id = -1;
    };
    static bool ParseTouchInfo(const TouchEventInfo& info, std::list<TouchInfo>& touchInfos);

    std::optional<std::string> webSrc_;
    std::optional<std::string> webData_;
    std::optional<std::string> customScheme_;
    RefPtr<WebController> webController_;
    SetWebIdCallback setWebIdCallback_ = nullptr;
    JsProxyCallback jsProxyCallback_ = nullptr;
    RefPtr<WebDelegate> delegate_;
    RefPtr<RenderSurface> renderSurface_ = RenderSurface::Create();
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<InputEvent> axisEvent_;
    RefPtr<SelectOverlayProxy> selectOverlayProxy_ = nullptr;
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle_ = nullptr;
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle_ = nullptr;
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle_ = nullptr;
    float selectHotZone_ = 10.0f;
    RefPtr<DragEvent> dragEvent_;
    bool isUrlLoaded_ = false;
    std::queue<MouseClickInfo> doubleClickQueue_;
    bool needOnFocus_ = false;
    Size drawSize_;
    Size drawSizeCache_;
    bool needUpdateWeb_ = true;
    bool isFocus_ = false;
    VkState isVirtualKeyBoardShow_ { VkState::VK_NONE };
    bool isDragging_ = false;
    bool isW3cDragEvent_ = false;
    bool isWindowShow_ = true;
    bool isActive_ = true;

    ACE_DISALLOW_COPY_AND_MOVE(WebPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_PATTERN_H
