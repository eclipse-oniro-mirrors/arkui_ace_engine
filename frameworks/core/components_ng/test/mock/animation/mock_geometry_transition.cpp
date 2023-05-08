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

#include "core/components_ng/animation/geometry_transition.h"

namespace OHOS::Ace::NG {
    void GeometryTransition::Build(const WeakPtr<FrameNode>& frameNode, bool isNodeIn) {}
    void GeometryTransition::WillLayout(const RefPtr<FrameNode>& frameNode) {}
    void GeometryTransition::DidLayout(const RefPtr<FrameNode>& frameNode) {}

    bool GeometryTransition::Update(const WeakPtr<FrameNode>& which, const WeakPtr<FrameNode>& value)
    {
        return false;
    }

    bool GeometryTransition::IsNodeInAndActive(const RefPtr<FrameNode>& frameNode) const
    {
        return false;
    }

    bool GeometryTransition::IsRunning() const
    {
        return false;
    }

    bool GeometryTransition::IsInAndOutEmpty() const
    {
        return true;
    }

    std::string GeometryTransition::ToString() const
    {
        return std::string("");
    }
} // namespace OHOS::Ace::NG