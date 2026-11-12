/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

// RecoverableStorage: kill/abnormal-exit snapshot & restore for declared state variables.
// Snapshot model (proposal FR-3): values are extracted only when the save chain triggers
// (GetContentInfo, ContentInfoType 2/3), never written-through on variable change.
// Design: .codespec/changes/draft-20261022-recoverable-storage/design.md (ADR-1/4/5/6/7).
// ADR-12 crash pre-serialization removed (2026-10-22 Owner decision: periodic cost) — see design R-8 (deferred).

// Error handling policy (Owner final v5, 2026-11-02): connect entry errors THROW
// BusinessError (PersistenceV2 pattern); I/O errors (serialization/deserialization)
// are silent via notifyOnError callback + mandatory log. Error code 140124
// was occupied by REUSABLE_V1_OLD_TOOLCHAIN). Budget 200KB per instance (400KB is
// shared with system data: router/nav/node info).
const RECOVERABLE_STORAGE_UNSUPPORTED_TYPE: number = 140124;

type RecoverableStorageErrorReason =
  'serialization' | 'deserialization';

// Payload delivered via notifyOnError (spec F-E3).
interface RecoverableStorageError {
  reason: RecoverableStorageErrorReason;
  keys: Array<string>;
  message: string;
}

type NotifyOnErrorCallback = (error: RecoverableStorageError) => void;

// connect options bag (ADR-9): single-object parameter integrating all connect inputs.
// Field separation removes the key-or-creator overload ambiguity of the positional form.
interface RecoverableConnectOptions<T extends object, S extends object> {
  // (Mandatory) The custom component instance, pass `this` — provides the window-level
  // instanceId used for per-instance partitioning (R-7/R-8).
  component: RecoverableComponent;
  // (Mandatory) Object type constructor (T extends object; simple types are rejected).
  type: TypeConstructorWithArgs<T>;
  // (Optional) Storage key; defaults to the type constructor name.
  key?: string;
  // (Mandatory) Default creator, used on first connect and as restore fallback.
  defaultCreator: StorageDefaultCreator<T>;
  // (Optional) Default sub creator for nested @Type members (collection rebuild).
  defaultSubCreator?: StorageDefaultCreator<S>;
  // (Optional) Serialization/deserialization error callback (UI thread).
  notifyOnError?: NotifyOnErrorCallback;
}

// Structural subset of BaseCustomComponent / PUV2ViewBase used by connect:
// window-level instanceId accessor (V1 ViewPU and V2 views both inherit it,
// see partial_update/pu_view.ts:59 and puv2_common/puv2_view_base.ts:348).
interface RecoverableComponent {
  getInstanceId(): number;
}

interface RecoverableEntry {
  value: object;
  typeName: string;
  notifyOnError?: NotifyOnErrorCallback;
}

class RecoverableStorageImpl extends StorageHelper {
  private static instance_: RecoverableStorageImpl = undefined;
  private static readonly NOT_SUPPORT_TYPE_MESSAGE_: string =
    'Not supported type! The type should have function constructor signature.';
  private static readonly MISSING_COMPONENT_MESSAGE_: string =
    'RecoverableStorage.connect requires the component (pass this) providing getInstanceId().';
  private static readonly MISSING_DEFAULT_CREATOR_MESSAGE_: string =
    'The default creator should be function when first connect.';
  private static readonly NULL_OR_UNDEFINED_KEY_: string = 'The parameter cannot be null or undefined';
  private static readonly DELETE_NOT_EXIST_KEY_: string = 'The key to be deleted does not exist';
  private static readonly INVALID_KEY_TYPE_MESSAGE_: string =
    'The key must be a string when provided.';
  private static readonly INVALID_CALLBACK_MESSAGE_: string =
    'The parameter must be a function when provided.';
  private static readonly INVALID_KEY_MESSAGE_: string =
    'The key is invalid: it cannot be an empty string and its length must be shorter than 1024.';
  // DEFAULT_RECOVERY_MAX_RESTORE_SIZE (spec F-E5): ability_recovery.cpp:44 enforces this on
  // the whole serialized payload; the TS side pre-checks per instance to avoid whole-payload
  // rejection (ADR-7).
  private static readonly SERIALIZATION_BUDGET_BYTES_: number = 150 * 1024;

