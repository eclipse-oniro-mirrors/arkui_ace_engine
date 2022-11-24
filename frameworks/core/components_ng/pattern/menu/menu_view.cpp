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

#include "core/components_ng/pattern/menu/menu_view.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/option/option_paint_property.h"
#include "core/components_ng/pattern/option/option_view.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

// create menuWrapper and menu node, update menu props
std::pair<RefPtr<FrameNode>, RefPtr<FrameNode>> CreateMenu(const std::string& targetTag, int32_t targetId,
    bool isNeedUpdateMenuOffset, const NG::OffsetF& offset = NG::OffsetF(), bool isNavigationMenu = false)
{
    // use wrapper to detect click events outside menu
    auto wrapperNode = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(targetId));

    auto menuNode = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuPattern>(targetId));

    // update menu props
    auto layoutProps = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    if (layoutProps && isNeedUpdateMenuOffset) {
        layoutProps->UpdateMenuOffset(offset);
    }

    auto menuFrameNode = menuNode->GetPattern<MenuPattern>();
    menuFrameNode->SetIsNavigationMenu(isNavigationMenu);
    menuNode->MountToParent(wrapperNode);
    menuNode->MarkModifyDone();

    return { wrapperNode, menuNode };
}

// create menu with menuItems
RefPtr<FrameNode> MenuView::Create(
    std::vector<OptionParam>&& params, const std::string& targetTag, int32_t targetId,
    const NG::OffsetF& offset, bool isNavigationMenu)
{
    auto [wrapperNode, menuNode] = CreateMenu(targetTag, targetId, true, offset, isNavigationMenu);
    // append options to menu
    for (size_t i = 0; i < params.size(); ++i) {
        auto optionNode = OptionView::CreateMenuOption(params[i].first, std::move(params[i].second), i);
        // first node never paints divider
        if (i == 0) {
            auto props = optionNode->GetPaintProperty<OptionPaintProperty>();
            props->UpdateNeedDivider(false);
        }
        optionNode->MountToParent(menuNode);
        optionNode->MarkModifyDone();
    }
    return wrapperNode;
}

// create menu with custom node from a builder
RefPtr<FrameNode> MenuView::Create(
    const RefPtr<UINode>& customNode, const std::string& targetTag, int32_t targetId, const NG::OffsetF& offset)
{
    auto [wrapperNode, menuNode] = CreateMenu(targetTag, targetId, true, offset);
    // put custom node in a scroll to limit its height
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    CHECK_NULL_RETURN(scroll, nullptr);
    auto props = scroll->GetLayoutProperty<ScrollLayoutProperty>();
    props->UpdateAxis(Axis::VERTICAL);

    customNode->MountToParent(scroll);
    scroll->MountToParent(menuNode);
    scroll->MarkModifyDone();

    return wrapperNode;
}

RefPtr<FrameNode> MenuView::Create(
    const std::vector<SelectParam>& params, const std::string& targetTag, int32_t targetId)
{
    auto [wrapperNode, menuNode] = CreateMenu(targetTag, targetId, false);
    for (size_t i = 0; i < params.size(); ++i) {
        auto optionNode = OptionView::CreateSelectOption(params[i].first, params[i].second, i);
        // first node never paints divider
        if (i == 0) {
            auto props = optionNode->GetPaintProperty<OptionPaintProperty>();
            props->UpdateNeedDivider(false);
        }
        optionNode->MountToParent(menuNode);
    }
    return wrapperNode;
}

} // namespace OHOS::Ace::NG
