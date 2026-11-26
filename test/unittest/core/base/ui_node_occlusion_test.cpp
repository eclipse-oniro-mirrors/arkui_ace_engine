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

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>

#include "gtest/gtest.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

#include "base/geometry/matrix4.h"
#include "core/components/common/properties/clip_path.h"
#include "core/components_ng/pattern/blank/blank_paint_property.h"
#include "core/components_ng/pattern/blank/blank_pattern.h"
#include "core/components_ng/property/progress_mask_property.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_occlusion_collector.h"
#include "core/components_ng/base/inspector_occlusion_region.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
namespace {

TEST(OcclusionRegionTest, CompleteAndPartialCover)
{
    OcclusionRegionSet region;
    EXPECT_FALSE(region.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));
    EXPECT_EQ(region.Add(RectF(0.0f, 0.0f, 5.0f, 10.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_FALSE(region.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));
    EXPECT_EQ(region.Add(RectF(5.0f, 0.0f, 5.0f, 10.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_TRUE(region.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));
}

TEST(OcclusionRegionTest, OverlapDoesNotFillGap)
{
    OcclusionRegionSet region;
    EXPECT_EQ(region.Add(RectF(0.0f, 0.0f, 4.0f, 10.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_EQ(region.Add(RectF(3.0f, 0.0f, 3.0f, 10.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_EQ(region.Add(RectF(7.0f, 0.0f, 3.0f, 10.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_FALSE(region.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));
    EXPECT_EQ(region.Add(RectF(6.0f, 0.0f, 1.0f, 10.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_TRUE(region.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));
}

TEST(OcclusionRegionTest, ClippedEvidenceStaysInsideClip)
{
    OcclusionRegionSet region;
    EXPECT_EQ(region.AddClipped(RectF(0.0f, 0.0f, 10.0f, 10.0f), RectF(2.0f, 2.0f, 6.0f, 6.0f)),
        OcclusionRegionSet::AddResult::ADDED);
    EXPECT_TRUE(region.Covers(RectF(2.0f, 2.0f, 6.0f, 6.0f)));
    EXPECT_FALSE(region.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));
}

TEST(OcclusionRegionTest, LimitDoesNotExpandEvidence)
{
    OcclusionRegionSet::Limits limits;
    limits.maxRects = 1;
    OcclusionRegionSet region(limits);
    EXPECT_EQ(region.Add(RectF(0.0f, 0.0f, 4.0f, 10.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_EQ(region.Add(RectF(6.0f, 0.0f, 4.0f, 10.0f)), OcclusionRegionSet::AddResult::LIMIT_REACHED);
    EXPECT_TRUE(region.IsLimitReached());
    EXPECT_FALSE(region.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));
}

TEST(OcclusionRegionTest, OperationAndMemoryLimitsDisablePositiveCoverage)
{
    OcclusionRegionSet::Limits operationLimits;
    operationLimits.maxOperations = 0;
    OcclusionRegionSet operationLimited(operationLimits);
    EXPECT_EQ(operationLimited.Add(RectF(0.0f, 0.0f, 10.0f, 10.0f)), OcclusionRegionSet::AddResult::LIMIT_REACHED);
    EXPECT_FALSE(operationLimited.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));

    OcclusionRegionSet::Limits memoryLimits;
    memoryLimits.maxBytes = 1;
    OcclusionRegionSet memoryLimited(memoryLimits);
    EXPECT_EQ(memoryLimited.Add(RectF(0.0f, 0.0f, 10.0f, 10.0f)), OcclusionRegionSet::AddResult::LIMIT_REACHED);
    EXPECT_FALSE(memoryLimited.Covers(RectF(0.0f, 0.0f, 10.0f, 10.0f)));

    memoryLimits.maxBytes = 2 * 3 * 4 * sizeof(double);
    OcclusionRegionSet twoRectLimit(memoryLimits);
    EXPECT_EQ(twoRectLimit.Add(RectF(0.0f, 0.0f, 2.0f, 2.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_EQ(twoRectLimit.Add(RectF(2.0f, 0.0f, 2.0f, 2.0f)), OcclusionRegionSet::AddResult::ADDED);
    EXPECT_EQ(twoRectLimit.Add(RectF(4.0f, 0.0f, 2.0f, 2.0f)), OcclusionRegionSet::AddResult::LIMIT_REACHED);
    EXPECT_FALSE(twoRectLimit.Covers(RectF(0.0f, 0.0f, 6.0f, 2.0f)));
}

TEST(OcclusionRegionTest, InvalidGeometryCannotOcclude)
{
    OcclusionRegionSet region;
    EXPECT_EQ(region.Add(RectF(0.0f, 0.0f, -1.0f, 1.0f)), OcclusionRegionSet::AddResult::INVALID);
    EXPECT_EQ(region.Add(RectF(std::numeric_limits<float>::quiet_NaN(), 0.0f, 1.0f, 1.0f)),
        OcclusionRegionSet::AddResult::INVALID);
    EXPECT_FALSE(region.Covers(RectF(0.0f, 0.0f, 1.0f, 1.0f)));
    EXPECT_FALSE(region.Covers(RectF(0.0f, 0.0f, 0.0f, 1.0f)));
}

TEST(OcclusionRegionTest, DiscreteCoverageOracle)
{
    constexpr int32_t side = 8;
    std::array<std::array<bool, side>, side> oracle = {};
    OcclusionRegionSet region;
    const std::array<std::array<int32_t, 4>, 5> evidence = { { { 0, 0, 3, 6 }, { 2, 1, 4, 5 }, { 5, 0, 3, 3 },
        { 0, 6, 8, 2 }, { 6, 3, 2, 3 } } };
    for (const auto& rect : evidence) {
        EXPECT_EQ(region.Add(RectF(rect[0], rect[1], rect[2], rect[3])), OcclusionRegionSet::AddResult::ADDED);
        for (int32_t y = rect[1]; y < rect[1] + rect[3]; ++y) {
            for (int32_t x = rect[0]; x < rect[0] + rect[2]; ++x) {
                oracle[y][x] = true;
            }
        }
        for (int32_t top = 0; top < side; ++top) {
            for (int32_t left = 0; left < side; ++left) {
                for (int32_t bottom = top + 1; bottom <= side; ++bottom) {
                    for (int32_t right = left + 1; right <= side; ++right) {
                        if (right - left > 2 || bottom - top > 2) {
                            continue;
                        }
                        bool covered = true;
                        for (int32_t y = top; y < bottom; ++y) {
                            for (int32_t x = left; x < right; ++x) {
                                covered = covered && oracle[y][x];
                            }
                        }
                        EXPECT_EQ(region.Covers(RectF(left, top, right - left, bottom - top)), covered)
                            << "rectangle " << left << "," << top << " to " << right << "," << bottom;
                    }
                }
            }
        }
    }
}

TEST(OcclusionRegionTest, BoundedRandomizedCoverageNeverExceedsCellOracle)
{
    constexpr int32_t side = 12;
    std::array<std::array<bool, side>, side> oracle = {};
    OcclusionRegionSet region;
    uint32_t state = 0x31415926;
    auto next = [&state]() {
        state = state * 1664525u + 1013904223u;
        return state >> 8;
    };
    for (int32_t index = 0; index < 48; ++index) {
        const int32_t left = static_cast<int32_t>(next() % 16) - 2;
        const int32_t top = static_cast<int32_t>(next() % 16) - 2;
        const int32_t width = static_cast<int32_t>(next() % 5) + 1;
        const int32_t height = static_cast<int32_t>(next() % 5) + 1;
        const auto added = region.AddClipped(RectF(left, top, width, height), RectF(0, 0, side, side));
        if (added == OcclusionRegionSet::AddResult::LIMIT_REACHED) {
            break;
        }
        for (int32_t y = std::max(0, top); y < std::min(side, top + height); ++y) {
            for (int32_t x = std::max(0, left); x < std::min(side, left + width); ++x) {
                oracle[y][x] = true;
            }
        }
        for (int32_t query = 0; query < 32; ++query) {
            const int32_t x = static_cast<int32_t>(next() % side);
            const int32_t y = static_cast<int32_t>(next() % side);
            const int32_t right = std::min(side, x + static_cast<int32_t>(next() % 4) + 1);
            const int32_t bottom = std::min(side, y + static_cast<int32_t>(next() % 4) + 1);
            if (!region.Covers(RectF(x, y, right - x, bottom - y))) {
                continue;
            }
            for (int32_t yy = y; yy < bottom; ++yy) {
                for (int32_t xx = x; xx < right; ++xx) {
                    EXPECT_TRUE(oracle[yy][xx]) << "false coverage at " << xx << "," << yy;
                }
            }
        }
    }
    EXPECT_LE(region.GetRectCount(), 256u);
    EXPECT_LE(region.GetOperationCount(), 16384u);
    RecordProperty("retained_rects", static_cast<int>(region.GetRectCount()));
    RecordProperty("operations", static_cast<int>(region.GetOperationCount()));
}

TEST(OcclusionCollectorTest, PaintOrderAndDomainIsolation)
{
    std::vector<OcclusionPaintSnapshot> bottomToTop = {
        { 1, 10, RectF(0.0f, 0.0f, 10.0f, 10.0f), {}, 1.0, true, false, true },
        { 2, 10, RectF(0.0f, 0.0f, 5.0f, 10.0f), RectF(0.0f, 0.0f, 5.0f, 10.0f), 1.0, true, true, true },
        { 3, 10, RectF(5.0f, 0.0f, 5.0f, 10.0f), RectF(5.0f, 0.0f, 5.0f, 10.0f), 1.0, true, true, true },
        { 4, 11, RectF(0.0f, 0.0f, 10.0f, 10.0f), RectF(0.0f, 0.0f, 10.0f, 10.0f), 1.0, true, true, true },
    };
    const auto result = InspectorOcclusionCollector::Evaluate(bottomToTop, 1.0);
    EXPECT_EQ(result.at(1), OcclusionDecision::OCCLUDED);
    EXPECT_EQ(result.at(2), OcclusionDecision::VISIBLE);
    EXPECT_EQ(result.at(3), OcclusionDecision::VISIBLE);
    EXPECT_EQ(result.at(4), OcclusionDecision::VISIBLE);
}

TEST(OcclusionCollectorTest, DuplicateNodeInPaintOrderFallsBack)
{
    std::vector<OcclusionPaintSnapshot> repeated = {
        { 1, 10, RectF(0.0f, 0.0f, 10.0f, 10.0f), {}, 1.0, true, false, true },
        { 2, 10, RectF(0.0f, 0.0f, 10.0f, 10.0f), RectF(0.0f, 0.0f, 10.0f, 10.0f), 1.0, true, true, true },
        { 1, 10, RectF(0.0f, 0.0f, 10.0f, 10.0f), {}, 1.0, true, false, true },
    };
    const auto decisions = InspectorOcclusionCollector::Evaluate(repeated, 1.0);
    EXPECT_EQ(decisions.at(1), OcclusionDecision::UNKNOWN);
    EXPECT_EQ(decisions.at(2), OcclusionDecision::UNKNOWN);
}

TEST(OcclusionCollectorTest, ThresholdAndUnknownEvidence)
{
    std::vector<OcclusionPaintSnapshot> bottomToTop = {
        { 1, 10, RectF(0.0f, 0.0f, 10.0f, 10.0f), {}, 1.0, true, false, true },
        { 2, 10, RectF(0.0f, 0.0f, 10.0f, 10.0f), RectF(0.0f, 0.0f, 10.0f, 10.0f), 0.75, true, true, true },
    };
    EXPECT_EQ(InspectorOcclusionCollector::Evaluate(bottomToTop, 0.75).at(1), OcclusionDecision::OCCLUDED);
    EXPECT_EQ(InspectorOcclusionCollector::Evaluate(bottomToTop, 0.76).at(1), OcclusionDecision::VISIBLE);
    bottomToTop[1].evidenceTrusted = false;
    EXPECT_EQ(InspectorOcclusionCollector::Evaluate(bottomToTop, 0.5).at(1), OcclusionDecision::VISIBLE);
    bottomToTop[1].paintOrderTrusted = false;
    EXPECT_EQ(InspectorOcclusionCollector::Evaluate(bottomToTop, 0.5).at(1), OcclusionDecision::UNKNOWN);
    bottomToTop[1].paintOrderTrusted = true;
    bottomToTop[1].evidenceTrusted = true;
    bottomToTop[1].finalOpacity = 0.0;
    EXPECT_EQ(InspectorOcclusionCollector::Evaluate(bottomToTop, 0.0).at(1), OcclusionDecision::VISIBLE);
}

TEST(OcclusionCollectorTest, FragmentLimitKeepsTarget)
{
    std::vector<OcclusionPaintSnapshot> bottomToTop;
    bottomToTop.push_back({ 1, 10, RectF(0.0f, 0.0f, 600.0f, 1.0f), {}, 1.0, true, false, true });
    for (int32_t index = 0; index < 257; ++index) {
        const float left = static_cast<float>(index * 2);
        bottomToTop.push_back(
            { index + 2, 10, RectF(left, 0.0f, 1.0f, 1.0f), RectF(left, 0.0f, 1.0f, 1.0f), 1.0, true, true, true });
    }
    const auto decisions = InspectorOcclusionCollector::Evaluate(bottomToTop, 1.0);
    EXPECT_EQ(decisions.at(1), OcclusionDecision::UNKNOWN);
}

TEST(OcclusionCollectorTest, PendingRenderTreeFallsBackWithoutRebuild)
{
    auto id = ElementRegister::GetInstance()->MakeUniqueId();
    auto node = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, id, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    node->MarkNeedSyncRenderTree();
    ASSERT_TRUE(node->IsRenderTreeSyncPending());
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(node, 1.0, 1).empty());
    EXPECT_TRUE(node->IsRenderTreeSyncPending());
}

TEST(OcclusionCollectorTest, ExternalSurfaceIsNotStrongOcclusionEvidence)
{
    const std::array<const char*, 5> externalTags = { V2::XCOMPONENT_ETS_TAG, V2::VIDEO_ETS_TAG, V2::FORM_ETS_TAG,
        V2::PLUGIN_ETS_TAG, V2::EMBEDDED_COMPONENT_ETS_TAG };
    for (const auto* tag : externalTags) {
        SCOPED_TRACE(tag);
        auto id = ElementRegister::GetInstance()->MakeUniqueId();
        auto node = FrameNode::CreateFrameNode(tag, id, AceType::MakeRefPtr<Pattern>(), true);
        ASSERT_NE(node, nullptr);
        auto context = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
        ASSERT_NE(context, nullptr);
        context->UpdateBackgroundColor(Color::BLUE);
        context->UpdateClipEdge(true);
        node->ClearNeedSyncRenderTree();
        const auto result = InspectorOcclusionCollector::Collect(node, 1.0, 1);
        ASSERT_EQ(result.count(id), 1u);
        EXPECT_EQ(result.at(id), OcclusionDecision::UNKNOWN);
    }
}

TEST(OcclusionCollectorTest, SyncedZIndexOpacityAndEffects)
{
    auto* ids = ElementRegister::GetInstance();
    auto root = FrameNode::CreateFrameNode("root", ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto top = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto behind =
        FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(root, nullptr);
    ASSERT_NE(top, nullptr);
    ASSERT_NE(behind, nullptr);
    auto rootContext = AceType::DynamicCast<MockRenderContext>(root->GetRenderContext());
    auto topContext = AceType::DynamicCast<MockRenderContext>(top->GetRenderContext());
    auto behindContext = AceType::DynamicCast<MockRenderContext>(behind->GetRenderContext());
    ASSERT_NE(rootContext, nullptr);
    ASSERT_NE(topContext, nullptr);
    ASSERT_NE(behindContext, nullptr);
    rootContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    behindContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->UpdateBackgroundColor(Color::BLUE);
    topContext->UpdateZIndex(10);
    root->SetActive(true);
    top->SetActive(true);
    behind->SetActive(true);
    root->AddChild(top);
    root->AddChild(behind);
    root->MarkNeedSyncRenderTree();
    root->RebuildRenderContextTree();
    top->RebuildRenderContextTree();
    behind->RebuildRenderContextTree();
    ASSERT_EQ(root->GetFrameChildren().size(), 2u);
    auto decisionForBehind = [&](double threshold) {
        const auto result = InspectorOcclusionCollector::Collect(root, threshold, 1);
        const auto it = result.find(behind->GetId());
        EXPECT_NE(it, result.end());
        return it == result.end() ? OcclusionDecision::UNKNOWN : it->second;
    };
    EXPECT_EQ(decisionForBehind(1.0), OcclusionDecision::OCCLUDED);

    rootContext->UpdateOpacity(0.5);
    EXPECT_EQ(decisionForBehind(1.0), OcclusionDecision::VISIBLE);
    EXPECT_EQ(decisionForBehind(0.5), OcclusionDecision::OCCLUDED);
    rootContext->UpdateOpacity(1.0);

    topContext->SetAnimationsCount(1);
    EXPECT_EQ(decisionForBehind(1.0), OcclusionDecision::VISIBLE);
    topContext->SetAnimationsCount(0);
    topContext->UpdateBorderRadius(BorderRadiusProperty());
    EXPECT_EQ(decisionForBehind(1.0), OcclusionDecision::VISIBLE);
}

TEST(OcclusionCollectorTest, ExplicitlyClippedLeafHasBoundedCandidate)
{
    auto* ids = ElementRegister::GetInstance();
    auto root = FrameNode::CreateFrameNode("root", ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto behind =
        FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto top = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(root, nullptr);
    ASSERT_NE(behind, nullptr);
    ASSERT_NE(top, nullptr);
    auto rootContext = AceType::DynamicCast<MockRenderContext>(root->GetRenderContext());
    auto behindContext = AceType::DynamicCast<MockRenderContext>(behind->GetRenderContext());
    auto topContext = AceType::DynamicCast<MockRenderContext>(top->GetRenderContext());
    ASSERT_NE(rootContext, nullptr);
    ASSERT_NE(behindContext, nullptr);
    ASSERT_NE(topContext, nullptr);
    rootContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    behindContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->UpdateBackgroundColor(Color::BLUE);
    root->SetActive(true);
    behind->SetActive(true);
    top->SetActive(true);
    root->AddChild(behind);
    root->AddChild(top);
    root->MarkNeedSyncRenderTree();
    root->RebuildRenderContextTree();
    behind->RebuildRenderContextTree();
    top->RebuildRenderContextTree();

    const auto unbounded = InspectorOcclusionCollector::Collect(root, 1.0, 1);
    ASSERT_EQ(unbounded.count(behind->GetId()), 1u);
    EXPECT_EQ(unbounded.at(behind->GetId()), OcclusionDecision::UNKNOWN);

    behindContext->UpdateClipEdge(true);
    const auto bounded = InspectorOcclusionCollector::Collect(root, 1.0, 1);
    ASSERT_EQ(bounded.count(behind->GetId()), 1u);
    EXPECT_EQ(bounded.at(behind->GetId()), OcclusionDecision::OCCLUDED);
}

TEST(OcclusionCollectorTest, AncestorClipLimitsDescendantEvidence)
{
    auto* ids = ElementRegister::GetInstance();
    auto root = FrameNode::CreateFrameNode("root", ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto behind =
        FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto container = FrameNode::CreateFrameNode("container", ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto top = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(root, nullptr);
    ASSERT_NE(behind, nullptr);
    ASSERT_NE(container, nullptr);
    ASSERT_NE(top, nullptr);
    auto rootContext = AceType::DynamicCast<MockRenderContext>(root->GetRenderContext());
    auto behindContext = AceType::DynamicCast<MockRenderContext>(behind->GetRenderContext());
    auto containerContext = AceType::DynamicCast<MockRenderContext>(container->GetRenderContext());
    auto topContext = AceType::DynamicCast<MockRenderContext>(top->GetRenderContext());
    ASSERT_NE(rootContext, nullptr);
    ASSERT_NE(behindContext, nullptr);
    ASSERT_NE(containerContext, nullptr);
    ASSERT_NE(topContext, nullptr);
    rootContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    behindContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    containerContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 5.0f, 10.0f));
    topContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    containerContext->UpdateClipEdge(true);
    topContext->UpdateBackgroundColor(Color::BLUE);
    root->SetActive(true);
    behind->SetActive(true);
    container->SetActive(true);
    top->SetActive(true);
    root->AddChild(behind);
    root->AddChild(container);
    container->AddChild(top);
    root->MarkNeedSyncRenderTree();
    root->RebuildRenderContextTree();
    behind->RebuildRenderContextTree();
    container->RebuildRenderContextTree();
    top->RebuildRenderContextTree();

    const auto clipped = InspectorOcclusionCollector::Collect(root, 1.0, 1);
    ASSERT_EQ(clipped.count(behind->GetId()), 1u);
    EXPECT_EQ(clipped.at(behind->GetId()), OcclusionDecision::VISIBLE);

    containerContext->UpdateClipEdge(false);
    const auto unclipped = InspectorOcclusionCollector::Collect(root, 1.0, 1);
    ASSERT_EQ(unclipped.count(behind->GetId()), 1u);
    EXPECT_EQ(unclipped.at(behind->GetId()), OcclusionDecision::OCCLUDED);
}

TEST(OcclusionCollectorTest, OverlayAndCrossWindowFallback)
{
    auto* ids = ElementRegister::GetInstance();
    auto root = FrameNode::CreateFrameNode("root", ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto behind =
        FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto overlay =
        FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(root, nullptr);
    ASSERT_NE(behind, nullptr);
    ASSERT_NE(overlay, nullptr);
    auto rootContext = AceType::DynamicCast<MockRenderContext>(root->GetRenderContext());
    auto behindContext = AceType::DynamicCast<MockRenderContext>(behind->GetRenderContext());
    auto overlayContext = AceType::DynamicCast<MockRenderContext>(overlay->GetRenderContext());
    ASSERT_NE(rootContext, nullptr);
    ASSERT_NE(behindContext, nullptr);
    ASSERT_NE(overlayContext, nullptr);
    rootContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    behindContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    overlayContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    overlayContext->UpdateBackgroundColor(Color::BLUE);
    root->SetActive(true);
    behind->SetActive(true);
    overlay->SetActive(true);
    root->AddChild(behind);
    root->SetOverlayNode(overlay);
    root->MarkNeedSyncRenderTree();
    root->RebuildRenderContextTree();
    behind->RebuildRenderContextTree();
    overlay->RebuildRenderContextTree();
    const auto decisions = InspectorOcclusionCollector::Collect(root, 1.0, 1);
    ASSERT_EQ(decisions.count(behind->GetId()), 1u);
    EXPECT_EQ(decisions.at(behind->GetId()), OcclusionDecision::OCCLUDED);

    std::vector<OcclusionPaintSnapshot> crossWindow = {
        { 1, 1, RectF(0.0f, 0.0f, 10.0f, 10.0f), {}, 1.0, true, false, true },
        { 2, 2, RectF(0.0f, 0.0f, 10.0f, 10.0f), RectF(0.0f, 0.0f, 10.0f, 10.0f), 1.0, true, true, true },
    };
    EXPECT_EQ(InspectorOcclusionCollector::Evaluate(crossWindow, 1.0).at(1), OcclusionDecision::VISIBLE);
}

TEST(OcclusionCollectorTest, AncestorUnsupportedPaintKeepsDescendantOcclusion)
{
    auto* ids = ElementRegister::GetInstance();
    auto root = FrameNode::CreateFrameNode("root", ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto behind =
        FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto top = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(root, nullptr);
    ASSERT_NE(behind, nullptr);
    ASSERT_NE(top, nullptr);
    auto rootContext = AceType::DynamicCast<MockRenderContext>(root->GetRenderContext());
    auto behindContext = AceType::DynamicCast<MockRenderContext>(behind->GetRenderContext());
    auto topContext = AceType::DynamicCast<MockRenderContext>(top->GetRenderContext());
    ASSERT_NE(rootContext, nullptr);
    ASSERT_NE(behindContext, nullptr);
    ASSERT_NE(topContext, nullptr);
    rootContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    behindContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->UpdateBackgroundColor(Color::BLUE);
    // An unsupported paint on the ancestor must only disqualify the ancestor itself.
    rootContext->SetAnimationsCount(1);
    root->SetActive(true);
    behind->SetActive(true);
    top->SetActive(true);
    root->AddChild(behind);
    root->AddChild(top);
    root->MarkNeedSyncRenderTree();
    root->RebuildRenderContextTree();
    behind->RebuildRenderContextTree();
    top->RebuildRenderContextTree();

    const auto decisions = InspectorOcclusionCollector::Collect(root, 1.0, 1);
    ASSERT_EQ(decisions.count(behind->GetId()), 1u);
    EXPECT_EQ(decisions.at(behind->GetId()), OcclusionDecision::OCCLUDED);
}

TEST(OcclusionCollectorTest, ScrollContainerKeepsDescendantOcclusion)
{
    auto* ids = ElementRegister::GetInstance();
    auto root = FrameNode::CreateFrameNode("root", ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto scroll =
        FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto behind =
        FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    auto top = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, ids->MakeUniqueId(), AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(root, nullptr);
    ASSERT_NE(scroll, nullptr);
    ASSERT_NE(behind, nullptr);
    ASSERT_NE(top, nullptr);
    auto rootContext = AceType::DynamicCast<MockRenderContext>(root->GetRenderContext());
    auto scrollContext = AceType::DynamicCast<MockRenderContext>(scroll->GetRenderContext());
    auto behindContext = AceType::DynamicCast<MockRenderContext>(behind->GetRenderContext());
    auto topContext = AceType::DynamicCast<MockRenderContext>(top->GetRenderContext());
    ASSERT_NE(rootContext, nullptr);
    ASSERT_NE(scrollContext, nullptr);
    ASSERT_NE(behindContext, nullptr);
    ASSERT_NE(topContext, nullptr);
    rootContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    scrollContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    behindContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 10.0f, 10.0f));
    topContext->UpdateBackgroundColor(Color::BLUE);
    root->SetActive(true);
    scroll->SetActive(true);
    behind->SetActive(true);
    top->SetActive(true);
    root->AddChild(scroll);
    scroll->AddChild(behind);
    scroll->AddChild(top);
    root->MarkNeedSyncRenderTree();
    root->RebuildRenderContextTree();
    scroll->RebuildRenderContextTree();
    behind->RebuildRenderContextTree();
    top->RebuildRenderContextTree();

    const auto decisions = InspectorOcclusionCollector::Collect(root, 1.0, 1);
    ASSERT_EQ(decisions.count(behind->GetId()), 1u);
    EXPECT_EQ(decisions.at(behind->GetId()), OcclusionDecision::OCCLUDED);
}

} // namespace

namespace {
constexpr int32_t MAX_SNAPSHOTS = 4096;
constexpr int32_t MAX_DOMAINS = 16;
constexpr int32_t MAX_DEPTH = 128;
const double INF = std::numeric_limits<double>::infinity();

RefPtr<FrameNode> MakeNode(const std::string& tag = V2::BLANK_ETS_TAG)
{
    return FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>(), true);
}

void SyncNode(const RefPtr<FrameNode>& node)
{
    node->SetActive(true);
    node->ClearNeedSyncRenderTree();
}

OcclusionPaintSnapshot TrustedSnapshot(int32_t nodeId, int32_t domainId, double finalOpacity = 1.0)
{
    OcclusionPaintSnapshot snapshot;
    snapshot.nodeId = nodeId;
    snapshot.domainId = domainId;
    snapshot.candidateEnvelope = RectF(0.0f, 0.0f, 10.0f, 10.0f);
    snapshot.opaqueEvidence = RectF(0.0f, 0.0f, 10.0f, 10.0f);
    snapshot.finalOpacity = finalOpacity;
    snapshot.paintOrderTrusted = true;
    snapshot.candidateTrusted = true;
    snapshot.evidenceTrusted = true;
    return snapshot;
}
} // namespace

/**
 * @tc.name: EvaluateRejectsInvalidThreshold
 * @tc.desc: Evaluate rejects non-finite and out-of-range thresholds.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, EvaluateRejectsInvalidThreshold)
{
    /**
     * @tc.steps: evaluate with NaN / infinite / out-of-range thresholds.
     * @tc.expected: non-finite or out-of-range thresholds produce empty decisions.
     */
    std::vector<OcclusionPaintSnapshot> paintOrder = { TrustedSnapshot(1, 10) };
    EXPECT_TRUE(InspectorOcclusionCollector::Evaluate(paintOrder, std::nan("")).empty());
    EXPECT_TRUE(InspectorOcclusionCollector::Evaluate(paintOrder, INF).empty());
    EXPECT_TRUE(InspectorOcclusionCollector::Evaluate(paintOrder, -0.1).empty());
    EXPECT_TRUE(InspectorOcclusionCollector::Evaluate(paintOrder, 1.5).empty());
    EXPECT_FALSE(InspectorOcclusionCollector::Evaluate(paintOrder, 1.0).empty());
}

/**
 * @tc.name: EvaluateRejectsOversizedPaintOrder
 * @tc.desc: Evaluate rejects paint order beyond the snapshot budget.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, EvaluateRejectsOversizedPaintOrder)
{
    /**
     * @tc.steps: evaluate with a paint order beyond the snapshot budget.
     * @tc.expected: oversized input is rejected with empty decisions.
     */
    std::vector<OcclusionPaintSnapshot> paintOrder;
    paintOrder.reserve(static_cast<size_t>(MAX_SNAPSHOTS) + 1);
    for (int32_t i = 0; i <= MAX_SNAPSHOTS; ++i) {
        paintOrder.push_back(TrustedSnapshot(i, 10));
    }
    EXPECT_TRUE(InspectorOcclusionCollector::Evaluate(paintOrder, 1.0).empty());
}

/**
 * @tc.name: EvaluateDomainBudgetFallsBackToUnknown
 * @tc.desc: Evaluate falls back to UNKNOWN beyond the domain budget.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, EvaluateDomainBudgetFallsBackToUnknown)
{
    /**
     * @tc.steps: feed snapshots of more distinct domains than the budget.
     * @tc.expected: the first domains decide normally; domains beyond the
     *               budget resolve to UNKNOWN.
     */
    std::vector<OcclusionPaintSnapshot> paintOrder;
    for (int32_t domain = 0; domain <= MAX_DOMAINS; ++domain) {
        paintOrder.push_back(TrustedSnapshot(domain, domain));
    }
    const auto decisions = InspectorOcclusionCollector::Evaluate(paintOrder, 1.0);
    EXPECT_EQ(decisions.size(), static_cast<size_t>(MAX_DOMAINS) + 1);
    // Evaluate walks paintOrder in reverse, so the highest domain is seen first.
    EXPECT_EQ(decisions.at(0), OcclusionDecision::UNKNOWN);
    EXPECT_EQ(decisions.at(MAX_DOMAINS), OcclusionDecision::VISIBLE);
}

/**
 * @tc.name: EvaluateNaNFinalOpacitySkipsEvidence
 * @tc.desc: Evaluate skips evidence with NaN final opacity.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, EvaluateNaNFinalOpacitySkipsEvidence)
{
    /**
     * @tc.steps: mark an evidence snapshot with NaN final opacity.
     * @tc.expected: the NaN check drops the evidence; later nodes in the same
     *               domain are not covered by it.
     */
    std::vector<OcclusionPaintSnapshot> paintOrder = {
        TrustedSnapshot(1, 10, std::nan("")),
        TrustedSnapshot(2, 10),
    };
    const auto decisions = InspectorOcclusionCollector::Evaluate(paintOrder, 1.0);
    EXPECT_EQ(decisions.at(1), OcclusionDecision::OCCLUDED); // no evidence added, node stays visible
    EXPECT_EQ(decisions.at(2), OcclusionDecision::VISIBLE);
}

/**
 * @tc.name: CollectRejectsNullNodeAndInvalidThreshold
 * @tc.desc: Collect rejects null node and non-finite threshold.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, CollectRejectsNullNodeAndInvalidThreshold)
{
    /**
     * @tc.steps: collect with a null node and a non-finite threshold.
     * @tc.expected: both guard branches produce empty decisions.
     */
    RefPtr<FrameNode> nullNode;
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(nullNode, 1.0, 1).empty());
    auto node = MakeNode();
    SyncNode(node);
    auto context = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
    ASSERT_NE(context, nullptr);
    context->UpdateBackgroundColor(Color::BLUE);
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(node, std::nan(""), 1).empty());
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(node, INF, 1).empty());
}

/**
 * @tc.name: CollectStopsAtDepthBudget
 * @tc.desc: Collect stops at the depth budget.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, CollectStopsAtDepthBudget)
{
    /**
     * @tc.steps: build a chain deeper than the traversal budget.
     * @tc.expected: the depth budget aborts collection with empty decisions.
     */
    auto parent = MakeNode();
    SyncNode(parent);
    auto current = parent;
    for (int32_t i = 0; i < MAX_DEPTH + 3; ++i) {
        auto child = MakeNode();
        SyncNode(child);
        current->AddChild(child);
        current = child;
    }
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(parent, 1.0, 1).empty());
}

/**
 * @tc.name: CollectStopsAtChildWindowBoundary
 * @tc.desc: Collect stops at a child window boundary.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, CollectStopsAtChildWindowBoundary)
{
    /**
     * @tc.steps: mark a non-root node as a window boundary.
     * @tc.expected: the subtree walk stops at the boundary child.
     */
    auto parent = MakeNode();
    auto child = MakeNode();
    SyncNode(parent);
    SyncNode(child);
    parent->AddChild(child);
    child->SetWindowBoundary(true);
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(parent, 1.0, 1).empty());
}

/**
 * @tc.name: CollectStopsAtNaNOpacity
 * @tc.desc: Collect stops at non-finite opacity on node or parent.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, CollectStopsAtNaNOpacity)
{
    /**
     * @tc.steps: set NaN opacity on the start node and on a parent.
     * @tc.expected: both non-finite opacity guards abort collection.
     */
    auto parent = MakeNode();
    auto child = MakeNode();
    SyncNode(parent);
    SyncNode(child);
    parent->AddChild(child);

    auto childContext = AceType::DynamicCast<MockRenderContext>(child->GetRenderContext());
    ASSERT_NE(childContext, nullptr);
    childContext->UpdateOpacity(std::nan(""));
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(parent, 1.0, 1).empty());

    // restore child opacity, poison the parent opacity instead: the child's
    // inherited parent opacity check fails
    childContext->UpdateOpacity(1.0);
    auto parentContext = AceType::DynamicCast<MockRenderContext>(parent->GetRenderContext());
    ASSERT_NE(parentContext, nullptr);
    parentContext->UpdateOpacity(std::nan(""));
    EXPECT_TRUE(InspectorOcclusionCollector::Collect(child, 1.0, 1).empty());
}

/**
 * @tc.name: CollectRemainingExternalTagsStayUnknown
 * @tc.desc: Collect keeps remaining external tags UNKNOWN.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, CollectRemainingExternalTagsStayUnknown)
{
    /**
     * @tc.steps: collect leaf nodes with the four tag families missing from
     *            the existing external-tag test.
     * @tc.expected: all of them resolve to UNKNOWN.
     */
    const std::array<const char*, 4> externalTags = { V2::WEB_ETS_TAG, V2::UI_EXTENSION_COMPONENT_TAG,
        V2::UI_EXTENSION_SURFACE_TAG, V2::UI_EXTENSION_COMPONENT_ETS_TAG };
    for (const auto* tag : externalTags) {
        SCOPED_TRACE(tag);
        auto node = MakeNode(tag);
        SyncNode(node);
        auto context = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
        ASSERT_NE(context, nullptr);
        context->UpdateBackgroundColor(Color::BLUE);
        context->UpdateClipEdge(true);
        const auto result = InspectorOcclusionCollector::Collect(node, 1.0, 1);
        ASSERT_EQ(result.count(node->GetId()), 1u);
        EXPECT_EQ(result.at(node->GetId()), OcclusionDecision::UNKNOWN);
    }
}

/**
 * @tc.name: UnsupportedGeometryMarksEvidenceUntrusted
 * @tc.desc: Unsupported geometry marks evidence untrusted.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, UnsupportedGeometryMarksEvidenceUntrusted)
{
    /**
     * @tc.steps: attach each unsupported geometry decoration to a plain leaf.
     * @tc.expected: unsupported geometry keeps the snapshot untrusted so the
     *               node resolves to UNKNOWN.
     */
    auto applyUnsupported = [](const RefPtr<FrameNode>& node, int variant) {
        auto context = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
        switch (variant) {
            case 0:
                context->UpdateTransformMatrix(Matrix4::CreateIdentity());
                break;
            case 1:
                context->UpdateTransform3DMatrix(Matrix4::CreateIdentity());
                break;
            case 2:
                context->UpdateClipShape(AceType::MakeRefPtr<BasicShape>());
                break;
            case 3:
                context->UpdateProgressMask(AceType::MakeRefPtr<ProgressMaskProperty>());
                break;
            default:
                break;
        }
    };
    for (int variant = 0; variant < 4; ++variant) {
        auto node = MakeNode();
        SyncNode(node);
        auto context = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
        ASSERT_NE(context, nullptr);
        context->UpdateBackgroundColor(Color::BLUE);
        context->UpdateClipEdge(true);
        applyUnsupported(node, variant);
        const auto result = InspectorOcclusionCollector::Collect(node, 1.0, 1);
        ASSERT_EQ(result.count(node->GetId()), 1u);
        EXPECT_EQ(result.at(node->GetId()), OcclusionDecision::UNKNOWN);
    }
}

/**
 * @tc.name: AncestorDisqualifiersAbortCollection
 * @tc.desc: Ancestor disqualifiers abort collection.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, AncestorDisqualifiersAbortCollection)
{
    /**
     * @tc.steps: build a leaf under a mid ancestor and trip every ancestor
     *            disqualifier one at a time.
     * @tc.expected: pending sync, geometry transition, transition animator and
     *               external tag ancestors all abort the ancestor walk.
     */
    auto makeChain = [](const std::string& midTag = V2::BLANK_ETS_TAG) {
        auto root = MakeNode("root");
        auto mid = MakeNode(midTag);
        auto leaf = MakeNode();
        SyncNode(root);
        SyncNode(mid);
        SyncNode(leaf);
        root->AddChild(mid);
        mid->AddChild(leaf);
        return std::tuple { root, mid, leaf };
    };

