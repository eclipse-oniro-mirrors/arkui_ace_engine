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

import { int32 } from '@koalaui/common';
import { RecordData } from '@ohos.base';
import { default as uiObserver } from '@ohos/arkui/observer';
import { CustomComponentLifecycle, CustomDelegate } from './customComponent';
import { __CustomComponentLifecycle__Internal } from './customComponentLifecycle';
import { ArkCustomComponentRootPeer } from './staticComponents';
import { SerializerBase, DeserializerBase, KPointer } from '@koalaui/interop';
import {
    IProvideDecoratedVariable,
    IProviderDecoratedVariable,
    IVariableOwner,
    LocalStorage
} from '@ohos.arkui.stateManagement';
import { UIContext } from '@ohos/arkui/UIContext';
import { PeerNode } from '../PeerNode';
import promptAction from '@ohos/promptAction'
import { Theme } from '@ohos/arkui/theme';
import { DecoratedVariableBase } from '../stateManagement/decoratorImpl/decoratorBase';
import { IDecoratorBaseRegistry } from '../stateManagement/decorator';
import { IEnvironmentValueBase, IEnvVariable, EnvDecoratedVariable } from '../stateManagement/decoratorImpl/decoratorEnv';
import { ActiveAndInactiveCallbackType, CustomComponentAsyncLoadOptions,
    CustomComponentContext } from '../stateManagement/utils';
import { ArkUIAniCustomNodeModule } from "arkui.ani";
import { CustomComponentContextImpl } from './customComponent.ets';
import { ObserveSingleton } from '../stateManagement/base/observeSingleton';
import { ExtendableComponentDFX } from './extendableComponentDFX';
import { GlobalReusePool } from '../stateManagement/tools/globalReusePool';
import { ConsumeDecoratedVariable } from '../stateManagement';
import { ConsumerDecoratedVariable } from '../stateManagement/decoratorImpl/decoratorConsumer';
export class ExtendableComponentInternal {
    public static fromPtr(ptr: KPointer): ExtendableComponent {
        throw new Error("TBD");
    }
}

// Resolved asynchronous-loading configuration stored after an effective
// enableAsyncLoad request and pushed to the native node on every attach
// (timeout already resolved; '' means "no placeholder"/"unset axis").
// JSON payload delivered over the ani bridge — the same format the dynamic
// bridge carries, so the native parsing is shared. Optional/empty members are
// omitted to keep "unset" distinguishable from "set to empty".
interface AsyncLoadDeliveryPayload {
    timeout: int;
    placeholderId?: string;
    width?: string;
    height?: string;
}

interface AsyncLoadResolvedConfig {
    timeoutMs: int;
    placeholderId?: string;
    sizeWidth: string;
    sizeHeight: string;
}

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
    __getMemOpt__Internal(): int32;
    queryNavigationInfo(): uiObserver.NavigationInfo;
    queryNavDestinationInfo(isInner: boolean | undefined): uiObserver.NavDestinationInfo;
    queryNavDestinationInfo(): uiObserver.NavDestinationInfo
    queryRouterPageInfo(): uiObserver.RouterPageInfo;
    findCustomValueByKey(key: int32): Any | undefined;
    findWritableSystemEnvValueByKey(key: string): Any | undefined;
    registerOnCustomEnvUpdateCallback(callback: (key: int32, value: Any) => void): void;
    registerOnSystemEnvUpdateCallback(callback: (key: string, value: Any) => void): void;
    getDialogController(): promptAction.DialogController | undefined;
    __removeChildWhenDispose__IExtendableComponent_Internal(): void;
    __addMeToParent__IExtendableComponent_Internal(): void;
    isViewActive(): boolean;
    setActiveInternal(active: boolean, isReuse: boolean, suppressActiveLifecycle: boolean): void;
    fireAboutToReuse(param?: RecordData): void;
    fireAboutToRecycle(): void;
    __getParentNode__Internal(): PeerNode | undefined;
    getInstance(): ExtendableComponent;
    getCanUpdateStateVars(): boolean;
    __updateUIContext__Internal(newContext: UIContext): void;
}

export abstract class ExtendableComponent implements LifeCycle, IVariableOwner {
    static current: object | undefined = undefined;
    private activeCount_: number = 1;
    public ___reusePool: GlobalReusePool | undefined = undefined;
    // Lifecycle management
    @JSONStringifyIgnore
    private __lifecycle__Internal?: CustomComponentLifecycle;

