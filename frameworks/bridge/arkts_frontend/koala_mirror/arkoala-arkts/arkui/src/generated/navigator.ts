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
export class ArkNavigatorPeer extends ArkCommonMethodPeer {
    protected constructor(peerPtr: KPointer, id: int32, name: string = "", flags: int32 = 0) {
        super(peerPtr, id, name, flags)
    }
    public static create(component?: ComponentBase, flags: int32 = 0): ArkNavigatorPeer {
        const peerId  = PeerNode.nextId()
        const _peerPtr  = ArkUIGeneratedNativeModule._Navigator_construct(peerId, flags)
        const _peer  = new ArkNavigatorPeer(_peerPtr, peerId, "Navigator", flags)
        component?.setPeer(_peer)
        return _peer
    }
    setNavigatorOptions0Attribute(value?: Literal_String_target_NavigationType_type): void {
        const thisSerializer : Serializer = Serializer.hold()
        let value_type : int32 = RuntimeType.UNDEFINED
        value_type = runtimeType(value)
        thisSerializer.writeInt8(value_type as int32)
        if ((RuntimeType.UNDEFINED) != (value_type)) {
            const value_value  = value!
            const value_value_target  = value_value.target
            thisSerializer.writeString(value_value_target)
            const value_value_type  = value_value.type
            let value_value_type_type : int32 = RuntimeType.UNDEFINED
            value_value_type_type = runtimeType(value_value_type)
            thisSerializer.writeInt8(value_value_type_type as int32)
            if ((RuntimeType.UNDEFINED) != (value_value_type_type)) {
                const value_value_type_value  = (value_value_type as NavigationType)
                thisSerializer.writeInt32(TypeChecker.NavigationType_ToNumeric(value_value_type_value))
            }
        }
        ArkUIGeneratedNativeModule._NavigatorInterface_setNavigatorOptions0(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
    setNavigatorOptions1Attribute(): void {
        ArkUIGeneratedNativeModule._NavigatorInterface_setNavigatorOptions1(this.peer.ptr)
    }
    activeAttribute(value: boolean): void {
        ArkUIGeneratedNativeModule._NavigatorAttribute_active(this.peer.ptr, value ? 1 : 0)
    }
    typeAttribute(value: NavigationType): void {
        ArkUIGeneratedNativeModule._NavigatorAttribute_type(this.peer.ptr, TypeChecker.NavigationType_ToNumeric(value))
    }
    targetAttribute(value: string): void {
        ArkUIGeneratedNativeModule._NavigatorAttribute_target(this.peer.ptr, value)
    }
    paramsAttribute(value: Object): void {
        const thisSerializer : Serializer = Serializer.hold()
        thisSerializer.holdAndWriteObject(value)
        ArkUIGeneratedNativeModule._NavigatorAttribute_params(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
}
export enum NavigationType {
    PUSH = 0,
    Push = 0,
    BACK = 1,
    Back = 1,
    REPLACE = 2,
    Replace = 2
}
export interface Literal_String_target_NavigationType_type {
    target: string;
    type?: NavigationType;
}

export interface NavigatorAttribute extends CommonMethod {
    active(value: boolean): this
    type(value: NavigationType): this
    target(value: string): this
    params(value: Object): this
    attributeModifier(value: AttributeModifier<NavigatorAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export interface UINavigatorAttribute extends UICommonMethod {
    /** @memo */
    active(value: boolean): this
    /** @memo */
    type(value: NavigationType): this
    /** @memo */
    target(value: string): this
    /** @memo */
    params(value: Object): this
    /** @memo */
    attributeModifier(value: AttributeModifier<NavigatorAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export class ArkNavigatorStyle extends ArkCommonMethodStyle implements NavigatorAttribute {
    active_value?: boolean
    type_value?: NavigationType
    target_value?: string
    params_value?: Object
    public active(value: boolean): this {
        return this
    }
    public type(value: NavigationType): this {
        return this
    }
    public target(value: string): this {
        return this
    }
    public params(value: Object): this {
        return this
    }
    public attributeModifier(value: AttributeModifier<NavigatorAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        throw new Error("Not implemented")
    }
}
/** @memo:stable */
export class ArkNavigatorComponent extends ArkCommonMethodComponent implements UINavigatorAttribute {
    getPeer(): ArkNavigatorPeer {
        return (this.peer as ArkNavigatorPeer)
    }
    /** @memo */
    public setNavigatorOptions(value?: Literal_String_target_NavigationType_type): this {
        if (this.checkPriority("setNavigatorOptions")) {
            const value_type = runtimeType(value)
            this.getPeer()?.setNavigatorOptions1Attribute()
            return this
        }
        return this
    }
    /** @memo */
    public active(value: boolean): this {
        if (this.checkPriority("active")) {
            const value_casted = value as (boolean)
            this.getPeer()?.activeAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public type(value: NavigationType): this {
        if (this.checkPriority("type")) {
            const value_casted = value as (NavigationType)
            this.getPeer()?.typeAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public target(value: string): this {
        if (this.checkPriority("target")) {
            const value_casted = value as (string)
            this.getPeer()?.targetAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public params(value: Object): this {
        if (this.checkPriority("params")) {
            const value_casted = value as (Object)
            this.getPeer()?.paramsAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public attributeModifier(value: AttributeModifier<NavigatorAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        console.log("attributeModifier() not implemented")
        return this
    }
    public applyAttributesFinish(): void {
        // we call this function outside of class, so need to make it public
        super.applyAttributesFinish()
    }
}
/** @memo */
export function ArkNavigator(
    /** @memo */
    style: ((attributes: UINavigatorAttribute) => void) | undefined,
    /** @memo */
    content_: (() => void) | undefined,
    value?: Literal_String_target_NavigationType_type,
): void {
    const receiver = remember(() => {
        return new ArkNavigatorComponent()
    })
    NodeAttach<ArkNavigatorPeer>((): ArkNavigatorPeer => ArkNavigatorPeer.create(receiver), (_: ArkNavigatorPeer) => {
        receiver.setNavigatorOptions(value)
        style?.(receiver)
        content_?.()
        receiver.applyAttributesFinish()
    })
}
