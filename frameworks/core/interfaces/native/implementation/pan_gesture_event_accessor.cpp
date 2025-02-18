/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/implementation/pan_gesture_event_peer.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace PanGestureEventAccessor {
void DestroyPeerImpl(Ark_PanGestureEvent peer)
{
    delete peer;
}
Ark_PanGestureEvent CtorImpl()
{
    return new PanGestureEventPeer();
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
Ark_Int32 GetOffsetXImpl(Ark_PanGestureEvent peer)
{
    return {};
}
void SetOffsetXImpl(Ark_PanGestureEvent peer,
                    const Ark_Number* offsetX)
{
}
Ark_Int32 GetOffsetYImpl(Ark_PanGestureEvent peer)
{
    return {};
}
void SetOffsetYImpl(Ark_PanGestureEvent peer,
                    const Ark_Number* offsetY)
{
}
Ark_Int32 GetVelocityXImpl(Ark_PanGestureEvent peer)
{
    return {};
}
void SetVelocityXImpl(Ark_PanGestureEvent peer,
                      const Ark_Number* velocityX)
{
}
Ark_Int32 GetVelocityYImpl(Ark_PanGestureEvent peer)
{
    return {};
}
void SetVelocityYImpl(Ark_PanGestureEvent peer,
                      const Ark_Number* velocityY)
{
}
Ark_Int32 GetVelocityImpl(Ark_PanGestureEvent peer)
{
    return {};
}
void SetVelocityImpl(Ark_PanGestureEvent peer,
                     const Ark_Number* velocity)
{
}
} // PanGestureEventAccessor
const GENERATED_ArkUIPanGestureEventAccessor* GetPanGestureEventAccessor()
{
    static const GENERATED_ArkUIPanGestureEventAccessor PanGestureEventAccessorImpl {
        PanGestureEventAccessor::DestroyPeerImpl,
        PanGestureEventAccessor::CtorImpl,
        PanGestureEventAccessor::GetFinalizerImpl,
        PanGestureEventAccessor::GetOffsetXImpl,
        PanGestureEventAccessor::SetOffsetXImpl,
        PanGestureEventAccessor::GetOffsetYImpl,
        PanGestureEventAccessor::SetOffsetYImpl,
        PanGestureEventAccessor::GetVelocityXImpl,
        PanGestureEventAccessor::SetVelocityXImpl,
        PanGestureEventAccessor::GetVelocityYImpl,
        PanGestureEventAccessor::SetVelocityYImpl,
        PanGestureEventAccessor::GetVelocityImpl,
        PanGestureEventAccessor::SetVelocityImpl,
    };
    return &PanGestureEventAccessorImpl;
}
}
