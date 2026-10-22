# UISession Service Context

> 文档版本：v1.2
> 更新时间：2026-10-20
> 来源：`docs/context_registry.json` 主题 `UISessionService`

## 定位

UISession 是 ArkUI 在应用进程内提供给系统 SA、调试工具和 AI 能力的内部 UI 会话通道，负责将跨进程请求转换为 Pipeline、组件树、Web、翻译和内容变化检测的内部操作。它不是面向应用的公共组件或 Public API，而是框架内部 IPC 通道和状态管理中心。

本 KB 只提供稳定的源码、SDK、测试和 Spec 路由。具体事件传递细节、门控逻辑、翻译流程和并发保护应回到当前源码、测试与相关 Spec 核实。

### Report 对象与死亡监听方向

SA 侧 `UIContentServiceProxy::Connect` 创建本地 `UiReportStub`，通过 IPC 传给应用侧
`UiContentStub::ConnectInner`。应用侧 `SaveReportProxy` 保存的是 `sptr<IRemoteObject>` 远程对象，
上报时通过 `iface_cast<ReportService>` 使用 `UiReportProxy` 业务代理；不能将 map 中的对象
直接等同于 `UiReportProxy` C++ 实例。

应用侧在该远程对象上添加 `UiReportProxyRecipient`，感知 SA 死亡并清理本地注册。
模拟 SA 的 `UiSaService::getArkUIService` 则在指向应用侧 `UiContentStub` 的远程对象上添加
`UiContentProxyRecipient`，感知应用死亡并清理本地缓存。两者均在本地 proxy 上注册，
通知对应远端的死亡；应按 IPC 接收端和对象类型判断进程方向，避免因函数名包含 Stub 而倒置两端。

### SendCommand 完成回调

两个旧 `SendCommand` 和 `SendCommandSync` 保持原行为。新增 callback 重载分别处理节点命令和全局命令，
接口返回值表示是否受理，最终执行结果经 `ReportService::ReportCommandResult` 反向 IPC 返回。

- `UIContentServiceProxy::SendCommandWithFinishCallback` 注册 pending、发送 requestId，并安排 5000ms 超时。
- `UiReportStub` 以 requestId 匹配结果，先清状态再锁外回调；重复、迟到结果忽略。
- 超时、死亡、重连及销毁清理不回调。超时不取消已经开始的 UI 操作。
- 应用侧 `UiSessionManagerOhos::MakeCommandResultCallback` 捕获原 report remote，校验其仍为当前连接，避免重连串单。
- `UIContentImpl` 将执行投递至 UI 线程；节点经 `FrameNode/Pattern::OnRecvCommandWithResult`，
  keycode 使用实际 handled 返回值，relaxed 等待 workflow 最终态。
- `ui_session_command_result.h` 统一序列化；非批量成功精确为 `{"result":"SUCCESS"}`，失败为四种 errorType，
  errorMessage 合法 UTF-8 且不超过 128 字节，消息正文不进入新诊断日志。
- batch 保持旧请求格式及目标范围；整批一次回调，以 `results` 数组按输入顺序返回每项 nodeId 和结果。
  无效节点标识用 nodeKey 保留；一项失败后继续处理后续节点。

源码定位：`ui_content_proxy.cpp::SendCommandWithFinishCallback`、`ui_report_stub.cpp::ReportCommandResult`、
`ui_content_stub.cpp::SendCommandWithFinishCallbackInner`、`ui_session_batch_command.cpp::ExecuteWithResult`。
本节是路由索引；验收状态以 change 的 fresh verification evidence 为准。

## 快速路由

### 源码入口

