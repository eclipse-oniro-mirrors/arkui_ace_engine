/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/core/components/svg/parse/svg_path.h"

#include "frameworks/core/components/svg/render_svg_path.h"

namespace OHOS::Ace {

SvgPath::SvgPath() : SvgNode()
{
    component_ = AceType::MakeRefPtr<SvgPathComponent>();
}

RefPtr<SvgNode> SvgPath::Create()
{
    return AceType::MakeRefPtr<SvgPath>();
}

void SvgPath::SetAttr(const std::string& name, const std::string& value)
{
    component_->GetDeclaration()->SetSpecializedAttr(std::make_pair(name, value));
}

void SvgPath::AppendChild(const RefPtr<SvgNode>& child)
{
    component_->AppendChild(child->GetComponent());
}

RefPtr<RenderNode> SvgPath::CreateRender(
    const LayoutParam& layoutParam, const RefPtr<SvgBaseDeclaration>& parent, bool useBox)
{
    component_->Inherit(parent);
    auto& declaration = component_->GetDeclaration();
    auto href = declaration->GetFillState().GetHref();
    if (!href.empty()) {
        auto gradient = GetGradient(href);
        if (gradient) {
            declaration->SetGradient(gradient.value());
        }
    }
    auto renderNode = AceType::DynamicCast<RenderSvgPath>(component_->CreateRenderNode());
    if (!renderNode) {
        LOGE("create renderNode failed");
        return nullptr;
    }
    renderNode->Attach(context_);
    renderNode->Update(component_);
    if (!useBox || declaration->GetClipPathHref().empty()) {
        LOGW("path of tag skip box create");
        return renderNode;
    }

    auto boxComponent = CreateBoxComponent(layoutParam, declaration->GetClipPathHref());
    auto renderBox = boxComponent->CreateRenderNode();
    renderBox->Attach(context_);
    renderBox->Update(boxComponent);
    renderBox->AddChild(renderNode);
    return renderBox;
}

RSPath SvgPath::AsPath(const Size& viewPort) const
{
    RSPath out;
    if (!component_->GetD().empty()) {
        out.BuildFromSVGString(component_->GetD());
        if (component_->GetDeclaration()->GetClipState().IsEvenodd()) {
            out.SetFillStyle(RSPathFillType::EVENTODD);
        }
    }
    return out;
}

} // namespace OHOS::Ace
