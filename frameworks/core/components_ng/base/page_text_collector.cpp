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
#include "core/components_ng/base/page_text_collector.h"

#include <cmath>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/subwindow/subwindow_manager.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/page_text_json.h"
#include "core/components_ng/pattern/page_translate/page_translate_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "interfaces/native/native_type.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Container> GetPageTextContainer(int32_t instanceId)
{
    auto container = Container::GetContainer(instanceId);
    // Container lookup can redirect plugin IDs to their parent; this API must not.
    if (container && container->GetInstanceId() != instanceId) {
        container.Reset();
    }
    return container;
}

#ifndef CROSS_PLATFORM
enum class InputOwner { NONE, ATOMIC_SEARCH, COUNTER };

struct PageTextContext {
    InputOwner inputOwner = InputOwner::NONE;
    RefPtr<FrameNode> backgroundHost;
    RefPtr<FrameNode> backgroundRoot;
    bool excluded = false;
    bool inEditor = false;
    bool detachedOffscreen = false;
    bool ownedOverlay = false;
};

// All caches live for one read-only query. Never call GetChildren() while collecting.
class PageTextScan {
public:
    PageTextScan(int32_t instanceId, const RefPtr<PipelineContext>& pipeline) : instanceId_(instanceId)
    {
        pipelines_.emplace(instanceId, pipeline);
    }

    RefPtr<PipelineContext> GetPipeline(int32_t instanceId)
    {
        auto [iter, inserted] = pipelines_.try_emplace(instanceId, RefPtr<PipelineContext>());
        if (!inserted) {
            return iter->second;
        }
        auto manager = SubwindowManager::GetInstance();
        if (manager->GetParentContainerId(instanceId) != instanceId_) {
            return nullptr;
        }
        auto window = manager->GetSubwindowById(instanceId);
        if (!window || window->GetChildContainerId() != instanceId || !window->GetShown()) {
            return nullptr;
        }
        auto container = GetPageTextContainer(instanceId);
        auto executor = container ? container->GetTaskExecutor() : nullptr;
        if (!executor || !executor->WillRunOnCurrentThread(TaskExecutor::TaskType::UI)) {
            return nullptr;
        }
        auto pipeline = AceType::DynamicCast<PipelineContext>(container->GetPipelineContext());
        if (pipeline && !pipeline->IsDestroyed() && pipeline->GetInstanceId() == instanceId) {
            iter->second = pipeline;
        }
        return iter->second;
    }

    OffsetF GetWindowOffset(int32_t instanceId)
    {
        const auto& parentRect = pipelines_.at(instanceId_)->GetDisplayWindowRectInfo();
        const auto& childRect = pipelines_.at(instanceId)->GetDisplayWindowRectInfo();
        return OffsetF(childRect.Left() - parentRect.Left(), childRect.Top() - parentRect.Top());
    }

    void AddFrame(const WeakPtr<FrameNode>& node)
    {
        auto frame = node.Upgrade();
        if (frame) {
            frames.try_emplace(frame->GetId(), frame);
        }
    }

    void Scan()
    {
        ElementRegister::GetInstance()->IterateElements([this](auto, const RefPtr<AceType>& element) {
            auto frame = AceType::DynamicCast<FrameNode>(element);
            if (!frame || !GetPipeline(frame->GetInstanceId())) {
                return false;
            }
            // Keep the association even when disabled, so a retained background cannot leak as ordinary text.
            if (auto background = frame->GetBackgroundNode()) {
                backgrounds_[AceType::RawPtr(background)] = frame;
            }
            auto render = frame->GetRenderContext();
            auto carrier = AceType::DynamicCast<PageTranslateNode>(frame->GetPattern());
            if (carrier && carrier->GetPageTextPlaceholderNodeId() >= 0) {
                placeholderIds_.emplace(carrier->GetPageTextPlaceholderNodeId());
            }
            if (carrier || (render && render->HasOverlayText())) {
                frames.try_emplace(frame->GetId(), frame);
            }
            return false;
        });
    }

