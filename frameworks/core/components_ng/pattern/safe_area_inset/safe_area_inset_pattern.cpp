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

#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_pattern.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_layout_algorithm.h"

namespace OHOS::Ace::NG {

RefPtr<LayoutAlgorithm> SafeAreaInsetPattern::CreateLayoutAlgorithm()
{
    return MakeRefPtr<SafeAreaInsetLayoutAlgorithm>();
}

bool SafeAreaInsetPattern::SetEdgeContent(SafeAreaInsetEdge edge, const RefPtr<UINode>& content)
{
    const auto index = SafeAreaInsetOptions::ToIndex(edge);
    auto oldContent = contents_[index];
    if (oldContent == content) {
        return false;
    }

    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    if (oldContent) {
        host->RemoveChild(oldContent);
    }
    contents_[index] = content;
    if (content) {
        auto oldParent = content->GetParent();
        if (oldParent && oldParent != host) {
            oldParent->RemoveChild(content);
        }
        content->MountToParent(host, GetEdgeMountSlot(edge));
        content->FlushUpdateAndMarkDirty();
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return true;
}

int32_t SafeAreaInsetPattern::GetContentFrameIndex(SafeAreaInsetEdge edge) const
{
    if (!GetEdgeContent(edge)) {
        return -1;
    }
    int32_t frameIndex = 0;
    for (size_t index = 0; index < SafeAreaInsetOptions::ToIndex(edge); ++index) {
        if (contents_[index]) {
            frameIndex += contents_[index]->FrameCount();
        }
    }
    return frameIndex;
}

bool SafeAreaInsetPattern::Empty() const
{
    for (const auto& content : contents_) {
        if (content) {
            return false;
        }
    }
    return true;
}

int32_t SafeAreaInsetPattern::GetEdgeMountSlot(SafeAreaInsetEdge edge) const
{
    int32_t slot = 0;
    const auto end = SafeAreaInsetOptions::ToIndex(edge);
    for (size_t index = 0; index < end; ++index) {
        if (contents_[index]) {
            ++slot;
        }
    }
    return slot;
}

} // namespace OHOS::Ace::NG
