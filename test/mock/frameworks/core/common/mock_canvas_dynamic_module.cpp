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

#include "mock_canvas_dynamic_module.h"

#include "base/utils/utils.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_api_bridge.h"
#include "core/interfaces/arkoala/arkoala_api.h"
#include "core/interfaces/native/canvas_runtime_bridge_api.h"

namespace OHOS::Ace {

namespace {
#ifdef PIXEL_MAP_SUPPORTED
void TransferCanvasRenderingContext2DFromImageBitmapMockImpl(
    const RefPtr<RenderingContext2DModel>& context, const RefPtr<AceType>& pixelMap)
{
    auto model = AceType::DynamicCast<CanvasRenderingContext2DModel>(context);
    CHECK_NULL_VOID(model);
    model->TransferFromImageBitmap(pixelMap);
}
#else
void TransferCanvasRenderingContext2DFromImageBitmapMockImpl(
    const RefPtr<RenderingContext2DModel>& context, const std::shared_ptr<::OHOS::Ace::ImageData>& imageData)
{
    auto model = AceType::DynamicCast<CanvasRenderingContext2DModel>(context);
    CHECK_NULL_VOID(model);
    model->TransferFromImageBitmap(imageData);
}
#endif

const ArkUICanvasRuntimeBridge g_canvasRuntimeBridgeMock = {
    .transferCanvasRenderingContext2DFromImageBitmap = TransferCanvasRenderingContext2DFromImageBitmapMockImpl,
};
void NoopCanvasInvalidate(ArkUINodeHandle node)
{
}

void NoopCanvasRSCallback(ArkUINodeHandle node, void* callback)
{
}

ArkUINodeHandle NoopCreateCanvasFrameNode(ArkUI_Int32 nodeId)
{
    return nullptr;
}

const ArkUICanvasModifier g_canvasModifierMock = {
    .createModel = nullptr,
    .setCanvasOnReady = nullptr,
    .resetCanvasOnReady = nullptr,
    .setCanvasEnableAnalyzer = nullptr,
    .resetCanvasEnableAnalyzer = nullptr,
    .setCanvasAntiAlias = nullptr,
    .setCanvasImmediateRender = nullptr,
    .resetCanvasImmediateRender = nullptr,
    .setCanvasUpdateUnit = nullptr,
    .resetCanvasUpdateUnit = nullptr,
    .createFrameNode = NoopCreateCanvasFrameNode,
    .setInvalidate = NoopCanvasInvalidate,
    .setAntiAlias = nullptr,
    .setRSCanvasCallback = NoopCanvasRSCallback,
    .setUpdateContextCallback = nullptr,
    .setRSCanvasForDrawingContext = nullptr,
};
} // namespace

void MockCanvasDynamicModule::RegisterAttributes(panda::Local<panda::ObjectRef> object, panda::ecmascript::EcmaVM* vm)
{
}

const void* MockCanvasDynamicModule::GetDynamicModifier()
{
    return &g_canvasModifierMock;
}

const void* MockCanvasDynamicModule::GetCustomModifier(const std::string& name)
{
    if (name == "canvasBridge") {
        return &g_canvasRuntimeBridgeMock;
    }
    return nullptr;
}

} // namespace OHOS::Ace
