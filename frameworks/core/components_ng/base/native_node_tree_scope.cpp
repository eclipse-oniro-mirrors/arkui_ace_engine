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

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
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
    std::vector<RefPtr<UINode>> candidates;
    std::vector<RefPtr<UINode>> temporaries;
    std::vector<Conflict> conflicts;
};

struct NativeNodeTreeScope::Context {
    struct State {
        uint64_t id;
        std::vector<RefPtr<UINode>> members;
        std::unordered_set<Context*> owners;
    };
    struct Registry {
        uint64_t nextId = 1;
        // Only members of currently claimed snapshots are indexed. There is no
        // persistent node registration or assumption that a root stays a root.
        std::unordered_map<const UINode*, std::shared_ptr<State>> members;
    };
    struct Location {
        std::shared_ptr<State> state;
        RefPtr<UINode> root;
        std::vector<RefPtr<UINode>> path;
    };

    static Registry& GetRegistry()
    {
        static Registry registry;
        return registry;
    }

    static Location Locate(UINode* node, Preparation& preparation)
    {
        Location result;
        std::unordered_set<const UINode*> visited;
        auto* current = node;
        while (current && visited.insert(current).second) {
            auto found = GetRegistry().members.find(current);
            if (found != GetRegistry().members.end()) {
                // Do not read a claimed node's mutable parent link. Its owner
                // may currently be splitting or merging the business tree.
                result.state = found->second;
                return result;
            }
            if (current->RefCount() == 0) {
                return result;
            }
            auto pin = AceType::Claim(current);
            preparation.temporaries.push_back(pin);
            result.path.push_back(pin);
            auto parent = pin->GetParent();
            if (!parent) {
                parent = pin->GetAdoptParent();
            }
            if (!parent) {
                result.root = pin;
                return result;
            }
            preparation.temporaries.push_back(parent);
            current = parent.GetRawPtr();
        }
        return result;
    }

    static void AddMember(const std::shared_ptr<State>& state, const RefPtr<UINode>& node)
    {
        if (GetRegistry().members.emplace(node.GetRawPtr(), state).second) {
            state->members.push_back(node);
        }
    }

    static void CaptureChildren(const std::shared_ptr<State>& state, const RefPtr<UINode>& root,
        Preparation& preparation)
    {
        std::vector<RefPtr<UINode>> pending { root };
        std::unordered_set<const UINode*> visited;
        while (!pending.empty()) {
            auto node = pending.back();
            pending.pop_back();
            if (!node || !visited.insert(node.GetRawPtr()).second) {
                continue;
            }
            auto found = GetRegistry().members.find(node.GetRawPtr());
            if (found != GetRegistry().members.end() && found->second != state) {
                continue;
            }
            AddMember(state, node);
            for (auto& child : node->GetChildrenForNativeTreeCheck()) {
                if (!child) {
                    continue;
                }
                preparation.temporaries.push_back(child);
                // A disappearing list can retain a child already detached from
                // this tree. Never treat list membership alone as a parent link.
                auto existing = GetRegistry().members.find(child.GetRawPtr());
                if (existing != GetRegistry().members.end()) {
                    if (existing->second == state) {
                        pending.push_back(child);
                    }
                    continue;
                }
                auto parent = child->GetParent();
                if (!parent) {
                    parent = child->GetAdoptParent();
                }
                if (parent.GetRawPtr() == node.GetRawPtr()) {
                    pending.push_back(child);
                }
                if (parent) {
                    preparation.temporaries.push_back(std::move(parent));
                }
            }
        }
    }