    {
        auto [root, mid, leaf] = makeChain();
        mid->MarkNeedSyncRenderTree();
        EXPECT_TRUE(InspectorOcclusionCollector::Collect(leaf, 1.0, 1).empty());
    }
    {
        auto [root, mid, leaf] = makeChain();
        mid->SetIsGeometryTransitionIn(true);
        EXPECT_TRUE(InspectorOcclusionCollector::Collect(leaf, 1.0, 1).empty());
    }
    {
        auto [root, mid, leaf] = makeChain();
        mid->SetIsUseTransitionAnimator(true);
        EXPECT_TRUE(InspectorOcclusionCollector::Collect(leaf, 1.0, 1).empty());
    }
    {
        auto [root, mid, leaf] = makeChain(V2::WEB_ETS_TAG);
        EXPECT_TRUE(InspectorOcclusionCollector::Collect(leaf, 1.0, 1).empty());
    }
    {
        auto [root, mid, leaf] = makeChain();
        auto midContext = AceType::DynamicCast<MockRenderContext>(mid->GetRenderContext());
        ASSERT_NE(midContext, nullptr);
        midContext->UpdateOpacity(std::nan(""));
        EXPECT_TRUE(InspectorOcclusionCollector::Collect(leaf, 1.0, 1).empty());
    }
}