  // same unsupported-type family as PersistenceV2Impl.NOT_SUPPORT_TYPES_ (v2_persistence.ts:290)
  private static readonly NOT_SUPPORT_TYPES_: Array<Function> =
    [WeakSet, WeakMap, Boolean, Number, String, Symbol, BigInt, RegExp, Function, Promise, ArrayBuffer];

  // STATE-1: per-instance registry, partitioned by window-level instanceId (INV-1/INV-6)
  private instances_: Map<number, Map<string, RecoverableEntry>>;
  // STATE-2: one-shot restored data cache, consumed by connect (INV-4);
  // values are kept as DataCoder-serialized strings, deserialized at consume time
  // where the connect type context is available.
  private restored_: Map<number, Map<string, string>>;
  // Corrupted injected sections (parse failure): flagged so the FIRST connect of the
  // instance can notify ('deserialization') once, then the flag is cleared (INV-4/INV-5).
  private restoredFailures_: Set<number>;

  constructor() {
    super();
    this.instances_ = new Map<number, Map<string, RecoverableEntry>>();
    this.restored_ = new Map<number, Map<string, string>>();
    this.restoredFailures_ = new Set<number>();
  }

  public static instance(): RecoverableStorageImpl {
    if (RecoverableStorageImpl.instance_) {
      return RecoverableStorageImpl.instance_;
    }
    RecoverableStorageImpl.instance_ = new RecoverableStorageImpl();
    return RecoverableStorageImpl.instance_;
  }

  public connect<T extends object, S extends object>(
    options: RecoverableConnectOptions<T, S>
  ): T {
    // Entry errors throw 140124 (Owner final v5): missing/invalid parameters,
    // unsupported types, invalid keys (empty string or length >= 1024).
    const instanceId: number = this.throwIfConnectEntryInvalid(options);
    const key: string = this.getValidatedConnectKey(options.type, options.key);
    const partition: Map<string, RecoverableEntry> = this.getOrCreatePartition(instanceId);
    const existing: T | undefined = this.getExistingValue<T>(partition, key, options.type);
    if (existing !== undefined) {
      return existing;
    }
    return this.resolveNewValue<T, S>(instanceId, key, options.type, partition,
      options.defaultCreator, options.defaultSubCreator, options.notifyOnError);
  }

