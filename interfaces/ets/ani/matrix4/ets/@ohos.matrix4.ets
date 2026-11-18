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

export default namespace matrix4 {
    loadLibrary("matrix4_ani");
    export interface TranslateOption {
        x?: double;
        y?: double;
        z?: double;
    }

    export interface ScaleOption {
        x?: double;
        y?: double;
        z?: double;
        centerX?: double;
        centerY?: double;
    }

    export interface RotateOption {
        x?: double;
        y?: double;
        z?: double;
        centerX?: double;
        centerY?: double;
        angle?: double;
    }

    export interface Point {
        x: double;
        y: double;
    }

    export interface PolyToPolyOptions {
        src: Array<Point>;
        srcIndex?: int;
        dst: Array<Point>;
        dstIndex?: int;
        pointCount?: int;
    }

    export default interface Matrix4Transit {
        copy():Matrix4Transit;
        invert(): Matrix4Transit;
        combine(options: Matrix4Transit): Matrix4Transit;
        scale(options: ScaleOption): Matrix4Transit;
        rotate(options: RotateOption): Matrix4Transit;
        translate(options: TranslateOption): Matrix4Transit;
        skew(x: double, y: double): Matrix4Transit;
        setPolyToPoly(options: PolyToPolyOptions): Matrix4Transit;
        transformPoint(options: [
            double,
            double
        ]): [
            double,
            double
        ];
    }

    class Matrix4Cleaner {
        private ptr: long = 0;
        constructor(ptr: long) { this.ptr = ptr; }
        clean(): void {
            if (this.ptr != 0) {
                Matrix4TransitInner.nativeDestroy(this.ptr);
                this.ptr = 0;
            }
        }
    }
    class Matrix4TransitInner implements Matrix4Transit {
        private matrix4Object: long = 0;
        protected cleaner_: Matrix4Cleaner | undefined = undefined;
        private static destroyRegister: FinalizationRegistry<Matrix4Cleaner> =
            new FinalizationRegistry<Matrix4Cleaner>(Matrix4TransitInner.cleanCallback);
        private static unregisterToken: object = new object();

        static {
            loadLibrary("matrix4_ani");
        }

        static native nativeDestroy(ptr: long): void;

        static cleanCallback(cleaner: Matrix4Cleaner): void {
            cleaner.clean();
        }

        constructor(context: long) {
            this.matrix4Object = context;
            this.registerCleaner(context);
        }
        registerCleaner(ptr: long): void {
            if (this.cleaner_ !== undefined) {
                this.cleaner_!.clean();
            }
            this.cleaner_ = new Matrix4Cleaner(ptr);
            Matrix4TransitInner.destroyRegister.register(this, this.cleaner_!, Matrix4TransitInner.unregisterToken);
        }
        unregisterCleaner(): void {
            Matrix4TransitInner.destroyRegister.unregister(Matrix4TransitInner.unregisterToken);
        }

        static matrixTransferStatic(input: Any): Any {
            let inputVal = ESValue.wrap(input) as ESValue;
            if (inputVal && inputVal.getProperty("matrix4x4")) {
                let matrixTuple = inputVal.getProperty("matrix4x4");
                let arrVal: number[] = new Array<number>
                for (let i = 0; i < 16; i++) {
                    arrVal.push(matrixTuple.getProperty(i).toNumber());
                }
                return Matrix4TransitInner.nativeTransferStatic(arrVal);
            }
            return undefined;
        }

        static matrixTransferDynamic(input: Any): Any {
            let matrixObj = input as Matrix4TransitInner;
            if (matrixObj) {
                let addr = matrixObj.matrix4Object;
                let module = ESValue.load("@ohos.matrix4").getProperty('default');
                let initFunc = module.getProperty("__InitTransfer__")
                if (initFunc) {
                    let initArr = initFunc.invoke(ESObject.wrap(addr));
                    return initArr;
                }
            }
            return undefined;
        }

        private static native nativeTransferStatic(input: Any): Any;

        public native copy():Matrix4Transit;
        public native invert():Matrix4Transit;
        public native combine(options: Matrix4Transit): Matrix4Transit;
        public native scale(options: ScaleOption): Matrix4Transit;
        public native rotate(options: RotateOption): Matrix4Transit;
        public native translate(options: TranslateOption): Matrix4Transit;
        public native skew(x: double, y: double): Matrix4Transit;
        public native setPolyToPoly(options: PolyToPolyOptions): Matrix4Transit;
        public native transformPoint(options: [
                            double,
                            double
                        ]): [
                            double,
                            double
                        ]; 
    }
    export native function identity():Matrix4Transit;
    export native function init(options: [
        double, double, double, double, 
        double, double, double, double,
        double, double, double, double,
        double, double, double, double]):Matrix4Transit;
}