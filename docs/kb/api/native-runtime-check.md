# Native Runtime Check Context

> 更新时间：2026-10-24

## 定位

运行时配置与既有线程资格检查保持分离。MULTI_THREAD 只检测通过
`ARKUI_MULTI_THREAD_NATIVE_NODE` 获取的接口调用；集合及接口线程约束见
OpenHarmony docs 的 `zh-cn/application-dev/ui/ndk-build-on-multi-thread.md`。
内部布局、渲染、LazyForEach、Repeat 不独立建立本检测的操作域。

## 源码与测试路由

| 内容 | 路径 |
|---|---|
| 公开检查类型、模式 | `interfaces/native/native_interface.h` |
| 配置存储与模式下发 | `interfaces/native/node/config_manager.cpp` |
| 多线程接口集合 | `interfaces/native/node/native_impl.cpp` 的 `multiThreadNodeImpl_1` |
| 外部操作域包装 | `interfaces/native/node/node_model_safely.cpp` |
| 引擎桥接 | `frameworks/core/interfaces/native/node/node_api_multi_thread.cpp` |
| 临时树占用与关系变更通知 | `frameworks/core/components_ng/base/native_node_tree_scope.cpp` |
| 关系原语、Free 标志 | `frameworks/core/components_ng/base/ui_node.cpp` |
| 拆合树、模式切换、关闭兼容性测试 | `test/unittest/core/base/native_node_tree_scope_test.cpp` |
| 配置边界测试 | `test/unittest/capi/runtime_check/capi_runtime_check_test.cpp` |

## 设计边界与排查

- MULTI_THREAD=3，ATTRIBUTE_MATCH=2 为独立保留项；LOG=1 诊断后继续，CRASH=2 才主动终止。
- 仅活动的外层 Native 调用持有树状态和祖先引用。同线程嵌套扩展该调用的持有集合；
  不给每个 UINode 注册 Association，不在节点构造、释放、析构时维护检测注册表。
- 进入时在短锁内沿真实父关系查找根。SetParent、ResetParent、SetAdoptParent 仅通知检测侧
  拆合树结果，不主动修改业务关系或深度；外层调用返回前保留迁移后的占用。
- 普通内部操作不会自行占用树。仅支持外部接口的树关系/Free 标志同步，不扩展为全引擎检测。
- 稳定 DISABLED 状态不获取新增协议锁、不创建占用或额外保活；UINode 原有写入与释放逻辑保留。
  关闭前已进入的域仍配对清理；关闭期间变化的树在重新进入时从真实关系定位，不复用陈旧注册表。
- 模式切换控制后续检测，不追溯关闭期间已经开始的调用，也不修复应用本身的错误并发。
  启用前已开始的未检测调用不得与同树新调用重叠；应用仍须遵守原有串行化约束。
- 所有诊断在协议锁外同步输出，诊断只持有值快照，防止慢速符号解析阻塞无关树；日志重入不递归输出。
- 关闭时 Native TLS 仍采用原实现的退出行为；启用时恢复外层值以支持同步回调重入。
- 将内联 UINode 方法移到 cpp 时，补齐 libace 导出和轻量 Mock 定义；Mock 不依赖真实树协议。

旧方案曾通过每节点注册、全局 getter 锁和 MaybeRelease 提前拆树维持协议。把这些实现细节
当成接口需求导致了侵入范围扩大；检查覆盖率应先从外部接口集合核对，不能用“内部改树”
作为到处添加独立操作域的依据。测试通过范围须分别记录真实引擎构建与使用节点桩的宿主测试。
