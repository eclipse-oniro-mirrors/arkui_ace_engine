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

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/common/form_manager.h"
#include "core/components/form/resource/form_manager_delegate.h"
#include "core/components/form/sub_container.h"
#include "core/components_ng/pattern/form/form_event_hub.h"
#include "core/components_ng/pattern/form/form_layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "pointer_event.h"

#ifdef ENABLE_DRAG_FRAMEWORK
#include "foundation/distributeddatamgr/udmf/interfaces/innerkits/common/unified_types.h"
#include "foundation/distributeddatamgr/udmf/interfaces/innerkits/data/system_defined_form.h"
#include "foundation/distributeddatamgr/udmf/interfaces/innerkits/data/unified_data.h"
#endif // ENABLE_DRAG_FRAMEWORK

namespace OHOS::Ace::NG {
namespace {
void ShowPointEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!pointerEvent) {
        LOGE("Func: %{public}s, pointerEvent is null.", __func__);
        return;
    }
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("Func: %{public}s, get pointer item failed.", __func__);
        return;
    }
}

constexpr uint32_t DELAY_TIME_FOR_FORM_SUBCONTAINER_CACHE = 30000;
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
    // Init the render context for RSSurfaceNode from FRS.
    externalRenderContext_ = RenderContext::Create();
    externalRenderContext_->InitContext(false, "Form_" + std::to_string(host->GetId()) + "_Remote_Surface", true);
    InitFormManagerDelegate();
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnCache([weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        auto subContainer = pattern->GetSubContainer();
        CHECK_NULL_VOID(subContainer);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        auto id = subContainer->GetRunningCardId();
        FormManager::GetInstance().AddSubContainer(id, subContainer);
        uiTaskExecutor.PostDelayedTask(
            [id, nodeId = subContainer->GetNodeId()] {
                auto cachedubContainer = FormManager::GetInstance().GetSubContainer(id);
                if (cachedubContainer != nullptr && cachedubContainer->GetNodeId() == nodeId) {
                    FormManager::GetInstance().RemoveSubContainer(id);
                }
            },
            DELAY_TIME_FOR_FORM_SUBCONTAINER_CACHE);
    });
}

void FormPattern::OnRebuildFrame()
{
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

bool FormPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }

    auto size = dirty->GetGeometryNode()->GetFrameSize();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, false);
    auto info = layoutProperty->GetRequestFormInfo().value_or(RequestFormInfo());
    info.width = Dimension(size.Width());
    info.height = Dimension(size.Height());
    layoutProperty->UpdateRequestFormInfo(info);
    if (formManagerBridge_ && cardInfo_.allowUpdate != info.allowUpdate) {
        formManagerBridge_->SetAllowUpdate(info.allowUpdate);
    }

    if (info.bundleName != cardInfo_.bundleName || info.abilityName != cardInfo_.abilityName ||
        info.moduleName != cardInfo_.moduleName || info.cardName != cardInfo_.cardName ||
        info.dimension != cardInfo_.dimension) {
        // When cardInfo has changed, it will call AddForm in Fwk
        // If the width or height equal to zero, it will not
        if (NonPositive(size.Width()) || NonPositive(size.Height())) {
            return false;
        }
        cardInfo_ = info;
    } else {
        // for update form component
        if (cardInfo_.allowUpdate != info.allowUpdate) {
            cardInfo_.allowUpdate = info.allowUpdate;
            LOGI(" update card allow info:%{public}d", cardInfo_.allowUpdate);
            if (subContainer_) {
                subContainer_->SetAllowUpdate(cardInfo_.allowUpdate);
            }
        }

        if (formManagerBridge_ && (cardInfo_.width != info.width || cardInfo_.height != info.height)) {
            LOGI("Form surfaceChange callback");
            formManagerBridge_->NotifySurfaceChange(size.Width(), size.Height());
        }

        if (cardInfo_.width != info.width || cardInfo_.height != info.height) {
            cardInfo_.width = info.width;
            cardInfo_.height = info.height;
            if (subContainer_) {
                subContainer_->SetFormPattern(WeakClaim(this));
                subContainer_->UpdateRootElementSize();
                subContainer_->UpdateSurfaceSizeWithAnimathion();
            }
        }
        if (isLoaded_) {
            auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
            layoutProperty->UpdateVisibility(visible);
        }
        return false;
    }
    CreateCardContainer();
    if (host->IsDraggable()) {
        EnableDrag();
    }
    if (formManagerBridge_) {
        formManagerBridge_->AddForm(host->GetContext(), info);
    }
    return false;
}

