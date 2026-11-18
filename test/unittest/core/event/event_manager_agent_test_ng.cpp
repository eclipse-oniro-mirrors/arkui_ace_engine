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

#include "test/unittest/core/event/event_manager_test_ng.h"

#include <array>

#include "core/components_ng/event/event_constants.h"
#include "core/components_ng/base/node_sensitive_input.h"
#include "core/components_ng/base/observer_handler.h"
#include "core/components_ng/gestures/recognizers/click_recognizer.h"
#include "core/components_ng/gestures/recognizers/parallel_recognizer.h"
#include "core/components_ng/pattern/command/ui_command_security_gate.h"
#include "test/mock/adapter/ohos/osal/mock_input_manager.h"

using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t POINTER_ID = 10;
constexpr int32_t CALLING_UID = 201;
constexpr uint64_t SESSION_ID = 0x10000000cULL;
constexpr auto ALLOW = AgentControlMode::ALLOW;
constexpr auto DENY = AgentControlMode::DENY;
using ExpectedReport = std::pair<RefPtr<FrameNode>, bool>;

class AgentInputProbe final {
public:
    AgentInputProbe()
        : notify_(UIObserverHandler::GetInstance().nodeSensitiveInputHandleFunc_),
          report_(MockInputManager::agentInputReporter),
          pipeline_(MockPipelineContext::pipeline_),
          apiVersion_(AceApplicationInfo::GetInstance().GetApiTargetVersion()),
          permission_(MockInputManager::controlComponentPermissionGranted),
          permissionCache_(MockInputManager::controlComponentPermissionCache),
          permissionChecks_(MockInputManager::controlComponentPermissionCheckCount)
    {
        MockInputManager::controlComponentPermissionGranted = true;
        MockInputManager::controlComponentPermissionCache.reset();
        MockInputManager::controlComponentPermissionCheckCount = 0;
        UIObserverHandler::GetInstance().SetNodeSensitiveInputHandleFunc(
            [this](const RefPtr<FrameNode>& node, const NodeSensitiveInputInfo& info) {
                notifications.emplace_back(node->GetId(), info.isAgent);
            });
        MockInputManager::agentInputReporter =
            [this](const AgentInputResult& result) { reports.emplace_back(result); };
    }
    ~AgentInputProbe()
    {
        UIObserverHandler::GetInstance().SetNodeSensitiveInputHandleFunc(std::move(notify_));
        MockInputManager::agentInputReporter = std::move(report_);
        MockPipelineContext::pipeline_ = pipeline_;
        AceApplicationInfo::GetInstance().SetApiTargetVersion(apiVersion_);
        MockInputManager::controlComponentPermissionGranted = permission_;
        MockInputManager::controlComponentPermissionCache = permissionCache_;
        MockInputManager::controlComponentPermissionCheckCount = permissionChecks_;
    }
    void ExpectReports(const std::vector<ExpectedReport>& expected, uint64_t session = SESSION_ID)
    {
        ASSERT_EQ(reports.size(), expected.size());
        for (size_t index = 0; index < expected.size(); ++index) {
            const auto& [node, allowed] = expected[index];
            EXPECT_EQ(reports[index].nodeId, node->GetId());
            EXPECT_EQ(reports[index].compId, std::to_string(node->GetId()));
            EXPECT_EQ(reports[index].tag, node->GetTag());
            EXPECT_EQ(reports[index].allowed, allowed);
            EXPECT_EQ(reports[index].sessionId, session);
        }
    }
    std::vector<AgentInputResult> reports;
    std::vector<std::pair<int32_t, bool>> notifications;

private:
    UIObserverHandler::NodeSensitiveInputHandleFunc notify_;
    MockInputManager::AgentInputReportFunc report_;
    RefPtr<MockPipelineContext> pipeline_;
    int32_t apiVersion_;
    bool permission_;
    std::optional<bool> permissionCache_;
    int32_t permissionChecks_;
};

