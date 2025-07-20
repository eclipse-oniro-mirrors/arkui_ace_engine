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
#include "ani_utils.h"
#include "ui/base/referenced.h"
#include "base/log/log.h"
#include "base/memory/ace_type.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/common/multi_thread_build_manager.h"
#include "core/common/thread_checker.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/token_theme/token_theme_storage.h"
#include "core/interfaces/native/ani/ani_theme.h"
#include "core/interfaces/native/ani/ani_theme_module.h"
#include "core/interfaces/native/ani/resource_ani_modifier.h"
#include "core/interfaces/native/implementation/frame_node_peer_impl.h"
#include "core/interfaces/native/node/theme_modifier.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "bridge/arkts_frontend/ani_graphics_module.h"
#include "bridge/arkts_frontend/arkts_frontend.h"
#include "bridge/arkts_frontend/ani_context_module.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components_ng/token_theme/token_colors.h"
#include "bridge/arkts_frontend/arkts_ani_utils.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace OHOS::Ace::NG {
namespace {
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int NUM_3 = 3;
constexpr int NUM_4 = 4;
constexpr int NUM_5 = 5;
constexpr int NUM_6 = 6;
constexpr int NUM_7 = 7;
constexpr int NUM_8 = 8;

constexpr uint32_t COLOR_ALPHA_OFFSET = 24;
constexpr uint32_t COLOR_ALPHA_VALUE = 0xFF000000;

uint32_t ColorAlphaAdapt(uint32_t origin)
{
    uint32_t result = origin;
    if ((origin >> COLOR_ALPHA_OFFSET) == 0) {
        result = origin | COLOR_ALPHA_VALUE;
    }
    return result;
}
} // namespace

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

ArkUI_Int32 GetFocusedInstanceId()
{
    auto container = Container::GetFoucsed();
    CHECK_NULL_RETURN(container, -1);
    return container->GetInstanceId();
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

void SetBackgroundImagePixelMap(ani_env* env, ArkUINodeHandle node, ani_ref pixelMapPtr, ArkUI_Int32 repeat)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto pixelMapValue = reinterpret_cast<void*>(pixelMapPtr);
    CHECK_NULL_VOID(pixelMapValue);
#if defined(PIXEL_MAP_SUPPORTED)
    auto pixelMap = PixelMap::CreatePixelMap(pixelMapValue);
    CHECK_NULL_VOID(pixelMap);
    ViewAbstract::SetBackgroundImage(frameNode, ImageSourceInfo { pixelMap });
#endif
    auto arkRepeat = static_cast<OHOS::Ace::ImageRepeat>(repeat);
    if (arkRepeat >= OHOS::Ace::ImageRepeat::NO_REPEAT && arkRepeat <= OHOS::Ace::ImageRepeat::REPEAT) {
        ViewAbstract::SetBackgroundImageRepeat(frameNode, arkRepeat);
    } else {
        ViewAbstract::SetBackgroundImageRepeat(frameNode, OHOS::Ace::ImageRepeat::NO_REPEAT);
    }
}

void SetCustomCallback(ani_env* env, ani_long ptr, ani_fn_object fnObjMeasure, ani_fn_object fnObjLayout)
{
    Framework::AniGraphicsModule::SetCustomCallback(env, ptr, fnObjMeasure, fnObjLayout);
}

ArkUI_Int32 RequireArkoalaNodeId(ArkUI_Int32 capacity)
{
    auto cursor = ElementRegister::GetInstance()->RequireArkoalaNodeId(capacity);
    return cursor;
}

ani_boolean CheckIsUIThread(ArkUI_Int32 instanceId)
{
    SyncInstanceId(instanceId);
    bool res = CheckThread(TaskExecutor::TaskType::UI);
    RestoreInstanceId();
    return res;
}

