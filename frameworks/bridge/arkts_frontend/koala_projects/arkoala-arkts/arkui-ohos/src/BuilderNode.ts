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

import { TouchEvent, } from "./component/common"
import { Size } from "./Graphics"
import { UIContext } from "@ohos/arkui/UIContext"
import { Finalizable, InteropNativeModule, KPointer, loadNativeModuleLibrary, nullptr, pointer } from "@koalaui/interop"
import { finalizerRegister, Thunk, int32 } from "@koalaui/common"
import { ComputableState, MutableState, rememberMutableState, StateManager, memoEntry, StateContext, createStateManager, GlobalStateManager, IncrementalNode } from "@koalaui/runtime"
import { WrappedBuilder } from "./component/builder"
import { BuilderRootFrameNode, FrameNode, FrameNodeUtils } from "./FrameNode"
import { BuilderRootNodeType, findPeerNode, PeerNode, PeerNodeType } from "./PeerNode"
import { BuilderNodeOps, BuilderNodeOptions } from "./component/arkui-custom"
import { ArkBuilderProxyNodePeer } from "./handwritten/BuilderProxyNode"
import { setNeedCreate } from "./ArkComponentRoot"
import { UIContextUtil } from 'arkui/handwritten/UIContextUtil'
import { ArkUIAniModule } from 'arkui.ani'

export enum NodeRenderType {
    RENDER_TYPE_DISPLAY = 0,
    RENDER_TYPE_TEXTURE = 1
}
export interface RenderOptions {
    selfIdealSize?: Size;
    type?: NodeRenderType;
    surfaceId?: string;
}
export interface BuildOptions {
    nestingBuilderSupported?: boolean;
}

const __uiNode_name_map__: Array<string> = ["ContentSlot", "CustomComponent"]

class BuilderRootNode extends IncrementalNode {
    public __parent?: ArkBuilderProxyNodePeer;

    constructor() {
        super(BuilderRootNodeType)
    }
    getPeerPtr(): pointer {
        let peer = findPeerNode(this)
        return peer ? peer!.getPeerPtr() : nullptr
    }
    isFirstUINode(): boolean {
        const peer = this.firstChild
        if (peer?.isKind(PeerNodeType)) {
            return __uiNode_name_map__.includes((peer as PeerNode)!.name!)
        } else {
            return false;
        }
    }
}
// mark the tree create by BuilderNode
let detachedStatMgt: Map<WeakRef<StateManager>, WeakRef<ComputableState<BuilderRootNode>>> = new Map<WeakRef<StateManager>, WeakRef<ComputableState<BuilderRootNode>>>()


function createUiDetachedBuilderRoot(
    peerFactory: () => BuilderRootNode,
    /** @memo */
    builder: () => void,
    manager: StateManager,
    instanceId: int32
): ComputableState<BuilderRootNode> {
    const node = manager.updatableNode<BuilderRootNode>(peerFactory(), (context: StateContext) => {
        const frozen = manager.frozen
        manager.frozen = true
        ArkUIAniModule._Common_Sync_InstanceId(instanceId)
        memoEntry<void>(context, 0, builder)
        ArkUIAniModule._Common_Restore_InstanceId()
        manager.frozen = frozen
    })
    detachedStatMgt.set(new WeakRef<StateManager>(manager), new WeakRef<ComputableState<BuilderRootNode>>(node))
    return node
}

export function flushBuilderRootNode(): void {
    // updateState in BuilderNode
    let deletedMap: Array<WeakRef<StateManager>> = new Array<WeakRef<StateManager>>()
    for (const mgt of detachedStatMgt) {
        let stateMgt = mgt[0]?.deref()
        let peerNode = mgt[1]?.deref()
        if (stateMgt !== undefined && peerNode !== undefined) {
            const old = GlobalStateManager.GetLocalManager();
            GlobalStateManager.SetLocalManager(stateMgt);
            stateMgt.syncChanges()
            stateMgt.updateSnapshot()
            peerNode!.value;
            GlobalStateManager.SetLocalManager(old);
        } else {
            deletedMap.push(mgt[0]);
        }
    }
    // delete stateManager used by BuilderNode
    for (const mgt of deletedMap) {
        detachedStatMgt.delete(mgt);
    }
}

class BuilderNodeFinalizationRegisterCleanerThunk<T> implements Thunk {
    private cleaner?: (builderNode: JSBuilderNode<T>) => void
    private __jsBuilderNode?: JSBuilderNode<T>
    constructor(cleaner: (builderNode: JSBuilderNode<T>) => void, jsBuilderNode: JSBuilderNode<T>) {
        this.cleaner = cleaner
        this.__jsBuilderNode = jsBuilderNode;
    }
    clean() {
        if (this.__jsBuilderNode) {
            this.cleaner?.(this.__jsBuilderNode!)
        }
        this.cleaner = undefined;
        this.__jsBuilderNode = undefined;
    }
}

