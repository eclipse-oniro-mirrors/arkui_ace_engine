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

import { TypeChecker } from "#components"
import { DragEvent, DragPreviewOptions, ICurve, CustomBuilder, Callback, DragItemInfo } from 'arkui/component/common';
import { TouchPoint, ResourceColor } from 'arkui/component/units';
import { Curve, Color } from 'arkui/component/enums';
import { UnifiedData, PixelMap } from "#external";
import { ArkUIAniModule } from "arkui.ani";
import { Serializer } from 'arkui/component';
import { int32 } from "@koalaui/common";
import { KPointer, runtimeType, RuntimeType } from '@koalaui/interop';
import { Resource } from "global.resource";
export namespace dragController {

    export enum DragStatus {
        STARTED = 0,
        ENDED = 1,
    }

    export enum DragStartRequestStatus {
        WAITING = 0,
        READY = 1,
    }

    export interface AnimationOptions {
        duration?: number;
        curve?: Curve | ICurve;
    }

    export interface DragPreview {
        setForegroundColor(color: ResourceColor): void;
        animate(options: AnimationOptions, handler: () =>void): void;
    }

    class Cleaner {
        private ptr: KPointer = 0;
        private className: string;
        constructor(className: string, ptr: KPointer) {
            this.ptr = ptr;
            this.className = className;
        }
        public clean(): void {
            if (this.className == "DragAction") {
                ArkUIAniModule._DragController_cleanDragAction(this.ptr);
            } else if (this.className == "DragPreview") {
                ArkUIAniModule._DragController_cleanDragPreview(this.ptr);
            }
        }
    }

    export function callback(cleaner: Cleaner): void {
        cleaner.clean()
    }

    let destroyRegister = new FinalizationRegistry<Cleaner>(callback)
    let unregisterToken = new object()

    class DragPreviewInner implements DragPreview {
        dragPreview:KPointer = 0;
        private cleaner: Cleaner | null = null;
        constructor(result:KPointer) {
            this.dragPreview = result;
            this.registerCleaner(this.dragPreview)
        }
        public setForegroundColor(color: ResourceColor) {
            const thisSerializer : Serializer = Serializer.hold()
            let color_type : int32 = RuntimeType.UNDEFINED
            color_type = runtimeType(color)
            if (TypeChecker.isColor(color)) {
                thisSerializer.writeInt8(0 as int32)
                const color_0  = color as Color
                thisSerializer.writeInt32(TypeChecker.Color_ToNumeric(color_0))
            }
            else if (RuntimeType.NUMBER == color_type) {
                thisSerializer.writeInt8(1 as int32)
                const color_1  = color as number
                thisSerializer.writeNumber(color_1)
            }
            else if (RuntimeType.STRING == color_type) {
                thisSerializer.writeInt8(2 as int32)
                const color_2  = color as string
                thisSerializer.writeString(color_2)
            }
            else if (RuntimeType.OBJECT == color_type) {
                thisSerializer.writeInt8(3 as int32)
                const color_3  = color as Resource
                thisSerializer.writeResource(color_3)
            }
            ArkUIAniModule._DragController_setForegroundColor(
                thisSerializer.asBuffer(), thisSerializer.length(), this.dragPreview);
            thisSerializer.release()
        }
        public animate(options: AnimationOptions, handler: () =>void) {
            ArkUIAniModule._DragController_animate(options, handler, this.dragPreview);
        }

        registerCleaner(ptr: KPointer): void {
            this.cleaner = new Cleaner("DragPreview", ptr);
            destroyRegister.register(this, this.cleaner!, unregisterToken);
        }
        unregisterCleaner(): void {
            destroyRegister.unregister(unregisterToken);
        }
    }
    
    export interface DragInfo {
        pointerId: number;
        data?: UnifiedData;
        extraParams?: string;
        touchPoint?: TouchPoint;
        previewOptions?: DragPreviewOptions;
    }

    export interface DragEventParam {
        event: DragEvent;
        extraParams: string;
    }

    export interface DragAndDropInfo {
        status: DragStatus;
        event: DragEvent;
        extraParams: string;
    }

    class DragItemInfoInner implements DragItemInfo {
        pixelMap?: PixelMap;
        builder?: CustomBuilder;
        extraInfo?: string;
    }
    
    export class DragAndDropInfoInner implements DragAndDropInfo {
        status: DragStatus;
        _event: DragEvent;
        extraParams: string;
        constructor(status: DragStatus, event: DragEvent, extraParmas: string) {
            this.status = status
            this._event = event
            this.extraParams = extraParmas
        }
        set event(value: DragEvent) {
            this._event = value;
        }
        get event() : DragEvent {
            return this._event;
        }
    }

    export class DragEventParamInner implements DragEventParam {
        _event: DragEvent;
        extraParams: string;
        constructor(event: DragEvent, extraParams: string) {
            this._event = event
            this.extraParams = extraParams
        }
        set event(value: DragEvent) {
            this._event = value;
        }
        get event() : DragEvent {
            return this._event;
        }
    }

    export interface DragAction {
        startDrag(): Promise<void>;
        on(type: string, callback: Callback<DragAndDropInfo>): void;
        off(type: string, callback?: Callback<DragAndDropInfo>): void;
    }

    export class DragActionInner implements DragAction {
        dragAction:KPointer;
        private cleaner: Cleaner | null = null;
        constructor(result:KPointer) {
            this.dragAction = result;
            this.registerCleaner(this.dragAction)
        }
        public startDrag(): Promise<void> {
            let promise = ArkUIAniModule._DragController_startDrag(this.dragAction);
            return promise;
        }
        public on(type: string, callback: Callback<DragAndDropInfo>) {
            ArkUIAniModule._DragController_on(type, callback, this.dragAction);
        }
        public off(type: string, callback?: Callback<DragAndDropInfo>) {
            ArkUIAniModule._DragController_off(type, callback, this.dragAction);
        }

        registerCleaner(ptr: KPointer): void {
            this.cleaner = new Cleaner("DragAction", ptr);
            destroyRegister.register(this, this.cleaner!, unregisterToken);
        }
        unregisterCleaner(): void {
            destroyRegister.unregister(unregisterToken);
        }
    }
}