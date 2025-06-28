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

import { FrameNode, FrameNodeInternal, FrameNodeUtils } from "arkui/FrameNode"
import { ArkUIGeneratedNativeModule } from "#components"
import { int32, int64 } from "@koalaui/common"
import { nullptr, KPointer, KSerializerBuffer, toPeerPtr } from "@koalaui/interop"
import { _animateTo } from "arkui/handwritten/ArkAnimation"
import { AnimateParam } from 'arkui/component'
import { AnimatorResult , AnimatorOptions, Animator, SimpleAnimatorOptions } from "@ohos/animator"
import { UIContext, MeasureUtils, Font, TextMenuController, FocusController, ContextMenuController, ComponentUtils,
    FrameCallback, UIInspector, UIObserver, OverlayManager, PromptAction, AtomicServiceBar, Router, CursorController,
    MediaQuery, ComponentSnapshot, OverlayManagerOptions, DragController }
    from "@ohos/arkui/UIContext"
import { StateManager, ComputableState } from "@koalaui/runtime"
import { Context, PointerStyle, PixelMap } from "#external"
import { Nullable,  WidthBreakpoint, HeightBreakpoint } from "arkui/component/enums"
import { KeyEvent } from "arkui/component/common"
import { GlobalScope_ohos_font } from "arkui/component/arkui-external"
import router from '@ohos/router'
import { AlertDialog, AlertDialogParamWithConfirm, AlertDialogParamWithButtons,
    AlertDialogParamWithOptions }from "arkui/component/alertDialog"
import { ActionSheet, ActionSheetOptions } from "arkui/component/actionSheet"
import inspector from "@ohos/arkui/inspector"
import { ComponentContent } from 'arkui/ComponentContent'
import overlayManager from '@ohos/overlayManager'
import promptAction from '@ohos/promptAction'
import { ContextMenu } from 'arkui/component/contextMenu'
import { ArkUIAniModule } from "arkui.ani"
import { FontOptions, FontInfo } from "@ohos/font"
import { MeasureOptions } from "@ohos/measure"
import { GlobalScope_ohos_measure_utils } from "arkui/component/arkui-external"
import { SizeOptions } from "arkui/component/units"
import { Frame } from "arkui/Graphics"
import { TextMenuOptions } from "arkui/component/textCommon"
import { focusController } from "@ohos/arkui/focusController"
import { KeyProcessingMode } from "arkui/component/focus"
import { componentUtils } from "@ohos/arkui/componentUtils"
import { componentSnapshot } from "@ohos/arkui/componentSnapshot"
import { dragController } from "@ohos/arkui/dragController"
import { UIContextDispatchKeyEvent, UIContextAtomicServiceBar } from "arkui/component/arkui-custom"
import { Serializer } from "arkui/component/peers/Serializer"
import { GlobalScopeUicontextFontScale, GlobalScopeUicontextTextMenu } from "arkui/component/arkui-uicontext-text-utils"
import { GlobalScope } from "arkui/component/GlobalScope"
import { mediaquery } from '@ohos/mediaquery'
import { AsyncCallback, CustomBuilder, ArkComponentRootPeer, DragItemInfo } from 'arkui/component'
import { createUiDetachedRoot, destroyUiDetachedRoot } from "arkui/ArkUIEntry"
import { PeerNode } from 'arkui/PeerNode'
import { Deserializer } from "arkui/component/peers/Deserializer"
import { Serializer } from "arkui/component/peers/Serializer"
import { KBuffer } from "@koalaui/interop"
import { deserializeAndCallCallback } from "arkui/component/peers/CallbackDeserializeCall"
import { InteropNativeModule } from "@koalaui/interop"
import { Router as RouterExt } from 'arkui/handwritten';
import { ComponentContent } from "arkui/ComponentContent"

export class ContextRecord {
    uiContext?: UIContext
}

export class FontImpl extends Font {
    instanceId_: int32;
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }
    public registerFont(options: FontOptions) : void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        GlobalScope_ohos_font.registerFont(options);
        ArkUIAniModule._Common_Restore_InstanceId();
    }
    public getSystemFontList() : Array<string> {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let arrayResult_ = GlobalScope_ohos_font.getSystemFontList();
        ArkUIAniModule._Common_Restore_InstanceId();
        return arrayResult_;
    }
    public getFontByName(fontName : string) : FontInfo {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let fontInfo : FontInfo = GlobalScope_ohos_font.getFontByName(fontName);
        ArkUIAniModule._Common_Restore_InstanceId();
        return fontInfo;
    }
}

