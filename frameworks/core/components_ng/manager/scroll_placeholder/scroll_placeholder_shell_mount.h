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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_SHELL_MOUNT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_SHELL_MOUNT_H

#include <cmath>
#include <string>
#include <string_view>

#include "base/log/log_wrapper.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_manager.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_node_clone.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_shell_node.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_types.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#ifndef ACE_UNITTEST
#include "core/components_ng/render/adapter/rosen_render_context.h"
#endif

namespace OHOS::Ace::NG {
namespace scroll_placeholder_mount {
// Shared mount-side helpers for the component adapters of the scroll placeholder
// (Grid/WaterFlow; List keeps its pattern-local copies, see list_pattern.cpp).
// Every helper is UI-thread only, like the manager itself.

// Same value as V2::SCROLL_PLACEHOLDER_SHELL_ETS_TAG (inspector_constants.h):
// the extern const char[] definition is not exported through libace.map to the
// split component libraries, so the adapters carry their own constexpr copy of
// the informational tag instead of referencing the cross-library symbol.
constexpr std::string_view SHELL_ETS_TAG = "ScrollPlaceholderShell";

// The instance subtree is built offline (isolated view stack at registration or a
// background clone): every node still carries the default inactive flag, and
// FrameNode::SetActive - unlike the UINode base - activates only the node itself.
// An inactive subtree is dropped from layout participation and render sync
// (SetRerenderable gates on IsActive): a shell subtree left inactive renders
// blank inside the sized shell. Activate the whole subtree on mount.
inline void ActivatePlaceholderSubtree(const RefPtr<UINode>& node)
{
    auto frame = AceType::DynamicCast<FrameNode>(node);
    if (frame) {
        frame->SetActive(true);
    }
    for (const auto& child : node->GetChildren()) {
        ActivatePlaceholderSubtree(child);
    }
}

// RS-node probe: 1 = render context holds a live RS node, 0 = none (the
// GetChildrenRSNodes drop condition), -1 = probe unavailable in this build
// (host TDD: mocked render layer, no Rosen headers).
inline int32_t ProbeRsNodeState(const RefPtr<FrameNode>& node)
{
#ifndef ACE_UNITTEST
    auto rosen = AceType::DynamicCast<RosenRenderContext>(node->GetRenderContext());
    return (rosen && rosen->GetRSNode() != nullptr) ? 1 : 0;
#else
    (void)node;
    return -1;
#endif
}

// Deferral-blind probe around the shell's index for an already-built real item of
// the same host (GetFrameNodeChildByIndexWithoutBuild never resolves a deferral or
// a shell, and never builds). Raw pointer: the host tree owns the node, the
// caller only reads state within one log statement.
inline FrameNode* FindNeighborRealItem(const RefPtr<FrameNode>& host, int32_t index)
{
    constexpr int32_t probeRadius = 3;
    const int32_t totalCount = host->GetTotalChildCount();
    for (int32_t step = 1; step <= probeRadius; step++) {
        for (int32_t probe : { index - step, index + step }) {
            if (probe < 0 || probe >= totalCount) {
                continue;
            }
            auto* real = host->GetFrameNodeChildByIndexWithoutBuild(static_cast<uint32_t>(probe));
            if (real != nullptr) {
                return real;
            }
        }
    }
    return nullptr;
}

// Post-layout state of one live shell for diagnostics (same gate-layer breakdown
// as the List implementation, see list_pattern.cpp).
inline void LogScrollPlaceholderShellState(const RefPtr<FrameNode>& host, int32_t index, const RefPtr<FrameNode>& shell)
{
    auto shellSize = shell->GetGeometryNode()->GetFrameSize();
    bool shellOnRenderTree = shell->IsPendingOnMainRenderTree();
    auto* realItem = host ? FindNeighborRealItem(host, index) : nullptr;
    if (shell->GetChildren().empty()) {
        TAG_LOGD(AceLogTag::ACE_SCROLL,
            "ScrollPlaceholder shell live: index=%{public}d, shellId=%{public}d, "
            "shellSize=%{public}fx%{public}f, shellRs=%{public}d, content=none, "
            "shellActive=%{public}d, shellRsNode=%{public}d, realIdx=%{public}d, realPending=%{public}d, "
            "realRsNode=%{public}d",
            index, shell->GetId(), shellSize.Width(), shellSize.Height(),
            static_cast<int32_t>(shellOnRenderTree), static_cast<int32_t>(shell->IsActive()),
            ProbeRsNodeState(shell), realItem ? realItem->GetId() : -1,
            static_cast<int32_t>(realItem && realItem->IsPendingOnMainRenderTree()),
            (realItem ? ProbeRsNodeState(AceType::Claim(realItem)) : -1));
        return;
    }
    auto content = AceType::DynamicCast<FrameNode>(shell->GetChildren().front());
    CHECK_NULL_VOID(content);
    auto contentSize = content->GetGeometryNode()->GetFrameSize();
    TAG_LOGD(AceLogTag::ACE_SCROLL,
        "ScrollPlaceholder shell live: index=%{public}d, shellId=%{public}d, "
        "shellSize=%{public}fx%{public}f, shellRs=%{public}d, contentTag=%{public}s, contentActive=%{public}d, "
        "contentSize=%{public}fx%{public}f, contentRs=%{public}d, shellActive=%{public}d, shellRsNode=%{public}d, "
        "realIdx=%{public}d, realPending=%{public}d, realRsNode=%{public}d",
        index, shell->GetId(), shellSize.Width(), shellSize.Height(), static_cast<int32_t>(shellOnRenderTree),
        content->GetTag().c_str(), static_cast<int32_t>(content->IsActive()), contentSize.Width(),
        contentSize.Height(), static_cast<int32_t>(content->IsPendingOnMainRenderTree()),
        static_cast<int32_t>(shell->IsActive()), ProbeRsNodeState(shell),
        realItem ? realItem->GetId() : -1,
        static_cast<int32_t>(realItem && realItem->IsPendingOnMainRenderTree()),
        (realItem ? ProbeRsNodeState(AceType::Claim(realItem)) : -1));
}

// Placeholder shell: a plain item node carrying only the placeholder visual
// subtree (template instance) or just the suggested size - no item business
// logic, no business events by construction. The shell node type keeps a zero
// FrameCount so host-level child-slot enumerations never count it as an item
// (index-space integrity, see ScrollPlaceholderShellNode). The content pattern
// type follows the mounting component (ListItemPattern/GridItemPattern/
// WaterFlowItemPattern) so the shell measures like a real item of that host.
inline RefPtr<FrameNode> CreatePlaceholderShellNode(const ScrollPlaceholderOptions& options,
    const RefPtr<ScrollPlaceholderManager>& manager, const RefPtr<Pattern>& contentPattern)
{
    auto shell = ScrollPlaceholderShellNode::CreateScrollPlaceholderShellNode(SHELL_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), contentPattern);
    CHECK_NULL_RETURN(shell, nullptr);
    // Distinguish the three content-less shell causes in the mount log: a
    // size-only default shell is a designed provider choice, the other two are
    // wiring anomalies (acquire failures log on the manager side).
    if (options.id.empty()) {
        TAG_LOGD(AceLogTag::ACE_SCROLL,
            "ScrollPlaceholder size-only default shell mounted: provider returned no template id");
    } else if (!manager->IsTemplateRegistered(options.id)) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "ScrollPlaceholder shell mounted without template content: templateId=%{public}s "
            "is not registered (check the provider id matches PlaceholderRegister.register)",
            options.id.c_str());
    } else {
        auto instance = manager->AcquireTemplateInstance(options.id);
        if (instance) {
            shell->AddChild(instance);
            ActivatePlaceholderSubtree(instance);
            int32_t nodeCount = 0;
            int32_t depth = 0;
            CountScrollPlaceholderSubtree(instance, nodeCount, depth);
            TAG_LOGD(AceLogTag::ACE_SCROLL,
                "ScrollPlaceholder shell mounted with template content: templateId=%{public}s, "
                "rootTag=%{public}s, nodeCount=%{public}d, depth=%{public}d",
                options.id.c_str(), instance->GetTag().c_str(), nodeCount, depth);
        }
    }
    if (options.size.has_value()) {
        // Options carry unit-preserving Dimensions (bridges map plain numbers to vp;
        // strings keep PERCENT/PX/...). Pass through untranslated so PERCENT resolves
        // against the host cross-axis constraint at measure.
        // A suggested size that passes the raw-value guard can still overflow at px
        // normalization (e.g. Number.MAX_VALUE vp: NormalizeToPx x density then
        // static_cast<float> yields +Infinity). CreateIdealSize takes a valid
        // selfIdealSize as-is and the host measure path has no finite check, so such a
        // slot poisons the host scroll geometry - drop the unusable dimension, same
        // contract as an unset one. percentReference 0 keeps PERCENT finite (it still
        // resolves against the host cross-axis at measure); a nullopt px (calc without
        // rpnexp) stays on the existing passthrough path.
        auto isSuggestedSizeUsable = [&shell](const Dimension& dim) {
            if (!std::isfinite(dim.Value()) || LessNotEqual(dim.Value(), 0.0)) {
                return false;
            }
            auto px = ConvertToPx(CalcLength(dim), ScaleProperty::CreateScaleProperty(shell->GetContext()),
                0.0f, {});
            return !px.has_value() || std::isfinite(px.value());
        };
        CalcSize idealSize;
        bool hasUsableDimension = false;
        if (options.size->width.has_value()) {
            const auto& width = options.size->width.value();
            if (isSuggestedSizeUsable(width)) {
                idealSize.SetWidth(CalcLength(width));
                hasUsableDimension = true;
            } else {
                TAG_LOGW(AceLogTag::ACE_SCROLL,
                    "ScrollPlaceholder suggested size dropped as unusable: dim=%{public}s", width.ToString().c_str());
            }
        }
        if (options.size->height.has_value()) {
            const auto& height = options.size->height.value();
            if (isSuggestedSizeUsable(height)) {
                idealSize.SetHeight(CalcLength(height));
                hasUsableDimension = true;
            } else {
                TAG_LOGW(AceLogTag::ACE_SCROLL,
                    "ScrollPlaceholder suggested size dropped as unusable: dim=%{public}s", height.ToString().c_str());
            }
        }
        if (hasUsableDimension) {
            shell->GetLayoutProperty()->UpdateUserDefinedIdealSize(idealSize);
        }
    }
    // Node-as-wrapper: the host layout algorithm will drive this shell through
    // FrameNode::Measure directly. The measure flag is consumed by that first
    // measure, so it is set once here.
    shell->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    return shell;
}

