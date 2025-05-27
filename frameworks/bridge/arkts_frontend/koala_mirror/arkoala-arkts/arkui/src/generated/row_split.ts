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
import { CallbackKind } from "./peers/CallbackKind"
import { CallbackTransformer } from "./../CallbackTransformer"
import { UICommonBase, AttributeModifier } from "./../handwritten"
import { NodeAttach, remember } from "@koalaui/runtime"
export class ArkRowSplitPeer extends ArkCommonMethodPeer {
    protected constructor(peerPtr: KPointer, id: int32, name: string = "", flags: int32 = 0) {
        super(peerPtr, id, name, flags)
    }
    public static create(component?: ComponentBase, flags: int32 = 0): ArkRowSplitPeer {
        const peerId  = PeerNode.nextId()
        const _peerPtr  = ArkUIGeneratedNativeModule._RowSplit_construct(peerId, flags)
        const _peer  = new ArkRowSplitPeer(_peerPtr, peerId, "RowSplit", flags)
        component?.setPeer(_peer)
        return _peer
    }
    setRowSplitOptionsAttribute(): void {
        ArkUIGeneratedNativeModule._RowSplitInterface_setRowSplitOptions(this.peer.ptr)
    }
    resizeableAttribute(value: boolean): void {
        ArkUIGeneratedNativeModule._RowSplitAttribute_resizeable(this.peer.ptr, value ? 1 : 0)
    }
}

export interface RowSplitAttribute extends CommonMethod {
    resizeable(value: boolean): this
    attributeModifier(value: AttributeModifier<RowSplitAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export interface UIRowSplitAttribute extends UICommonMethod {
    /** @memo */
    resizeable(value: boolean): this
    /** @memo */
    attributeModifier(value: AttributeModifier<RowSplitAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export class ArkRowSplitStyle extends ArkCommonMethodStyle implements RowSplitAttribute {
    resizeable_value?: boolean
    public resizeable(value: boolean): this {
        return this
    }
    public attributeModifier(value: AttributeModifier<RowSplitAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        throw new Error("Not implemented")
    }
}
/** @memo:stable */
export class ArkRowSplitComponent extends ArkCommonMethodComponent implements UIRowSplitAttribute {
    getPeer(): ArkRowSplitPeer {
        return (this.peer as ArkRowSplitPeer)
    }
    /** @memo */
    public setRowSplitOptions(): this {
        if (this.checkPriority("setRowSplitOptions")) {
            this.getPeer()?.setRowSplitOptionsAttribute()
            return this
        }
        return this
    }
    /** @memo */
    public resizeable(value: boolean): this {
        if (this.checkPriority("resizeable")) {
            const value_casted = value as (boolean)
            this.getPeer()?.resizeableAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public attributeModifier(value: AttributeModifier<RowSplitAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        console.log("attributeModifier() not implemented")
        return this
    }
    public applyAttributesFinish(): void {
        // we call this function outside of class, so need to make it public
        super.applyAttributesFinish()
    }
}
/** @memo */
export function ArkRowSplit(
    /** @memo */
    style: ((attributes: UIRowSplitAttribute) => void) | undefined,
    /** @memo */
    content_: (() => void) | undefined,
    
): void {
    const receiver = remember(() => {
        return new ArkRowSplitComponent()
    })
    NodeAttach<ArkRowSplitPeer>((): ArkRowSplitPeer => ArkRowSplitPeer.create(receiver), (_: ArkRowSplitPeer) => {
        receiver.setRowSplitOptions()
        style?.(receiver)
        content_?.()
        receiver.applyAttributesFinish()
    })
}
