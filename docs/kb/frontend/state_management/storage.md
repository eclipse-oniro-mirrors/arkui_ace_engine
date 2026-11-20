# State Management — 存储体系 Context

> 文档版本：v1.3
> 更新时间：2026-10-31
> 来源：`docs/context_registry.json` 主题 `StateMgmtStorage`

## 定位

存储体系提供跨组件树、跨页面、跨进程生命周期的状态承载与持久化能力，分 V1 和 V2 两套并存。V1：`AppStorage`（应用级全局单例，继承 `LocalStorage`）、`LocalStorage`（页面级，可多实例）、`PersistentStorage`（磁盘持久化，经 `IStorage` 后端）、`Environment`（设备环境变量写入 `AppStorage`）。V2：`AppStorageV2`（内存全局存储，仅 `@ObservedV2`）、`PersistenceV2`（磁盘持久化，经可配置 `IStorage` 后端 + `DataCoder` 序列化保持原型链）。V1 的存储逻辑主体在 `LocalStorage`，`AppStorage` 只增加单例和静态 API 委托。V2 持久化通过 `ObserveV2` 依赖图自动追踪 `@Trace` 变化，无需手动通知。

本文档用于快速定位存储体系相关源码和测试入口。装饰器 `@StorageLink`/`@StorageProp`/`@LocalStorageLink`/`@LocalStorageProp` 见「V1 装饰器」子页面；`@Env`/`@CustomEnv` 见「V2 装饰器」子页面。

## 快速路由

### 源码入口

| 关注点 | 稳定路径 | 说明 |
|--------|----------|------|
| 页面级存储基类 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/sdk/local_storage.ts` | `LocalStorage extends NativeLocalStorage`（`storage_: Map<string, ObservedPropertyAbstract>`）；V1 存储逻辑主体 |
| 应用级存储单例 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/sdk/app_storage.ts` | `AppStorage extends LocalStorage` — 全局单例 + 静态 API 委托 |
| V1 磁盘持久化 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/sdk/persistent_storage.ts` | `PersistentStorage implements IMultiPropertiesChangeSubscriber` — 持久化管理、Map/Set/Date 序列化 |
| V1 设备环境 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/sdk/environment.ts` | `Environment`（单例）— 6 个固定 key 触发后端查询，写入 `AppStorage` |
| 持久化后端抽象 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/common/i_storage.ts` | `IStorage` 接口（不支持增量更新，`write()` 写所有属性） |
| 环境后端抽象 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/common/i_environment_backend.ts` | `IEnvironmentBackend`（6 个 getter 接口） |
| 环境原生声明 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/common/environment_setting.d.ts` | `EnvironmentSetting`（由 C++ `JSEnvironment` 注入） |
| V2 存储/持久化 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/sdk/v2_persistence.ts` | `AppStorageV2Impl`、`PersistenceV2Impl`（`MIN_PERSISTENCE_ID = 0x1020000000000`）、`StorageHelper`（key 校验） |
| V2 序列化 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/v2/v2_data_coder/` | `DataCoder.stringify/parse/restoreTo`、`JSONCoder`、`JSON2`、`Meta`（`@Type`）— 保持原型链 |
| LocalStorage C++ 绑定 | `frameworks/bridge/declarative_frontend/jsview/js_local_storage.cpp`、`.h` | `JSLocalStorage`（`storages_` thread_local，多 containerId） |
| PersistentStorage C++ 绑定 | `frameworks/bridge/declarative_frontend/jsview/js_persistent.cpp`、`.h` | `JSPersistent::JSBind`（"Storage"）→ `StorageProxy` |
| Environment C++ 绑定 | `frameworks/bridge/declarative_frontend/jsview/js_environment.cpp`、`.h` | `JSEnvironment::JSBind`（"EnvironmentSetting"） |
| 环境后端平台实现 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/env_base/` | `system_env.ts`、`env_factory.ts`、`i_env_breakpoint.ts`、`i_env_value.ts`、`customEnv_key.ts` |
| Interop 存储桥接 | `frameworks/bridge/declarative_frontend/state_mgmt/src/lib/interop/interop_storage.ts`、`v2_interop_storage.ts` | `InteropStorage`、`InteropStorageV2` — 跨动态/静态前端存储可见 |

