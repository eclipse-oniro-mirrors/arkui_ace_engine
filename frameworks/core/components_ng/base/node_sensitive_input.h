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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_NODE_SENSITIVE_INPUT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_NODE_SENSITIVE_INPUT_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "base/memory/ace_type.h"

namespace OHOS::Ace {
struct TouchEvent;
struct MouseEvent;
struct KeyEvent;
class TouchEventInfo;
class MouseInfo;
class KeyEventInfo;
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
class FrameNode;

enum class NodeSensitiveInputEventType : int32_t {
    TOUCH = 0,
    MOUSE = 1,
    KEY = 2,
};

struct NodeSensitiveInputInfo {
    bool isAgent = false;
    NodeSensitiveInputEventType type = NodeSensitiveInputEventType::TOUCH;
    const TouchEvent* touchEvent = nullptr;
    const MouseEvent* mouseEvent = nullptr;
    const KeyEvent* keyEvent = nullptr;
};

struct AgentInputResult {
    uint64_t sessionId = 0;
    int32_t callingUid = -1;
    int32_t instanceId = -1;
    int32_t nodeId = -1;
    std::string compId;
    std::string tag;
    bool allowed = true;
};

// Frozen per-node decision, captured when the node is first engaged by the gesture.
struct NodeSnapshot {
    WeakPtr<FrameNode> node;
    std::string compId;
    bool allowed = true;
    bool hasControlMode = false;
};

// Per-gesture state: each node's decision freezes on first engagement; reporting
// dedups across MOVE and repeats once more on UP/RELEASE.
struct AgentGestureState {
    int32_t initialNodeId = -1;
    bool hasSession = false;
    uint64_t sessionId = 0;
    int32_t callingUid = -1;
    int32_t instanceId = -1;
    std::unordered_set<int32_t> enteredNodes;
    std::unordered_map<int32_t, NodeSnapshot> nodes;
};

// Input dispatch does not need the navigation and page observer definitions.
ACE_EXPORT void NotifyNodeSensitiveInput(const RefPtr<FrameNode>& node, const NodeSensitiveInputInfo& info);
using OwnedNodeSensitiveInputPeer = std::unique_ptr<void, void (*)(void*)>;
ACE_FORCE_EXPORT OwnedNodeSensitiveInputPeer CreateNodeSensitiveInputPeer(
    const NodeSensitiveInputInfo& info, FrameNode* node);
ACE_EXPORT TouchEventInfo CreateNodeSensitiveTouchInfo(
    const TouchEvent& event, FrameNode* node = nullptr);
ACE_EXPORT MouseInfo CreateNodeSensitiveMouseInfo(
    const MouseEvent& event, FrameNode* node = nullptr);
ACE_EXPORT KeyEventInfo CreateNodeSensitiveKeyInfo(
    const KeyEvent& event, FrameNode* node = nullptr);
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_NODE_SENSITIVE_INPUT_H
