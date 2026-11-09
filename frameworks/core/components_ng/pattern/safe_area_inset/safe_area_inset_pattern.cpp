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

#include "base/json/json_util.h"
#include "base/utils/string_utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_layout_algorithm.h"
#include "core/components_ng/property/safe_area_inset_property.h"

namespace OHOS::Ace::NG {
namespace {

// A settled layout is required for the contribution fields: pending layout
// work must not publish stale numbers.
bool HasPendingSafeAreaLayout(const FrameNode* node)
{
    auto property = node->GetLayoutProperty();
    CHECK_NULL_RETURN(property, true);
    auto flag = property->GetPropertyChangeFlag();
    return node->IsLayoutDirtyMarked() || CheckNeedMeasure(flag) || CheckNeedLayout(flag);
}

// A cached result and clean owner/carrier are required for both numeric fields.
// Never use GetOrCreateSafeAreaPadding here: dumping must not populate caches.
bool HasSettledSafeAreaContribution(const FrameNode* owner, const FrameNode* carrier)
{
    auto geometry = owner->GetGeometryNode();
    return geometry && geometry->GetResolvedSingleSafeAreaPadding() &&
        !HasPendingSafeAreaLayout(owner) && !HasPendingSafeAreaLayout(carrier);
}

// Only the new SafeAreaInsetContribution/EffectiveSafeAreaContribution fields use this
// presentation: numbers fixed to two decimals, missing edges as NA, same edge order as
// PaddingPropertyF::ToString. Legacy fields keep their original formatters.
std::string FormatSafeAreaInsetContributionEdge(const std::optional<float>& value)
{
    constexpr int32_t CONTRIBUTION_DECIMALS = 2;
    return value.has_value() ? StringUtils::DoubleToString(value.value(), CONTRIBUTION_DECIMALS) : "NA";
}

std::string FormatSafeAreaInsetContribution(const PaddingPropertyF& contribution)
{
    std::string str;
    str.append("[").append(FormatSafeAreaInsetContributionEdge(contribution.left));
    str.append(",").append(FormatSafeAreaInsetContributionEdge(contribution.right));
    str.append(",").append(FormatSafeAreaInsetContributionEdge(contribution.top));
    str.append(",").append(FormatSafeAreaInsetContributionEdge(contribution.bottom)).append("]");
    return str;
}
} // namespace

RefPtr<LayoutAlgorithm> SafeAreaInsetPattern::CreateLayoutAlgorithm()
{
    return MakeRefPtr<SafeAreaInsetLayoutAlgorithm>();
}

RefPtr<UINode> SafeAreaInsetPattern::GetEdgeContent(SafeAreaInsetEdge edge) const
{
    // contents_ in the header spells the edge count to avoid including the
    // full safeAreaInset option definitions there.
    static_assert(std::tuple_size<decltype(contents_)>::value == SAFE_AREA_INSET_EDGE_COUNT);
    return contents_[SafeAreaInsetOptions::ToIndex(edge)];
}

std::unique_ptr<JsonValue> BuildSafeAreaInsetConfig(const FrameNode* owner)
{
    auto carrier = owner->GetSafeAreaInsetNode();
    CHECK_NULL_RETURN(carrier, nullptr);
    auto pattern = carrier->GetPattern<SafeAreaInsetPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    auto property = owner->GetLayoutProperty();
    CHECK_NULL_RETURN(property, nullptr);
    const auto* options = property->GetSafeAreaInsetState().GetSafeAreaInset();
    CHECK_NULL_RETURN(options, nullptr);
    constexpr std::array<const char*, SAFE_AREA_INSET_EDGE_COUNT> EDGE_NAMES = { "top", "end", "bottom", "start" };
    constexpr std::array<const char*, 3> ALIGNMENTS = { "START", "CENTER", "END" };
    auto config = JsonUtil::Create(true);
    bool hasContent = false;
    for (size_t index = 0; index < EDGE_NAMES.size(); ++index) {
        auto edge = static_cast<SafeAreaInsetEdge>(index);
        const auto& option = options->GetEdge(edge);
        auto content = pattern->GetEdgeContent(edge);
        if (!option || !content) {
            continue;
        }
        auto value = JsonUtil::Create(true);
        value->Put("contentNodeId", content->GetId());
        value->Put("spacing", option->spacing.ToString().c_str());
        auto alignment = static_cast<size_t>(option->alignment);
        value->Put("alignment", alignment < ALIGNMENTS.size() ? ALIGNMENTS[alignment] : "UNKNOWN");
        config->Put(EDGE_NAMES[index], value);
        hasContent = true;
    }
    return hasContent ? std::move(config) : nullptr;
}

std::unique_ptr<JsonValue> BuildSafeAreaInsetDumpInfo(const FrameNode* node)
{
    auto carrier = node->GetSafeAreaInsetNode();
    if (carrier) {
        auto info = JsonUtil::Create(true);
        info->Put("SafeAreaInsetContainerNodeId", carrier->GetId());
        if (HasSettledSafeAreaContribution(node, AceType::RawPtr(carrier))) {
            info->Put("EffectiveSafeAreaContribution",
                FormatSafeAreaInsetContribution(*node->GetGeometryNode()->GetResolvedSingleSafeAreaPadding()).c_str());
        }
        return info;
    }
    if (node->GetTag() != SAFE_AREA_INSET_CONTAINER_TAG) {
        return nullptr;
    }
    auto owner = AceType::DynamicCast<FrameNode>(node->GetParent());
    CHECK_NULL_RETURN(owner, nullptr);
    auto ownerCarrier = owner->GetSafeAreaInsetNode();
    if (AceType::RawPtr(ownerCarrier) != node) {
        return nullptr;
    }
    auto config = BuildSafeAreaInsetConfig(AceType::RawPtr(owner));
    CHECK_NULL_RETURN(config, nullptr);
    auto info = JsonUtil::Create(true);
    info->Put("SafeAreaInset", config);
    if (HasSettledSafeAreaContribution(AceType::RawPtr(owner), node)) {
        info->Put("SafeAreaInsetContribution",
            FormatSafeAreaInsetContribution(
                owner->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution())
                .c_str());
    }
    return info;
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