RefPtr<FrameNode> MakeNode(AgentControlMode mode = ALLOW)
{
    auto node = FrameNode::CreateFrameNode("agentInput", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    node->SetAgentControlMode(mode);
    return node;
}

class AgentTouchTarget final : public MockTouchEventTarget {
    DECLARE_ACE_TYPE(AgentTouchTarget, MockTouchEventTarget);
public:
    bool DispatchEvent(const TouchEvent&) override { return true; }
    bool HandleEvent(const TouchEvent&) override
    {
        ++received;
        return true;
    }
    int32_t received = 0;
};

TouchEvent MakeTouch()
{
    TouchEvent event;
    event.id = POINTER_ID;
    event.originalId = POINTER_ID;
    event.isAgent = true;
    event.hasSafSessionId = true;
    event.safSessionId = SESSION_ID;
    event.callingUid = CALLING_UID;
    return event;
}

KeyEvent MakeKey()
{
    KeyEvent event(KeyCode::KEY_ENTER, KeyAction::DOWN);
    event.isAgent = true;
    event.hasSafSessionId = true;
    event.safSessionId = SESSION_ID;
    event.callingUid = CALLING_UID;
    return event;
}

void CheckMouseSequence(AgentInputProbe& probe, const RefPtr<EventManager>& manager, MouseEvent event,
    const std::vector<RefPtr<AgentTouchTarget>>& targets, const std::array<int32_t, 2>& received,
    const std::vector<ExpectedReport>& expected, bool denied)
{
    int32_t dispatched = 0;
    for (auto action : { MouseAction::PRESS, MouseAction::RELEASE }) {
        event.action = action;
        if (action == MouseAction::PRESS) {
            manager->touchTestResults_[event.CreateTouchPoint().id] = { targets.front(), targets.back() };
            manager->PrepareAgentMousePress(event);
        }
        probe.reports.clear();
        EXPECT_TRUE(manager->DispatchTouchEvent(event.CreateTouchPoint()));
        EXPECT_EQ(manager->DispatchMouseEventNG(event), denied);
        // Mouse-derived touch and mouse dispatch together produce one report set per round.
        probe.ExpectReports(expected);
        ++dispatched;
        EXPECT_EQ(received.front(), dispatched);
        EXPECT_EQ(received.back(), denied ? 0 : dispatched);
        EXPECT_EQ(targets.front()->received, dispatched);
        EXPECT_EQ(targets.back()->received, denied ? 0 : dispatched);
    }
    EXPECT_TRUE(manager->agentMouseStates_.empty());
    EXPECT_TRUE(manager->pressMouseTestResultsMap_.empty());
}

} // namespace

