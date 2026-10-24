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

#include "core/components_ng/base/native_node_tree_scope.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "base/log/log_wrapper.h"
#include "core/common/ace_application_info.h"
#include "core/components_ng/base/ui_node.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t CHECK_DISABLED = 0;
constexpr int32_t CHECK_CRASH = 2;
std::atomic<int32_t> g_checkMode { -1 };
std::atomic<uint32_t> g_activeCalls { 0 };
std::atomic<uint64_t> g_nextThreadId { 1 };
thread_local const uint64_t THREAD_ID = g_nextThreadId.fetch_add(1, std::memory_order_relaxed);
thread_local bool g_emittingDiagnostics = false;

std::recursive_mutex& GetTreeMutex()
{
    static std::recursive_mutex mutex;
    return mutex;
}

int32_t GetCheckMode()
{
    const auto mode = g_checkMode.load(std::memory_order_acquire);
    return mode < 0 ? (AceApplicationInfo::GetInstance().IsDebugForParallel() ? CHECK_CRASH : CHECK_DISABLED) : mode;
}

struct Conflict {
    std::string apiName;
    int32_t nodeId;
    uint64_t stateId;
    uint64_t owner;
    int32_t mode;
};

void EmitConflict(const Conflict& conflict)
{
    TAG_LOGE(AceLogTag::ACE_NATIVE_NODE,
        "ArkUI runtime check hit. checkName: MULTI_THREAD, apiName: %{public}s, "
        "reason: Same component tree is already operated by another thread, "
        "nodeId: %{public}d, stateId: %{public}llu, currentThreadId: %{public}llu, ownerThreadId: %{public}llu",
        conflict.apiName.c_str(), conflict.nodeId, static_cast<unsigned long long>(conflict.stateId),
        static_cast<unsigned long long>(THREAD_ID), static_cast<unsigned long long>(conflict.owner));
    LogBacktrace();
    if (conflict.mode == CHECK_CRASH) {
        LOGF_ABORT("ArkUI runtime check abort. checkName: MULTI_THREAD, apiName: %{public}s, "
                   "stateId: %{public}llu, currentThreadId: %{public}llu, ownerThreadId: %{public}llu",
            conflict.apiName.c_str(), static_cast<unsigned long long>(conflict.stateId),
            static_cast<unsigned long long>(THREAD_ID), static_cast<unsigned long long>(conflict.owner));
    }
}
} // namespace

struct NativeNodeTreeScope::Preparation {
    std::vector<Conflict> conflicts;
    // Temporary references are destroyed after releasing the protocol lock.
    std::vector<RefPtr<UINode>> nodes;
};

struct NativeNodeTreeScope::Context {
    struct State {
        uint64_t id;
        uint64_t threadId;
        std::vector<RefPtr<UINode>> roots;
    };

    static std::vector<Context*>& ActiveContexts()
    {
        static std::vector<Context*> contexts;
        return contexts;
    }

    static uint64_t NextStateId()
    {
        static uint64_t nextId = 1;
        return nextId++;
    }

    // Ownership bookkeeping only. This is called after reading a nonzero owner
    // from the node; it never supplies a cached parent/root for traversal.
    static std::shared_ptr<State> FindClaim(const UINode* root)
    {
        std::unordered_set<const State*> visited;
        for (const auto* context : ActiveContexts()) {
            for (const auto& state : context->states) {
                if (!visited.insert(state.get()).second) {
                    continue;
                }
                for (const auto& claimed : state->roots) {
                    if (claimed.GetRawPtr() == root) {
                        return state;
                    }
                }
            }
        }
        return nullptr;
    }

    void Claim(const std::shared_ptr<State>& state)
    {
        if (std::find(states.begin(), states.end(), state) == states.end()) {
            states.push_back(state);
        }
    }

    static void MarkRoot(const std::shared_ptr<State>& state, const RefPtr<UINode>& node)
    {
        if (node->GetNativeTreeOperationThreadId() == 0) {
            state->roots.push_back(node);
            node->SetNativeTreeOperationThreadId(state->threadId);
        }
    }

