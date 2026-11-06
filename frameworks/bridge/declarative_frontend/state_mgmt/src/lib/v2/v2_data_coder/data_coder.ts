/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 *
 */

function nullOrUndef(value: unknown): boolean {
  return value === null || value === undefined;
}

// Read-only skip policy for stringify/restoreTo:
// - boolean: legacy getter-only semantics (skip props with a getter and no setter anywhere
//   on the prototype chain) — PersistenceV2 opt-in behavior, unchanged;
// - predicate: caller-defined read-only detection, e.g. RecoverableStorage also skips
//   @Computed accessors and non-writable data properties (ADR-14).
type ReadOnlyPropertyFilter = boolean | ((target: object, prop: string) => boolean);

class DataCoder {
  // Tag to detect payload format
  public static readonly FORMAT_TAG = 'JSON2';

  // Track visited target objects to prevent recursion in restoreObject()
  private static visitedTargets_ = new Set<object>();
  // Track visited source objects to restore shared references
  private static visitedSources_ = new Map<object, object>();

  /**
   * Serialize an object to a JSON2
   */
  public static stringify<T>(value: T, forceLegacyFormat: boolean = false, ignoreReadOnlyProperties: ReadOnlyPropertyFilter = false): string {
    if (forceLegacyFormat) {
      return JSONCoder.stringify(value);
    }

    const origValue = ObserveV2.IsMakeObserved(value)
      ? UIUtilsImpl.instance().getTarget(value)
      : value;

    const result = this.FORMAT_TAG + JSON2.stringify(origValue, ignoreReadOnlyProperties);

    if (ObserveV2.IsMakeObserved(value)) {
      DataCoder.touchAll(value as unknown as object);
    }

    return result;
  }

  /**
   * Parse a JSON2/JSON string to an object
   */
  public static parse<T extends object>(text: string): T | T[] {
    return text.startsWith(this.FORMAT_TAG)
      ? JSON2.parse(text.slice(this.FORMAT_TAG.length))
      : JSON.parse(text); // fallback to legacy format;
  }

  static isReadOnlyGetter(target: object, prop: string): boolean {
    let hasGetter = false;
    let hasSetter = false;
    let current: object | null = target;
    while (current !== null && current !== undefined) {
      const desc = Object.getOwnPropertyDescriptor(current, prop);
      if (desc) {
        if (typeof desc.get === 'function') {
          hasGetter = true;
        }
        if (typeof desc.set === 'function') {
          hasSetter = true;
        }
        if (hasGetter || hasSetter) {
          break;
        }
      }
      current = Object.getPrototypeOf(current);
    }
    return hasGetter && !hasSetter;
  }

  // Resolve the read-only skip decision for one property. A boolean keeps the legacy
  // getter-only semantics; a predicate delegates read-only detection to the caller.
  static shouldSkipReadOnlyProperty(filter: ReadOnlyPropertyFilter, target: object, prop: string): boolean {
    if (filter === false || filter === undefined) {
      return false;
    }
    if (typeof filter === 'function') {
      return filter(target, prop);
    }
    return DataCoder.isReadOnlyGetter(target, prop);
  }

  /**
   * Restore the state of 'source' object into 'target' object(s), restore aliased prop names
   */
  public static restoreTo<T extends object, S extends object>(
    target: T | T[], source: T | T[], defaultSubCreator?: StorageDefaultCreator<S>,
    ignoreReadOnlyProperties: ReadOnlyPropertyFilter = false
  ): T | T[] {
    const origTarget = ObserveV2.IsMakeObserved(target)
      ? UIUtilsImpl.instance().getTarget(target)
      : target;

    let factory: FactoryConstructor<S> | undefined = undefined;

    if (defaultSubCreator) {
      factory = (_: object): TypeConstructor<S> => {
        return class {
          constructor() {
            return defaultSubCreator() ?? DataCoder.throwInvalidSubCreatorResult();
          }
        } as TypeConstructor<S>;
      };
    }

    try {
      if (!nullOrUndef(source) && globalThis.isSendable(source)) {
        this.throwIfNotSendable(origTarget);
        // The root is Sendable; only properties are restored
        this.restoreObject(origTarget, source, { factory }, ignoreReadOnlyProperties);
      } else {
        const dst = { root: origTarget };
        const src = { root: source };
        this.restorePropValue(dst, 'root', src, 'root', { factory }, ignoreReadOnlyProperties);
      }
    } finally {
      this.visitedTargets_.clear();
      this.visitedSources_.clear();
    }

    // Regular or @ObservedV2 target, return as-is
    if (!ObserveV2.IsMakeObserved(target)) {
      return target;
    }

    return DataCoder.touchAll(target);
  }

