/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GENERATED_FOUNDATION_ACE_CAPI_TEST_WEB_H
#define GENERATED_FOUNDATION_ACE_CAPI_TEST_WEB_H
#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "test_fixtures.h"
#include "type_helpers.h"

#include "core/interfaces/native/utility/reverse_converter.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;
using namespace Converter;
using namespace TypeHelper;
namespace TestConst::Web {
const auto ATTRIBUTE_VALUE_NAME = "value";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_NAME = "javaScriptProxy";
const auto ATTRIBUTE_MEDIA_OPTIONS_NAME = "mediaOptions";
const auto ATTRIBUTE_NESTED_SCROLL_NAME = "nestedScroll";
const auto ATTRIBUTE_ENABLE_NATIVE_MEDIA_PLAYER_NAME = "enableNativeMediaPlayer";
const auto ATTRIBUTE_REGISTER_NATIVE_EMBED_RULE_NAME = "registerNativeEmbedRule";
const auto ATTRIBUTE_VALUE_I_SRC_NAME = "src";
const auto ATTRIBUTE_VALUE_I_SRC_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_VALUE_I_CONTROLLER_NAME = "controller";
const auto ATTRIBUTE_VALUE_I_CONTROLLER_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_VALUE_I_RENDER_MODE_NAME = "renderMode";
const auto ATTRIBUTE_VALUE_I_RENDER_MODE_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_VALUE_I_INCOGNITO_MODE_NAME = "incognitoMode";
const auto ATTRIBUTE_VALUE_I_INCOGNITO_MODE_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_VALUE_I_SHARED_RENDER_PROCESS_TOKEN_NAME = "sharedRenderProcessToken";
const auto ATTRIBUTE_VALUE_I_SHARED_RENDER_PROCESS_TOKEN_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_JAVA_SCRIPT_ACCESS_NAME = "javaScriptAccess";
const auto ATTRIBUTE_JAVA_SCRIPT_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_FILE_ACCESS_NAME = "fileAccess";
const auto ATTRIBUTE_FILE_ACCESS_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_ONLINE_IMAGE_ACCESS_NAME = "onlineImageAccess";
const auto ATTRIBUTE_ONLINE_IMAGE_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_DOM_STORAGE_ACCESS_NAME = "domStorageAccess";
const auto ATTRIBUTE_DOM_STORAGE_ACCESS_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_IMAGE_ACCESS_NAME = "imageAccess";
const auto ATTRIBUTE_IMAGE_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_MIXED_MODE_NAME = "mixedMode";
const auto ATTRIBUTE_MIXED_MODE_DEFAULT_VALUE = "MixedMode.None";
const auto ATTRIBUTE_ZOOM_ACCESS_NAME = "zoomAccess";
const auto ATTRIBUTE_ZOOM_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_GEOLOCATION_ACCESS_NAME = "geolocationAccess";
const auto ATTRIBUTE_GEOLOCATION_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_NAME_NAME = "name";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_NAME_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_METHOD_LIST_NAME = "methodList";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_METHOD_LIST_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_CONTROLLER_NAME = "controller";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_CONTROLLER_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_ASYNC_METHOD_LIST_NAME = "asyncMethodList";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_ASYNC_METHOD_LIST_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_PERMISSION_NAME = "permission";
const auto ATTRIBUTE_JAVA_SCRIPT_PROXY_I_PERMISSION_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_PASSWORD_NAME = "password";
const auto ATTRIBUTE_PASSWORD_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_CACHE_MODE_NAME = "cacheMode";
const auto ATTRIBUTE_CACHE_MODE_DEFAULT_VALUE = "CacheMode.Default";
const auto ATTRIBUTE_DARK_MODE_NAME = "darkMode";
const auto ATTRIBUTE_DARK_MODE_DEFAULT_VALUE = "WebDarkMode.Off";
const auto ATTRIBUTE_FORCE_DARK_ACCESS_NAME = "forceDarkAccess";
const auto ATTRIBUTE_FORCE_DARK_ACCESS_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_MEDIA_OPTIONS_I_RESUME_INTERVAL_NAME = "resumeInterval";
const auto ATTRIBUTE_MEDIA_OPTIONS_I_RESUME_INTERVAL_DEFAULT_VALUE = "-1";
const auto ATTRIBUTE_MEDIA_OPTIONS_I_AUDIO_EXCLUSIVE_NAME = "audioExclusive";
const auto ATTRIBUTE_MEDIA_OPTIONS_I_AUDIO_EXCLUSIVE_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_TABLE_DATA_NAME = "tableData";
const auto ATTRIBUTE_TABLE_DATA_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_WIDE_VIEW_MODE_ACCESS_NAME = "wideViewModeAccess";
const auto ATTRIBUTE_WIDE_VIEW_MODE_ACCESS_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_OVERVIEW_MODE_ACCESS_NAME = "overviewModeAccess";
const auto ATTRIBUTE_OVERVIEW_MODE_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_OVER_SCROLL_MODE_NAME = "overScrollMode";
const auto ATTRIBUTE_OVER_SCROLL_MODE_DEFAULT_VALUE = "OverScrollMode.NEVER";
const auto ATTRIBUTE_TEXT_ZOOM_ATIO_NAME = "textZoomAtio";
const auto ATTRIBUTE_TEXT_ZOOM_ATIO_DEFAULT_VALUE = "100";
const auto ATTRIBUTE_TEXT_ZOOM_RATIO_NAME = "textZoomRatio";
const auto ATTRIBUTE_TEXT_ZOOM_RATIO_DEFAULT_VALUE = "100";
const auto ATTRIBUTE_DATABASE_ACCESS_NAME = "databaseAccess";
const auto ATTRIBUTE_DATABASE_ACCESS_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_INITIAL_SCALE_NAME = "initialScale";
const auto ATTRIBUTE_INITIAL_SCALE_DEFAULT_VALUE = "empty";
const auto ATTRIBUTE_USER_AGENT_NAME = "userAgent";
const auto ATTRIBUTE_USER_AGENT_DEFAULT_VALUE = "";
const auto ATTRIBUTE_META_VIEWPORT_NAME = "metaViewport";
const auto ATTRIBUTE_META_VIEWPORT_DEFAULT_VALUE = "empty";
const auto ATTRIBUTE_MEDIA_PLAY_GESTURE_ACCESS_NAME = "mediaPlayGestureAccess";
const auto ATTRIBUTE_MEDIA_PLAY_GESTURE_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_MULTI_WINDOW_ACCESS_NAME = "multiWindowAccess";
const auto ATTRIBUTE_MULTI_WINDOW_ACCESS_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_WEB_STANDARD_FONT_NAME = "webStandardFont";
const auto ATTRIBUTE_WEB_STANDARD_FONT_DEFAULT_VALUE = "sans-serif";
const auto ATTRIBUTE_WEB_SERIF_FONT_NAME = "webSerifFont";
const auto ATTRIBUTE_WEB_SERIF_FONT_DEFAULT_VALUE = "serif";
const auto ATTRIBUTE_WEB_SANS_SERIF_FONT_NAME = "webSansSerifFont";
const auto ATTRIBUTE_WEB_SANS_SERIF_FONT_DEFAULT_VALUE = "sans-serif";
const auto ATTRIBUTE_WEB_FIXED_FONT_NAME = "webFixedFont";
const auto ATTRIBUTE_WEB_FIXED_FONT_DEFAULT_VALUE = "monospace";
const auto ATTRIBUTE_WEB_FANTASY_FONT_NAME = "webFantasyFont";
const auto ATTRIBUTE_WEB_FANTASY_FONT_DEFAULT_VALUE = "fantasy";
const auto ATTRIBUTE_WEB_CURSIVE_FONT_NAME = "webCursiveFont";
const auto ATTRIBUTE_WEB_CURSIVE_FONT_DEFAULT_VALUE = "cursive";
const auto ATTRIBUTE_DEFAULT_FIXED_FONT_SIZE_NAME = "defaultFixedFontSize";
const auto ATTRIBUTE_DEFAULT_FIXED_FONT_SIZE_DEFAULT_VALUE = "13";
const auto ATTRIBUTE_DEFAULT_FONT_SIZE_NAME = "defaultFontSize";
const auto ATTRIBUTE_DEFAULT_FONT_SIZE_DEFAULT_VALUE = "16";
const auto ATTRIBUTE_MIN_FONT_SIZE_NAME = "minFontSize";
const auto ATTRIBUTE_MIN_FONT_SIZE_DEFAULT_VALUE = "8";
const auto ATTRIBUTE_MIN_LOGICAL_FONT_SIZE_NAME = "minLogicalFontSize";
const auto ATTRIBUTE_MIN_LOGICAL_FONT_SIZE_DEFAULT_VALUE = "8";
const auto ATTRIBUTE_DEFAULT_TEXT_ENCODING_FORMAT_NAME = "defaultTextEncodingFormat";
const auto ATTRIBUTE_DEFAULT_TEXT_ENCODING_FORMAT_DEFAULT_VALUE = "UTF-8";
const auto ATTRIBUTE_FORCE_DISPLAY_SCROLL_BAR_NAME = "forceDisplayScrollBar";
const auto ATTRIBUTE_FORCE_DISPLAY_SCROLL_BAR_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_BLOCK_NETWORK_NAME = "blockNetwork";
const auto ATTRIBUTE_BLOCK_NETWORK_DEFAULT_VALUE = "empty";
const auto ATTRIBUTE_HORIZONTAL_SCROLL_BAR_ACCESS_NAME = "horizontalScrollBarAccess";
const auto ATTRIBUTE_HORIZONTAL_SCROLL_BAR_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_VERTICAL_SCROLL_BAR_ACCESS_NAME = "verticalScrollBarAccess";
const auto ATTRIBUTE_VERTICAL_SCROLL_BAR_ACCESS_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_PINCH_SMOOTH_NAME = "pinchSmooth";
const auto ATTRIBUTE_PINCH_SMOOTH_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_ALLOW_WINDOW_OPEN_METHOD_NAME = "allowWindowOpenMethod";
const auto ATTRIBUTE_ALLOW_WINDOW_OPEN_METHOD_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_JAVA_SCRIPT_ON_DOCUMENT_START_NAME = "javaScriptOnDocumentStart";
const auto ATTRIBUTE_JAVA_SCRIPT_ON_DOCUMENT_START_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_JAVA_SCRIPT_ON_DOCUMENT_END_NAME = "javaScriptOnDocumentEnd";
const auto ATTRIBUTE_JAVA_SCRIPT_ON_DOCUMENT_END_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_LAYOUT_MODE_NAME = "layoutMode";
const auto ATTRIBUTE_LAYOUT_MODE_DEFAULT_VALUE = "WebLayoutMode.NONE";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_FORWARD_NAME = "scrollForward";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_FORWARD_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_BACKWARD_NAME = "scrollBackward";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_BACKWARD_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_UP_NAME = "scrollUp";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_UP_DEFAULT_VALUE = "NestedScrollMode.SELF_FIRST";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_DOWN_NAME = "scrollDown";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_DOWN_DEFAULT_VALUE = "NestedScrollMode.SELF_FIRST";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_RIGHT_NAME = "scrollRight";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_RIGHT_DEFAULT_VALUE = "NestedScrollMode.SELF_FIRST";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_LEFT_NAME = "scrollLeft";
const auto ATTRIBUTE_NESTED_SCROLL_I_SCROLL_LEFT_DEFAULT_VALUE = "NestedScrollMode.SELF_FIRST";
const auto ATTRIBUTE_ENABLE_NATIVE_EMBED_MODE_NAME = "enableNativeEmbedMode";
const auto ATTRIBUTE_ENABLE_NATIVE_EMBED_MODE_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_COPY_OPTIONS_NAME = "copyOptions";
const auto ATTRIBUTE_COPY_OPTIONS_DEFAULT_VALUE = "CopyOptions.CROSS_DEVICE";
const auto ATTRIBUTE_TEXT_AUTOSIZING_NAME = "textAutosizing";
const auto ATTRIBUTE_TEXT_AUTOSIZING_DEFAULT_VALUE = "true";
const auto ATTRIBUTE_ENABLE_NATIVE_MEDIA_PLAYER_I_ENABLE_NAME = "enable";
const auto ATTRIBUTE_ENABLE_NATIVE_MEDIA_PLAYER_I_ENABLE_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_ENABLE_NATIVE_MEDIA_PLAYER_I_SHOULD_OVERLAY_NAME = "shouldOverlay";
const auto ATTRIBUTE_ENABLE_NATIVE_MEDIA_PLAYER_I_SHOULD_OVERLAY_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_ENABLE_SMOOTH_DRAG_RESIZE_NAME = "enableSmoothDragResize";
const auto ATTRIBUTE_ENABLE_SMOOTH_DRAG_RESIZE_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_SELECTION_MENU_OPTIONS_NAME = "selectionMenuOptions";
const auto ATTRIBUTE_SELECTION_MENU_OPTIONS_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_KEYBOARD_AVOID_MODE_NAME = "keyboardAvoidMode";
const auto ATTRIBUTE_KEYBOARD_AVOID_MODE_DEFAULT_VALUE = "empty";
const auto ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_NAME = "enableHapticFeedback";
const auto ATTRIBUTE_ENABLE_HAPTIC_FEEDBACK_DEFAULT_VALUE = "empty";
const auto ATTRIBUTE_REGISTER_NATIVE_EMBED_RULE_I_TAG_NAME = "tag";
const auto ATTRIBUTE_REGISTER_NATIVE_EMBED_RULE_I_TAG_DEFAULT_VALUE = "";
const auto ATTRIBUTE_REGISTER_NATIVE_EMBED_RULE_I_TYPE_NAME = "type";
const auto ATTRIBUTE_REGISTER_NATIVE_EMBED_RULE_I_TYPE_DEFAULT_VALUE = "";
} // namespace TestConst::Web

class WebModifierTest : public ModifierTestBase<GENERATED_ArkUIWebModifier,
                            &GENERATED_ArkUINodeModifiers::getWebModifier, GENERATED_ARKUI_WEB> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        for (auto& [id, strid, res] : Fixtures::resourceInitTable) {
            AddResource(id, res);
            AddResource(strid, res);
        }
    }
};

} // namespace OHOS::Ace::NG

#endif // GENERATED_FOUNDATION_ACE_CAPI_TEST_WEB_H