export class MediaQueryImpl extends MediaQuery {
    instanceId_: int32 = 100000;
    constructor(instanceId: int32) {
        this.instanceId_ = instanceId;
    }
    public matchMediaSync(condition: string): mediaquery.MediaQueryListener {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let mediaQueryListener = mediaquery.matchMediaSync(condition);
        ArkUIAniModule._Common_Restore_InstanceId();
        return mediaQueryListener;
    }
}

export class MeasureUtilsImpl extends MeasureUtils {
    instanceId_: int32;
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }
    public measureText(options: MeasureOptions) : number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let width = GlobalScope_ohos_measure_utils.measureText(options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return width;
    }
    public measureTextSize(options: MeasureOptions) : SizeOptions {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let sizeOptions = GlobalScope_ohos_measure_utils.measureTextSize(options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return sizeOptions;
    }
}

export class TextMenuControllerImpl extends TextMenuController {
    instanceId_: int32;
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }
    public setMenuOptions(options: TextMenuOptions) : void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        GlobalScopeUicontextTextMenu.setMenuOptions(options);
        ArkUIAniModule._Common_Restore_InstanceId();
    }
}

export class AtomicServiceBarInternal implements AtomicServiceBar {
    instanceId_: int32;
    constructor(instanceId: int32) {
        this.instanceId_ = instanceId;
    }
    public getBarRect(): Frame {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let frame = UIContextAtomicServiceBar.getBarRect();
        ArkUIAniModule._Common_Restore_InstanceId();
        return frame;
    }
}

export class FocusControllerImpl extends FocusController {
    instanceId_: int32
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    public clearFocus(): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        focusController.clearFocus();
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public requestFocus(key: string): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        focusController.requestFocus(key);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public activate(isActive: boolean, autoInactive?: boolean): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        focusController.activate(isActive, autoInactive);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public setAutoFocusTransfer(isAutoFocusTransfer: boolean): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        focusController.setAutoFocusTransfer(isAutoFocusTransfer);
        ArkUIAniModule._Common_Restore_InstanceId();
    }
    public setKeyProcessingMode(mode: KeyProcessingMode): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        focusController.setKeyProcessingMode(mode);
        ArkUIAniModule._Common_Restore_InstanceId();
    }
}

class ContextMenuControllerImpl extends ContextMenuController {
    instanceId_: int32
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    public close(): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        ContextMenu.close();
        ArkUIAniModule._Common_Restore_InstanceId();
    }
}

export class ComponentUtilsImpl extends ComponentUtils {
    instanceId_: int32
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    public getRectangleById(id: string): componentUtils.ComponentInfo {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
       let componentInformation = componentUtils.getRectangleById(id);
       ArkUIAniModule._Common_Restore_InstanceId();
       return componentInformation;
    }
}

