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

#include "core/components_ng/manager/page_scene/split_scene_recognizer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <optional>
#include <set>
#include <tuple>

#include "base/utils/utils.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t EARLY_EXIT_INVALID_WINDOW = 1;
constexpr uint32_t EARLY_EXIT_NAVIGATION = 2;
constexpr uint32_t EARLY_EXIT_NOT_ENOUGH_CANDIDATES = 3;
constexpr uint32_t EARLY_EXIT_WIDTH = 4;
constexpr size_t SINGLE = 1;
constexpr size_t MULTIPLE = 2;
constexpr size_t WIDTH_STATE_COUNT = MULTIPLE + 1;
constexpr size_t RECT_EDGE_COUNT = 4;
constexpr size_t EVENTS_PER_RECT = 2;
constexpr float CENTER_OFFSET_RATIO = 0.5f;
constexpr double UNREACHABLE_WIDTH = -std::numeric_limits<double>::infinity();

struct CandidateRect {
    int32_t nodeId;
    RectF rect;
    size_t snapshotIndex;
};

struct BandEvent {
    float y;
    size_t index;
    bool entering;
};

struct PreparedGeometry {
    std::vector<CandidateRect> rects;
    std::vector<CandidateRect> sortedRects;
    std::vector<size_t> predecessors;
    std::vector<float> bandTops;
    std::vector<BandEvent> events;
};

struct CachedMatch {
    SplitSceneRuleSpec rule;
    SplitSceneMatchState match;
};

struct RecognitionSnapshot {
    RecognitionSnapshot(const std::vector<SplitSceneCandidate>& nodes, const RectF& windowRect)
        : candidates(nodes), window(windowRect), clipped(nodes.size())
    {}

    const RectF& Clip(size_t index)
    {
        if (!clipped[index]) {
            clipped[index] = SplitSceneRecognizer::ClipToWindow(candidates[index].rect, window);
            ++stats.rectClips;
        }
        return *clipped[index];
    }

    PreparedGeometry& Prepare(std::vector<CandidateRect> rects)
    {
        for (auto& prepared : geometry) {
            if (prepared.rects.size() == rects.size() &&
                std::equal(rects.begin(), rects.end(), prepared.rects.begin(),
                    [](const auto& a, const auto& b) { return a.snapshotIndex == b.snapshotIndex; })) {
                return prepared;
            }
        }
        geometry.emplace_back();
        geometry.back().rects = std::move(rects);
        return geometry.back();
    }

    const std::vector<SplitSceneCandidate>& candidates;
    RectF window;
    std::vector<std::optional<RectF>> clipped;
    std::vector<PreparedGeometry> geometry;
    std::vector<CachedMatch> matches;
    SplitSceneBatchStats stats;
};

bool IsFiniteRect(const RectF& rect)
{
    const std::array<float, RECT_EDGE_COUNT> edges { rect.Left(), rect.Top(), rect.Right(), rect.Bottom() };
    return rect.Width() > 0.0f && rect.Height() > 0.0f &&
           std::all_of(edges.begin(), edges.end(), [](float edge) { return std::isfinite(edge); });
}

bool IsCandidateVisible(const SplitSceneCandidate& candidate)
{
    return candidate.visible && candidate.active && !candidate.fullyCovered;
}

bool IsSplitNavigation(const SplitSceneCandidate& candidate)
{
    return candidate.navigation && candidate.navigationSplit && candidate.navBarVisible;
}

bool IsNavigationCandidate(const SplitSceneRuleSpec& rule, const SplitSceneCandidate& candidate)
{
    return rule.allowNavigation && IsSplitNavigation(candidate) && IsCandidateVisible(candidate);
}

struct WidthChoice {
    double width = UNREACHABLE_WIDTH;
    size_t last = 0;
    size_t previousRow = 0;
    size_t previousState = 0;
};
using WidthChoices = std::vector<std::array<WidthChoice, WIDTH_STATE_COUNT>>;

std::vector<int32_t> BuildMatchedIds(
    const std::vector<CandidateRect>& rects, const WidthChoices& best, WidthChoice choice)
{
    std::vector<int32_t> ids;
    while (choice.width > 0.0) {
        ids.emplace_back(rects[choice.last].nodeId);
        if (choice.previousState == 0) {
            break;
        }
        choice = best[choice.previousRow][choice.previousState];
    }
    std::reverse(ids.begin(), ids.end());
    return ids;
}

