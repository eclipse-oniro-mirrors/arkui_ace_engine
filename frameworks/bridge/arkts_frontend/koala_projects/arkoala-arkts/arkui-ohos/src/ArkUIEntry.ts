/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { ComputableState, IncrementalNode, GlobalStateManager, StateManager, StateContext, memoEntry, MutableState, createAnimationTimer, getAnimationTimer, callScheduledCallbacks } from "@koalaui/runtime"
import { KoalaProfiler, int32, int64 } from "@koalaui/common"
import { pointer, nullptr, KPointer, InteropNativeModule, registerNativeModuleLibraryName, KSerializerBuffer } from "@koalaui/interop"
import { PeerNode } from "./PeerNode"
import { ArkUINativeModule } from "#components"
import { EventEmulator } from "./component"
import { UserView, UserViewBuilder, EntryPoint } from "./UserView"
import { ClickEvent, ClickEventInternal } from "./component"
import { checkEvents, setCustomEventsChecker } from "./component/Events"
import { checkArkoalaCallbacks } from "./component/peers/CallbacksChecker"
import { enterForeignContext, leaveForeignContext } from "./handwritten"
import { wrapSystemCallback, KUint8ArrayPtr } from "@koalaui/interop"
import { deserializeAndCallCallback } from "./component/peers/CallbackDeserializeCall"
import { Deserializer } from "./component/peers/Deserializer"
import { StateUpdateLoop } from "./stateManagement"
import { Routed } from "./handwritten/Router"
import { updateLazyItems } from "./handwritten/LazyForEachImpl"
import router from "@ohos/router"
import { UIContext } from "@ohos/arkui/UIContext"
import { createStateManager } from "@koalaui/runtime"
import { UIContextImpl, ContextRecord, DetachedRootEntryManager, DetachedRootEntry } from "arkui/handwritten/UIContextImpl"
import { UIContextUtil } from "arkui/handwritten/UIContextUtil"
import { flushBuilderRootNode } from "./BuilderNode"
import { ObserveSingleton } from './stateManagement/base/observeSingleton';
import { AceTrace } from "./AceTrace"

setCustomEventsChecker(checkArkoalaCallbacks)

enum EventType {
    Click,
    Text,
    ExitApp
}

class PartialUpdateRecord {
    public update: () => void
    public context: Object
    public callback: (before: boolean) => void

    constructor(update: () => void, context: Object, callback: (before: boolean) => void) {
        this.callback = callback
        this.context = context
        this.update = update
    }
}

let partialUpdates = new Array<PartialUpdateRecord>()
let _currentPartialUpdateContext: Object | undefined = undefined
let _undefinedInstanceId : int32 = -1000

export enum MessageType {
    DELETE_DETACHED_ROOT = 1,
}

/**
 * Provide partial update lambda and context.
 *
 * @param update - function that performs state update
 * @param context - context available to UI code when state update effect happens
 */
export function addPartialUpdate<T>(update: () => void, context: T, callback: (before: boolean) => void): void {
    partialUpdates.push(new PartialUpdateRecord(update, context as Object, callback))
}

/**
 * Current partial update context or undefined.
 *
 * @returns current partial update context
 */
export function currentPartialUpdateContext<T>(): T | undefined {
    return _currentPartialUpdateContext as (T | undefined)
}

function getDetachedRootsByInstanceId(instanceId: int32): Map<KPointer, DetachedRootEntry> {
    if (instanceId === _undefinedInstanceId) {
        instanceId = UIContextUtil.getCurrentInstanceId();
        InteropNativeModule._NativeLog(
            `get DetachedRoots by current instanceId: ${instanceId}`);
    }

    let context = UIContextUtil.getOrCreateUIContextById(instanceId) as UIContextImpl;
    return context.getDetachedRootEntryManager().getDetachedRoots();
}

function getUicontextByInstanceId(instanceId: int32): UIContextImpl {
    let uicontext = UIContextUtil.getUIContextById(instanceId);
    if (uicontext === undefined) {
        uicontext = UIContextUtil.getOrCreateCurrentUIContext();
    }

    return uicontext as UIContextImpl;
}

export function createUiDetachedRoot(
    peerFactory: () => PeerNode,
    /** @memo */
    builder: () => void,
    instanceId: int32 = _undefinedInstanceId
): PeerNode {
    let uicontext = getUicontextByInstanceId(instanceId);
    return uicontext.getDetachedRootEntryManager().createUiDetachedRoot(peerFactory, builder);
}

export function createUiDetachedRootT<T>(
    peerFactory: () => PeerNode,
    /** @memo */
    builder: (arg: T) => void,
    arg: T,
    instanceId: int32 = _undefinedInstanceId
): PeerNode {
    let uicontext = getUicontextByInstanceId(instanceId);
    return uicontext.getDetachedRootEntryManager().createUiDetachedRootT(peerFactory, builder, arg);
}