export class ComponentSnapshotImpl extends ComponentSnapshot {
    instanceId_: int32
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    //@ts-ignore
    public get(id: string, callback: AsyncCallback<PixelMap>, options?: componentSnapshot.SnapshotOptions): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        componentSnapshot.get(id, callback, options);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    // @ts-ignore
    public get(id: string, options?: componentSnapshot.SnapshotOptions): Promise<PixelMap> {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let pixmap = componentSnapshot.get(id, options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return pixmap;
    }

    // @ts-ignore
    public createFromBuilder(builder: CustomBuilder, callback: AsyncCallback<PixelMap>,
                             delay?: number, checkImageStatus?: boolean,
                             options?: componentSnapshot.SnapshotOptions): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const peerNode = createUiDetachedRoot((): PeerNode => {
            return ArkComponentRootPeer.create(undefined);
        }, builder);
        let rootNode = peerNode.peer.ptr;
        const destroyCallback = (): void => {
            destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
        }
        ArkUIAniModule._ComponentSnapshot_createFromBuilderWithCallback(
            rootNode, destroyCallback, callback, delay, checkImageStatus);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    // @ts-ignore
    public createFromBuilder(builder: CustomBuilder, delay?: number, checkImageStatus?: boolean,
                             options?: componentSnapshot.SnapshotOptions): Promise<PixelMap> {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const peerNode = createUiDetachedRoot((): PeerNode => {
            return ArkComponentRootPeer.create(undefined);
        }, builder);
        let rootNode = peerNode.peer.ptr;
        const destroyCallback = (): void => {
            destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
        }
        let pixmap = ArkUIAniModule._ComponentSnapshot_createFromBuilderWithPromise(
            rootNode, destroyCallback, delay, checkImageStatus);
        ArkUIAniModule._Common_Restore_InstanceId();
        return pixmap;
    }

    public getSync(id: string, options?: componentSnapshot.SnapshotOptions): PixelMap {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let pixmap = componentSnapshot.getSync(id, options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return pixmap;
    }
    public getWithUniqueId(uniqueId: number, options?: componentSnapshot.SnapshotOptions): Promise<PixelMap> {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let pixmap = componentSnapshot.getWithUniqueId(uniqueId, options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return pixmap;
    }

    public getSyncWithUniqueId(uniqueId: number, options?: componentSnapshot.SnapshotOptions): PixelMap {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let pixmap = componentSnapshot.getSyncWithUniqueId(uniqueId, options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return pixmap;
    }

    public createFromComponent<T extends Object>(content: ComponentContent<T>, delay?: number, checkImageStatus?: boolean, options?: componentSnapshot.SnapshotOptions): Promise<PixelMap> {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let node = content.getFrameNode();
        if (node === undefined || node === null) {
            throw Error("The type of parameters is incorrect.");
        }
        let frameNode = node as FrameNode
        const destroyCallback = (): void => {}
        let pixmap = ArkUIAniModule._ComponentSnapshot_createFromComponentWithPromise(
            toPeerPtr(frameNode), destroyCallback, delay, checkImageStatus, options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return pixmap;
    }
}

export class DragControllerImpl extends DragController {
    instanceId_: int32
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    //@ts-ignore
    public executeDrag(custom: CustomBuilder | DragItemInfo, dragInfo: dragController.DragInfo,
        callback: AsyncCallback<dragController.DragEventParam>): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        if (typeof custom === "function") {
            const peerNode = createUiDetachedRoot((): PeerNode => {
                return ArkComponentRootPeer.create(undefined);
            }, custom as CustomBuilder);
            let rootNode = peerNode.peer.ptr;
            const destroyCallback = (): void => {
                destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
            }
            let dragItemInfoNull: DragItemInfo = {};
            ArkUIAniModule._DragController_executeDragWithCallback(dragItemInfoNull, rootNode,
                destroyCallback, dragInfo, callback);
        } else {
            let destroyCallback = (): void => { };
            let customDragInfo = custom as DragItemInfo;

            if (customDragInfo.pixelMap !== undefined) {
                ArkUIAniModule._DragController_executeDragWithCallback(custom as DragItemInfo, nullptr,
                    destroyCallback, dragInfo, callback);
            } else {
                const peerNode = createUiDetachedRoot((): PeerNode => {
                    return ArkComponentRootPeer.create(undefined);
                }, customDragInfo.builder as CustomBuilder);
                let rootNode = peerNode.peer.ptr;
                destroyCallback = (): void => {
                    destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
                }
                let dragItemInfoNull: DragItemInfo = {};
                ArkUIAniModule._DragController_executeDragWithCallback(dragItemInfoNull, rootNode,
                    destroyCallback, dragInfo, callback);
            }
        }
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    // @ts-ignore
    public executeDrag(custom: CustomBuilder | DragItemInfo, dragInfo: dragController.DragInfo):
        Promise<dragController.DragEventParam> {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let promise = new Promise<dragController.DragEventParam>((resolve, reject) => { });
        if (typeof custom === "function") {
            const peerNode = createUiDetachedRoot((): PeerNode => {
                return ArkComponentRootPeer.create(undefined);
            }, custom as CustomBuilder);
            let rootNode = peerNode.peer.ptr;
            const destroyCallback = (): void => {
                destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
            }
            let dragItemInfoNull: DragItemInfo = {};
            promise = ArkUIAniModule._DragController_executeDragWithPromise(dragItemInfoNull, rootNode,
                destroyCallback, dragInfo);
        } else {
            let destroyCallback = (): void => { };
            let customDragInfo = custom as DragItemInfo;

            if (customDragInfo.pixelMap !== undefined) {
                promise = ArkUIAniModule._DragController_executeDragWithPromise(custom as DragItemInfo,
                    nullptr, destroyCallback, dragInfo);
            } else {
                const peerNode = createUiDetachedRoot((): PeerNode => {
                    return ArkComponentRootPeer.create(undefined);
                }, customDragInfo.builder as CustomBuilder);
                let rootNode = peerNode.peer.ptr;
                destroyCallback = (): void => {
                    destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
                }
                let dragItemInfoNull: DragItemInfo = {};
                promise = ArkUIAniModule._DragController_executeDragWithPromise(dragItemInfoNull, rootNode,
                    destroyCallback, dragInfo);
            }
        }
        ArkUIAniModule._Common_Restore_InstanceId();
        return promise;
    }

    private handleDragActionCreationForBuilder(customArray: Array<CustomBuilder | DragItemInfo>,
        dragInfo: dragController.DragInfo): dragController.DragAction {
        let rootNodeArray: Array<KPointer> = [];
        let peerNodeArray: Array<PeerNode> = [];
        customArray.forEach((customBuilder) => {
            const builder = customBuilder as CustomBuilder;
            const peerNode = createUiDetachedRoot((): PeerNode => {
                return ArkComponentRootPeer.create(undefined);
            }, builder);
            let rootNode = peerNode.peer.ptr;
            rootNodeArray.push(rootNode);
            peerNodeArray.push(peerNode);
        });
        let destroyCallback = (): void => {
            peerNodeArray.forEach((peerNode) => {
                destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
            });
        };
        let dragItemInfoArray: Array<DragItemInfo> = [];
        return ArkUIAniModule._DragController_createDragAction(dragItemInfoArray, rootNodeArray,
            destroyCallback, dragInfo);
    }

    private handleDragActionCreationForDragItemInfo(customArray: Array<CustomBuilder | DragItemInfo>,
        dragInfo: dragController.DragInfo): dragController.DragAction {
        let rootNodeArray: Array<KPointer> = [];
        let peerNodeArray: Array<PeerNode> = [];
        let dragItemInfoArray: Array<DragItemInfo> = [];
        let destroyCallback = (): void => { };
        customArray.forEach(item => {
            if (item instanceof DragItemInfo) {
                dragItemInfoArray.push(item);
            } else {
                const builder = item as CustomBuilder;
                const peerNode = createUiDetachedRoot((): PeerNode => {
                    return ArkComponentRootPeer.create(undefined);
                }, builder);
                const rootNode = peerNode.peer.ptr;
                rootNodeArray.push(rootNode);
                peerNodeArray.push(peerNode);
                destroyCallback = (): void => {
                    peerNodeArray.forEach((peerNode) => {
                        destroyUiDetachedRoot(peerNode.peer.ptr, this.instanceId_);
                    });
                    rootNodeArray.length = 0;
                    peerNodeArray.length = 0;
                };
            }
        });
        return ArkUIAniModule._DragController_createDragAction(dragItemInfoArray, rootNodeArray,
            destroyCallback, dragInfo);
    }

    public createDragAction(customArray: Array<CustomBuilder | DragItemInfo>,
        dragInfo: dragController.DragInfo): dragController.DragAction {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);

        let firstNonUndefinedElement = customArray.find((element) => element !== undefined);
        if (typeof firstNonUndefinedElement === "function") {
            let dragAction = this.handleDragActionCreationForBuilder(customArray, dragInfo);
            ArkUIAniModule._Common_Restore_InstanceId();
            return dragAction;
        } else {
            let dragAction = this.handleDragActionCreationForDragItemInfo(customArray, dragInfo);
            ArkUIAniModule._Common_Restore_InstanceId();
            return dragAction;
        }
    }

    public setDragEventStrictReportingEnabled(enable: boolean): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        ArkUIAniModule._DragController_setDragEventStrictReportingEnabled(enable);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public cancelDataLoading(key: string): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        ArkUIAniModule._DragController_cancelDataLoading(key);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public notifyDragStartRequest(requestStatus: dragController.DragStartRequestStatus): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        ArkUIAniModule._DragController_notifyDragStartReques(requestStatus);
        ArkUIAniModule._Common_Restore_InstanceId();
    }
}

export class RouterImpl extends Router {
    public pushUrl(options: router.RouterOptions): Promise<void> {
        return new Promise<void>((resolve, reject) => {
            router.pushUrl(options)
        })
    }

    public back(options?:router.RouterOptions): void {
        router.back(options)
    }

    public clear(): void {
        router.clear()
    }
}

export class UIInspectorImpl extends UIInspector {
    instanceId_: int32 = -1;
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }
    public createComponentObserver(id: string): inspector.ComponentObserver {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let componentObserver = inspector.createComponentObserver(id);
        ArkUIAniModule._Common_Restore_InstanceId();
        return componentObserver;
    }
}

export class OverlayManagerImpl extends OverlayManager {
    instanceId_: int32 = -1;
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    setOverlayManagerOptions(options: OverlayManagerOptions): boolean {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const retval = overlayManager.setOverlayManagerOptions(options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return retval;
    }

    getOverlayManagerOptions(): OverlayManagerOptions {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const retval = overlayManager.getOverlayManagerOptions();
        ArkUIAniModule._Common_Restore_InstanceId();
        return retval;
    }

    addComponentContent(content: ComponentContent, index?: number): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let ptr: KPointer = 0
        if (content.getNodePtr() !== undefined) {
            ptr = content.getNodePtr() as (KPointer)
        }
        let idx: number = -1
        if (index !== undefined) {
            idx = index
        }
        overlayManager.addComponentContent(ptr, idx);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    addComponentContentWithOrder(content: ComponentContent, levelOrder?: number): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let ptr: KPointer = 0
        if (content.getNodePtr() !== undefined) {
            ptr = content.getNodePtr() as (KPointer)
        }
        let order: number = 0
        if (levelOrder !== undefined) {
            order = levelOrder
        }
        overlayManager.addComponentContentWithOrder(ptr, order);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    removeComponentContent(content: ComponentContent): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let ptr: KPointer = 0
        if (content.getNodePtr() !== undefined) {
            ptr = content.getNodePtr() as (KPointer)
        }
        overlayManager.removeComponentContent(ptr);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    showComponentContent(content: ComponentContent): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let ptr: KPointer = 0
        if (content.getNodePtr() !== undefined) {
            ptr = content.getNodePtr() as (KPointer)
        }
        overlayManager.showComponentContent(ptr);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    hideComponentContent(content: ComponentContent): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let ptr: KPointer = 0
        if (content.getNodePtr() !== undefined) {
            ptr = content.getNodePtr() as (KPointer)
        }
        overlayManager.hideComponentContent(ptr);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    showAllComponentContents(): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        overlayManager.showAllComponentContents();
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    hideAllComponentContents(): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        overlayManager.hideAllComponentContents();
        ArkUIAniModule._Common_Restore_InstanceId();
    }
}

export class PromptActionImpl extends PromptAction {
    instanceId_: int32 = -1;
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    showToast(options: promptAction.ShowToastOptions): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        promptAction.showToast(options);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    openToast(options: promptAction.ShowToastOptions): Promise<number> {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let promise = promptAction.openToast(options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return promise;
    }

    closeToast(toastId: number): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        promptAction.closeToast(toastId);
        ArkUIAniModule._Common_Restore_InstanceId();
    }
}

export class CursorControllerImpl extends CursorController {
    instanceId_: int32
    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
    }

    public restoreDefault(): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        GlobalScope.cursorControl_restoreDefault()
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public setCursor(value: PointerStyle): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        GlobalScope.cursorControl_setCursor(value)
        ArkUIAniModule._Common_Restore_InstanceId();
    }
}