std::vector<size_t> BuildPredecessors(const std::vector<CandidateRect>& rects)
{
    std::vector<float> ends;
    std::vector<size_t> predecessors;
    ends.reserve(rects.size());
    predecessors.reserve(rects.size());
    for (const auto& item : rects) {
        // Exact edge contact is allowed; positive X intersection is not.
        auto previous = std::upper_bound(ends.begin(), ends.end(), item.rect.Left());
        predecessors.emplace_back(static_cast<size_t>(previous - ends.begin()));
        ends.emplace_back(item.rect.Right());
    }
    return predecessors;
}

bool IntersectsBand(const RectF& rect, float y)
{
    return rect.Top() <= y && rect.Bottom() > y;
}

void UpdateWidthChoice(
    const std::vector<CandidateRect>& rects, const std::vector<size_t>& predecessors, size_t index, WidthChoices& best)
{
    const auto width = rects[index].rect.Width();
    auto& current = best[index + 1];
    if (width > current[SINGLE].width) {
        current[SINGLE] = { width, index, 0, 0 };
    }
    const auto prefix = predecessors[index];
    const auto state = best[prefix][MULTIPLE].width > best[prefix][SINGLE].width ? MULTIPLE : SINGLE;
    const auto combined = width + best[prefix][state].width;
    // An inactive predecessor prefix remains unreachable, even when prefix > 0.
    if (combined > current[MULTIPLE].width) {
        current[MULTIPLE] = { combined, index, prefix, state };
    }
}

std::vector<int32_t> FindNonOverlapping(const std::vector<CandidateRect>& rects,
    const std::vector<size_t>& predecessors, double requiredWidth, float y, WidthChoices& best)
{
    std::fill(best.begin(), best.end(), std::array<WidthChoice, WIDTH_STATE_COUNT> {});
    for (size_t index = 0; index < rects.size(); ++index) {
        best[index + 1] = best[index];
        if (IntersectsBand(rects[index].rect, y)) {
            UpdateWidthChoice(rects, predecessors, index, best);
        }
        if (best[index + 1][MULTIPLE].width > requiredWidth) {
            return BuildMatchedIds(rects, best, best[index + 1][MULTIPLE]);
        }
    }
    return {};
}

struct BandSummary {
    size_t count = 0;
    double width = 0.0;
    float minCenter = std::numeric_limits<float>::infinity();
    float maxCenter = -std::numeric_limits<float>::infinity();
};

BandSummary SummarizeBand(const std::vector<CandidateRect>& rects, float y)
{
    BandSummary summary;
    for (const auto& item : rects) {
        if (IntersectsBand(item.rect, y)) {
            const auto center = item.rect.Left() + item.rect.Width() * CENTER_OFFSET_RATIO;
            summary.minCenter = std::min(summary.minCenter, center);
            summary.maxCenter = std::max(summary.maxCenter, center);
            ++summary.count;
            summary.width += item.rect.Width();
        }
    }
    return summary;
}

std::vector<float> GetBandTops(const std::vector<CandidateRect>& rects)
{
    std::vector<float> tops;
    tops.reserve(rects.size());
    for (const auto& item : rects) {
        tops.emplace_back(item.rect.Top());
    }
    std::sort(tops.begin(), tops.end());
    tops.erase(std::unique(tops.begin(), tops.end()), tops.end());
    return tops;
}

bool HasCommonBand(const std::vector<CandidateRect>& rects, float top)
{
    return std::all_of(rects.begin(), rects.end(), [top](const auto& item) { return item.rect.Bottom() > top; });
}

std::vector<int32_t> CollectBandIds(const std::vector<CandidateRect>& rects, float y)
{
    std::vector<int32_t> ids;
    for (const auto& item : rects) {
        if (IntersectsBand(item.rect, y)) {
            ids.emplace_back(item.nodeId);
        }
    }
    return ids;
}

bool CanMatchDisjointBand(const BandSummary& summary, double requiredWidth)
{
    return summary.count >= MULTIPLE && summary.width > requiredWidth;
}