/**
 * @tc.name: AgentTouchPropagation001
 * @tc.desc: Report reached nodes before business delivery; preserve human and ACL bypass.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, AgentTouchPropagation001, TestSize.Level1)
{
    for (const auto& [agent, permission] : { std::pair { true, true }, { false, true }, { true, false } }) {
        for (auto mode : { ALLOW, DENY }) {
            AgentInputProbe probe;
            MockInputManager::controlComponentPermissionGranted = permission;
            auto manager = AceType::MakeRefPtr<EventManager>();
            auto child = MakeNode();
            auto parent = MakeNode(mode);
            auto first = AceType::MakeRefPtr<AgentTouchTarget>();
            auto second = AceType::MakeRefPtr<AgentTouchTarget>();
            first->AttachFrameNode(child);
            second->AttachFrameNode(parent);
            manager->touchTestResults_[POINTER_ID] = { first, second };
            manager->touchInputHitNodes_[POINTER_ID] = child;
            auto event = MakeTouch();
            event.isAgent = agent;
            const bool denied = agent && permission && mode == DENY;
            const std::vector<ExpectedReport> expected = agent ?
                std::vector<ExpectedReport> { { child, true }, { parent, !denied } } :
                std::vector<ExpectedReport> {};
            int32_t dispatched = 0;
            MockInputManager::agentInputReporter = [&](const AgentInputResult& result) {
                EXPECT_EQ(result.nodeId == child->GetId() ? first->received : second->received,
                    result.nodeId == parent->GetId() && denied ? 0 : dispatched);
                probe.reports.emplace_back(result);
            };
            for (auto type : { TouchType::DOWN, TouchType::MOVE, TouchType::UP }) {
                probe.reports.clear();
                event.type = type;
                EXPECT_TRUE(manager->DispatchTouchEvent(event));
                probe.ExpectReports(type == TouchType::MOVE ?
                    std::vector<ExpectedReport> {} : expected);
                EXPECT_EQ(first->received, ++dispatched);
                EXPECT_EQ(second->received, denied ? 0 : dispatched);
            }
            EXPECT_EQ(probe.notifications, (std::vector<std::pair<int32_t, bool>>(3, { child->GetId(), agent })));
            EXPECT_TRUE(manager->agentTouchStates_.empty());
        }
    }
}

/**
 * @tc.name: AgentMousePropagation001
 * @tc.desc: Mouse and derived Touch share per-event reports; human and unprivileged input retain propagation.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, AgentMousePropagation001, TestSize.Level1)
{
    for (auto api : { PlatformVersion::VERSION_TWELVE, PlatformVersion::VERSION_THIRTEEN }) {
        for (const auto& [agent, permission] : { std::pair { true, true }, { false, true }, { true, false } }) {
            AgentInputProbe probe;
            AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(api));
            MockInputManager::controlComponentPermissionGranted = permission;
            auto manager = AceType::MakeRefPtr<EventManager>();
            auto child = MakeNode();
            auto parent = MakeNode(DENY);
            std::array<int32_t, 2> received {};
            std::vector<RefPtr<AgentTouchTarget>> touchTargets;
            MouseEvent event;
            event.id = POINTER_ID;
            event.button = MouseButton::LEFT_BUTTON;
            event.sourceType = SourceType::MOUSE;
            event.isAgent = agent;
            event.hasSafSessionId = true;
            event.safSessionId = SESSION_ID;
            event.callingUid = CALLING_UID;
            for (const auto& node : { child, parent }) {
                const auto index = touchTargets.size();
                auto target = AceType::MakeRefPtr<MouseEventTarget>("mouse", node->GetId());
                target->AttachFrameNode(node);
                target->SetCallback([&, index](MouseInfo&) { ++received[index]; });
                manager->currMouseTestResultsMap_[event.id].emplace_back(target);
                auto touch = AceType::MakeRefPtr<AgentTouchTarget>();
                touch->AttachFrameNode(node);
                touchTargets.emplace_back(touch);
            }
            const bool denied = agent && permission;
            const std::vector<ExpectedReport> expected = agent ?
                std::vector<ExpectedReport> { { child, true }, { parent, !denied } } :
                std::vector<ExpectedReport> {};
            CheckMouseSequence(probe, manager, event, touchTargets, received, expected, denied);
        }
    }
}

/**
 * @tc.name: AgentKeyInputControl002
 * @tc.desc: Enter follows existing focus bubbling, reports per node and respects human/ACL bypass.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, AgentKeyInputControl002, TestSize.Level1)
{
    for (const auto& [agent, permission] : { std::pair { true, true }, { false, true }, { true, false } }) {
        AgentInputProbe probe;
        MockInputManager::controlComponentPermissionGranted = permission;
        auto pipeline = AceType::MakeRefPtr<MockPipelineContext>();
        auto child = MakeNode();
        auto parent = MakeNode(DENY);
        child->AttachContext(AceType::RawPtr(pipeline));
        parent->AttachContext(AceType::RawPtr(pipeline));
        auto childFocus = child->GetOrCreateFocusHub();
        auto parentFocus = parent->GetOrCreateFocusHub();
        childFocus->currentFocus_ = parentFocus->currentFocus_ = true;
        childFocus->focusType_ = FocusType::NODE;
        parentFocus->focusType_ = FocusType::SCOPE;
        parentFocus->lastWeakFocusNode_ = childFocus;
        int32_t childCalls = 0;
        int32_t parentCalls = 0;
        childFocus->SetOnKeyEventInternal([&](const KeyEvent&) { ++childCalls; return false; });
        parentFocus->SetOnKeyEventInternal([&](const KeyEvent&) { ++parentCalls; return false; });
        KeyEvent event(KeyCode::KEY_ENTER, KeyAction::DOWN);
        event.isAgent = agent;
        event.hasSafSessionId = true;
        event.safSessionId = SESSION_ID;
        event.callingUid = CALLING_UID;
        const bool denied = agent && permission;
        int32_t dispatched = 0;
        for (auto action : { KeyAction::DOWN, KeyAction::UP }) {
            probe.reports.clear();
            event.action = action;
            EXPECT_EQ(parentFocus->HandleEvent(event), denied);
            probe.ExpectReports(agent ? std::vector<ExpectedReport> { { child, true }, { parent, !denied } } :
                std::vector<ExpectedReport> {});
            EXPECT_EQ(childCalls, ++dispatched);
            EXPECT_EQ(parentCalls, denied ? 0 : dispatched);
        }
    }
}

/**
 * @tc.name: AgentClickInputControl001
 * @tc.desc: A real child Click survives a denied parent in a gesture group; a denied child cannot click.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, AgentClickInputControl001, TestSize.Level1)
{
    for (auto mode : { ALLOW, DENY }) {
        AgentInputProbe probe;
        auto pipeline = AceType::MakeRefPtr<MockPipelineContext>();
        MockPipelineContext::pipeline_ = pipeline;
        auto manager = AceType::MakeRefPtr<EventManager>();
        pipeline->eventManager_ = manager;
        std::vector<RefPtr<FrameNode>> nodes;
        std::vector<RefPtr<NGGestureRecognizer>> recognizers;
        std::array<int32_t, 2> clicks {};
        for (auto policy : { mode, DENY }) {
            const auto index = nodes.size();
            auto node = MakeNode(policy);
            node->AttachContext(AceType::RawPtr(pipeline));
            auto click = AceType::MakeRefPtr<ClickRecognizer>(1, 1, 100.0);
            click->AttachFrameNode(node);
            click->SetUseCatchMode(false);
            click->SetOnAction([&, index](GestureEvent&) { ++clicks[index]; });
            nodes.emplace_back(node);
            recognizers.emplace_back(click);
        }
        auto group = AceType::MakeRefPtr<ParallelRecognizer>(recognizers);
        group->BeginReferee(POINTER_ID, POINTER_ID, true);
        manager->touchTestResults_[POINTER_ID] = { group };
        auto event = MakeTouch();
        event.sourceType = SourceType::TOUCH;
        event.sourceTool = SourceTool::FINGER;
        event.childTouchTestList = { "" };
        // With per-recognizer denial, both children report even when the first is denied.
        const std::vector<ExpectedReport> expected = { { nodes.front(), mode == ALLOW }, { nodes.back(), false } };
        for (auto type : { TouchType::DOWN, TouchType::UP }) {
            probe.reports.clear();
            event.type = type;
            EXPECT_TRUE(manager->DispatchTouchEvent(event));
            probe.ExpectReports(expected);
        }
        EXPECT_EQ(clicks.front(), mode == ALLOW ? 1 : 0);
        EXPECT_EQ(clicks.back(), 0);
    }
}

/**
 * @tc.name: AgentKeyCancel001
 * @tc.desc: Cancel closes the operation; the next DOWN reads fresh policy and reuses the application ACL.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, AgentKeyCancel001, TestSize.Level1)
{
    AgentInputProbe probe;
    auto pipeline = AceType::MakeRefPtr<MockPipelineContext>();
    auto node = MakeNode();
    node->AttachContext(AceType::RawPtr(pipeline));
    auto focus = node->GetOrCreateFocusHub();
    focus->currentFocus_ = true;
    focus->focusType_ = FocusType::NODE;
    int32_t calls = 0;
    focus->SetOnKeyEventInternal([&](const KeyEvent&) { ++calls; return false; });
    auto event = MakeKey();
    EXPECT_FALSE(focus->HandleEvent(event));
    event.action = KeyAction::CANCEL;
    EXPECT_FALSE(focus->HandleEvent(event));
    EXPECT_EQ(calls, 2);
    node->SetAgentControlMode(DENY);
    probe.reports.clear();
    event.action = KeyAction::DOWN;
    EXPECT_TRUE(focus->HandleEvent(event));
    probe.ExpectReports({ { node, false } });
    EXPECT_EQ(calls, 2);
    EXPECT_EQ(MockInputManager::controlComponentPermissionCheckCount, 1);
    event.action = KeyAction::CANCEL;
    EXPECT_TRUE(focus->HandleEvent(event));
}

/**
 * @tc.name: CommandSecurityGateMatrix001
 * @tc.desc: AC-1/AC-2: SendCommand security gate refuses only when the caller declared auth, the target
 *           itself is DENY and the control-component permission is granted; the fence report
 *           fires exactly once per refused command and never on the legacy path.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, CommandSecurityGateMatrix001, TestSize.Level1)
{
    auto deniedNode = MakeNode(DENY);
    auto allowNode = MakeNode(ALLOW);
    auto unsetNode = FrameNode::CreateFrameNode("unsetMode",
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_TRUE(unsetNode);
    EXPECT_FALSE(unsetNode->HasAgentControlMode());

    // Legacy path: auth flag false never triggers the permission check or a fence report.
    {
        AgentInputProbe probe;
        EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(deniedNode, false));
        EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(allowNode, false));
        EXPECT_EQ(MockInputManager::controlComponentPermissionCheckCount, 0);
        EXPECT_TRUE(probe.reports.empty());
    }

    // Auth flag true but target is ALLOW (explicit or unset default): proceed.
    {
        AgentInputProbe probe;
        EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(allowNode, true));
        EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(unsetNode, true));
        EXPECT_TRUE(probe.reports.empty());
    }

    // Auth flag true and target DENY but the permission check fails: auth fails, proceed.
    {
        AgentInputProbe probe;
        MockInputManager::controlComponentPermissionGranted = false;
        MockInputManager::controlComponentPermissionCache.reset();
        EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(deniedNode, true));
        EXPECT_TRUE(probe.reports.empty());
    }

    // Auth flag true, target DENY and permission granted: refuse and report to the fence.
    // The caller-relayed tokenId routes the notification when present.
    {
        AgentInputProbe probe;
        EXPECT_TRUE(UiCommandSecurityGate::IsCommandDenied(deniedNode, true));
        CommandSecurityContext security;
        security.needSecurityAuth = true;
        security.tokenId = SESSION_ID;
        UiCommandSecurityGate::ReportDeniedCommand(deniedNode, security);
        ASSERT_EQ(probe.reports.size(), 1u);
        const auto& result = probe.reports.front();
        EXPECT_EQ(result.nodeId, deniedNode->GetId());
        EXPECT_EQ(result.compId, std::to_string(deniedNode->GetId()));
        EXPECT_EQ(result.tag, deniedNode->GetTag());
        EXPECT_FALSE(result.allowed);
        EXPECT_EQ(result.sessionId, SESSION_ID);
    }

    // A null target never triggers the gate.
    EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(nullptr, true));
}

/**
 * @tc.name: CommandSecurityGateParentChildAndDynamic001
 * @tc.desc: AC-3/R-4: the gate reads only the target's own AgentControlMode; a parent's
 *           DENY/ALLOW is never inherited, and a dynamic mode change applies from the
 *           next command check on.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, CommandSecurityGateParentChildAndDynamic001, TestSize.Level1)
{
    AgentInputProbe probe;
    auto parent = MakeNode(DENY);
    auto child = MakeNode(ALLOW);
    parent->AddChild(child);
    // Parent DENY, child ALLOW: the child command is not denied.
    EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(child, true));

    auto allowParent = MakeNode(ALLOW);
    auto denyChild = MakeNode(DENY);
    allowParent->AddChild(denyChild);
    // Parent ALLOW, child DENY: the child command is denied by its own policy.
    EXPECT_TRUE(UiCommandSecurityGate::IsCommandDenied(denyChild, true));

    // Dynamic update: the same node flips DENY -> ALLOW -> DENY and each check reads
    // the current value (changes apply from the next command).
    auto node = MakeNode(DENY);
    EXPECT_TRUE(UiCommandSecurityGate::IsCommandDenied(node, true));
    node->SetAgentControlMode(ALLOW);
    EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(node, true));
    node->SetAgentControlMode(DENY);
    EXPECT_TRUE(UiCommandSecurityGate::IsCommandDenied(node, true));
    // Unsetting keeps the ALLOW default.
    node->SetAgentControlMode(std::nullopt);
    EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(node, true));
    EXPECT_TRUE(probe.reports.empty());
}

/**
 * @tc.name: CommandSecurityGateInternalPopupTarget001
 * @tc.desc: AC-5/R-14: an internal popup target (e.g. a Menu node reached by nodeId)
 *           follows the same gate rule as a normal target within the same instance.
 * @tc.type: FUNC
 */
HWTEST_F(EventManagerTestNg, CommandSecurityGateInternalPopupTarget001, TestSize.Level1)
{
    AgentInputProbe probe;
    auto popupItem = FrameNode::CreateFrameNode(
        "Menu", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_TRUE(popupItem);
    popupItem->SetAgentControlMode(DENY);
    // Same rule as a normal DENY target: refused with the auth request on.
    EXPECT_TRUE(UiCommandSecurityGate::IsCommandDenied(popupItem, true));
    // And same rule when the popup target is ALLOW: proceeds.
    popupItem->SetAgentControlMode(ALLOW);
    EXPECT_FALSE(UiCommandSecurityGate::IsCommandDenied(popupItem, true));
    // A normal node in the same instance behaves identically.
    auto normal = MakeNode(DENY);
    EXPECT_TRUE(UiCommandSecurityGate::IsCommandDenied(normal, true));
    EXPECT_TRUE(probe.reports.empty());
}
} // namespace OHOS::Ace::NG
