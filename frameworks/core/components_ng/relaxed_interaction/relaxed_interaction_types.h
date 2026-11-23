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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_RELAXED_INTERACTION_TYPES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_RELAXED_INTERACTION_TYPES_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "base/geometry/ng/point_t.h"
#include "core/event/key_code.h"

namespace OHOS::Ace::NG {

// Security identity carried by injected commands (P2).
struct RelaxedCommandIdentity {
    std::string source;
    std::string token;
    std::string label;
};

// Deny check on the hit-tested frame node at the injection point.
using RelaxedDenyCallback = std::function<bool(const PointF&, const RelaxedCommandIdentity&)>;

// Terminal result codes reported through the relaxed/PC command finish callback.
// The first group keeps the legacy relaxed codes; the remaining codes are only
// produced by the four PC commands.
enum class RelaxedCommandErrorType : uint8_t {
    NONE,
    NODE_NOT_FOUND,
    INVALID_COMMAND,
    INVALID_PARAM,
    EXECUTE_FAIL,
    TIMEOUT,
    CONTROL_DENIED,
    USER_ACTION_PRIORITY,
    TARGET_UNAVAILABLE,
    CONTEXT_UNAVAILABLE,
};

// Terminal result object carried by the relaxed/PC finish callback; serialized to
// the fixed JSON contract {"result":...,"errorType":...,"errorMessage":...}.
struct RelaxedCommandResult {
    bool success = false;
    RelaxedCommandErrorType errorType = RelaxedCommandErrorType::EXECUTE_FAIL;
    std::string errorMessage;

    static RelaxedCommandResult Success()
    {
        return RelaxedCommandResult { true, RelaxedCommandErrorType::NONE, {} };
    }

    static RelaxedCommandResult Error(RelaxedCommandErrorType type, std::string message = "")
    {
        return RelaxedCommandResult { false, type, std::move(message) };
    }
};

inline constexpr char FALLBACK_CMD_KEY[] = "fallback_cmd";
inline constexpr char CMD_KEY[] = "cmd";
inline constexpr char TYPE_KEY[] = "type";

inline constexpr char BACKPRESS[] = "backpress";
inline constexpr char CONTENT_SWITCH[] = "content_switch";
inline constexpr char FALLBACK[] = "fallback";
inline constexpr char CLICK[] = "click";
inline constexpr char SCROLL[] = "scroll";
inline constexpr char TAP[] = "tap";
inline constexpr char SLIDE[] = "slide";
inline constexpr char KEY[] = "key";
inline constexpr char PRESS_KEY[] = "press_key";
inline constexpr char WHEEL_SCROLL[] = "wheel_scroll";
inline constexpr char PRESS_AND_MOVE[] = "press_and_move";

// Marker key attached to injected key events so the event pipeline can
// distinguish injected input from real user input.
inline constexpr char RELAXED_INJECTED_KEY_MESSAGE[] = "__relaxed_interaction_injected";

// Return status of ExecuteNextStep.
enum class ExecutionState {
    NONE,
    RUNNING,
    SUCCESS,
    FAILED,
    TARGET_NOT_FOUND,
    INTERRUPTED_BY_USER,
    TIMEOUT,
    CONTROL_DENIED,
    CONTEXT_UNAVAILABLE,
    SECURITY_DENIED,
};

// Executor execution result
enum class ExecutorResult {
    SUCCESS,
    FAILED,
    PENDING,
};

// Execute mode
enum class ExecuteMode {
    AUTO = 0,
    SIMULATING,
    TARGET,
};

// Return result of ProcessCommand.
enum class ProcessResult {
    SUCCESS,
    PARSE_ERROR,
    VALIDATION_ERROR,
    NO_VALID_CMD,
};

struct Coordinates {
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
};

// PC mouse click input (button/click count/modifiers extend the legacy click).
struct ClickInput {
    PointF coordinates;
    std::string button = "left";
    int32_t clickCount = 1;
    std::vector<std::string> modifiers;
    bool hasButton = false;
};

// PC key sequence with hold duration.
struct KeyInput {
    std::vector<KeyCode> keys;
    int32_t holdMs = 0;
};

enum class WheelScrollDirection {
    VERTICAL,
    HORIZONTAL,
};

// PC wheel scroll input.
struct WheelInput {
    PointF coordinates;
    int32_t delta = 0;
    WheelScrollDirection direction = WheelScrollDirection::VERTICAL;
    std::vector<std::string> modifiers;
    bool hasCoordinates = false;
};

enum class MoveInputType {
    TOUCH,
    MOUSE,
};

// PC press-and-move input.
struct MoveInput {
    Coordinates coordinates;
    MoveInputType inputType = MoveInputType::TOUCH;
};

enum class RelaxedScrollDirection {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    DOWN,
    UP
};

struct ScrollActionInfo {
    RelaxedScrollDirection direction;
    float distance;
    Coordinates coordinates;
};

struct Command {
    std::string type;
    ExecuteMode mode;
    ScrollActionInfo actionInfo;
    bool isY2Set = false;
};

// P2 conflict matrix: input classes that real user input competes with.
inline bool IsPointerClassCommand(const std::string& type)
{
    return type == CLICK || type == WHEEL_SCROLL || type == PRESS_AND_MOVE || type == SCROLL ||
        type == TAP || type == SLIDE || type == CONTENT_SWITCH;
}

inline bool IsKeyClassCommand(const std::string& type)
{
    return type == PRESS_KEY || type == KEY || type == BACKPRESS;
}

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_RELAXED_INTERACTION_TYPES_H
