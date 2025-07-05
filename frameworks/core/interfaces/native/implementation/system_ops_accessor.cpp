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
#include <numeric>

#include "arkoala_api_generated.h"

#include "core/common/container.h"
#include "core/common/resource/resource_manager.h"
#include "core/common/resource/resource_wrapper.h"
#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "ui/base/geometry/dimension.h"

static thread_local std::vector<int32_t> restoreInstanceIds_;

namespace OHOS::Ace::NG {
constexpr uint32_t COLOR_ALPHA_OFFSET = 24;
constexpr uint32_t COLOR_ALPHA_VALUE = 0xFF000000;
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::NG::GeneratedModifier {
namespace SystemOpsAccessor {
Ark_NativePointer StartFrameImpl()
{
    return {};
}
void EndFrameImpl(Ark_NativePointer root)
{
    Converter::FC->Clear();
}
void SyncInstanceIdImpl(Ark_Int32 instanceId)
{
    restoreInstanceIds_.emplace_back(Container::CurrentId());
    ContainerScope::UpdateCurrent(instanceId);
}
void RestoreInstanceIdImpl()
{
    if (restoreInstanceIds_.empty()) {
        ContainerScope::UpdateCurrent(INSTANCE_ID_UNDEFINED);
        return;
    }
    ContainerScope::UpdateCurrent(restoreInstanceIds_.back());
    restoreInstanceIds_.pop_back();
}
Ark_Int32 GetResourceIdImpl(const Ark_String* bundleName,
                            const Ark_String* moduleName,
                            const Array_String* params)
{
    auto valueBundleName = Converter::Convert<std::string>(*bundleName);
    auto valueModuleName = Converter::Convert<std::string>(*moduleName);
    auto paramsStr = Converter::Convert<std::vector<std::string>>(*params);
    std::string resourceStr = "";
    if (paramsStr.size() > 0) {
        resourceStr = paramsStr[0];
    }
    auto resourceObject =
        AceType::MakeRefPtr<Ace::ResourceObject>(valueBundleName, valueModuleName, Container::CurrentIdSafely());
    auto resourceAdapter = ResourceManager::GetInstance().GetOrCreateResourceAdapter(resourceObject);
    CHECK_NULL_RETURN(resourceAdapter, -1);
    uint32_t resId = resourceAdapter->GetResId(resourceStr);
    return resId;
}
void ResourceManagerResetImpl()
{
    ResourceManager::GetInstance().Reset();
}
void SetFrameCallbackImpl(const Callback_Number_Void* onFrameCallback,
                          const Callback_Number_Void* onIdleCallback,
                          const Ark_Number* delayTime)
{
    CHECK_NULL_VOID(delayTime);
    auto delayTimeInt = Converter::Convert<int32_t>(*delayTime);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto onFrameCallbackFunc = [callback = CallbackHelper(*onFrameCallback)](double delayTimeInt) -> void {
        auto delayTime = Converter::ArkValue<Ark_Number>(delayTimeInt);
        callback.Invoke(delayTime);
    };
    auto onIdleCallbackFunc = [callback = CallbackHelper(*onIdleCallback)](double delayTimeInt) -> void {
        auto delayTime = Converter::ArkValue<Ark_Number>(delayTimeInt);
        callback.Invoke(delayTime);
    };
    context->AddFrameCallback(std::move(onFrameCallbackFunc), std::move(onIdleCallbackFunc), delayTimeInt);
}
RefPtr<ResourceWrapper> CreateResourceWrapper(const std::string& bundleName, const std::string& moduleName)
{
    auto resourceObject = AceType::MakeRefPtr<ResourceObject>(bundleName, moduleName, Container::CurrentIdSafely());
    RefPtr<ResourceAdapter> resourceAdapter = nullptr;
    RefPtr<ThemeConstants> themeConstants = nullptr;
    if (SystemProperties::GetResourceDecoupling()) {
        resourceAdapter = ResourceManager::GetInstance().GetOrCreateResourceAdapter(resourceObject);
        CHECK_NULL_RETURN(resourceAdapter, nullptr);
    } else {
        auto themeManager = PipelineBase::CurrentThemeManager();
        CHECK_NULL_RETURN(themeManager, nullptr);
        themeConstants = themeManager->GetThemeConstants();
        CHECK_NULL_RETURN(themeConstants, nullptr);
    }
    auto resourceWrapper = AceType::MakeRefPtr<ResourceWrapper>(themeConstants, resourceAdapter);
    return resourceWrapper;
}
Ark_LengthMetricsCustom ResourceToLengthMetricsImpl(const Ark_Resource* res)
{
    CalcDimension result;
    const auto errValue = Converter::ArkValue<Ark_LengthMetricsCustom>(result);
    CHECK_NULL_RETURN(res, errValue);
    auto resId = Converter::Convert<int32_t>(res->id);
    auto bundleName = Converter::Convert<std::string>(res->bundleName);
    auto moduleName = Converter::Convert<std::string>(res->moduleName);
    auto resourceWrapper = CreateResourceWrapper(bundleName, moduleName);
    CHECK_NULL_RETURN(resourceWrapper, errValue);
    if (resId == -1) {
        auto optParams = Converter::OptConvert<std::vector<std::string>>(res->params);
        if (!optParams.has_value() || optParams->size() < 1) {
            return errValue;
        }
        auto params = optParams.value();
        result = resourceWrapper->GetDimensionByName(params[0]);
        return Converter::ArkValue<Ark_LengthMetricsCustom>(result);
    }
    auto resType = Converter::OptConvert<int32_t>(res->type);
    if (!resType.has_value()) {
        return errValue;
    }
    auto typeValue = resType.value();
    if (typeValue == static_cast<int32_t>(OHOS::Ace::NG::Converter::ResourceType::STRING)) {
        auto value = resourceWrapper->GetString(resId);
        StringUtils::StringToCalcDimensionNG(value, result, false, DimensionUnit::VP);
    }
    if (typeValue == static_cast<int32_t>(OHOS::Ace::NG::Converter::ResourceType::INTEGER)) {
        auto value = std::to_string(resourceWrapper->GetInt(resId));
        StringUtils::StringToDimensionWithUnitNG(value, result, DimensionUnit::VP);
    }
    if (typeValue == static_cast<int32_t>(OHOS::Ace::NG::Converter::ResourceType::FLOAT)) {
        result = resourceWrapper->GetDimension(resId);
    }
    return Converter::ArkValue<Ark_LengthMetricsCustom>(result);
}
void ParseArrayNumber(Color& color, std::vector<uint32_t>& indexes, bool result)
{
    indexes.clear();
    if (result) {
        indexes.emplace_back(1);
    } else {
        indexes.emplace_back(0);
    }
    indexes.emplace_back(color.GetRed());
    indexes.emplace_back(color.GetGreen());
    indexes.emplace_back(color.GetBlue());
    indexes.emplace_back(color.GetAlpha());
}
uint32_t ColorAlphaAdapt(uint32_t origin)
{
    uint32_t result = origin;
    if ((origin >> COLOR_ALPHA_OFFSET) == 0) {
        result = origin | COLOR_ALPHA_VALUE;
    }
    return result;
}
Array_Number ColorMetricsResourceColorImpl(const Ark_Resource* color)
{
    Color colorColor;
    std::vector<uint32_t> indexes;
    ParseArrayNumber(colorColor, indexes, false);
    Array_Number errValue = Converter::ArkValue<Array_Number>(indexes, Converter::FC);
    auto resId = Converter::Convert<int32_t>(color->id);
    auto bundleName = Converter::Convert<std::string>(color->bundleName);
    auto moduleName = Converter::Convert<std::string>(color->moduleName);
    auto resourceWrapper = CreateResourceWrapper(bundleName, moduleName);
    CHECK_NULL_RETURN(resourceWrapper, errValue);
    if (resId == -1) {
        auto optParams = Converter::OptConvert<std::vector<std::string>>(color->params);
        if (!optParams.has_value() || optParams->size() < 1) {
            return errValue;
        }
        auto params = optParams.value();
        colorColor = resourceWrapper->GetColorByName(params[0]);
        ParseArrayNumber(colorColor, indexes, true);
        return Converter::ArkValue<Array_Number>(indexes, Converter::FC);
    }
    auto resType = Converter::OptConvert<int32_t>(color->type);
    if (!resType.has_value()) {
        return errValue;
    }
    auto typeValue = resType.value();
    if (typeValue == static_cast<int32_t>(OHOS::Ace::NG::Converter::ResourceType::STRING)) {
        auto value = resourceWrapper->GetString(resId);
        if (!Color::ParseColorString(value, colorColor)) {
            return errValue;
        }
        ParseArrayNumber(colorColor, indexes, true);
        return Converter::ArkValue<Array_Number>(indexes, Converter::FC);
    }
    if (typeValue == static_cast<int32_t>(OHOS::Ace::NG::Converter::ResourceType::INTEGER)) {
        auto value = resourceWrapper->GetInt(resId);
        colorColor = Color(ColorAlphaAdapt(value));
        ParseArrayNumber(colorColor, indexes, true);
        return Converter::ArkValue<Array_Number>(indexes, Converter::FC);
    }
    if (typeValue == static_cast<int32_t>(OHOS::Ace::NG::Converter::ResourceType::COLOR)) {
        colorColor = resourceWrapper->GetColor(resId);
        ParseArrayNumber(colorColor, indexes, true);
        indexes.emplace_back(resId);
        return Converter::ArkValue<Array_Number>(indexes, Converter::FC);
    }
    return errValue;
}
} // namespace SystemOpsAccessor
const GENERATED_ArkUISystemOpsAccessor* GetSystemOpsAccessor()
{
    static const GENERATED_ArkUISystemOpsAccessor SystemOpsAccessorImpl {
        SystemOpsAccessor::StartFrameImpl,
        SystemOpsAccessor::EndFrameImpl,
        SystemOpsAccessor::SyncInstanceIdImpl,
        SystemOpsAccessor::RestoreInstanceIdImpl,
        SystemOpsAccessor::GetResourceIdImpl,
        SystemOpsAccessor::ResourceManagerResetImpl,
        SystemOpsAccessor::SetFrameCallbackImpl,
        SystemOpsAccessor::ResourceToLengthMetricsImpl,
        SystemOpsAccessor::ColorMetricsResourceColorImpl,
    };
    return &SystemOpsAccessorImpl;
}

} // namespace OHOS::Ace::NG::GeneratedModifier