  // Entry validation for connect: throws 140124 on missing options, unsupported type,
  // invalid fields (ADR-10 hardening) or missing component; returns the window-level
  // instanceId on success.
  private throwIfConnectEntryInvalid<T extends object, S extends object>(
    options: RecoverableConnectOptions<T, S>): number
  {
    if (options === null || options === undefined) {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.MISSING_COMPONENT_MESSAGE_);
    }
    this.throwIfTypeIsNotSupported(options.type);
    this.throwIfConnectOptionsInvalid(options);
    const instanceId: number = this.getInstanceIdFromComponent(options.component);
    if (instanceId === RecoverableStorageImpl.INVALID_INSTANCE_ID) {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.MISSING_COMPONENT_MESSAGE_);
    }
    return instanceId;
  }

  // Resolves and validates the connect key; throws 140124 when the key is an empty
  // string or its length is not less than 1024.
  private getValidatedConnectKey<T extends object>(
    type: TypeConstructorWithArgs<T>, key: string | undefined): string
  {
    const resolved: string | undefined = this.getConnectedKey(type, key);
    if (resolved === undefined || !this.isKeyValid(resolved)) {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.INVALID_KEY_MESSAGE_);
    }
    return resolved;
  }

  // INV-1: same instanceId+key has at most one entry; same type returns the existing
  // reference. Mismatched type throws (never returns a wrong-typed value).
  private getExistingValue<T extends object>(
    partition: Map<string, RecoverableEntry>, key: string,
    type: TypeConstructorWithArgs<T>): T | undefined
  {
    const existing: RecoverableEntry | undefined = partition.get(key);
    if (existing === undefined) {
      return undefined;
    }
    if (this.getTypeName(type) !== existing.typeName) {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        `The type mismatches when use the key '${key}' in storage`);
    }
    return existing.value as T;
  }

  // STATE-2 one-shot consumption: restored value wins over default creator; any failure
  // notifies (AC-12) and falls back to the default-creator path (INV-5).
  private resolveNewValue<T extends object, S extends object>(
    instanceId: number, key: string, type: TypeConstructorWithArgs<T>,
    partition: Map<string, RecoverableEntry>, defaultCreator: StorageDefaultCreator<T>,
    defaultSubCreator: StorageDefaultCreator<S> | undefined,
    notifyOnError: NotifyOnErrorCallback | undefined): T
  {
    // Guard (hardening 2026-11-03): 'defaultSubCreator' is the container element factory
    // (PersistenceV2 CollectionType overload semantics). On a non-container root it corrupts
    // restore (DataCoder.restorePropValue ctor-mismatch branch replaces the root with the
    // factory product, losing the whole restored section) — ignore it with a warning log,
    // never throws.
    let subCreator: StorageDefaultCreator<S> | undefined = defaultSubCreator;
    if (subCreator !== undefined && !RecoverableStorageImpl.isContainerType_(type)) {
      stateMgmtConsole.applicationWarn(`RecoverableStorage: 'defaultSubCreator' is only ` +
        `effective for container types (Array/Map/Set) as the element factory; ` +
        `it is ignored for type '${this.getTypeName(type)}'.`);
      subCreator = undefined;
    }
    let value: T | undefined = this.consumeRestored<T, S>(instanceId, key, type,
      defaultCreator, subCreator, notifyOnError);
    if (value === undefined) {
      if (typeof defaultCreator !== 'function') {
        throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
          RecoverableStorageImpl.MISSING_DEFAULT_CREATOR_MESSAGE_);
      }
      value = defaultCreator(); // creator's own exception propagates (PersistenceV2 pattern)
      this.throwIfValueNotSupported(value);
      if (!(value instanceof type)) {
        throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
          `The type mismatches when use the key '${key}' in storage`);
      }
    }
    partition.set(key, { value, typeName: this.getTypeName(type), notifyOnError: notifyOnError });
    return value;
  }

  // ADR-8: per-instance key query (component-scoped, mirrors connect's instanceId partition).
  public keys(component: RecoverableComponent): Array<string> {
    const instanceId: number = this.getInstanceIdFromComponent(component);
    if (!RecoverableStorageImpl.isValidInstanceId(instanceId)) {
      stateMgmtConsole.applicationError(RecoverableStorageImpl.MISSING_COMPONENT_MESSAGE_);
      return [];
    }
    const partition: Map<string, RecoverableEntry> = this.instances_.get(instanceId);
    return partition ? Array.from(partition.keys()) : [];
  }

  // ADR-8: per-instance key unregistration. Drops the registry entry AND any unconsumed
  // restored data for the key — after remove, snapshot/restore/re-connect no longer
  // involve it. Unknown key warns (AppStorageV2.remove family semantics, :221-234).
  public remove<T>(component: RecoverableComponent, keyOrType: string | TypeConstructorWithArgs<T>): void {
    if (keyOrType === null || keyOrType === undefined) {
      stateMgmtConsole.applicationWarn(RecoverableStorageImpl.NULL_OR_UNDEFINED_KEY_);
      return;
    }
    const instanceId: number = this.getInstanceIdFromComponent(component);
    if (!RecoverableStorageImpl.isValidInstanceId(instanceId)) {
      stateMgmtConsole.applicationError(RecoverableStorageImpl.MISSING_COMPONENT_MESSAGE_);
      return;
    }
    const key: string | undefined = this.getKeyOrTypeName(keyOrType);
    if (key === undefined || !this.isKeyValid(key)) {
      return;
    }
    const partition: Map<string, RecoverableEntry> = this.instances_.get(instanceId);
    const restoredSection: Map<string, string> | undefined = this.restored_.get(instanceId);
    const removedEntry: boolean = partition ? partition.delete(key) : false;
    const removedRestored: boolean = restoredSection ? restoredSection.delete(key) : false;
    if (removedRestored && restoredSection.size === 0) {
      this.restored_.delete(instanceId);
    }
    if (!removedEntry && !removedRestored) {
      stateMgmtConsole.applicationWarn(RecoverableStorageImpl.DELETE_NOT_EXIST_KEY_ + ` key: ${key}`);
    }
  }

  // STATE-3 producer: builds the instance-level snapshot section for the save chain.
  // Byte budget pre-check per ADR-7: entries exceeding the remaining budget are evicted and
  // reported via notifyOnError ('serialization') instead of pushing an oversized section to the
  // serialization layer (ability_recovery.cpp:248 rejects the WHOLE payload on overflow).
  public extractSnapshot(instanceId: number): string {
    const instances: Record<string, Record<string, string>> = {};
    const values: Record<string, string> | undefined = this.buildPartitionValues(instanceId);
    if (values !== undefined) {
      instances[String(instanceId)] = values;
    }
    return JSON.stringify({ version: 1, instances: instances });
  }

  // Shared serialization path (byte budget / over-limit eviction / failure notify).
  // Returns the partition's serialized values, or undefined when nothing registered.
  private buildPartitionValues(instanceId: number): Record<string, string> | undefined {
    if (!RecoverableStorageImpl.isValidInstanceId(instanceId)) {
      return undefined;
    }
    const partition: Map<string, RecoverableEntry> = this.instances_.get(instanceId);
    if (!partition || partition.size === 0) {
      return undefined;
    }
    const values: Record<string, string> = {};
    let budget: number = RecoverableStorageImpl.SERIALIZATION_BUDGET_BYTES_;
    const overLimitKeys: Array<string> = [];
    partition.forEach((entry: RecoverableEntry, key: string) => {
      let serialized: string;
      try {
        // ADR-14: read-only properties (getter-only / @Computed / non-writable) are
        // skipped from the snapshot, silently (no callback — skipping is not an error).
        serialized = DataCoder.stringify(entry.value, false, RecoverableStorageImpl.readOnlyFilter_);
      } catch (err) {
        // INV-5: serialization failure must not crash nor block other entries
        this.notifyErrorFor(partition, key, 'serialization',
          `fail to serialize the key '${key}': ${(err as Error)?.message}`);
        return;
      }
      if (rsUtf8ByteLength(serialized) > budget) {
        overLimitKeys.push(key);
        return;
      }
      budget -= rsUtf8ByteLength(serialized);
      values[key] = serialized;
    });
    if (overLimitKeys.length > 0) {
      this.notifyErrorFor(partition, overLimitKeys[0], 'serialization',
        `serialize budget (${RecoverableStorageImpl.SERIALIZATION_BUDGET_BYTES_} bytes) exceeded, ` +
        `${overLimitKeys.length} key(s) evicted`);
      // notify every evicted key's own callback (payload keys = all evicted keys)
      for (const k of overLimitKeys) {
        this.notifyErrorFor(partition, k, 'serialization',
          `serialize budget exceeded, key '${k}' evicted from snapshot`, overLimitKeys);
      }
    }
    if (Object.keys(values).length === 0) {
      return undefined;
    }
    return values;
  }

  // STATE-2 producer: one-shot injection from the restore chain (TASK-3 wired the delegate
  // Restore path: FrontendDelegateDeclarative::RestoreRouterStack parses the
  // "recoverableStorage" field and delivers it here via __RecoverableStorageHook__).
  public setRestoredData(instanceId: number, json: string): void {
    if (!RecoverableStorageImpl.isValidInstanceId(instanceId) || typeof json !== 'string') {
      return;
    }
    try {
      const parsed = JSON.parse(json) as Record<string, unknown>;
      if (parsed === null || typeof parsed !== 'object' || parsed.version !== 1 ||
        parsed.instances === null || typeof parsed.instances !== 'object') {
        throw new Error('invalid recoverableStorage section structure');
      }
      const instanceKeys: string[] = Object.keys(parsed.instances);
      if (instanceKeys.length === 0) {
        return;
      }
      // The id embedded in the section JSON is the pre-kill original instanceId; the
      // caller passes the consuming window's (reassigned) id — map the section's own
      // single instance key onto the passed id (launch-order realignment).
      const values = parsed.instances[instanceKeys[0]] as Record<string, unknown>;
      if (values === null || typeof values !== 'object') {
        // section valid but nothing for this instance: nothing to inject
        return;
      }
      const section: Map<string, string> = new Map<string, string>();
      for (const key of Object.keys(values)) {
        if (typeof values[key] === 'string') {
          section.set(key, values[key] as string);
        }
      }
      if (section.size > 0) {
        this.restored_.set(instanceId, section);
      }
    } catch (err) {
      // INV-5: corrupted payload must not crash; flag once for the first connect to notify.
      this.restored_.delete(instanceId);
      this.restoredFailures_.add(instanceId);
      stateMgmtConsole.applicationError(`RecoverableStorage restored section for instance ` +
        `${instanceId} is corrupted: ${(err as Error)?.message}`);
    }
  }


  // ADR-5: instance destroy cleanup, mirrors unregisterInstanceIdForEnv (v2_env.ts:302).
  // Called by JsiDeclarativeEngine::JsUnregisterInstanceId on engine destroy.
  public unregisterInstanceIdForRecoverable(instanceId: number): void {
    if (!RecoverableStorageImpl.isValidInstanceId(instanceId)) {
      return;
    }
    try {
      this.instances_.delete(instanceId);
      this.restored_.delete(instanceId);
      this.restoredFailures_.delete(instanceId);
    } catch (err) {
      stateMgmtConsole.applicationError(`RecoverableStorage failed to unregister instance ` +
        `${instanceId}: ${(err as Error)?.message}`);
    }
  }




  // INV-5: deliver an error to the entry's notifyOnError (UI thread, must not throw out).
  private notifyErrorFor(partition: Map<string, RecoverableEntry>, key: string,
    reason: RecoverableStorageErrorReason, message: string, allKeys?: Array<string>): void {
    const entry: RecoverableEntry = partition.get(key);
    if (!entry || typeof entry.notifyOnError !== 'function') {
      stateMgmtConsole.applicationError(`RecoverableStorage key '${key}' error ${reason}: ${message}`);
      return;
    }
    this.safeNotify(entry.notifyOnError, reason, allKeys ? allKeys.slice() : [key], message, key);
  }

  private getOrCreatePartition(instanceId: number): Map<string, RecoverableEntry> {
    let partition: Map<string, RecoverableEntry> = this.instances_.get(instanceId);
    if (!partition) {
      partition = new Map<string, RecoverableEntry>();
      this.instances_.set(instanceId, partition);
    }
    return partition;
  }

  private consumeRestored<T extends object, S extends object>(instanceId: number, key: string,
    type: TypeConstructorWithArgs<T>, defaultCreator: StorageDefaultCreator<T> | undefined,
    defaultSubCreator: StorageDefaultCreator<S> | undefined,
    notifyOnError?: NotifyOnErrorCallback): T | undefined {
    // Deserialization errors: code=0 (no error code), but callback IS invoked if set.
    // Always log; always fall back to the defaultCreator path.
    if (this.restoredFailures_.has(instanceId)) {
      this.restoredFailures_.delete(instanceId);
      this.safeNotify(notifyOnError, 'deserialization', [key],
        `restored section for instance ${instanceId} is corrupted, falling back to default`);
      return undefined;
    }
    const section: Map<string, string> = this.restored_.get(instanceId);
    if (!section) {
      return undefined;
    }
    if (!section.has(key)) {
      return undefined; // key absent from section: normal cold-start path (default creator)
    }
    const text: string = section.get(key);
    section.delete(key); // INV-4: read-once, regardless of outcome
    if (section.size === 0) {
      this.restored_.delete(instanceId);
    }
    if (typeof defaultCreator !== 'function') {
      this.safeNotify(notifyOnError, 'deserialization', [key],
        `default creator is required to restore the key '${key}', falling back`);
      return undefined;
    }
    try {
      const target: T = defaultCreator();
      const parsed = DataCoder.parse(text) as T;
      // ADR-14 injection defense: read-only properties on the target (getter-only /
      // @Computed / non-writable) are skipped during restore — including keys present in
      // archives written by older versions — silently, without failing the whole key.
      DataCoder.restoreTo<T, S>(target, parsed, defaultSubCreator, RecoverableStorageImpl.readOnlyFilter_);
      if (!(target instanceof type)) {
        throw new Error(`restored value is not an instance of the connected type`);
      }
      return target;
    } catch (err) {
      this.safeNotify(notifyOnError, 'deserialization', [key],
        `fail to restore the key '${key}': ${(err as Error)?.message}, using default value`);
      return undefined;
    }
  }

  // INV-5: a notifyOnError callback must never throw out (UI thread, no crash).
  private safeNotify(notifyOnError: NotifyOnErrorCallback | undefined,
    reason: RecoverableStorageErrorReason, keys: Array<string>, message: string,
    keyForLog?: string): void {
    stateMgmtConsole.applicationError(`RecoverableStorage error (${reason}) ` +
      `(key '${keyForLog ?? keys.join(',')}'): ${message}`);
    if (typeof notifyOnError !== 'function') {
      return;
    }
    try {
      notifyOnError({ reason: reason, keys: keys.slice(), message: message });
    } catch (err) {
      stateMgmtConsole.applicationError(`RecoverableStorage notifyOnError callback threw: ` +
        `${(err as Error)?.message}`);
    }
  }

  private static readonly INVALID_INSTANCE_ID: number = Number.NaN;

  private getInstanceIdFromComponent(component: RecoverableComponent): number {
    if (component === null || component === undefined ||
      typeof component.getInstanceId !== 'function') {
      return RecoverableStorageImpl.INVALID_INSTANCE_ID;
    }
    return component.getInstanceId();
  }

  // ADR-10 v5: field-level validation THROWS (PersistenceV2 pattern).
  private throwIfConnectOptionsInvalid<T extends object, S extends object>(
    options: RecoverableConnectOptions<T, S>): void {
    if (options.key !== undefined && typeof options.key !== 'string') {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.INVALID_KEY_TYPE_MESSAGE_);
    }
    if (options.defaultCreator !== undefined && typeof options.defaultCreator !== 'function') {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.INVALID_CALLBACK_MESSAGE_);
    }
    if (options.defaultSubCreator !== undefined && typeof options.defaultSubCreator !== 'function') {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.INVALID_CALLBACK_MESSAGE_);
    }
    if (options.notifyOnError !== undefined && typeof options.notifyOnError !== 'function') {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.INVALID_CALLBACK_MESSAGE_);
    }
  }

  // Blacklisted wrapper/collection instances returned by creators
  // (mirror of PersistenceV2 throwIfNotSupported).
  private throwIfValueNotSupported(value: object): void {
    const classes: Array<Function> = RecoverableStorageImpl.NOT_SUPPORT_TYPES_;
    if (value === null || value === undefined ||
      classes.some((clazz) => value instanceof clazz)) {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.NOT_SUPPORT_TYPE_MESSAGE_);
    }
  }

  // ADR-10 defensive guard for the native bridge entries (C++ is the only caller;
  // non-finite ids are ignored without side effects).
  private static isValidInstanceId(instanceId: number): boolean {
    return typeof instanceId === 'number' && Number.isFinite(instanceId);
  }

  private throwIfTypeIsNotSupported<T>(type: TypeConstructorWithArgs<T>): void {
    if (typeof type !== 'function' ||
      RecoverableStorageImpl.NOT_SUPPORT_TYPES_.includes(type as unknown as Function)) {
      throw new BusinessError(RECOVERABLE_STORAGE_UNSUPPORTED_TYPE,
        RecoverableStorageImpl.NOT_SUPPORT_TYPE_MESSAGE_);
    }
  }

  // Container roots where 'defaultSubCreator' serves as the element factory — aligned with
  // PersistenceV2 CollectionType (v2_persistence.ts) minus Sendable variants evaluated lazily.
  private static isContainerType_<T>(type: TypeConstructorWithArgs<T>): boolean {
    const ctor: Function = type as unknown as Function;
    return ctor === Array || ctor === Map || ctor === Set ||
      (typeof SendableArray === 'function' && ctor === SendableArray) ||
      (typeof SendableMap === 'function' && ctor === SendableMap) ||
      (typeof SendableSet === 'function' && ctor === SendableSet);
  }

  // ADR-14: read-only property filter, applied by default (no option) on BOTH sides:
  // save-side skip (stringify) and restore-side injection defense (restoreTo skip —
  // the assignment to a read-only property never happens, hence no throw, no callback).
  // Beyond DataCoder's getter-only check (getter with no setter anywhere on the chain):
  //  - @Computed accessors: instance-own get+set pair backed by the ComputedV2 internal
  //    cached property (the setter always throws 'readonly', see v2_computed.ts InitRun);
  //    the computed VALUE still round-trips via its writable cached backing property;
  //  - non-writable own data properties: assignment would throw in strict mode.
  // Skipping is silent — not an error, never notified (AC-14).
  private static readonly readOnlyFilter_: (target: object, prop: string) => boolean =
    function (target: object, prop: string): boolean {
      if (target === null || target === undefined || typeof target !== 'object') {
        return false;
      }
      if (DataCoder.isReadOnlyGetter(target, prop)) {
        return true;
      }
      const desc = Object.getOwnPropertyDescriptor(target, prop);
      if (desc !== undefined) {
        if (typeof desc.get === 'function' && typeof desc.set === 'function' &&
          Object.prototype.hasOwnProperty.call(target, ComputedV2.COMPUTED_CACHED_PREFIX + prop)) {
          return true;
        }
        if (desc.get === undefined && desc.set === undefined && desc.writable === false) {
          return true;
        }
      }
      return false;
    };
}