  // Recursively touch all properties of an object to record dependencies
  private static touchAll<T>(target: T, visited: Set<object> = new Set()): T {
    if (typeof target !== 'object' || target === null || visited.has(target)) {
      return target;
    }

    visited.add(target);

    if ([Array, Map, Set, SendableArray, SendableMap, SendableSet].some(clazz => target instanceof clazz)) {
      (target as unknown as { forEach: Function }).forEach?.((value: unknown) => {
        DataCoder.touchAll(value, visited);
      });
      return target;
    }

    if (target instanceof Date) {
      target.getTime();
      return target;
    }

    for (const key in target) {
      if (Object.prototype.hasOwnProperty.call(target, key)) {
        DataCoder.touchAll((target as Record<string, unknown>)[key], visited);
      }
    }

    return target;
  }

  // Recursively restore object 'source' into 'target' considering TransformOptions
  // opts = { alias?, factory?, disabled? }
  private static restoreObject(target: any, source: any, opts: TransformOptions<any>,
    ignoreReadOnlyProperties: ReadOnlyPropertyFilter = false): void {
    if (!this.trackVisited_(target, source)) {
      return;
    }
    if (source instanceof Array) {
      this.restoreArray_(target, source, opts, ignoreReadOnlyProperties);
    } else if (source instanceof Set) {
      this.restoreSet_(target, source, opts, ignoreReadOnlyProperties);
    } else if (source instanceof Map) {
      this.restoreMap_(target, source, opts, ignoreReadOnlyProperties);
    } else if (source instanceof SendableArray) {
      this.restoreSendableArray_(target, source, opts, ignoreReadOnlyProperties);
    } else if (source instanceof SendableMap) {
      this.restoreSendableMap_(target, source, opts, ignoreReadOnlyProperties);
    } else if (source instanceof SendableSet) {
      this.restoreSendableSet_(target, source, opts, ignoreReadOnlyProperties);
    } else if (source && typeof source === 'object') {
      this.restorePlainProperties_(target, source, ignoreReadOnlyProperties);
    }
  }

  // Track visited targets to prevent recursion; returns false when target was already visited.
  private static trackVisited_(target: unknown, source: unknown): boolean {
    if (typeof target === 'object' && target != null) {
      if (this.visitedTargets_.has(target)) {
        return false;
      }
      this.visitedTargets_.add(target);
    }
    if (typeof source === 'object' && source != null) {
      if (!this.visitedSources_.has(source)) {
        this.visitedSources_.set(source, target as object);
      }
    }
    return true;
  }

  // Clone or copy an array/map/set item value (factory rebuild when present).
  private static cloneOrCopy_(value: unknown, opts: TransformOptions<unknown>,
    ignoreReadOnlyProperties: ReadOnlyPropertyFilter): unknown {
    if (typeof value !== 'object') {
      return value;
    }
    if (this.visitedSources_.has(value)) {
      return this.visitedSources_.get(value);
    }
    if (opts?.factory === undefined) {
      this.visitedSources_.set(value, value);
      return value;
    }
    const clazz = opts.factory(value);
    const newValue = clazz ? new clazz() : value;
    // we don't allow collections directly nested inside other collections
    this.throwIfCollection(newValue);
    this.restoreObject(newValue, value, opts as TransformOptions<any>, ignoreReadOnlyProperties);
    return newValue;
  }

  private static restoreArray_(target: unknown[], source: unknown[], opts: TransformOptions<unknown>,
    ignoreReadOnlyProperties: ReadOnlyPropertyFilter): void {
    this.throwIfNotInstanceOf(target, Array);
    target.length = source.length;
    source.forEach((item, idx) => {
      target[idx] = this.cloneOrCopy_(item, opts, ignoreReadOnlyProperties);
    });
  }

