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

#include "frameworks/core/components_ng/svg/parse/svg_use.h"

#include "base/utils/utils.h"
#include "frameworks/core/components/declaration/svg/svg_declaration.h"

namespace OHOS::Ace::NG {
namespace {
const char DOM_SVG_SRC_VIEW_BOX[] = "viewBox";
}

SvgUse::SvgUse() : SvgGraphic() {}

void SvgUse::OnInitStyle()
{
    useOffsetX_ = useAttr_.x.Value();
    useOffsetY_ = useAttr_.y.Value();
}

RefPtr<SvgNode> SvgUse::Create()
{
    return AceType::MakeRefPtr<SvgUse>();
}

RSRecordingPath SvgUse::AsPath(const Size& viewPort) const
{
    auto svgContext = svgContext_.Upgrade();
    CHECK_NULL_RETURN(svgContext, RSRecordingPath());
    if (attributes_.href.empty()) {
        LOGE("href is empty");
        return {};
    }
    auto refSvgNode = svgContext->GetSvgNodeById(attributes_.href);
    CHECK_NULL_RETURN(refSvgNode, RSRecordingPath());

    AttributeScope scope(refSvgNode);
    refSvgNode->InheritAttr(attributes_);
    return refSvgNode->AsPath(viewPort);
}

void SvgUse::OnDraw(RSCanvas& canvas, const Size& layout, const std::optional<Color>& color)
{
    auto svgContext = svgContext_.Upgrade();
    CHECK_NULL_VOID(svgContext);
    if (attributes_.href.empty()) {
        return;
    }
    auto refSvgNode = svgContext->GetSvgNodeById(attributes_.href);
    CHECK_NULL_VOID(refSvgNode);

    if (useAttr_.x.Value() != 0 || useAttr_.y.Value() != 0) {
        canvas.Translate(useAttr_.x.Value(), useAttr_.y.Value());
    }
    AttributeScope scope(refSvgNode);
    refSvgNode->InheritUseAttr(attributes_);

    refSvgNode->Draw(canvas, layout, color);
}

bool SvgUse::ParseAndSetSpecializedAttr(const std::string& name, const std::string& value)
{
    static const LinearMapNode<void (*)(const std::string&, SvgAttributes&)> SVG_ATTR_ARRAY[] = {
        { DOM_SVG_MIRROR,
            [](const std::string& val, SvgAttributes& attr) {
                attr.autoMirror = val == "true";
            } },
        { DOM_SVG_HEIGHT,
            [](const std::string& val, SvgAttributes& attr) {
                attr.height = SvgAttributesParser::ParseDimension(val);
            } },
        { DOM_SVG_SRC_VIEW_BOX,
            [](const std::string& val, SvgAttributes& attr) {
                if (val.empty()) {
                    return;
                }
                std::vector<double> viewBox;
                StringUtils::StringSplitter(val, ' ', viewBox);
                if (viewBox.size() == 4) {
                    attr.viewBox = Rect(viewBox[0], viewBox[1], viewBox[2], viewBox[3]);
                }
            } },
        { DOM_SVG_VIEW_BOX,
            [](const std::string& val, SvgAttributes& attr) {
                if (val.empty()) {
                    return;
                }
                std::vector<double> viewBox;
                StringUtils::StringSplitter(val, ' ', viewBox);
                if (viewBox.size() == 4) {
                    attr.viewBox = Rect(viewBox[0], viewBox[1], viewBox[2], viewBox[3]);
                }
            } },
        { DOM_SVG_WIDTH,
            [](const std::string& val, SvgAttributes& attr) {
                attr.width = SvgAttributesParser::ParseDimension(val);
            } },
        { DOM_SVG_X,
            [](const std::string& val, SvgAttributes& attr) {
                attr.x = SvgAttributesParser::ParseDimension(val);
            } },
        { DOM_SVG_Y,
            [](const std::string& val, SvgAttributes& attr) {
                attr.y = SvgAttributesParser::ParseDimension(val);
            } },
    };
    auto attrIter = BinarySearchFindIndex(SVG_ATTR_ARRAY, ArraySize(SVG_ATTR_ARRAY), name.c_str());
    if (attrIter != -1) {
        SVG_ATTR_ARRAY[attrIter].value(value, useAttr_);
        return true;
    }
    return false;
}

SvgUse::AttributeScope::AttributeScope(const RefPtr<SvgNode>& node) : node_(node)
{
    auto nodeAttr = node->GetBaseAttributes();
    attributes_ = nodeAttr;
}

SvgUse::AttributeScope::~AttributeScope()
{
    auto node = node_.Upgrade();
    CHECK_NULL_VOID(node);
    node->SetBaseAttributes(attributes_);
}
} // namespace OHOS::Ace::NG
