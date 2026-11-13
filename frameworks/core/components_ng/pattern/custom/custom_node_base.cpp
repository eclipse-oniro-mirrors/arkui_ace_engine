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

#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/pipeline_ng/async_load_scheduler.h"
#include "base/json/json_util.h"
#include "base/utils/string_utils.h"

#include <cmath>

#include "base/utils/time_util.h"
#include "core/components_ng/base/async_load_config.h"
#include "core/components_ng/pattern/recycle_view/recycle_manager.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/syntax/lazy_for_each_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

CustomNodeBase::~CustomNodeBase()
{
    RecycleManager::Erase(recycleInfo_.elemtId);
    // appearFunc_ & destroyFunc_ should be executed in pairs
    if (!executeFireOnAppear_ && appearFunc_) {
        appearFunc_();
        if (didBuildFunc_) {
            didBuildFunc_();
        }
    }
    if (destroyFunc_) {
        ACE_SCOPED_TRACE("CustomNodeBase:Destroy [%s]", GetJSViewName().c_str());
        FireTriggerLifecycleFunc(LifeCycleEvent::ON_DISAPPEAR);
        destroyFunc_();
    }
}

bool CustomNodeBase::FireTriggerLifecycleFunc(int32_t eventId)
{
    ACE_SCOPED_TRACE("FireTriggerLifecycleFunc, eventId = [%d], name = [%s]", eventId, GetJSViewName().c_str());
    return triggerLifecycleFunc_ && triggerLifecycleFunc_(eventId);
}

void CustomNodeBase::FireOnAppear()
{
    if (appearFunc_) {
        appearFunc_();
    }
    executeFireOnAppear_ = true;
}

void CustomNodeBase::FireOnDisappear()
{
    if (destroyFunc_) {
        FireTriggerLifecycleFunc(LifeCycleEvent::ON_DISAPPEAR);
        destroyFunc_();
    }
}

void CustomNodeBase::FireDidBuild()
{
    if (didBuildFunc_) {
        didBuildFunc_();
    }
}

void CustomNodeBase::SetTriggerLifecycleFunction(std::function<bool(int32_t)>&& triggerLifecycleFunc)
{
    triggerLifecycleFunc_ = std::move(triggerLifecycleFunc);
}

void CustomNodeBase::SetAppearFunction(std::function<void()>&& appearFunc)
{
    appearFunc_ = std::move(appearFunc);
}

void CustomNodeBase::SetDidBuildFunction(std::function<void()>&& didBuildFunc)
{
    didBuildFunc_ = std::move(didBuildFunc);
}

void CustomNodeBase::SetAsyncLoadConfig(std::shared_ptr<AsyncLoadConfig>&& config)
{
    if (AsyncLoadScheduler::InBuildScope()) {
        // Born inside an ancestor's in-progress asynchronous build: ignore the
        // request (sticky, so re-delivery on later re-attach cannot re-enable it).
        asyncLoadSuppressed_ = true;
        auto* owner = RefPtrTraits<CustomNodeBase>::GetOwner(this);
        TAG_LOGD(AceLogTag::ACE_STATE_MGMT,
            "AsyncLoad ignored: node %{public}d created inside an in-progress asynchronous build",
            owner ? owner->GetId() : 0);
        return;
    }
    asyncLoadConfig_ = std::move(config);
}

bool CustomNodeBase::HasAsyncLoadConfig() const
{
    return asyncLoadConfig_ != nullptr;
}

const AsyncLoadConfig* CustomNodeBase::GetAsyncLoadConfig() const
{
    return asyncLoadConfig_.get();
}

// --- Asynchronous-loading state machine ---
// Implementation bodies live here so the widely-included header only carries
// declarations (header fan-out gate).

bool CustomNodeBase::IsAsyncLoadPending() const
{
    return asyncLoadState_ == AsyncLoadState::PENDING || asyncLoadState_ == AsyncLoadState::LOADING
        || asyncLoadState_ == AsyncLoadState::FORCED;
}