void FormPattern::InitFormManagerDelegate()
{
    if (formManagerBridge_) {
        LOGD("Form manager bridge is already initialized.");
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    formManagerBridge_ = AceType::MakeRefPtr<FormManagerDelegate>(context);
    formManagerBridge_->AddRenderDelegate();
    formManagerBridge_->RegisterRenderDelegateEvent();
    auto formUtils = FormManager::GetInstance().GetFormUtils();
    if (formUtils) {
        formManagerBridge_->SetFormUtils(formUtils);
    }
    int32_t instanceID = context->GetInstanceId();
    formManagerBridge_->AddFormAcquireCallback([weak = WeakClaim(this), instanceID](int64_t id, const std::string& path,
                                                   const std::string& module, const std::string& data,
                                                   const std::map<std::string, sptr<AppExecFwk::FormAshmem>>&
                                                       imageDataMap,
                                                   const AppExecFwk::FormJsInfo& formJsInfo,
                                                   const FrontendType& frontendType,
                                                   const FrontendType& uiSyntax) {
        ContainerScope scope(instanceID);
        auto form = weak.Upgrade();
        CHECK_NULL_VOID(form);
        auto host = form->GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([id, path, module, data, imageDataMap, formJsInfo, weak,
                                 instanceID, frontendType, uiSyntax] {
            ContainerScope scope(instanceID);
            auto form = weak.Upgrade();
            CHECK_NULL_VOID(form);
            auto container = form->GetSubContainer();
            CHECK_NULL_VOID(container);
            container->SetWindowConfig({ formJsInfo.formWindow.designWidth, formJsInfo.formWindow.autoDesignWidth });
            container->RunCard(id, path, module, data, imageDataMap, formJsInfo.formSrc, frontendType, uiSyntax);
        });
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
            });
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
            });
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
        });
    });

    formManagerBridge_->AddFormSurfaceNodeCallback(
        [weak = WeakClaim(this), instanceID](const std::shared_ptr<Rosen::RSSurfaceNode>& node) {
            LOGI("Form surface node callback");
            ContainerScope scope(instanceID);
            CHECK_NULL_VOID(node);
            node->CreateNodeInRenderThread();

            auto formComponent = weak.Upgrade();
            CHECK_NULL_VOID(formComponent);
            auto host = formComponent->GetHost();
            CHECK_NULL_VOID(host);
            auto size = host->GetGeometryNode()->GetFrameSize();

            auto externalRenderContext = DynamicCast<NG::RosenRenderContext>(formComponent->GetExternalRenderContext());
            CHECK_NULL_VOID(externalRenderContext);
            externalRenderContext->SetRSNode(node);
            externalRenderContext->SetBounds(0, 0, size.Width(), size.Height());

            auto formComponentContext = DynamicCast<NG::RosenRenderContext>(host->GetRenderContext());
            CHECK_NULL_VOID(formComponentContext);
            formComponentContext->AddChild(externalRenderContext, 0);

            auto layoutProperty = host->GetLayoutProperty<FormLayoutProperty>();
            CHECK_NULL_VOID(layoutProperty);
            auto visible = layoutProperty->GetVisibleType().value_or(VisibleType::VISIBLE);
            layoutProperty->UpdateVisibility(visible);
            formComponent->isLoaded_ = true;

            host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
            auto parent = host->GetParent();
            CHECK_NULL_VOID(parent);
            parent->MarkNeedSyncRenderTree();
            parent->RebuildRenderContextTree();
            host->GetRenderContext()->RequestNextFrame();
            formComponent->OnLoadEvent();
    });

    formManagerBridge_->AddFormSurfaceChangeCallback([weak = WeakClaim(this), instanceID](float width, float height) {
        auto formComponent = weak.Upgrade();
        CHECK_NULL_VOID(formComponent);
        auto externalRenderContext = DynamicCast<NG::RosenRenderContext>(formComponent->GetExternalRenderContext());
        CHECK_NULL_VOID(externalRenderContext);
        externalRenderContext->SetBounds(0, 0, width, height);
        auto host = formComponent->GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        auto parent = host->GetParent();
        CHECK_NULL_VOID(parent);
        parent->MarkNeedSyncRenderTree();
        parent->RebuildRenderContextTree();
        host->GetRenderContext()->RequestNextFrame();
    });

    formManagerBridge_->AddActionEventHandle(
        [weak = WeakClaim(this), instanceID](const std::string& action) {
            ContainerScope scope(instanceID);
            LOGI("OnActionEvent action: %{public}s", action.c_str());
            auto formPattern = weak.Upgrade();
            CHECK_NULL_VOID(formPattern);
            formPattern->OnActionEvent(action);
    });
}

