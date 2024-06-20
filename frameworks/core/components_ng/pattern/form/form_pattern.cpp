/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/form/form_pattern.h"

#include "form_constants.h"
#include "form_info_base.h"
#include "locale_config.h"
#include "locale_info.h"
#include "pointer_event.h"
#include "transaction/rs_interfaces.h"

#include "adapter/ohos/osal/resource_adapter_impl_v2.h"
#include "base/geometry/dimension.h"
#include "base/i18n/localization.h"
#include "base/log/log_wrapper.h"
#include "base/utils/string_utils.h"
#include "base/utils/system_properties.h"
#include "base/utils/time_util.h"
#include "base/utils/utils.h"
#include "core/common/form_manager.h"
#include "core/common/frontend.h"
#include "core/common/resource/resource_manager.h"
#include "core/components/form/resource/form_manager_delegate.h"
#include "core/components/form/sub_container.h"
#include "core/components_ng/pattern/form/form_event_hub.h"
#include "core/components_ng/pattern/form/form_layout_property.h"
#include "core/components_ng/pattern/form/form_node.h"
#include "core/components_ng/pattern/form/form_theme.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/shape/rect_pattern.h"
#include "core/components_ng/pattern/symbol/constants.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/pipeline_ng/pipeline_context.h"

#if OHOS_STANDARD_SYSTEM
#include "form_info.h"
#endif

#include "core/common/udmf/udmf_client.h"
static const int64_t MAX_NUMBER_OF_JS = 0x20000000000000;

namespace OHOS::Ace::NG {
namespace {
constexpr double FORM_CLICK_OPEN_LIMIT_DISTANCE = 20.0;
constexpr uint32_t DELAY_TIME_FOR_FORM_SUBCONTAINER_CACHE = 30000;
constexpr uint32_t DELAY_TIME_FOR_FORM_SNAPSHOT_3S = 3000;
constexpr uint32_t DELAY_TIME_FOR_DELETE_IMAGE_NODE = 100;
constexpr double ARC_RADIUS_TO_DIAMETER = 2.0;
constexpr double NON_TRANSPARENT_VAL = 1.0;
constexpr double TRANSPARENT_VAL = 0;
constexpr int32_t MAX_CLICK_DURATION = 500000000; // ns
constexpr int32_t DOUBLE = 2;
constexpr char FORM_DIMENSION_SPLITTER = '*';
constexpr int32_t FORM_SHAPE_CIRCLE = 2;
constexpr Dimension TIME_LIMIT_FONT_SIZE_VAL = 18.0_fp;
constexpr double TIME_LIMIT_TRANSPARENT_VAL = 0.3;
constexpr int32_t TIME_LIMIT_RADIUS_SIZE = 60;
constexpr char TIME_LIMIT_RESOURCE_NAME[] = "form_disable_time_limit";

class FormSnapshotCallback : public Rosen::SurfaceCaptureCallback {
public:
    explicit FormSnapshotCallback(const WeakPtr<FormPattern>& node) : weakFormPattern_(node) {}
    ~FormSnapshotCallback() override = default;
    void OnSurfaceCapture(std::shared_ptr<Media::PixelMap> pixelMap) override
    {
        auto formPattern_ = weakFormPattern_.Upgrade();
        CHECK_NULL_VOID(formPattern_);
        formPattern_->OnSnapshot(pixelMap);
    }

private:
    WeakPtr<FormPattern> weakFormPattern_ = nullptr;
};
} // namespace

FormPattern::FormPattern()
{
    ACE_SCOPED_TRACE("FormCreate");
}

FormPattern::~FormPattern() = default;

void FormPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
    host->GetRenderContext()->SetClipToBounds(true);
    host->GetRenderContext()->UpdateRenderGroup(true);
    // Init the render context for RSSurfaceNode from FRS.
    externalRenderContext_ = RenderContext::Create();
    // for external RSNode, name is meaningless.
    static RenderContext::ContextParam param = { RenderContext::ContextType::EXTERNAL, std::nullopt };
    externalRenderContext_->InitContext(false, param);
    InitFormManagerDelegate();
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnCache([weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        auto context = host->GetContextRefPtr();
        CHECK_NULL_VOID(context);
        auto subContainer = pattern->GetSubContainer();
        CHECK_NULL_VOID(subContainer);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        auto id = subContainer->GetRunningCardId();
        FormManager::GetInstance().AddSubContainer(id, subContainer);
        uiTaskExecutor.PostDelayedTask(
            [id, nodeId = subContainer->GetNodeId()] {
                auto cachedSubContainer = FormManager::GetInstance().GetSubContainer(id);
                if (cachedSubContainer != nullptr && cachedSubContainer->GetNodeId() == nodeId) {
                    FormManager::GetInstance().RemoveSubContainer(id);
                }
            },
            DELAY_TIME_FOR_FORM_SUBCONTAINER_CACHE, "ArkUIFormRemoveSubContainer");
    });

    InitClickEvent();

    scopeId_ = Container::CurrentId();
}

void FormPattern::InitClickEvent()
{
    // Init click event for static form.
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureEventHub = host->GetOrCreateGestureEventHub();
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto formPattern = weak.Upgrade();
        CHECK_NULL_VOID(formPattern);
        formPattern->HandleStaticFormEvent(
            { static_cast<float>(info.GetLocalLocation().GetX()), static_cast<float>(info.GetLocalLocation().GetY()) });
    };
    auto clickEvent = AceType::MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gestureEventHub->AddClickEvent(clickEvent);

    // check touch duration in click event
    auto touchCallback = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto formPattern = weak.Upgrade();
        CHECK_NULL_VOID(formPattern);
        auto touchType = info.GetTouches().front().GetTouchType();
        if (touchType == TouchType::DOWN) {
            formPattern->HandleTouchDownEvent(info);
            return;
        }
        if (touchType == TouchType::UP || touchType == TouchType::CANCEL) {
            formPattern->HandleTouchUpEvent(info);
            return;
        }
    };
    auto touchEvent = AceType::MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
    gestureEventHub->AddTouchEvent(touchEvent);
}

void FormPattern::HandleTouchDownEvent(const TouchEventInfo& event)
{
    touchDownTime_ = event.GetTimeStamp();
    shouldResponseClick_ = true;
    if (!event.GetTouches().empty()) {
        lastTouchLocation_ = event.GetTouches().front().GetScreenLocation();
    }
}

void FormPattern::HandleTouchUpEvent(const TouchEventInfo& event)
{
    auto duration = event.GetTimeStamp().time_since_epoch().count() - touchDownTime_.time_since_epoch().count();
    if (duration > MAX_CLICK_DURATION) {
        TAG_LOGI(AceLogTag::ACE_FORM, "reject click. duration is %{public}lld.", duration);
        shouldResponseClick_ = false;
        return;
    }
    if (event.GetTouches().empty()) {
        return;
    }
    auto distance = event.GetTouches().front().GetScreenLocation() - lastTouchLocation_;
    if (distance.GetDistance() > FORM_CLICK_OPEN_LIMIT_DISTANCE) {
        shouldResponseClick_ = false;
    }
}

void FormPattern::HandleUnTrustForm()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (externalRenderContext_) {
        auto renderContext = DynamicCast<NG::RosenRenderContext>(host->GetRenderContext());
        CHECK_NULL_VOID(renderContext);
        renderContext->RemoveChild(externalRenderContext_);
    }

    isUnTrust_ = true;
    isLoaded_ = true;
    if (!isJsCard_) {
        LoadFormSkeleton();
    }

    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    auto parent = host->GetParent();
    CHECK_NULL_VOID(parent);
    parent->MarkNeedSyncRenderTree();
    parent->RebuildRenderContextTree();
    host->GetRenderContext()->RequestNextFrame();
}