// True while the content is still deferred (not yet built). Layout-path build
// entry points must not render the component in this window, and
// custom measure/layout callbacks are suppressed while deferred.
bool CustomNodeBase::IsAsyncLoadDeferred() const
{
    return asyncLoadState_ == AsyncLoadState::PENDING || asyncLoadState_ == AsyncLoadState::LOADING;
}

// First-content-build gate shared by every build entry: Build() itself plus the
// layout-path entries (AdjustLayoutWrapperTree / GetFrameChildByIndex /
// RenderCustomChild) — update paths such as if/else branch flips or runtime-added
// array items never pass through Build(). Detached prebuild chains keep synchronous
// semantics through the on-tree flag .
bool CustomNodeBase::ShouldStartAsyncLoad(bool onMainTree) const
{
    return asyncLoadState_ == AsyncLoadState::NONE && !asyncLoadSuppressed_ && HasAsyncLoadConfig()
        && onMainTree && HasUnbuiltContent();
}


namespace {
// Default timeout of asynchronous loading in ms (API contract, frozen).
constexpr int64_t DEFAULT_ASYNC_LOAD_TIMEOUT_MS = 1000;
} // namespace

bool CustomNodeBase::TryStartAsyncLoad()
{
    auto host = GetAsyncLoadHost();
    CHECK_NULL_RETURN(host, false);
    // Single evaluation point of the first-build gate: callers may invoke this
    // unguarded — a detached prebuild chain, a suppressed descendant or an
    // already-built node simply keeps synchronous semantics (GetContext() would
    // otherwise fall back to the ambient pipeline for detached nodes).
    if (!ShouldStartAsyncLoad(host->IsOnMainTree())) {
        return false;
    }
    auto context = host->GetContext();
    if (!context) {
        return false; // the caller keeps synchronous semantics; never stall
    }
    const AsyncLoadConfig* config = GetAsyncLoadConfig();
    CHECK_NULL_RETURN(config, false);
    resolvedTimeoutMs_ = static_cast<int64_t>(config->timeoutMs.value_or(DEFAULT_ASYNC_LOAD_TIMEOUT_MS));
    asyncLoadState_ = AsyncLoadState::PENDING;
    TAG_LOGI(AceLogTag::ACE_STATE_MGMT,
        "AsyncLoad register: node %{public}d, placeholder configured %{public}d, resolvedTimeoutMs %{public}lld",
        host->GetId(), config->HasPlaceholder(), static_cast<long long>(resolvedTimeoutMs_));
    // Register into the pipeline idle set; the entry snapshots the timer fields
    // (remounts re-register with a fresh timer).
    context->RegisterAsyncLoadIdleEntry(WeakPtr<UINode>(host));
    MountAsyncLoadPlaceholder();
    return true;
}

void CustomNodeBase::RunAsyncLoadOnIdle(int64_t deadline)
{
    if (!IsAsyncLoadDeferred()) {
        return;
    }
    asyncLoadState_ = AsyncLoadState::LOADING;
    // The scheduler brackets this call with its build scope: descendants created
    // during the build keep synchronous semantics. Hosts whose build settles
    // internally (CustomNode via NodeDidBuild) report a completed build while no
    // longer pending, so the settle below no-ops for them.
    bool completed = ExecuteAsyncBuild(deadline);
    if (completed && IsAsyncLoadPending()) {
        SettleAsyncLoadDone(false);
    }
}

void CustomNodeBase::ForceLoad()
{
    if (!IsAsyncLoadPending()) {
        return;
    }
    asyncLoadState_ = AsyncLoadState::FORCED;
    auto host = GetAsyncLoadHost();
    TAG_LOGI(AceLogTag::ACE_STATE_MGMT, "AsyncLoad force-load begin: node %{public}d",
        host ? host->GetId() : 0);
    // deadline 0 means no budget limit: build the whole content synchronously so the
    // component is guaranteed to reach the screen (timeout backstop). The scheduler
    // brackets this call with its build scope (descendant suppression).
    ExecuteAsyncBuild(0);
    if (IsAsyncLoadPending()) {
        // Degenerate completion (nothing left to build): settle anyway so the node
        // does not stay tracked and the placeholder does not linger.
        SettleAsyncLoadDone(true);
    }
    FlushAsyncLoadCompletion(true);
}