### 关键机制锚点

| 机制 | 源码位置 |
|------|----------|
| PU/FU 属性创建分支 | `sdk/local_storage.ts` `addNewPropertyInternal` — `ViewStackProcessor.UsesNewPipeline()` 决定 `ObservedPropertyPU` vs FU |
| `link()`/`prop()` 双向/单向 | `sdk/local_storage.ts` `link()`/`prop()` |
| 装饰器同步工厂入口 | `sdk/local_storage.ts` `__createSync`（`@StorageLink`/`@StorageProp` 编译产物） |
| `AppStorage` 单例委托 | `sdk/app_storage.ts` — 静态 API 委托到内部 `LocalStorage` 实例 |
| `persistProp` 决策逻辑 | `sdk/persistent_storage.ts` `persistProp1` — 先 `AppStorage.link`；不在 AppStorage 则查磁盘 `storage_.has` → 读或用默认 → `AppStorage.setAndLink` |
| Map/Set/Date 序列化标记 | `sdk/persistent_storage.ts` `MapInfo`/`SetInfo`/`DateInfo` + `replacer`（NewVersion/CompatibleVersion 检测） |
| 嵌套对象持久化（V1） | 需手动 `PersistentStorage.notifyHasChanged('data')` |
| `delete`/`clear` 订阅者保护 | `sdk/local_storage.ts` — `numberOfSubscrbers()` > 0 时返回 false |
| `Environment` 扇出 | `sdk/environment.ts` `envProp` → `AppStorage.setAndProp`；`onValueChanged` → `AppStorage.set` 扇出 |
| Environment 6 固定 key | `accessibilityEnabled`/`colorMode`/`fontScale`/`fontWeightScale`/`layoutDirection`/`languageCode` |
| PersistenceV2 双格式分流 | `sdk/v2_persistence.ts` `save`/`writeAllChangedToFile` — 全局路径 `stringify(value, false)`（JSON2），模块路径 `stringify(value, true)`（Legacy JSON）；读取按 `json.startsWith('JSON2')` 分流（`getValueFromDisk`） |
| PersistenceV2 自动持久化 | `sdk/v2_persistence.ts` `connect` 分配 PersistenceV2 ID → `startRecordDependencies`/`stopRecordDependencies` 注册依赖图 → `@Trace` change → `fireChange` → `onChangeObserved` → `writeAllChangedToFile` |
| `PersistenceV2.connect` defaultCreator 时机 | 仅首次 connect（内存+磁盘都不存在） |
| PersistenceV2 不支持的类型 | WeakSet/WeakMap/Boolean/Number/String/Symbol/BigInt/RegExp/Function/Promise/ArrayBuffer |
| AppStorageV2 全局扁平 Map | 不支持 UIContext 隔离，需 `@Env`/`@CustomEnv` 按 instanceId 隔离 |
| `DataCoder.restoreTo` 保持原型链 | 将解析数据恢复到已有 `@ObservedV2` 实例（区别于 `JSON.parse`） |
| `IStorage.write()` 写所有属性 | 后端不支持增量更新 |

### API 入口

| 范式 | 稳定路径 | 说明 |
|------|----------|------|
| V1 装饰器（Dynamic） | `<OH_ROOT>/interface/sdk-js/api/@internal/component/ets/common.d.ts` | `@StorageLink`/`@StorageProp`/`@LocalStorageLink`/`@LocalStorageProp` 声明 |
| StateManagement 模块 | `<OH_ROOT>/interface/sdk-js/api/@ohos.arkui.StateManagement.d.ts` | `AppStorageV2`、`PersistenceV2`、`PersistenceV2.configureBackend(IStorage)`、`IStorage`、`DataType` |
| V1 装饰器（Static） | `<OH_ROOT>/interface/sdk-js/api/arkui/stateManagement/decorator.static.d.ets` | 静态 ArkTS 侧存储装饰器声明 |