void FormPattern::UpdateBackgroundColorWhenUnTrustForm()
{
    if (!isUnTrust_) {
        return;
    }

    if (colorMode != SystemProperties::GetColorMode()) {
        colorMode = SystemProperties::GetColorMode();
        HandleUnTrustForm();
    }
}

void FormPattern::HandleSnapshot(uint32_t delayTime)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto executor = pipeline->GetTaskExecutor();
    CHECK_NULL_VOID(executor);
    snapshotTimestamp_ = GetCurrentTimestamp();
    executor->PostDelayedTask(
        [weak = WeakClaim(this), delayTime]() mutable {
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            int64_t currentTime = GetCurrentTimestamp();
            if (currentTime - form->snapshotTimestamp_ < delayTime) {
                TAG_LOGD(AceLogTag::ACE_FORM, "another snapshot task has been posted.");
                return;
            }
            form->TakeSurfaceCaptureForUI();
        },
        TaskExecutor::TaskType::UI, delayTime, "ArkUIFormTakeSurfaceCapture");
}

void FormPattern::HandleStaticFormEvent(const PointF& touchPoint)
{
    if (formLinkInfos_.empty() || isDynamic_ || !shouldResponseClick_) {
        return;
    }
    for (const auto& info : formLinkInfos_) {
        auto linkInfo = JsonUtil::ParseJsonString(info);
        CHECK_NULL_VOID(linkInfo);
        auto action = linkInfo->GetValue("action")->GetString();
        auto rectStr = linkInfo->GetValue("formLinkRect")->GetString();
        RectF linkRect = RectF::FromString(rectStr);
        if (linkRect.IsInRegion(touchPoint)) {
            OnActionEvent(action);
            break;
        }
    }
}

void FormPattern::HandleEnableForm(const bool enable)
{
    TAG_LOGI(AceLogTag::ACE_FORM, "FormPattern::HandleEnableForm, enable = %{public}d", enable);
    if (enable) {
        RemoveDisableFormStyle();
    } else {
        LoadDisableFormStyle();
    }
}

void FormPattern::TakeSurfaceCaptureForUI()
{
    if (isFrsNodeDetached_) {
        TAG_LOGI(AceLogTag::ACE_FORM, "Frs node is detached, cancel snapshot.");
        return;
    }
    
    if (isDynamic_) {
        formLinkInfos_.clear();
    }
    TAG_LOGI(AceLogTag::ACE_FORM, "Static-form take snapshot.");
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto renderContext = host->GetRenderContext();
    auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
    auto opacity = renderContext->GetOpacityValue(NON_TRANSPARENT_VAL);
    if (visible == VisibleType::INVISIBLE || visible == VisibleType::GONE || opacity == TRANSPARENT_VAL) {
        TAG_LOGI(AceLogTag::ACE_FORM, "The form is invisible, TakeSurfaceCaptureForUI later.");
        needSnapshotAgain_ = true;
        return;
    }
    auto externalContext = DynamicCast<NG::RosenRenderContext>(GetExternalRenderContext());
    CHECK_NULL_VOID(externalContext);
    auto rsNode = externalContext->GetRSNode();
    CHECK_NULL_VOID(rsNode);
    auto& rsInterface = Rosen::RSInterfaces::GetInstance();
    rsInterface.TakeSurfaceCaptureForUI(rsNode, std::make_shared<FormSnapshotCallback>(WeakClaim(this)));
}

void FormPattern::OnSnapshot(std::shared_ptr<Media::PixelMap> pixelMap)
{
    ContainerScope scope(scopeId_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto uiTaskExecutor =
        SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    uiTaskExecutor.PostTask([weak = WeakClaim(this), pixelMap] {
        auto formPattern = weak.Upgrade();
        CHECK_NULL_VOID(formPattern);
        formPattern->HandleOnSnapshot(pixelMap);
        }, "ArkUIFormHandleOnSnapshot");
}

void FormPattern::HandleOnSnapshot(std::shared_ptr<Media::PixelMap> pixelMap)
{
    TAG_LOGI(AceLogTag::ACE_FORM, "call.");
    CHECK_NULL_VOID(pixelMap);
    pixelMap_ = PixelMap::CreatePixelMap(reinterpret_cast<void*>(&pixelMap));
    UpdateStaticCard();
    isSnapshot_ = true;
    needSnapshotAgain_ = false;
}

void FormPattern::OnAccessibilityChildTreeRegister(uint32_t windowId, int32_t treeId, int64_t accessibilityId)
{
    TAG_LOGD(AceLogTag::ACE_FORM, "call, treeId: %{public}d, id: %{public}" PRId64, treeId, accessibilityId);
    if (formManagerBridge_ == nullptr) {
        TAG_LOGE(AceLogTag::ACE_FORM, "formManagerBridge_ is null");
        return;
    }
    formManagerBridge_->OnAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
}

void FormPattern::OnAccessibilityChildTreeDeregister()
{
    TAG_LOGD(AceLogTag::ACE_FORM, "call.");
    if (formManagerBridge_ == nullptr) {
        TAG_LOGE(AceLogTag::ACE_FORM, "formManagerBridge_ is null");
        return;
    }
    formManagerBridge_->OnAccessibilityChildTreeDeregister();
}

void FormPattern::OnAccessibilityDumpChildInfo(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    TAG_LOGD(AceLogTag::ACE_FORM, "call.");
    if (formManagerBridge_ == nullptr) {
        TAG_LOGE(AceLogTag::ACE_FORM, "formManagerBridge_ is null");
        return;
    }
    formManagerBridge_->OnAccessibilityDumpChildInfo(params, info);
}

RefPtr<AccessibilitySessionAdapter> FormPattern::GetAccessibilitySessionAdapter()
{
    return accessibilitySessionAdapter_;
}

void FormPattern::UpdateStaticCard()
{
    // 1. Use imageNode to display pixelMap
    UpdateImageNode();
    // 2. Remove FrsNode from formNode
    RemoveFrsNode();
    // 3. Release renderer obj
    ReleaseRenderer();
}

void FormPattern::DeleteImageNode()
{
    ContainerScope scope(scopeId_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto child = host->GetLastChild();
    CHECK_NULL_VOID(child);

    std::list<RefPtr<UINode>> children = host->GetChildren();
    if (children.size() > 0 && child->GetTag() == V2::IMAGE_ETS_TAG) {
        host->RemoveChildAtIndex(children.size() - 1);
    }
}

void FormPattern::DeleteImageNodeAfterRecover(bool needHandleCachedClick)
{
    // delete image rs node
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto imageNode = GetImageNode();
    if (imageNode) {
        renderContext->RemoveChild(imageNode->GetRenderContext());

        // delete image frame node
        DeleteImageNode();
    }

    // set frs node non transparent
    auto externalRenderContext = DynamicCast<NG::RosenRenderContext>(GetExternalRenderContext());
    CHECK_NULL_VOID(externalRenderContext);
    externalRenderContext->SetOpacity(NON_TRANSPARENT_VAL);
    TAG_LOGI(AceLogTag::ACE_FORM, "delete imageNode and setOpacity:1");

    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    auto parent = host->GetParent();
    CHECK_NULL_VOID(parent);
    parent->MarkNeedSyncRenderTree();
    parent->RebuildRenderContextTree();
    renderContext->RequestNextFrame();

    // handle cached pointer event
    if (needHandleCachedClick && formManagerBridge_) {
        formManagerBridge_->HandleCachedClickEvents();
    }
}

RefPtr<FrameNode> FormPattern::CreateImageNode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto formNode = DynamicCast<FormNode>(host);
    CHECK_NULL_RETURN(formNode, nullptr);
    auto imageId = formNode->GetImageId();
    auto imageNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, imageId, AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_RETURN(imageNode, nullptr);
    auto imagePattern = imageNode->GetPattern<ImagePattern>();
    CHECK_NULL_RETURN(imagePattern, nullptr);
    imagePattern->SetSyncLoad(true);
    host->AddChild(imageNode);
    auto eventHub = imageNode->GetOrCreateGestureEventHub();
    if (eventHub != nullptr) {
        eventHub->RemoveDragEvent();
    }
    return imageNode;
}

RefPtr<FrameNode> FormPattern::GetImageNode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto child = host->GetLastChild();
    CHECK_NULL_RETURN(child, nullptr);
    if (child->GetTag() == V2::IMAGE_ETS_TAG) {
        return AceType::DynamicCast<FrameNode>(child);
    }
    return nullptr;
}

