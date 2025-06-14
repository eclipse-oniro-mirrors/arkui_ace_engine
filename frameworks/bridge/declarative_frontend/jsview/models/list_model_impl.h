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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_LIST_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_LIST_MODEL_IMPL_H

#include "core/components_ng/pattern/list/list_model.h"

namespace OHOS::Ace::Framework {

class ListModelImpl : public ListModel {
public:
    ListModelImpl() = default;
    ~ListModelImpl() override = default;

    void Create(bool isCreateArc = false) override;
    void SetSpace(const Dimension& space) override;
    void SetInitialIndex(int32_t initialIndex) override;
    RefPtr<ScrollControllerBase> CreateScrollController() override;
    void SetScroller(RefPtr<ScrollControllerBase> scroller, RefPtr<ScrollProxy> proxy) override;
    void SetListDirection(Axis axis) override;
    void SetScrollBar(DisplayMode scrollBar) override;
    void SetScrollBarColor(const std::string& /* value */) override {};
    void SetScrollBarWidth(const std::string& /* value */) override {};
    void SetEdgeEffect(EdgeEffect edgeEffect, bool alwaysEnabled, EffectEdge effectEdge) override;
    void SetEditMode(bool editMode) override;
    void SetDivider(const V2::ItemDivider& divider) override;
    void SetChainAnimation(bool enableChainAnimation) override;
    void SetChainAnimationOptions(const ChainAnimationOptions& options) override {}
    void SetLanes(int32_t lanes) override;
    void SetLaneConstrain(const Dimension& laneMinLength, const Dimension& laneMaxLength) override;
    void SetLaneMinLength(const Dimension& laneMinLength) override {}
    void SetLaneMaxLength(const Dimension& laneMaxLength) override {}
    void SetLaneGutter(const Dimension& laneGutter) override {}
    void SetListItemAlign(V2::ListItemAlign listItemAlign) override;
    void SetCachedCount(int32_t cachedCount, bool show = false) override;
    void SetMultiSelectable(bool selectable) override;
    void SetHasWidth(bool hasWidth) override;
    void SetHasHeight(bool hasHeight) override;
    void SetSticky(V2::StickyStyle stickyStyle) override;
    void SetContentStartOffset(float startOffset) override;
    void SetContentEndOffset(float endOffset) override;
    void SetNestedScroll(const NestedScrollOptions& nestedOpt) override {};
    void SetScrollEnabled(bool scrollEnabled) override {};
    void SetFriction(double friction) override {};
    void SetFocusWrapMode(FocusWrapMode focusWrapMode) override {};
    void SetMaintainVisibleContentPosition(bool enabled) override {};
    void SetStackFromEnd(bool enabled) override {};
    void SetSyncLoad(bool enabled) override {};
    void SetOnScroll(OnScrollEvent&& onScroll) override;
    void SetOnScrollBegin(OnScrollBeginEvent&& onScrollBegin) override;
    void SetOnScrollFrameBegin(OnScrollFrameBeginEvent&& onScrollFrameBegin) override;
    void SetOnScrollStart(OnScrollStopEvent&& onScrollStop) override {}
    void SetOnScrollStop(OnScrollStopEvent&& onScrollStop) override;
    void SetOnScrollIndex(OnScrollIndexEvent&& onScrollIndex) override;
    void SetOnScrollVisibleContentChange(OnScrollVisibleContentChangeEvent&& onScrollVisibleContentChange) override {};
    void SetOnReachStart(OnReachEvent&& onReachStart) override;
    void SetOnReachEnd(OnReachEvent&& onReachEnd) override;
    void SetOnItemDelete(OnItemDeleteEvent&& onItemDelete) override;
    void SetOnItemMove(OnItemMoveEvent&& onItemMove) override;
    void SetOnItemDragStart(OnItemDragStartFunc&& onItemDragStart) override;
    void SetOnItemDragEnter(OnItemDragEnterFunc&& onItemDragEnter) override;
    void SetOnItemDragLeave(OnItemDragLeaveFunc&& onItemDragLeave) override;
    void SetOnItemDragMove(OnItemDragMoveFunc&& onItemDragMove) override;
    void SetOnItemDrop(OnItemDropFunc&& onItemDrop) override;
    void SetDividerColorByUser(bool isByUser) override {};

    DisplayMode GetDisplayMode() const override
    {
        return DisplayMode::OFF;
    }
};

} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_MODEL_H
