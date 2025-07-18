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


// WARNING! THIS FILE IS AUTO-GENERATED, DO NOT MAKE CHANGES, THEY WILL BE LOST ON NEXT GENERATION!

import { UIContext } from "@ohos/arkui/UIContext"
import { UIContextImpl } from "arkui/handwritten/UIContextImpl"
import { Position, Edges, Size, LengthMetrics, SizeT } from "./Graphics"
import { TypeChecker, ArkUIGeneratedNativeModule } from "#components"
import {
    Finalizable, runtimeType, RuntimeType, SerializerBase, registerCallback, wrapCallback, toPeerPtr, KPointer,
    MaterializedBase, NativeBuffer, nullptr, pointer, KSerializerBuffer, KUint8ArrayPtr
} from "@koalaui/interop"
import { unsafeCast, int32, float32 } from "@koalaui/common"
import { Serializer } from "./component"
import { ArkUIAniModule } from "arkui.ani"
import { RenderNode, RenderNodeInternal } from "./RenderNode"
import { CommonAttribute, ArkCommonMethodPeer, CommonMethod, UIGestureEvent, UICommonEvent, UICommonEventInternal } from './component/common'
import { ArkBaseNode } from './handwritten/modifiers/ArkBaseNode'
import { ArkListNode } from './handwritten/modifiers/ArkListNode'
import { ArkSearchNode } from './handwritten/modifiers/ArkSearchNode'
import { ArkTextAreaNode } from './handwritten/modifiers/ArkTextAreaNode'
import { ArkTextInputNode } from './handwritten/modifiers/ArkTextInputNode'
import { ModifierType } from './handwritten/modifiers/ArkCommonModifier'
import { ListOptions, ListAttribute, ArkListPeer } from './component/list'
import { SearchOptions, SearchAttribute, ArkSearchPeer } from './component/search'
import { TextAreaOptions, TextAreaAttribute, ArkTextAreaPeer } from './component/textArea'
import { TextInputOptions, TextInputAttribute, ArkTextInputPeer } from './component/textInput'
import { ArkTextNode } from './handwritten/modifiers/ArkTextNode'
import { TextOptions, TextAttribute, ArkTextPeer } from './component/text'
import { Deserializer } from "./component/peers/Deserializer";
import { ComponentContent } from './ComponentContent';
import { DrawContext } from './Graphics';
import { JSBuilderNode } from "./BuilderNode"
import { BusinessError } from '#external';
import { Resource } from 'global.resource';

export interface CrossLanguageOptions {
    attributeSetting?: boolean;
}
export interface LayoutConstraint {
    maxSize: Size;
    minSize: Size;
    percentReference: Size;
}

export class ArkFrameNodePeer extends ArkCommonMethodPeer {
    constructor(peerPtr: KPointer, id: int32, name: string = "", flags: int32 = 0) {
        super(peerPtr, id, name, flags)
    }
}

export enum ExpandMode {
    NOT_EXPAND = 0,
    EXPAND = 1,
    LAZY_EXPAND = 2,
}
export class FrameNodeFinalizationRegisterProxy {
    constructor() { }
    public static instance_: FrameNodeFinalizationRegisterProxy = new FrameNodeFinalizationRegisterProxy();
    public static ElementIdToOwningFrameNode_ = new Map<number, FrameNode>();

}

export class FrameNodeInternal {
    public static fromPtr(ptr: KPointer): FrameNode {
        const obj: FrameNode = new FrameNode(undefined)
        obj.peer = new Finalizable(ptr, FrameNode.getFinalizer())
        return obj
    }
}

// @ts-ignore
function GetExactRetValue(data: KSerializerBuffer | KUint8ArrayPtr): Array<byte> {
    // @ts-ignore
    const retval = data as FixedArray<byte>;
    // @ts-ignore
    let exactRetValue: byte[] = new Array<byte>();
    for (let i = 0; i < retval.length; i++) {
        // @ts-ignore
        exactRetValue.push(new Byte(retval[i]));
    }
    return exactRetValue;
}

export class FrameNode implements MaterializedBase {
    peer?: Finalizable | undefined = undefined
    uiContext: UIContextImpl | undefined = undefined
    renderNode_: RenderNode | undefined = undefined
    instanceId_?: number;
    // the name of "nodePtr_" is used in ace_engine/interfaces/native/node/native_node_ani.cpp.
    nodePtr_: KPointer | undefined;
    public _nodeId: number = -1;
    protected _commonAttribute: CommonAttribute | undefined = undefined;
    protected _gestureEvent: UIGestureEvent | undefined = undefined;

    getType(): string {
        return 'CustomFrameNode';
    }

