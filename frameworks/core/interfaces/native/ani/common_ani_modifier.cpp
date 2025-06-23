/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "common_ani_modifier.h"
#include "base/log/log.h"
#include "base/memory/ace_type.h"
#include "core/common/container_scope.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "bridge/arkts_frontend/ani_graphics_module.h"
#include "bridge/arkts_frontend/arkts_frontend.h"
#include "bridge/arkts_frontend/ani_context_module.h"

#include <memory>
#include <vector>

namespace OHOS::Ace::NG {

static thread_local std::vector<int32_t> restoreInstanceIds_;

ani_ref* GetHostContext()
{
    auto contextValue = OHOS::Ace::Framework::AniContextModule::GetAniContext();
    if (contextValue) {
        return contextValue.get();
    }
    return nullptr;
}

void SyncInstanceId(ArkUI_Int32 instanceId)
{
    restoreInstanceIds_.emplace_back(ContainerScope::CurrentId());
    ContainerScope::UpdateCurrent(instanceId);
}

void RestoreInstanceId()
{
    if (restoreInstanceIds_.empty()) {
        ContainerScope::UpdateCurrent(-1);
        return;
    }
    ContainerScope::UpdateCurrent(restoreInstanceIds_.back());
    restoreInstanceIds_.pop_back();
}

void SetDrawCallback(ani_env* env, ani_long ptr, ani_fn_object fnObj)
{
    Framework::AniGraphicsModule::SetDrawCallback(env, ptr, fnObj);
}

ArkUI_Int32 GetCurrentInstanceId()
{
    return ContainerScope::CurrentId();
}

ani_long BuilderProxyNodeConstruct(ArkUI_Int32 id)
{
    auto proxyNode = NG::FrameNode::GetOrCreateFrameNode(
        "BuilderProxyNode", id, []() { return AceType::MakeRefPtr<StackPattern>(); });
    CHECK_NULL_RETURN(proxyNode, 0);
    auto stackLayoutAlgorithm = proxyNode->GetLayoutProperty<LayoutProperty>();
    CHECK_NULL_RETURN(stackLayoutAlgorithm, 0);
    stackLayoutAlgorithm->UpdateAlignment(Alignment::TOP_LEFT);
    proxyNode->IncRefCount();
    return reinterpret_cast<ani_long>(AceType::RawPtr(proxyNode));
}

ani_ref GetSharedLocalStorage()
{
    auto context = NG::PipelineContext::GetCurrentContextSafely();
    if (context == nullptr) {
        TAG_LOGE(AceLogTag::ACE_LAYOUT_INSPECTOR, "GetSharedLocalStorage-ani can not get current context.");
        return nullptr;
    }
    auto frontend = context->GetFrontend();
    if (frontend == nullptr) {
        TAG_LOGE(AceLogTag::ACE_LAYOUT_INSPECTOR, "GetSharedLocalStorage-ani can not get current frontend.");
        return nullptr;
    }
    auto arkTsFrontend = AceType::DynamicCast<ArktsFrontend>(frontend);
    if (arkTsFrontend == nullptr) {
        TAG_LOGE(AceLogTag::ACE_LAYOUT_INSPECTOR, "GetSharedLocalStorage-ani can not convert to arkts frontend.");
        return nullptr;
    }
    int32_t currentInstance = Container::CurrentIdSafely();
    auto storage = arkTsFrontend->GetShared(currentInstance);
    if (storage) {
        return storage;
    }
    return nullptr;
}

const ArkUIAniCommonModifier* GetCommonAniModifier()
{
    static const ArkUIAniCommonModifier impl = {
        .getHostContext = OHOS::Ace::NG::GetHostContext,
        .syncInstanceId = OHOS::Ace::NG::SyncInstanceId,
        .restoreInstanceId = OHOS::Ace::NG::RestoreInstanceId,
        .setDrawCallback = OHOS::Ace::NG::SetDrawCallback,
        .getCurrentInstanceId = OHOS::Ace::NG::GetCurrentInstanceId,
        .builderProxyNodeConstruct = OHOS::Ace::NG::BuilderProxyNodeConstruct,
        .getSharedLocalStorage = OHOS::Ace::NG::GetSharedLocalStorage,
    };
    return &impl;
}

void SetDrawModifier(ani_env* env, ani_long ptr, ani_object fnObj)
{
    Framework::AniGraphicsModule::SetDrawModifier(env, ptr, fnObj);
}

void Invalidate(ani_env* env, ani_long ptr)
{
    Framework::AniGraphicsModule::Invalidate(env, ptr);
}

const ArkUIAniDrawModifier* GetArkUIAniDrawModifier()
{
    static const ArkUIAniDrawModifier impl = {
        .setDrawModifier = OHOS::Ace::NG::SetDrawModifier,
        .invalidate = OHOS::Ace::NG::Invalidate,
    };
    return &impl;
}
} // namespace OHOS::Ace::NG