具体 API（`AppStorage.Set/Get/Has/Delete`、`LocalStorage` 构造与方法、`PersistentStorage.persistProp`、`Environment.envProp`）的签名、错误码以 SDK 声明为准。

### 测试入口

| 类型 | 稳定路径 | 用途 |
|------|----------|------|
| 通用测试套 | `frameworks/bridge/declarative_frontend/state_mgmt/test/unittest/entry/src/main/common_tests/` | `AppStorage`、`LocalStorage`、`PersistentStorage`、`Environment` 行为回归 |
| V2 测试套 | `frameworks/bridge/declarative_frontend/state_mgmt/test/unittest/entry/src/main/v2_tests/` | `AppStorageV2`、`PersistenceV2`、`DataCoder`、`@Type` 序列化 |
| Env 测试套 | `frameworks/bridge/declarative_frontend/state_mgmt/test/unittest/entry/src/main/env_tests/` | `Environment`、`@Env`/`@CustomEnv` |
| Context registry | `docs/context_registry.json` | 主题 `StateMgmtStorage` |

### 相关 Spec

| FuncID | Spec 域 | 状态 |
|--------|---------|------|
| `07-02-03` | `specs/07-frontend/02-state-management/03-v1-app-state` | Baselined（V1 存储 4 Feat） |
| `07-02-06` | `specs/07-frontend/02-state-management/06-v2-app-state` | Baselined（V2 存储 2 Feat） |

## PersistenceV2 序列化规格

PersistenceV2 的磁盘持久化通过 `DataCoder` 编解码，支持 JSON2 与 Legacy JSON 两套格式并存，由存储路径决定选用哪套。`@Type` 装饰器提供 `alias`/`disabled`/`factory` 元数据，`DataCoder.restoreTo` 在已有 `@ObservedV2` 实例上还原以保持原型链。

### 属性枚举范围（getter 不参与序列化）

序列化时统一通过 `Object.keys(value)` / `Object.entries(source)` 枚举**自有可枚举字符串属性**，因此 getter 定义不会被序列化：

- 原型上的 getter（如 `@ObservedV2 class Foo { get bar() { ... } }`）不是自有属性，不在 `Object.keys` 中，**不参与序列化**；反序列化也只遍历 source 的 `Object.entries`，不会覆盖原型 getter，保持类定义行为。
- 非可枚举（`enumerable: false`）访问器属性同样不在 `Object.keys` 中。
- 类方法（函数值）不被序列化：`JSON.stringify` 丢弃函数；`restorePropValue` 遇 `typeof tgtVal === 'function'` 直接跳过（`data_coder.ts:253`）。
- `@Trace` 属性的值经 `trackInternal` 移到 `__ob_<prop>` 后备字段（自有可枚举数据属性），replacer 剥离 `__ob_` 前缀后以原属性名序列化——区别于无后备字段的纯 getter，`@Trace` 属性**会**被序列化。
- `@Computed` 属性经 `ComputedV2.InitRun` 用 `Reflect.defineProperty` 在实例上重定义为 `enumerable: true` 的访问器（getter 返回缓存值，setter 抛只读错），因此**会**进入 `Object.keys` 并被序列化（写缓存计算结果）；但原始类型缓存值反序列化时因 setter 抛错而失败。详见 [v2-decorators](v2-decorators.md)「@Computed 属性重定义与序列化」。

> 源码：`json2.ts:164` `toAliasedObject`、`json_coder.ts:104` `ObservedReplacer`、`json_coder.ts:325` `cloneWithoutVisited`（均 `Object.keys(value)`）；`data_coder.ts:232` `restoreObject`（`Object.entries(source)`）；`data_coder.ts:125` `touchAll`（`for...in` + `hasOwnProperty`）；`data_coder.ts:253` 函数值跳过。