void FormPattern::UpdateImageNode()
{
    ContainerScope scope(scopeId_);
    CHECK_NULL_VOID(pixelMap_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    DeleteImageNode();
    auto imageNode = CreateImageNode();
    CHECK_NULL_VOID(imageNode);
    auto pixelLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(pixelLayoutProperty);
    auto pixelSourceInfo = ImageSourceInfo(pixelMap_);

    auto width = static_cast<float>(cardInfo_.width.Value()) - cardInfo_.borderWidth * DOUBLE;
    auto height = static_cast<float>(cardInfo_.height.Value()) - cardInfo_.borderWidth * DOUBLE;
    CalcSize idealSize = { CalcLength(width), CalcLength(height) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    imageNode->UpdateLayoutConstraint(layoutConstraint);
    pixelLayoutProperty->UpdateImageSourceInfo(pixelSourceInfo);
    auto externalContext = DynamicCast<NG::RosenRenderContext>(imageNode->GetRenderContext());
    CHECK_NULL_VOID(externalContext);
    externalContext->SetVisible(true);

    imageNode->MarkModifyDone();
    imageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void FormPattern::RemoveFrsNode()
{
    ContainerScope scope(scopeId_);
    CHECK_NULL_VOID(externalRenderContext_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = DynamicCast<NG::RosenRenderContext>(host->GetRenderContext());
    CHECK_NULL_VOID(renderContext);
    renderContext->RemoveChild(externalRenderContext_);

    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    auto parent = host->GetParent();
    CHECK_NULL_VOID(parent);
    parent->MarkNeedSyncRenderTree();
    parent->RebuildRenderContextTree();
    host->GetRenderContext()->RequestNextFrame();
}

void FormPattern::ReleaseRenderer()
{
    ContainerScope scope(scopeId_);
    CHECK_NULL_VOID(formManagerBridge_);
    formManagerBridge_->ReleaseRenderer();
}

void FormPattern::OnRebuildFrame()
{
    if (isSnapshot_) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->AddChild(externalRenderContext_, 0);
}

void FormPattern::OnVisibleChange(bool isVisible)
{
    isVisible_ = isVisible;
}

void FormPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureEventHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    // FormComponent do not response to user's onClick callback.
    gestureEventHub->ClearUserOnClick();

    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto &&layoutConstraint = layoutProperty->GetCalcLayoutConstraint();
    CHECK_NULL_VOID(layoutConstraint);
    auto size = layoutConstraint->selfIdealSize;
    CHECK_NULL_VOID(size);
    auto sizeWidth = size->Width();
    auto sizeHeight = size->Height();
    CHECK_NULL_VOID(sizeWidth);
    CHECK_NULL_VOID(sizeHeight);
    auto width = sizeWidth->GetDimension();
    auto height = sizeHeight->GetDimension();
    if (width.Unit() == DimensionUnit::PERCENT || height.Unit() == DimensionUnit::PERCENT) {
        /**
         * If DimensionUnit is DimensionUnit::PERCENT, it need parentNode-size to calculate formNode-size.
         * However, the parentNode-size cannot be obtained in the current callback function,
         * so HandleFormComponent in OnDirtyLayoutWrapperSwap function.
         */
        return;
    }
    // Convert DimensionUnit to DimensionUnit::PX
    auto info = layoutProperty->GetRequestFormInfo().value_or(RequestFormInfo());
    info.width = Dimension(width.ConvertToPx());
    info.height = Dimension(height.ConvertToPx());
    auto &&borderWidthProperty = layoutProperty->GetBorderWidthProperty();
    float borderWidth = 0.0f;
    if (borderWidthProperty && borderWidthProperty->topDimen) {
        borderWidth = borderWidthProperty->topDimen->ConvertToPx();
    }
    info.borderWidth = borderWidth;
    layoutProperty->UpdateRequestFormInfo(info);
    UpdateBackgroundColorWhenUnTrustForm();
    info.obscuredMode = isFormObscured_;
    HandleFormComponent(info);
}

bool FormPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    isBeenLayout_ = true;
    auto size = dirty->GetGeometryNode()->GetFrameSize();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, false);
    auto info = layoutProperty->GetRequestFormInfo().value_or(RequestFormInfo());
    info.width = Dimension(size.Width());
    info.height = Dimension(size.Height());
    auto &&borderWidthProperty = layoutProperty->GetBorderWidthProperty();
    float borderWidth = 0.0f;
    if (borderWidthProperty && borderWidthProperty->topDimen) {
        borderWidth = borderWidthProperty->topDimen->ConvertToPx();
    }
    info.borderWidth = borderWidth;
    layoutProperty->UpdateRequestFormInfo(info);

    UpdateBackgroundColorWhenUnTrustForm();
    info.obscuredMode = isFormObscured_;
    HandleFormComponent(info);
    return false;
}

void FormPattern::HandleFormComponent(const RequestFormInfo& info)
{
    if (info.bundleName != cardInfo_.bundleName || info.abilityName != cardInfo_.abilityName ||
        info.moduleName != cardInfo_.moduleName || info.cardName != cardInfo_.cardName ||
        info.dimension != cardInfo_.dimension || info.renderingMode != cardInfo_.renderingMode) {
        AddFormComponent(info);
    } else {
        UpdateFormComponent(info);
    }
}

void FormPattern::AddFormComponent(const RequestFormInfo& info)
{
    ACE_FUNCTION_TRACE();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    // When cardInfo has changed, it will call AddForm in Fwk
    // If the width or height equal to zero, it will not
    if (NonPositive(info.width.Value()) || NonPositive(info.height.Value())) {
        TAG_LOGW(AceLogTag::ACE_FORM, "Invalid form size.");
        return;
    }
    TAG_LOGI(AceLogTag::ACE_FORM, "width: %{public}f   height: %{public}f  borderWidth: %{public}f",
        info.width.Value(), info.height.Value(), info.borderWidth);
    cardInfo_ = info;
    if (info.dimension == static_cast<int32_t>(OHOS::AppExecFwk::Constants::Dimension::DIMENSION_1_1)
        || info.shape == FORM_SHAPE_CIRCLE) {
        BorderRadiusProperty borderRadius;
        Dimension diameter = std::min(info.width, info.height);
        borderRadius.SetRadius(diameter / ARC_RADIUS_TO_DIAMETER);
        host->GetRenderContext()->UpdateBorderRadius(borderRadius);
    }
    isJsCard_ = true;
#if OHOS_STANDARD_SYSTEM
    AppExecFwk::FormInfo formInfo;
    if (FormManagerDelegate::GetFormInfo(info.bundleName, info.moduleName, info.cardName, formInfo) &&
        formInfo.uiSyntax == AppExecFwk::FormType::ETS) {
        isJsCard_ = false;
    }
#endif

    CreateCardContainer();
    if (host->IsDraggable()) {
        EnableDrag();
    }

    if (!isJsCard_) {
        LoadFormSkeleton();
    }

    if (formManagerBridge_) {
#if OHOS_STANDARD_SYSTEM
        formManagerBridge_->AddForm(host->GetContextRefPtr(), info, formInfo);
#else
        formManagerBridge_->AddForm(host->GetContextRefPtr(), info);
#endif
    }
}

void FormPattern::UpdateFormComponent(const RequestFormInfo& info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (cardInfo_.allowUpdate != info.allowUpdate) {
        cardInfo_.allowUpdate = info.allowUpdate;
        if (subContainer_) {
            subContainer_->SetAllowUpdate(cardInfo_.allowUpdate);
        }
        if (formManagerBridge_) {
            formManagerBridge_->SetAllowUpdate(cardInfo_.allowUpdate);
        }
    }
    if (cardInfo_.width != info.width || cardInfo_.height != info.height || cardInfo_.borderWidth != info.borderWidth) {
        UpdateFormComponentSize(info);
    }
    if (cardInfo_.obscuredMode != info.obscuredMode) {
        cardInfo_.obscuredMode = info.obscuredMode;
        if (formManagerBridge_) {
            formManagerBridge_->SetObscured(info.obscuredMode);
        }
    }
    if (isLoaded_) {
        auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
        layoutProperty->UpdateVisibility(visible);
        if (!isDynamic_ && !isSnapshot_ && needSnapshotAgain_) {
            auto renderContext = host->GetRenderContext();
            CHECK_NULL_VOID(renderContext);
            auto opacity = renderContext->GetOpacityValue(NON_TRANSPARENT_VAL);
            TAG_LOGI(AceLogTag::ACE_FORM, "Static-form, current opacity: %{public}f, visible: %{public}d",
                opacity, static_cast<int>(visible));
            if (visible == VisibleType::VISIBLE && opacity == NON_TRANSPARENT_VAL) {
                HandleSnapshot(DELAY_TIME_FOR_FORM_SNAPSHOT_3S);
            }
        }
    }
    UpdateConfiguration();
}

void FormPattern::UpdateFormComponentSize(const RequestFormInfo& info)
{
    TAG_LOGI(AceLogTag::ACE_FORM, "update size, width: %{public}f   height: %{public}f  borderWidth: %{public}f",
        info.width.Value(), info.height.Value(), info.borderWidth);
    cardInfo_.width = info.width;
    cardInfo_.height = info.height;
    cardInfo_.borderWidth = info.borderWidth;

    if (formManagerBridge_) {
        formManagerBridge_->NotifySurfaceChange(info.width.Value(), info.height.Value(), info.borderWidth);
    } else {
        TAG_LOGE(AceLogTag::ACE_FORM, "form manager delagate is nullptr, card id is %{public}" PRId64 ".",
            cardInfo_.id);
    }
    if (isSnapshot_) {
        auto imageNode = GetImageNode();
        if (imageNode != nullptr) {
            auto width = static_cast<float>(info.width.Value()) - info.borderWidth * DOUBLE;
            auto height = static_cast<float>(info.height.Value()) - info.borderWidth * DOUBLE;
            CalcSize idealSize = { CalcLength(width), CalcLength(height) };
            MeasureProperty layoutConstraint;
            layoutConstraint.selfIdealSize = idealSize;
            layoutConstraint.maxSize = idealSize;
            imageNode->UpdateLayoutConstraint(layoutConstraint);
        }
    }
    if (info.dimension == static_cast<int32_t>(OHOS::AppExecFwk::Constants::Dimension::DIMENSION_1_1)) {
        BorderRadiusProperty borderRadius;
        Dimension diameter = std::min(info.width, info.height);
        borderRadius.SetRadius(diameter / ARC_RADIUS_TO_DIAMETER);
        GetHost()->GetRenderContext()->UpdateBorderRadius(borderRadius);
    }
    if (subContainer_) {
        subContainer_->SetFormPattern(WeakClaim(this));
        subContainer_->UpdateRootElementSize();
        subContainer_->UpdateSurfaceSizeWithAnimathion();
    }
}

void FormPattern::RemoveDisableFormStyle()
{
    ContainerScope scope(scopeId_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    RefPtr<UINode> findText = FindUINodeByTag(V2::TEXT_ETS_TAG);
    RefPtr<UINode> findColumn = FindUINodeByTag(V2::COLUMN_ETS_TAG);
    if (findText && findColumn) {
        host->RemoveChild(findText);
        host->RemoveChild(findColumn);
    }
    // FormNode节点标脏;
    host->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void FormPattern::LoadDisableFormStyle()
{
    auto findColumnNode = FindUINodeByTag(V2::COLUMN_ETS_TAG);
    auto findTextNode = FindUINodeByTag(V2::TEXT_ETS_TAG);
    if (findColumnNode && findTextNode) {
        TAG_LOGI(AceLogTag::ACE_FORM, "findColumnNode && findTextNode ready exit");
        return;
    }
    TAG_LOGI(AceLogTag::ACE_FORM, "FormPattern::LoadDisableFormStyle");
    int32_t dimension = cardInfo_.dimension;
    int32_t dimensionHeight = GetFormDimensionHeight(dimension);
    if (dimensionHeight <= 0) {
        TAG_LOGE(AceLogTag::ACE_FORM, "LoadDisableFormStyle failed, invalid dimensionHeight!");
        return;
    }
    auto columnNode = CreateColumnNode();
    CHECK_NULL_VOID(columnNode);
    auto renderContext = columnNode->GetRenderContext();
    if (renderContext == nullptr) {
        TAG_LOGE(AceLogTag::ACE_FORM, "LoadDisableFormStyle(), renderContext is null");
        return;
    }
    BlurOption blurOption;
    blurOption.grayscale.assign(blurOption.grayscale.data(),
        blurOption.grayscale.data() + blurOption.grayscale.size());
    CalcDimension dimensionRadius(TIME_LIMIT_RADIUS_SIZE, DimensionUnit::PX);
    renderContext->UpdateFrontBlur(dimensionRadius, blurOption);
    if (renderContext->GetFrontBlurStyle().has_value()) {
        renderContext->UpdateFrontBlurStyle(std::nullopt);
    }
    renderContext->UpdateBackgroundColor(Color(CONTENT_BG_COLOR_DARK));
    renderContext->UpdateOpacity(TIME_LIMIT_TRANSPARENT_VAL);

    auto textNode = CreateTimeLimitNode();
    CHECK_NULL_VOID(textNode);
    textNode->MarkModifyDone();
    textNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    columnNode->MarkModifyDone();
    columnNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
    layoutProperty->UpdateVisibility(visible);
}

void FormPattern::LoadFormSkeleton()
{
    TAG_LOGI(AceLogTag::ACE_FORM, "LoadFormSkeleton");
    int32_t dimension = cardInfo_.dimension;
    int32_t dimensionHeight = GetFormDimensionHeight(dimension);
    if (dimensionHeight <= 0) {
        TAG_LOGE(AceLogTag::ACE_FORM, "LoadFormSkeleton failed, invalid dimensionHeight!");
        return;
    }

    RemoveFormSkeleton();

    auto columnNode = CreateColumnNode();
    CHECK_NULL_VOID(columnNode);

    double cardWidth = cardInfo_.width.Value();
    double cardHeight = cardInfo_.height.Value();
    auto colorMode = SystemProperties::GetColorMode();
    bool isDarkMode = colorMode == ColorMode::DARK;
    std::shared_ptr<FormSkeletonParams> params = std::make_shared<FormSkeletonParams>(cardWidth,
        cardHeight, dimension, dimensionHeight, isDarkMode);
    CreateSkeletonView(columnNode, params, dimensionHeight);

    auto renderContext = columnNode->GetRenderContext();
    if (renderContext != nullptr) {
        BlurStyleOption styleOption;
        styleOption.blurStyle = static_cast<BlurStyle>(static_cast<int>(BlurStyle::COMPONENT_ULTRA_THICK));
        renderContext->UpdateBackBlurStyle(styleOption);
        renderContext->UpdateBackgroundColor(isDarkMode ?
            Color(CONTENT_BG_COLOR_DARK) : Color(CONTENT_BG_COLOR_LIGHT));
        renderContext->UpdateOpacity(CONTENT_BG_OPACITY);
    }
    columnNode->MarkModifyDone();
    columnNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
    layoutProperty->UpdateVisibility(visible);
}

int32_t FormPattern::GetFormDimensionHeight(int32_t dimension)
{
    auto iter = OHOS::AppExecFwk::Constants::DIMENSION_MAP.
        find(static_cast<OHOS::AppExecFwk::Constants::Dimension>(dimension));
    if (iter == OHOS::AppExecFwk::Constants::DIMENSION_MAP.end()) {
        TAG_LOGE(AceLogTag::ACE_FORM, "GetFormDimensionHeight failed, invalid dimension: %{public}d",
            dimension);
        return 0;
    }
    std::string formDimensionStr = iter->second;
    std::stringstream streamDimension(formDimensionStr);
    std::string dimensionHeightStr;
    if (!std::getline(streamDimension, dimensionHeightStr, FORM_DIMENSION_SPLITTER)) {
        TAG_LOGE(AceLogTag::ACE_FORM, "GetFormDimensionHeight failed!");
        return 0;
    }
    return StringUtils::StringToInt(dimensionHeightStr);
}

RefPtr<FrameNode> FormPattern::CreateTimeLimitNode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);

    std::string content;
    GetTimeLimitResource(content);
    TAG_LOGI(AceLogTag::ACE_FORM, "GetTimeLimitContent, content = %{public}s", content.c_str());

    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
     auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);

    auto width = static_cast<float>(cardInfo_.width.Value()) - cardInfo_.borderWidth * DOUBLE;
    auto height = static_cast<float>(cardInfo_.height.Value()) - cardInfo_.borderWidth * DOUBLE;
    CalcSize idealSize = { CalcLength(width), CalcLength(height) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    textNode->UpdateLayoutConstraint(layoutConstraint);
    textLayoutProperty->UpdateContent(content);
    textLayoutProperty->UpdateFontWeight(FontWeight::BOLDER);
    textLayoutProperty->UpdateFontSize(TIME_LIMIT_FONT_SIZE_VAL);
    textLayoutProperty->UpdateTextColor(Color::WHITE);
    textLayoutProperty->UpdateTextAlign(TextAlign::CENTER);
    auto externalContext = DynamicCast<NG::RosenRenderContext>(textNode->GetRenderContext());
    CHECK_NULL_RETURN(externalContext, nullptr);
    externalContext->SetVisible(true);
    host->AddChild(textNode);
    return textNode;
}

void FormPattern::CreateSkeletonView(const RefPtr<FrameNode>& parent,
    const std::shared_ptr<FormSkeletonParams>& params, int32_t dimensionHeight)
{
    float lineHeight = params->GetLineHeight();
    uint32_t fillColor = params->GetFillColor();
    float lineMarginLeft = params->GetLineMarginLeft();

    // 1. Set title line
    MarginProperty titleMargin;
    titleMargin.top = CalcLength(params->GetTitleMarginTop());
    titleMargin.left = CalcLength(lineMarginLeft);
    CalcSize titleIdealSize = { CalcLength(params->GetTitleLineWidth()), CalcLength(lineHeight) };
    auto titleLineNode = CreateRectNode(parent, titleIdealSize, titleMargin,
        fillColor, params->GetTitleOpacity());
    CHECK_NULL_VOID(titleLineNode);

    // 2. Set content lines
    for (int32_t i = 0; i < params->GetContentLineNum(); i++) {
        MarginProperty contentMargin;
        contentMargin.top = CalcLength(i == 0 ? params->GetTitleContentMargins() :
            params->GetContentMargins());
        contentMargin.left = CalcLength(lineMarginLeft);
        CalcSize contentIdealSize = { CalcLength(params->GetLineWidth()), CalcLength(lineHeight) };
        auto contentLineNode = CreateRectNode(parent, contentIdealSize, contentMargin,
            fillColor, params->GetContentOpacity());
        CHECK_NULL_VOID(contentLineNode);
    }
    
    // 3. Set ending line if form dimension height greater than 1
    if (dimensionHeight > 1) {
        MarginProperty endingMargin;
        endingMargin.top = CalcLength(params->GetEndingLineMarginTop());
        endingMargin.left = CalcLength(lineMarginLeft);
        CalcSize endingIdealSize = { CalcLength(params->GetEndingLineWidth()), CalcLength(lineHeight) };
        auto endingLineNode = CreateRectNode(parent, endingIdealSize, endingMargin,
            fillColor, params->GetContentOpacity());
        CHECK_NULL_VOID(endingLineNode);
    }
}

void FormPattern::RemoveFormSkeleton()
{
    ContainerScope scope(scopeId_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto child = host->GetLastChild();
    CHECK_NULL_VOID(child);

    std::list<RefPtr<UINode>> children = host->GetChildren();
    if (children.size() > 0 && child->GetTag() == V2::COLUMN_ETS_TAG) {
        host->RemoveChildAtIndex(children.size() - 1);
    }
}

RefPtr<UINode> FormPattern::FindUINodeByTag(const std::string &tag)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
     std::list<RefPtr<UINode>> children = host->GetChildren();
     for (auto child : children) {
        if (child->GetTag() == tag) {
            TAG_LOGI(AceLogTag::ACE_FORM, "FindUINodeByTag Sucessful, child->GetTag() = %{public}s,",
                child->GetTag().c_str());
            return child;
        }
    }
    return nullptr;
}

RefPtr<FrameNode> FormPattern::CreateColumnNode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);

    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    CHECK_NULL_RETURN(columnNode, nullptr);

    auto width = static_cast<float>(cardInfo_.width.Value());
    auto height = static_cast<float>(cardInfo_.height.Value());
    CalcSize idealSize = { CalcLength(width), CalcLength(height) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    columnNode->UpdateLayoutConstraint(layoutConstraint);

    auto layoutProperty = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    layoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);

    host->AddChild(columnNode);
    return columnNode;
}

