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
#ifndef FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_CANVAS_RENDERING_CONTEXT2D_PEER_IMPL_H
#define FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_CANVAS_RENDERING_CONTEXT2D_PEER_IMPL_H

#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/common/container_consts.h"
#include "core/components_ng/pattern/canvas/canvas_pattern.h"

#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
enum class CanvasCallbackType {
    ON_ATTACH = 0,
    ON_DETACH,
    UNKNOWN
};
using CanvasCallbackList = std::list<CallbackHelper<Callback_Void>>;
class CanvasRenderingContext2DPeerImpl : public Referenced {
public:
    CanvasRenderingContext2DPeerImpl() = default;
    ~CanvasRenderingContext2DPeerImpl() override = default;

    void OnAttachToCanvas();
    void OnDetachFromCanvas();

    Ark_NativePointer TriggerStartImageAnalyzer(const std::vector<ImageAnalyzerType> vector);
    void TriggerStopImageAnalyzer();
    Ark_Int32 TriggerGetHeight();
    Ark_Int32 TriggerGetWidth();

    void UpdateDensity()
    {
        CHECK_NULL_VOID(pattern_);
        double density = PipelineBase::GetCurrentDensity();
        pattern_->SetDensity(density);
    }
    void SetAntiAlias(bool antialias)
    {
        antialias_ = antialias;
    }

    void UpdateAntiAlias()
    {
        CHECK_NULL_VOID(pattern_);
        pattern_->SetAntiAlias(antialias_);
    }

    void SetCanvasPattern(const RefPtr<AceType>& pattern);

    void On(CallbackHelper<Callback_Void> &&callback, const CanvasCallbackType& type);
    void Off(CallbackHelper<Callback_Void> &&callback, const CanvasCallbackType& type);

    void SetInstanceId(int32_t instanceId)
    {
        instanceId_ = instanceId;
    }

    int32_t GetInstanceId()
    {
        return instanceId_;
    }

protected:
    RefPtr<CanvasPattern> pattern_;
    bool antialias_ = false;
    int32_t instanceId_ = INSTANCE_ID_UNDEFINED;

private:
    CanvasCallbackList::const_iterator FindCallbackInList(const CanvasCallbackList& callbackFuncPairList,
    const CallbackHelper<Callback_Void>& callback) const;
    void DeleteCallbackFromList(const CallbackHelper<Callback_Void>& callback, const CanvasCallbackType& type);
    void AddCallbackToList(CallbackHelper<Callback_Void> &&callback, const CanvasCallbackType& type);

    bool isImageAnalyzing_ = false;
    std::vector<ImageAnalyzerType> vector_;
    CanvasCallbackList attachCallback_;
    CanvasCallbackList detachCallback_;
};

} // namespace OHOS::Ace::NG::GeneratedModifier
#endif // FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_SCROLLER_PEER_IMPL_H