### 格式选择规则

| 存储路径 | 写入（`stringify`） | 读取 | 触发位置 |
|----------|---------------------|------|----------|
| 全局路径（`globalConnect`，带 `areaMode`） | JSON2 格式：`DataCoder.stringify(value, false)` | `json.startsWith('JSON2')` → `DataCoder.parse` + `DataCoder.restoreTo` | `save()`、`writeAllChangedToFile()` |
| 模块路径（`connect`，无 `areaMode`） | Legacy JSON 格式：`DataCoder.stringify(value, true)` → `JSONCoder.stringify` | 不以 `JSON2` 开头 → `JSONCoder.parseTo(target, json)` | `save()`、`writeAllChangedToFile()` |

> 源码：`sdk/v2_persistence.ts:543`/`:552`（`save`）、`:774`（`writeAllChangedToFile` 按 `keyType !== GLOBAL_MAP` 选 `forceLegacyFormat`）、`:708`（读取按 `FORMAT_TAG` 分流）。

### JSON2 格式（新）

`DataCoder.FORMAT_TAG = 'JSON2'`，由 `JSON2.stringify`/`JSON2.parse` 实现（`v2/v2_data_coder/json2.ts`）：

- 序列化时根对象包裹为 `{ $: value }`，每个访问过的对象分配 4 位 32 进制 `refId`（`visited.size.toString(32).padStart(4, '0')`）。
- 编码形态为 `<typeTag><refId><payload>`，typeTag 取值：

| typeTag | 含义 | payload |
|---------|------|---------|
| `st` | 普通字符串 | `st????<value>` |
| `bi` | bigint | `bi????<value>` |
| `re` | undefined（`re????`）或对象回引（`re<refId>`） | — |
| `Bo`/`Nu`/`St` | Boolean/Number/String 包装对象 | `toString()` 结果 |
| `Da` | Date | ISO 字符串 |
| `Ar`/`Ma`/`Se` | Array/Map/Set | 子项分别序列化后拼装 |
| `SA`/`SM`/`SS`/`SO` | SendableArray/SendableMap/SendableSet/通用 @Sendable | `JSON.stringifySendable` 产物 |

- 循环引用通过 `refId` 回引还原；`@Type` 的 `alias`/`disabled` 经 `JSON2.toAliasedObject` 应用。
- @Sendable 经 `JSON.stringifySendable`/`parseSendable` 处理，要求属性为纯类型（`throwIfNotJSONSafe`，嵌套对象抛 `PERSISTENCE_V2_APPSTORAGE_V2_UNSUPPORTED_TYPE`），且 reviver 不生效。

### Legacy JSON 格式

`JSONCoder`（`v2/v2_data_coder/json_coder.ts`），基于 `JSON.stringify` + `ObservedReplacer`：

- Map → `[...entries]`、Set → `[...values]`，剥离 `__ob_` 前缀（`@Trace` 后备存储），应用 `Meta` 的 `alias`/`disabled`。
- 循环引用：`stringifyCircular` 回退将已访问对象替换为 `[Circular]`（`getValueFromDisk` 在 JSON2 路径出错时用其保存 `oldValue` 给错误回调）。
- 反序列化 `JSONCoder.parseTo(target, source)`：在已有 target 上还原（保留原型/方法/装饰器）。

### `restoreTo` 原型链保持

`DataCoder.restoreTo(target, source, defaultSubCreator?)`（`data_coder.ts:63`）将解析数据恢复到已有 `@ObservedV2` 实例，区别于 `JSON.parse` 直接重建：

- 通过 `Meta.gets(target)` 构建 `alias2prop` 反向映射，按 alias 还原属性名。
- `defaultSubCreator` 包装为 `factory`，用于集合元素（`CollectionType`）的嵌套类实例化。
- 共享引用通过 `visitedSources_`（`Map<object, object>`）还原；递归保护通过 `visitedTargets_`（`Set<string>`）。
- 集合（Array/Map/Set/Sendable*）原地清空再填充；Date 经 `setTime` 保持实例；Sendable 根仅还原属性。
- 还原后对 `@ObservedV2` target 调 `touchAll` 重新记录依赖。

