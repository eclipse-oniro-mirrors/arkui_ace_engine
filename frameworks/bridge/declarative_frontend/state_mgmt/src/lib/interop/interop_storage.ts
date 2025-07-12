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

/**
 * Interop AppStorage with ArkTS1.2
 * 1) Each AppStorage stores the value in each map by API call;
 * 2) ArkTS1.1 AppStorage will provide getValue, removeValue, clear for ArkTS1.2, in addition will call
 * key(add/remove/clear) function provided by ArkTS1.2 to speed up key search in ArkTS1.2;
 * 3) ArkTS1.2 AppStorage will provide getValue, removeValue, clear, getSize for ArkTS1.1, avoid to slow down
 * the set operation in ArkTS1.2, it will no key speed up for ArkTS1.1
 * 4) getValue function provided by ArkTS1.1 will return ESValue(ArkTS1.1 ObservedPropertyPU), ArkTS1.2 need to create
 * static StorageProperty and storage in interopStorage to speed up.
 * 5) getValue function provided by ArkTS1.2 will return ESValue(ArkTS1.1 ObservedPropertyPU), ArkTS1.2 need to create
 * dynamic ObservedPropertyPU and storage in origin static StorageProperty.
 * 6) Static StorageProperty and Dynamic ObservedPropertyPU will store same raw value and will bind each other to
 * notify value change event.
 */
class InteropStorage extends Map<string, ObservedPropertyAbstract<any>> {
    originStorage_: Map<string, ObservedPropertyAbstract<any>> = new Map<string, ObservedPropertyAbstract<any>>();

    totalKeys_ = new Map<string, string>();

    // get value from AppStorage in ArkTS1.2
    getStaticValue_: (value: string) => ObservedPropertyAbstract<any> = () => {
        throw new Error('not implement');
    };
    removeStaticValue_: (value: string) => boolean = () => {
        throw new Error('not implement');
    };
    getStaticValueSize_: () => number = () => {
        throw new Error('not implement');
    };
    getStaticTotalKeys_: () => Set<string> = () => {
        throw new Error('not implement');
    };

    // call ArkTS1.2 to update interop key map.
    addKeyFunc_: (key: string) => void = (key: string) => {
        throw new Error('not implement');
    };
    removeKeyFunc_: (key: string) => void = (key: string) => {
        throw new Error('not implement');
    };
    clearKeyFunc_: () => void = () => {
        throw new Error('not implement');
    };

    constructor() {
        super();
    }

    clear(): void {
        this.originStorage_.clear();
        // clear key func will also clear ArkTS1.2 storage.
        this.clearKeyFunc_();
        return;
    }

    delete(key: string): boolean {
        const value = this.originStorage_.delete(key);
        if (value) {
            // update ArkTS1.2 cached map.
            this.removeKeyFunc_(key);
            return true;
        }
        // remove ArkTS1.2 stroage.
        return this.removeStaticValue_(key);
    }

    get(key: string): ObservedPropertyAbstract<any> | undefined {
        const value = this.originStorage_.get(key);
        if (value != undefined) {
            return value;
        }
        // Check ArkTS1.2 storage
        return this.getStaticValue_(key);
    }

    has(key: string): boolean {
        const result = this.originStorage_.has(key);
        if (result) {
            return result;
        }
        let interopValue = this.getStaticValue_(key);
        if (interopValue) {
            return true;
        }
        return false;
    }

    // This will call after has or get check.
    // When it call, ArkTS1.2 has no this key value.
    set(key: string, value: ObservedPropertyAbstract<any>): this {
        this.originStorage_.set(key, value);
        // update ArkTS1.2 cached key.
        this.addKeyFunc_(key);
        return this;
    }

    get size(): number {
        return this.originStorage_.size + this.getStaticValueSize_();
    }

    keys(): IterableIterator<string> {
        this.totalKeys_.clear();
        const staticKeys = this.getStaticTotalKeys_();
        if (staticKeys.size == 0) {
            return this.originStorage_.keys();
        }
        staticKeys.forEach((value: string) => {
            this.totalKeys_.set(value, value);
        });
        this.originStorage_.forEach((value: ObservedPropertyAbstract<any>, key: string) => {
            this.totalKeys_.set(key, key);
        });
        return this.totalKeys_.keys();
    }
}

function bindStaticAppStorage(
    getStaticValue: (value: string) => ObservedPropertyAbstract<any>,
    removeStaticValue: (value: string) => boolean,
    getStaticValueSize: () => number,
    getStaticTotalKeys: () => Set<string>,

    // call ArkTS1.2 to update interop key map.
    addKeyFunc: (key: string) => void,
    removeKeyFunc: (key: string) => void,
    clearKeyFunc: () => void,

    // set callback to ArkTS1.2
    setGetValueFunc: (event: (value: string) => ObservedPropertyAbstract<any> | undefined) => void,
    setRemoveValueFunc: (event: (value: string) => boolean) => void,
    setClearValueFunc: (event: () => boolean) => void
) {
    const appStorage = AppStorage._getOrCreateByInterop_();

    // use interop storage replace origin map.
    const interopStorage = new InteropStorage();
    interopStorage.originStorage_ = appStorage._getOriginStorageByInterop_();
    appStorage._setOriginStorageByInterop_(interopStorage);

    // update ArkTS1.2 key cache.
    interopStorage.originStorage_.forEach((value: ObservedPropertyAbstractPU<any>, key: string) => {
        addKeyFunc(key);
    });

    // bind static storage.
    interopStorage.getStaticValue_ = getStaticValue;
    interopStorage.removeStaticValue_ = removeStaticValue;
    interopStorage.getStaticValueSize_ = getStaticValueSize;
    interopStorage.getStaticTotalKeys_ = getStaticTotalKeys;
    interopStorage.addKeyFunc_ = addKeyFunc;
    interopStorage.removeKeyFunc_ = removeKeyFunc;
    interopStorage.clearKeyFunc_ = clearKeyFunc;

    setGetValueFunc((value: string) => {
        return interopStorage.originStorage_.get(value);
    });
    setRemoveValueFunc((value: string) => {
        let state: ObservedPropertyAbstract<any> | undefined = interopStorage.originStorage_.get(value);
        if (state) {
            if (state.numberOfSubscrbers() > 0) {
                return false;
            }
            state.aboutToBeDeleted();
            interopStorage.originStorage_.delete(value);
            return true;
        }
        return false;
    });
    setClearValueFunc(() => {
        const storage = interopStorage.originStorage_;
        for (let propName of storage.keys()) {
            let state: ObservedPropertyAbstract<any> = storage.get(propName);
            if (state.numberOfSubscrbers() > 0) {
                return false;
            }
        }
        for (let propName of storage.keys()) {
            let state: ObservedPropertyAbstract<any> = storage.get(propName);
            state.aboutToBeDeleted();
        }
        storage.clear();
        return true;
    });
}