ani_boolean IsDebugMode(ArkUI_Int32 instanceId)
{
    SyncInstanceId(instanceId);
    auto& aceApplicationInfo = AceApplicationInfo::GetInstance();
    bool res = aceApplicationInfo.IsDebugForParallel();
    RestoreInstanceId();
    return res;
}

void OnMeasureInnerMeasure(ani_env* env, ani_long ptr)
{
    Framework::AniGraphicsModule::OnMeasureInnerMeasure(env, ptr);
}

void OnLayoutInnerLayout(ani_env* env, ani_long ptr)
{
    Framework::AniGraphicsModule::OnLayoutInnerLayout(env, ptr);
}

void SetParallelScoped(ani_boolean parallel)
{
    MultiThreadBuildManager::SetIsThreadSafeNodeScope(parallel);
}

Alignment ParseAlignment(int32_t align)
{
    Alignment alignment = Alignment::CENTER;
    switch (align) {
        case NUM_0:
            alignment = Alignment::TOP_LEFT;
            break;
        case NUM_1:
            alignment = Alignment::TOP_CENTER;
            break;
        case NUM_2:
            alignment = Alignment::TOP_RIGHT;
            break;
        case NUM_3:
            alignment = Alignment::CENTER_LEFT;
            break;
        case NUM_4:
            alignment = Alignment::CENTER;
            break;
        case NUM_5:
            alignment = Alignment::CENTER_RIGHT;
            break;
        case NUM_6:
            alignment = Alignment::BOTTOM_LEFT;
            break;
        case NUM_7:
            alignment = Alignment::BOTTOM_CENTER;
            break;
        case NUM_8:
            alignment = Alignment::BOTTOM_RIGHT;
            break;
        default:
            break;
    }
    return alignment;
}

void SetOverlayComponent(ani_long node, ani_long builderPtr, AniOverlayOptions options)
{
    CHECK_NULL_VOID(node);
    FrameNode* frameNode = reinterpret_cast<FrameNode*>(node);
    FrameNodePeer* overlayNodePeer = builderPtr ? reinterpret_cast<FrameNodePeer*>(builderPtr) : nullptr;
    RefPtr<NG::FrameNode> overlayNode = FrameNodePeer::GetFrameNodeByPeer(overlayNodePeer);
    CalcDimension x(options.x, DimensionUnit::VP);
    CalcDimension y(options.y, DimensionUnit::VP);
    Alignment align = ParseAlignment(options.alignment);
    ViewAbstract::SetOverlayBuilder(frameNode, overlayNode, align, x, y);
}
ani_double Vp2px(ani_double value, ani_int instanceId)
{
    if (NearZero(value)) {
        return 0;
    }
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, 0);
    ContainerScope cope(instanceId);
    double density = PipelineBase::GetCurrentDensity();
    return value * density;
}

ani_double Px2vp(ani_double value, ani_int instanceId)
{
    if (NearZero(value)) {
        return 0;
    }
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, 0);
    ContainerScope cope(instanceId);
    double density = PipelineBase::GetCurrentDensity();
    if (NearZero(density)) {
        return 0;
    }
    return value / density;
}

ani_double Fp2px(ani_double value, ani_int instanceId)
{
    if (NearZero(value)) {
        return 0;
    }
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, 0);
    ContainerScope cope(instanceId);
    double density = PipelineBase::GetCurrentDensity();
    if (NearZero(density)) {
        return 0;
    }
    double fontScale = 1.0;
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, 0);
    auto pipelineContext = container->GetPipelineContext();
    if (pipelineContext) {
        fontScale = pipelineContext->GetFontScale();
    }
    return  value * density * fontScale;
}

ani_double Px2fp(ani_double value, ani_int instanceId)
{
    if (NearZero(value)) {
        return 0;
    }
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, 0);
    ContainerScope cope(instanceId);
    double density = PipelineBase::GetCurrentDensity();
    if (NearZero(density)) {
        return 0;
    }
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, 0);
    auto pipelineContext = container->GetPipelineContext();
    double fontScale = 1.0;
    if (pipelineContext) {
        fontScale = pipelineContext->GetFontScale();
    }
    double ratio = density * fontScale;
    double fpValue = value / ratio;
    return fpValue;
}