std::vector<int32_t> SearchDisjointBands(const std::vector<CandidateRect>& rects,
    const std::vector<size_t>& predecessors, const std::vector<float>& tops, double requiredWidth, WidthChoices& best)
{
    for (auto y : tops) {
        if (!CanMatchDisjointBand(SummarizeBand(rects, y), requiredWidth)) {
            continue;
        }
        auto ids = FindNonOverlapping(rects, predecessors, requiredWidth, y, best);
        if (!ids.empty()) {
            return ids;
        }
    }
    return {};
}

std::vector<int32_t> FindDisjointBand(
    PreparedGeometry& geometry, double requiredWidth, float commonTop, SplitSceneBatchStats& stats)
{
    auto& rects = geometry.sortedRects;
    if (rects.empty()) {
        rects = geometry.rects;
        std::sort(rects.begin(), rects.end(), [](const auto& left, const auto& right) {
            return std::make_tuple(left.rect.Right(), left.rect.Left(), left.nodeId) <
                   std::make_tuple(right.rect.Right(), right.rect.Left(), right.nodeId);
        });
        geometry.predecessors = BuildPredecessors(rects);
        ++stats.geometrySorts;
    }
    WidthChoices best(rects.size() + 1);
    if (HasCommonBand(rects, commonTop)) {
        return FindNonOverlapping(rects, geometry.predecessors, requiredWidth, commonTop, best);
    }
    if (geometry.bandTops.empty()) {
        geometry.bandTops = GetBandTops(rects);
        ++stats.geometrySorts;
    }
    return SearchDisjointBands(rects, geometry.predecessors, geometry.bandTops, requiredWidth, best);
}

struct ActiveBand {
    std::set<size_t> indices;
    std::multiset<float> centers;
    double width = 0.0;
};

std::vector<BandEvent> BuildBandEvents(const std::vector<CandidateRect>& rects)
{
    std::vector<BandEvent> events;
    events.reserve(rects.size() * EVENTS_PER_RECT);
    for (size_t index = 0; index < rects.size(); ++index) {
        events.push_back({ rects[index].rect.Top(), index, true });
        events.push_back({ rects[index].rect.Bottom(), index, false });
    }
    std::sort(events.begin(), events.end(), [](const auto& left, const auto& right) {
        return std::tie(left.y, left.entering, left.index) < std::tie(right.y, right.entering, right.index);
    });
    return events;
}

void UpdateActiveBand(const std::vector<CandidateRect>& rects, const BandEvent& event, ActiveBand& band)
{
    const auto& rect = rects[event.index].rect;
    const auto center = rect.Left() + rect.Width() * CENTER_OFFSET_RATIO;
    if (event.entering) {
        band.indices.insert(event.index);
        band.centers.insert(center);
        band.width += rect.Width();
    } else {
        band.indices.erase(event.index);
        band.centers.erase(band.centers.find(center));
        band.width -= rect.Width();
    }
}

size_t ApplyBandEvents(
    const std::vector<CandidateRect>& rects, const std::vector<BandEvent>& events, size_t index, ActiveBand& band)
{
    const auto y = events[index].y;
    while (index < events.size() && events[index].y == y) {
        UpdateActiveBand(rects, events[index], band);
        ++index;
    }
    return index;
}

bool CanMatchActiveBand(const ActiveBand& band, double requiredWidth)
{
    return band.indices.size() >= MULTIPLE && band.width > requiredWidth &&
           !NearEqual(*band.centers.begin(), *band.centers.rbegin());
}

std::vector<int32_t> CollectActiveIds(const std::vector<CandidateRect>& rects, const ActiveBand& band)
{
    std::vector<int32_t> ids;
    for (auto index : band.indices) {
        ids.emplace_back(rects[index].nodeId);
    }
    return ids;
}

std::vector<int32_t> FindOverlappingBand(PreparedGeometry& geometry, double requiredWidth, SplitSceneBatchStats& stats)
{
    const auto& rects = geometry.rects;
    if (geometry.events.empty()) {
        geometry.events = BuildBandEvents(rects);
        ++stats.geometrySorts;
    }
    const auto& events = geometry.events;
    ActiveBand band;
    size_t index = 0;
    while (index < events.size()) {
        index = ApplyBandEvents(rects, events, index, band);
        if (CanMatchActiveBand(band, requiredWidth)) {
            return CollectActiveIds(rects, band);
        }
    }
    return {};
}

