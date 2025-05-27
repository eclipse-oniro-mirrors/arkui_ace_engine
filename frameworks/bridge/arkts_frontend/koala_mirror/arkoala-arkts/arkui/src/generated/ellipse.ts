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
import { ArkCommonShapeMethodPeer, CommonShapeMethod, ArkCommonShapeMethodComponent, ArkCommonShapeMethodStyle, UICommonShapeMethod, ArkCommonMethodComponent, ArkCommonMethodStyle, CommonMethod, UICommonMethod } from "./common"
import { CallbackKind } from "./peers/CallbackKind"
import { CallbackTransformer } from "./../CallbackTransformer"
import { UICommonBase, AttributeModifier } from "./../handwritten"
import { NodeAttach, remember } from "@koalaui/runtime"
export class ArkEllipsePeer extends ArkCommonShapeMethodPeer {
    protected constructor(peerPtr: KPointer, id: int32, name: string = "", flags: int32 = 0) {
        super(peerPtr, id, name, flags)
    }
    public static create(component?: ComponentBase, flags: int32 = 0): ArkEllipsePeer {
        const peerId  = PeerNode.nextId()
        const _peerPtr  = ArkUIGeneratedNativeModule._Ellipse_construct(peerId, flags)
        const _peer  = new ArkEllipsePeer(_peerPtr, peerId, "Ellipse", flags)
        component?.setPeer(_peer)
        return _peer
    }
    setEllipseOptionsAttribute(options?: EllipseOptions): void {
        const thisSerializer : Serializer = Serializer.hold()
        let options_type : int32 = RuntimeType.UNDEFINED
        options_type = runtimeType(options)
        thisSerializer.writeInt8(options_type as int32)
        if ((RuntimeType.UNDEFINED) != (options_type)) {
            const options_value  = options!
            thisSerializer.writeEllipseOptions(options_value)
        }
        ArkUIGeneratedNativeModule._EllipseInterface_setEllipseOptions(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
}
export interface EllipseOptions {
    width?: string | number;
    height?: string | number;
}

export interface EllipseAttribute extends CommonShapeMethod {
    attributeModifier(value: AttributeModifier<EllipseAttribute> | AttributeModifier<CommonShapeMethod> | AttributeModifier<CommonMethod> | undefined): this
}
export interface UIEllipseAttribute extends UICommonShapeMethod {
    /** @memo */
    attributeModifier(value: AttributeModifier<EllipseAttribute> | AttributeModifier<CommonShapeMethod> | AttributeModifier<CommonMethod> | undefined): this
}
export class ArkEllipseStyle extends ArkCommonShapeMethodStyle implements EllipseAttribute {
    public attributeModifier(value: AttributeModifier<EllipseAttribute> | AttributeModifier<CommonShapeMethod> | AttributeModifier<CommonMethod> | undefined): this {
        throw new Error("Not implemented")
    }
}
/** @memo:stable */
export class ArkEllipseComponent extends ArkCommonShapeMethodComponent implements UIEllipseAttribute {
    getPeer(): ArkEllipsePeer {
        return (this.peer as ArkEllipsePeer)
    }
    /** @memo */
    public setEllipseOptions(options?: EllipseOptions): this {
        if (this.checkPriority("setEllipseOptions")) {
            const options_casted = options as (EllipseOptions | undefined)
            this.getPeer()?.setEllipseOptionsAttribute(options_casted)
            return this
        }
        return this
    }
    /** @memo */
    public attributeModifier(value: AttributeModifier<EllipseAttribute> | AttributeModifier<CommonShapeMethod> | AttributeModifier<CommonMethod> | undefined): this {
        console.log("attributeModifier() not implemented")
        return this
    }
    public applyAttributesFinish(): void {
        // we call this function outside of class, so need to make it public
        super.applyAttributesFinish()
    }
}
/** @memo */
export function ArkEllipse(
    /** @memo */
    style: ((attributes: UIEllipseAttribute) => void) | undefined,
    /** @memo */
    content_: (() => void) | undefined,
    options?: EllipseOptions,
): void {
    const receiver = remember(() => {
        return new ArkEllipseComponent()
    })
    NodeAttach<ArkEllipsePeer>((): ArkEllipsePeer => ArkEllipsePeer.create(receiver), (_: ArkEllipsePeer) => {
        receiver.setEllipseOptions(options)
        style?.(receiver)
        content_?.()
        receiver.applyAttributesFinish()
    })
}