void CustomNodeBase::SettleAsyncLoadDone(bool wasForced)
{
    // DONE is terminal; the swap happens exactly once. Content is mounted by now, so
    // the placeholder goes away within the same frame: never a frame with neither,
    // nor two visible children. The idle-set entry is dropped by the pipeline
    // consumer on the same completion signal.
    asyncLoadState_ = AsyncLoadState::DONE;
    DetachAsyncLoadPlaceholder();
    // Slice-retained build state must not outlive the machine: a degenerate
    // settle (nothing left to build) can still carry a parked partial subtree.
    ReleaseAsyncBuildState();
    auto host = GetAsyncLoadHost();
    TAG_LOGI(AceLogTag::ACE_STATE_MGMT, "AsyncLoad done: node %{public}d, %{public}s",
        host ? host->GetId() : 0, wasForced ? "forced" : "natural");
    if (!wasForced) {
        // Natural completion happens outside the layout pass (idle slice): the host
        // issues the structural flush so the swap reaches the screen automatically.
        FlushAsyncLoadCompletion(false);
    }
}

void CustomNodeBase::MountAsyncLoadPlaceholder()
{
    if (asyncLoadPlaceholder_) {
        return;
    }
    const AsyncLoadConfig* config = GetAsyncLoadConfig();
    CHECK_NULL_VOID(config);
    auto host = GetAsyncLoadHost();
    CHECK_NULL_VOID(host);
    asyncLoadPlaceholder_ = AcquireAsyncLoadPlaceholder(host, *config);
}

void CustomNodeBase::DetachAsyncLoadPlaceholder()
{
    auto placeholder = asyncLoadPlaceholder_;
    if (!placeholder) {
        return;
    }
    asyncLoadPlaceholder_.Reset();
    CustomNodeBase::DetachAsyncLoadPlaceholder(std::move(placeholder));
}

int64_t CustomNodeBase::GetAsyncLoadResolvedTimeoutMs() const
{
    return resolvedTimeoutMs_;
}

// Shared JSON bridge parser: {"placeholderId"?: string, "timeout"?: int,
// "width"/"height"?: string}. Suggested sizes cross as strings — numbers were
// stringified on the caller side and mean vp; suffixed strings ("100%", "10px")
// keep their unit (same mapping as the scroll-placeholder bridges). Unparsable
// values stay unset. Returns nullptr when the payload is malformed.
std::shared_ptr<AsyncLoadConfig> CustomNodeBase::ParseAsyncLoadConfigJson(const std::string& payload)
{
    auto configJson = JsonUtil::ParseJsonString(payload);
    if (!configJson || !configJson->IsObject()) {
        return nullptr;
    }
    auto config = std::make_shared<AsyncLoadConfig>();
    auto placeholderId = configJson->GetValue("placeholderId");
    if (placeholderId && placeholderId->IsString()) {
        config->placeholderId = placeholderId->GetString();
    }
    auto timeout = configJson->GetValue("timeout");
    if (timeout && timeout->IsNumber()) {
        config->timeoutMs = timeout->GetInt();
    }
    auto width = configJson->GetValue("width");
    if (width && width->IsString()) {
        config->sizeWidth = StringUtils::StringToDimension(width->GetString(), true);
    }
    auto height = configJson->GetValue("height");
    if (height && height->IsString()) {
        config->sizeHeight = StringUtils::StringToDimension(height->GetString(), true);
    }
    return config;
}

// Consume the scroll-placeholder template registry (read-only reuse); no
// system default is substituted for an empty or unregistered id (no substitution rules from the placeholder module).
namespace {
void ApplyPlaceholderSuggestedSize(const RefPtr<UINode>& placeholder, const AsyncLoadConfig& config);
}