    checkValid(node: FrameNode): boolean {
        return true;
    }
    public getPeer(): Finalizable | undefined {
        return this.peer
    }
    get commonEvent(): UICommonEvent {
        return this.getCommonEvent()
    }
    get gestureEvent(): UIGestureEvent {
        if (this._gestureEvent === undefined) {
            this._gestureEvent = new UIGestureEvent();
            const retval  = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(this))
            let peer = new ArkFrameNodePeer(retval, this._nodeId as int32, "FrameNode", 0);
            this._gestureEvent!.setPeer(peer);
        }
        return this._gestureEvent as UIGestureEvent;
    }
    static ctor_framenode(): KPointer {
        const thisSerializer: Serializer = Serializer.hold()
        const retval = ArkUIGeneratedNativeModule._FrameNode_ctor(thisSerializer.asBuffer(), thisSerializer.length())
        thisSerializer.release()
        return retval
    }
    constructor(uiContext?: UIContext, type?: string, ptr?: KPointer) {
        if ((uiContext) !== (undefined)) {
            this.uiContext = uiContext as UIContextImpl;
            this.instanceId_ = this.uiContext!.instanceId_;
            if (type === 'ProxyFrameNode') {
                if (ptr) {
                    this.peer = new Finalizable(ptr, FrameNode.getFinalizer());
                    this.nodePtr_ = this.peer?.ptr
                }
                return;
            }
            if (this.getType() === 'BuilderRootFrameNode') {
                this.renderNode_ = new RenderNode(this.getType());
                this.renderNode_!.setFrameNode(new WeakRef<FrameNode>(this));
                return;
            }
            const instanceId = this.instanceId_!.toInt();
            ArkUIAniModule._Common_Sync_InstanceId(instanceId);
            if (this.getType() === undefined || this.getType() === "CustomFrameNode") {
                this.renderNode_ = new RenderNode('CustomFrameNode')
                const ctorPtr: KPointer = FrameNode.ctor_framenode()
                this.peer = new Finalizable(ctorPtr, FrameNode.getFinalizer())
                this.nodePtr_ = this.peer?.ptr
            } else {
                const retval = ArkUIGeneratedNativeModule._FrameNode_createTypedFrameNode(type as string);
                this.peer = new Finalizable(retval, FrameNode.getFinalizer());
                this.nodePtr_ = this.peer?.ptr
            }
            this.renderNode_?.setFrameNode(new WeakRef<FrameNode>(this))

            this._nodeId = this.getIdByFrameNode_serialize(this);
            ArkUIAniModule._Common_Restore_InstanceId();
            FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.set(this._nodeId, this);
            this.onDraw_serialize(this.onDraw);
            ArkUIAniModule._SetCustomCallback(this!.peer!.ptr, this.onMeasureInner, this.onLayoutInner);
        }
    }
    static getFinalizer(): KPointer {
        return ArkUIGeneratedNativeModule._FrameNode_getFinalizer();
    }
    public isModifiable(): boolean {
        return this.isModifiable_serialize();
    }
    private onDraw_serialize(drawCallBack: (context: DrawContext) => void): void {
        const thisSerializer: Serializer = Serializer.hold();
        const event_value = drawCallBack;
        thisSerializer.holdAndWriteCallback(event_value);
        ArkUIGeneratedNativeModule._FrameNode_onDraw(this.peer!.ptr, thisSerializer.asBuffer(), thisSerializer.length());
        thisSerializer.release();
    }
    public appendChild(node: FrameNode): void {
        if (node.getType() === 'ProxyFrameNode' || !this.checkValid(node)) {
            const error = Error('The FrameNode is not modifiable.');
            throw new BusinessError(100021, error);
        }
        const node_casted = node as (FrameNode)
        this.appendChild_serialize(node_casted)
        return
    }
    public addComponentContent<T = undefined>(content: ComponentContent<T>) {
        if (content === undefined || content === null || content.getFrameNode() == undefined ||
            content.getFrameNode() == null || content.getNodePtr() == undefined) {
            return;
        }
        if (!this.isModifiable()) {
            throw Error("The FrameNode is not modifiable.");
        }
        const instanceId = this.instanceId_!.toInt();
        ArkUIAniModule._Common_Sync_InstanceId(instanceId);
        this.appendChild(content.getFrameNode()!);
        ArkUIAniModule._Common_Restore_InstanceId();
        content.setAttachedParent(new WeakRef<FrameNode>(this));
    }
    public removeComponentContent<T = undefined>(content: ComponentContent<T>) {
        if (content === undefined || content === null || content.getFrameNode() == undefined ||
            content.getFrameNode() == null || content.getNodePtr() == undefined) {
            return;
        }
        const instanceId = this.instanceId_!.toInt();
        ArkUIAniModule._Common_Sync_InstanceId(instanceId);
        this.removeChild(content.getFrameNode()!);
        ArkUIAniModule._Common_Restore_InstanceId();
        content.setAttachedParent(undefined);
    }
    public insertChildAfter(child: FrameNode, sibling: FrameNode | null): void {
        if (child === undefined || child === null) {
            return;
        }
        if (child.getType() === 'ProxyFrameNode' || !this.checkValid(child)) {
            const error = Error('The FrameNode is not modifiable.');
            throw new BusinessError(100021, error);
        }
        const child_casted = child as (FrameNode);
        if (sibling === null || sibling === undefined) {
            this.insertChildAfter_serialize(child_casted, null);
        } else {
            const sibling_casted = sibling as (FrameNode);
            this.insertChildAfter_serialize(child_casted, sibling_casted);
        }
        return
    }
    public removeChild(node: FrameNode): void {
        const node_casted = node as (FrameNode)
        this.removeChild_serialize(node_casted)
        return
    }
    public clearChildren(): void {
        this.clearChildren_serialize()
        return
    }
    convertToFrameNode(node: FrameNode, nodeId: number = -1): FrameNode {
        if (nodeId === -1) {
            const instanceId = this.instanceId_!.toInt();
            ArkUIAniModule._Common_Sync_InstanceId(instanceId);
            nodeId = this.getIdByFrameNode_serialize(node);
            ArkUIAniModule._Common_Restore_InstanceId();
        }
        if (nodeId !== -1 && !node.isModifiable()) {
            node._nodeId = nodeId;
            node.uiContext = this.uiContext;
            node.instanceId_ = this.instanceId_;
            FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.set(node._nodeId, node);
        }
        return node;
    }
    public getChild(index: number, expandMode?: ExpandMode | undefined): FrameNode | null {
        const index_casted = index as (number);
        let expand_casted: number = 1;
        if (expandMode !== undefined && expandMode !== null) {
            expand_casted = expandMode;
        }
        const child = this.getChild_serialize(index_casted, expand_casted);
        if (child) {
            const nodeId = this.getIdByFrameNode_serialize(child);
            if (nodeId === -1) {
                return null;
            }
            if (FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.has(nodeId)) {
                const node = FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.get(nodeId);
                return node === undefined ? null : node;
            }
            return this.convertToFrameNode(child, nodeId);
        }
        return null;
    }
    public getFirstChild(): FrameNode | null {
        const child = this.getFirstChild_serialize();
        if (child) {
            const nodeId = this.getIdByFrameNode_serialize(child);
            if (nodeId === -1) {
                return null;
            }
            if (FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.has(nodeId)) {
                const node = FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.get(nodeId);
                return node === undefined ? null : node;
            }
            return this.convertToFrameNode(child, nodeId);
        }
        return null;
    }
    public getNextSibling(): FrameNode | null {
        const child = this.getNextSibling_serialize();
        if (child) {
            const nodeId = this.getIdByFrameNode_serialize(child);
            if (nodeId === -1) {
                return null;
            }
            if (FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.has(nodeId)) {
                const node = FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.get(nodeId);
                return node === undefined ? null : node;
            }
            return this.convertToFrameNode(child, nodeId);
        }
        return null;
    }
    public getPreviousSibling(): FrameNode | null {
        const child = this.getPreviousSibling_serialize();
        if (child) {
            const nodeId = this.getIdByFrameNode_serialize(child);
            if (nodeId === -1) {
                return null;
            }
            if (FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.has(nodeId)) {
                const node = FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.get(nodeId);
                return node === undefined ? null : node;
            }
            return this.convertToFrameNode(child, nodeId);
        }
        return null;
    }
    public getParent(): FrameNode | null {
        const parent = this.getParent_serialize();
        if (parent) {
            const nodeId = this.getIdByFrameNode_serialize(parent);
            if (nodeId === -1) {
                return null;
            }
            if (FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.has(nodeId)) {
                const node = FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.get(nodeId);
                return node === undefined ? null : node;
            }
            return this.convertToFrameNode(parent, nodeId);
        }
        return null;
    }
    public getChildrenCount(): number {
        return this.getChildrenCount_serialize();
    }
    public dispose(): void {
        this.dispose_serialize();
        this.nodePtr_ = undefined;
        return
    }
    public getOpacity(): number {
        return this.getOpacity_serialize();
    }
    public getPositionToWindowWithTransform(): Position {
        return this.getPositionToWindowWithTransform_serialize();
    }
    public static getFrameNodeByKey(name: string): FrameNode {
        const name_casted = name as (string);
        return FrameNode.getFrameNodeByKey_serialize(name_casted);
    }
    public getPositionToParent(): Position {
        return this.getPositionToParent_serialize()
    }
    public getPositionToScreen(): Position {
        return this.getPositionToScreen_serialize()
    }
    public getPositionToWindow(): Position {
        return this.getPositionToWindow_serialize()
    }
    public getPositionToParentWithTransform(): Position {
        return this.getPositionToParentWithTransform_serialize()
    }
    public getPositionToScreenWithTransform(): Position {
        return this.getPositionToScreenWithTransform_serialize()
    }
    public getMeasuredSize(): Size {
        return this.getMeasuredSize_serialize()
    }
    public getLayoutPosition(): Position {
        return this.getLayoutPosition_serialize()
    }
    public getUserConfigBorderWidth(): Edges<LengthMetrics> {
        return this.getUserConfigBorderWidth_serialize()
    }
    public getUserConfigPadding(): Edges<LengthMetrics> {
        return this.getUserConfigPadding_serialize()
    }
    public getUserConfigMargin(): Edges<LengthMetrics> {
        return this.getUserConfigMargin_serialize()
    }
    public getUserConfigSize(): SizeT<LengthMetrics> {
        return this.getUserConfigSize_serialize()
    }
     public getId(): string {
        return ArkUIGeneratedNativeModule._FrameNode_getId(this.peer!.ptr);
    }
    public getUniqueId(): number {
        return ArkUIGeneratedNativeModule._FrameNode_getUniqueId(this.peer!.ptr);
    }
    public isVisible(): boolean {
        return ArkUIGeneratedNativeModule._FrameNode_isVisible(this.peer!.ptr);
    }
    public isClipToFrame(): boolean {
        return ArkUIGeneratedNativeModule._FrameNode_isClipToFrame(this.peer!.ptr);
    }
    public isAttached(): boolean {
        return ArkUIGeneratedNativeModule._FrameNode_isAttached(this.peer!.ptr);
    }
    public getInspectorInfo(): Object {
        return ArkUIGeneratedNativeModule._FrameNode_getInspectorInfo(this.peer!.ptr);
    }
    public onDraw(context: DrawContext): void {
    }
    public onMeasureInner(width1: number, height1: number, width2: number, height2: number,
        width3: number, height3: number): void {
        const maxSize: Size = { width: width1, height: height1 };
        const minSize: Size = { width: width2, height: height2 };
        const percentReference: Size = { width: width3, height: height3 };
        const constraint: LayoutConstraint = { maxSize: maxSize, minSize: minSize, percentReference: percentReference };
        this.onMeasure(constraint);
    }
    public onMeasure(constraint: LayoutConstraint): void {
        ArkUIAniModule._OnMeasure_InnerMeasure(this!.peer!.ptr);
    }
    public onLayoutInner(x: number, y: number): void {
        const position: Position = { x: x, y: y };
        this.onLayout(position);
    }
    public onLayout(position: Position): void {
        ArkUIAniModule._OnLayout_InnerLayout(this!.peer!.ptr);
    }
    public setMeasuredSize(size: Size): void {
        const width = Math.max(size.width, 0);
        const height = Math.max(size.height, 0);
        const size_casted = { width: width, height: height } as (Size);
        this.setMeasuredSize_serialize(size_casted);
        return;
    }
    public setLayoutPosition(position: Position): void {
        const position_casted = position as (Position);
        this.setLayoutPosition_serialize(position_casted);
        return;
    }
    public measure(constraint: LayoutConstraint): void {
        const constraint_casted = constraint as (LayoutConstraint);
        const instanceId = this.instanceId_!.toInt();
        ArkUIAniModule._Common_Sync_InstanceId(instanceId);
        this.measure_serialize(constraint_casted);
        ArkUIAniModule._Common_Restore_InstanceId();
        return;
    }
    public layout(position: Position): void {
        const position_casted = position as (Position);
        const instanceId = this.instanceId_!.toInt();
        ArkUIAniModule._Common_Sync_InstanceId(instanceId);
        this.layout_serialize(position_casted);
        ArkUIAniModule._Common_Restore_InstanceId();
        return;
    }
    private setMeasuredSize_serialize(size: Size): void {
        const thisSerializer : Serializer = Serializer.hold();
        thisSerializer.writeSize(size);
        ArkUIGeneratedNativeModule._FrameNode_setMeasuredSize(this.peer!.ptr, thisSerializer.asBuffer(), thisSerializer.length());
        thisSerializer.release();
    }
    private setLayoutPosition_serialize(position: Position): void {
        const thisSerializer : Serializer = Serializer.hold();
        thisSerializer.writeGraphicsPosition(position);
        ArkUIGeneratedNativeModule._FrameNode_setLayoutPosition(this.peer!.ptr, thisSerializer.asBuffer(), thisSerializer.length());
        thisSerializer.release();
    }
    private measure_serialize(constraint: LayoutConstraint): void {
        const thisSerializer : Serializer = Serializer.hold();
        thisSerializer.writeLayoutConstraint(constraint);
        ArkUIGeneratedNativeModule._FrameNode_measure(this.peer!.ptr, thisSerializer.asBuffer(), thisSerializer.length());
        thisSerializer.release();
    }
    private layout_serialize(position: Position): void {
        const thisSerializer : Serializer = Serializer.hold();
        thisSerializer.writeGraphicsPosition(position);
        ArkUIGeneratedNativeModule._FrameNode_layout(this.peer!.ptr, thisSerializer.asBuffer(), thisSerializer.length());
        thisSerializer.release();
    }
    public setNeedsLayout(): void {
        this.setNeedsLayout_serialize();
        return;
    }
    private setNeedsLayout_serialize(): void {
        ArkUIGeneratedNativeModule._FrameNode_setNeedsLayout(this.peer!.ptr);
    }
    public invalidate(): void {
        ArkUIGeneratedNativeModule._FrameNode_invalidate(this.peer!.ptr);
    };
    public disposeTree(): void {
        ArkUIGeneratedNativeModule._FrameNode_disposeTree(this.peer!.ptr);
    };
    public setCrossLanguageOptions(options: CrossLanguageOptions): void {
        let setting: boolean = options.attributeSetting ? options.attributeSetting! : false;
        ArkUIGeneratedNativeModule._FrameNode_setCrossLanguageOptions(this.peer!.ptr, setting);
    }
    public getCrossLanguageOptions(): CrossLanguageOptions {
        let option: boolean = ArkUIGeneratedNativeModule._FrameNode_getCrossLanguageOptions(this.peer!.ptr);
        const crossLanguageOptions: CrossLanguageOptions = {
            attributeSetting: option,
        };
        return crossLanguageOptions;
    }
    private getCommonEvent(): UICommonEvent {
        return this.getCommonEvent_serialize()
    }
    private getCommonEvent_serialize(): UICommonEvent {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getCommonEvent(this.peer!.ptr)
        const obj : UICommonEvent = UICommonEventInternal.fromPtr(retval)
        return obj
    }
    private isModifiable_serialize(): boolean {
        const retval = ArkUIGeneratedNativeModule._FrameNode_isModifiable(this.peer!.ptr)
        return retval
    }
    private appendChild_serialize(node: FrameNode): void {
        ArkUIGeneratedNativeModule._FrameNode_appendChild(this.peer!.ptr, toPeerPtr(node))
    }
    private insertChildAfter_serialize(child: FrameNode, sibling: FrameNode | null): void {
        if (sibling === null || sibling === undefined) {
            ArkUIGeneratedNativeModule._FrameNode_insertChildAfter(this.peer!.ptr, toPeerPtr(child), nullptr);
        } else {
            ArkUIGeneratedNativeModule._FrameNode_insertChildAfter(this.peer!.ptr, toPeerPtr(child),
                toPeerPtr(sibling!));
        }
    }
    private removeChild_serialize(node: FrameNode): void {
        ArkUIGeneratedNativeModule._FrameNode_removeChild(this.peer!.ptr, toPeerPtr(node));
    }
    private clearChildren_serialize(): void {
        ArkUIGeneratedNativeModule._FrameNode_clearChildren(this.peer!.ptr);
    }
    private getChild_serialize(index: number, expandMode: number): FrameNode | null {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getChild(this.peer!.ptr, index, expandMode);
        if (retval) {
            const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
            return obj;
        } else {
            return null;
        }
    }
    private getFirstChild_serialize(): FrameNode | null {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFirstChild(this.peer!.ptr);
        if (retval) {
            const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
            return obj;
        } else {
            return null;
        }
    }
    private getNextSibling_serialize(): FrameNode | null {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getNextSibling(this.peer!.ptr);
        if (retval) {
            const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
            return obj;
        } else {
            return null;
        }
    }
    private getPreviousSibling_serialize(): FrameNode | null {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getPreviousSibling(this.peer!.ptr);
        if (retval) {
            const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
            return obj;
        } else {
            return null;
        }
    }
    private getParent_serialize(): FrameNode | null {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getParent(this.peer!.ptr);
        if (retval) {
            const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
            return obj;
        } else {
            return null;
        }
    }
    private getChildrenCount_serialize(): int32 {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getChildrenCount(this.peer!.ptr);
        return retval;
    }
    private dispose_serialize(): void {
        ArkUIGeneratedNativeModule._FrameNode_dispose(this.peer!.ptr);
    }
    private getOpacity_serialize(): number {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getOpacity(this.peer!.ptr);
        return retval;
    }
    private getPositionToWindowWithTransform_serialize(): Position {
        // @ts-ignore
        const retval = ArkUIGeneratedNativeModule._FrameNode_getPositionToWindowWithTransform(this.peer!.ptr) as FixedArray<byte>;
        // @ts-ignore
        let exactRetValue: byte[] = new Array<byte>;
        for (let i = 0; i < retval.length; i++) {
            // @ts-ignore
            exactRetValue.push(new Byte(retval[i]));
        }
        let retvalDeserializer: Deserializer = new Deserializer(exactRetValue, exactRetValue.length!.toInt());
        const returnResult: Position = retvalDeserializer.readGraphicsPosition();
        return returnResult;
    }
    private static getFrameNodeByKey_serialize(name: string): FrameNode {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodeByKey(name)
        const obj: FrameNode = FrameNodeInternal.fromPtr(retval)
        return obj
    }
    public getIdByFrameNode(node: FrameNode): number {
        const node_casted = node as (FrameNode);
        return this.getIdByFrameNode_serialize(node_casted);
    }
    private getIdByFrameNode_serialize(node: FrameNode): number {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getIdByFrameNode(this.peer!.ptr, toPeerPtr(node));
        return retval;
    }
    public moveTo(targetParent: FrameNode, index?: number): void {
        if (targetParent === undefined || targetParent === null) {
            return;
        }
        let index_casted: number = -1;
        if (index !== undefined && index !== null) {
            index_casted = index;
        }
        const oldParent = this.getParent();
        if (oldParent && !oldParent.isModifiable() || !targetParent.isModifiable() || !targetParent.checkValid(this)) {
            const error = Error('The FrameNode is not modifiable.');
            throw new BusinessError(100021, error);
        }
        const targetParent_casted = targetParent as (FrameNode);
        this.moveTo_serialize(targetParent_casted, index_casted);
        return;
    }
    private moveTo_serialize(targetParent: FrameNode, index: number): void {
        ArkUIGeneratedNativeModule._FrameNode_moveTo(this.peer!.ptr, toPeerPtr(targetParent), index);
    }

    public getFirstChildIndexWithoutExpand(): number {
        return this.getFirstChildIndexWithoutExpand_serialize();
    }
    public getLastChildIndexWithoutExpand(): number {
        return this.getLastChildIndexWithoutExpand_serialize();
    }

    private getFirstChildIndexWithoutExpand_serialize(): number {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFirstChildIndexWithoutExpand(this.peer!.ptr);
        return retval;
    }
    private getLastChildIndexWithoutExpand_serialize(): number {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getLastChildIndexWithoutExpand(this.peer!.ptr);
        return retval;
    }
    public static getAttachedFrameNodeById(id: string): FrameNode {
        const id_casted = id;
        return FrameNode.getAttachedFrameNodeById_serialize(id_casted);
    }
    public static getFrameNodeById(id: number): FrameNode {
        const id_casted = id;
        return FrameNode.getFrameNodeById_serialize(id_casted);
    }
    public static getFrameNodeByUniqueId(id: number): FrameNode {
        const id_casted = id;
        return FrameNode.getFrameNodeByUniqueId_serialize(id_casted);
    }
    private static getAttachedFrameNodeById_serialize(id: string): FrameNode {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getAttachedFrameNodeById(id);
        const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
        return obj;
    }
    private static getFrameNodeById_serialize(id: number): FrameNode {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodeById(id);
        const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
        return obj;
    }
    private static getFrameNodeByUniqueId_serialize(id: number): FrameNode {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodeByUniqueId(id);
        const obj: FrameNode = FrameNodeInternal.fromPtr(retval);
        return obj;
    }
    private getPositionToParent_serialize(): Position {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getPositionToParent(this.peer!.ptr);
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Position = retvalDeserializer.readGraphicsPosition();
        return returnResult;
    }
    private getPositionToScreen_serialize(): Position {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getPositionToScreen(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32)
        const returnResult : Position = retvalDeserializer.readGraphicsPosition()
        return returnResult
    }
    private getPositionToWindow_serialize(): Position {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getPositionToWindow(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Position = retvalDeserializer.readGraphicsPosition()
        return returnResult
    }
    private getPositionToParentWithTransform_serialize(): Position {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getPositionToParentWithTransform(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Position = retvalDeserializer.readGraphicsPosition()
        return returnResult
    }
    private getPositionToScreenWithTransform_serialize(): Position {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getPositionToScreenWithTransform(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Position = retvalDeserializer.readGraphicsPosition()
        return returnResult
    }
    private getPositionToWindowWithTransform1_serialize(): Position {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getPositionToWindowWithTransform1(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Position = retvalDeserializer.readGraphicsPosition()
        return returnResult
    }
    private getMeasuredSize_serialize(): Size {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getMeasuredSize(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Size = retvalDeserializer.readSize()
        return returnResult
    }
    private getLayoutPosition_serialize(): Position {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getLayoutPosition(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Position = retvalDeserializer.readGraphicsPosition()
        return returnResult
    }
    private getUserConfigBorderWidth_serialize(): Edges<LengthMetrics> {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getUserConfigBorderWidth(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Edges<LengthMetrics> = retvalDeserializer.readEdgesLengthMetrics()
        return returnResult
    }
    private getUserConfigPadding_serialize(): Edges<LengthMetrics> {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getUserConfigPadding(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Edges<LengthMetrics> = retvalDeserializer.readEdgesLengthMetrics()
        return returnResult
    }
    private getUserConfigMargin_serialize(): Edges<LengthMetrics> {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getUserConfigMargin(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : Edges<LengthMetrics> = retvalDeserializer.readEdgesLengthMetrics()
        return returnResult
    }
    private getUserConfigSize_serialize(): SizeT<LengthMetrics> {
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getUserConfigSize(this.peer!.ptr)
        const exactRetValue = GetExactRetValue(retval);
        let retvalDeserializer : Deserializer = new Deserializer(exactRetValue, exactRetValue.length as int32);
        const returnResult : SizeT<LengthMetrics> = retvalDeserializer.readSizeLengthMetrics()
        return returnResult
    }
    public reuse(): void {
        this.reuse_serialize()
        return
    }
    public recycle(): void {
        this.recycle_serialize()
        return
    }
    private reuse_serialize(): void {
        ArkUIGeneratedNativeModule._FrameNode_reuse(this.peer!.ptr);
    }
    private recycle_serialize(): void {
        ArkUIGeneratedNativeModule._FrameNode_recycle(this.peer!.ptr);
    }
    public getRenderNode(): RenderNode | null {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getRenderNode(this.peer!.ptr)
        return RenderNodeInternal.fromPtr(retval)
    }
    public static getFrameNodePtr(node: FrameNode): KPointer {
        const node_casted = node as (FrameNode)
        return FrameNode.getFrameNodePtr_serialize(node_casted)
    }
    private static getFrameNodePtr_serialize(node: FrameNode): KPointer {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(node))
        return retval
    }
    get commonAttribute(): CommonMethod {
        if (this._commonAttribute === undefined) {
            let baseNode = new ArkBaseNode();
            const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(this))
            let peer = new ArkFrameNodePeer(retval, this._nodeId!.toInt(), "FrameNode", 0);
            baseNode.setPeer(peer);
            this._commonAttribute = baseNode as CommonAttribute;
        }
        return this._commonAttribute!;
    }
    public static createTypedFrameNode(type: string): FrameNode {
        const type_casted = type
        return FrameNode.createTypedFrameNode_serialize(type_casted)
    }
    private static createTypedFrameNode_serialize(type: string): FrameNode {
        const retval = ArkUIGeneratedNativeModule._FrameNode_createTypedFrameNode(type)
        const obj: FrameNode = FrameNodeInternal.fromPtr(retval)
        return obj
    }
    public getNodeType(): string {
        return this.getNodeType_serialize()
    }
    private getNodeType_serialize(): string {
        const retval = ArkUIGeneratedNativeModule._FrameNode_getNodeType(this.peer!.ptr)
        return retval
    }
}
class ImmutableFrameNode extends FrameNode {
    constructor(uiContext: UIContext, type: string, ptr?: KPointer) {
        super(uiContext, type, ptr);
    }
    isModifiable(): boolean {
        return false;
    }
    invalidate(): void {
        return;
    }
    appendChild(node: FrameNode): void {
        const error = Error('The FrameNode is not modifiable.');
        throw new BusinessError(100021, error);
    }
    insertChildAfter(child: FrameNode, sibling: FrameNode | null): void {
        const error = Error('The FrameNode is not modifiable.');
        throw new BusinessError(100021, error);
    }
    removeChild(node: FrameNode): void {
        const error = Error('The FrameNode is not modifiable.');
        throw new BusinessError(100021, error);
    }
    clearChildren(): void {
        const error = Error('The FrameNode is not modifiable.');
        throw new BusinessError(100021, error);
    }
}

export class ProxyFrameNode extends ImmutableFrameNode {
    constructor(uiContext: UIContext, type: string = 'ProxyFrameNode', ptr?: KPointer) {
        super(uiContext, type, ptr);
    }
    getType(): string {
        return 'ProxyFrameNode';
    }
    moveTo(targetParent: FrameNode, index?: number): void {
        const error = Error('The FrameNode is not modifiable.');
        throw new BusinessError(100021, error);
    }
}
export class BuilderRootFrameNode<T> extends ImmutableFrameNode {
    private __BuilderNodeOpt: JSBuilderNode<T> | undefined = undefined;
    constructor(uiContext: UIContext, type: string = 'BuilderRootFrameNode', ptr?: KPointer) {
        super(uiContext, type, ptr);
    }
    getType(): string {
        return 'BuilderRootFrameNode';
    }
    setJsBuilderNode(weak?: JSBuilderNode<T>) {
        this.__BuilderNodeOpt = weak;
    }
    public disposeNode(): void {
        super.dispose();
        this.__BuilderNodeOpt = undefined;
        this._nodeId = -1;
    }
    public dispose(): void {
        super.dispose();
        this.__BuilderNodeOpt?.disposeNode();
        this.__BuilderNodeOpt = undefined;
        this._nodeId = -1;
    }
}
export interface ResultFrameNode {
    nodeId: int32;
    frameNode: FrameNode;
}
export class FrameNodeUtils {
    static searchNodeInRegisterProxy(ptr: KPointer): FrameNode | null {
        const nodeId = ArkUIGeneratedNativeModule._FrameNode_getIdByFrameNode(ptr, ptr);
        if (nodeId === -1) {
            return null;
        }
        if (FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.has(nodeId)) {
            const node = FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.get(nodeId);
            return node === undefined ? null : node;
        }
        return null;
    }
    static createFrameNode(uiContext: UIContext, ptr: KPointer): FrameNode | null {
        const nodeId = ArkUIGeneratedNativeModule._FrameNode_getIdByFrameNode(ptr, ptr);
        if (nodeId !== -1 && !ArkUIGeneratedNativeModule._FrameNode_isModifiable(ptr)) {
            let frameNode = new ProxyFrameNode(uiContext, "ProxyFrameNode", ptr);
            frameNode._nodeId = nodeId;
            frameNode!.peer!.ptr = ptr;
            FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.set(nodeId, frameNode);
            return frameNode;
        }
        return null;
    }
    static createBuilderRootFrameNode<T = undefined>(uiContext: UIContext, ptr: KPointer): BuilderRootFrameNode<T> | null {
        const nodeId = ArkUIGeneratedNativeModule._FrameNode_getIdByFrameNode(ptr, ptr);
        if (nodeId !== -1 && !ArkUIGeneratedNativeModule._FrameNode_isModifiable(ptr)) {
            let frameNode = new BuilderRootFrameNode<T>(uiContext, "BuilderRootFrameNode", ptr);
            frameNode._nodeId = nodeId;
            frameNode.peer = new Finalizable(ptr, FrameNode.getFinalizer());
            frameNode.nodePtr_ = frameNode.peer?.ptr;
            FrameNodeFinalizationRegisterProxy.ElementIdToOwningFrameNode_.set(nodeId, frameNode);
            return frameNode;
        }
        return null;
    }
}

abstract class TypedFrameNode<T extends Object> extends FrameNode {
    attribute_: T | undefined = undefined;
    attrCreator_: (node: FrameNode, type: ModifierType) => T
    type_: string = "";

    constructor(uiContext: UIContext, type: string, attrCreator: (node: FrameNode, type: ModifierType) => T) {
        super(uiContext, type, nullptr);
        this.attrCreator_ = attrCreator;
        this.type_ = type;
    }
    getType(): string {
        return this.type_;
    }
    checkValid(node?: FrameNode): boolean {
        if (this.attribute_ === undefined) {
            this.attribute_ = this.attrCreator_(this, ModifierType.FRAME_NODE);
        }
        const a = this.attribute_ as Object as ArkBaseNode;
        const allowCount: number = a.allowChildCount();
        const childrenCount = this.getChildrenCount();
        if (allowCount != -1) {
            if (childrenCount >= allowCount) {
                return false;
            }
        }

        if (node !== undefined) {
            const childType = node!.getNodeType();
            const allowTypes = a.allowChildTypes();
            if (allowTypes === undefined) {
                return true;
            }
            let isValid = false;
            allowTypes!.forEach((nodeType: string) => {
                if (nodeType === childType) {
                    isValid = true;
                }
            });
            return isValid;
        }
        return true;
    }
    get attribute(): T {
        if (this.attribute_ === undefined) {
            this.attribute_ = this.attrCreator_(this, ModifierType.FRAME_NODE);
        }
        return this.attribute_!;
    }
}
export namespace typeNode {
    class ListFrameNode extends TypedFrameNode<ArkListNode> {
        constructor(uiContext: UIContext, type: string, attrCreator: (node: FrameNode, type: ModifierType) => ArkListNode) {
            super(uiContext, type, attrCreator);
        }
        initialize(options: ListOptions): ListAttribute {
            let arkListNode = this.attribute as ArkListNode;
            return arkListNode!.initialize(options);
        }
    }

    class SearchFrameNode extends TypedFrameNode<ArkSearchNode> {
        constructor(uiContext: UIContext, type: string, attrCreator: (node: FrameNode, type: ModifierType) => ArkSearchNode) {
            super(uiContext, type, attrCreator);
        }
        initialize(options: SearchOptions): SearchAttribute {
            let arkSearchNode = this.attribute as ArkSearchNode;
            return arkSearchNode!.initialize(options);
        }
    }

    class TextAreaFrameNode extends TypedFrameNode<ArkTextAreaNode> {
        constructor(uiContext: UIContext, type: string, attrCreator: (node: FrameNode, type: ModifierType) => ArkTextAreaNode) {
            super(uiContext, type, attrCreator);
        }
        initialize(options: TextAreaOptions): TextAreaAttribute {
            let arkTextAreaNode = this.attribute as ArkTextAreaNode;
            return arkTextAreaNode!.initialize(options);
        }
    }

    class TextInputFrameNode extends TypedFrameNode<ArkTextInputNode> {
        constructor(uiContext: UIContext, type: string, attrCreator: (node: FrameNode, type: ModifierType) => ArkTextInputNode) {
            super(uiContext, type, attrCreator);
        }
        initialize(options: TextInputOptions): TextInputAttribute {
            let arkTextInputNode = this.attribute as ArkTextInputNode;
            return arkTextInputNode!.initialize(options);
        }
    }

    class TextFrameNode extends TypedFrameNode<ArkTextNode> {
        constructor(uiContext: UIContext, type: string, attrCreator: (node: FrameNode, type: ModifierType) => ArkTextNode) {
            super(uiContext, type, attrCreator);
        }
        initialize(content?: string | Resource, value?: TextOptions): TextAttribute {
            let arkTextNode = this.attribute as ArkTextNode;
            return arkTextNode!.initialize(content, value);
        }
    }

    overload createNode { createListNode, createSearchNode, createTextAreaNode, createTextInputNode, createTextNode }

    // @ts-ignore
    function createListNode(context: UIContext, type: string): ListFrameNode {
        return new ListFrameNode(context, 'List', (node: FrameNode, type: ModifierType): ArkListNode => {
            let arknode = new ArkListNode();
            const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(node));
            const peer = new ArkListPeer(retval, node._nodeId!.toInt(), "List", 0);
            arknode.setPeer(peer);
            return arknode;
        });
    }

    // @ts-ignore
    function createSearchNode(context: UIContext, type: string): SearchFrameNode {
        return new SearchFrameNode(context, 'Search', (node: FrameNode, type: ModifierType): ArkSearchNode => {
            let arknode = new ArkSearchNode();
            const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(node));
            const peer = new ArkSearchPeer(retval, node._nodeId as int32, "Search", 0);
            arknode.setPeer(peer);
            return arknode;
        });
    }

    // @ts-ignore
    function createTextAreaNode(context: UIContext, type: string): TextAreaFrameNode {
        return new TextAreaFrameNode(context, 'TextArea', (node: FrameNode, type: ModifierType): ArkTextAreaNode => {
            let arknode = new ArkTextAreaNode();
            const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(node));
            const peer = new ArkTextAreaPeer(retval, node._nodeId as int32, "TextArea", 0);
            arknode.setPeer(peer);
            return arknode;
        });
    }

    // @ts-ignore
    function createTextInputNode(context: UIContext, type: string): TextInputFrameNode {
        return new TextInputFrameNode(context, 'TextInput', (node: FrameNode, type: ModifierType): ArkTextInputNode => {
            let arknode = new ArkTextInputNode();
            const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(node));
            const peer = new ArkTextInputPeer(retval, node._nodeId as int32, "TextInput", 0);
            arknode.setPeer(peer);
            return arknode;
        });
    }

   // @ts-ignore
   function createTextNode(context: UIContext, type: 'Text'): TextFrameNode {
       return new TextFrameNode(context, 'Text', (node: FrameNode, type: ModifierType): ArkTextNode => {
           let arknode = new ArkTextNode();
           const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodePtr(toPeerPtr(node));
           const peer = new ArkTextPeer(retval, node._nodeId as int32, "Text", 0);
           arknode.setPeer(peer);
           return arknode;
       });
   }
}
