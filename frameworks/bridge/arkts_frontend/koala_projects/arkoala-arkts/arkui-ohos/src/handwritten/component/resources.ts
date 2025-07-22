import { Resource } from "global.resource";
import { int32 } from "@koalaui/common";
import { ArkUIGeneratedNativeModule } from "#components";
import { Serializer } from "./peers/Serializer";
import { asArray } from "@koalaui/common";
import { InteropNativeModule } from "@koalaui/interop";

enum ResourceType {
    COLOR = 10001,
    FLOAT,
    STRING,
    PLURAL,
    BOOLEAN,
    INTARRAY,
    INTEGER,
    PATTERN,
    STRARRAY,
    MEDIA = 20000,
    RAWFILE = 30000,
    SYMBOL = 40000
}

const re = new RegExp("^\\[\\S+?]");

class ArkResource implements Resource {
    bundleName: string = "";
    moduleName: string = "";
    params?: Array<Object | undefined> | undefined;
    type?: int32 | undefined;
    _id: long = -1;

    castParams(params: Object[]): Array<Object | undefined> {
        let result: Array<Object | undefined> = new Array<Object | undefined>();
        for (let param of params) {
            result.push(param);
        }
        return result;
    }

    constructor(resourceName: string | null, bundleName: string, moduleName: string, ...params: Object[]) {
        this.bundleName = bundleName;
        this.moduleName = moduleName;
        if (resourceName !== null) {
            let param1 = new Array<Object | undefined>();
            param1.push(resourceName);
            this.params = param1.concat(this.castParams(params));
        } else {
            this.params = this.castParams(params);
        }
        this._id = -1;
        if (this.params!.length > 0) {
            const name: string = this.params![0] as string;
            this.type = this.parseResourceType(name);
        } else {
            InteropNativeModule._NativeLog("UI-Plugin do not send resourceName when id is -1");
        }
    }
    constructor(id: long, type: int32, bundleName: string, moduleName: string, ...params: Object[]) {
        this._id = id;
        this.type = type;
        this.params = this.castParams(params);
        this.bundleName = bundleName;
        this.moduleName = moduleName;
    }
    set id(value: long) {
        this._id = value;
    }
    get id(): long {
        if (this._id == -1) {
            if (this.params == null || this.params == undefined) {
                return this._id;
            }
            let param = this.params;
            let bundleNamea = this.bundleName;
            let moduleNamea = this.moduleName;
            const thisSerializer: Serializer = Serializer.hold();
            if (param == null) {
                thisSerializer.release();
                return this._id;
            }
            thisSerializer.writeInt32(param.length as int32);
            for (let i = 0; i < param.length; i++) {
                const param_value = String(param[i]).replace(re, "app"); // Process [hsp].type.name to app.type.name
                thisSerializer.writeString(param_value);
            }
            const retval = ArkUIGeneratedNativeModule._SystemOps_getResourceId(bundleNamea, moduleNamea, thisSerializer.asBuffer(), thisSerializer.length());
            thisSerializer.release();
            this._id = retval;
            if (this.id > -1 && this.params!.length > 0) {
                this.params!.shift();
            }
        }
        return this._id;
    }
    parseResourceType(resourceName: string): ResourceType {
        const typeName: string = resourceName.split(".")[1];
        switch (typeName) {
            case 'color':
                return ResourceType.COLOR;
            case 'float':
                return ResourceType.FLOAT;
            case 'string':
                return ResourceType.STRING;
            case 'plural':
                return ResourceType.PLURAL;
            case 'boolean':
                return ResourceType.BOOLEAN;
            case 'intarray':
                return ResourceType.INTARRAY
            case 'integer':
                return ResourceType.INTEGER;
            case 'pattern':
                return ResourceType.PATTERN;
            case 'strarray':
                return ResourceType.STRARRAY
            case 'media':
                return ResourceType.MEDIA;
            case 'RAWFILE':
                return ResourceType.RAWFILE;
            case 'symbol':
                return ResourceType.SYMBOL;
        }
        return ResourceType.STRING;
    }
}
export function _r(bundleName: string, moduleName: string, name: string, ...params: Object[]): Resource {
    return new ArkResource(name, bundleName, moduleName, ...params)
}
export function _rawfile(bundleName: string, moduleName: string, name: string): Resource {
    return new ArkResource(0, 30000, bundleName, moduleName, name);
}
export function _r(id: number, type: number, bundleName: string, moduleName: string, ...params: Object[]): Resource {
    if (id === -1) {
        return new ArkResource(null, bundleName, moduleName, ...params);
    }
    return new ArkResource(id as long, type as int32, bundleName, moduleName, ...params);
}
export function _rawfile(id: number, type: number, bundleName: string, moduleName: string, ...params: Object[]): Resource {
    const name: string = params[0] as string;
    return _rawfile(bundleName, moduleName, name);
}