    @JSONStringifyIgnore
    private __customComponentContext__Internal?: CustomComponentContext;

    __getLifecycle__Internal(): CustomComponentLifecycle {
        if (!this.__lifecycle__Internal) {
            this.__lifecycle__Internal = new __CustomComponentLifecycle__Internal(this as Object);
        }
        return this.__lifecycle__Internal!;
    }

    /**
    * Walks up the parent chain from this component looking for the nearest
    * ancestor whose ___reusePool accepts the given class and is still active.
    * Used by PeerNode.reuse/recycle to route reusable instances to the correct shared pool.
    * Returns undefined if no matching pool is found.
    */
    public __getReusePoolForClass__Internal(classKey: string, skipPool?: GlobalReusePool): GlobalReusePool | undefined {
        // Fast path: no pool anywhere in the chain we can see
        if (this.___reusePool === undefined && this.parent_ === undefined) {
            return undefined;
        }
	    console.log(`GlobalReuse: WALK start from=${Class.of(this).getName()} classKey=${classKey}`);
        let current: ExtendableComponent | undefined = this;
        while (current !== undefined) {
            const pool = current!.___reusePool;
            if (pool !== undefined && pool!.acceptsComponent(classKey) && pool!.isActive() && pool !== skipPool) {
                return pool;
            }
            current = current!.parent_;
        }
        return undefined;
    }


    private parent_: ExtendableComponent | undefined;
    private providedVars_: Map<string, IProvideDecoratedVariable<object>> = new Map<string, IProvideDecoratedVariable<object>>();
    private providedVarsV2_: Map<string, IProviderDecoratedVariable<object>> = new Map<string, IProviderDecoratedVariable<object>>();
    private delegate_?: IExtendableComponent;
    // Resolved asynchronous-loading configuration (resolved timeout and
    // placeholder id) once an effective enableAsyncLoad request has been made.
    // Persisted for the whole instance lifetime so that every (re-)attach of the
    // component pushes the configuration to the freshly created native node,
    // mirroring the dynamic side where the config lives on the JS view and is
    // re-read on every native node creation.
    private __asyncLoadConfig__Internal?: AsyncLoadResolvedConfig;
    // True once the first effective enableAsyncLoad request has been accepted
    // (first request wins; later requests never rewrite).
    private __asyncLoadRequested__Internal: boolean = false;
    private localStoragebackStore_?: LocalStorage | undefined = undefined;
    private backLocalStorage_?: LocalStorage | undefined = undefined;
    private useSharedStorage_?: boolean | undefined = undefined;
    private childrenWeakRefMap_: Map<int32, WeakRef<ExtendableComponent>> = new Map<int32, WeakRef<ExtendableComponent>>();
    private activeChangeListenerForInterop_: Set<(active: boolean, isReuse: boolean) => void> = new Set<(active: boolean, isReuse: boolean) => void>();
    private hasBeenRecycled_: boolean = false;
    private ownedStateVariables_: Array<IDecoratorBaseRegistry> | undefined = undefined;
    private ownedConsumerStateVariables_: Array<IDecoratorBaseRegistry> | undefined = undefined;
    private envPropertySet: Set<IEnvVariable> = new Set<IEnvVariable>();
    private envInstanceId: int32 | undefined;
    protected memopt_: int32 = 0;
    private readonlyEnvDispatchFuncs_: Map<string, (value: Any) => void> = new Map<string, (value: Any) => void>();
    private readonlyEnvUnregisterFuncs_: Map<string, () => void> = new Map<string, () => void>();
    private customEnvCallbackMap_: Map<int32, Set<(key: int32, value: Any) => void>> = new Map<int32, Set<(key: int32, value: Any) => void>>();
    private systemEnvCallbacksMap_: Map<string, Set<(value: Any) => void>> = new Map<string, Set<(value: Any) => void>>();
    private hasRegisteredWritableEnvCallback_: boolean = false;

    private __activeFunc__Internal?: ActiveAndInactiveCallbackType;
    private __inactiveFunc__Internal?: ActiveAndInactiveCallbackType;