### `@Type` 元数据

`@Type` 装饰器（`json_coder.ts:72` `__Type__`）在原型上经 `Meta.define` 注册 `TransformOptions`：

| 字段 | 作用 |
|------|------|
| `factory` | 嵌套对象/集合元素的构造器（由类型构造函数或 `defaultSubCreator` 派生） |
| `alias` | 序列化字段名映射（磁盘 key ≠ 属性名） |
| `disabled` | 该属性不参与序列化/反序列化 |

`Meta` 通过 `WeakMap<object, meta>` 按原型链存储，`Meta.gets` 沿原型链合并。

### 序列化相关错误码

| 错误码 | 触发条件 | 源码位置 |
|--------|----------|----------|
| `PERSISTENCE_V2_LACK_TYPE` | 嵌套对象缺 `@Type`（target 为 null 或集合元素无 factory） | `data_coder.ts:370`、`json_coder.ts:427`/`:483` |
| `PERSISTENCE_V2_MISMATCH_BETWEEN_KEY_AND_TYPE` | target 与 source 类型不匹配（Array/Map/Set/Sendable 交叉、原始类型不符） | `data_coder.ts:340`、`json_coder.ts:214`/`:420`/`:447` |
| `PERSISTENCE_V2_APPSTORAGE_V2_UNSUPPORTED_TYPE` | 集合元素为集合、Sendable 源对应非 Sendable target、@Sendable 含嵌套对象 | `data_coder.ts:357`/`:365`、`json2.ts:287` |
| `PERSISTENCE_V2_APPSTORAGE_V2_INVALID_DEFAULT_CREATOR` | `defaultSubCreator` 返回无效值 | `data_coder.ts:377` |

### 持久化错误回调

`PersistenceV2.notifyOnError(cb)` 注册 `PersistErrorCallback`（`v2_persistence.ts:562`）。回调签名 `(key, reason, error, oldValue?)`：

- `reason: 'serialization'`：序列化/反序列化异常，`oldValue` 为出错的原始 JSON（JSON2 路径经 `stringifyCircular` 处理）。
- `reason: 'quota'`：key 命中保留名 `___keys_arr`。
- `reason: 'unknown'`：存储后端读写异常或 keys_arr 操作失败。

未注册回调时，`serialization` 错误抛 `BusinessError`，`unknown` 错误抛 `Error`。

### 验证入口

| 关注点 | 测试文件 |
|--------|----------|
| JSON2 编解码（refs/cycles/特殊类型） | `v2_tests/json2TestsV2.ts` |
| Legacy JSONCoder（Map/Set/Date/alias/disabled） | `v2_tests/jsonCoderTestsV2.ts` |
| DataCoder stringify/parse/restoreTo 端到端 | `v2_tests/dataCoderTestsV2.ts` |
| PersistenceV2 持久化全流程 | `v2_tests/persistenceV2TestsV2.ets` |

## 常见问题定位

