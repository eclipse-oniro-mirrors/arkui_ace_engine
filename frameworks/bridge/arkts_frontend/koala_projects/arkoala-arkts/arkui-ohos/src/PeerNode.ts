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

import { int32, KoalaCallsiteKey } from "@koalaui/common"
import { Disposable, GlobalStateManager, IncrementalNode, MutableState, scheduleCallback } from "@koalaui/runtime"
import { NativePeerNode } from "./NativePeerNode"
import { nullptr, pointer } from "@koalaui/interop"
import { ArkRootPeer } from "./component"
import { ReusablePool } from "./ReusablePool"
import { StateStylesOps } from './component/arkui-custom'
import { ArkUIAniModule } from "arkui.ani"
import { StateUpdateLoop } from "./stateManagement"     

export const PeerNodeType = 11
export const RootPeerType = 33
export const LazyItemNodeType = 17 // LazyItems are detached node trees that are stored privately in LazyForEach
export const BuilderRootNodeType = 19 // BuilderRootNode are detached node trees that are stored privately in BuilderNode
const INITIAL_ID = 0
const ID_CAPACITY = 10000
const UNINITIALIZE_CURSOR = -1

export class PeerNode extends IncrementalNode {
    static generateRootPeer() {
        return ArkRootPeer.create(undefined)
    }
    peer: NativePeerNode
    protected static currentId: int32 = INITIAL_ID
    protected static cursor: int32 = UNINITIALIZE_CURSOR
    static nextId(): int32 {
        if (PeerNode.cursor === UNINITIALIZE_CURSOR || PeerNode.currentId >= PeerNode.cursor + ID_CAPACITY) {
            PeerNode.cursor = ArkUIAniModule._RequireArkoalaNodeId(ID_CAPACITY)
            PeerNode.currentId = PeerNode.cursor
        }
        return PeerNode.currentId++;
    }
    override incrementalUpdateDone(parent?: IncrementalNode) {
        super.incrementalUpdateDone(parent)
        ArkUIAniModule._UINode_OnUpdateDone(this.peer.ptr)
    }
    private id: int32
    private _reuseCb?: () => void
    private _recycleCb?: () => void
    // Pool to store recycled child scopes, grouped by type
    private _reusePool?: Map<string, ReusablePool>
    reusable: boolean = false
    private _uiStateStyle?: MutableState<int32>;

    getPeerPtr(): pointer {
        return this.peer.ptr
    }

    setId(id: int32) {
        PeerNode.peerNodeMap.delete(this.id)
        this.id = id
        PeerNode.peerNodeMap.set(this.id, this)
    }

    getId(): int32 {
        return this.id
    }

    onReuse(): void {
        if (!this.reusable) {
            return
        }
        scheduleCallback(this._reuseCb) // could change states
    }

    onRecycle(): void {
        this._recycleCb?.()
    }

    updateReusePoolSize(size: number, reuseKey: string) {
        this._reusePool?.get(reuseKey)?.setMaxSize(size)
    }

    /* reuse and recycle object on RootPeers */
    override reuse(reuseKey: string | undefined, id: KoalaCallsiteKey): Disposable | undefined {
        if (reuseKey === undefined) {
            return undefined;
        }

        if (!this.isKind(RootPeerType))
            return this.parent?.reuse(reuseKey, id)

        if (this._reusePool === undefined)
            return undefined
        if (this._reusePool!.has(reuseKey)) {
            const pool = this._reusePool!.get(reuseKey)!;
            return pool.get();
        }
        return undefined;
    }

    override recycle(reuseKey: string | undefined, child: Disposable, id: KoalaCallsiteKey): boolean {
        if (reuseKey === undefined) {
            return false;
        }

        if (!this.isKind(RootPeerType)) {
            return this.parent?.recycle(reuseKey, child, id) ?? false
        }
        if (!this._reusePool)
            this._reusePool = new Map<string, ReusablePool>()
        if (!this._reusePool!.has(reuseKey)) {
            this._reusePool!.set(reuseKey, new ReusablePool());
        }
        this._reusePool!.get(reuseKey)!.put(child);
        return true
    }

    setReusePoolSize(size: number, reuseKey: string): void {
        if (size < 0) return
        if (!this.isKind(RootPeerType)) {
            if (this.parent?.isKind(PeerNodeType))
                (this.parent! as PeerNode).setReusePoolSize(size, reuseKey)
            return
        }
        if (!this._reusePool) {
            this._reusePool = new Map<string, ReusablePool>()
        }
        if (!this._reusePool?.has(reuseKey)) {
            this._reusePool?.set(reuseKey, new ReusablePool())
        }
        this._reusePool?.get(reuseKey)?.setMaxSize(size)
    }

