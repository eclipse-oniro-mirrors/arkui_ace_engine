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
#include "core/interfaces/native/node/node_xcomponent_modifier.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_model_ng.h"
#include "core/pipeline/base/element_register.h"
#include "core/components_ng/base/view_abstract.h"
#include "frameworks/bridge/common/utils/utils.h"
#include "core/interfaces/native/node/node_api.h"

namespace OHOS::Ace::NG {
namespace {
const uint32_t ERROR_UINT_CODE = -1;
std::string g_strValue;

void SetXComponentBackgroundColor(ArkUINodeHandle node, uint32_t color)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto type = XComponentModelNG::GetType(frameNode);
    if (!XComponentModel::IsBackGroundColorAvailable(type)) {
        return;
    }
    ViewAbstract::SetBackgroundColor(frameNode, Color(color));
}

void ResetXComponentBackgroundColor(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto type = XComponentModelNG::GetType(frameNode);
    if (!XComponentModel::IsBackGroundColorAvailable(type)) {
        return;
    }
    ViewAbstract::SetBackgroundColor(frameNode, (type == XComponentType::SURFACE) ? Color::BLACK : Color::TRANSPARENT);
}

void SetXComponentOpacity(ArkUINodeHandle node, ArkUI_Float32 opacity)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    if (!XComponentModelNG::IsTexture(frameNode)) {
        return;
    }
    if ((LessNotEqual(opacity, 0.0)) || opacity > 1) {
        opacity = 1.0f;
    }
    ViewAbstract::SetOpacity(frameNode, opacity);
}

void ResetXComponentOpacity(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (!XComponentModelNG::IsTexture(frameNode)) {
        return;
    }
    ViewAbstract::SetOpacity(frameNode, 1.0f);
}

void SetXComponentId(ArkUINodeHandle node, const char* id)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    XComponentModelNG::SetXComponentId(frameNode, id);
}

void SetXComponentType(ArkUINodeHandle node, uint32_t type)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    XComponentModelNG::SetXComponentType(frameNode, static_cast<XComponentType>(type));
}

void SetXComponentSurfaceSize(ArkUINodeHandle node, uint32_t width, uint32_t height)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    XComponentModelNG::SetXComponentSurfaceSize(frameNode, width, height);
}

ArkUI_CharPtr GetXComponentId(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, "");
    g_strValue = XComponentModelNG::GetXComponentId(frameNode);
    return g_strValue.c_str();
}

ArkUI_Int32 GetXComponentType(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return static_cast<ArkUI_Int32>(XComponentModelNG::GetXComponentType(frameNode));
}

ArkUI_Uint32 GetXComponentSurfaceWidth(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return XComponentModelNG::GetXComponentSurfaceWidth(frameNode);
}

ArkUI_Uint32 GetXComponentSurfaceHeight(ArkUINodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_RETURN(frameNode, ERROR_UINT_CODE);
    return XComponentModelNG::GetXComponentSurfaceHeight(frameNode);
}
} // namespace

namespace NodeModifier {
const ArkUIXComponentModifier* GetXComponentModifier()
{
    static const ArkUIXComponentModifier modifier = {
        nullptr, // loadXComponent
        nullptr, // setXComponentOptions
        nullptr, // getXComponentSurfaceId
        nullptr, // getXComponentController
        SetXComponentBackgroundColor,
        ResetXComponentBackgroundColor,
        SetXComponentOpacity,
        ResetXComponentOpacity,
        SetXComponentId,
        SetXComponentType,
        SetXComponentSurfaceSize,
        GetXComponentId,
        GetXComponentType,
        GetXComponentSurfaceWidth,
        GetXComponentSurfaceHeight,
    };

    return &modifier;
}
} // namespace NodeModifier
} // namespace OHOS::Ace::NG