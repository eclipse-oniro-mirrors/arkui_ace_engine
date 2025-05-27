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
import { ArkCommonMethodPeer, CommonMethod, CustomBuilder, ArkCommonMethodComponent, ArkCommonMethodStyle, UICommonMethod } from "./common"
import { Resource } from "./resource"
import { VoidCallback, Length, ResourceColor, ResourceStr, Padding, Dimension, LocalizedPadding } from "./units"
import { CallbackKind } from "./peers/CallbackKind"
import { CallbackTransformer } from "./../CallbackTransformer"
import { SymbolGlyphModifier, IndicatorStyle, LabelStyle } from "./arkui-external"
import { UICommonBase, AttributeModifier } from "./../handwritten"
import { NodeAttach, remember } from "@koalaui/runtime"
import { ComponentContent } from "./arkui-custom"
import { VerticalAlign } from "./enums"
export class ArkTabContentPeer extends ArkCommonMethodPeer {
    protected constructor(peerPtr: KPointer, id: int32, name: string = "", flags: int32 = 0) {
        super(peerPtr, id, name, flags)
    }
    public static create(component?: ComponentBase, flags: int32 = 0): ArkTabContentPeer {
        const peerId  = PeerNode.nextId()
        const _peerPtr  = ArkUIGeneratedNativeModule._TabContent_construct(peerId, flags)
        const _peer  = new ArkTabContentPeer(_peerPtr, peerId, "TabContent", flags)
        component?.setPeer(_peer)
        return _peer
    }
    setTabContentOptionsAttribute(): void {
        ArkUIGeneratedNativeModule._TabContentInterface_setTabContentOptions(this.peer.ptr)
    }
    tabBar0Attribute(value: string | Resource | CustomBuilder | TabBarOptions): void {
        const thisSerializer : Serializer = Serializer.hold()
        let value_type : int32 = RuntimeType.UNDEFINED
        value_type = runtimeType(value)
        if (RuntimeType.STRING == value_type) {
            thisSerializer.writeInt8(0 as int32)
            const value_0  = value as string
            thisSerializer.writeString(value_0)
        }
        else if (TypeChecker.isResource(value, false, false, false, false, false)) {
            thisSerializer.writeInt8(1 as int32)
            const value_1  = value as Resource
            thisSerializer.writeResource(value_1)
        }
        else if (RuntimeType.FUNCTION == value_type) {
            thisSerializer.writeInt8(2 as int32)
            const value_2  = value as CustomBuilder
            thisSerializer.holdAndWriteCallback(CallbackTransformer.transformFromCustomBuilder(value_2))
        }
        else if (TypeChecker.isTabBarOptions(value, false, false)) {
            thisSerializer.writeInt8(3 as int32)
            const value_3  = value as TabBarOptions
            thisSerializer.writeTabBarOptions(value_3)
        }
        ArkUIGeneratedNativeModule._TabContentAttribute_tabBar0(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
    tabBar1Attribute(value: SubTabBarStyle | BottomTabBarStyle): void {
        const thisSerializer : Serializer = Serializer.hold()
        let value_type : int32 = RuntimeType.UNDEFINED
        value_type = runtimeType(value)
        if (((RuntimeType.OBJECT) == (value_type)) && (TypeChecker.isSubTabBarStyle(value))) {
            thisSerializer.writeInt8(0 as int32)
            const value_0  = value as SubTabBarStyle
            thisSerializer.writeSubTabBarStyle(value_0)
        }
        else if (((RuntimeType.OBJECT) == (value_type)) && (TypeChecker.isBottomTabBarStyle(value))) {
            thisSerializer.writeInt8(1 as int32)
            const value_1  = value as BottomTabBarStyle
            thisSerializer.writeBottomTabBarStyle(value_1)
        }
        ArkUIGeneratedNativeModule._TabContentAttribute_tabBar1(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
    onWillShowAttribute(value: VoidCallback): void {
        const thisSerializer : Serializer = Serializer.hold()
        thisSerializer.holdAndWriteCallback(value)
        ArkUIGeneratedNativeModule._TabContentAttribute_onWillShow(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
    onWillHideAttribute(value: VoidCallback): void {
        const thisSerializer : Serializer = Serializer.hold()
        thisSerializer.holdAndWriteCallback(value)
        ArkUIGeneratedNativeModule._TabContentAttribute_onWillHide(this.peer.ptr, thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
    }
}
export enum SelectedMode {
    INDICATOR = 0,
    BOARD = 1
}
export enum LayoutMode {
    AUTO = 0,
    VERTICAL = 1,
    HORIZONTAL = 2
}
export interface BoardStyle {
    borderRadius?: Length;
}
export interface TabBarIconStyle {
    selectedColor?: ResourceColor;
    unselectedColor?: ResourceColor;
}
export interface TabBarSymbol {
    normal: SymbolGlyphModifier;
    selected?: SymbolGlyphModifier;
}
export interface TabBarOptions {
    icon?: string | Resource;
    text?: string | Resource;
}

export interface TabContentAttribute extends CommonMethod {
    tabBar(value: string | Resource | CustomBuilder | TabBarOptions | SubTabBarStyle | BottomTabBarStyle): this
    onWillShow(value: VoidCallback): this
    onWillHide(value: VoidCallback): this
    attributeModifier(value: AttributeModifier<TabContentAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export interface UITabContentAttribute extends UICommonMethod {
    /** @memo */
    tabBar(value: string | Resource | CustomBuilder | TabBarOptions | SubTabBarStyle | BottomTabBarStyle): this
    /** @memo */
    onWillShow(value: VoidCallback): this
    /** @memo */
    onWillHide(value: VoidCallback): this
    /** @memo */
    attributeModifier(value: AttributeModifier<TabContentAttribute> | AttributeModifier<CommonMethod> | undefined): this
}
export class ArkTabContentStyle extends ArkCommonMethodStyle implements TabContentAttribute {
    tabBar_value?: string | Resource | CustomBuilder | TabBarOptions
    onWillShow_value?: VoidCallback
    onWillHide_value?: VoidCallback
    public tabBar(value: string | Resource | CustomBuilder | TabBarOptions | SubTabBarStyle | BottomTabBarStyle): this {
        return this
    }
    public onWillShow(value: VoidCallback): this {
        return this
    }
    public onWillHide(value: VoidCallback): this {
        return this
    }
    public attributeModifier(value: AttributeModifier<TabContentAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        throw new Error("Not implemented")
    }
}
/** @memo:stable */
export class ArkTabContentComponent extends ArkCommonMethodComponent implements UITabContentAttribute {
    getPeer(): ArkTabContentPeer {
        return (this.peer as ArkTabContentPeer)
    }
    /** @memo */
    public setTabContentOptions(): this {
        if (this.checkPriority("setTabContentOptions")) {
            this.getPeer()?.setTabContentOptionsAttribute()
            return this
        }
        return this
    }
    /** @memo */
    public tabBar(value: SubTabBarStyle | BottomTabBarStyle | string | Resource | CustomBuilder | TabBarOptions): this {
        if (this.checkPriority("tabBar")) {
            const value_type = runtimeType(value)
            if ((((RuntimeType.OBJECT) == (value_type)) && (TypeChecker.isSubTabBarStyle(value))) || (((RuntimeType.OBJECT) == (value_type)) && (TypeChecker.isBottomTabBarStyle(value)))) {
                const value_casted = value as (SubTabBarStyle | BottomTabBarStyle)
                this.getPeer()?.tabBar1Attribute(value_casted)
                return this
            }
            if ((RuntimeType.STRING == value_type) || (TypeChecker.isResource(value, false, false, false, false, false)) || (RuntimeType.FUNCTION == value_type) || (TypeChecker.isTabBarOptions(value, false, false))) {
                const value_casted = value as (string | Resource | CustomBuilder | TabBarOptions)
                this.getPeer()?.tabBar0Attribute(value_casted)
                return this
            }
            throw new Error("Can not select appropriate overload")
        }
        return this
    }
    /** @memo */
    public onWillShow(value: VoidCallback): this {
        if (this.checkPriority("onWillShow")) {
            const value_casted = value as (VoidCallback)
            this.getPeer()?.onWillShowAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public onWillHide(value: VoidCallback): this {
        if (this.checkPriority("onWillHide")) {
            const value_casted = value as (VoidCallback)
            this.getPeer()?.onWillHideAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public attributeModifier(value: AttributeModifier<TabContentAttribute> | AttributeModifier<CommonMethod> | undefined): this {
        console.log("attributeModifier() not implemented")
        return this
    }
    public applyAttributesFinish(): void {
        // we call this function outside of class, so need to make it public
        super.applyAttributesFinish()
    }
}
/** @memo */
export function ArkTabContent(
    /** @memo */
    style: ((attributes: UITabContentAttribute) => void) | undefined,
    /** @memo */
    content_: (() => void) | undefined,
    
): void {
    const receiver = remember(() => {
        return new ArkTabContentComponent()
    })
    NodeAttach<ArkTabContentPeer>((): ArkTabContentPeer => ArkTabContentPeer.create(receiver), (_: ArkTabContentPeer) => {
        receiver.setTabContentOptions()
        style?.(receiver)
        content_?.()
        receiver.applyAttributesFinish()
    })
}
export class SubTabBarStyle {
    _content?: ResourceStr | ResourceStr | ComponentContent | undefined
    _indicator?: IndicatorStyle | undefined
    _selectedMode?: SelectedMode | undefined
    _board?: BoardStyle | undefined
    _labelStyle?: LabelStyle | undefined
    _padding?: Padding | Dimension | LocalizedPadding | undefined
    _id?: string | undefined
    constructor(content: ResourceStr | ResourceStr | ComponentContent) {
        this._content = content
    }
    public static of(content: ResourceStr | ResourceStr | ComponentContent): SubTabBarStyle {
        return new SubTabBarStyle(content)
    }
    indicator(value: IndicatorStyle): this {
        this._indicator = value
        return this
    }
    selectedMode(value: SelectedMode): this {
        this._selectedMode = value
        return this
    }
    board(value: BoardStyle): this {
        this._board = value
        return this
    }
    labelStyle(value: LabelStyle): this {
        this._labelStyle = value
        return this
    }
    public padding(value: Padding | Dimension | LocalizedPadding): this {
        this._padding = value
        return this
    }
    id(value: string): this {
        this._id = value
        return this
    }
}
export class BottomTabBarStyle {
    _icon?: ResourceStr | TabBarSymbol | undefined
    _text?: ResourceStr | undefined
    _labelStyle?: LabelStyle | undefined
    _padding?: Padding | Dimension | LocalizedPadding | undefined
    _layoutMode?: LayoutMode | undefined
    _verticalAlign?: VerticalAlign | undefined
    _symmetricExtensible?: boolean | undefined
    _id?: string | undefined
    _iconStyle?: TabBarIconStyle | undefined
    constructor(icon: ResourceStr | TabBarSymbol, text: ResourceStr) {
        this._icon = icon
        this._text = text
    }
    static of(icon: ResourceStr | TabBarSymbol, text: ResourceStr): BottomTabBarStyle {
        return new BottomTabBarStyle(icon, text)
    }
    labelStyle(value: LabelStyle): this {
        this._labelStyle = value
        return this
    }
    padding(value: Padding | Dimension | LocalizedPadding): this {
        this._padding = value
        return this
    }
    layoutMode(value: LayoutMode): this {
        this._layoutMode = value
        return this
    }
    verticalAlign(value: VerticalAlign): this {
        this._verticalAlign = value
        return this
    }
    symmetricExtensible(value: boolean): this {
        this._symmetricExtensible = value
        return this
    }
    id(value: string): this {
        this._id = value
        return this
    }
    iconStyle(style: TabBarIconStyle): this {
        this._iconStyle = style
        return this
    }
}