    setOnRecycle(cb: () => void): void {
        this._recycleCb = cb
    }
    setOnReuse(cb: () => void): void {
        this._reuseCb = cb
    }

    private static peerNodeMap = new Map<number, PeerNode>()

    static findPeerByNativeId(id: number): PeerNode | undefined {
        return PeerNode.peerNodeMap.get(id)
    }
    readonly name: string
    private insertMark: pointer = nullptr
    private insertDirection: int32 = 0

    setInsertMark(mark: pointer, upDirection: boolean) {
        this.insertMark = mark
        this.insertDirection = upDirection ? 0 : 1
    }

    constructor(peerPtr: pointer, id: int32, name: string, flags: int32, derivedNodeType?: int32) {
        super(derivedNodeType ?? PeerNodeType)
        this.id = id
        this.peer = NativePeerNode.create(this, peerPtr, flags)
        PeerNode.peerNodeMap.set(this.id, this)
        this.onChildInserted = (child: IncrementalNode) => {
            // TODO: rework to avoid search
            let peer = findPeerNode(child)
            if (peer) {
                peer.reusable ? peer!.onReuse() : peer.reusable = true // becomes reusable after initial mount
                let peerPtr = peer.peer.ptr
                if (this.insertMark != nullptr) {
                    if (this.insertDirection == 0) {
                        this.peer.insertChildBefore(peerPtr, this.insertMark)
                    } else {
                        this.peer.insertChildAfter(peerPtr, this.insertMark)
                    }
                    this.insertMark = peerPtr
                    return
                }
                // Find the closest peer node forward.
                let sibling: PeerNode | undefined = findSiblingPeerNode(child, true)
                if (sibling === undefined) {
                    // Add to the end (common case!).
                    this.peer.addChild(peerPtr)
                } else {
                    // Insert child in the middle.
                    this.peer.insertChildBefore(peerPtr, sibling?.peer?.ptr ?? nullptr)
                }
            }
        }
        this.onChildRemoved = (child: IncrementalNode) => {
            if (child.isKind(PeerNodeType) && !child.disposed) {
                const peer = child as PeerNode
                peer.onRecycle()
                this.peer.removeChild(peer.peer.ptr)
            }
        }
        this.name = name
    }
    applyAttributes(attrs: Object) { }

    override dispose(): void {
        let parent = this.parent
        if (parent != undefined && parent.isKind(PeerNodeType)) {
            (parent as PeerNode).peer.removeChild(this.peer.ptr)
        }
        this.peer.dispose()
        PeerNode.peerNodeMap.delete(this.id)
        this._reusePool?.forEach((pool: ReusablePool) =>
            pool.dispose()
        )
        this._reusePool = undefined
        this._recycleCb = undefined
        this._reuseCb = undefined
        this._uiStateStyle = undefined;
        super.dispose()
    }

    public getStateStyleMutable(): MutableState<int32> | undefined {
        return this._uiStateStyle;
    }
    public getOrCreateStateStyleMutable(): MutableState<int32> | undefined {
        if (this._uiStateStyle !== undefined) {
            return this._uiStateStyle!;
        }
        else {
            const manager = GlobalStateManager.instance
            this._uiStateStyle = manager.mutableState<int32>(0 as int32, true)
            StateStylesOps.onStateStyleChange(this.getPeerPtr(), (state: int32) => {
                StateUpdateLoop.add(() => {
                    this._uiStateStyle!.value = state
                })
            })
            return this._uiStateStyle!;
        }
    }
}

export function findPeerNode(node: IncrementalNode): PeerNode | undefined {
    if (node.isKind(PeerNodeType)) return node as PeerNode
    for (let child = node.firstChild; child; child = child!.nextSibling) {
        let peer = findPeerNode(child!)
        if (peer) return peer
    }
    return undefined
}

function findSiblingPeerNode(node: IncrementalNode, forward: boolean): PeerNode | undefined {
    if (forward) {
        for (let sibling = node.nextSibling; sibling; sibling = sibling!.nextSibling) {
            if (sibling!.isKind(PeerNodeType)) {
                return sibling as PeerNode
            }
        }
    } else {
        for (let sibling = node.previousSibling; sibling; sibling = sibling!.previousSibling) {
            if (sibling!.isKind(PeerNodeType)) {
                return sibling as PeerNode
            }
        }
    }
    return undefined
}
