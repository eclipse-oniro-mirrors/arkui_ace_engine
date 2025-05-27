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


// WARNING! THIS FILE IS AUTO-GENERATED, DO NOT MAKE CHANGES, THEY WILL BE LOST ON NEXT GENERATION!

import { int32, int64, float32 } from "@koalaui/common"
import { nullptr, KPointer, KInt, KBoolean, KStringPtr, runtimeType, RuntimeType, MaterializedBase, toPeerPtr, wrapCallback, NativeBuffer } from "@koalaui/interop"
import { Serializer } from "./peers/Serializer"
import { ComponentBase } from "./../ComponentBase"
import { PeerNode } from "./../PeerNode"
import { ArkUIGeneratedNativeModule, TypeChecker } from "#components"
import { ArkCommonMethodPeer, CommonMethod, ArkCommonMethodComponent, ArkCommonMethodStyle, UICommonMethod } from "./common"
import { NodeController } from "./ohos.arkui.node"
import { CallbackKind } from "./peers/CallbackKind"
import { CallbackTransformer } from "./../CallbackTransformer"
import { UICommonBase, AttributeModifier } from "./../handwritten"
import { NodeAttach, remember } from "@koalaui/runtime"
export class ArkNodeContainerPeer extends ArkCommonMethodPeer {
    protected constructor(peerPtr: KPointer, id: int32, name: string = "", flags: int32 = 0) {
        super(peerPtr, id, name, flags)
    }
    public static create(component?: ComponentBase, flags: int32 = 0): ArkNodeContainerPeer {
        const peerId  = PeerNode.nextId()
        const _peerPtr  = ArkUIGeneratedNativeModule._NodeContainer_construct(peerId, flags)
        const _peer  = new ArkNodeContainerPeer(_peerPtr, peerId, "NodeContainer", flags)
        component?.setPeer(_peer)
        return _peer
    }
    setNodeContainerOptionsAttribute(controller: NodeController): void {
        const thisSerializer : Serializer = Serializer.hold()
        thisSerializer.writeNodeController(controller)
        ArkUIGeneratedNativeModule._NodeContainerInterface_setNodeContainerOptions(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
}

export interface NodeContainerAttribute extends CommonMethod {
    attributeModifier(value: AttributeModifier<NodeContainerAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export interface UINodeContainerAttribute extends UICommonMethod {
    /** @memo */
    attributeModifier(value: AttributeModifier<NodeContainerAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export class ArkNodeContainerStyle extends ArkCommonMethodStyle implements NodeContainerAttribute {
    public attributeModifier(value: AttributeModifier<NodeContainerAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        throw new Error("Not implemented")
    }
}
/** @memo:stable */
export class ArkNodeContainerComponent extends ArkCommonMethodComponent implements UINodeContainerAttribute {
    getPeer(): ArkNodeContainerPeer {
        return (this.peer as ArkNodeContainerPeer)
    }
    /** @memo */
    public setNodeContainerOptions(controller: NodeController): this {
        if (this.checkPriority("setNodeContainerOptions")) {
            const controller_casted = controller as (NodeController)
            this.getPeer()?.setNodeContainerOptionsAttribute(controller_casted)
            return this
        }
        return this
    }
    /** @memo */
    public attributeModifier(value: AttributeModifier<NodeContainerAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        console.log("attributeModifier() not implemented")
        return this
    }
    public applyAttributesFinish(): void {
        // we call this function outside of class, so need to make it public
        super.applyAttributesFinish()
    }
}
/** @memo */
export function ArkNodeContainer(
    /** @memo */
    style: ((attributes: UINodeContainerAttribute) => void) | undefined,
    /** @memo */
    content_: (() => void) | undefined,
    controller: NodeController,
): void {
    const receiver = remember(() => {
        return new ArkNodeContainerComponent()
    })
    NodeAttach<ArkNodeContainerPeer>((): ArkNodeContainerPeer => ArkNodeContainerPeer.create(receiver), (_: ArkNodeContainerPeer) => {
        receiver.setNodeContainerOptions(controller)
        style?.(receiver)
        content_?.()
        receiver.applyAttributesFinish()
    })
}