class BuilderNodeFinalizationRegisterProxy {
    public static removeJSBuilderNode<T>(builderNode: JSBuilderNode<T>): void {
        if (builderNode) {
            builderNode.dispose();
        }
    }
    public static register<T>(target: object): void {
        let builderNode = target as BuilderNode<T>
        let trunk = new BuilderNodeFinalizationRegisterCleanerThunk<T>(BuilderNodeFinalizationRegisterProxy.removeJSBuilderNode, builderNode._JSBuilderNode)
        finalizerRegister(builderNode, trunk)
    }
    public static instance_: BuilderNodeFinalizationRegisterProxy = new BuilderNodeFinalizationRegisterProxy();
}

export type voidBuilderFunc = /** @memo */ () => void;

export type TBuilderFunc<T> = /** @memo */ (t: T) => void;

export class BuilderNode<T = undefined>{
    public _JSBuilderNode: JSBuilderNode<T>;
    constructor(uiContext: UIContext, options?: RenderOptions) {
        this._JSBuilderNode = new JSBuilderNode<T>(uiContext, options);
        BuilderNodeFinalizationRegisterProxy.register<T>(this);
    }
    //@ts-ignore
    public build(builder: WrappedBuilder<TBuilderFunc<T>>, params: T, options?: BuildOptions): void {
        this._JSBuilderNode.buildT(builder, params, options);
    }
    //@ts-ignore
    public build(builder: WrappedBuilder<TBuilderFunc<T>>, params: T): void {
        this._JSBuilderNode.buildT(builder, params);
    }
    //@ts-ignore
    public build(builder: WrappedBuilder<voidBuilderFunc>): void {
        this._JSBuilderNode.build0(builder);
    }
    public update(params: T): void {
        this._JSBuilderNode.update(params);
    }
    public getNodePtr(): KPointer | undefined {
        return this._JSBuilderNode.getValidNodePtr();
    }
    public getFrameNode(): FrameNode | null {
        return this._JSBuilderNode.getFrameNode();
    }
    public getFrameNodeWithoutCheck(): FrameNode | null | undefined {
        return this._JSBuilderNode.getFrameNodeWithoutCheck();
    }
    public postTouchEvent(touchEvent: TouchEvent): boolean {
        let ret = this._JSBuilderNode.postTouchEvent(touchEvent);
        return ret;
    }
    public dispose(): void {
        this._JSBuilderNode.disposeNode();
    }
    public reuse(param?: Record<string, Object>): void {
        this._JSBuilderNode.reuse(param);
    }
    public recycle(): void {
        this._JSBuilderNode.recycle();
    }
    public updateConfiguration(): void {
        this._JSBuilderNode.updateConfiguration();
    }
}

export class JSBuilderNode<T> extends BuilderNodeOps {
    peer?: Finalizable | undefined = undefined
    private __root?: BuilderRootNode;
    private __builderRootNode?: PeerNode;
    private __builderProxyNode?: ArkBuilderProxyNodePeer;
    private __arg?: T;
    private __rootStage?: ComputableState<BuilderRootNode>;
    private __manager: StateManager;
    private __params?: MutableState<T>;
    private __builder?: WrappedBuilder<TBuilderFunc<T>>;
    private __builder0?: WrappedBuilder<voidBuilderFunc>;
    private __frameNode: BuilderRootFrameNode<T> | null;
    private __uiContext?: UIContext;

    private reset(): void {
        this.__root = undefined;
        this.__builderProxyNode = undefined;
        this.__builderRootNode = undefined;
        this.__rootStage = undefined;
        this.__params = undefined;
        this.__arg = undefined;
        this.__builder0 = undefined;
        this.__builder = undefined;
        this.__frameNode = null;
    }
    public getPeer(): Finalizable | undefined {
        return this.peer
    }
    constructor(uiContext: UIContext, options?: RenderOptions) {
        super();
        this.__uiContext = uiContext;
        if (options !== undefined) {
            let buildOptions: BuilderNodeOptions = {};
            if (options?.selfIdealSize !== undefined) {
                buildOptions.selfIdealSize = options.selfIdealSize;
            }
            if (options?.surfaceId !== undefined) {
                buildOptions.surfaceId = options.surfaceId;
            }
            if (options?.type !== undefined) {
                buildOptions.type = options.type!;
            }
            this.setOptions(buildOptions);
        }
        this.__manager = createStateManager()!;
        this.__frameNode = null;
    }