    PageTextContext Resolve(const RefPtr<UINode>& node)
    {
        std::vector<RefPtr<UINode>> path;
        std::unordered_set<const UINode*> visiting;
        PageTextContext context;
        auto current = node;
        while (current) {
            if (!visiting.emplace(AceType::RawPtr(current)).second) {
                context.excluded = true;
                break;
            }
            auto found = contexts_.find(AceType::RawPtr(current));
            if (found != contexts_.end()) {
                context = found->second;
                break;
            }
            path.emplace_back(current);
            auto background = backgrounds_.find(AceType::RawPtr(current));
            if (background == backgrounds_.end()) {
                current = current->GetParent();
            } else {
                current = background->second;
            }
        }
        for (auto iter = path.rbegin(); iter != path.rend(); ++iter) {
            auto& item = *iter;
            context.excluded |= item->IsAccessibilityVirtualNode() || item->GetTag() == V2::WEB_ETS_TAG ||
                placeholderIds_.count(item->GetId()) > 0;
            ResolveOverlay(item, context);
            context.inEditor |= item->GetTag() == V2::RICH_EDITOR_ETS_TAG;
            context.detachedOffscreen |= item->UseOffscreenProcess() && !item->IsOnMainTree();
            if (AceType::InstanceOf<CustomNode>(item)) {
                const auto name = item->GetCustomTag();
                // The nearest custom component owns the input; do not inherit an exception into user builders.
                context.inputOwner = name == "AtomicServiceSearch" ? InputOwner::ATOMIC_SEARCH :
                    (name == "CounterComponent" || name == "CounterV2Component" ?
                        InputOwner::COUNTER : InputOwner::NONE);
            }
            auto background = backgrounds_.find(AceType::RawPtr(item));
            if (background != backgrounds_.end()) {
                context.backgroundHost = background->second;
                context.backgroundRoot = AceType::DynamicCast<FrameNode>(item);
                auto render = background->second->GetRenderContext();
                context.excluded |= !render || !render->GetBuilderBackgroundFlagValue(false);
            }
            contexts_.emplace(AceType::RawPtr(item), context);
        }
        return context;
    }

    std::map<int32_t, RefPtr<FrameNode>> frames;

private:
    bool OwnsTarget(int32_t targetId) const
    {
        auto target = ElementRegister::GetInstance()->GetUINodeById(targetId);
        return target && target->GetInstanceId() == instanceId_;
    }

    void ResolveOverlay(const RefPtr<UINode>& node, PageTextContext& context)
    {
        auto frame = AceType::DynamicCast<FrameNode>(node);
        CHECK_NULL_VOID(frame);
        if (frame->GetTag() == V2::MENU_WRAPPER_ETS_TAG) {
            int32_t targetId = -1;
            bool isShown = false;
            const bool hasMenu = OverlayManager::GetMenuWrapperState(frame, targetId, isShown);
            context.ownedOverlay = hasMenu && OwnsTarget(targetId);
            context.excluded |= !hasMenu || !isShown;
        } else if (frame->GetTag() == V2::POPUP_ETS_TAG) {
            int32_t targetId = -1;
            const bool hasBubble = OverlayManager::GetPopupTargetId(frame, targetId);
            auto pipeline = GetPipeline(frame->GetInstanceId());
            auto overlay = pipeline ? pipeline->GetOverlayManager() : nullptr;
            if (!hasBubble || !overlay) {
                context.excluded = true;
                return;
            }
            auto info = overlay->GetPopupInfo(targetId);
            context.ownedOverlay = info.isTips && OwnsTarget(targetId);
            context.excluded |= info.popupNode != frame || !info.isCurrentOnShow;
        }
    }

    int32_t instanceId_;
    std::unordered_map<int32_t, RefPtr<PipelineContext>> pipelines_;
    std::unordered_map<const UINode*, RefPtr<FrameNode>> backgrounds_;
    std::unordered_map<const UINode*, PageTextContext> contexts_;
    std::unordered_set<int32_t> placeholderIds_;
};

bool AllowInputText(const RefPtr<FrameNode>& frame, const PageTextContext& context)
{
    if (context.inputOwner == InputOwner::ATOMIC_SEARCH && frame->GetTag() == V2::SEARCH_Field_ETS_TAG) {
        auto parent = frame->GetAncestorNodeOfFrame(false);
        return parent && parent->GetTag() == V2::SEARCH_ETS_TAG;
    }
    if (context.inputOwner != InputOwner::COUNTER || frame->GetTag() != V2::TEXTINPUT_ETS_TAG) {
        return false;
    }
    const auto id = frame->GetInspectorIdValue("");
    for (const std::string prefix : { "InlineTextInput", "DateTextInput1", "DateTextInput2", "DateTextInput3" }) {
        if (id.compare(0, prefix.size(), prefix) == 0 && id.size() > prefix.size() &&
            id.find_first_not_of("0123456789", prefix.size()) == std::string::npos) {
            return true;
        }
    }
    return false;
}

