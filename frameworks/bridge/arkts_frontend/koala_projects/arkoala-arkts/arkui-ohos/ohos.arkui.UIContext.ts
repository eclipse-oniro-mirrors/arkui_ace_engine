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

import { FrameNode, FrameNodeInternal } from "./src/FrameNode"
import { GlobalScope_ohos_font } from "./src/component/arkui-external"
import { GlobalScope_ohos_measure_utils } from "./src/component/arkui-external"
import { FontOptions, FontInfo } from "@ohos/font/font"
import { MeasureOptions } from "@ohos/measure"
import { SizeOptions } from "./src/component/units"
import { ArkUIGeneratedNativeModule } from "#components"
import { int32 } from "@koalaui/common"
import { nullptr } from "@koalaui/interop"
import { _animateTo } from "./src/handwritten"
// import { AnimateParam } from './src/component'
import { AnimatorResult , AnimatorOptions, Animator} from "@ohos/animator"
import { Context } from "#external"
import { ArkUIAniModule } from "arkui.ani"
import { Serializer } from "./src/component/peers/Serializer"

export class Font {
    instanceId_: int32 = 10001;
    constructor(instanceId: int32) {
        this.instanceId_ = instanceId;
    }
    public registerFont(options: FontOptions) : void {
        ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
        GlobalScope_ohos_font.registerFont(options);
        ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
    }
    public getSystemFontList() : Array<string> {
        ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
        let arrayResult_ = GlobalScope_ohos_font.getSystemFontList();
        ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
        return new Array<string>();
    }
    public getFontByName(fontName : string) : FontInfo {
        ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
        let fontInfo : FontInfo = GlobalScope_ohos_font.getFontByName(fontName);
        ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
        return fontInfo;
    }
}

export class MeasureUtils {
    instanceId_: int32 = 10001;
    constructor(instanceId: int32) {
        this.instanceId_ = instanceId;
    }
    public measureText(options: MeasureOptions) : number {
        ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
        let width = GlobalScope_ohos_measure_utils.measureText(options);
        ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
        return 0;
    }
    public measureTextSize(options: MeasureOptions) : SizeOptions {
        ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
        let sizeOptions = GlobalScope_ohos_measure_utils.measureTextSize(options);
        ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
        return {};
    }
}

export class UIContext {
    instanceId_: int32 = 100000;
    constructor(instanceId: int32) {
        this.instanceId_ = instanceId;
    }
    public getFont() : Font {
        let font : Font = new Font(this.instanceId_);
        return font;
    }
    public getMeasureUtils() : MeasureUtils {
        let measureUtils : MeasureUtils = new MeasureUtils(this.instanceId_);
        return measureUtils;
    }
    public getFrameNodeById(id: string): FrameNode | null {
        const id_casted = id as (string);
        ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
        const retval  = ArkUIGeneratedNativeModule._FrameNode_getFrameNodeByKey(id_casted);
        if (retval != nullptr) {
            const obj : FrameNode = FrameNodeInternal.fromPtr(retval);
            ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
            return obj;
        }
        ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
        return null;
    }
    getHostContext(): Context | undefined {
        return ArkUIAniModule._Common_GetHostContext();
    }
    // public animateTo(param: AnimateParam, event: (() => void)): void {
    //     ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
    //     _animateTo(param, event);
    //     ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
    // }
    public createAnimator(options: AnimatorOptions): AnimatorResult {
        ArkUIGeneratedNativeModule._SystemOps_syncInstanceId(this.instanceId_);
        let animatorRet = Animator.create(options);
        ArkUIGeneratedNativeModule._SystemOps_restoreInstanceId();
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
}
export abstract class FrameCallback {
    onFrame(frameTimeInNano: number): void {}
    onIdle(timeLeftInNano: number): void {}
}
export class UIObserver {
}