    public buildFunc(): void {
        let instanceId : int32 = 0;
        if (this.__uiContext === undefined) {
            instanceId = UIContextUtil.getCurrentInstanceId();
        }
        this.__rootStage = createUiDetachedBuilderRoot(() => {
            if (this.__root == null) {
                this.__root = new BuilderRootNode();
            }
            return this.__root!;
        }, () => {
            if (this.__builder0) {
                const result = setNeedCreate(true);
                this.__builder0?.builder();
                setNeedCreate(result);
                return;
            }
            if (this.__params === undefined && this.__arg !== undefined) {
                this.__params = rememberMutableState<T>(this.__arg!);
            }
            if (this.__params?.value) {
                const result = setNeedCreate(true);
                this.__builder?.builder(this.__params!.value);
                setNeedCreate(result);
            }
        }, this.__manager!, instanceId)
        this.__rootStage?.value;
        if (!this.__root?.firstChild || !findPeerNode(this.__root!)?.getPeerPtr()) {
            this.reset();
            return;
        }
        if (this.__root?.isFirstUINode()) {
            this.__builderProxyNode = ArkBuilderProxyNodePeer.create(undefined);
            this.__root!.__parent = this.__builderProxyNode;
            this.__builderProxyNode!.hasChild = true;
            this.__builderProxyNode?.peer?.addChild(this.__root!.getPeerPtr());
            this.__builderRootNode = this.__builderProxyNode;
        } else {
            this.__builderRootNode = findPeerNode(this.__root!);
        }
        this.__frameNode = null;
        if (this.__builderRootNode?.getPeerPtr() !== undefined && this.__builderRootNode?.getPeerPtr() !== nullptr) {
            if (this.__uiContext === undefined) {
                InteropNativeModule._NativeLog("UIContext in BuilderNode is undefined, please check it.");
                return;
            }
            const frameNodeOfBuilderNode: KPointer = this.setRootFrameNodeInBuilderNode(this.__builderRootNode!.getPeerPtr()!)
            this.__frameNode = FrameNodeUtils.createBuilderRootFrameNode<T>(this.__uiContext!, frameNodeOfBuilderNode);
            this.__frameNode?.setJsBuilderNode(this);
            this.setUpdateConfigurationCallback(this.updateConfiguration);
        }
    }

    public build0(builder: WrappedBuilder<voidBuilderFunc>): void {
        this.__builder0 = builder;
        this.__arg = undefined;
        this.__builder = undefined;
        const old = GlobalStateManager.GetLocalManager();
        GlobalStateManager.SetLocalManager(this.__manager);
        this.create(this.buildFunc);
        GlobalStateManager.SetLocalManager(old);
    }

    public buildT(builder: WrappedBuilder<TBuilderFunc<T>>, arg?: T, options?: BuildOptions): void {
        this.__builder = builder;
        this.__builder0 = undefined;
        this.__arg = arg;
        const old = GlobalStateManager.GetLocalManager();
        GlobalStateManager.SetLocalManager(this.__manager);
        this.create(this.buildFunc);
        GlobalStateManager.SetLocalManager(old);
    }

    public update(arg: T): void {
        if (this.__params === undefined) {
            InteropNativeModule._NativeLog("the params is not Initialized!!!");
            return;
        }
        const old = GlobalStateManager.GetLocalManager();
        GlobalStateManager.SetLocalManager(this.__manager);
        this.__params!.value = arg!;
        this.__manager!.syncChanges();
        this.__manager!.updateSnapshot();
        this.__rootStage?.value;
        GlobalStateManager.SetLocalManager(old);
    }

    public getFrameNode(): FrameNode | null {
        if (this.__frameNode !== undefined &&
            this.__frameNode !== null && this.__frameNode!._nodeId !== - 1) {
            return this.__frameNode!;
        }
        return null;
    }

    public reuse(param?: Record<string, Object>): void {
    }

    public recycle(): void {
    }

    public getFrameNodeWithoutCheck(): FrameNode | null | undefined {
        return this.__frameNode;
    }

    public updateConfiguration(): void {
        const old = GlobalStateManager.GetLocalManager();
        GlobalStateManager.SetLocalManager(this.__manager);
        this.__rootStage?.forceCompleteRerender();
        this.__manager?.syncChanges();
        this.__manager?.updateSnapshot();
        this.__rootStage?.value;
        GlobalStateManager.SetLocalManager(old);
    }

    public getValidNodePtr(): KPointer | undefined {
        return this.__frameNode?.peer?.ptr;
    }

    public disposeNode(): void {
        this.disposeAll();
        this.__frameNode = null;
    }

    private disposeAll(): void {
        super.disposeNode();
        this.__rootStage?.dispose();
        this.__builderProxyNode?.dispose();
        this.__manager.reset();
    }

    public dispose(): void {
        this.disposeAll();
        this.__frameNode?.disposeNode();
        this.__frameNode = null;
    }
}