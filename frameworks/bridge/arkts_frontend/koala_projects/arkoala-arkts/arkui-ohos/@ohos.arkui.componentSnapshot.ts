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

import { AsyncCallback } from 'arkui/component';
import { PixelMap } from "#external";

export declare namespace componentSnapshot {
    export interface SnapshotRegion {
        left: number;
        right: number;
        top: number;
        bottom: number;
    }
    export interface LocalizedSnapshotRegion {
        start: number;
        end: number;
        top: number;
        bottom: number;
    }
    export type SnapshotRegionType = SnapshotRegion | LocalizedSnapshotRegion;
    export interface SnapshotOptions {
        scale?: number;
        waitUntilRenderFinished?: boolean;
        region?: SnapshotRegionType;
    }
    export function get(callbackId: string, callback: AsyncCallback<PixelMap>, options?: SnapshotOptions): void;
    export function get(promiseId: string, options?: SnapshotOptions): Promise<PixelMap>;
    export function getSync(value: string, options?: SnapshotOptions): PixelMap;
    export function getWithUniqueId(uniqueIdValue: number, options?: SnapshotOptions): Promise<PixelMap>;
    export function getSyncWithUniqueId(uniqueIdValue: number, options?: SnapshotOptions): PixelMap;
}