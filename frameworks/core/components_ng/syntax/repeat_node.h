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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_REPEAT_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_REPEAT_NODE_H

#include <cstdint>
#include <list>
#include <optional>
#include <string>
#include <type_traits>

#include "base/utils/macros.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/syntax/for_each_base_node.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT RepeatNode : public ForEachBaseNode {
    DECLARE_ACE_TYPE(RepeatNode, ForEachBaseNode);

public:
    static RefPtr<RepeatNode> GetOrCreateRepeatNode(int32_t nodeId);

    explicit RepeatNode(int32_t nodeId) : ForEachBaseNode(V2::JS_REPEAT_ETS_TAG, nodeId) {}

    ~RepeatNode() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    void CreateTempItems();

    // RepeatNode only
    void FinishRepeatRender(std::list<int32_t>& removedElmtId);

    void FlushUpdateAndMarkDirty() override;

    // RepeatNode only
    void MoveChild(uint32_t fromIndex);

    const std::list<std::string>& GetTempIds() const
    {
        return tempIds_;
    }

    void SetIds(std::list<std::string>&& ids)
    {
        ids_ = std::move(ids);
    }

    void AfterAddChild();
    void SetOnMove(std::function<void(int32_t, int32_t)>&& onMove);
    void SetItemDragHandler(std::function<void(int32_t)>&& onLongPress, std::function<void(int32_t)>&& onDragStart,
        std::function<void(int32_t, int32_t)>&& onMoveThrough, std::function<void(int32_t)>&& onDrop);

    void MoveData(int32_t from, int32_t to, bool isNeedUpdate = false) override;
    RefPtr<FrameNode> GetFrameNode(int32_t index) override;
    void InitDragManager(const RefPtr<UINode>& childNode);
    void InitAllChildrenDragManager(bool init);
    void DumpInfo() override;

    bool IsAllowAnimation();
    bool IsChildInAnimation(uint32_t fromIndex);
    std::pair<int32_t, int32_t> GetActiveRange();
    void DoSetActiveChildRange(
        int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool showCache = false) override;

    void OnAttachToMainTree(bool recursive) override;
    void OnDetachFromMainTree(bool recursive, PipelineContext* context = nullptr) override;

    /**
     * tri-state reuse-in-animation permission. Explicitly false opts this node out of
     * reuse during implicit animations and switches the direct Grid / WaterFlow parent
     * to the sync-cache-in-layout mode while attached; nullopt / true keep the default
     * behavior. See LazyForEachUtils::ToggleParentSyncCacheLayout.
     */
    void SetAllowReuseInAnimation(std::optional<bool> allow);

private:
    std::list<std::string> ids_;
    int32_t from_ = -1;
    int32_t to_ = -1;

    int32_t activeRangeStart_ = -1;
    int32_t activeRangeEnd_ = -1;

    // tri-state reuse-in-animation permission; explicitly false switches the direct
    // Grid / WaterFlow parent to the sync-cache-in-layout mode while attached
    std::optional<bool> isAllowReuseInAnimation_;
    // Grid / WaterFlow frame node holding this node's sync-cache-in-layout reference
    WeakPtr<FrameNode> syncCacheHost_;

    // temp items use to compare each update.
    std::list<std::string> tempIds_;
    std::list<RefPtr<UINode>> tempChildren_;

    // RepeatNode only
    std::vector<RefPtr<UINode>> tempChildrenOfRepeat_;

    uint32_t AdjustFromIndex(uint32_t fromIndex);

    ACE_DISALLOW_COPY_AND_MOVE(RepeatNode);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_REPEAT_NODE_H