    std::shared_ptr<State> WalkParents(UINode* node, const char* apiName, Preparation& preparation,
        std::unordered_set<const State*>& reported)
    {
        if (!node || node->RefCount() == 0) {
            return nullptr;
        }
        auto current = AceType::Claim(node);
        std::unordered_set<const UINode*> visited;
        while (visited.insert(current.GetRawPtr()).second) {
            pins.push_back(current);
            std::shared_ptr<State> claimed;
            const auto primaryOwner = current->GetNativeTreeOperationThreadId();
            if (primaryOwner != 0) {
                claimed = FindClaim(current.GetRawPtr());
                if (!claimed) {
                    LOGF_ABORT("MULTI_THREAD: node owner has no active operation claim");
                }
                if (claimed->threadId != threadId) {
                    if (GetCheckMode() != CHECK_DISABLED && reported.insert(claimed.get()).second) {
                        preparation.conflicts.push_back({ apiName ? apiName : "Unknown", node->GetId(),
                            claimed->id, claimed->threadId, GetCheckMode() });
                    }
                    // A foreign operation may be rewriting this exact link.
                    // CRASH terminates before business; LOG continues business
                    // without making an unsafe parent read just for diagnosis.
                    return nullptr;
                }
            }
            // Always traverse real links, including roots marked by this thread.
            // An existing claim is not a root-finding cache or an early success.
            auto parent = current->GetParent();
            if (!parent) {
                parent = current->GetAdoptParent();
            }
            if (!parent) {
                if (!claimed) {
                    claimed = std::make_shared<State>(State { NextStateId(), threadId, {} });
                    Claim(claimed);
                    MarkRoot(claimed, current);
                }
                return claimed;
            }
            current = std::move(parent);
        }
        return nullptr;
    }

    void Acquire(const std::vector<UINode*>& nodes, const char* apiName, Preparation& preparation)
    {
        std::unordered_set<const State*> reported;
        for (auto* node : nodes) {
            auto state = WalkParents(node, apiName, preparation, reported);
            if (!state) {
                continue;
            }
            // A newly announced operand/detach root joins the current domain.
            Claim(state);
            MarkRoot(state, AceType::Claim(node));
        }
    }

    void Release()
    {
        for (const auto& state : states) {
            for (const auto& root : state->roots) {
                root->SetNativeTreeOperationThreadId(0);
            }
        }
        auto& active = ActiveContexts();
        active.erase(std::remove(active.begin(), active.end(), this), active.end());
    }

    static thread_local Context* current;
    const uint64_t threadId = THREAD_ID;
    std::vector<std::shared_ptr<State>> states;
    std::vector<RefPtr<UINode>> pins;
};

thread_local NativeNodeTreeScope::Context* NativeNodeTreeScope::Context::current = nullptr;

NativeNodeTreeScope::NativeNodeTreeScope(std::initializer_list<UINode*> nodes, const char* apiName) : apiName_(apiName)
{
    if (!IsCheckEnabled() && !Context::current) {
        return;
    }
    Preparation preparation;
    {
        std::lock_guard<std::recursive_mutex> lock(GetTreeMutex());
        context_ = Context::current;
        if (!context_) {
            if (!IsCheckEnabled()) {
                return;
            }
            context_ = new Context;
            Context::current = context_;
            Context::ActiveContexts().push_back(context_);
            outermost_ = true;
            g_activeCalls.fetch_add(1, std::memory_order_release);
        }
        context_->Acquire({ nodes.begin(), nodes.end() }, apiName_, preparation);
    }
    Report(preparation);
}

void NativeNodeTreeScope::Report(Preparation& preparation)
{
    // No metadata lock covers diagnostics, original business logic or callbacks.
    if (!g_emittingDiagnostics) {
        g_emittingDiagnostics = true;
        for (const auto& conflict : preparation.conflicts) {
            EmitConflict(conflict);
        }
        g_emittingDiagnostics = false;
    }
}

void NativeNodeTreeScope::IncludeNode(UINode* node)
{
    if (!context_ || !node) {
        return;
    }
    Preparation preparation;
    {
        std::lock_guard<std::recursive_mutex> lock(GetTreeMutex());
        context_->Acquire({ node }, apiName_, preparation);
    }
    Report(preparation);
}

void NativeNodeTreeScope::PrepareChildren(UINode* parent)
{
    if (!context_ || !parent) {
        return;
    }
    Preparation preparation;
    {
        std::lock_guard<std::recursive_mutex> lock(GetTreeMutex());
        // Snapshot only the direct roots that clearChildren/Clean can detach.
        // The accessor is read-only and does not invoke virtual GetChildren().
        preparation.nodes = parent->GetChildrenForNativeTreeCheck();
        std::vector<UINode*> roots;
        roots.reserve(preparation.nodes.size());
        for (const auto& child : preparation.nodes) {
            roots.push_back(child.GetRawPtr());
        }
        context_->Acquire(roots, apiName_, preparation);
    }
    Report(preparation);
}

NativeNodeTreeScope::~NativeNodeTreeScope()
{
    if (!outermost_) {
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(GetTreeMutex());
        context_->Release();
        Context::current = nullptr;
        g_activeCalls.fetch_sub(1, std::memory_order_release);
    }
    delete context_;
}

void NativeNodeTreeScope::SetCheckMode(int32_t mode)
{
    g_checkMode.store(mode, std::memory_order_release);
}

bool NativeNodeTreeScope::IsCheckEnabled()
{
    return GetCheckMode() != CHECK_DISABLED;
}

bool NativeNodeTreeScope::NeedsSynchronization()
{
    return IsCheckEnabled() || g_activeCalls.load(std::memory_order_acquire) != 0;
}
} // namespace OHOS::Ace::NG