RefPtr<FrameNode> FormPattern::CreateRectNode(const RefPtr<FrameNode>& parent, const CalcSize& idealSize,
    const MarginProperty& margin, uint32_t fillColor, double opacity)
{
    auto rectNode = FrameNode::CreateFrameNode(V2::RECT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<RectPattern>());
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    rectNode->UpdateLayoutConstraint(layoutConstraint);

    rectNode->GetLayoutProperty()->UpdateMargin(margin);

    auto paintProperty = rectNode->GetPaintProperty<RectPaintProperty>();
    CHECK_NULL_RETURN(paintProperty, nullptr);
    paintProperty->UpdateFill(Color(fillColor));
    paintProperty->UpdateFillOpacity(opacity);

    paintProperty->UpdateTopLeftRadius(NG::Radius(RECT_RADIUS));
    paintProperty->UpdateTopRightRadius(NG::Radius(RECT_RADIUS));
    paintProperty->UpdateBottomLeftRadius(NG::Radius(RECT_RADIUS));
    paintProperty->UpdateBottomRightRadius(NG::Radius(RECT_RADIUS));

    rectNode->MountToParent(parent);
    rectNode->MarkDirtyNode();

    return rectNode;
}

void FormPattern::InitFormManagerDelegate()
{
    if (formManagerBridge_) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContextRefPtr();
    CHECK_NULL_VOID(context);
    formManagerBridge_ = AceType::MakeRefPtr<FormManagerDelegate>(context);
    formManagerBridge_->AddRenderDelegate();
    formManagerBridge_->RegisterRenderDelegateEvent();
    auto formUtils = FormManager::GetInstance().GetFormUtils();
    if (formUtils) {
        formManagerBridge_->SetFormUtils(formUtils);
    }
    int32_t instanceID = context->GetInstanceId();
    accessibilitySessionAdapter_ = AceType::MakeRefPtr<AccessibilitySessionAdapterForm>(formManagerBridge_);
    formManagerBridge_->AddFormAcquireCallback([weak = WeakClaim(this), instanceID](int64_t id, const std::string& path,
                                                   const std::string& module, const std::string& data,
                                                   const std::map<std::string, sptr<AppExecFwk::FormAshmem>>&
                                                       imageDataMap,
                                                   const AppExecFwk::FormJsInfo& formJsInfo,
                                                   const FrontendType& frontendType, const FrontendType& uiSyntax) {
        ContainerScope scope(instanceID);
        auto form = weak.Upgrade();
        CHECK_NULL_VOID(form);
        auto host = form->GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([id, path, module, data, imageDataMap, formJsInfo, weak, instanceID, frontendType,
                                    uiSyntax] {
            ContainerScope scope(instanceID);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            auto container = form->GetSubContainer();
            CHECK_NULL_VOID(container);
            container->SetWindowConfig({ formJsInfo.formWindow.designWidth, formJsInfo.formWindow.autoDesignWidth });
            container->RunCard(id, path, module, data, imageDataMap, formJsInfo.formSrc, frontendType, uiSyntax);
            }, "ArkUIFormRunCard");
    });

    formManagerBridge_->AddFormUpdateCallback(
        [weak = WeakClaim(this), instanceID](int64_t id, const std::string& data,
            const std::map<std::string, sptr<AppExecFwk::FormAshmem>>& imageDataMap) {
            ContainerScope scope(instanceID);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            auto host = form->GetHost();
            CHECK_NULL_VOID(host);
            auto uiTaskExecutor =
                SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
            uiTaskExecutor.PostTask([id, data, imageDataMap, weak, instanceID] {
                ContainerScope scope(instanceID);
                auto form = weak.Upgrade();
                CHECK_NULL_VOID(form);
                if (form->ISAllowUpdate()) {
                    form->GetSubContainer()->UpdateCard(data, imageDataMap);
                }
                }, "ArkUIFormUpdateCard");
        });

    formManagerBridge_->AddFormErrorCallback(
        [weak = WeakClaim(this), instanceID](const std::string& code, const std::string& msg) {
            ContainerScope scope(instanceID);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            auto host = form->GetHost();
            CHECK_NULL_VOID(host);
            auto uiTaskExecutor =
                SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
            uiTaskExecutor.PostTask([code, msg, weak, instanceID] {
                ContainerScope scope(instanceID);
                auto form = weak.Upgrade();
                CHECK_NULL_VOID(form);
                form->FireOnErrorEvent(code, msg);
                }, "ArkUIFormFireErrorEvent");
        });

    formManagerBridge_->AddFormUninstallCallback([weak = WeakClaim(this), instanceID](int64_t formId) {
        ContainerScope scope(instanceID);
        auto form = weak.Upgrade();
        CHECK_NULL_VOID(form);
        auto host = form->GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([formId, weak, instanceID] {
            ContainerScope scope(instanceID);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            form->FireOnUninstallEvent(formId);
            }, "ArkUIFormFireUninstallEvent");
    });

    formManagerBridge_->AddFormSurfaceNodeCallback(
        [weak = WeakClaim(this), instanceID](
            const std::shared_ptr<Rosen::RSSurfaceNode>& node, const AAFwk::Want& want) {
            ContainerScope scope(instanceID);
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto executor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID(executor);
            auto uiTaskExecutor =
                SingleTaskExecutor::Make(executor, TaskExecutor::TaskType::UI);
            uiTaskExecutor.PostTask([weak, instanceID, node, want] {
                ContainerScope scope(instanceID);
                auto form = weak.Upgrade();
                CHECK_NULL_VOID(form);
                form->FireFormSurfaceNodeCallback(node, want);
                }, "ArkUIFormFireSurfaceNodeCallback");
        });

    formManagerBridge_->AddFormSurfaceChangeCallback([weak = WeakClaim(this), instanceID](float width, float height,
        float borderWidth) {
        ContainerScope scope(instanceID);
        auto form = weak.Upgrade();
        CHECK_NULL_VOID(form);
        auto host = form->GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([weak, instanceID, width, height, borderWidth] {
            ContainerScope scope(instanceID);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            form->FireFormSurfaceChangeCallback(width, height, borderWidth);
            }, "ArkUIFormFireSurfaceChange");
    });

    formManagerBridge_->AddFormSurfaceDetachCallback([weak = WeakClaim(this), instanceID]() {
            ContainerScope scope(instanceID);
            auto formPattern = weak.Upgrade();
            CHECK_NULL_VOID(formPattern);
            formPattern->FireFormSurfaceDetachCallback();
        });

    formManagerBridge_->AddActionEventHandle([weak = WeakClaim(this), instanceID](const std::string& action) {
        ContainerScope scope(instanceID);
        TAG_LOGI(AceLogTag::ACE_FORM, "Card receive action event, action: %{public}zu", action.length());
        auto formPattern = weak.Upgrade();
        CHECK_NULL_VOID(formPattern);
        formPattern->OnActionEvent(action);
    });

    formManagerBridge_->AddUnTrustFormCallback([weak = WeakClaim(this), instanceID]() {
        ContainerScope scope(instanceID);
        auto formPattern = weak.Upgrade();
        CHECK_NULL_VOID(formPattern);
        auto host = formPattern->GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([weak, instanceID] {
            ContainerScope scope(instanceID);
            auto formPattern = weak.Upgrade();
            CHECK_NULL_VOID(formPattern);
            formPattern->HandleUnTrustForm();
            }, "ArkUIFormHandleUnTrust");
    });

    formManagerBridge_->AddSnapshotCallback([weak = WeakClaim(this), instanceID](const uint32_t& delayTime) {
        ContainerScope scope(instanceID);
        auto formPattern = weak.Upgrade();
        CHECK_NULL_VOID(formPattern);
        formPattern->HandleSnapshot(delayTime);
    });

    formManagerBridge_->AddFormLinkInfoUpdateCallback(
        [weak = WeakClaim(this), instanceID](const std::vector<std::string>& infos) {
            ContainerScope scope(instanceID);
            auto formPattern = weak.Upgrade();
            CHECK_NULL_VOID(formPattern);
            formPattern->SetFormLinkInfos(infos);
        });

    formManagerBridge_->AddGetRectRelativeToWindowCallback(
        [weak = WeakClaim(this), instanceID](int32_t &top, int32_t &left) {
            ContainerScope scope(instanceID);
            auto formPattern = weak.Upgrade();
            CHECK_NULL_VOID(formPattern);
            formPattern->GetRectRelativeToWindow(top, left);
        });

    formManagerBridge_->AddEnableFormCallback(
        [weak = WeakClaim(this), instanceID](const bool enable) {
            ContainerScope scope(instanceID);
            auto formPattern = weak.Upgrade();
            CHECK_NULL_VOID(formPattern);
            formPattern->HandleEnableForm(enable);
        });
}

