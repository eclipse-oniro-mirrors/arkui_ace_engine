/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import { int32, int64, float32, unsafeCast } from '@koalaui/common'
import { KInt, KPointer, KBoolean, NativeBuffer, KStringPtr, resourceFinalizerRegister, SerializerBase, DeserializerBase, CallbackResource, InteropNativeModule, MaterializedBase, Tags, RuntimeType, toPeerPtr, nullptr, KSerializerBuffer, KUint8ArrayPtr } from '@koalaui/interop'
import { memo, memo_stable } from 'arkui.stateManagement.runtime'
import { ComponentBuilder, Builder } from '@koalaui/builderLambda'
import { CallbackTransformer } from './../CallbackTransformer'
export type ScrollSnapApproachCallback = (velocity: double, decayOffset: double) => double;
export type ScrollSnapTargetCallback = (velocity: double) => double;
export interface ScrollSnapOffsetProviderProxy {
    calculateApproachOffset: ScrollSnapApproachCallback;
    calculateSnapOffset: ScrollSnapTargetCallback;
}
export class ScrollSnapOffsetProviderProxy_serializer {
    public static write(buffer: SerializerBase, value: ScrollSnapOffsetProviderProxy): void {
        let valueSerializer: SerializerBase = buffer
        const valueHolderForCalculateApproachOffset = value.calculateApproachOffset
        valueSerializer.holdAndWriteCallback(valueHolderForCalculateApproachOffset)
        const valueHolderForCalculateSnapOffset = value.calculateSnapOffset
        valueSerializer.holdAndWriteCallback(valueHolderForCalculateSnapOffset)
    }
    public static read(buffer: DeserializerBase): ScrollSnapOffsetProviderProxy {
        let valueDeserializer: DeserializerBase = buffer
        const calculateApproachOffsetTmpBufBufResource: CallbackResource = valueDeserializer.readCallbackResource()
        const calculateApproachOffsetTmpBufBufCall: KPointer = valueDeserializer.readPointer()
        const calculateApproachOffsetTmpBufBufCallSync: KPointer = valueDeserializer.readPointer()
        const calculateApproachOffsetTmpBufClosure = (velocity: double, decayOffset: double):double => {
            const calculateApproachOffsetTmpBufBufArgsSerializer: SerializerBase = SerializerBase.hold();
            calculateApproachOffsetTmpBufBufArgsSerializer.writeInt32(calculateApproachOffsetTmpBufBufResource.resourceId);
            calculateApproachOffsetTmpBufBufArgsSerializer.writePointer(calculateApproachOffsetTmpBufBufCall);
            calculateApproachOffsetTmpBufBufArgsSerializer.writePointer(calculateApproachOffsetTmpBufBufCallSync);
            calculateApproachOffsetTmpBufBufArgsSerializer.writeFloat64(velocity);
            calculateApproachOffsetTmpBufBufArgsSerializer.writeFloat64(decayOffset);
            let calculateApproachOffsetTmpBufBufContinuationValue: double | undefined;
            const calculateApproachOffsetTmpBufBufContinuationCallback: ((progress: double) => void) = (value: double):void => {
            calculateApproachOffsetTmpBufBufContinuationValue = value;
        }
            calculateApproachOffsetTmpBufBufArgsSerializer.holdAndWriteCallback(calculateApproachOffsetTmpBufBufContinuationCallback);
            InteropNativeModule._CallCallbackSync(10, -1790733684, calculateApproachOffsetTmpBufBufArgsSerializer.asBuffer(), calculateApproachOffsetTmpBufBufArgsSerializer.length());
            calculateApproachOffsetTmpBufBufArgsSerializer.release();
            return calculateApproachOffsetTmpBufBufContinuationValue!.toDouble();
        }
        resourceFinalizerRegister(calculateApproachOffsetTmpBufClosure, calculateApproachOffsetTmpBufBufResource)
        const calculateApproachOffsetTmpResult: ScrollSnapApproachCallback = calculateApproachOffsetTmpBufClosure
        const calculateSnapOffsetTmpBufBufResource: CallbackResource = valueDeserializer.readCallbackResource()
        const calculateSnapOffsetTmpBufBufCall: KPointer = valueDeserializer.readPointer()
        const calculateSnapOffsetTmpBufBufCallSync: KPointer = valueDeserializer.readPointer()
        const calculateSnapOffsetTmpBufClosure = (velocity: double):double => {
            const calculateSnapOffsetTmpBufBufArgsSerializer: SerializerBase = SerializerBase.hold();
            calculateSnapOffsetTmpBufBufArgsSerializer.writeInt32(calculateSnapOffsetTmpBufBufResource.resourceId);
            calculateSnapOffsetTmpBufBufArgsSerializer.writePointer(calculateSnapOffsetTmpBufBufCall);
            calculateSnapOffsetTmpBufBufArgsSerializer.writePointer(calculateSnapOffsetTmpBufBufCallSync);
            calculateSnapOffsetTmpBufBufArgsSerializer.writeFloat64(velocity);
            let calculateSnapOffsetTmpBufBufContinuationValue: double | undefined;
            const calculateSnapOffsetTmpBufBufContinuationCallback: ((progress: double) => void) = (value: double):void => {
            calculateSnapOffsetTmpBufBufContinuationValue = value;
        }
            calculateSnapOffsetTmpBufBufArgsSerializer.holdAndWriteCallback(calculateSnapOffsetTmpBufBufContinuationCallback);
            InteropNativeModule._CallCallbackSync(10, 1963518795, calculateSnapOffsetTmpBufBufArgsSerializer.asBuffer(), calculateSnapOffsetTmpBufBufArgsSerializer.length());
            calculateSnapOffsetTmpBufBufArgsSerializer.release();
            return calculateSnapOffsetTmpBufBufContinuationValue!.toDouble();
        }
        resourceFinalizerRegister(calculateSnapOffsetTmpBufClosure, calculateSnapOffsetTmpBufBufResource)
        const calculateSnapOffsetTmpResult: ScrollSnapTargetCallback = calculateSnapOffsetTmpBufClosure
        let value: ScrollSnapOffsetProviderProxy = ({calculateApproachOffset: calculateApproachOffsetTmpResult, calculateSnapOffset: calculateSnapOffsetTmpResult} as ScrollSnapOffsetProviderProxy)
        return value
    }
}
