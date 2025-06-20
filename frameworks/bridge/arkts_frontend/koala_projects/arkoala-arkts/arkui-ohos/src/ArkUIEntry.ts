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
import { int32, int64 } from "@koalaui/common"
import { pointer, nullptr, KPointer, InteropNativeModule, registerNativeModuleLibraryName, KSerializerBuffer } from "@koalaui/interop"
import { PeerNode } from "./PeerNode"
import { ArkUINativeModule } from "#components"
import { EventEmulator } from "./component"
import { UserView, UserViewBuilder, EntryPoint } from "./UserView"
import { ClickEvent, ClickEventInternal } from "./component"
import { checkEvents, setCustomEventsChecker } from "./component/Events"
import { checkArkoalaCallbacks } from "./component/peers/CallbacksChecker"
import { setUIDetachedRootCreator } from "./component/peers/CallbackTransformer"
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
import { UIContextImpl, ContextRecord } from "arkui/handwritten/UIContextImpl"
import { UIContextUtil } from "arkui/handwritten/UIContextUtil"

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

// TODO: move to Application class.
let detachedRoots: Map<KPointer, ComputableState<PeerNode>> = new Map<KPointer, ComputableState<PeerNode>>()

// mark the tree create by BuilderNode
let detachedStatMgt: Map<WeakRef<StateManager>, WeakRef<ComputableState<PeerNode>>> = new Map<WeakRef<StateManager>, WeakRef<ComputableState<PeerNode>>>()

export function createUiDetachedRoot(
    peerFactory: () => PeerNode,
    /** @memo */
    builder: () => void
): PeerNode {
    const manager = GlobalStateManager.instance
    const node = manager.updatableNode<PeerNode>(peerFactory(), (context: StateContext) => {
        const frozen = manager.frozen
        manager.frozen = true
        memoEntry<void>(context, 0, builder)
        manager.frozen = frozen
    })
    detachedRoots.set(node.value.peer.ptr, node)
    return node.value
}
setUIDetachedRootCreator(createUiDetachedRoot)

//used By BuilderNode
export function createUiDetachedBuilderRoot(
    peerFactory: () => PeerNode,
    /** @memo */
    builder: () => void,
    manager: StateManager
): ComputableState<PeerNode> {
    const node = manager.updatableNode<PeerNode>(peerFactory(), (context: StateContext) => {
        const frozen = manager.frozen
        manager.frozen = true
        memoEntry<void>(context, 0, builder)
        manager.frozen = frozen
    })
    detachedRoots.set(node.value.peer.ptr, node)
    detachedStatMgt.set(new WeakRef<StateManager>(manager), new WeakRef<ComputableState<PeerNode>>(node))
    return node
}

export function destroyUiDetachedRoot(node: PeerNode): void {
    if (!detachedRoots.has(node.peer.ptr))
        throw new Error(`Root with id ${node.peer.ptr} is not registered`)
    const root = detachedRoots.get(node.peer.ptr)!
    detachedRoots.delete(node.peer.ptr)
    root.dispose()
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
    wrapSystemCallback(1, (buff:KSerializerBuffer, len:int32) => {
        deserializeAndCallCallback(new Deserializer(buff, len))
        return 0
    })
}

export class Application {
    private manager: StateManager | undefined = undefined
    private uiContext: UIContextImpl | undefined = undefined
    private timer: MutableState<int64> | undefined = undefined
    private currentCrash: Object | undefined = undefined
    private enableDumpTree = false
    private exitApp: boolean = false
    private userView?: UserView
    private entryPoint?: EntryPoint
    private moduleName: string
    private startUrl: string
    private startParam: string

    private withLog = false
    private useNativeLog = true

    constructor(useNativeLog: boolean, moduleName: string, startUrl: string, startParam: string, userView?: UserView, entryPoint?: EntryPoint) {
        this.useNativeLog = useNativeLog
        this.moduleName = moduleName
        this.startUrl = startUrl
        this.startParam = startParam
        this.userView = userView
        this.entryPoint = entryPoint
    }

