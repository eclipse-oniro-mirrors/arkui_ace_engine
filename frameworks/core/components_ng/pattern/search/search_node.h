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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SEARCH_SEARCH_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SEARCH_SEARCH_NODE_H

#include <optional>

#include "core/components_ng/base/group_node.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT SearchNode : public GroupNode {
    DECLARE_ACE_TYPE(SearchNode, GroupNode);

public:
    SearchNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot = false)
        : GroupNode(tag, nodeId, pattern, isRoot)
    {}
    ~SearchNode() override = default;

    void AddChildToGroup(const RefPtr<UINode>& child, int32_t slot = DEFAULT_NODE_SLOT) override;
    bool HasTextFieldNode() const
    {
        return textFieldId_.has_value();
    }

    bool HasSearchIconNodeCreated() const
    {
        return searchIconNodeCreated_;
    }

    bool HasButtonNode() const
    {
        return buttonId_.has_value();
    }

    bool HasCancelIconNodeCreated() const
    {
        return cancelIconNodeCreated_;
    }

    bool HasCancelButtonNode() const
    {
        return cancelButtonId_.has_value();
    }

    int32_t GetTextFieldId()
    {
        if (!textFieldId_.has_value()) {
            textFieldId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return textFieldId_.value();
    }

    int32_t GetButtonId()
    {
        if (!buttonId_.has_value()) {
            buttonId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return buttonId_.value();
    }

    int32_t GetCancelButtonId()
    {
        if (!cancelButtonId_.has_value()) {
            cancelButtonId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return cancelButtonId_.value();
    }

    const Dimension& GetSearchIconSize() const
    {
        return searchIconSize_;
    }

    const Dimension& GetCancelIconSize() const
    {
        return cancelIconSize_;
    }

    const Color& GetSearchIconColor() const
    {
        return searchIconColor_;
    }

    const Color& GetCancelIconColor() const
    {
        return cancelIconColor_;
    }

    void SetSearchIconSize(const Dimension searchIconSize)
    {
        searchIconSize_ = searchIconSize;
    }

    void SetCancelIconSize(const Dimension cancelIconSize)
    {
        cancelIconSize_ = cancelIconSize;
    }

    void SetSearchIconColor(const Color searchIconColor)
    {
        searchIconColor_ = searchIconColor;
    }

    void SetCancelIconColor(const Color cancelIconColor)
    {
        cancelIconColor_ = cancelIconColor;
    }

    void UpdateHasSearchIconNodeCreated(bool hasNodeCreated)
    {
        searchIconNodeCreated_ = hasNodeCreated;
    }

    void UpdateHasCancelIconNodeCreated(bool hasNodeCreated)
    {
        cancelIconNodeCreated_ = hasNodeCreated;
    }

private:
    std::optional<int32_t> textFieldId_;
    std::optional<int32_t> buttonId_;
    std::optional<int32_t> cancelButtonId_;

    std::set<int32_t> searchChildren_;

    Dimension searchIconSize_ = 16.0_vp;
    Color searchIconColor_;
    Dimension cancelIconSize_ = 16.0_vp;
    Color cancelIconColor_;
    bool searchIconNodeCreated_ = false;
    bool cancelIconNodeCreated_ = false;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SEARCH_SEARCH_NODE_H