| 关注点 | 稳定路径 | 说明 |
|--------|----------|------|
| IPC 接口定义 | `interfaces/inner_api/ui_session/ui_content_service_interface.h` | IUiContentService + ReportService IPC 事务码 |
| Manager 抽象接口 | `interfaces/inner_api/ui_session/ui_session_manager.h` | UISession Meyers singleton，虚方法 + Save*Function + atomic 计数器 |
| Manager OHOS 实现 | `adapter/ohos/entrance/ui_session/ui_session_manager_ohos.h/.cpp` | IPC 路由、reportObjectMap_、并发门控、SA 死亡清理 |
| IPC Stub | `adapter/ohos/entrance/ui_session/ui_content_stub.cpp` | OnRemoteRequest 分发，IsSACalling + interface token 校验 |
| Stub 实现 | `adapter/ohos/entrance/ui_session/ui_content_stub_impl.cpp` | 委托转发至 UiSessionManager::GetInstance() |
| IPC Proxy | `adapter/ohos/entrance/ui_session/ui_content_proxy.cpp` | SendPageTranslateRequest、SyncRequestGuard、HitTest 分包 |
| Report Stub | `adapter/ohos/entrance/ui_session/ui_report_stub.cpp` | PageTranslate callback 注册、超时、watchdog |
| Report Proxy | `adapter/ohos/entrance/ui_session/ui_report_proxy.cpp` | 反向 IPC 代理（app→SA 上报） |
| 参数配置 | `interfaces/inner_api/ui_session/param_config.h` | ParamConfig / ContentChangeConfig / ComponentEventType / ChangeType |
| JSON 工具 | `interfaces/inner_api/ui_session/ui_session_json_util.h/.cpp` | InspectorJsonValue (cJSON RAII) + InspectorJsonUtil 工厂 |
| 翻译管理器 | `interfaces/inner_api/ui_session/ui_translate_manager.h` | UiTranslateManager 翻译管理器虚基类 |
| 翻译实现 | `adapter/ohos/entrance/ace_translate_manager.cpp` | UiTranslateManagerImpl，WebView + Page 双通道 |
| 翻译类型 | `interfaces/inner_api/ui_session/ui_translate_type.h` | TranslateContentScope / TranslateTextRequest / TranslateResult |
| 并发保护 | `interfaces/inner_api/ui_session/ui_session_request_guard.h` | SyncRequestGuard RAII CAS |
| IPC 工具 | `interfaces/inner_api/ui_session/ui_session_ipc_util.h` | Ashmem 大字符串传输 |
| 回调注册 | `adapter/ohos/entrance/ui_content_impl.cpp` | InitUISessionManagerCallbacks 回调注册 |
| 内容变更 | `frameworks/core/components_ng/manager/content_change_manager/` | ContentChangeManager 内容变更检测 |
| SA 验证 | `interfaces/inner_api/ui_session/ui_session_sample/` | UiSaService SA 示例（SA_ID=16666） |

### API 入口

UISession 是框架内部 IPC 通道，没有独立 SDK API。IPC 接口通过 `IUiContentService` 定义，由 `UiContentStub`/`UiContentProxy` 在 OHOS 侧实现，不对外暴露公共 API。

### 外部依赖入口

| 依赖方向 | 本仓入口 | 外部仓路径 | 说明 |
|----------|----------|-----------|------|
| WMS 窗口管理 | `adapter/ohos/entrance/ui_session/ui_content_proxy.cpp` | `foundation/window/window_manager` | GetUIContentRemoteObj 获取焦点窗口 IUiContentService |
| Samgr 系统能力 | `adapter/ohos/entrance/ui_session/ui_session_manager_ohos.cpp` | `foundation/systemabilitymgr` | SA 注册/死亡监听/IPC 代理获取 |
| Ashmem 共享内存 | `interfaces/inner_api/ui_session/ui_session_ipc_util.h` | `commonlibrary/c_utils` | 大字符串跨进程传输 |
| ArkWeb | `adapter/ohos/entrance/ace_translate_manager.cpp` | `foundation/arkui/ace_engine`（Web 组件） | WebView 翻译通道 |

### Spec 路由

| Spec | 路径 | 状态 |
|------|------|------|
| 设计文档 | `specs/03-engine-framework/09-uisession/01-uisession-service/design.md` | Draft；本轮仅补录Feat-11增量 |
| Feat-01 IPC安全框架 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-01-ipc-security-framework-spec.md` | Baselined |
| Feat-02 InspectorTree | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-02-inspector-tree-query-and-web-aggregation-spec.md` | Baselined |
| Feat-03 事件上报 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-03-event-reporting-and-atomic-gating-spec.md` | Baselined |
| Feat-04 命令下发 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-04-command-dispatch-and-sync-request-spec.md` | Baselined |
| Feat-05 翻译能力 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-05-translate-capability-and-dfx-spec.md` | Baselined |
| Feat-06 内容变化 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-06-content-change-detection-and-threshold-spec.md` | Baselined |
| Feat-07 查询与Dump | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-07-query-and-auxiliary-dump-spec.md` | Baselined |
| Feat-08 SA验证服务 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-08-sa-verification-service-spec.md` | Baselined |
| Feat-09 页面场景规则化感知 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-09-pagescene-rule-awareness-spec.md` | Draft |
| Feat-10 WM验证链路 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-10-wm-uicontent-remoteobj-verification-spec.md` | Draft |

| Feat-11 SendCommand 完成回调 | `specs/03-engine-framework/09-uisession/01-uisession-service/Feat-11-send-command-finish-callback-spec.md` | Draft；设备验收未闭合 |

### 测试入口

