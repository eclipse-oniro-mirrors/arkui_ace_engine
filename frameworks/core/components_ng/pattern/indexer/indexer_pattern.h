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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_INDEXER_INDEXER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_INDEXER_INDEXER_PATTERN_H

#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/indexer/indexer_event_hub.h"
#include "core/components_ng/pattern/indexer/indexer_layout_algorithm.h"
#include "core/components_ng/pattern/indexer/indexer_layout_property.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {

class IndexerPattern : public Pattern {
    DECLARE_ACE_TYPE(IndexerPattern, Pattern);

public:
    IndexerPattern() = default;
    ~IndexerPattern() override = default;

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<IndexerEventHub>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<IndexerLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto indexerLayoutAlgorithm = MakeRefPtr<IndexerLayoutAlgorithm>();
        indexerLayoutAlgorithm->SetIsInitialized(isInitialized_);
        indexerLayoutAlgorithm->SetSelected(selected_);
        return indexerLayoutAlgorithm;
    }
    
    void SetIsTouch(bool isTouch)
    {
        isTouch_ = isTouch;
    }

    const std::vector<std::string>& GetArrayValue()
    {
        return arrayValue_;
    }
    
    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true };
    }
    
private:
    void OnModifyDone() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void OnTouchDown(const TouchEventInfo& info);

    void MoveIndexByOffset(const Offset& offset, bool isRepeatCalled = true);
    bool MoveIndexByStep(int32_t step);
    bool MoveIndexBySearch(const std::string& searchStr);
    void ApplyIndexChanged();

    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitOnKeyEvent(const RefPtr<FocusHub>& focusHub);
    bool OnKeyEvent(const KeyEvent& event);
    void BeginBubbleAnimation(RefPtr<FrameNode> animationNode);

    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<PanEvent> panEvent_;
    bool isInitialized_ = false;
    bool isTouch_ = false;

    std::vector<std::string> arrayValue_;
    int32_t itemCount_ = 0;
    int32_t selected_ = 0;
    float itemSizeRender_ = 0.0f;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_PATTERN_H