bool HasPositivePaintSize(const RefPtr<FrameNode>& frame)
{
    const auto rect = frame->GetTransformRectRelativeToWindowOnlyVisible();
    return !LessOrEqual(rect.Width(), 0.0f) && !LessOrEqual(rect.Height(), 0.0f);
}

struct PageTextTransform {
    OffsetF offset;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

std::optional<PageTextTransform> GetBackgroundTextTransform(const RefPtr<FrameNode>& frame,
    const PageTextContext& context, const PageTextTransform& hostTransform)
{
    auto host = context.backgroundHost;
    auto root = context.backgroundRoot;
    auto offset = frame->GetPaintRectGlobalOffsetWithTranslate(false, true).first -
        root->GetPaintRectGlobalOffsetWithTranslate(false, true).first;
    auto region = host->GetBackGroundAccumulatedSafeAreaExpand();
    const auto& constraint = root->GetGeometryNode()->GetParentLayoutConstraint();
    auto initialSize = constraint ? constraint->maxSize : region.GetSize();
    if (!host->GetRenderContext()->GetIsTransitionBackgroundValue(false)) {
        const auto hostSize = host->GetGeometryNode()->GetFrameSize();
        initialSize -= region.GetSize() - hostSize;
        region = RectF(OffsetF(), hostSize);
    }
    if (!initialSize.IsPositive()) {
        return std::nullopt;
    }
    const auto sourceSize = root->GetGeometryNode()->GetFrameSize();
    auto align = host->GetRenderContext()->GetBackgroundAlignValue(Alignment::CENTER);
    // Use the same source/destination alignment as builder-background drawing.
    offset += Alignment::GetAlignPosition(initialSize, sourceSize, align) -
        Alignment::GetAlignPosition(sourceSize, initialSize, align);
    const float scaleX = region.Width() / initialSize.Width();
    const float scaleY = region.Height() / initialSize.Height();
    // Background-local offsets inherit every outer background's scale; the host offset is already global.
    offset.SetX((region.Left() + offset.GetX() * scaleX) * hostTransform.scaleX);
    offset.SetY((region.Top() + offset.GetY() * scaleY) * hostTransform.scaleY);
    return PageTextTransform { hostTransform.offset + offset,
        hostTransform.scaleX * scaleX, hostTransform.scaleY * scaleY };
}

std::optional<PageTextTransform> GetPageTextTransform(
    const RefPtr<FrameNode>& frame, const PageTextContext& context, PageTextScan& scan)
{
    if (context.excluded || (context.inEditor && frame->GetTag() != V2::RICH_EDITOR_ETS_TAG)) {
        return std::nullopt;
    }
    if (!context.backgroundHost) {
        // Mounted preload hosts retain the offscreen flag; apply normal eligibility to their visible text.
        // Detached offscreen nodes still require a current builder-background owner.
        if (context.detachedOffscreen || !HasPositivePaintSize(frame)) {
            return std::nullopt;
        }
        return PageTextTransform { frame->GetPaintRectGlobalOffsetWithTranslate(false, true).first };
    }
    auto host = context.backgroundHost;
    auto root = context.backgroundRoot;
    const auto hostTransform = GetPageTextTransform(host, scan.Resolve(host), scan);
    if (!host->IsVisible() || !hostTransform) {
        return std::nullopt;
    }
    auto render = frame->GetRenderContext();
    CHECK_NULL_RETURN(render, std::nullopt);
    auto paintRect = render->GetPaintRectWithTransform();
    if (LessOrEqual(paintRect.Width(), 0.0f) || LessOrEqual(paintRect.Height(), 0.0f)) {
        return std::nullopt;
    }
    for (auto current = frame; current; current = current->GetAncestorNodeOfFrame(false)) {
        if (!current->IsVisible() || (!current->IsActive() && !current->UseOffscreenProcess())) {
            return std::nullopt;
        }
        if (current == root) {
            break;
        }
    }
    return GetBackgroundTextTransform(frame, context, *hostTransform);
}

bool AppendCarrier(const RefPtr<FrameNode>& frame, const std::string& text,
    const OffsetF& offset, PageTextJson& json, bool& first)
{
    if (text.empty()) {
        return true;
    }
    const auto& rect = frame->GetGeometryNode()->GetFrameRect();
    const float values[] = { offset.GetX(), offset.GetY(), rect.Width(), rect.Height() };
    for (auto value : values) {
        if (!std::isfinite(value)) {
            return json.Fail("Page text geometry contains a non-finite value.");
        }
    }
    if (rect.Width() < 0 || rect.Height() < 0) {
        return json.Fail("Page text geometry has a negative size.");
    }
    if (!first && !json.Append(",")) {
        return false;
    }
    first = false;
    if (!json.Append("{\"id\":") || !json.Number(frame->GetId()) || !json.Append(",\"rect\":[")) {
        return false;
    }
    for (size_t i = 0; i < std::size(values); ++i) {
        if ((i && !json.Append(",")) || !json.Number(values[i])) {
            return false;
        }
    }
    return json.Append("],\"content\":") && json.String(text) && json.Append("}");
}

std::string GetFramePageText(const RefPtr<FrameNode>& frame, const PageTextContext& context)
{
    std::string text;
    if (auto carrier = AceType::DynamicCast<PageTranslateNode>(frame->GetPattern())) {
        text = carrier->GetPageTextForReport(AllowInputText(frame, context));
    }
    auto render = frame->GetRenderContext();
    if (!render || !render->HasOverlayText()) {
        return text;
    }
    const auto overlay = render->GetOverlayTextValue(OverlayOptions());
    if (overlay.content.empty()) {
        return text;
    }
    if (!text.empty()) {
        text += '\n';
    }
    text += overlay.content;
    return text;
}

bool AppendPageText(int32_t instanceId, const RefPtr<PipelineContext>& pipeline, PageTextJson& json)
{
    bool first = true;
    PageTextScan scan(instanceId, pipeline);
    auto manager = pipeline->GetUiTranslateManagerImpl();
    if (manager) {
        manager->ForEachArkUITranslateFrameNode([&scan](const WeakPtr<FrameNode>& node) { scan.AddFrame(node); });
    }
    scan.Scan();
    for (const auto& [id, frame] : scan.frames) {
        const auto nodeInstanceId = frame->GetInstanceId();
        if (!scan.GetPipeline(nodeInstanceId)) {
            continue;
        }
        ContainerScope nodeScope(nodeInstanceId);
        const auto context = scan.Resolve(frame);
        if (context.excluded || (nodeInstanceId != instanceId && !context.ownedOverlay)) {
            continue;
        }
        const auto text = GetFramePageText(frame, context);
        if (text.empty()) {
            continue;
        }
        const auto transform = GetPageTextTransform(frame, context, scan);
        if (transform &&
            !AppendCarrier(frame, text, transform->offset + scan.GetWindowOffset(nodeInstanceId), json, first)) {
            return false;
        }
    }
    return true;
}

#endif
} // namespace