export class DetachedRootEntryManager {
    instanceId_: int32;
    detachedRoots_: Map<KPointer, ComputableState<PeerNode>>;
    constructor(instanceId: int32) {
        this.detachedRoots_ = new Map<KPointer, ComputableState<PeerNode>>();
        this.instanceId_ = instanceId;
    }

    public getDetachedRoots() : Map<KPointer, ComputableState<PeerNode>> {
        return this.detachedRoots_;
    }
}

export class UIContextImpl extends UIContext {
    instanceId_: int32 = -1;
    stateMgr: StateManager | undefined = undefined;
    observer_ :UIObserver |null = null;
    router_: Router = new RouterImpl()
    focusController_: FocusControllerImpl;
    componentUtils_: ComponentUtilsImpl;
    componentSnapshot_: ComponentSnapshotImpl;
    dragController_: DragControllerImpl;
    atomicServiceBar_: AtomicServiceBarInternal;
    uiInspector_: UIInspectorImpl | null = null;
    contextMenuController_: ContextMenuControllerImpl;
    overlayManager_: OverlayManagerImpl | null = null;
    promptAction_: PromptActionImpl | null = null;
    cursorController_: CursorControllerImpl;
    font_: FontImpl;
    measureUtils_: MeasureUtilsImpl;
    textMenuController_: TextMenuControllerImpl;
    detachedRootEntryManager_: DetachedRootEntryManager;