export function destroyUiDetachedRoot(ptr: KPointer, instanceId: int32): boolean {
    if (instanceId < 0) {
        InteropNativeModule._NativeLog(
            `ArkTS destroyUiDetachedRoot failed due to instanceId: ${instanceId} is illegal`)
        return false;
    }

    let uicontext = getUicontextByInstanceId(instanceId);
    return uicontext.getDetachedRootEntryManager().destroyUiDetachedRoot(ptr);
}

function dumpTree(node: IncrementalNode, indent: int32 = 0) {
    const indentToString = (indent: number) => {
        let str = ""
        for (let i = 0; i <= indent; i++) str += " "
        return str
    }

    if (indent == 0) InteropNativeModule._NativeLog("> Dump tree:")

    let child = node.firstChild
    InteropNativeModule._NativeLog("> " + indentToString(indent++) + "| " + node)

    while (child) {
        dumpTree(child!, indent + 1)
        child = child!.nextSibling
    }
}

function errorInfo(crash: Object): string {
    let msg = crash.toString() + "\n"
    if (Object.hasOwn(crash, "stack")) msg += (crash as Error).stack
    return msg
}

let crashDumped = false
function drawCurrentCrash(crash: Object) {
    let msg = errorInfo(crash)
    if (!crashDumped) {
        InteropNativeModule._NativeLog(msg)
        crashDumped = true
    }
    ArkUINativeModule._ShowCrash(msg ?? "unknown error message")
}

function registerSyncCallbackProcessor() {
    wrapSystemCallback(1, (buff: KSerializerBuffer, len: int32) => {
        deserializeAndCallCallback(new Deserializer(buff, len))
        return 0
    })
}

export class Application {
    private manager: StateManager | undefined = undefined
    private uiContext: UIContextImpl | undefined = undefined
    private currentCrash: Object | undefined = undefined
    private enableDumpTree = false
    private exitApp: boolean = false
    private userView?: UserView
    private entryPoint?: EntryPoint
    private moduleName: string
    private startUrl: string
    private startParam: string
    private instanceId: int32 = -1

    private withLog = false
    private useNativeLog = true
    private rootState: ComputableState<PeerNode> | undefined = undefined

    constructor(useNativeLog: boolean, moduleName: string, startUrl: string, startParam: string, userView?: UserView, entryPoint?: EntryPoint) {
        this.userView = userView
        this.entryPoint = entryPoint
        this.moduleName = moduleName
        this.startUrl = startUrl
        this.startParam = startParam
        this.useNativeLog = useNativeLog
    }

    static createMemoRootState(manager: StateManager,
        /** @memo */
        builder: UserViewBuilder,
        moduleName: string,
        initUrl: string
    ): void {
        // init router module
        Routed(builder, moduleName, initUrl)
        let routerOption: router.RouterOptions = {url: initUrl}
        router.runPage(routerOption, builder)
    }

    start(): pointer {
        KoalaProfiler.initTrace(AceTrace.begin, AceTrace.end)
        KoalaProfiler.initNativeLog(InteropNativeModule._NativeLog)
        if (this.withLog) UserView.startNativeLog(1)
        let root: PeerNode | undefined = undefined
        try {
            this.manager = GlobalStateManager.instance
            let uiContext: UIContextImpl = UIContextUtil.getOrCreateCurrentUIContext() as UIContextImpl;
            this.instanceId = uiContext.getInstanceId();
            uiContext.stateMgr = this.manager
            let uiData = new ContextRecord();
            uiData.uiContext = uiContext;
            this.uiContext = uiContext;
            this.manager!.contextData = uiData;
            this.manager!.isDebugMode = uiContext.isDebugMode_;
            this.manager!.setThreadChecker(() => UIContextUtil.checkCurrentThread());
            /** @memo */
            let builder: UserViewBuilder
            if (this.entryPoint) {
                builder = this.entryPoint!.entry
            } else if (this.userView) {
                builder = this.userView!.getBuilder()
            } else {
                throw new Error("Invalid EntryPoint")
            }
            Application.createMemoRootState(this.manager!, builder, this.moduleName, this.startUrl)
            InteropNativeModule._NativeLog(`[${this.instanceId}] ArkTS Application.start before computeRoot`)
            const manager = GlobalStateManager.instance
            this.rootState = manager.updatableNode<PeerNode>(PeerNode.generateRootPeer(), (context: StateContext) => {})
            root = this.rootState!.value
            InteropNativeModule._NativeLog(`[${this.instanceId}] ArkTS Application.start after computeRoot`)
        } catch (e) {
            if (e instanceof Error) {
                InteropNativeModule._NativeLog(`[${this.instanceId}] ArkTS Application.start error name: ${e.name} message: ${e.message}`);
                const stack = e.stack
                if (stack) {
                    InteropNativeModule._NativeLog("ArkTS Application.start stack trace: " + stack)
                }
                return nullptr
            }
        }
        if (this.withLog) {
            UserView.stopNativeLog(1)
            if (this.useNativeLog) {
                InteropNativeModule._PrintGroupedLog(1)
            } else {
                let log = UserView.getNativeLog(1)
                if (log.length > 0) {
                    InteropNativeModule._NativeLog(log)
                }
            }
        }
        return root!.peer.ptr
    }