void FormPattern::GetRectRelativeToWindow(int32_t &top, int32_t &left)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto rect = host->GetTransformRectRelativeToWindow();
    top = rect.Top();
    left = rect.Left();
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", top: %{public}d, left: %{public}d",
        host->GetAccessibilityId(), top, left);
}

void FormPattern::ProcDeleteImageNode(const AAFwk::Want& want)
{
    if (want.GetBoolParam(OHOS::AppExecFwk::Constants::FORM_IS_RECOVER_FORM, false)) {
        DelayDeleteImageNode(want.GetBoolParam(
            OHOS::AppExecFwk::Constants::FORM_IS_RECOVER_FORM_TO_HANDLE_CLICK_EVENT, false));
    } else {
        DeleteImageNode();
    }
}

void FormPattern::AttachRSNode(const std::shared_ptr<Rosen::RSSurfaceNode>& node, const AAFwk::Want& want)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto externalRenderContext = DynamicCast<NG::RosenRenderContext>(GetExternalRenderContext());
    CHECK_NULL_VOID(externalRenderContext);
    externalRenderContext->SetRSNode(node);
    float boundWidth = cardInfo_.width.Value() - cardInfo_.borderWidth * DOUBLE;
    float boundHeight = cardInfo_.height.Value() - cardInfo_.borderWidth * DOUBLE;
    if (isBeenLayout_) {
        auto geometryNode = host->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        auto size = geometryNode->GetFrameSize();
        boundWidth = size.Width() - cardInfo_.borderWidth * DOUBLE;
        boundHeight = size.Height() - cardInfo_.borderWidth * DOUBLE;
    }
    externalRenderContext->SetBounds(cardInfo_.borderWidth, cardInfo_.borderWidth, boundWidth, boundHeight);

    bool isRecover = want.GetBoolParam(OHOS::AppExecFwk::Constants::FORM_IS_RECOVER_FORM, false);
    bool isDynamic = want.GetBoolParam(OHOS::AppExecFwk::Constants::FORM_IS_DYNAMIC, false);
    if (isRecover) {
        TAG_LOGI(AceLogTag::ACE_FORM, "surfaceNode: %{public}s setOpacity:0", std::to_string(node->GetId()).c_str());
        externalRenderContext->SetOpacity(TRANSPARENT_VAL);
    }

    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (isDynamic && !isRecover) {
        renderContext->ClearChildren();
    }
    renderContext->AddChild(externalRenderContext, 0);
}