/**
 * @tc.name: AncestorClipAccumulationSucceeds
 * @tc.desc: Ancestor clip and boundary accumulation succeeds.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, AncestorClipAccumulationSucceeds)
{
    /**
     * @tc.steps: walk from a leaf through clipping and boundary ancestors.
     * @tc.expected: the ancestor walk accumulates the clip, the boundary break
     *               stops the walk and collection still reports the leaf.
     */
    {
        // clipped ancestor + root tag ancestor: clip branches accumulate
        auto root = MakeNode(V2::ROOT_ETS_TAG);
        auto mid = MakeNode();
        auto leaf = MakeNode();
        root->AddChild(mid);
        mid->AddChild(leaf);
        SyncNode(root);
        SyncNode(mid);
        SyncNode(leaf);
        auto midContext = AceType::DynamicCast<MockRenderContext>(mid->GetRenderContext());
        ASSERT_NE(midContext, nullptr);
        midContext->UpdateClipEdge(true);
        midContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, 8.0f, 8.0f));
        auto leafContext = AceType::DynamicCast<MockRenderContext>(leaf->GetRenderContext());
        ASSERT_NE(leafContext, nullptr);
        leafContext->UpdateBackgroundColor(Color::BLUE);
        const auto result = InspectorOcclusionCollector::Collect(leaf, 1.0, 1);
        EXPECT_EQ(result.count(leaf->GetId()), 1u);
    }
    {
        // ancestor marked as window boundary ends the walk and still collects
        auto root = MakeNode();
        auto mid = MakeNode();
        auto leaf = MakeNode();
        root->AddChild(mid);
        mid->AddChild(leaf);
        SyncNode(root);
        SyncNode(mid);
        SyncNode(leaf);
        root->SetWindowBoundary(true);
        auto leafContext = AceType::DynamicCast<MockRenderContext>(leaf->GetRenderContext());
        ASSERT_NE(leafContext, nullptr);
        leafContext->UpdateBackgroundColor(Color::BLUE);
        const auto result = InspectorOcclusionCollector::Collect(leaf, 1.0, 1);
        EXPECT_EQ(result.count(leaf->GetId()), 1u);
    }
    {
        // start node itself is a window boundary: the ancestor walk short-circuits
        auto leaf = MakeNode();
        SyncNode(leaf);
        leaf->SetWindowBoundary(true);
        auto leafContext = AceType::DynamicCast<MockRenderContext>(leaf->GetRenderContext());
        ASSERT_NE(leafContext, nullptr);
        leafContext->UpdateBackgroundColor(Color::BLUE);
        const auto result = InspectorOcclusionCollector::Collect(leaf, 1.0, 1);
        EXPECT_EQ(result.count(leaf->GetId()), 1u);
    }
}