RefPtr<UINode> CustomNodeBase::AcquireAsyncLoadPlaceholder(
    const RefPtr<UINode>& host, const AsyncLoadConfig& config)
{
    CHECK_NULL_RETURN(host, nullptr);
    auto context = host->GetContext();
    CHECK_NULL_RETURN(context, nullptr);
    auto scrollManager = context->GetOrCreateScrollPlaceholderManager();
    CHECK_NULL_RETURN(scrollManager, nullptr);
    if (!config.HasPlaceholder()) {
        // Not configured: the component stays invisible until its content is ready.
        return nullptr;
    }
    if (!scrollManager->IsTemplateRegistered(config.placeholderId)) {
        // An unregistered (or already unregistered) id degrades to "no
        // placeholder" for the caller, but is recorded as a maintainability
        // warning log (hilog), not an error code.
        TAG_LOGW(AceLogTag::ACE_STATE_MGMT,
            "AsyncLoad placeholder id '%{public}s' is not registered, no placeholder shown",
            config.placeholderId.c_str());
        return nullptr;
    }
    auto placeholder = scrollManager->AcquireTemplateInstance(config.placeholderId);
    if (!placeholder) {
        return nullptr;
    }
    placeholder->MountToParent(host);
    ApplyPlaceholderSuggestedSize(placeholder, config);
    return placeholder;
}

// Suggested size (PlaceholderOptions.size): acts as the placeholder root's
// ideal size during measurement, mirroring the scroll-placeholder shell
// (same validity contract: non-finite/non-positive or px-unnormalizable
// dimensions are dropped as unusable; a suggested size overrides the
// template root's own user-defined size for this instance, an unset axis
// keeps the template's layout).
namespace {
void ApplyPlaceholderSuggestedSize(const RefPtr<UINode>& placeholder, const AsyncLoadConfig& config)
{
    if (!config.sizeWidth.has_value() && !config.sizeHeight.has_value()) {
        return;
    }
    auto frame = AceType::DynamicCast<FrameNode>(placeholder);
    if (!frame) {
        TAG_LOGD(AceLogTag::ACE_STATE_MGMT,
            "AsyncLoad placeholder suggested size ignored: root is not a FrameNode");
        return;
    }
    auto context = frame->GetContext();
    auto scaleProperty = context ? ScaleProperty::CreateScaleProperty(context) : ScaleProperty();
    auto isSuggestedSizeUsable = [&scaleProperty](const Dimension& dim) {
        if (!std::isfinite(dim.Value()) || LessNotEqual(dim.Value(), 0.0)) {
            return false;
        }
        auto px = ConvertToPx(CalcLength(dim), scaleProperty, 0.0f, {});
        return !px.has_value() || std::isfinite(px.value());
    };
    CalcSize idealSize;
    bool hasUsableDimension = false;
    if (config.sizeWidth.has_value() && isSuggestedSizeUsable(config.sizeWidth.value())) {
        idealSize.SetWidth(CalcLength(config.sizeWidth.value()));
        hasUsableDimension = true;
    }
    if (config.sizeHeight.has_value() && isSuggestedSizeUsable(config.sizeHeight.value())) {
        idealSize.SetHeight(CalcLength(config.sizeHeight.value()));
        hasUsableDimension = true;
    }
    if (hasUsableDimension) {
        frame->GetLayoutProperty()->UpdateUserDefinedIdealSize(idealSize);
    }
}
} // namespace

void CustomNodeBase::DetachAsyncLoadPlaceholder(RefPtr<UINode> placeholder)
{
    auto parent = placeholder ? placeholder->GetParent() : nullptr;
    if (parent) {
        parent->RemoveChild(placeholder);
    }
}

void CustomNodeBase::SetUpdateFunction(std::function<void()>&& updateFunc)
{
    updateFunc_ = std::move(updateFunc);
}

