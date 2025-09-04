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


// HANDWRITTEN, DO NOT REGENERATE

import { __context, __id, remember } from "@koalaui/runtime"
import { DynamicNode, OnMoveHandler, ItemDragEventHandler } from "./common"
import { LazyForEachImplForOptions } from "../handwritten/LazyForEachImpl"
import { InteropNativeModule } from "@koalaui/interop";

export enum DataOperationType {
    ADD = "add",
    DELETE = "delete",
    EXCHANGE = "exchange",
    MOVE = "move",
    CHANGE = "change",
    RELOAD = "reload"
}
export interface DataAddOperation {
    type: DataOperationType;
    index: number;
    count?: number;
    key?: string | Array<string>;
}
export interface DataDeleteOperation {
    type: DataOperationType;
    index: number;
    count?: number;
}
export interface DataChangeOperation {
    type: DataOperationType;
    index: number;
    key?: string;
}
export interface MoveIndex {
    from: number;
    to: number;
}
export interface ExchangeIndex {
    start: number;
    end: number;
}
export interface ExchangeKey {
    start: string;
    end: string;
}
export interface DataMoveOperation {
    type: DataOperationType;
    index: MoveIndex;
    key?: string;
}
export interface DataExchangeOperation {
    type: DataOperationType;
    index: ExchangeIndex;
    key?: ExchangeKey;
}
export interface DataReloadOperation {
    type: DataOperationType;
}
export type DataOperation = DataAddOperation | DataDeleteOperation | DataChangeOperation | DataMoveOperation | DataExchangeOperation | DataReloadOperation;

export interface DataChangeListener {
    onDataReloaded(): void;
    onDataAdded(index: number): void;
    onDataAdd(index: number): void;
    onDataMoved(from: number, to: number): void;
    onDataMove(from: number, to: number): void;
    onDataDeleted(index: number): void;
    onDataDelete(index: number): void;
    onDataChanged(index: number): void;
    onDataChange(index: number): void;
    onDatasetChange(dataOperations: DataOperation[]): void;
}

/**
 * Developers need to implement this interface to provide data to LazyForEach component.
 * @since 7
 */
export interface IDataSource<T> {
    /**
     * Total data count.
     * @since 7
     */
    totalCount(): number;
    /**
     * Return the data of index.
     * @since 7
     */
    getData(index: number): T;
    /**
     * Register data change listener.
     * @since 7
     */
    registerDataChangeListener(listener: DataChangeListener): void;
    /**
     * Unregister data change listener.
     * @since 7
     */
    unregisterDataChangeListener(listener: DataChangeListener): void;
}

export interface LazyForEachAttribute<T> extends DynamicNode {
    dataSource: IDataSource<T> | null;
    setLazyForEachOptions(dataSource: IDataSource<T>,
        /** @memo */
        itemGenerator: (item: T, index: number) => void,
        keyGenerator?: (item: T, index: number) => string): this {
        return this;
    }
    applyAttributesFinish(): void {}
    
    onMove(handler?: OnMoveHandler): this {
        return this;
    }
    onMove(handler?: OnMoveHandler, eventHandler?: ItemDragEventHandler): this {
        return this;
    }
}
export class ArkLazyForEachComponent<T> implements LazyForEachAttribute<T> {
    dataSource: IDataSource<T> | null = null;
    /** @memo */
    itemGenerator: (item: T, index: number) => void = (item: T, index: number) => {};
    keyGenerator?: (item: T, index: number) => string = undefined;
    onMoveEvent?: OnMoveHandler = undefined;
    itemDragEvent?: ItemDragEventHandler = undefined;

    public onMove(handler?: OnMoveHandler): this {
        this.onMoveEvent = handler;
        return this;
    }

    public onMove(handler?: OnMoveHandler, eventHandler?: ItemDragEventHandler): this {
        this.onMoveEvent = handler;
        this.itemDragEvent = eventHandler;
        return this;
    }

    public setLazyForEachOptions(dataSource: IDataSource<T>,
        /** @memo */
        itemGenerator: (item: T, index: number) => void,
        keyGenerator?: (item: T, index: number) => string): this {
        this.dataSource = dataSource;
        this.itemGenerator = itemGenerator;
        this.keyGenerator = keyGenerator;
        return this;
    }
    applyAttributesFinish(): void {}
}
/** @memo */
export function LazyForEachImpl<T>(
    /** @memo */
    style: ((attributes: LazyForEachAttribute<T>) => void) | undefined
) {
    const receiver = remember(() => {
        return new ArkLazyForEachComponent<T>()
    })
    style?.(receiver)
    if(!receiver.dataSource) {
            InteropNativeModule._NativeLog("LazyForEach receiver.dataSource null ")
    }
    else {
        LazyForEachImplForOptions(
            receiver.dataSource!,
            receiver.itemGenerator,
            receiver.keyGenerator,
            false,
            receiver.onMoveEvent,
            receiver.itemDragEvent);
    }
}
