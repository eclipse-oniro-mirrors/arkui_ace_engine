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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_STICKY_HEADER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_STICKY_HEADER_PATTERN_H

#include <cstdint>
#include <optional>

#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {
class FrameNode;
class UINode;
enum class StickyStyle : uint32_t;

// Geometry-only section handoff; implementing this capability does not grant lazy layout eligibility.
class ACE_FORCE_EXPORT StickyHeaderPattern : public virtual AceType {
    DECLARE_ACE_TYPE(StickyHeaderPattern, AceType);

public:
    struct ACE_FORCE_EXPORT EdgeState {
        EdgeState();

        // Only a committed, non-null header/footer enables edge layout. Keep the index mode after clear;
        // sticky configuration and initial empty resets must not opt legacy instances into this mode.
        bool enabled = false;
        bool footerControlled = false;
        WeakPtr<UINode> header;
        WeakPtr<UINode> headerContent;
        WeakPtr<UINode> footerContent;
        uint64_t headerGeneration = 0;
        uint64_t footerGeneration = 0;
        StickyStyle sticky;
        std::optional<float> nextStickyHeaderGap;
        bool lastNotifiedHasStickyHeader = false;
        OffsetF origin;
        float totalSize = 0.0f;
        float measuredTotalSize = 0.0f;
        bool viewportRegistered = false;

        uint64_t BeginUpdate(bool isHeader)
        {
            if (!isHeader) {
                footerControlled = true;
            }
            return isHeader ? ++headerGeneration : ++footerGeneration;
        }

        bool IsCurrent(bool isHeader, uint64_t generation) const
        {
            return generation != 0 && generation == (isHeader ? headerGeneration : footerGeneration);
        }

        void InvalidateGenerations()
        {
            headerContent.Reset();
            footerContent.Reset();
            ++headerGeneration;
            ++footerGeneration;
        }
    };

    // Reserve a per-edge token; only the latest builder result may replace the live subtree.
    uint64_t BeginEdgeUpdate(bool isHeader);
    bool IsValidEdgeContent(bool isHeader, const RefPtr<UINode>& content) const;
    void SetSticky(StickyStyle style);
    virtual bool HasStickyHeader() const = 0;
    virtual void PublishNextStickyHeaderGap(const std::optional<float>& gap);
    virtual const std::optional<float>& GetNextStickyHeaderGap() const
    {
        return edgeState_.nextStickyHeaderGap;
    }
    // Existing lazy sections retain their measurement handoff; only edge-slot clients opt into layout refreshes.
    virtual bool RequiresStickyHandoffInLayout() const
    {
        return edgeState_.enabled;
    }
    void NotifyStickyHeaderChange();
    // Save natural layout geometry and register one weak viewport callback until detach.
    void SetEdgeGeometry(const OffsetF& origin, float totalSize, std::optional<float> measuredTotalSize = std::nullopt);
    virtual void UpdateEdgePositions() {}
    const EdgeState& GetEdgeState() const
    {
        return edgeState_;
    }

protected:
    // The component retains ownership of its legacy footer and supplies its measured edge sizes.
    virtual RefPtr<UINode> GetStickyFooter() const
    {
        return nullptr;
    }
    void UpdateStickyEdgePositions(float headerHeight, float footerHeight);
    void DetachStickyEdges(FrameNode* frameNode);
    EdgeState edgeState_;

private:
    RefPtr<FrameNode> GetStickyHost() const;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_STICKY_HEADER_PATTERN_H
