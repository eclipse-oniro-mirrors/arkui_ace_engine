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

#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_restricted_scope.h"

#include "core/components_ng/base/ui_node.h"

namespace OHOS::Ace::NG {

// Counters instead of raw booleans so nested/re-entrant paired calls stay observable
// and a probe can diff before/after snapshots.
std::atomic<uint32_t> ScrollPlaceholderRestrictedScope::beginCount_ = 0;
std::atomic<uint32_t> ScrollPlaceholderRestrictedScope::endCount_ = 0;
std::atomic<uint32_t> ScrollPlaceholderRestrictedScope::violationCount_ = 0;
std::atomic<uint32_t> ScrollPlaceholderRestrictedScope::scopeDepth_ = 0;
std::unordered_set<int32_t> ScrollPlaceholderRestrictedScope::markedProducts_;

void ScrollPlaceholderRestrictedScope::Begin()
{
    beginCount_.fetch_add(1, std::memory_order_relaxed);
    scopeDepth_.fetch_add(1, std::memory_order_relaxed);
}

void ScrollPlaceholderRestrictedScope::End()
{
    endCount_.fetch_add(1, std::memory_order_relaxed);
    uint32_t depth = scopeDepth_.load(std::memory_order_relaxed);
    if (depth > 0) {
        scopeDepth_.compare_exchange_weak(depth, depth - 1, std::memory_order_relaxed);
    }
}

bool ScrollPlaceholderRestrictedScope::IsInRestrictedScope()
{
    return scopeDepth_ > 0;
}

void ScrollPlaceholderRestrictedScope::NotifyViolation()
{
    violationCount_.fetch_add(1, std::memory_order_relaxed);
}

void ScrollPlaceholderRestrictedScope::MarkProduct(const RefPtr<UINode>& root)
{
    CHECK_NULL_VOID(root);
    if (markedProducts_.size() >= MAX_MARKED_PRODUCTS) {
        // Bounded by design: the marks are advisory; discarding them never
        // changes behavior, it only weakens the defense-in-depth signal.
        markedProducts_.clear();
    }
    markedProducts_.insert(root->GetId());
}

bool ScrollPlaceholderRestrictedScope::IsMarkedProduct(const RefPtr<UINode>& root)
{
    if (!root) {
        return false;
    }
    return markedProducts_.find(root->GetId()) != markedProducts_.end();
}

void ScrollPlaceholderRestrictedScope::ClearProductMark(const RefPtr<UINode>& root)
{
    CHECK_NULL_VOID(root);
    markedProducts_.erase(root->GetId());
}

// Arming snapshot: taken at construction so the window covers the whole probe lifetime.
ScrollPlaceholderRestrictedScope::Probe::Probe()
{
    beginCountAtArmed_ = beginCount_;
    endCountAtArmed_ = endCount_;
    violationCountAtArmed_ = violationCount_;
}

void ScrollPlaceholderRestrictedScope::Probe::Close()
{
    beginCountAtClosed_ = beginCount_;
    endCountAtClosed_ = endCount_;
    violationCountAtClosed_ = violationCount_;
    closed_ = true;
}

bool ScrollPlaceholderRestrictedScope::Probe::RestrictedScopeEntered() const
{
    return beginCountAtClosed_ > beginCountAtArmed_;
}

bool ScrollPlaceholderRestrictedScope::Probe::ScopeClosed() const
{
    // The pairing is closed when every begin that fired inside the window also
    // fired its end (equal increments) and no scope is left open.
    uint32_t beginDelta = beginCountAtClosed_ - beginCountAtArmed_;
    uint32_t endDelta = endCountAtClosed_ - endCountAtArmed_;
    return beginDelta > 0 && beginDelta == endDelta && scopeDepth_ == 0;
}

bool ScrollPlaceholderRestrictedScope::Probe::OutOfScopeTriggered() const
{
    return violationCountAtClosed_ > violationCountAtArmed_;
}

bool ScrollPlaceholderRestrictedScope::Probe::Admitted() const
{
    return RestrictedScopeEntered() && ScopeClosed() && !OutOfScopeTriggered();
}

ScopedScrollPlaceholderRestricted::~ScopedScrollPlaceholderRestricted()
{
    ScrollPlaceholderRestrictedScope::End();
}

} // namespace OHOS::Ace::NG