int32_t CollectPageText(int32_t instanceId, char** data, uint32_t* size, const char** reason)
{
    *data = nullptr;
    *size = 0;
    auto container = GetPageTextContainer(instanceId);
    auto pipeline = container ? AceType::DynamicCast<PipelineContext>(container->GetPipelineContext()) : nullptr;
    if (!pipeline || pipeline->IsDestroyed()) {
        *reason = "UI context is invalid or destroyed.";
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    auto executor = container->GetTaskExecutor();
    if (!executor || !executor->WillRunOnCurrentThread(TaskExecutor::TaskType::UI)) {
        LOGF_ABORT("OH_ArkUI_NativeModule_UIAgentGetPageText must run on the target UI thread.");
    }
    ContainerScope scope(instanceId);
    *reason = "Page text serialization, allocation or size check failed.";
    PageTextJson json;
    bool success = json.Append("{\"schemaVersion\":1,\"texts\":[");
#ifndef CROSS_PLATFORM
    success = success && AppendPageText(instanceId, pipeline, json);
#endif
    if (!success || !json.Append("]}")) {
        *reason = json.GetError();
        return ARKUI_ERROR_CODE_INTERNAL_ERROR;
    }
    *data = json.Release(*size);
    return ARKUI_ERROR_CODE_NO_ERROR;
}
} // namespace OHOS::Ace::NG