| 关注点 | 路径 | 说明 |
|--------|------|------|
| Mock Manager | `test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.*` | UiSessionManager mock |
| 页面场景回调回归 | `test/unittest/adapter/ohos/entrance/ui_session/ui_content_stub_test.cpp` | `UIContentStubTestPageScene*`：规则集数据隔离、旧版事件兼容和重复注册保护 |
| SendCommand host 协议/状态/序列化/批量 | `test/unittest/adapter/ohos/entrance/ui_session/ui_session_send_command_test.cpp` | 实际 Proxy/Report IPC 传递及可控时间、并发、重入验证 |
| 组件结构化结果 | `test/unittest/core/base/send_command_result_test_ng.cpp` | 组件错误矩阵、单次执行、参数类型 |
| relaxed 最终状态 | `test/unittest/core/pattern/relaxed_interaction/relaxed_interaction_manager_test_ng.cpp` | 跨帧、fallback、隔离、旧语义回归 |
| SA 验证构建 | `interfaces/inner_api/ui_session:ui_session_example` | ui_sa 构建 target |
| 内容变化 Dump | `PipelineContext::DumpInfo -contentChange` | 非 release 版本调试入口 |

### 调试入口

| 关注点 | 断点位置 | 说明 |
|--------|----------|------|
| IPC 入口 | `ui_content_stub.cpp` OnRemoteRequest | 检查 code、SA token、interface token |
| 连接链路 | `ui_content_stub.cpp` ConnectInner + `ui_session_manager_ohos.cpp` SaveReportProxy | 连接与 Report 注册 |
| 事件注册 | `ui_content_stub_impl.cpp` | 各类事件注册回调 |
| 组件事件过滤 | `ui_session_manager_ohos.cpp` | mask 过滤与广播上报 |
| InspectorTree | `ui_content_impl.cpp` + `ui_session_manager_ohos.cpp` | 查询与 Web 聚合 |
| 页面翻译 | `ui_session_manager_ohos.cpp` Get/Start/End/Reset | 翻译生命周期 |
| 翻译 DFX | `ui_content_proxy.cpp` + `ui_report_stub.cpp` | callback 超时与 watchdog |
| 翻译高频调用限制 | `ui_content_proxy.cpp` + `ui_report_stub.cpp` | 重复 Get/Start 返回 LAST_UNFINISH |
| 死亡恢复 | `ui_session_manager_ohos.cpp` OnRemoteDied | ordered cleanup |
| 内容变化 | `pipeline_context.cpp` DumpInfo | 非 release 版 `-contentChange` 入口 |

## 常见问题定位

| 问题 | 定位路径 | 说明 |
|------|----------|------|
| 事件上报 SA 未收到 | `ui_session_manager_ohos.cpp` | 检查 ReportService 非空 + 注册计数 > 0 + mask 命中 |
| 页面场景回调串集 / 重复注册后无事件 | `ui_report_stub.cpp`、`ui_content_proxy.cpp` | 检查按 ruleSetId 过滤 hits/exits；检查失败注册是否覆盖或删除已有回调。回归用例应验证回调所属数据和原注册仍有效，不能用合并 JSON 原样分发的断言代替隔离验证 |
| InspectorTree 无结果 | `ui_session_manager_ohos.cpp` | 检查 processMap_ / inspectorFunction_ / webTaskNums_ |
| 翻译请求 PARAM_INVALID | `ui_session_manager_ohos.cpp` | 检查 scope 格式和 IsTranslateScopeValid |
| 内容变化无 TEXT 上报 | `content_change_manager.cpp` | 检查 ContentChangeConfig 阈值和滚动/过渡状态 |
| WM remote object 为空 | `ui_sa_service.cpp` + WM 补丁 | 检查焦点窗口和 WMS GetUIContentRemoteObj 链路 |

### 本地回归环境核对

- relaxed/Slide测试依赖根Geometry：`test/mock/frameworks/core/pipeline/mock_pipeline_context.cpp:2097`的`GetRootRect`应读取rootNode的frame rect。空实现会让已设置根尺寸的用例进入错误分支；先核对mock与正式实现，再判断业务回归。vector结果先ASSERT长度再索引，避免断言失败后越界中止。
- ARM用户态QEMU的动态库搜索目录不能仅按basename任选同名库。`libutils.z.so`同时存在于`commonlibrary/c_utils`和`commonlibrary/ets_utils`；Binder/RefBase所需的是前者。记录每个软链接的来源及实际loader错误，修复运行环境后再采集XML。
- TimePicker成功测试需通过Model初始化主题和列节点。裸Pattern没有hour列，不能作为可执行成功场景。TextField/Search的requestKeyboard完成结果需检查真实焦点请求bool；无焦点能力时不能因旧parser返回成功而判定执行成功。

上述核对仅服务本地测试，不替代真实Binder、ArkWeb或设备性能验收。

## 附录：验证部署步骤

