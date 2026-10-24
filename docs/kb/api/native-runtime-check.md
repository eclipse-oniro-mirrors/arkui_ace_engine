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
| 临时树根占用与拆树预告 | `frameworks/core/components_ng/base/native_node_tree_scope.cpp` |
| 新增只读直接子根辅助方法 | `frameworks/core/components_ng/base/ui_node.cpp` |
| 拆合树、模式切换、关闭兼容性测试 | `test/unittest/core/base/native_node_tree_scope_test.cpp` |
| 配置边界测试 | `test/unittest/capi/runtime_check/capi_runtime_check_test.cpp` |

## 树根方案边界与排查

- MULTI_THREAD=3，ATTRIBUTE_MATCH=2 为独立保留项；LOG=1 诊断后继续，CRASH=2 才主动终止。
- 外部接口登记实际树根和输入/返回的操作子树根，RemoveAllChildren 在业务前 PrepareChildren
  登记直接待拆子树根。不扫描 g_nodeSetSafely，不为全部后代建立占用映射。
- UINode 原有方法与 master 保持一致。只新增非虚只读 GetChildrenForNativeTreeCheck，返回
  children_ 的直接引用副本，不调用虚 GetChildren。没有构造注册、析构注销、父关系通知或 Free getter 锁。
- 查找先匹配根索引，命中待拆/移动根时停止，不读取该根正在变化的 parent。根预告必须在原业务前完成；
  不能只在拆树后更新。全局短锁仅保护协议数据，不覆盖业务，不能单凭查根加锁宣称原父链接写入已同步。
- 根索引在外层返回前保留。内层退出不释放 Context；LOG 每个 Context 独立记账，多输入状态保留相关 owner。
- 稳定 DISABLED 不创建 Context/State、不查树、不获取新增协议锁、不额外保活。关闭后旧域仍配对清理。
  重新开启沿真实父链定位，无须全节点补注册；不追溯关闭期间开始的调用，不修复错误业务并发。
- 普通内部布局、渲染、LazyForEach、Repeat 不独立建立检测域。未预告的内部任意拓扑改写不属于全引擎检测承诺。
- 诊断在锁外同步输出值快照，避免慢速符号解析阻塞无关树；日志重入不递归输出。
- g_checkedScopeDepth 是 TLS Native 包装对象层数，仅辅助 MarkNodeTreeFreeScope 恢复原许可标志；
  g_activeCalls 是全局外层 Context 数。它们都不是每节点 ownerDepth。
- 新版没有 NativeNodeTreeMutation、State.members 或 CaptureNodes。排查时不要引用已撤下方案的类图。
- 全部类图、时序与成本说明：[树根方案说明](../../design/NDK同树并发检测v2_修改流程与类图时序图.md)。

旧方案曾通过每节点注册、getter 锁和 MaybeRelease 提前拆树维持协议，后续快照方案又引入了全局句柄扫描。
这些实现细节不能当成接口需求。当前以原有 UINode 方法不变、外部入口提前根登记为边界；
测试通过范围必须区分真实引擎构建、使用节点桩的宿主测试和设备回归。
