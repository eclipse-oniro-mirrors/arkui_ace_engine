/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/event/mouse_event.h"

#include "base/log/log.h"
#include "core/accessibility/accessibility_utils.h"
#include "core/common/ace_application_info.h"
#include "core/components_ng/base/frame_node.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace {
bool HoverEventTarget::HandleHoverEvent(bool isHovered, const MouseEvent& event)
{
    if (!onHoverEventCallback_) {
        return false;
    }
    HoverInfo hoverInfo;
    hoverInfo.SetTimeStamp(event.time);
    hoverInfo.SetDeviceId(event.deviceId);
    hoverInfo.SetSourceDevice(event.sourceType);
    hoverInfo.SetSourceTool(event.sourceTool);
    hoverInfo.SetPressedKeyCodes(event.pressedKeyCodes_);
    onHoverEventCallback_(isHovered, hoverInfo);
    return !hoverInfo.IsStopPropagation();
}
} // namespace OHOS::Ace