    public static finalizationRegistry_: FinalizationRegistry<Object> = new FinalizationRegistry<Object>(
        (destroyProperty: Object) => {
            if (destroyProperty instanceof IEnvironmentValueBase) {
                (destroyProperty as IEnvironmentValueBase).offWatchFunc();
                if (EnvDecoratedVariable.instanceIdSet.size > 0) {
                    EnvDecoratedVariable.instanceIdSet.clear();
                }
            }
        }
    );

    constructor(useSharedStorage?: boolean, storage?: LocalStorage) {
        this.useSharedStorage_ = useSharedStorage;
        this.backLocalStorage_ = storage;
        this.parent_ = ExtendableComponent.current as (ExtendableComponent | undefined);
    }

    __findWritableSystemEnvValueByKey__Internal(key: string): Any {
        return this.findWritableSystemEnvValueByKey(key);
    }

    __addSystemEnvValueCallback__Internal(key: string, callback: (value: Any) => void): void {
        let callbackSet = this.systemEnvCallbacksMap_.get(key);
        if (callbackSet === undefined) {
            callbackSet = new Set<(value: Any) => void>();
            this.systemEnvCallbacksMap_.set(key, callbackSet);
        }
        callbackSet.add(callback);
    }

    __registerWritableEnvUpdateCallback__Internal(callback: (key: string, value: Any) => void): void {
        this.registerOnSystemEnvUpdateCallback(callback);
        this.hasRegisteredWritableEnvCallback_ = true;
    }

    __removeSystemEnvValueCallback__Internal(key: string, callback: (value: Any) => void): boolean {
        const callbackSet = this.systemEnvCallbacksMap_.get(key);
        if (callbackSet !== undefined) {
            callbackSet.delete(callback);
            if (callbackSet.size === 0) {
                this.systemEnvCallbacksMap_.delete(key);
                return true;
            }
        }
        return false;
    }

    __getSystemEnvValueCallbacks__Internal(key: string): Set<(value: Any) => void> | undefined  {
        return this.systemEnvCallbacksMap_.get(key);
    }

    __getSystemEnvCallbacksMap__Internal(): Map<string, Set<(value: Any) => void>>  {
        return this.systemEnvCallbacksMap_;
    }

    __getHasRegisteredWritableEnvCallback__Internal(): boolean  {
        return this.hasRegisteredWritableEnvCallback_;
    }

    __updateCustomEnvVarValue__Internal(internalId: int32, callback: (key: int32, value: Any) => void): void {
        if (!this.customEnvCallbackMap_.has(internalId)) {
            this.customEnvCallbackMap_.set(internalId, new Set<(key: int32, value: Any) => void>());
        }
        this.customEnvCallbackMap_.get(internalId)!.add(callback);
    }

    __getCustomEnvCallbackMap__Internal(): Map<int32, Set<(key: int32, value: Any) => void>> {
        return this.customEnvCallbackMap_;
    }

    __removeCustomEnvCallback__Internal(internalId: int32, callback: (key: int32, value: Any) => void): boolean {
        const callbackSubMap = this.customEnvCallbackMap_.get(internalId);
        if (callbackSubMap !== undefined) {
            callbackSubMap.delete(callback);
            if (callbackSubMap.size === 0) {
                this.customEnvCallbackMap_.delete(internalId);
                return true;
            }
        }
        return false;
    }

    __findCustomEnvValueByKey__Internal(key: int32): Any {
        return this.findCustomValueByKey(key);
    }

    __registerCustomEnvUpdateCallback__Internal(callback: (key: int32, value: Any) => void): void {
        this.registerOnCustomEnvUpdateCallback(callback);
    }

    __registerStateVariables__Internal(stateVariable: IDecoratorBaseRegistry): void {
        if ((stateVariable instanceof ConsumerDecoratedVariable)
            || (stateVariable instanceof ConsumeDecoratedVariable)) {
            if (!this.ownedConsumerStateVariables_) {
                this.ownedConsumerStateVariables_ = new Array<IDecoratorBaseRegistry>();
            }
            this.ownedConsumerStateVariables_!.push(stateVariable);
        }
        if (!this.ownedStateVariables_) {
            this.ownedStateVariables_ = new Array<IDecoratorBaseRegistry>();
        }
        this.ownedStateVariables_!.push(stateVariable);
    }

