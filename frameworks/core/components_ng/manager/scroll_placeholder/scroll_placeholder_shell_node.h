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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_SHELL_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_SHELL_NODE_H

#include <string_view>

#include "base/memory/ace_type.h"
#include "core/components_ng/base/frame_node.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {

// Dedicated tag of the placeholder shell node (V2::SCROLL_PLACEHOLDER_SHELL_ETS_TAG,
// maintained in inspector_constants.h with the other component tags). The shell
// deliberately carries a ListItem pattern but NOT the ListItem tag (e.g. the focus
// path's IsListItem must not mistake a shell for a focusable item), and
// dumps/inspector output distinguish shells from real items at a glance.

// A placeholder shell is mounted as a direct child of the scroll container host but
// is NOT a list/grid/waterflow item: it must stay invisible to every child-slot
// enumeration — TotalChildCount, the repeat/total item count walk and index-based
// child resolution all walk host children by FrameCount(). A zero frame count keeps
// the container's item index space, and every index derived from it, exactly what it
// was before any shell was mounted; otherwise each mounted shell inflates the total
// item count and the layout starts walking phantom indexes past the real data range.
// The flip side — index-based cleanup can not see shells either — is closed by the
// mounting pattern's mounted-shell registry and orphan sweep (see ListPattern): every
// shell the pattern mounted is tracked there, and any shell the two retirement tables
// no longer track is swept at the next layout round, so a bookkeeping miss can never
// leak a mounted shell onto the tree.
class ScrollPlaceholderShellNode : public FrameNode {
public:
    explicit ScrollPlaceholderShellNode(std::string_view tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : FrameNode(tag, nodeId, pattern)
    {}
    ~ScrollPlaceholderShellNode() override = default;

    static RefPtr<ScrollPlaceholderShellNode> CreateScrollPlaceholderShellNode(
        std::string_view tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
    {
        auto shell = MakeRefPtr<ScrollPlaceholderShellNode>(tag, nodeId, pattern);
        ElementRegister::GetInstance()->AddUINode(shell);
        shell->InitializePatternAndContext();
        return shell;
    }

    int32_t FrameCount() const override
    {
        return 0;
    }

    int32_t CurrentFrameCount() const override
    {
        return 0;
    }

    // The shell owns no index slot, so index-derived activation windows are
    // meaningless for it — and actively harmful: the shell's FrameProxy entry
    // starts at the total item count, so every real viewport window maps to a
    // negative window in its coordinates, which the inherited FrameNode
    // implementation reads as out-of-range and deactivates the shell on every
    // layout round; the inactive shell is then dropped by the render-tree
    // children collection. The shell manages its own and its content's
    // activation explicitly at mount (ActivatePlaceholderSubtree), so the
    // inherited range math is replaced by a no-op. This covers every dispatch
    // path into the shell, not only FrameProxy::SetActiveChildRange.
    void DoSetActiveChildRange(
        int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool showCache = false) override
    {
        (void)start;
        (void)end;
        (void)cacheStart;
        (void)cacheEnd;
        (void)showCache;
    }
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_SHELL_NODE_H
