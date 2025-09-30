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

import { DragEvent, DragPreviewOptions, ICurve, CustomBuilder, Callback, DragItemInfo } from 'arkui/framework'
import { TouchPoint, ResourceColor } from 'arkui/framework'
import { Curve } from 'arkui/framework'
import { UnifiedData, PixelMap } from "#external";
import { ArkUIAniModule } from "arkui.ani"
import { KPointer } from '@koalaui/interop';
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

    class DragPreviewInner implements DragPreview {
        dragPreview:KPointer = 0;
        constructor(result:KPointer) {
            this.dragPreview = result;
        }
        public setForegroundColor(color: ResourceColor) {
            ArkUIAniModule._DragController_setForegroundColor(color, this.dragPreview);
        }
        public animate(options: AnimationOptions, handler: () =>void) {
            ArkUIAniModule._DragController_animate(options, handler, this.dragPreview);
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
        constructor(result:KPointer) {
            this.dragAction = result;
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
    }
}