| 问题 | 优先查看 |
|------|----------|
| `delete`/`clear` 返回 false | 属性仍有订阅者（`numberOfSubscrbers() > 0`，注意源码拼写缺 `i`） |
| `AppStorage` vs `LocalStorage` | 前者全局单例，后者可多实例（页面级）；`AppStorage` 继承 `LocalStorage` |
| `@StorageLink` 与 `AppStorage.link()` 关系 | 前者是编译器糖 → `__createSync` → `SynchedPropertyTwoWayPU` |
| 嵌套对象变化未持久化（V1） | 需手动 `PersistentStorage.notifyHasChanged('data')` |
| `persistProp` defaultValue 何时用 | `AppStorage` 和磁盘都不存在时 |
| `write()` 写所有属性 | `IStorage` 不支持增量更新 |
| PersistenceV2 反序列化类型失败 | 嵌套对象缺 `@Type` 抛 `PERSISTENCE_V2_LACK_TYPE` |
| PersistenceV2 不支持的类型 | WeakSet/WeakMap/Boolean/Number/String/Symbol/BigInt/RegExp/Function/Promise/ArrayBuffer |
| 全局 vs 模块路径序列化格式不同 | 全局路径（`globalConnect`）写 JSON2，模块路径（`connect`）写 Legacy JSON；读取按 `JSON2` 前缀分流 |
| @Sendable 序列化失败 | JSON2 路径要求 @Sendable 属性为纯类型（`throwIfNotJSONSafe`），嵌套对象抛 `PERSISTENCE_V2_APPSTORAGE_V2_UNSUPPORTED_TYPE` |
| 集合元素为自定义类 | 需提供 `defaultSubCreator`（`ConnectOptionsCollections`），否则元素无法还原原型 |
| getter 定义未被持久化 | 序列化枚举用 `Object.keys`（自有可枚举属性），原型 getter / 非可枚举访问器不在其中；`@Trace` 经 `__ob_<prop>` 后备字段序列化，纯 getter 无后备字段故不序列化；**例外**：`@Computed` 经 `Reflect.defineProperty` 置 `enumerable: true` 会被序列化，但原始类型缓存值反序列化因 setter 抛错失败 |
| AppStorageV2 不支持 UIContext 隔离 | 全局单例扁平 Map；需 `@Env`/`@CustomEnv` 按 instanceId 隔离 |
| `PersistenceV2.connect` defaultCreator 调用时机 | 仅首次 connect（内存+磁盘都不存在） |
| `Environment` 写入 AppStorage | `envProp` → `AppStorage.setAndProp`；`onValueChanged` → `AppStorage.set` 扇出 |
| 持久化恢复异常 | V1 检查 Map/Set/Date 序列化标记版本；V2 检查 `DataCoder.restoreTo` 原型链 |

## 调试入口

- **存储状态检查**：`AppStorage.keys()`、`size()`、`has(key)`、`get(key)`。
- **订阅者计数**：`ref.numberOfSubscrbers()`（注意源码拼写）。
- **日志关键字**：`Creating AppStorage instance`、`PersistentStorage: persistProp1 ...`、`writing 'X' to storage`、`property changed`、`sync peer X has changed`。
- **持久化恢复验证**：`persistProp` → 修改 → 重启 → 检查恢复值。
- **V2 持久化排查**：`PersistenceV2.connect` 的 `keyItemName`/`type`、`StorageHelper.isKeyValid`、`DataCoder` 输入输出对比。
- **V2 序列化排查**：检查磁盘 JSON 是否以 `JSON2` 前缀开头（区分格式）；`@Type` 的 `alias`/`disabled` 经 `Meta.gets(obj)` 查看；`restoreTo` 类型不匹配看 `PERSISTENCE_V2_MISMATCH_BETWEEN_KEY_AND_TYPE`；注册 `PersistenceV2.notifyOnError` 捕获 `serialization`/`quota`/`unknown` 及 `oldValue`。

## 相关主题

- [总览](../state-management.md)
- V1 核心（`ObservedPropertyPU` 是存储 `link()` 的返回基础）：[v1-core](v1-core.md)
- V2 核心（`PersistenceV2` 经 `ObserveV2` 依赖图自动追踪）：[v2-core](v2-core.md)
- V1 装饰器（`@StorageLink`/`@StorageProp`/`@LocalStorageLink`/`@LocalStorageProp`）：[v1-decorators](v1-decorators.md)
- V2 装饰器（`@Env`/`@CustomEnv`/`@Type`）：[v2-decorators](v2-decorators.md)
- C++ 绑定（`JSLocalStorage`/`JSPersistent`/`JSEnvironment`）：[cpp-bindings](cpp-bindings.md)
- [Interop（跨前端存储可见）](interop-bridge.md)
- [静态存储体系（arkoala 侧）](static-storage.md)