    handleMessage(ptr: KPointer, type: int32, param : string) : boolean {
        let result : boolean = false
        switch (type as MessageType) {
            case MessageType.DELETE_DETACHED_ROOT: {
                result = destroyUiDetachedRoot(ptr, this.instanceId);
                break
            }
            default: {
                InteropNativeModule._NativeLog(`[${this.instanceId}] ARKTS: [handleMessage] type = ${type} is unknown.`)
                break
            }
        }
        InteropNativeModule._NativeLog(`[${this.instanceId}] ARKTS: [handleMessage] ptr: ${ptr}, type: ${type}, param: ${param}`);
        return result;
    }

    private checkEvents(what: int32) {
        // NativeModule._NativeLog("ARKTS: checkEvents")
        checkEvents()
    }

    private checkUIContextCallbacks(): void {
        const uiContext = this.uiContext
        if (uiContext) {
            if (this.withLog) InteropNativeModule._NativeLog("ARKTS: checkUIContextCallbacks")
            uiContext.callCallbacks()
        }
    }

    private updateState() {
        // NativeModule._NativeLog("ARKTS: updateState")
        let uiContextRouter = this.uiContext!.getRouter();
        let rootState = uiContextRouter.getStateRoot();
        let preState = uiContextRouter.getPreState();
        ObserveSingleton.instance.updateDirty();
        this.updateStates(this.manager!, rootState, preState);
        ObserveSingleton.instance.makeDelayedMutableStateChanges();
        while (StateUpdateLoop.len) {
            StateUpdateLoop.consume();
            ObserveSingleton.instance.updateDirty();
            this.updateStates(this.manager!, rootState, preState);
            ObserveSingleton.instance.makeDelayedMutableStateChanges();
        }
        // Here we request to draw a frame and call custom components callbacks.
        rootState!.value;
        if (preState) {
            preState!.value;
        }
        // Call callbacks and sync
        callScheduledCallbacks();
    }

    updateStates(manager: StateManager, root: ComputableState<IncrementalNode>, pre: ComputableState<IncrementalNode> | undefined) {
        if (this.instanceId < 0) {
            InteropNativeModule._NativeLog(
                `ArkTS updateStates failed due to instanceId: ${this.instanceId} is illegal`)
            return;
        }
        // Ensure all current state updates took effect.
        manager.syncChanges()
        manager.updateSnapshot()
        let detachedRoots = getDetachedRootsByInstanceId(this.instanceId);
        for (const detachedRoot of detachedRoots.values()) {
            detachedRoot.compute()
        }
        updateLazyItems()
        flushBuilderRootNode()
        if (partialUpdates.length > 0) {
            // If there are pending partial updates - we apply them one by one and provide update context.
            for (let update of partialUpdates) {
                // Set the context available via currentPartialUpdateContext() to @memo code.
                _currentPartialUpdateContext = update.context
                // Update states.
                update.update()
                // Propagate changes.
                manager.syncChanges()
                manager.updateSnapshot()
                // Notify subscriber.
                update.callback(true)
                // Compute new tree state
                try {
                    root.value
                    if (pre) {
                        pre!.value;
                    }
                    for (const detachedRoot of detachedRoots.values()) {
                        detachedRoot.compute()
                    }
                } catch (error) {
                    InteropNativeModule._NativeLog('has error in partialUpdates')
                }
                // Notify subscriber.
                update.callback(false)
                // Clear context.
                _currentPartialUpdateContext = undefined
            }
            // Clear partial updates list.
            partialUpdates.splice(0, partialUpdates.length)
        }
    }


