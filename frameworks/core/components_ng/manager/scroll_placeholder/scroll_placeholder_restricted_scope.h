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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_RESTRICTED_SCOPE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_RESTRICTED_SCOPE_H

#include <atomic>
#include <cstdint>
#include <unordered_set>

#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::NG {

class UINode;

// Runtime consumption of the restricted-builder instrumentation contract (single-layer
// instrumentation): the compiler emits a paired
// __aceBeginScrollPlaceholder__ / __aceEndScrollPlaceholder__ call at the first/last
// line of every @Builder({ isPlaceholder: true }) conversion product; plain builders
// are not instrumented at all (zero-overhead rule). Whether "begin" fires during an
// isolated try-execute is therefore the sole legality signal for registration
// admission, and the scope state below is what the begin/end entries drive.
//
// Binding form: JsRegisterViews
// mounts __aceBegin/EndScrollPlaceholder__ on globalThis as native functions so
// compiler products can call them bare without imports. Unlike a module-binding-only
// form this is referenceable by dynamic JS, so the admission
// signal is a compiler-product convention rather than an unforgeable guarantee.
//
// UI thread only: builder execution never leaves the UI thread.
class ScrollPlaceholderRestrictedScope {
public:
    // Paired with End. Re-entrant: nested begin/end pairs keep a stack count.
    static void Begin();
    static void End();

    static bool IsInRestrictedScope();

    // Fail-fast hook for out-of-scope behavior observed while a restricted scope is
    // active (reactive-state subscription, gesture or animation registration). Marks
    // the current scope as violated: the pending probe fails, the half-built product
    // is discarded by the caller and the creation is aborted.
    static void NotifyViolation();

    // Marks the root of a subtree produced inside a restricted scope. The commit path
    // validates the mark before mounting (products without it are treated as
    // unverified and never mounted).
    static void MarkProduct(const RefPtr<UINode>& root);
    static bool IsMarkedProduct(const RefPtr<UINode>& root);
    static void ClearProductMark(const RefPtr<UINode>& root);

    // Per-execution observation of the instrumentation signals: armed around one
    // isolated try-execute of a builder (registration admission shares this execution
    // with the registration probe). RestrictedScopeEntered() answers whether
    // begin fired (an uninstrumented plain builder never fires it); ScopeClosed()
    // whether the pairing closed; OutOfScopeTriggered() whether a violation was
    // reported in between.
    class Probe {
    public:
        Probe();
        ~Probe() = default;

        // Snapshots the counters after the try-execute returned; accessors read the
        // snapshot so they keep reporting the same result once closed.
        void Close();

        bool RestrictedScopeEntered() const;
        bool ScopeClosed() const;
        bool OutOfScopeTriggered() const;

        // begin fired && pairing closed && no violation.
        bool Admitted() const;

    private:
        uint32_t beginCountAtArmed_ = 0;
        uint32_t endCountAtArmed_ = 0;
        uint32_t violationCountAtArmed_ = 0;
        uint32_t beginCountAtClosed_ = 0;
        uint32_t endCountAtClosed_ = 0;
        uint32_t violationCountAtClosed_ = 0;
        bool closed_ = false;
    };

private:
    // Relaxed atomics: begin/end are driven by compiler-injected calls and the
    // admission probes read them on the UI thread; atomics keep the counters
    // well-defined should an out-of-contract thread ever touch them.
    static std::atomic<uint32_t> beginCount_;
    static std::atomic<uint32_t> endCount_;
    static std::atomic<uint32_t> violationCount_;
    static std::atomic<uint32_t> scopeDepth_;
    // Bounded advisory mark registry: overflows discard the whole set (marks are
    // a defense-in-depth signal, never a correctness requirement).
    static constexpr size_t MAX_MARKED_PRODUCTS = 1024;
    static std::unordered_set<int32_t> markedProducts_;
};

// RAII guard forwarding to Begin/End so a native try-execute site keeps the pairing
// balanced on every exit path.
class ScopedScrollPlaceholderRestricted {
public:
    ScopedScrollPlaceholderRestricted()
    {
        ScrollPlaceholderRestrictedScope::Begin();
    }
    ~ScopedScrollPlaceholderRestricted();

private:
    ACE_DISALLOW_COPY_AND_MOVE(ScopedScrollPlaceholderRestricted);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_RESTRICTED_SCOPE_H
