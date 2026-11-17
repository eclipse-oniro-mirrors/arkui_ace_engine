/*
 * Copyright (C) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { Position, SizeOptions, Length, ResourceColor }  from './arkui.component.units';

export interface ShapeSize {

    width?: double | string;

    height?: double | string;
}

export interface RectShapeOptions extends ShapeSize {

    radius?: double | string | Array<double | string>;
}

export interface RoundRectShapeOptions extends ShapeSize {

    radiusWidth?: double | string;

    radiusHeight?: double | string;
}

export interface PathShapeOptions {

    commands?: string;
}

export class CommonShapeMethod {
    static { loadLibrary("shape_ani") }

    public native offset(offset: Position): this;

    public native fill(color: ResourceColor): this;

    public native position(position: Position): this;
}

export class BaseShape extends CommonShapeMethod {

   public native width(width: Length): this;

   public native height(height: Length): this;

   public native size(size: SizeOptions): this;
}

export class RectShapeCleaner {
    private ptr: long = 0;
    constructor(ptr: long) { this.ptr = ptr; }
    clean(): void {
        if (this.ptr != 0) {
            RectShape.nativeDestroy(this.ptr);
            this.ptr = 0;
        }
    }
}

export class EllipseShapeCleaner {
    private ptr: long = 0;
    constructor(ptr: long) { this.ptr = ptr; }
    clean(): void {
        if (this.ptr != 0) {
            EllipseShape.nativeDestroy(this.ptr);
            this.ptr = 0;
        }
    }
}

export class CircleShapeCleaner {
    private ptr: long = 0;
    constructor(ptr: long) { this.ptr = ptr; }
    clean(): void {
        if (this.ptr != 0) {
            CircleShape.nativeDestroy(this.ptr);
            this.ptr = 0;
        }
    }
}

export class PathShapeCleaner {
    private ptr: long = 0;
    constructor(ptr: long) { this.ptr = ptr; }
    clean(): void {
        if (this.ptr != 0) {
            PathShape.nativeDestroy(this.ptr);
            this.ptr = 0;
        }
    }
}

export class RectShape extends BaseShape {
    basicShapeResult: long = 0;
    protected cleaner_: RectShapeCleaner | undefined = undefined;
    private static destroyRegister: FinalizationRegistry<RectShapeCleaner> =
        new FinalizationRegistry<RectShapeCleaner>(RectShape.cleanCallback);
    private static unregisterToken: object = new object();

    static native create(options?: RectShapeOptions | RoundRectShapeOptions): long;
    static native nativeDestroy(ptr: long): void;

    static cleanCallback(cleaner: RectShapeCleaner): void {
        cleaner.clean();
    }

    constructor(options?: RectShapeOptions | RoundRectShapeOptions) {
        super();
        if (this.basicShapeResult == 0) {
            let ptr = RectShape.create(options);
            this.basicShapeResult = ptr;
            this.registerCleaner(ptr);
        }
    }

    registerCleaner(ptr: long): void {
        if (this.cleaner_ !== undefined) {
            this.cleaner_!.clean();
        }
        this.cleaner_ = new RectShapeCleaner(ptr);
        RectShape.destroyRegister.register(this, this.cleaner_!, RectShape.unregisterToken);
    }
    unregisterCleaner(): void {
        RectShape.destroyRegister.unregister(RectShape.unregisterToken);
    }

    public native radiusWidth(rWidth: double | string): this;

    public native radiusHeight(rHeight: double | string): this;

    public native radius(radius: double | string | Array<double | string>): this;

    public native width(width: Length): this;

    public native height(height: Length): this;

    public native size(size: SizeOptions): this;

    public native offset(offset: Position): this;

    public native fill(color: ResourceColor): this;

    public native position(position: Position): this;
}

export class EllipseShape  extends BaseShape {
    basicShapeResult: long = 0;
    protected cleaner_: EllipseShapeCleaner | undefined = undefined;
    private static destroyRegister: FinalizationRegistry<EllipseShapeCleaner> =
        new FinalizationRegistry<EllipseShapeCleaner>(EllipseShape.cleanCallback);
    private static unregisterToken: object = new object();

    static native create(options?: ShapeSize): long;
    static native nativeDestroy(ptr: long): void;

    static cleanCallback(cleaner: EllipseShapeCleaner): void {
        cleaner.clean();
    }

    constructor(options?: ShapeSize) {
        super();
        if (this.basicShapeResult == 0) {
            let ptr = EllipseShape.create(options);
            this.basicShapeResult = ptr;
            this.registerCleaner(ptr);
        }
    }

    registerCleaner(ptr: long): void {
        if (this.cleaner_ !== undefined) {
            this.cleaner_!.clean();
        }
        this.cleaner_ = new EllipseShapeCleaner(ptr);
        EllipseShape.destroyRegister.register(this, this.cleaner_!, EllipseShape.unregisterToken);
    }
    unregisterCleaner(): void {
        EllipseShape.destroyRegister.unregister(EllipseShape.unregisterToken);
    }

    public native width(width: Length): this;

    public native height(height: Length): this;

    public native size(size: SizeOptions): this;

    public native offset(offset: Position): this;

    public native fill(color: ResourceColor): this;

    public native position(position: Position): this;
}

export class CircleShape extends BaseShape {
    basicShapeResult: long = 0;
    protected cleaner_: CircleShapeCleaner | undefined = undefined;
    private static destroyRegister: FinalizationRegistry<CircleShapeCleaner> =
        new FinalizationRegistry<CircleShapeCleaner>(CircleShape.cleanCallback);
    private static unregisterToken: object = new object();

    static native create(options?: ShapeSize): long;
    static native nativeDestroy(ptr: long): void;

    static cleanCallback(cleaner: CircleShapeCleaner): void {
        cleaner.clean();
    }

    constructor(options?: ShapeSize) {
        super();
        if (this.basicShapeResult == 0) {
            let ptr = CircleShape.create(options);
            this.basicShapeResult = ptr;
            this.registerCleaner(ptr);
        }
    }

    registerCleaner(ptr: long): void {
        if (this.cleaner_ !== undefined) {
            this.cleaner_!.clean();
        }
        this.cleaner_ = new CircleShapeCleaner(ptr);
        CircleShape.destroyRegister.register(this, this.cleaner_!, CircleShape.unregisterToken);
    }
    unregisterCleaner(): void {
        CircleShape.destroyRegister.unregister(CircleShape.unregisterToken);
    }

    public native width(width: Length): this;

    public native height(height: Length): this;

    public native size(size: SizeOptions): this;

    public native offset(offset: Position): this;

    public native fill(color: ResourceColor): this;

    public native position(position: Position): this;
}

export class PathShape extends CommonShapeMethod {
    basicShapeResult: long = 0;
    protected cleaner_: PathShapeCleaner | undefined = undefined;
    private static destroyRegister: FinalizationRegistry<PathShapeCleaner> =
        new FinalizationRegistry<PathShapeCleaner>(PathShape.cleanCallback);
    private static unregisterToken: object = new object();

    static native create(options?: PathShapeOptions): long;
    static native nativeDestroy(ptr: long): void;

    static cleanCallback(cleaner: PathShapeCleaner): void {
        cleaner.clean();
    }

    constructor(options?: PathShapeOptions) {
        super();
        if (this.basicShapeResult == 0) {
            let ptr = PathShape.create(options);
            this.basicShapeResult = ptr;
            this.registerCleaner(ptr);
        }
    }

    registerCleaner(ptr: long): void {
        if (this.cleaner_ !== undefined) {
            this.cleaner_!.clean();
        }
        this.cleaner_ = new PathShapeCleaner(ptr);
        PathShape.destroyRegister.register(this, this.cleaner_!, PathShape.unregisterToken);
    }
    unregisterCleaner(): void {
        PathShape.destroyRegister.unregister(PathShape.unregisterToken);
    }

    public native commands(commands: string): this;

    public native offset(offset: Position): this;

    public native fill(color: ResourceColor): this;

    public native position(position: Position): this;
}

export class __ShapeInnerMethods__ {
    static { loadLibrary("shape_ani") }
    public static native CircleFromPtr(ptr: long): CircleShape;
    public static native RectFromPtr(ptr: long): RectShape;
    public static native EllipseFromPtr(ptr: long): EllipseShape;
    public static native PathFromPtr(ptr: long): PathShape;
}