    private render() {
        if (this.withLog) InteropNativeModule._NativeLog("ARKTS: render")
    }
    enter(arg0: int32, arg1: int32, foreignContext: pointer): boolean {
        StateUpdateLoop.canRequestFrame = true;
        enterForeignContext(foreignContext)
        if (this.withLog) UserView.startNativeLog(1)

        if (this.currentCrash) {
            drawCurrentCrash(this.currentCrash!)
        } else {
            try {
                this.loopIteration2(arg0, arg1) // loop iteration without callbacks execution
                if (this.enableDumpTree) {
                    dumpTree(this.rootState!.value)
                }
            } catch (error) {
                if (error instanceof Error) {
                    InteropNativeModule._NativeLog(`[${this.instanceId}]ArkTS Application.enter error name: ${error.name} message: ${error.message}`);
                    if (error.stack) {
                        leaveForeignContext()
                        InteropNativeModule._NativeLog(error.stack!.toString())
                        return true
                    }
                }
                this.currentCrash = error as Object
            }
        }
        if (this.withLog) {
            UserView.stopNativeLog(1)
            if (this.useNativeLog) {
                InteropNativeModule._PrintGroupedLog(1)
            } else {
                let log = UserView.getNativeLog(1)
                if (log.length > 0) {
                    InteropNativeModule._NativeLog(log)
                }
            }
        }
        leaveForeignContext()
        return this.exitApp
    }

    loopIteration(arg0: int32, arg1: int32) {
        if (this.withLog) InteropNativeModule._NativeLog("ARKTS: loopIteration")
        this.checkEvents(arg0)
        this.updateState()
        this.render()
    }

    // loop iteration without callbacks execution, callbacks execution will be done at the tail of vsync
    loopIteration2(arg0: int32, arg1: int32) {
        if (this.withLog) InteropNativeModule._NativeLog("ARKTS: loopIteration2")
        this.updateState()
        this.render()
    }

    // called at the tail of vsync
    checkCallbacks(): void {
        if (this.withLog) InteropNativeModule._NativeLog("ARKTS: checkCallbacks")
        checkEvents()
        this.checkUIContextCallbacks()
    }

    // TODO: make [emitEvent] suitable to get string argument
    emitEvent(type: int32, target: int32, arg0: int32, arg1: int32): string {
        console.log("emitEvent for " + target)
        const node = PeerNode.findPeerByNativeId(target)
        if (node != undefined) {
            try {
                switch (type as EventType) {
                    case EventType.Click: {
                        EventEmulator.emitClickEvent(node.peer.ptr, makeClickEvent(10, 20))
                        break
                    }
                    case EventType.Text: {
                        InteropNativeModule._NativeLog("ARKTS: [emitEvent] EventType.Text is not implemented." + type)
                        break
                    }
                    case EventType.ExitApp: {
                        this.exitApp = true
                        break
                    }
                    default: {
                        InteropNativeModule._NativeLog("ARKTS: [emitEvent] type = " + type + " is unknown.")
                        break
                    }
                }
            } catch (error) {
                InteropNativeModule._NativeLog("emitEvent error: " + errorInfo(error as Object))
            }
        } else {
            console.log(`Cannot find target node ${target}`)
        }
        return "0"
    }

    notifyConfigurationChange() {
        if (!this.uiContext || !this.manager) {
            console.warn("Arkoala haven't been initialized !");
            return;
        }
        let uiContextRouter = this.uiContext!.getRouter();
        let rootState = uiContextRouter.getStateRoot();
        rootState.forceCompleteRerender();
        this.manager!.syncChanges();
        this.manager!.updateSnapshot();
    }

    static createApplication(startUrl: string, startParams: string, useNativeLog: boolean, moduleName: string, userView?: UserView, entryPoint?: EntryPoint): Application {
        if (!userView && !entryPoint) {
            throw new Error(`Invalid EntryPoint`)
        }
        registerNativeModuleLibraryName("InteropNativeModule", "ArkoalaNative_ark.z")
        registerNativeModuleLibraryName("ArkUINativeModule", "ArkoalaNative_ark.z")
        registerNativeModuleLibraryName("ArkUIGeneratedNativeModule", "ArkoalaNative_ark.z")
        registerNativeModuleLibraryName("TestNativeModule", "ArkoalaNative_ark.z")
        registerSyncCallbackProcessor()
        return new Application(useNativeLog, moduleName, startUrl, startParams, userView, entryPoint)
    }

    static registerNativeModulePreloader(): void {
        registerNativeModuleLibraryName("InteropNativeModule", "ArkoalaNative_ark.z")
        registerNativeModuleLibraryName("ArkUINativeModule", "ArkoalaNative_ark.z")
        registerNativeModuleLibraryName("ArkUIGeneratedNativeModule", "ArkoalaNative_ark.z")
        registerNativeModuleLibraryName("TestNativeModule", "ArkoalaNative_ark.z")
    }
}

function makeClickEvent(x: number, y: number): ClickEvent {
    let result = new ClickEventInternal()
    result.x = x
    result.y = y
    return result
}
