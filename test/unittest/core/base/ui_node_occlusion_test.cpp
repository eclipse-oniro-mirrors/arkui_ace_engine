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

} // namespace
} // namespace OHOS::Ace::NG