    static std::shared_ptr<State> Resolve(UINode* node, Preparation& preparation)
    {
        auto location = Locate(node, preparation);
        auto state = location.state;
        const bool fresh = !state && location.root;
        if (fresh) {
            state = std::make_shared<State>(State { GetRegistry().nextId++, {}, {} });
        }
        if (!state) {
            return nullptr;
        }
        for (const auto& member : location.path) {
            AddMember(state, member);
        }
        if (!fresh) {
            return state;
        }
        CaptureChildren(state, location.root, preparation);
        // Published Native handles also cover cached nodes whose real parent
        // link survives removal from the ordinary/disappearing child lists.
        for (const auto& candidate : preparation.candidates) {
            if (GetRegistry().members.count(candidate.GetRawPtr()) != 0) {
                continue;
            }
            auto cached = Locate(candidate.GetRawPtr(), preparation);
            if (cached.state != state) {
                continue;
            }
            for (const auto& member : cached.path) {
                AddMember(state, member);
                CaptureChildren(state, member, preparation);
            }
        }
        return state;
    }

    void Claim(const std::shared_ptr<State>& state)
    {
        if (state->owners.insert(this).second) {
            states.push_back(state);
        }
    }

    void Acquire(std::initializer_list<UINode*> nodes, const char* apiName, Preparation& preparation)
    {
        const auto mode = GetCheckMode();
        std::unordered_set<const State*> checked;
        std::vector<std::shared_ptr<State>> requested;
        for (auto* node : nodes) {
            auto state = Resolve(node, preparation);
            if (!state || !checked.insert(state.get()).second) {
                continue;
            }
            for (const auto* owner : state->owners) {
                if (mode != CHECK_DISABLED && owner->threadId != threadId) {
                    preparation.conflicts.push_back({ apiName ? apiName : "Unknown", node->GetId(), state->id,
                        owner->threadId, mode });
                    break;
                }
            }
            requested.push_back(std::move(state));
        }
        // A multi-node call may join its input trees. LOG keeps every existing
        // owner on the complete input set until that owner's outer call ends.
        std::unordered_set<Context*> owners { this };
        for (const auto& state : requested) {
            owners.insert(state->owners.begin(), state->owners.end());
        }
        for (auto* owner : owners) {
            for (const auto& state : requested) {
                owner->Claim(state);
            }
        }
    }

    void Release()
    {
        auto& members = GetRegistry().members;
        for (const auto& state : states) {
            state->owners.erase(this);
            if (!state->owners.empty()) {
                continue;
            }
            for (const auto& node : state->members) {
                members.erase(node.GetRawPtr());
            }
        }
    }

    static thread_local Context* current;
    const uint64_t threadId = THREAD_ID;
    std::vector<std::shared_ptr<State>> states;
};

thread_local NativeNodeTreeScope::Context* NativeNodeTreeScope::Context::current = nullptr;

void* NativeNodeTreeScope::CaptureNodes(UINode* const* nodes, size_t count)
{
    auto snapshot = std::make_unique<Preparation>();
    snapshot->candidates.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        if (nodes[i] && nodes[i]->RefCount() != 0) {
            snapshot->candidates.emplace_back(AceType::Claim(nodes[i]));
        }
    }
    return snapshot.release();
}

NativeNodeTreeScope::NativeNodeTreeScope(
    std::initializer_list<UINode*> nodes, const char* apiName, void* snapshot) : apiName_(apiName)
{
    std::unique_ptr<Preparation> preparation(static_cast<Preparation*>(snapshot));
    if (!IsCheckEnabled() && !Context::current) {
        return;
    }
    if (!preparation) {
        preparation = std::make_unique<Preparation>();
    }
    Enter(nodes, *preparation);
    // Preparation, including unrelated candidates, is released outside all
    // protocol/Native-handle locks. Only the claimed tree members stay pinned.
}

void NativeNodeTreeScope::Enter(std::initializer_list<UINode*> nodes, Preparation& preparation)
{
    {
        std::lock_guard<std::recursive_mutex> lock(GetTreeMutex());
        if (!context_) {
            context_ = Context::current;
            if (!context_) {
                if (!IsCheckEnabled()) {
                    return;
                }
                context_ = new Context;
                Context::current = context_;
                outermost_ = true;
                g_activeCalls.fetch_add(1, std::memory_order_release);
            }
        }
        context_->Acquire(nodes, apiName_, preparation);
    }
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
    if (context_ && node) {
        Preparation preparation;
        Enter({ node }, preparation);
    }
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