void FormPattern::FireFormSurfaceNodeCallback(
    const std::shared_ptr<Rosen::RSSurfaceNode>& node, const AAFwk::Want& want)
{
    ACE_FUNCTION_TRACE();
    CHECK_NULL_VOID(node);
    node->CreateNodeInRenderThread();

    RemoveFormSkeleton();
    AttachRSNode(node, want);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
    TAG_LOGI(AceLogTag::ACE_FORM, "VisibleType: %{public}d, surfaceNode: %{public}s",
        static_cast<int32_t>(visible), std::to_string(node->GetId()).c_str());
    layoutProperty->UpdateVisibility(visible);

    isLoaded_ = true;
    isUnTrust_ = false;
    isFrsNodeDetached_ = false;
    isDynamic_ = want.GetBoolParam(OHOS::AppExecFwk::Constants::FORM_IS_DYNAMIC, false);

    ProcDeleteImageNode(want);

    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    auto parent = host->GetParent();
    CHECK_NULL_VOID(parent);
    parent->MarkNeedSyncRenderTree();
    parent->RebuildRenderContextTree();
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->RequestNextFrame();
    OnLoadEvent();

    auto formNode = DynamicCast<FormNode>(host);
    CHECK_NULL_VOID(formNode);
    formNode->NotifyAccessibilityChildTreeRegister();
}