    static createMemoRootState(manager: StateManager,
        /** @memo */
        builder: UserViewBuilder,
        moduleName: string,
        initUrl: string
    ): void {
        const peer = PeerNode.generateRootPeer()
        // init router module
        Routed(builder, moduleName, peer, initUrl)
        let routerOption: router.RouterOptions = {url: initUrl}
        router.runPage(routerOption, builder)
    }

    private computeRoot(): PeerNode {
        // let handle = ArkUINativeModule._SystemAPI_StartFrame()
        let result: PeerNode
        try {
            result = router.getStateRoot().value
        } finally {
            // ArkUINativeModule._SystemAPI_EndFrame(handle)
        }
        return result
    }

    start(): pointer {
        if (this.withLog) UserView.startNativeLog(1)
        let root: PeerNode | undefined = undefined
        try {
            this.manager = GlobalStateManager.instance
            let uiContext: UIContextImpl = UIContextUtil.getOrCreateCurrentUIContext() as UIContextImpl;
            uiContext.stateMgr = this.manager
            let uiData = new ContextRecord();
            uiData.uiContext = uiContext;
            this.uiContext = uiContext;
            this.manager!.contextData = uiData;
            this.timer = getAnimationTimer() ?? createAnimationTimer(this.manager!)
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
            InteropNativeModule._NativeLog(`ArkTS Application.start before computeRoot`)
            root = this.computeRoot()
            InteropNativeModule._NativeLog(`ArkTS Application.start after computeRoot`)
        } catch (e) {
            if (e instanceof Error) {
                InteropNativeModule._NativeLog(`ArkTS Application.start error name: ${e.name} message: ${e.message}`);
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
        let rootState = router.getStateRoot();
        this.updateStates(this.manager!, rootState)
        while (StateUpdateLoop.len) {
            StateUpdateLoop.consume();
            this.updateStates(this.manager!, rootState)
        }
        // Here we request to draw a frame and call custom components callbacks.
        let root = rootState.value;
        // updateState in BuilderNode
        let deletedMap: Array<WeakRef<StateManager>> = new Array<WeakRef<StateManager>>()
        for (const mgt of detachedStatMgt) {
            let stateMgt = mgt[0]?.deref()
            if (stateMgt !== undefined && mgt[1]?.deref() !== undefined) {
                const old = GlobalStateManager.GetLocalManager();
                GlobalStateManager.SetLocalManager(stateMgt);
                this.updateStates(stateMgt!, mgt[1]!.deref()!);
                mgt[1]!.deref()!.value;
                GlobalStateManager.SetLocalManager(old);
            } else {
                deletedMap.push(mgt[0]);
            }
        }
        // delete stateManager used by BuilderNode
        for (const mgt of deletedMap) {
            detachedStatMgt.delete(mgt);
        }
        if (root.peer.ptr) {
            ArkUINativeModule._MeasureLayoutAndDraw(root.peer.ptr);
            // Call callbacks and sync
            callScheduledCallbacks();
        }
    }

    updateStates(manager: StateManager, root: ComputableState<PeerNode>) {
        // Ensure all current state updates took effect.
        manager.syncChanges()
        manager.updateSnapshot()
        this.computeRoot()
        for (const detachedRoot of detachedRoots.values())
            detachedRoot.value
        updateLazyItems()

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
                    for (const detachedRoot of detachedRoots.values())
                        detachedRoot.value
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
        // TODO: maybe
        enterForeignContext(foreignContext)
        if (this.withLog) UserView.startNativeLog(1)

        if (this.currentCrash) {
            drawCurrentCrash(this.currentCrash!)
        } else {
            try {
                this.timer!.value = Date.now() as int64
                this.loopIteration2(arg0, arg1) // loop iteration without callbacks execution
                if (this.enableDumpTree) {
                    let rootState = router.getStateRoot();
                    dumpTree(rootState.value)
                }
            } catch (error) {
                if (error instanceof Error) {
                    InteropNativeModule._NativeLog(`ArkTS Application.enter error name: ${error.name} message: ${error.message}`);
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
}

function makeClickEvent(x: number, y: number): ClickEvent {
    let result = new ClickEventInternal()
    result.x = x
    result.y = y
    return result
}