// Exact UTF-8 byte length of a JS string (ASCII-identical, surrogate-aware).
// Used by the snapshot byte budget (INV-3); no reliance on platform encoders.
function rsUtf8ByteLength(s: string): number {
  let bytes: number = 0;
  for (let i: number = 0; i < s.length; i++) {
    const code: number = s.charCodeAt(i);
    if (code < 0x80) {
      bytes += 1;
    } else if (code < 0x800) {
      bytes += 2;
    } else if (code >= 0xD800 && code <= 0xDBFF) {
      bytes += 4;
      i++;
    } else {
      bytes += 3;
    }
  }
  return bytes;
}

// Native bridge hook (ADR-2/ADR-3): the frontend delegate save/restore chain calls these
// via the JS engine on the save (GetContentInfo, ContentInfoType 2/3) and restore paths.
(globalThis as Record<string, unknown>).__RecoverableStorageHook__ = {
  extractSnapshot: (instanceId: number): string => {
    return RecoverableStorageImpl.instance().extractSnapshot(instanceId);
  },
  setRestoredData: (instanceId: number, json: string): void => {
    RecoverableStorageImpl.instance().setRestoredData(instanceId, json);
  },
};

// Global cleanup entry (module-none top-level declaration = global binding), called by
// JsiDeclarativeEngine::JsUnregisterInstanceId next to unregisterInstanceIdForEnv
// (ADR-5, mirrors v2_env.ts:302 cleanup pattern).
function unregisterInstanceIdForRecoverable(instanceId: number): void {
  RecoverableStorageImpl.instance().unregisterInstanceIdForRecoverable(instanceId);
}