void CustomNodeBase::SetDestroyFunction(std::function<void()>&& destroyFunc)
{
    destroyFunc_ = std::move(destroyFunc);
}

void CustomNodeBase::SetReloadFunction(std::function<void(bool)>&& reloadFunc)
{
    reloadFunc_ = std::move(reloadFunc);
}

void CustomNodeBase::FireReloadFunction(bool deep)
{
    if (reloadFunc_) {
        reloadFunc_(deep);
    }
}

void CustomNodeBase::Update()
{
    needRebuild_ = false;
    if (updateFunc_) {
        updateFunc_();
    }
}

void CustomNodeBase::SetPageTransitionFunction(std::function<void()>&& pageTransitionFunc)
{
    pageTransitionFunc_ = std::move(pageTransitionFunc);
}

void CustomNodeBase::CallPageTransitionFunction() const
{
    if (pageTransitionFunc_) {
        pageTransitionFunc_();
    }
}

void CustomNodeBase::SetForceUpdateNodeFunc(std::function<void(int32_t)>&& forceNodeUpdateFunc)
{
    forceNodeUpdateFunc_ = std::move(forceNodeUpdateFunc);
}

void CustomNodeBase::FireNodeUpdateFunc(ElementIdType id)
{
    if (forceNodeUpdateFunc_) {
        forceNodeUpdateFunc_(id);
    } else {
        LOGE("fail to find node update func to execute %{public}d node update", id);
    }
}

void CustomNodeBase::SetHasNodeUpdateFunc(std::function<bool(int32_t)>&& hasNodeUpdateFunc)
{
    hasNodeUpdateFunc_ = std::move(hasNodeUpdateFunc);
}

bool CustomNodeBase::FireHasNodeUpdateFunc(ElementIdType id)
{
    return hasNodeUpdateFunc_ && hasNodeUpdateFunc_(id);
}

void CustomNodeBase::SetRecycleFunction(std::function<void(RefPtr<CustomNodeBase>)>&& recycleCustomNode)
{
    recycleCustomNodeFunc_ = std::move(recycleCustomNode);
}

void CustomNodeBase::SetRecycleRenderFunc(std::function<void()>&& func)
{
    recycleRenderFunc_ = std::move(func);
}

bool CustomNodeBase::HasRecycleRenderFunc()
{
    return recycleRenderFunc_ != nullptr;
}

void CustomNodeBase::ResetRecycle()
{
    recycleRenderFunc_ = nullptr;
}

void CustomNodeBase::SetSetActiveFunc(std::function<void(bool, bool, bool)>&& func)
{
    setActiveFunc_ = std::move(func);
}

void CustomNodeBase::FireSetActiveFunc(bool active, bool isReuse, bool suppressActiveLifecycle)
{
    if (setActiveFunc_) {
        setActiveFunc_(active, isReuse, suppressActiveLifecycle);
    }
}

void CustomNodeBase::FireOnDumpInfoFunc(const std::vector<std::string>& params)
{
    if (onDumpInfoFunc_) {
        onDumpInfoFunc_(params);
    }
}

void CustomNodeBase::Reset()
{
    updateFunc_ = nullptr;
    appearFunc_ = nullptr;
    destroyFunc_ = nullptr;
    didBuildFunc_ = nullptr;
    setActiveFunc_ = nullptr;
    pageTransitionFunc_ = nullptr;
    reloadFunc_ = nullptr;
    completeReloadFunc_ = nullptr;
    forceNodeUpdateFunc_ = nullptr;
    hasNodeUpdateFunc_ = nullptr;
    recycleCustomNodeFunc_ = nullptr;
    recycleRenderFunc_ = nullptr;
    onDumpInfoFunc_ = nullptr;
    onDumpInspectorFunc_ = nullptr;
    clearParentReusePoolFunc_ = nullptr;
    getThisFunc_ = nullptr;
    onRecycleFunc_ = nullptr;
    onReuseFunc_ = nullptr;
    releaseRecyclePoolFunc_ = nullptr;
    getMemOptFunc_ = nullptr;
    enableReleaseExpiringNodesFunc_ = nullptr;
}