void FormPattern::DelayDeleteImageNode(bool needHandleCachedClick)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);

    auto uiTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    uiTaskExecutor.PostDelayedTask(
        [weak = WeakClaim(this), needHandleCachedClick] {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->DeleteImageNodeAfterRecover(needHandleCachedClick);
        },
        DELAY_TIME_FOR_DELETE_IMAGE_NODE, "ArkUIFormDeleteImageNodeAfterRecover");
}

void FormPattern::FireFormSurfaceChangeCallback(float width, float height, float borderWidth)
{
    auto externalRenderContext = DynamicCast<NG::RosenRenderContext>(GetExternalRenderContext());
    CHECK_NULL_VOID(externalRenderContext);
    externalRenderContext->SetBounds(borderWidth, borderWidth, width - borderWidth * DOUBLE,
        height - borderWidth * DOUBLE);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    isUnTrust_ = false;
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    auto parent = host->GetParent();
    CHECK_NULL_VOID(parent);
    parent->MarkNeedSyncRenderTree();
    parent->RebuildRenderContextTree();
    renderContext->RequestNextFrame();
}

void FormPattern::FireFormSurfaceDetachCallback()
{
    TAG_LOGI(AceLogTag::ACE_FORM, "FireFormSurfaceDetachCallback isFrsNodeDetached:%{public}d", isFrsNodeDetached_);
    isFrsNodeDetached_ = true;
}

void FormPattern::CreateCardContainer()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContextRefPtr();
    CHECK_NULL_VOID(context);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto hasContainer = false;
    RemoveSubContainer();
    if (cardInfo_.id != 0 && Container::IsCurrentUseNewPipeline()) {
        auto subContainer = FormManager::GetInstance().GetSubContainer(cardInfo_.id);
        if (subContainer && context->GetInstanceId() == subContainer->GetInstanceId() &&
            subContainer->GetCardType() == FrontendType::JS_CARD) {
            subContainer_ = subContainer;
            FormManager::GetInstance().RemoveSubContainer(cardInfo_.id);
            hasContainer = true;
        }
    }
    if (!subContainer_) {
        subContainer_ = AceType::MakeRefPtr<SubContainer>(context, context->GetInstanceId());
    }
    CHECK_NULL_VOID(subContainer_);
    subContainer_->SetFormPattern(WeakClaim(this));
    subContainer_->Initialize();
    subContainer_->SetNodeId(host->GetId());

    subContainer_->AddFormAcquireCallback([weak = WeakClaim(this)](int64_t id) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([id, weak] {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->FireOnAcquiredEvent(id);
            }, "ArkUIFormFireAcquiredEvent");
    });

    subContainer_->SetFormLoadCallback([weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnLoadEvent();
    });

    subContainer_->AddFormVisiableCallback([weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
        layoutProperty->UpdateVisibility(visible);
        pattern->isLoaded_ = true;
    });

    if (hasContainer) {
        subContainer_->RunSameCard();
    }
}

std::unique_ptr<DrawDelegate> FormPattern::GetDrawDelegate()
{
    auto drawDelegate = std::make_unique<DrawDelegate>();
#ifdef ENABLE_ROSEN_BACKEND
    drawDelegate->SetDrawRSFrameCallback(
        [weak = WeakClaim(this)](std::shared_ptr<RSNode>& node, const Rect& /* dirty */) {
            CHECK_NULL_VOID(node);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            auto host = form->GetHost();
            CHECK_NULL_VOID(host);
            auto context = DynamicCast<NG::RosenRenderContext>(host->GetRenderContext());
            CHECK_NULL_VOID(context);
            auto rsNode = context->GetRSNode();
            CHECK_NULL_VOID(rsNode);
            rsNode->AddChild(node, -1);
            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        });

    drawDelegate->SetDrawRSFrameByRenderContextCallback(
        [weak = WeakClaim(this)](RefPtr<OHOS::Ace::NG::RenderContext>& renderContext) {
            auto context = DynamicCast<NG::RosenRenderContext>(renderContext);
            CHECK_NULL_VOID(context);
            auto node = context->GetRSNode();
            CHECK_NULL_VOID(node);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            auto host = form->GetHost();
            CHECK_NULL_VOID(host);
            auto formContext = DynamicCast<NG::RosenRenderContext>(host->GetRenderContext());
            CHECK_NULL_VOID(formContext);
            auto rsNode = formContext->GetRSNode();
            CHECK_NULL_VOID(rsNode);
            rsNode->AddChild(node, -1);
            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        });
#endif
    return drawDelegate;
}

