/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import {  AttributeModifier } from 'arkui/component/common'
import {  ButtonAttribute } from 'arkui/component/button'

import { AttributeUpdateFlag, CommonMethodModifier } from './CommonMethodModifier';
import { Length } from './component/units';
import { PeerNode } from './PeerNode';
import { ArkButtonPeer } from './component/button';
import { InteropNativeModule } from "@koalaui/interop"
import { TextAttribute } from './component';
export class TextModifier extends CommonMethodModifier implements TextAttribute, AttributeModifier<TextAttribute> {

    applyNormalAttribute(instance: ButtonAttribute): void { }
    applyPressedAttribute(instance: ButtonAttribute): void { }
    applyFocusedAttribute(instance: ButtonAttribute): void { }
    applyDisabledAttribute(instance: ButtonAttribute): void { }
    applySelectedAttribute(instance: ButtonAttribute): void { }
    applyModifierPatch(value: PeerNode): void {
        super.applyModifierPatch(value)
    }
    mergeModifier(value: CommonMethodModifier): void {
        super.mergeModifier(value)
    }
    public draggable(value: boolean | undefined): this {
        return this
    }
}