void CustomNodeBase::SetJSViewName(std::string&& name)
{
    jsViewName_ = name;
}

std::string& CustomNodeBase::GetJSViewName()
{
    return jsViewName_;
}

void CustomNodeBase::SetExtraInfo(const ExtraInfo extraInfo)
{
    extraInfo_ = std::move(extraInfo);
}

bool CustomNodeBase::GetIsV2()
{
    return isV2_;
}

void CustomNodeBase::SetIsV2(bool isV2)
{
    isV2_ = isV2;
}

ReusableMemOptStrategy CustomNodeBase::GetReusableMemOptStrategy()
{
    if (reusableMemOptStrategy_ != ReusableMemOptStrategy::UNDEFINED) {
        return reusableMemOptStrategy_;
    }
    auto applicationStrategy = LazyForEachUtils::GetReusableMemOptStrategy();
    if (applicationStrategy != ReusableMemOptStrategy::UNDEFINED) {
        reusableMemOptStrategy_ = applicationStrategy;
        return reusableMemOptStrategy_;
    }
    auto systemStrategy = SystemProperties::GetSyntaxMemOptStrategy();
    reusableMemOptStrategy_ = systemStrategy == 1 ?
        ReusableMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION : ReusableMemOptStrategy::DEFAULT;
    return reusableMemOptStrategy_;
}

void CustomNodeBase::SetReusableMemOptStrategy(int32_t reusableMemOptStrategy)
{
    reusableMemOptStrategy_ = static_cast<ReusableMemOptStrategy>(reusableMemOptStrategy);
}

void CustomNodeBase::SetReleaseRecyclePoolFunction(std::function<bool(int32_t, bool, bool)>&& callback)
{
    releaseRecyclePoolFunc_ = std::move(callback);
}

void CustomNodeBase::SetEnableReleaseExpiringNodesFunction(
    std::function<void(bool, const std::vector<std::string>&)>&& callback)
{
    enableReleaseExpiringNodesFunc_ = std::move(callback);
}

void CustomNodeBase::TryEnableParentCustomNodeMemOpt()
{
    if (GetReusableMemOptStrategy() == ReusableMemOptStrategy::DEFAULT) {
        return;
    }
    auto uiNode = AceType::DynamicCast<UINode>(AceType::Claim(this));
    CHECK_NULL_VOID(uiNode);
    auto parentCustomNode = uiNode->GetParentCustomNode();
    CHECK_NULL_VOID(parentCustomNode);
    parentCustomNode->StartMemOpt();
}

const ExtraInfo& CustomNodeBase::GetExtraInfo() const
{
    return extraInfo_;
}

bool CustomNodeBase::HasExtraInfo()
{
    if (!extraInfo_.page.empty()) {
        return true;
    }
    return false;
}

void CustomNodeBase::SetThisFunc(std::function<void*()>&& getThisFunc)
{
    getThisFunc_ = std::move(getThisFunc);
}

void* CustomNodeBase::FireThisFunc()
{
    if (getThisFunc_) {
        return getThisFunc_();
    }
    return nullptr;
}

void CustomNodeBase::FireClearAllRecycleFunc()
{
    if (clearAllRecycleFunc_) {
        ACE_SCOPED_TRACE("CustomNode:FireClearAllRecycleFunc %s", GetJSViewName().c_str());
        RecycleManager::ClearAll();
        clearAllRecycleFunc_();
    }
}

std::string CustomNodeBase::FireOnDumpInspectorFunc()
{
    if (onDumpInspectorFunc_) {
        return onDumpInspectorFunc_();
    }
    return "";
}

void CustomNodeBase::SetClearParentReusePoolFunc(std::function<void()>&& func)
{
    clearParentReusePoolFunc_ = std::move(func);
}