void FormPattern::CreateCardContainer()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
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
    subContainer_->Initialize();
    subContainer_->SetFormPattern(WeakClaim(this));
    subContainer_->SetNodeId(host->GetId());

    subContainer_->AddFormAcquireCallback([weak = WeakClaim(this)](size_t id) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        auto uiTaskExecutor =
            SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([id, weak] {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID_NOLOG(pattern);
            LOGI("card id:%{public}zu", id);
            pattern->FireOnAcquiredEvent(id);
        });
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
    LOGI("FireOnErrorEvent code: %{public}s, msg: %{public}s", code.c_str(), msg.c_str());
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
    LOGI("FireOnUninstallEvent id: %{public}s", std::to_string(id).c_str());
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto json = JsonUtil::Create(true);
    json->Put("id", std::to_string(id).c_str());
    eventHub->FireOnUninstall(json->ToString());
}

void FormPattern::FireOnAcquiredEvent(int64_t id) const
{
    LOGI("FireOnAcquiredEvent id: %{public}s", std::to_string(id).c_str());
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto json = JsonUtil::Create(true);
    json->Put("id", std::to_string(id).c_str());
    eventHub->FireOnAcquired(json->ToString());
}

void FormPattern::FireOnRouterEvent(const std::unique_ptr<JsonValue>& action) const
{
    LOGI("FireOnAcquiredEvent action: %{public}s", action->ToString().c_str());
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
    LOGI("FireOnLoadEvent");
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<FormEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireOnLoad("");
}

void FormPattern::OnLoadEvent()
{
    LOGI("OnLoadEvent");
    ACE_FUNCTION_TRACE();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto uiTaskExecutor = SingleTaskExecutor::Make(host->GetContext()->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    uiTaskExecutor.PostTask([weak = WeakClaim(this)] {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->FireOnLoadEvent();
    });
}

void FormPattern::OnActionEvent(const std::string& action) const
{
    auto eventAction = JsonUtil::ParseJsonString(action);
    if (!eventAction->IsValid()) {
        LOGE("get event action failed");
        return;
    }
    auto actionType = eventAction->GetValue("action");
    if (!actionType->IsValid()) {
        LOGE("get event key failed");
        return;
    }

    auto type = actionType->GetString();
    if (type != "router" && type != "message" && type != "call") {
        LOGE("undefined event type");
        return;
    }

    if ("router" == type) {
        FireOnRouterEvent(eventAction);
    }

    CHECK_NULL_VOID_NOLOG(formManagerBridge_);
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

void FormPattern::DispatchPointerEvent(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    if (!pointerEvent || !formManagerBridge_) {
        LOGE("Func: %{public}s, pointerEvent or formManagerBridge is null", __func__);
        return;
    }

    if (!isVisible_) {
        LOGW("The form is invisible, stop to dispatch pointEvent");
        auto pointerAction = pointerEvent->GetPointerAction();
        if (pointerAction == OHOS::MMI::PointerEvent::POINTER_ACTION_UP ||
            pointerAction == OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_UP ||
            pointerAction == OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW) {
            // still dispatch 'up' event to finish this pointer event
            formManagerBridge_->DispatchPointerEvent(pointerEvent);
        }
        return;
    }

    ShowPointEvent(pointerEvent);
    formManagerBridge_->DispatchPointerEvent(pointerEvent);
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

#ifdef ENABLE_DRAG_FRAMEWORK
        auto form = weak.Upgrade();
        CHECK_NULL_RETURN(form, info);
        auto subcontainer = form->GetSubContainer();
        CHECK_NULL_RETURN(subcontainer, info);

        auto formRecord = std::make_shared<UDMF::SystemDefinedForm>();
        formRecord->SetFormId(subcontainer->GetRunningCardId());
        formRecord->SetFormName(form->cardInfo_.cardName);
        formRecord->SetBundleName(form->cardInfo_.bundleName);
        formRecord->SetAbilityName(form->cardInfo_.abilityName);
        formRecord->SetModule(form->cardInfo_.moduleName);
        formRecord->SetType(UDMF::UDType::SYSTEM_DEFINED_FORM);

        auto unifiedData = std::make_shared<UDMF::UnifiedData>();
        unifiedData->AddRecord(formRecord);
        event->SetData(unifiedData);
#endif // ENABLE_DRAG_FRAMEWORK

        info.extraInfo = "card drag";
        return info;
    };
    auto eventHub = GetHost()->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDragStart(std::move(dragStart));
}
} // namespace OHOS::Ace::NG
