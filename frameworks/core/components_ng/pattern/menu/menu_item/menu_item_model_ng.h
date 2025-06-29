/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_ITEM_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_ITEM_MODEL_NG_H

#include "core/components_ng/pattern/menu/menu_item/menu_item_model.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/property/menu_property.h"
#include "core/components/select/select_theme.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT MenuItemModelNG : public OHOS::Ace::MenuItemModel {
public:
    void Create(const RefPtr<NG::UINode>& customNode) override;
    void Create(const MenuItemProperties& props) override;
    void SetSelected(bool isSelected = false) override;
    void SetSelectIcon(bool isShow = false) override;
    void SetSelectIconSrc(const std::string& src) override;
    void SetOnChange(std::function<void(bool)>&& onChange) override;
    void SetFontSize(const Dimension& fontSize) override;
    void SetFontWeight(FontWeight weight) override;
    void SetFontStyle(Ace::FontStyle style) override;
    void SetFontColor(const std::optional<Color>& color) override;
    void SetFontFamily(const std::vector<std::string> &families) override;
    void SetLabelFontSize(const Dimension& fontSize) override;
    void SetLabelFontWeight(FontWeight weight) override;
    void SetLabelFontStyle(Ace::FontStyle style) override;
    void SetLabelFontColor(const std::optional<Color>& color) override;
    void SetLabelFontFamily(const std::vector<std::string> &families) override;
    void SetSelectedChangeEvent(std::function<void(bool)>&& selectedChangeEvent) override;
    void SetSelectIconSymbol(std::function<void(WeakPtr<NG::FrameNode>)>&& symbolApply) override;
    void CreateWithColorResourceObj(const RefPtr<ResourceObject>& resObj, MenuItemFontColorType type) override;
    void CreateWithDimensionFpResourceObj(const RefPtr<ResourceObject>& resObj, MenuItemFontSizeType type) override;
    void CreateWithFontFamilyResourceObj(const RefPtr<ResourceObject>& resObj, MenuItemFontFamilyType type) override;
    void CreateWithStringResourceObj(const RefPtr<ResourceObject>& resObj, MenuItemStringType type) override;
    void CreateWithMediaResourceObj(const RefPtr<ResourceObject>& resObj, const MenuItemIconType type) override;

    static void SetSelected(FrameNode* frameNode, bool isSelected = false);
    static void SetLabelFontColor(FrameNode* frameNode, const std::optional<Color>& color);
    static void SetLabelFontSize(FrameNode* frameNode, const Dimension& fontSize);
    static void SetLabelFontWeight(FrameNode* frameNode, FontWeight weight);
    static void SetLabelFontFamily(FrameNode* frameNode, const std::vector<std::string>& families);
    static void SetLabelFontStyle(FrameNode* frameNode, Ace::FontStyle style);
    static void SetFontColor(FrameNode* frameNode, const std::optional<Color>& color);
    static void SetFontSize(FrameNode* frameNode, const Dimension& fontSize);
    static void SetFontWeight(FrameNode* frameNode, FontWeight weight);
    static void SetFontFamily(FrameNode* frameNode, const std::vector<std::string>& families);
    static void SetFontStyle(FrameNode* frameNode, Ace::FontStyle style);
    static void SetSelectIcon(FrameNode* frameNode, bool isShow = false);
    static void SetSelectIconSrc(FrameNode* frameNode, const std::string& src);
    static void SetSelectIconSymbol(FrameNode* frameNode, std::function<void(WeakPtr<NG::FrameNode>)>&& symbolApply);
    static void SetOnChange(FrameNode* frameNode, std::function<void(bool)>&& onChange);
    static RefPtr<FrameNode> CreateMenuItem(OptionParam&& param, const MenuParam& menuParam);
    static void SetSelectedChangeEvent(FrameNode* frameNode, std::function<void(bool)>&& selectedChangeEvent);
    static void CreateWithColorResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj,
        MenuItemFontColorType type);
    static void CreateWithDimensionFpResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj,
        MenuItemFontSizeType type);
    static void CreateWithFontFamilyResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj,
        MenuItemFontFamilyType type);
    static void CreateWithStringResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj,
        MenuItemStringType type);
    static void CreateWithMediaResourceObj(FrameNode* frameNode, const RefPtr<ResourceObject>& resObj,
        const MenuItemIconType type);
private:
    void UpdateMenuProperty(const RefPtr<NG::FrameNode>& menuItem, const MenuItemProperties& menuItemProps);
    static void SetupMenuItemProperties(const RefPtr<FrameNode>& menuItem, const RefPtr<SelectTheme>& theme);
    static const std::string ColorTypeToString(const MenuItemFontColorType type);
    static const std::string DimensionTypeToString(const MenuItemFontSizeType type);
    static const std::string StringTypeToString(const MenuItemStringType type);
    static const std::string FamilyTypeToString(const MenuItemFontFamilyType type);
    static const std::string MediaTypeToString(const MenuItemIconType type);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_ITEM_MODEL_NG_H