  private static restoreSet_(target: Set<unknown>, source: Set<unknown>, opts: TransformOptions<unknown>,
    ignoreReadOnlyProperties: ReadOnlyPropertyFilter): void {
    this.throwIfNotInstanceOf(target, Set);
    target.clear();
    source.forEach(item => {
      target.add(this.cloneOrCopy_(item, opts, ignoreReadOnlyProperties));
    });
  }

  private static restoreMap_(target: Map<unknown, unknown>, source: Map<unknown, unknown>,
    opts: TransformOptions<unknown>, ignoreReadOnlyProperties: ReadOnlyPropertyFilter): void {
    this.throwIfNotInstanceOf(target, Map);
    target.clear();
    source.forEach((item, key) => {
      target.set(key, this.cloneOrCopy_(item, opts, ignoreReadOnlyProperties));
    });
  }

  private static restoreSendableArray_(target: any, source: SendableArray<any>,
    opts: TransformOptions<any>, ignoreReadOnlyProperties: ReadOnlyPropertyFilter): void {
    this.throwIfNotInstanceOf(target, SendableArray);
    const items = [...source].map(item => this.cloneOrCopy_(item, opts, ignoreReadOnlyProperties));
    target.splice(0, target.length, ...items);
  }

  private static restoreSendableMap_(target: any, source: SendableMap<any, any>,
    opts: TransformOptions<any>, ignoreReadOnlyProperties: ReadOnlyPropertyFilter): void {
    this.throwIfNotInstanceOf(target, SendableMap);
    target.clear();
    for (let [key, item] of source) {
      target.set(key, this.cloneOrCopy_(item, opts, ignoreReadOnlyProperties));
    }
  }

  private static restoreSendableSet_(target: any, source: SendableSet<any>,
    opts: TransformOptions<any>, ignoreReadOnlyProperties: ReadOnlyPropertyFilter): void {
    this.throwIfNotInstanceOf(target, SendableSet);
    target.clear();
    for (let item of source) {
      target.add(this.cloneOrCopy_(item, opts, ignoreReadOnlyProperties));
    }
  }

  // Restore target properties from source (incl. undefined) for plain objects.
  private static restorePlainProperties_(target: Record<string, unknown>, source: Record<string, unknown>,
    ignoreReadOnlyProperties: ReadOnlyPropertyFilter): void {
    // meta = { [prop]: { alias, factory, disabled } }
    const meta = Meta.gets(target) as Record<string, TransformOptions<unknown>>;
    // reversed map of meta = { [alias]: prop }
    const alias2prop = new Map<string, string>(
      Object.entries(meta).map(([prop, propOpts]) => [propOpts.alias ?? prop, prop])
    );
    for (const [key] of Object.entries(source)) {
      const targetProp = alias2prop.get(key) ?? key;
      if (DataCoder.shouldSkipReadOnlyProperty(ignoreReadOnlyProperties, target, targetProp)) {
        continue;
      }
      const sourceProp = meta[targetProp]?.alias ?? targetProp;
      const propOptions = meta[targetProp] || {};
      this.restorePropValue(target, targetProp, source, sourceProp,
        propOptions as TransformOptions<any>, ignoreReadOnlyProperties);
    }
  }