    bufferSize = 4096
    buffer: KBuffer = new KBuffer(this.bufferSize)
    position: int64 = 0
    deserializer: Deserializer = new Deserializer(this.buffer.buffer, this.bufferSize)

    constructor(instanceId: int32) {
        super()
        this.instanceId_ = instanceId;
        this.focusController_ = new FocusControllerImpl(instanceId);
        this.componentUtils_ = new ComponentUtilsImpl(instanceId);
        this.componentSnapshot_ = new ComponentSnapshotImpl(instanceId);
        this.dragController_ = new DragControllerImpl(instanceId);
        this.atomicServiceBar_ = new AtomicServiceBarInternal(instanceId);
        this.contextMenuController_ = new ContextMenuControllerImpl(instanceId);
        this.cursorController_ = new CursorControllerImpl(instanceId);
        this.font_ = new FontImpl(instanceId);
        this.measureUtils_ = new MeasureUtilsImpl(instanceId);
        this.textMenuController_ = new TextMenuControllerImpl(instanceId);
        this.detachedRootEntryManager_ = new DetachedRootEntryManager(instanceId);
    }
    public getInstanceId() : int32 {
        return this.instanceId_;
    }
    public getDetachedRootEntryManager() : DetachedRootEntryManager {
        return this.detachedRootEntryManager_;
    }
    dispatchCallback(buffer: KSerializerBuffer, length: int32): void {
        if (length <= 0) {
            return
        }

        if (this.position + length > this.bufferSize) {
            throw Error("Callback buffer of UIContextImpl is full!")
        }

        InteropNativeModule._CopyBuffer(this.buffer.buffer + this.position, length, buffer)
        this.position += length
    }