ani_double Lpx2px(ani_double value, ani_int instanceId)
{
    if (NearZero(value)) {
        return 0;
    }
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, 0);
    ContainerScope cope(instanceId);
    double density = PipelineBase::GetCurrentDensity();
    if (NearZero(density)) {
        return 0;
    }
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, 0);
    auto pipelineContext = container->GetPipelineContext();
    auto window = container->GetWindow();
    CHECK_NULL_RETURN(window, 0);
    auto width = window->GetCurrentWindowRect().Width();
    auto frontend = container->GetFrontend();
    CHECK_NULL_RETURN(frontend, 0);
    auto windowConfig = frontend->GetWindowConfig();
    if (pipelineContext && pipelineContext->IsContainerModalVisible()) {
        int32_t multiplier = 2;
        width -= multiplier * (CONTAINER_BORDER_WIDTH + CONTENT_PADDING).ConvertToPx();
    }
    if (!windowConfig.autoDesignWidth) {
        windowConfig.UpdateDesignWidthScale(width);
    }
    return value * windowConfig.designWidthScale;
}

ani_double Px2lpx(ani_double value, ani_int instanceId)
{
    if (NearZero(value)) {
        return 0;
    }
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, 0);
    ContainerScope cope(instanceId);
    CHECK_NULL_RETURN(value, 0);
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, 0);
    auto pipelineContext = container->GetPipelineContext();
    auto window = container->GetWindow();
    CHECK_NULL_RETURN(window, 0);
    auto width = window->GetCurrentWindowRect().Width();
    auto frontend = container->GetFrontend();
    CHECK_NULL_RETURN(frontend, 0);
    auto windowConfig = frontend->GetWindowConfig();
    if (pipelineContext && pipelineContext->IsContainerModalVisible()) {
        int32_t multiplier = 2;
        width -= multiplier * (CONTAINER_BORDER_WIDTH + CONTENT_PADDING).ConvertToPx();
    }
    if (!windowConfig.autoDesignWidth) {
        windowConfig.UpdateDesignWidthScale(width);
    }
    return value / windowConfig.designWidthScale;
}

ArkUI_Uint32 GetColorValueByString(const std::string& src)
{
    Color color;
    Color::ParseColorString(src, color);
    return color.GetValue();
}

ArkUI_Uint32 GetColorValueByNumber(ArkUI_Uint32 src)
{
    Color color(ColorAlphaAdapt(src));
    return color.GetValue();
}

void SendThemeToNative(ani_env* env, ani_array colorArray, ani_int id)
{
    auto colors = AceType::MakeRefPtr<AniThemeColors>();
    colors->SetColors(env, colorArray);
    colors->id_ = id;
    auto res = ArktsAniUtils::JsonStringify(env, (ani_object)colorArray);
    auto themeScopeId = static_cast<int32_t>(id);

    AniTheme aniTheme;
    aniTheme.SetColors(colors);
    aniTheme.id_ = themeScopeId;
    AniThemeScope::AddAniTheme(themeScopeId, aniTheme);

    // save the current theme when Theme was created by WithTheme container
    if (AniThemeScope::isCurrentThemeDefault || themeScopeId > 0) {
        std::optional<NG::AniTheme> themeOpt = std::make_optional(AniThemeScope::GetAniTheme(themeScopeId));
        AniThemeScope::aniCurrentTheme.swap(themeOpt);
    }
}

void RemoveThemeInNative(ani_env* env, ani_int withThemeId)
{
    auto themeScopeId = static_cast<int32_t>(withThemeId);
    AniThemeScope::RemoveAniTheme(themeScopeId);
}

