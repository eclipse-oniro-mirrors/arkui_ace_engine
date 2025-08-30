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


// HANDWRITTEN, DO NOT REGENERATE

import { __context, __id, remember } from '@koalaui/runtime';
import { RepeatImplForOptions } from '../handwritten/RepeatImpl';
import { InteropNativeModule } from "@koalaui/interop";

export interface RepeatItem<T> {
    readonly item: T;
    readonly index: number;
}

export type RepeatArray<T> = Array<T> | ReadonlyArray<T> | Readonly<Array<T>>;
export type RepeatItemBuilder<T> =
    /** @memo */
    (repeatItem: RepeatItem<T>) => void;
export type TemplateTypedFunc<T> = (item: T, index: number) => string;

export interface VirtualScrollOptions {
    totalCount?: number;
    reusable?: boolean;
    onLazyLoading?: (index: number) => void;
    onTotalCount?: () => number;
    disableVirtualScroll?: boolean;
}

export interface TemplateOptions {
    cachedCount?: number;
}

export interface RepeatAttribute<T> {
    arr: RepeatArray<T>;
    each(itemGenerator: RepeatItemBuilder<T>): RepeatAttribute<T>;
    key(keyGenerator: (item: T, index: number) => string): RepeatAttribute<T>;
    virtualScroll(options?: VirtualScrollOptions): RepeatAttribute<T>;
    template(type: string, itemBuilder: RepeatItemBuilder<T>, templateOptions?: TemplateOptions): RepeatAttribute<T>;
    templateId(typedFunc: TemplateTypedFunc<T>): RepeatAttribute<T>;
    setRepeatOptions(arr: RepeatArray<T>): this {
        return this;
    }
    applyAttributesFinish(): void {}
}
    
export class ArkRepeatComponent<T> implements RepeatAttribute<T> {
    public arr: RepeatArray<T> = [];
    public setRepeatOptions(arr: RepeatArray<T>): this {
        this.arr = arr;
        return this;
    }
    public each(itemGenerator: RepeatItemBuilder<T>): RepeatAttribute<T> {
        return this;
    };
    public key(keyGenerator: (item: T, index: number) => string): RepeatAttribute<T> {
        return this;
    };
    public virtualScroll(options?: VirtualScrollOptions): RepeatAttribute<T> {
        return this;
    };
    public template(type: string, itemBuilder: RepeatItemBuilder<T>, templateOptions?: TemplateOptions): RepeatAttribute<T> {
        return this;
    };
    public templateId(typedFunc: TemplateTypedFunc<T>): RepeatAttribute<T> {
        return this;
    };
    public applyAttributesFinish(): void {
        // we call this function outside of class, so need to make it public
    }
}
    
/** @memo */
export function RepeatImpl<T>(
    /** @memo */
    style: ((attributes: RepeatAttribute<T>) => void) | undefined,
): void {
    const receiver = remember(() => {
        return new ArkRepeatComponent<T>()
    })
    style?.(receiver)
    RepeatImplForOptions<T>(style, receiver.arr);
}
