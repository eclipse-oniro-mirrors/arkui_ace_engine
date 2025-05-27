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
import { ArkSecurityComponentMethodPeer, SecurityComponentMethod, ArkSecurityComponentMethodComponent, ArkSecurityComponentMethodStyle, UISecurityComponentMethod } from "./security_component"
import { ClickEvent } from "./common"
import { CallbackKind } from "./peers/CallbackKind"
import { CallbackTransformer } from "./../CallbackTransformer"
import { ButtonType } from "./button"
import { UICommonBase, AttributeModifier } from "./../handwritten"
import { NodeAttach, remember } from "@koalaui/runtime"
export class ArkLocationButtonPeer extends ArkSecurityComponentMethodPeer {
    protected constructor(peerPtr: KPointer, id: int32, name: string = "", flags: int32 = 0) {
        super(peerPtr, id, name, flags)
    }
    public static create(component?: ComponentBase, flags: int32 = 0): ArkLocationButtonPeer {
        const peerId  = PeerNode.nextId()
        const _peerPtr  = ArkUIGeneratedNativeModule._LocationButton_construct(peerId, flags)
        const _peer  = new ArkLocationButtonPeer(_peerPtr, peerId, "LocationButton", flags)
        component?.setPeer(_peer)
        return _peer
    }
    setLocationButtonOptions0Attribute(): void {
        ArkUIGeneratedNativeModule._LocationButtonInterface_setLocationButtonOptions0(this.peer.ptr)
    }
    setLocationButtonOptions1Attribute(options: LocationButtonOptions): void {
        const thisSerializer : Serializer = Serializer.hold()
        thisSerializer.writeLocationButtonOptions(options)
        ArkUIGeneratedNativeModule._LocationButtonInterface_setLocationButtonOptions1(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
    onClickAttribute(value: ((event: ClickEvent,result: LocationButtonOnClickResult) => void)): void {
        const thisSerializer : Serializer = Serializer.hold()
        thisSerializer.holdAndWriteCallback(value)
        ArkUIGeneratedNativeModule._LocationButtonAttribute_onClick(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
}
export enum LocationIconStyle {
    FULL_FILLED = 0,
    LINES = 1
}
export enum LocationDescription {
    CURRENT_LOCATION = 0,
    ADD_LOCATION = 1,
    SELECT_LOCATION = 2,
    SHARE_LOCATION = 3,
    SEND_LOCATION = 4,
    LOCATING = 5,
    LOCATION = 6,
    SEND_CURRENT_LOCATION = 7,
    RELOCATION = 8,
    PUNCH_IN = 9,
    CURRENT_POSITION = 10
}
export interface LocationButtonOptions {
    icon?: LocationIconStyle;
    text?: LocationDescription;
    buttonType?: ButtonType;
}
export enum LocationButtonOnClickResult {
    SUCCESS = 0,
    TEMPORARY_AUTHORIZATION_FAILED = 1
}

export type Callback_ClickEvent_LocationButtonOnClickResult_Void = (event: ClickEvent, result: LocationButtonOnClickResult) => void;
export interface LocationButtonAttribute extends SecurityComponentMethod {
    onClick(value: ((event: ClickEvent,result: LocationButtonOnClickResult) => void)): this
    attributeModifier(value: AttributeModifier<LocationButtonAttribute> | AttributeModifier<SecurityComponentMethod> | undefined): this
}
export interface UILocationButtonAttribute extends UISecurityComponentMethod {
    /** @memo */
    onClick(value: ((event: ClickEvent,result: LocationButtonOnClickResult) => void)): this
    /** @memo */
    attributeModifier(value: AttributeModifier<LocationButtonAttribute> | AttributeModifier<SecurityComponentMethod> | undefined): this
}
export class ArkLocationButtonStyle extends ArkSecurityComponentMethodStyle implements LocationButtonAttribute {
    onClick_value?: ((event: ClickEvent,result: LocationButtonOnClickResult) => void)
    public onClick(value: ((event: ClickEvent,result: LocationButtonOnClickResult) => void)): this {
        return this
    }
    public attributeModifier(value: AttributeModifier<LocationButtonAttribute> | AttributeModifier<SecurityComponentMethod> | undefined): this {
        throw new Error("Not implemented")
    }
}
/** @memo:stable */
export class ArkLocationButtonComponent extends ArkSecurityComponentMethodComponent implements UILocationButtonAttribute {
    getPeer(): ArkLocationButtonPeer {
        return (this.peer as ArkLocationButtonPeer)
    }
    /** @memo */
    public setLocationButtonOptions(options?: LocationButtonOptions): this {
        if (this.checkPriority("setLocationButtonOptions")) {
            const options_type = runtimeType(options)
            this.getPeer()?.setLocationButtonOptions0Attribute()
            return this
        }
        return this
    }
    /** @memo */
    public onClick(value: ((event: ClickEvent,result: LocationButtonOnClickResult) => void)): this {
        if (this.checkPriority("onClick")) {
            const value_casted = value as (((event: ClickEvent,result: LocationButtonOnClickResult) => void))
            this.getPeer()?.onClickAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public attributeModifier(value: AttributeModifier<LocationButtonAttribute> | AttributeModifier<SecurityComponentMethod> | undefined): this {
        console.log("attributeModifier() not implemented")
        return this
    }
    public applyAttributesFinish(): void {
        // we call this function outside of class, so need to make it public
        super.applyAttributesFinish()
    }
}
/** @memo */
export function ArkLocationButton(
    /** @memo */
    style: ((attributes: UILocationButtonAttribute) => void) | undefined,
    /** @memo */
    content_: (() => void) | undefined,
    options?: LocationButtonOptions,
): void {
    const receiver = remember(() => {
        return new ArkLocationButtonComponent()
    })
    NodeAttach<ArkLocationButtonPeer>((): ArkLocationButtonPeer => ArkLocationButtonPeer.create(receiver), (_: ArkLocationButtonPeer) => {
        receiver.setLocationButtonOptions(options)
        style?.(receiver)
        content_?.()
        receiver.applyAttributesFinish()
    })
}