  // Restore an property value from 'source' to 'target' considering TransformOptions
  // opts = { alias, factory, disabled }
  private static restorePropValue(
    target: any, targetProp: string, source: any, sourceProp: string, opts: TransformOptions<any> = {},
    ignoreReadOnlyProperties: ReadOnlyPropertyFilter = false
  ): void {
    if (DataCoder.shouldSkipReadOnlyProperty(ignoreReadOnlyProperties, target, targetProp)) {
      return;
    }
    const srcVal = source[sourceProp];
    const tgtVal = target[targetProp];

    if (opts.disabled) {
      return;
    }

    if (typeof tgtVal === 'function') {
      return;
    }

    if (['string','number','boolean','bigint'].includes(typeof srcVal)) {
      target[targetProp] = srcVal;
      return;
    }

    if (this.visitedSources_.get(srcVal)) {
      target[targetProp] = this.visitedSources_.get(srcVal);
      return;
    }

    // try to restore the Date without replacing the existing instance
    if (srcVal instanceof Date && tgtVal instanceof Date) {
      target[targetProp].setTime(srcVal.getTime())
      return;
    }

    if ([Boolean, Date, Number, String].includes(srcVal?.constructor)) {
      target[targetProp] = srcVal;
      return;
    }

    if ([Array, Map, Set, SendableArray, SendableMap, SendableSet].includes(srcVal?.constructor)) {
      this.restoreObject(tgtVal, srcVal, opts, ignoreReadOnlyProperties);
      return;
    }

    if (!nullOrUndef(srcVal) && globalThis.isSendable(srcVal)) {
      this.throwIfNotSendable(tgtVal);
      target[targetProp] = srcVal;
      return;
    }

    if (nullOrUndef(srcVal) && tgtVal instanceof Array) {
      target[targetProp].length = 0;
      return;
    }

    if (nullOrUndef(srcVal) && tgtVal instanceof Map) {
      target[targetProp].clear();
      return;
    }

    if (nullOrUndef(srcVal) && tgtVal instanceof Set) {
      target[targetProp].clear();
      return;
    }

    if (nullOrUndef(srcVal)) {
      target[targetProp] = srcVal;
      return;
    }

    if (nullOrUndef(tgtVal) && opts.factory) {
      const clazz = opts.factory(srcVal);
      target[targetProp] = clazz ? new clazz() : {};
      this.restoreObject(target[targetProp], srcVal, {}, ignoreReadOnlyProperties);
      return;
    }

    if (tgtVal === undefined) {
      target[targetProp] = srcVal;
      return;
    }

    if (tgtVal === null) {
      this.throwNoFactory(targetProp);
    }

    if (tgtVal.constructor !== srcVal.constructor && opts.factory !== undefined) {
      const clazz = opts.factory(srcVal)
      target[targetProp] = clazz ? new clazz() : {};
      this.restoreObject(target[targetProp], srcVal, {}, ignoreReadOnlyProperties);
      return;
    }

    this.restoreObject(tgtVal, srcVal, opts, ignoreReadOnlyProperties);
  }

  // Ensure target is instanceof the clazz
  private static throwIfNotInstanceOf(target: unknown, clazz: new (...args: unknown[]) => unknown): void {
    if (target instanceof clazz === false) {
      const type = target?.constructor?.name ?? typeof target;
      const msg = `The class of target (${type}) mismatches '${clazz.name}'`;
      throw new BusinessError(PERSISTENCE_V2_MISMATCH_BETWEEN_KEY_AND_TYPE, msg);
    }
  }

  // Ensure target has given type
  private static throwIfNotTypeOf(target: unknown, type: string): void {
    if (typeof target !== type) {
      const msg = `The type of target ('${typeof target}') mismatches '${type}'`;
      throw new BusinessError(PERSISTENCE_V2_MISMATCH_BETWEEN_KEY_AND_TYPE, msg);
    }
  }

  // Ensure target is Sendable
  private static throwIfNotSendable(target: unknown): void {
    if (target != null && globalThis.isSendable(target) === false) {
      const type = target.constructor?.name ?? typeof target;
      const msg = `Not supported type! The target (${type}) is not @Sendable`;
      throw new BusinessError(PERSISTENCE_V2_APPSTORAGE_V2_UNSUPPORTED_TYPE, msg);
    }
  }

  // Ensure value is not collection
  private static throwIfCollection(value: unknown): void {
    if ([Array, Map, Set, SendableArray, SendableMap, SendableSet].some(clazz => value instanceof clazz)) {
      const msg = `Not supported type! Array, Map, Set, or collections.Array/Map/Set cannot be used as collection items`;
      throw new BusinessError(PERSISTENCE_V2_APPSTORAGE_V2_UNSUPPORTED_TYPE, msg);
    }
  }

  private static throwNoFactory<T>(targetProp: string): void {
    const msg = `Miss @Type in object defined, the property name is ${targetProp}`;
    throw new BusinessError(PERSISTENCE_V2_LACK_TYPE, msg);
  }

  // todo: need check error message
  private static throwInvalidSubCreatorResult(): never {
    const msg = `The defaultSubCreator returned invalid value`;
    throw new BusinessError(PERSISTENCE_V2_APPSTORAGE_V2_INVALID_DEFAULT_CREATOR, msg);
  }

}