/**
 * @tc.name: BlankColorEvidenceVariants
 * @tc.desc: Blank color evidence variants.
 * @tc.type: FUNC
 */
TEST(OcclusionCollectorGapTest, BlankColorEvidenceVariants)
{
    /**
     * @tc.steps: run real BlankPattern leaves with a full-alpha color, a
     *            zero-alpha color and without any color.
     * @tc.expected: full alpha paints evidence; zero alpha and missing color
     *               keep the snapshot without evidence.
     */
    auto makeBlank = []() {
        auto node = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<BlankPattern>(), true);
        SyncNode(node);
        return node;
    };

    {
        auto blank = makeBlank();
        auto property = blank->GetPaintProperty<BlankPaintProperty>();
        ASSERT_NE(property, nullptr);
        property->UpdateColor(Color::BLUE);
        const auto result = InspectorOcclusionCollector::Collect(blank, 1.0, 1);
        EXPECT_EQ(result.count(blank->GetId()), 1u);
    }
    {
        auto blank = makeBlank();
        auto property = blank->GetPaintProperty<BlankPaintProperty>();
        ASSERT_NE(property, nullptr);
        property->UpdateColor(Color::TRANSPARENT);
        const auto result = InspectorOcclusionCollector::Collect(blank, 1.0, 1);
        EXPECT_EQ(result.count(blank->GetId()), 1u);
    }
    {
        auto blank = makeBlank();
        auto property = blank->GetPaintProperty<BlankPaintProperty>();
        ASSERT_NE(property, nullptr);
        EXPECT_FALSE(property->HasColor());
        const auto result = InspectorOcclusionCollector::Collect(blank, 1.0, 1);
        EXPECT_EQ(result.count(blank->GetId()), 1u);
    }
}
} // namespace OHOS::Ace::NG