std::vector<int32_t> MatchCommonOverlappingBand(const std::vector<CandidateRect>& rects, double requiredWidth, float y)
{
    const auto summary = SummarizeBand(rects, y);
    if (summary.count >= MULTIPLE && summary.width > requiredWidth &&
        !NearEqual(summary.minCenter, summary.maxCenter)) {
        return CollectBandIds(rects, y);
    }
    return {};
}

std::vector<int32_t> FindHorizontalGroup(
    PreparedGeometry& geometry, double requiredWidth, bool nonOverlapping, SplitSceneBatchStats& stats)
{
    const auto& rects = geometry.rects;
    const auto totalWidth = std::accumulate(
        rects.begin(), rects.end(), 0.0, [](double width, const auto& item) { return width + item.rect.Width(); });
    if (totalWidth <= requiredWidth) {
        return {};
    }
    const auto highest = std::max_element(rects.begin(), rects.end(),
        [](const auto& left, const auto& right) { return left.rect.Top() < right.rect.Top(); });
    const auto commonTop = highest->rect.Top();
    if (nonOverlapping) {
        return FindDisjointBand(geometry, requiredWidth, commonTop, stats);
    }
    if (HasCommonBand(rects, commonTop)) {
        return MatchCommonOverlappingBand(rects, requiredWidth, commonTop);
    }
    return FindOverlappingBand(geometry, requiredWidth, stats);
}

bool ShouldVisitCandidate(const SplitSceneRuleSpec& rule, const SplitSceneCandidate& candidate)
{
    return rule.allowNavigation || SplitSceneRecognizer::IsCustomCandidate(rule, candidate);
}

bool MatchesGeometry(const SplitSceneRuleSpec& rule, const RectF& rect, const RectF& window)
{
    return IsFiniteRect(rect) && rect.Height() > window.Height() * rule.heightRatioGt &&
           rect.Width() <= window.Width() * rule.widthRatioLte;
}

void CollectCustomRect(const SplitSceneRuleSpec& rule, size_t index, RecognitionSnapshot& snapshot,
    std::vector<CandidateRect>& rects, SplitSceneMatchState& state)
{
    const auto& candidate = snapshot.candidates[index];
    // ShouldVisitCandidate already checks custom eligibility when Navigation is disabled.
    if (rule.allowNavigation && !SplitSceneRecognizer::IsCustomCandidate(rule, candidate)) {
        return;
    }
    const auto& clipped = snapshot.Clip(index);
    if (!MatchesGeometry(rule, clipped, snapshot.window)) {
        ++state.filteredCandidates;
        return;
    }
    rects.push_back({ candidate.nodeId, clipped, index });
}

void SetNavigationMatch(const SplitSceneCandidate& candidate, SplitSceneMatchState& state)
{
    state.matched = true;
    state.splitType = "Navigation";
    state.matchedNodeIds = { candidate.nodeId };
    state.earlyExitStage = EARLY_EXIT_NAVIGATION;
}

bool VisitCandidate(const SplitSceneRuleSpec& rule, size_t index, RecognitionSnapshot& snapshot,
    std::vector<CandidateRect>& rects, SplitSceneMatchState& state)
{
    const auto& candidate = snapshot.candidates[index];
    if (!ShouldVisitCandidate(rule, candidate)) {
        return false;
    }
    ++state.visitedCandidates;
    if (IsNavigationCandidate(rule, candidate) && IsFiniteRect(snapshot.Clip(index))) {
        SetNavigationMatch(candidate, state);
        return true;
    }
    CollectCustomRect(rule, index, snapshot, rects, state);
    return false;
}

void MatchCustomRects(const SplitSceneRuleSpec& rule, std::vector<CandidateRect> rects, RecognitionSnapshot& snapshot,
    SplitSceneMatchState& state)
{
    if (rects.size() < MULTIPLE) {
        state.earlyExitStage = EARLY_EXIT_NOT_ENOUGH_CANDIDATES;
        return;
    }
    auto& geometry = snapshot.Prepare(std::move(rects));
    state.matchedNodeIds = FindHorizontalGroup(
        geometry, snapshot.window.Width() * rule.totalWidthRatioGt, rule.nonOverlapping, snapshot.stats);
    if (state.matchedNodeIds.empty()) {
        state.earlyExitStage = EARLY_EXIT_WIDTH;
        return;
    }
    state.matched = true;
    state.splitType = "Custom";
}