完整验证部署步骤已迁移至 `../../../test/tools/UISession/UISession_Verification_Guide.md`，包括：

- ui_session_sample 构建与部署
- WM 验证补丁构建与推送
- WM 验证重启后确认步骤
- hidumper 命令完整示例
- 结果确认与常见问题排查
- 扩展指南

## 附录：扩展指南

### 如何在 UISession 上新增能力

1. 在 `ui_content_service_interface.h` 增加 transaction code 和 `IUiContentService` 方法。
2. 在 `UiContentStub::OnRemoteRequest` 增加 code 分发，实现对应 `*Inner` 方法解析 `MessageParcel`。
3. 在 `UIContentServiceStubImpl` 中将服务方法转发到 `UiSessionManager`。
4. 在 `UiSessionManager` 增加虚接口与必要状态，在 `UiSessionManagerOhos` 中实现 OHOS 侧逻辑。
5. 如需访问 Pipeline 或节点树，在 `UIContentImpl::InitializeCallback` 或相关初始化路径注册回调，并通过 `TaskExecutor` 投递到 UI 线程。
6. 如需远端结果回调，更新 `ReportService` 相关 proxy/stub，并维护 `processMap_` 中对应能力 key 的请求进程集合。
7. 补充 `test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.*` 和相关单元测试。

### 如何新增本应用进程 dump 注入能力

当新能力只用于调试/验证时，优先复用应用进程已有 dump 链路：

- 链路：`UIContentImpl::DumpInfo` → `PostSyncTaskTimeout` → `AceContainer::Dump` → `AceView::Dump` / `PipelineContext::Dump`。
- 新增 dump 参数时 5 条边界规则：
  1. 在 `PipelineContext::OnDumpInfo` 或更靠近所属模块的 dump 分发点新增稳定参数，例如 `-contentChange` 这类以 `-` 开头的命令；调试专用能力应使用 `#ifndef IS_RELEASE_VERSION` 隔离。
  2. dump 处理必须在 UI 线程内快速完成。`UIContentImpl::DumpInfo` 当前超时为 1500ms，长耗时逻辑应拆为异步状态采集或仅触发动作。
  3. 参数需要显式防递归和防误触发。`UIContentImpl::DumpInfo` 已检查参数中是否包含当前 pid，避免跨进程 dump 回环。
  4. 输出应走 `DumpLog` 或 `info`，敏感内容只打印长度、id、version、错误码等摘要；涉及文本、图片或应用数据时不要默认打印正文。
  5. 动态组件或 UIExtension 需要透传到宿主 UIContent 时，复用 `NotifyUieDump → UIContentImpl::DumpInfo` 链路。

### 如何补充 ui_session_sample 验证手段

1. 在 `ui_sa_service.h` 声明 `HandleXxx`，在 `DUMP_MAP` 注册命令名到 handler。命令名应与 innerAPI 能力名一致。
2. handler 内只做参数解析、调用 `IUiContentService`、打印结构化结果。callback 类接口输出固定 tag、result、id、version、长度和结束标识，不打印正文；需要落盘时沿用 `-tofile` 模式写到 `/data/service/el1/public/ui_sa/`。
3. 需要依赖 callback timeout、watchdog 或死亡监听的能力，验证前必须先执行 `Connect`。`HandleConnect` 会把 sample 的 `eventHandler_` 传给 `IUiContentService::Connect`，后续 timeout 任务才能投递。
4. 验证脚本应覆盖正常请求、非法参数、批量输入、回调完成、超时清理、remote death 或 End/Reset 清理等分支；页面类能力还要在目标 ArkUI 应用处于前台焦点后执行。
5. 构建目标统一使用 `//foundation/arkui/ace_engine/interfaces/inner_api/ui_session:ui_session_example`。临时推送验证时同步推 `libui_sa.z.so`、`16666.json`、`ui_sa.cfg`，并确认 `hidumper -ls | grep 16666` 能看到 sample SA。

交付检查项：

1. 文档中列出 sample 命令、参数格式、预期日志 tag 和失败返回。
2. 真机验证至少包含一次 `Connect`、一次目标能力调用和一次恢复/取消路径。
3. 若命令会生成文件，验证 `/data/service/el1/public/ui_sa/` 的权限和文件名规则。
4. 若能力涉及翻译、文本或截图等敏感数据，sample 日志不得输出完整业务正文。

## 相关主题

- [多实例管理](./multi-instance-management.md) — Container/AceEngine 实例隔离与调度
- [窗口机制](./window-mechanism.md) — UIContent/AceContainer 窗口生命周期
- [子窗机制](./subwindow-mechanism.md) — SubwindowManager 子窗口管理
- [OH 平台适配层](./oh-platform-adapter.md) — adapter/ohos 入口与 UIContentImpl
