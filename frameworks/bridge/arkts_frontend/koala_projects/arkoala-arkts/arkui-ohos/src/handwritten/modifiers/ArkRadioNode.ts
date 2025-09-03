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

import { ArkBaseNode } from "./ArkBaseNode";
import { ArkRadioPeer, RadioOptions, RadioAttribute, OnRadioChangeCallback, RadioStyle, RadioConfiguration, Bindable } from "../../component";
import { ContentModifier } from "@component_handwritten/common";
import { RadioOpsHandWritten } from "../RadioOpsHandWritten";
import { RuntimeType, runtimeType } from "@koalaui/interop";
export class ArkRadioNode extends ArkBaseNode implements RadioAttribute {
    constructParam(...param: Object[]): this {
        if (param.length > 1) {
            throw new Error('more than 1 parameters');
        } 
        let options_casted: RadioOptions = param[0] as (RadioOptions);
        this.getPeer()?.setRadioOptionsAttribute(options_casted);
        return this;
    }
    getPeer(): ArkRadioPeer {
        return this.peer as ArkRadioPeer;
    }
    initialize(options: RadioOptions): this {
        const options_casted = options as (RadioOptions);
        this.getPeer()?.setRadioOptionsAttribute(options_casted);
        return this;
    }
    setRadioOptions(options: RadioOptions): this {
        const options_casted = options as (RadioOptions);
        this.getPeer()?.setRadioOptionsAttribute(options_casted);
        return this;
    }
    checked(value: boolean | Bindable<boolean> | undefined): this {
        const value_type = runtimeType(value);
        if ((RuntimeType.BOOLEAN === value_type) || (RuntimeType.UNDEFINED === value_type)) {
            const value_casted = value as (boolean | undefined);
            this.getPeer()?.checked0Attribute(value_casted);
            return this;
        }
        RadioOpsHandWritten.hookRadioAttributeCheckedImpl(this.getPeer().peer.ptr,
            (value as Bindable<boolean>));
        return this;
    }
    onChange(value: ((isVisible: boolean) => void) | OnRadioChangeCallback | undefined): this {
        const value_casted = value as (((isVisible: boolean) => void) | OnRadioChangeCallback | undefined);
        this.getPeer()?.onChange0Attribute(value_casted);
        return this;
    }
    radioStyle(value: RadioStyle | undefined): this {
        const value_casted = value as (RadioStyle | undefined);
        this.getPeer()?.radioStyleAttribute(value_casted);        
        return this;
    }
    contentModifier(value: ContentModifier<RadioConfiguration> | undefined): this {
        const value_casted = value as (ContentModifier<RadioConfiguration> | undefined);
        this.getPeer()?.contentModifier0Attribute(value_casted);
        return this;
    }
    _onChangeEvent_checked(callback: (select: boolean | undefined) => void): void {
        const callback_casted = callback as (((select: boolean | undefined) => void))
        this.getPeer()?._onChangeEvent_checkedAttribute(callback_casted)
        return;
    }
}