bool SameRule(const SplitSceneRuleSpec& left, const SplitSceneRuleSpec& right)
{
    const auto leftFields = std::tie(left.compNames, left.allowNavigation, left.heightRatioGt, left.widthRatioLte,
        left.totalWidthRatioGt, left.nonOverlapping);
    const auto rightFields = std::tie(right.compNames, right.allowNavigation, right.heightRatioGt, right.widthRatioLte,
        right.totalWidthRatioGt, right.nonOverlapping);
    return leftFields == rightFields;
}

SplitSceneMatchState RecognizeSnapshot(const SplitSceneRuleSpec& rule, RecognitionSnapshot& snapshot)
{
    SplitSceneMatchState state;
    if (!SplitSceneRecognizer::IsValidWindow(snapshot.window)) {
        state.earlyExitStage = EARLY_EXIT_INVALID_WINDOW;
        return state;
    }
    std::vector<CandidateRect> rects;
    for (size_t index = 0; index < snapshot.candidates.size(); ++index) {
        if (VisitCandidate(rule, index, snapshot, rects, state)) {
            return state;
        }
    }
    MatchCustomRects(rule, std::move(rects), snapshot, state);
    return state;
}
} // namespace

struct SplitSceneBatch::Impl : RecognitionSnapshot {
    using RecognitionSnapshot::RecognitionSnapshot;
};

SplitSceneBatch::SplitSceneBatch(const std::vector<SplitSceneCandidate>& candidates, const RectF& windowRect)
    : impl_(std::make_unique<Impl>(candidates, windowRect))
{}

SplitSceneBatch::~SplitSceneBatch() = default;

SplitSceneMatchState SplitSceneBatch::Recognize(const SplitSceneRuleSpec& rule)
{
    auto normalized = rule;
    std::sort(normalized.compNames.begin(), normalized.compNames.end());
    normalized.compNames.erase(
        std::unique(normalized.compNames.begin(), normalized.compNames.end()), normalized.compNames.end());
    for (const auto& cached : impl_->matches) {
        if (SameRule(cached.rule, normalized)) {
            ++impl_->stats.cacheHits;
            return cached.match;
        }
    }
    ++impl_->stats.recognitions;
    auto result = RecognizeSnapshot(rule, *impl_);
    impl_->matches.push_back({ std::move(normalized), result });
    return result;
}

const std::vector<SplitSceneCandidate>& SplitSceneBatch::GetCandidates() const
{
    return impl_->candidates;
}

const RectF& SplitSceneBatch::GetClippedRect(size_t index)
{
    return impl_->Clip(index);
}

const SplitSceneBatchStats& SplitSceneBatch::GetStats() const
{
    return impl_->stats;
}

bool SplitSceneRecognizer::IsValidWindow(const RectF& windowRect)
{
    return IsFiniteRect(windowRect);
}

bool SplitSceneRecognizer::IsNavigationMatch(
    const SplitSceneRuleSpec& rule, const SplitSceneCandidate& candidate, const RectF& windowRect)
{
    return IsNavigationCandidate(rule, candidate) && IsFiniteRect(ClipToWindow(candidate.rect, windowRect));
}

bool SplitSceneRecognizer::IsCustomCandidate(const SplitSceneRuleSpec& rule, const SplitSceneCandidate& candidate)
{
    return IsCandidateVisible(candidate) &&
           std::find(rule.compNames.begin(), rule.compNames.end(), candidate.compName) != rule.compNames.end();
}

RectF SplitSceneRecognizer::ClipToWindow(const RectF& rect, const RectF& windowRect)
{
    if (!IsFiniteRect(rect)) {
        return {};
    }
    const float left = std::max(rect.Left(), windowRect.Left());
    const float top = std::max(rect.Top(), windowRect.Top());
    const float right = std::min(rect.Right(), windowRect.Right());
    const float bottom = std::min(rect.Bottom(), windowRect.Bottom());
    return right > left && bottom > top ? RectF(left, top, right - left, bottom - top) : RectF();
}

SplitSceneMatchState SplitSceneRecognizer::Recognize(
    const SplitSceneRuleSpec& rule, const std::vector<SplitSceneCandidate>& candidates, const RectF& windowRect)
{
    SplitSceneBatch batch(candidates, windowRect);
    return batch.Recognize(rule);
}
} // namespace OHOS::Ace::NG
