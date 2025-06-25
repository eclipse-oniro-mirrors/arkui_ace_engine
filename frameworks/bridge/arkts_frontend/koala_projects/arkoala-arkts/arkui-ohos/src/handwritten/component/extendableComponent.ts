/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

import { int32 } from "@koalaui/common"
import { UIContext } from "@ohos/arkui/UIContext"
import { IProvideDecoratedVariable } from "../stateManagement/decorator";
import { PeerNode } from "../PeerNode";

export interface LifeCycle {
    aboutToAppear(): void {}
    aboutToDisappear(): void {}
    onDidBuild(): void {}
    /** @memo */
    build(): void
}

export interface IExtendableComponent {
    getUIContext(): UIContext;
    getUniqueId(): int32;
    getPeerNode(): PeerNode | undefined;
}

export abstract class ExtendableComponent implements LifeCycle {
    static current: object | undefined = undefined;

    private parent_: ExtendableComponent | undefined;
    private providedVars_: Map<string, IProvideDecoratedVariable<object>> = new Map<string, IProvideDecoratedVariable<object>>();
    private delegate_?: IExtendableComponent;

    constructor() {
        this.parent_ = ExtendableComponent.current as (ExtendableComponent | undefined);
    }

    setDelegate(delegate: IExtendableComponent): void {
        this.delegate_ = delegate;
    }

    addProvidedVar<T>(providedPropName: string, store: IProvideDecoratedVariable<T>, allowOverride?: boolean | undefined): void {
        if (!allowOverride && this.findProvide<T>(providedPropName)) {
            throw new ReferenceError(`Duplicate @Provide property with name ${providedPropName}. Property with this name is provided by one of the ancestor Component already.`);
        }
        this.providedVars_.set(providedPropName, store as object as IProvideDecoratedVariable<object>);
    }

    findProvide<T>(providedPropName: string): IProvideDecoratedVariable<T> | null {
        const localVar = this.providedVars_.get(providedPropName);
        if (localVar !== undefined) {
            return localVar as object as IProvideDecoratedVariable<T>;
        }
        if (this.parent_ !== undefined) {
            return this.parent_!.findProvide<T>(providedPropName);
        }
        return null;
    }

    getUIContext(): UIContext {
        return this.delegate_!.getUIContext();
    }
    getUniqueId(): int32 {
        return this.delegate_!.getUniqueId();
    }
    getPeerNode(): PeerNode | undefined {
        return this.delegate_!.getPeerNode();
    }
}