// Mounted as a direct child of the host, like other non-item direct children;
// indexing is unaffected (syntax proxies own index resolution).
inline void MountPlaceholderShellOnHost(const RefPtr<FrameNode>& host, const RefPtr<FrameNode>& shell)
{
    auto* context = host->GetContext();
    CHECK_NULL_VOID(context);
    // AddChild assigns parent and depth (UINode::DoAddChild).
    host->AddChild(shell);
    if (host->IsOnMainTree()) {
        shell->AttachToMainTree(false, context);
    }
    shell->MarkNeedSyncRenderTree();
    shell->SetActive(true);
    // Explicit render-tree materialization for the freshly mounted subtree.
    // The layout-time AddChild only sets sync flags, and no consumer is
    // guaranteed to pick them up for the shell within the same frame: the RS
    // node tree of a newly mounted subtree only takes shape when a rebuild
    // actually runs. Schedule it after layout — the host rebuild enters the
    // shell into the host's RS tree, the shell rebuild enters the content into
    // the shell's RS tree. All shells of a frame mount during the layout pass
    // and the tasks drain after it, so the first host rebuild of a drain
    // already collects every pending shell; the shell rebuild stays per-shell.
    auto weakHost = AceType::WeakClaim(AceType::RawPtr(host));
    auto weakShell = AceType::WeakClaim(AceType::RawPtr(shell));
    context->AddAfterLayoutTask([weakHost, weakShell]() {
        auto host = weakHost.Upgrade();
        auto shell = weakShell.Upgrade();
        if (!host || !shell) {
            return;
        }
        // The rebuild must be preceded by MarkNeedSyncRenderTree — the flag set
        // at AddChild may already have been consumed by a mid-layout rebuild,
        // and a bare rebuild silently no-ops on the stale flag.
        host->MarkNeedSyncRenderTree();
        host->RebuildRenderContextTree();
        shell->MarkNeedSyncRenderTree();
        shell->RebuildRenderContextTree();
    });
}
} // namespace scroll_placeholder_mount
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_SHELL_MOUNT_H