    callCallbacks(): void {
        if (this.position === 0) {
            return
        }
        this.deserializer.resetCurrentPosition()
        this.runScopedTask(() => {
            while (this.deserializer.currentPosition() < this.position) {
                deserializeAndCallCallback(this.deserializer)
            }
        })
        this.position = 0;
    }

    public getFont() : Font {
        return this.font_;
    }
    public getMediaQuery(): MediaQuery {
        return new MediaQueryImpl(this.instanceId_);
    }
    public getMeasureUtils() : MeasureUtils {
        return this.measureUtils_;
    }
    public getTextMenuController() : TextMenuController {
        return this.textMenuController_;
    }
    public isFollowingSystemFontScale() : boolean {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let follow = GlobalScopeUicontextFontScale.isFollowingSystemFontScale();
        ArkUIAniModule._Common_Restore_InstanceId();
        return follow;
    }
    public getMaxFontScale() : number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let fontScale = GlobalScopeUicontextFontScale.getMaxFontScale();
        ArkUIAniModule._Common_Restore_InstanceId();
        return fontScale;
    }
    public getFrameNodeById(id: string): FrameNode | null {
        const id_casted = id as (string);
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodeByKey(id_casted);
        if (retval === nullptr) {
            ArkUIAniModule._Common_Restore_InstanceId();
            return null;
        }
        let node = FrameNodeUtils.searchNodeInRegisterProxy(retval);
        if (!node) {
            node = FrameNodeUtils.createFrameNode(this, retval);
        }
        ArkUIAniModule._Common_Restore_InstanceId();
        return node;
    }
    getAttachedFrameNodeById(id: string): FrameNode | null {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const retval = ArkUIGeneratedNativeModule._FrameNode_getAttachedFrameNodeById(id);
        if (retval === nullptr) {
            ArkUIAniModule._Common_Restore_InstanceId();
            return null;
        }
        let node = FrameNodeUtils.searchNodeInRegisterProxy(retval);
        if (!node) {
            node = FrameNodeUtils.createFrameNode(this, retval);
        }
        ArkUIAniModule._Common_Restore_InstanceId();
        return node;
    }
    getFrameNodeByNodeId(id: number): FrameNode | null {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodeById(id);
        if (retval === nullptr) {
            ArkUIAniModule._Common_Restore_InstanceId();
            return null;
        }
        let node = FrameNodeUtils.searchNodeInRegisterProxy(retval);
        if (!node) {
            node = FrameNodeUtils.createFrameNode(this, retval);
        }
        ArkUIAniModule._Common_Restore_InstanceId();
        return node;
    }
    getFrameNodeByUniqueId(id: number): FrameNode | null {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        const retval = ArkUIGeneratedNativeModule._FrameNode_getFrameNodeByUniqueId(id);
        if (retval === nullptr) {
            ArkUIAniModule._Common_Restore_InstanceId();
            return null;
        }
        let node = FrameNodeUtils.searchNodeInRegisterProxy(retval);
        if (!node) {
            node = FrameNodeUtils.createFrameNode(this, retval);
        }
        ArkUIAniModule._Common_Restore_InstanceId();
        return node;
    }
    getHostContext(): Context | undefined {
        return ArkUIAniModule._Common_GetHostContext();
    }

    public getAtomicServiceBar(): Nullable<AtomicServiceBar> {
        return this.atomicServiceBar_;
    }

    public dispatchKeyEvent(node: number | string, event: KeyEvent): boolean {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let result = UIContextDispatchKeyEvent.dispatchKeyEvent(node, event);
        ArkUIAniModule._Common_Restore_InstanceId();
        return result;
    }

    public getFocusController(): FocusController {
        return this.focusController_;
    }

    public getContextMenuController(): ContextMenuController {
        return this.contextMenuController_;
    }

    public getComponentUtils(): ComponentUtils {
        return this.componentUtils_;
    }

    public getComponentSnapshot(): ComponentSnapshot {
        return this.componentSnapshot_;
    }

    public getDragController(): DragController {
        return this.dragController_;
    }

    public getCursorController(): CursorController {
        return this.cursorController_;
    }

    public getRouter(): Router {
        if (this.router_ === undefined) {
            this.router_ = new RouterImpl()
        }
        return this.router_
    }

    public setRouter(router: RouterExt) {
        if (this.router_ === undefined) {
            this.router_ = new RouterImpl()
        }
        this.router_.setRouter(router);
    }

    public animateTo(param: AnimateParam, event: (() => void)): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_)
        _animateTo(param, event);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public createAnimator(options: AnimatorOptions | SimpleAnimatorOptions): AnimatorResult {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_)
        let animatorRet = Animator.create(options);
        ArkUIAniModule._Common_Restore_InstanceId();
        return animatorRet;
    }
    public setFrameCallback(onFrameCallback: ((index: number) => void), onIdleCallback: ((index: number) => void),
                                              delayTime: number): void {
        const onFrameCallback_casted = onFrameCallback as (((index: number) => void))
        const onIdleCallback_casted = onIdleCallback as (((index: number) => void))
        const delayTime_casted = delayTime as (number)
        this.setFrameCallback_serialize(onFrameCallback_casted, onIdleCallback_casted, delayTime_casted)
        return
    }
      public getWindowName(): string | undefined {
        return  this.getWindowName_serialize();
       
    }
    public getWindowWidthBreakpoint(): WidthBreakpoint {
       
        return this.getWindowWidthBreakpoint_serialize() as WidthBreakpoint;
     
    }
    public getWindowHeightBreakpoint(): HeightBreakpoint {
      return this.getWindowHeightBreakpoint_serialize() as HeightBreakpoint;
       
    }
    public vp2px(value: number): number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let result = GlobalScope.vp2px(value);
        ArkUIAniModule._Common_Restore_InstanceId();
        return result;
    }
    public px2vp(value: number): number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let result = GlobalScope.px2vp(value);
        ArkUIAniModule._Common_Restore_InstanceId();
        return result;
    }
    public fp2px(value: number): number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let result = GlobalScope.fp2px(value);
        ArkUIAniModule._Common_Restore_InstanceId();
        return result;
    }
    public px2fp(value: number): number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let result = GlobalScope.px2fp(value);
        ArkUIAniModule._Common_Restore_InstanceId();
        return result;
    }
    public lpx2px(value: number): number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let result = GlobalScope.lpx2px(value);
        ArkUIAniModule._Common_Restore_InstanceId();
        return result;
    }
    public px2lpx(value: number): number {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        let result = GlobalScope.px2lpx(value);
        ArkUIAniModule._Common_Restore_InstanceId();
        return result;
    }
  
    private getWindowName_serialize(): string | undefined {
        return ArkUIGeneratedNativeModule._UIContext_getWindowName(this.instanceId_);
    }
    private getWindowWidthBreakpoint_serialize(): WidthBreakpoint {
        const widthBreakpoint = ArkUIGeneratedNativeModule._UIContext_getWindowWidthBreakpoint(this.instanceId_);
        let widthBreakpointEnum = widthBreakpoint  as int32 as WidthBreakpoint;
        return widthBreakpointEnum;
   
    }
    private getWindowHeightBreakpoint_serialize(): HeightBreakpoint {
        const heightBreakpoint = ArkUIGeneratedNativeModule._UIContext_getWindowHeightBreakpoint(this.instanceId_);
        let heightBreakpointEnum = heightBreakpoint as int32 as HeightBreakpoint;
        return heightBreakpointEnum;
    }
    private setFrameCallback_serialize(onFrameCallback: ((index: number) => void),
                                                         onIdleCallback: ((index: number) => void),
                                                         delayTime: number): void {
        const thisSerializer: Serializer = Serializer.hold()
        thisSerializer.holdAndWriteCallback(onFrameCallback)
        thisSerializer.holdAndWriteCallback(onIdleCallback)
        ArkUIGeneratedNativeModule._SystemOps_setFrameCallback(thisSerializer.asBuffer(),
                                                               thisSerializer.length(), delayTime)
        thisSerializer.release()
    }
    runScopedTask(callback: () => void): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_)
        if (callback !== undefined) {
            callback()
        }
        ArkUIAniModule._Common_Restore_InstanceId()
    }
    clearResourceCache(): void {
        ArkUIGeneratedNativeModule._SystemOps_resourceManagerReset()
    }
    postFrameCallback(frameCallback: FrameCallback): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_)
        const onFrameFunc = frameCallback.onFrame
        const onIdleFunc = frameCallback.onIdle
        this.setFrameCallback(onFrameFunc, onIdleFunc, 0)
        ArkUIAniModule._Common_Restore_InstanceId()
    }
    postDelayedFrameCallback(frameCallback: FrameCallback, delayTime: number): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_)
        const onFrameFunc = frameCallback.onFrame
        const onIdleFunc = frameCallback.onIdle
        this.setFrameCallback(onFrameFunc, onIdleFunc, delayTime)
        ArkUIAniModule._Common_Restore_InstanceId()
    }
    public getUIInspector(): UIInspector {
        if (!this.uiInspector_) {
            this.uiInspector_ = new UIInspectorImpl(this.instanceId_);
        }
        return this.uiInspector_ as UIInspector;
    }
    public getUIObserver(): UIObserver {
        if (!this.observer_) {
            this.observer_ = new UIObserver(this.instanceId_);
        }
        return this.observer_ as UIObserver;
    }

    public getOverlayManager(): OverlayManager {
        if (!this.overlayManager_) {
            this.overlayManager_ = new OverlayManagerImpl(this.instanceId_);
        }
        if (this.overlayManager_) {
            const options: OverlayManagerOptions = { renderRootOverlay: true, enableBackPressedEvent: false };
            this.overlayManager_!.setOverlayManagerOptions(options);
        }
        return this.overlayManager_ as OverlayManager;
    }

    public setOverlayManagerOptions(options: OverlayManagerOptions): boolean {
        if (!this.overlayManager_) {
            this.overlayManager_ = new OverlayManagerImpl(this.instanceId_);
        }
        if (this.overlayManager_) {
            return this.overlayManager_!.setOverlayManagerOptions(options);
        }
        return false;
    }

    public getOverlayManagerOptions(): OverlayManagerOptions {
        if (!this.overlayManager_) {
            this.overlayManager_ = new OverlayManagerImpl(this.instanceId_);
        }
        if (this.overlayManager_) {
            return this.overlayManager_!.getOverlayManagerOptions();
        }
        return {};
    }

    public getPromptAction(): PromptAction {
        if (!this.promptAction_) {
            this.promptAction_ = new PromptActionImpl(this.instanceId_);
        }
        return this.promptAction_ as PromptAction;
    }

    public showAlertDialog(options: AlertDialogParamWithConfirm | AlertDialogParamWithButtons |
        AlertDialogParamWithOptions): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        AlertDialog.show(options);
        ArkUIAniModule._Common_Restore_InstanceId();
    }

    public showActionSheet(options: ActionSheetOptions): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_);
        ActionSheet.show(options);
        ArkUIAniModule._Common_Restore_InstanceId();
    }
    
    // @ts-ignore
    public freezeUINode(id: number, isFrozen: boolean): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_)
        ArkUIGeneratedNativeModule._UIContextImp_freezeUINode1(id, isFrozen ? 1 : 0);
        ArkUIAniModule._Common_Restore_InstanceId()
    }

    // @ts-ignore
    public freezeUINode(id: string, isFrozen: boolean): void {
        ArkUIAniModule._Common_Sync_InstanceId(this.instanceId_)
        ArkUIGeneratedNativeModule._UIContextImp_freezeUINode0(id, isFrozen ? 1 : 0);
        ArkUIAniModule._Common_Restore_InstanceId()
    }
}