void FormPattern::FireOnErrorEvent(const std::string& code, const std::string& msg) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto json = JsonUtil::Create(true);
    json->Put("errcode", code.c_str());
    json->Put("msg", msg.c_str());
    eventHub->FireOnError(json->ToString());
}

void FormPattern::FireOnUninstallEvent(int64_t id) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    int64_t uninstallFormId = id < MAX_NUMBER_OF_JS ? id : -1;
    auto json = JsonUtil::Create(true);
    json->Put("id", std::to_string(uninstallFormId).c_str());
    json->Put("idString", std::to_string(id).c_str());
    eventHub->FireOnUninstall(json->ToString());
}

void FormPattern::FireOnAcquiredEvent(int64_t id) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    int64_t onAcquireFormId = id < MAX_NUMBER_OF_JS ? id : -1;
    auto json = JsonUtil::Create(true);
    json->Put("id", std::to_string(onAcquireFormId).c_str());
    json->Put("idString", std::to_string(id).c_str());
    eventHub->FireOnAcquired(json->ToString());
}

void FormPattern::FireOnRouterEvent(const std::unique_ptr<JsonValue>& action)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto json = JsonUtil::Create(true);
    json->Put("action", action);
    eventHub->FireOnRouter(json->ToString());
}

void FormPattern::FireOnLoadEvent() const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireOnLoad("");
}

void FormPattern::OnLoadEvent()
{
    ACE_FUNCTION_TRACE();
    isSnapshot_ = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto uiTaskExecutor = SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    uiTaskExecutor.PostTask([weak = WeakClaim(this)] {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->FireOnLoadEvent();
        }, "ArkUIFormFireLoadEvent");
}

void FormPattern::OnActionEvent(const std::string& action)
{
    CHECK_NULL_VOID(formManagerBridge_);
    auto eventAction = JsonUtil::ParseJsonString(action);
    if (!eventAction->IsValid()) {
        return;
    }

    auto actionType = eventAction->GetValue("action");
    if (!actionType->IsValid()) {
        return;
    }

    auto type = actionType->GetString();
    if (type != "router" && type != "message" && type != "call") {
        return;
    }

    if ("router" == type) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        if (uiTaskExecutor.IsRunOnCurrentThread()) {
            FireOnRouterEvent(eventAction);
        } else {
            uiTaskExecutor.PostTask([weak = WeakClaim(this), action] {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                auto eventAction = JsonUtil::ParseJsonString(action);
                TAG_LOGI(AceLogTag::ACE_FORM, "UI task execute begin.");
                pattern->FireOnRouterEvent(eventAction);
                }, "ArkUIFormFireRouterEvent");
        }
    }

    formManagerBridge_->OnActionEvent(action);
}

bool FormPattern::ISAllowUpdate() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, true);
    auto property = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_RETURN(property, true);
    auto formInfo = property->GetRequestFormInfo();
    CHECK_NULL_RETURN(property, true);
    return formInfo->allowUpdate;
}

const RefPtr<SubContainer>& FormPattern::GetSubContainer() const
{
    return subContainer_;
}

void FormPattern::DispatchPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    SerializedGesture& serializedGesture)
{
    CHECK_NULL_VOID(pointerEvent);
    CHECK_NULL_VOID(formManagerBridge_);

    if (!isVisible_) {
        auto pointerAction = pointerEvent->GetPointerAction();
        if (pointerAction == OHOS::MMI::PointerEvent::POINTER_ACTION_UP ||
            pointerAction == OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_UP ||
            pointerAction == OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW ||
            pointerAction == OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL) {
            // still dispatch 'up' or 'cancel' event to finish this pointer event
            formManagerBridge_->DispatchPointerEvent(pointerEvent, serializedGesture);
        } else {
            TAG_LOGD(AceLogTag::ACE_FORM, "form invisible, not dispatch pointerEvent: %{public}d.", pointerAction);
        }
        return;
    }
    formManagerBridge_->DispatchPointerEvent(pointerEvent, serializedGesture);
}

void FormPattern::RemoveSubContainer()
{
    auto host = GetHost();
    auto eventHub = host->GetEventHub<FormEventHub>();
    if (eventHub) {
        eventHub->FireOnCache();
    }
    subContainer_.Reset();
}

void FormPattern::EnableDrag()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto dragStart = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& event,
                         const std::string& /* extraParams */) -> DragDropInfo {
        DragDropInfo info;

        auto form = weak.Upgrade();
        CHECK_NULL_RETURN(form, info);
        auto subcontainer = form->GetSubContainer();
        CHECK_NULL_RETURN(subcontainer, info);

        RefPtr<UnifiedData> unifiedData = UdmfClient::GetInstance()->CreateUnifiedData();
        UdmfClient::GetInstance()->AddFormRecord(unifiedData, subcontainer->GetRunningCardId(), form->cardInfo_);
        event->SetData(unifiedData);

        info.extraInfo = "card drag";
        return info;
    };
    auto eventHub = GetHost()->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetDefaultOnDragStart(std::move(dragStart));
}

void FormPattern::UpdateConfiguration()
{
    auto localeTag = AceApplicationInfo::GetInstance().GetLocaleTag();
    if (localeTag != localeTag_ && subContainer_) {
        localeTag_ = localeTag;
        subContainer_->UpdateConfiguration();
    }
}

void FormPattern::OnLanguageConfigurationUpdate()
{
    std::string content;
    GetTimeLimitResource(content);
    TAG_LOGI(AceLogTag::ACE_FORM, "FormPattern::OnLanguageConfigurationUpdate, %{public}s", content.c_str());
    UpdateTimeLimitResource(content);
}

void FormPattern::GetTimeLimitResource(std::string &content)
{
    std::shared_ptr<Global::Resource::ResourceManager> sysResMgr(Global::Resource::CreateResourceManager());
    if (sysResMgr == nullptr) {
        TAG_LOGE(AceLogTag::ACE_FORM, "init sysMgr failed!");
        return;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AceLogTag::ACE_FORM, "init resConfig failed!");
        return;
    }

    sysResMgr->GetResConfig(*resConfig);
    UErrorCode status = U_ZERO_ERROR;
    std::string language = Global::I18n::LocaleConfig::GetSystemLanguage();
    icu::Locale locale = icu::Locale::forLanguageTag(language, status);
    if (status != U_ZERO_ERROR) {
        TAG_LOGE(AceLogTag::ACE_FORM, "forLanguageTag failed, errCode:%{public}d", status);
        return;
    }

    resConfig->SetLocaleInfo(locale.getLanguage(), locale.getScript(), locale.getCountry());
    Global::Resource::RState state = sysResMgr->UpdateResConfig(*resConfig);
    if (state != Global::Resource::RState::SUCCESS) {
        TAG_LOGE(AceLogTag::ACE_FORM, "UpdateResConfig failed! errcode:%{public}d.", state);
        return;
    }
    sysResMgr->GetStringByName(TIME_LIMIT_RESOURCE_NAME, content);
}

void FormPattern::UpdateTimeLimitResource(std::string &content)
{
    TAG_LOGI(AceLogTag::ACE_FORM, "UpdateTimeLimitResource");
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto findTextNode = FindUINodeByTag(V2::TEXT_ETS_TAG);
    if (findTextNode == nullptr) {
        TAG_LOGE(AceLogTag::ACE_FORM, "Not find TimeLimitTextNode");
        return;
    }
    auto textNode = AceType::DynamicCast<FrameNode>(findTextNode);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    textLayoutProperty->UpdateContent(content);
}
} // namespace OHOS::Ace::NG