void SetDefaultTheme(ani_env* env, ani_array colorArray, ani_boolean isDark)
{
    auto isDarkValue = static_cast<bool>(isDark);
    ani_size length;
    env->Array_GetLength(colorArray, &length);
    if (length < TokenColors::TOTAL_NUMBER) {
        LOGW("colorArray incorrect in SetDefaultTheme");
    }
    std::vector<uint32_t> colors;
    auto basisTheme = TokenThemeStorage::GetInstance()->ObtainSystemTheme();
    for (size_t i = 0; i < TokenColors::TOTAL_NUMBER; i++) {
        // type ResourceColor = number | string | Resource
        ani_ref value;
        auto status = env->Array_Get(colorArray, i, &value);
        if (status != ANI_OK) {
            LOGW("SetDefaultTheme colorArray get index: %{public}d failed", i);
            continue;
        }
        Color color;
        RefPtr<ResourceObject> resObj;
        bool isColorAvailable = false;
        if (!ResourceAniModifier::ParseAniColor(env, static_cast<ani_object>(value), color, resObj)) {
            if (basisTheme) {
                color = basisTheme->Colors()->GetByIndex(i);
                isColorAvailable = true;
            }
        } else {
            isColorAvailable = true;
        }
        colors.push_back(color.GetValue());
    }
    NodeModifier::GetThemeModifier()->setDefaultTheme(colors.data(), isDarkValue);
}

void UpdateColorMode(ani_int colorMode)
{
    auto colorModeValue = static_cast<int32_t>(colorMode);
    AniThemeModule::UpdateColorMode(colorModeValue);
}

void RestoreColorMode()
{
    AniThemeModule::RestoreColorMode();
}

void SetThemeScopeId(ani_env* env, ani_int themeScopeId)
{
    auto scopeId = static_cast<int32_t>(themeScopeId);
    AniThemeScope::isCurrentThemeDefault = scopeId == 0;

    std::optional<AniTheme> themeOpt = AniThemeScope::IsAniThemeExists(scopeId)
                                           ? std::make_optional(AniThemeScope::GetAniTheme(scopeId))
                                           : std::nullopt;

    AniThemeScope::aniCurrentTheme.swap(themeOpt);
}

void CreateAndBindTheme(ani_env* env, ani_int themeScopeId, ani_int themeId, ani_array colorsArg, ani_int colorMode,
    ani_fn_object onThemeScopeDestroy)
{
    int32_t themeScopeIdValue = static_cast<int32_t>(themeScopeId);
    int32_t themeIdValue = static_cast<int32_t>(themeId);
    int32_t colorModeValue = static_cast<int32_t>(colorMode);
    
    if (!colorsArg) {
        return;
    }

    std::vector<uint32_t> colors;
    std::vector<RefPtr<ResourceObject>> resObjs;
    if (!AniThemeModule::HandleThemeColorsArg(env, colorsArg, colors, resObjs)) {
        return;
    }

    if (!onThemeScopeDestroy) {
        return;
    }
    auto containerId = Container::CurrentId();
    ani_vm* vm = nullptr;
    env->GetVM(&vm);
    std::function<void()> onThemeScopeDestroyFunc = [vm, func = onThemeScopeDestroy, containerId]() {
        ContainerScope scope(containerId);
        auto* env = ArktsAniUtils::GetAniEnv(vm);
        if (env) {
            ani_ref result;
            env->FunctionalObject_Call(func, 0, nullptr, &result);
        }
    };

    auto themeModifier = NodeModifier::GetThemeModifier();
    auto theme = themeModifier->createTheme(themeId, colors.data(), colorModeValue, static_cast<void*>(&resObjs));
    CHECK_NULL_VOID(theme);
    ArkUINodeHandle node = themeModifier->getWithThemeNode(themeScopeId);
    if (!node) {
        node = AniThemeModule::CreateWithThemeNode(themeScopeId);
    }
    themeModifier->createThemeScope(node, theme);
    themeModifier->setOnThemeScopeDestroy(node, reinterpret_cast<void*>(&onThemeScopeDestroyFunc));
}