    __clearStateVariables__Internal(): void {
        this.ownedConsumerStateVariables_ = undefined;
        if (!this.ownedStateVariables_?.length) {
            return;
        }
        this.ownedStateVariables_!.forEach((state: IDecoratorBaseRegistry) => {
            if (state instanceof DecoratedVariableBase) {
                state.aboutToBeDeletedInternal();
            }
        });
        this.ownedStateVariables_ = undefined;
    }

    public connectConsumers(): void {
        if (this.ownedConsumerStateVariables_) {
            this.ownedConsumerStateVariables_!.forEach((state: IDecoratorBaseRegistry) => {
                if (state instanceof DecoratedVariableBase) {
                    (state as DecoratedVariableBase).connect();
                }
            });
        }
        this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
            const child = weak.deref();
            child?.connectConsumers();
        });
    }

    public disconnectConsumers(): void {
        if (this.ownedConsumerStateVariables_) {
            this.ownedConsumerStateVariables_!.forEach((state: IDecoratorBaseRegistry) => {
                if (state instanceof DecoratedVariableBase) {
                    (state as DecoratedVariableBase).disconnect();
                }
            });
        }
        this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
            const child = weak.deref();
            child?.disconnectConsumers();
        });
    }

    __aboutToReuse_Internal(param?: RecordData): void {
        this.delegate_!.fireAboutToReuse(param);
        this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
            const child = weak.deref();
            child?.__checkAndCallReuse_Internal(param);
        });
    }

    __checkAndCallReuse_Internal(param?: RecordData): void {
        if (!this.hasBeenRecycled_) {
            this.__aboutToReuse_Internal(param);
        }
    }

    __switchHasBeenRecycle_Internal(recycle: boolean): void {
        this.hasBeenRecycled_ = recycle;
    }
    __aboutToRecycle_Internal(): void {
        this.delegate_!.fireAboutToRecycle();
        this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
            const child = weak.deref();
            child?.__aboutToRecycle_Internal();
        });
        if (this.memopt_ === 1) {
            this.__getParentNode__Internal()?.setClearPool(true);
        }
    }

    __addMeToParent__Internal(): void {
        this.parent_?.__addChild__Internal(this);
    }

    __addChild__Internal(child: ExtendableComponent): void {
        this.childrenWeakRefMap_.set(child.getUniqueId(), new WeakRef<ExtendableComponent>(child));
    }

    __setParent__Internal(parent: ExtendableComponent | undefined): void {
        this.parent_ = parent;
    }

    __removeChildById__Internal(id: int32): void {
        this.childrenWeakRefMap_.delete(id);
    }

    __removeMeFromParent__Internal(): void {
        this.parent_?.__removeChildById__Internal(this.getUniqueId());
    }

    __getParentNode__Internal(): PeerNode | undefined {
        return this.parent_?.__getPeerNode__Internal();
    }

    __removeAllChild__Internal(): void {
        this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
            const child = weak.deref();
            child?.__setParent__Internal(undefined);
        });
        this.childrenWeakRefMap_.clear();
    }

    __getChildrenWeakRefMap__Internal(): Map<int32, WeakRef<ExtendableComponent>> {
        return this.childrenWeakRefMap_;
    }

    __clearReusePool__Internal(): void {
        this.__getPeerNode__Internal()?.clearReusePool();
        this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
            const child = weak.deref();
            child?.__clearReusePool__Internal();
        });
    }

    __getActive__Internal(): number {
        return this.activeCount_;
    }

    __setActive__Internal(activeCount: number): void {
        this.activeCount_ = activeCount;
    }

    __freezeWhenRecycle__Internal(): void {
        this.activeCount_--;
        if (this.activeCount_ === 0) {
            this.__fireActiveOrInactiveFunc__Internal(false);
        }
    }

    __unfreezeWhenReuse__Internal(): void {
        this.activeCount_++;
        if (this.activeCount_ === 1) {
            this.__fireActiveOrInactiveFunc__Internal(true);
        }
    }

    __isViewActive__Internal(): boolean {
        return this.delegate_!.isViewActive();
    }

    __addActiveChangeListenerForInterop__Internal(listener: (active: boolean, isReuse: boolean) => void): void {
        this.activeChangeListenerForInterop_.add(listener);
    }

    __removeActiveChangeListenerForInterop__Internal(listener: (active: boolean, isReuse: boolean) => void): void {
        this.activeChangeListenerForInterop_.delete(listener);
    }

    __propagateToChild__Internal(active: boolean, isReuse: boolean, suppressActiveLifecycle: boolean = false): void {
        if (this.childrenWeakRefMap_.size > 0) {
            this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
                const child = weak.deref();
                child?.__setActiveInternal__Internal(active, isReuse, suppressActiveLifecycle);
            });
        }
        if (this.activeChangeListenerForInterop_.size > 0) {
            this.activeChangeListenerForInterop_.forEach((listener: (active: boolean, isReuse: boolean) => void) => {
                listener(active, isReuse);
            });
        }
    }

    __setActiveInternal__Internal(active: boolean, isReuse: boolean, suppressActiveLifecycle: boolean): void {
        this.delegate_!.setActiveInternal(active, isReuse, suppressActiveLifecycle);
    }

    __setDelegate__Internal(delegate: IExtendableComponent): void {
        this.delegate_ = delegate;
    }

    __addProvider__Internal<T>(providedPropName: string, store: IProviderDecoratedVariable<T>): void {
        this.providedVarsV2_.set(providedPropName, store as object as IProviderDecoratedVariable<object>);
    }

    __addProvide__Internal<T>(providedPropName: string, store: IProvideDecoratedVariable<T>, allowOverride?: boolean): void {
        if (!allowOverride && this.__findProvide__Internal<T>(providedPropName)) {
            throw new ReferenceError(`Duplicate @Provide property with name ${providedPropName}. Property with this name is provided by one of the ancestor Component already.`);
        }
        this.providedVars_.set(providedPropName, store as object as IProvideDecoratedVariable<object>);
    }

    __findProvidedVarV2__Internal<T>(providedPropName: string): IProviderDecoratedVariable<T> | undefined {
        const result = this.providedVarsV2_.get(providedPropName);
        if (result == undefined) {
            return undefined;
        }
        return result as IProviderDecoratedVariable<T>;
    }

    __findProvider__Internal<T>(providedPropName: string): IProviderDecoratedVariable<T> | undefined {
        let parentCom = this.parent_;
        while (parentCom !== undefined) {
            let provideVar = parentCom.__findProvidedVarV2__Internal<Any>(providedPropName);
            if (provideVar !== undefined) {
                return provideVar as object as IProviderDecoratedVariable<T>;
            }
            parentCom = parentCom.parent_;
        }
        return undefined;
    }

    __findProvide__Internal<T>(providedPropName: string): IProvideDecoratedVariable<T> | undefined {
        const localVar = this.providedVars_.get(providedPropName);
        if (localVar !== undefined) {
            return localVar as object as IProvideDecoratedVariable<T>;
        }
        return this.parent_?.__findProvide__Internal<T>(providedPropName);
    }

    __addEnvInstance__Internal(envProperty: IEnvVariable): void {
        this.envPropertySet.add(envProperty);
    }

    checkEnvSetSize(): boolean {
        return this.envPropertySet.size > 0;
    }

    getEnvSet(): Set<IEnvVariable> {
        return this.envPropertySet;
    }

    getEnvInstance(): int32 | undefined {
        return this.envInstanceId;
    }

    setEnvInstance(newInstanceId: int32 | undefined): void {
        this.envInstanceId = newInstanceId;
    }

    __updateFreeRootUIContext__Internal(oldContext: UIContext, newContext: UIContext): void {
        const peer = this.__getPeerNode__Internal();
        if (peer instanceof ArkCustomComponentRootPeer) {
            peer.updateFreeRootUIContext(oldContext, newContext);
        }
    }

    __propagateInstanceSwitch__Internal(newInstanceId: int32, newContext: UIContext): void {
        this.childrenWeakRefMap_.forEach((weak: WeakRef<ExtendableComponent>, _: int32) => {
            const child = weak.deref();
            if (child) {
                const childOldContext = child.getUIContext();
                const childDelegate = child.__getIExtendableDelegate__Internal();
                if (childDelegate) {
                    childDelegate.__updateUIContext__Internal(newContext);
                }
                child.__updateFreeRootUIContext__Internal(childOldContext, newContext);
                child.__propagateInstanceSwitch__Internal(newInstanceId, newContext);
            }
        });
    }

    __getIExtendableDelegate__Internal(): IExtendableComponent | undefined {
        return this.delegate_;
    }

    __setReadonlyEnvDispatchFunc__Internal(envKey: string, dispatchFunc: (value: Any) => void): void {
        this.readonlyEnvDispatchFuncs_.set(envKey, dispatchFunc);
    }
 
    __getAndClearReadonlyEnvDispatchFunc__Internal(envKey: string): ((value: Any) => void) | undefined {
        const dispatchFunc = this.readonlyEnvDispatchFuncs_.get(envKey);
        this.readonlyEnvDispatchFuncs_.delete(envKey);
        return dispatchFunc;
    }
 
    __getReadonlyEnvDispatchFuncs__Internal(): Map<string, (value: Any) => void> {
        return this.readonlyEnvDispatchFuncs_;
    }

    __setReadonlyEnvUnregisterFunc__Internal(envKey: string, unregisterFunc: () => void): void {
        this.readonlyEnvUnregisterFuncs_.set(envKey, unregisterFunc);
    }

    __getReadonlyEnvUnregisterFuncs__Internal(): Map<string, () => void> {
        return this.readonlyEnvUnregisterFuncs_;
    }
 
    __clearAllEnvRegistrations__Internal(): void {
        this.customEnvCallbackMap_.clear();
        this.systemEnvCallbacksMap_.clear();
        this.readonlyEnvDispatchFuncs_.clear();
        this.readonlyEnvUnregisterFuncs_.clear();
    }    

    __getDelegate__Internal<T extends ExtendableComponent, T_Options>(): CustomDelegate<T, T_Options> | undefined {
        if (this.delegate_ instanceof CustomDelegate) {
            return this.delegate_ as CustomDelegate<T, T_Options>;
        }
        return undefined;
    }

    getownedStateVariables(): Array<IDecoratorBaseRegistry> | undefined {
        return this.ownedStateVariables_;
    }

    getUIContext(): UIContext {
        return this.delegate_!.getUIContext();
    }

    getUniqueId(): int32 {
        return this.delegate_!.getUniqueId();
    }
    
    queryNavigationInfo(): uiObserver.NavigationInfo {
        return this.delegate_!.queryNavigationInfo();
    }
    queryRouterPageInfo(): uiObserver.RouterPageInfo {
        return this.delegate_!.queryRouterPageInfo();
    }
    queryNavDestinationInfo(isInner: boolean | undefined): uiObserver.NavDestinationInfo {
        return this.delegate_!.queryNavDestinationInfo(isInner);
    }
    queryNavDestinationInfo() : uiObserver.NavDestinationInfo {
        return this.delegate_!.queryNavDestinationInfo();
    }
    findCustomValueByKey(key: int32): Any | undefined {
        return this.delegate_!.findCustomValueByKey(key);
    }
    findWritableSystemEnvValueByKey(key: string): Any | undefined {
        return this.delegate_!.findWritableSystemEnvValueByKey(key);
    }
    registerOnCustomEnvUpdateCallback(callback: (key: int32, value: Any) => void): void {
        this.delegate_!.registerOnCustomEnvUpdateCallback(callback);
    }
    registerOnSystemEnvUpdateCallback(callback: (key: string, value: Any) => void): void {
        this.delegate_!.registerOnSystemEnvUpdateCallback(callback);
    }

    getDialogController(): promptAction.DialogController | undefined {
        return this.delegate_!.getDialogController();
    }

    __getLocalStorage__Internal(): LocalStorage {
        return this.__localStorage__Internal;
    }

    public ClearParentReusePool() {
        let parentCom = this.parent_;
        while (parentCom !== undefined) {
            const parentPeer = parentCom.__getPeerNode__Internal();
            if (parentPeer !== undefined) {
                parentPeer.clearReusePool();
                return;
            }
            parentCom = parentCom.parent_;
        }
    }

    public __getMemOpt__Internal(): int32 {
        return this.memopt_;
    }

    public get __localStorage__Internal(): LocalStorage {
        // When the current component has no localStorage instance and has parent component.
        if (!this.localStoragebackStore_ && this.parent_) {
            this.localStoragebackStore_ = this.backLocalStorage_ ? this.backLocalStorage_ : this.parent_!.__localStorage__Internal;
        }
        // When the current component has no localStorage instance and no parent component.
        if (!this.localStoragebackStore_) {
            // When the useSharedStorage parameter is set to true.
            if (this.useSharedStorage_) {
                this.localStoragebackStore_ = this.getUIContext().getSharedLocalStorage() ?? new LocalStorage();
            }
            // When the useSharedStorage parameter is set to false, or when it is set to true but the localStorage instance obtained from UIContext is undefined.
            if (!this.localStoragebackStore_) {
                this.localStoragebackStore_ = this.backLocalStorage_ ? this.backLocalStorage_ : new LocalStorage();
            }
        }

        return this.localStoragebackStore_!;
    }

    public set __localStorage__Internal(instance: LocalStorage) {
        this.localStoragebackStore_ = instance;
    }
    
    __getPeerNode__Internal(): PeerNode | undefined {
        return this.delegate_?.getPeerNode();
    }

    // Default timeout of asynchronous loading in ms (API contract, frozen).
    private static readonly ASYNC_LOAD_DEFAULT_TIMEOUT_MS: int = 1000;
    // Resolved value for "never force-load" (negative timeout).
    private static readonly ASYNC_LOAD_TIMEOUT_NEVER: int = -1;

    /**
     * Resolves an `options.timeout` into the integer millisecond value the native
     * side consumes, so the bridge only ever carries a plain int32. The option is
     * typed `int` (a 32-bit domain), so only undefined and the sign apply here; the
     * dynamic-side resolver additionally guards NaN / +/-Infinity / fractions and
     * clamps out-of-int32 values, none of which an int can carry:
     *   undefined   -> 1000 (framework default)
     *   negative    -> -1   (never time out)
     *   otherwise   -> used as is
     */
    private static __resolveAsyncLoadTimeoutMs__Internal(timeout?: int): int {
        if (timeout === undefined) {
            return ExtendableComponent.ASYNC_LOAD_DEFAULT_TIMEOUT_MS;
        }
        if (timeout < 0) {
            return ExtendableComponent.ASYNC_LOAD_TIMEOUT_NEVER;
        }
        return timeout;
    }

    /**
     * Stores the asynchronous-loading configuration for this component instance.
     *
     * Reached by apps only through the CustomComponentContext returned by
     * UIUtils.getCustomComponentContext(); framework internal.
     *
     * Effective only once, and only inside the construction window:
     * on the static paradigm the window is bounded by the delegate being set —
     * the user constructor (including the @ComponentInit-decorated method) runs
     * before the framework creates the CustomDelegate, everything afterwards is
     * out of window and ignored. The first effective request wins — later calls,
     * even with different options, never rewrite the configuration (first request wins).
     *
     * Unlike the dynamic side, the native peer does not exist yet at this point,
     * so the resolved configuration is stored here and pushed by
     * __deliverAsyncLoadConfig__Internal when the peer is created, which always
     * happens before the first build is driven (delivery is
     * ordered before first build, the deferral itself lives in native).
     */
    __enableAsyncLoad__Internal(options?: CustomComponentAsyncLoadOptions): void {
        const ownerTag = Class.of(this as Object).getName();
        if (this.delegate_ !== undefined) {
            // Out-of-window calls are ignored without any observable effect,
            // but are recorded as a maintainability warning log.
            console.warn(`${ownerTag}: enableAsyncLoad ignored: outside the construction window`);
            return;
        }
        if (this.__asyncLoadRequested__Internal) {
            console.warn(`${ownerTag}: enableAsyncLoad ignored: already requested, first request wins`);
            return;
        }
        this.__asyncLoadRequested__Internal = true;
        const placeholder = options?.placeholder;
        let placeholderId = '';
        let sizeWidth = '';
        let sizeHeight = '';
        if (placeholder !== undefined) {
            const size = placeholder.size;
            if (size !== undefined) {
                // Stringify keeps the unit: numbers become bare strings meaning vp,
                // suffixed strings ("100%", "10px") keep their unit; a Resource
                // stringifies unusably and degrades to "unset" on the native side.
                if (size.width !== undefined) {
                    sizeWidth = String(size.width);
                }
                if (size.height !== undefined) {
                    sizeHeight = String(size.height);
                }
            }
            const id = placeholder.id;
            if (id !== undefined) {
                if (id !== '') {
                    placeholderId = id;
                }
            }
            if (placeholderId === '') {
                // A configured placeholder whose id is empty or not a string
                // degrades to "no placeholder", recorded as a maintainability warning.
                console.warn(`${ownerTag}: enableAsyncLoad: placeholder configured but id is empty or not a string, ` +
                    `placeholder ignored`);
            }
        }
        const config: AsyncLoadResolvedConfig = {
            timeoutMs: ExtendableComponent.__resolveAsyncLoadTimeoutMs__Internal(options?.timeout),
            sizeWidth: sizeWidth,
            sizeHeight: sizeHeight
        };
        if (placeholderId !== '') {
            config.placeholderId = placeholderId;
        }
        this.__asyncLoadConfig__Internal = config;
    }

    /**
     * Pushes the stored asynchronous-loading configuration (if any) to the native
     * node behind `peer`. Called by ArkComponentRoot right after the peer is
     * created for this component, before the first build is driven. Re-invoked on
     * every (re-)attach: each attach creates a fresh native node that starts with
     * no configuration, mirroring the dynamic side where node creation re-reads
     * the configuration stored on the JS view. No-op when enableAsyncLoad was
     * never effectively requested.
     */
    __deliverAsyncLoadConfig__Internal(peer: PeerNode): void {
        const config = this.__asyncLoadConfig__Internal;
        if (config === undefined) {
            return;
        }
        const payload: AsyncLoadDeliveryPayload = { timeout: config.timeoutMs };
        if (config.placeholderId !== undefined && config.placeholderId !== '') {
            payload.placeholderId = config.placeholderId;
        }
        if (config.sizeWidth !== '') {
            payload.width = config.sizeWidth;
        }
        if (config.sizeHeight !== '') {
            payload.height = config.sizeHeight;
        }
        ArkUIAniCustomNodeModule._CustomNode_SetAsyncLoadConfig(peer.peer.ptr, JSON.stringify(payload));
    }

    /**
     * Reports whether this component's backing peer node is in the process of
     * being disposed (or already disposed). Used by GlobalReusePool to decide
     * whether the pool's owner is being torn down — in which case recycled
     * children must disappear rather than be pooled.
     */
    __isPeerNodeDisposing__Internal(): boolean {
        const peer = this.__getPeerNode__Internal();
        return peer !== undefined ? (peer!.disposed || peer!.disposing) : false;
    }

    public __onDumpInspector__Internal(): string {
        return ExtendableComponentDFX.onDumpInspector(this);
    }

    public __onDumpInfo__Internal(commands: string[]): void {
        ExtendableComponentDFX.onDumpInfo(this, commands);
    }

    onWillApplyTheme(theme: Theme) {}

    __getCustomComponentContext__Internal(): CustomComponentContext {
        if (!this.__customComponentContext__Internal) {
            this.__customComponentContext__Internal = new CustomComponentContextImpl(this);
        }
        return this.__customComponentContext__Internal as CustomComponentContext;
    }

    __registerActiveAndInactiveCallback__Internal(active?: ActiveAndInactiveCallbackType, inactive?: ActiveAndInactiveCallbackType): void {
        this.__activeFunc__Internal = active;
        this.__inactiveFunc__Internal = inactive;
    }

    public __fireActiveOrInactiveFunc__Internal(active: boolean): void {
        if (active) {
            ObserveSingleton.instance.applyTaskDelayMutableStateChange(() => {
                this.__activeFunc__Internal?.();
            });
        } else {
            ObserveSingleton.instance.applyTaskDelayMutableStateChange(() => {
                this.__inactiveFunc__Internal?.();
            });
        }
    }

    public __getCanUpdateStateVars__Internal(): boolean {
        return this.delegate_!.getCanUpdateStateVars();
    }
}

export class ExtendableComponent_serializer {
    public static write(buffer: SerializerBase, value: ExtendableComponent): void {
    }
    public static read(buffer: DeserializerBase): ExtendableComponent {
        throw new Error("TBD")
    }
}
export class LifeCycle_serializer {
    public static write(buffer: SerializerBase, value: LifeCycle): void {
    }
    public static read(buffer: DeserializerBase): LifeCycle {
        throw new Error("TBD")
    }
}
