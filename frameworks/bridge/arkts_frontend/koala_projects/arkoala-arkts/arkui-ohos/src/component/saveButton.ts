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

import { int32, int64, float32 } from "@koalaui/common"
import { KInt, KPointer, KBoolean, KStringPtr, wrapCallback, NativeBuffer } from "@koalaui/interop"
import { NodeAttach, remember } from "@koalaui/runtime"
import { ButtonType } from "./button"
import { ClickEvent } from "./common"
import { SecurityComponentMethod, SecurityComponentLayoutDirection } from "./securityComponent"
import { Dimension, Position, Edges, LocalizedEdges, ResourceColor, Padding, Length, SizeOptions, ConstraintSizeOptions } from "./units"
import { FontStyle, FontWeight, BorderStyle } from "./enums"
import { Resource } from "global/resource";
import { ArkSaveButtonComponent } from "./../generated/ArkSaveButton"
import { ArkSaveButtonPeer } from "./../generated/peers/ArkSaveButtonPeer"
export enum SaveIconStyle {
    FULL_FILLED = 0,
    LINES = 1,
    PICTURE = 2
}
export enum SaveDescription {
    DOWNLOAD = 0,
    DOWNLOAD_FILE = 1,
    SAVE = 2,
    SAVE_IMAGE = 3,
    SAVE_FILE = 4,
    DOWNLOAD_AND_SHARE = 5,
    RECEIVE = 6,
    CONTINUE_TO_RECEIVE = 7,
    SAVE_TO_GALLERY = 8,
    EXPORT_TO_GALLERY = 9,
    QUICK_SAVE_TO_GALLERY = 10,
    RESAVE_TO_GALLERY = 11
}
export interface SaveButtonOptions {
    icon?: SaveIconStyle;
    text?: SaveDescription;
    buttonType?: ButtonType;
}
export enum SaveButtonOnClickResult {
    SUCCESS = 0,
    TEMPORARY_AUTHORIZATION_FAILED = 1
}
export type Callback_ClickEvent_SaveButtonOnClickResult_Void = (event: ClickEvent, result: SaveButtonOnClickResult) => void;
/** @memo:stable */
export interface SaveButtonAttribute extends SecurityComponentMethod {
    /** @memo */
    setSaveButtonOptions(options?: SaveButtonOptions): this
    /** @memo */
    onClick(value: ((event: ClickEvent,result: SaveButtonOnClickResult) => void)): this
}
/** @memo */
export function SaveButton(
  /** @memo */
  style: ((attributes: SaveButtonAttribute) => void) | undefined,
  options?: SaveButtonOptions | undefined, 
  /** @memo */
  content_?: () => void,
) {
    const receiver = remember(() => {
        return new ArkSaveButtonComponent()
    })
    NodeAttach<ArkSaveButtonPeer>((): ArkSaveButtonPeer => ArkSaveButtonPeer.create(receiver), (_: ArkSaveButtonPeer) => {
        receiver.setSaveButtonOptions(options)
        style?.(receiver)
        content_?.()
        receiver.applyAttributesFinish()
    })
}