void ApplyParentThemeScopeId(ani_env* env, ani_long self, ani_long parent)
{
    auto* selfPtr = reinterpret_cast<UINode*>(self);
    auto* parentPtr = reinterpret_cast<UINode*>(parent);
    if (!selfPtr || !parentPtr) {
        return;
    }
    int32_t elementThemeScopeId = selfPtr->GetThemeScopeId();
    if (parentPtr && elementThemeScopeId == 0) {
        int32_t themeScopeId = parentPtr->GetThemeScopeId();
        if (elementThemeScopeId != themeScopeId) {
            selfPtr->SetThemeScopeId(themeScopeId);
        }
    }
}

const ArkUIAniCommonModifier* GetCommonAniModifier()
{
    static const ArkUIAniCommonModifier impl = {
        .getHostContext = OHOS::Ace::NG::GetHostContext,
        .syncInstanceId = OHOS::Ace::NG::SyncInstanceId,
        .restoreInstanceId = OHOS::Ace::NG::RestoreInstanceId,
        .setDrawCallback = OHOS::Ace::NG::SetDrawCallback,
        .getCurrentInstanceId = OHOS::Ace::NG::GetCurrentInstanceId,
        .getFocusedInstanceId = OHOS::Ace::NG::GetFocusedInstanceId,
        .builderProxyNodeConstruct = OHOS::Ace::NG::BuilderProxyNodeConstruct,
        .getSharedLocalStorage = OHOS::Ace::NG::GetSharedLocalStorage,
        .setBackgroundImagePixelMap = OHOS::Ace::NG::SetBackgroundImagePixelMap,
        .setCustomCallback = OHOS::Ace::NG::SetCustomCallback,
        .requireArkoalaNodeId = OHOS::Ace::NG::RequireArkoalaNodeId,
        .checkIsUIThread = OHOS::Ace::NG::CheckIsUIThread,
        .isDebugMode =  OHOS::Ace::NG::IsDebugMode,
        .onMeasureInnerMeasure = OHOS::Ace::NG::OnMeasureInnerMeasure,
        .onLayoutInnerLayout = OHOS::Ace::NG::OnLayoutInnerLayout,
        .setParallelScoped = OHOS::Ace::NG::SetParallelScoped,
        .setOverlayComponent = OHOS::Ace::NG::SetOverlayComponent,
        .vp2px = OHOS::Ace::NG::Vp2px,
        .px2vp = OHOS::Ace::NG::Px2vp,
        .fp2px = OHOS::Ace::NG::Fp2px,
        .px2fp = OHOS::Ace::NG::Px2fp,
        .lpx2px = OHOS::Ace::NG::Lpx2px,
        .px2lpx = OHOS::Ace::NG::Px2lpx,
        .getColorValueByNumber = OHOS::Ace::NG::GetColorValueByNumber,
        .sendThemeToNative = OHOS::Ace::NG::SendThemeToNative,
        .removeThemeInNative = OHOS::Ace::NG::RemoveThemeInNative,
        .setDefaultTheme = OHOS::Ace::NG::SetDefaultTheme,
        .updateColorMode = OHOS::Ace::NG::UpdateColorMode,
        .restoreColorMode = OHOS::Ace::NG::RestoreColorMode,
        .setThemeScopeId = OHOS::Ace::NG::SetThemeScopeId,
        .createAndBindTheme = OHOS::Ace::NG::CreateAndBindTheme,
        .applyParentThemeScopeId = OHOS::Ace::NG::ApplyParentThemeScopeId };
    return &impl;
}

void SetDrawModifier(ani_env* env, ani_long ptr, ani_int flag, ani_object fnObj)
{
    Framework::AniGraphicsModule::SetDrawModifier(env, ptr, flag, fnObj);
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