void CustomNodeBase::FireClearParentReusePoolFunc()
{
    if (clearParentReusePoolFunc_) {
        ACE_SCOPED_TRACE("CustomNode:FireClearParentReusePoolFunc %s", GetJSViewName().c_str());
        clearParentReusePoolFunc_();
    }
}

bool CustomNodeBase::CheckFireOnAppear()
{
    return executeFireOnAppear_;
}

void CustomNodeBase::MarkNeedUpdate()
{
    auto context = PipelineContext::GetCurrentContext();
    if (!context) {
        TAG_LOGW(AceLogTag::ACE_STATE_MGMT, "context no longer exist when [%{public}s] call markNeedUpdate",
            GetJSViewName().c_str());
        return;
    }
    if (needRebuild_) {
        TAG_LOGW(AceLogTag::ACE_STATE_MGMT, "[%{public}s] call markNeedUpdate in rebuild", GetJSViewName().c_str());
        return;
    }
    needRebuild_ = true;
    context->AddDirtyCustomNode(AceType::DynamicCast<UINode>(AceType::Claim(this)));
}

void CustomNodeBase::FireRecycleSelf()
{
    auto uiNode = AceType::DynamicCast<UINode>(AceType::Claim(this));
    uiNode->OnRecycle();
    if (recycleCustomNodeFunc_) {
        recycleInfo_.Recycle(uiNode->GetId());
        RecycleManager::Push(uiNode->GetId(), AceType::WeakClaim(this));
        recycleCustomNodeFunc_(AceType::Claim<CustomNodeBase>(this));
    }
}

void CustomNodeBase::FireRecycleRenderFunc()
{
    if (recycleRenderFunc_) {
        auto node = AceType::DynamicCast<UINode>(AceType::Claim(this));
        recycleInfo_.Reuse();
        RecycleManager::Pop(node->GetId());
        {
            ScopedViewStackProcessor scopedViewStackProcessor;
            recycleRenderFunc_();
        }
        node->OnReuse();
        node->SetJSViewActive(true, false, true);
        recycleRenderFunc_ = nullptr;
    }
}

void CustomNodeBase::SetOnDumpInfoFunc(std::function<void(const std::vector<std::string>&)>&& func)
{
    onDumpInfoFunc_ = func;
}

void CustomNodeBase::SetOnDumpInspectorFunc(std::function<std::string()>&& func)
{
    onDumpInspectorFunc_ = func;
}

void CustomNodeBase::SetMemOptGetter(std::function<int32_t()>&& func)
{
    getMemOptFunc_ = std::move(func);
}

int32_t CustomNodeBase::GetMemOpt() const
{
    return getMemOptFunc_ ? getMemOptFunc_() : 0;
}

void CustomNodeBase::SetStaMemopt(int32_t opt)
{
    staReusableMemOptStrategy_ =
        (opt == 1)? StaReusableMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION: StaReusableMemOptStrategy::DEFAULT;
}

void CustomNodeBase::SetClearAllRecycleFunc(std::function<void()>&& func)
{
    clearAllRecycleFunc_ = func;
}

void CustomNodeBase::SetCreatorId(const std::string& creatorId)
{
    creatorId_ = creatorId;
}

const std::string& CustomNodeBase::GetCreatorId() const
{
    return creatorId_;
}

void CustomNodeBase::SetReuseId(const std::string& reuseId)
{
    reuseId_ = reuseId;
}

const std::string& CustomNodeBase::GetReuseId() const
{
    return reuseId_;
}

void CustomNodeBase::SetOnRecycleFunc(std::function<void()>&& func)
{
    onRecycleFunc_ = func;
}

void CustomNodeBase::SetOnReuseFunc(std::function<void(void*)>&& func)
{
    onReuseFunc_ = func;
}

void CustomNodeBase::FireOnRecycleFunc()
{
    if (onRecycleFunc_) {
        onRecycleFunc_();
    }
}

void CustomNodeBase::FireOnReuseFunc(void* params)
{
    if (onReuseFunc_) {
        onReuseFunc_(params);
    }
}
} // namespace OHOS::Ace::NG
