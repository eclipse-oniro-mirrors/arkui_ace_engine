# bug2.md 剩余拖拽问题的方案评估

核查基线：`b279759fa16`；第 4～7 项已单独修复并通过定向编译和所有权单测（验证范围见 `bug2_fix_validation.md`）。下文保留第 9～11 项实施前的方案评估，证据行号对应当时工作区。2026-11-19 已按用户授权实施推荐方案，实际改动和验证记录见 `bug2_drag_fix_validation.md`。

## 结论与优先级

| 条目 | 当前证据边界 | 推荐方案 | 建议顺序 |
|---|---|---|---|
| 10：后台获取拖拽数据 | 已弱捕获；后台仍 Upgrade 管理器。Pipeline 自身有 MaybeRelease 保护，不能据此认定 pipeline 跨线程析构 | 后台仅执行不依赖管理器状态的数据获取；管理器、节点只在 UI 任务内 Upgrade | 1，改动最集中 |
| 9：StopDrag 回调释放 | 监听器已有析构转投 JS 线程的处理；无执行器或投递失败时未保证转移。实际崩溃尚未复现 | 当前停止回调只捕获实例 ID，执行时按 ID 查执行器，消除监听器长期持有执行器的来源 | 2，先做局部捕获整改 |
| 11：异步快照 | pipeline、GestureEventHub、源 FrameNode 已改弱引用；dragDropInfo 仍间接持有 customNode 和 buildFunc | 将快照请求状态放在 UI 侧管理，外部回调只带弱引用、请求标识和结果；明确完成、取消、失败清理 | 3，需要完整生命周期设计 |

上述仍属源码可识别的释放边界问题。没有动态证据支持“每次都会崩溃”或“已确认可利用”的结论。

## 必须保留的判断前提

- `frameworks/core/pipeline/pipeline_base.cpp:1155`：`PipelineBase::MaybeRelease` 在非 UI 线程投递 `ArkUIDestroyPipeline`；投递成功后，最后一个 RefPtr 在后台释放不等于 pipeline 在后台析构。执行器为空、投递失败的分支需要另行证明可达性和影响，不能直接算作当前 bug。
- `frameworks/core/components_ng/base/ui_node.cpp:199`：`UINode::MaybeRelease` 也有保护，但取决于 `isThreadSafeNode_`、线程和 GC 配置，不能按 Pipeline 的保证推广到所有节点。
- `frameworks/core/components_ng/base/ui_node_gc.cpp:136`：GC 释放入口仍同步调用 `OnDelete()`，桶超限时直接删除；仅看到“启用 GC”不足以证明释放线程安全。
- `ContainerScope` 切换实例上下文，不迁移当前执行线程。

## 第 10 项：后台获取拖拽数据

### 证据

- `frameworks/core/components_ng/manager/drag_drop/drag_drop_manager.cpp:1506`：BACKGROUND 任务先升级 `weakManager`，然后升级 pipeline，调用 `RequestUDMFDataWithUDKey`。
- 同文件 `:1477`：数据获取函数只检查 udKey、通过 `UdmfClient` 创建和读取数据、处理返回码，没有访问 `DragDropManager` 实例成员。
- `frameworks/core/pipeline_ng/pipeline_context.cpp:6290`：实例 Destroy 会重置 `dragDropManager_`。因此后台持有的临时强引用可能跨越管理器主所有者的释放。
- `drag_drop_manager.cpp:142`：管理器析构包含拖拽窗口状态操作；其类没有 Pipeline 那样的 MaybeRelease 重载。最终是否形成数据竞争仍需运行验证。

### 可选方案

1. **提取与实例无关的数据获取函数（推荐）。** 提取到当前 .cpp 的内部函数，原成员函数可保留为薄封装，以免扩大接口变化。后台只持有 udKey、重试次数、输入数据、弱引用和投递所需上下文；不升级管理器。拿到结果后投递 UI 任务，在 UI 任务里升级管理器和节点，执行 DoDropAction 或调度重试。
2. **给 DragDropManager 增加 MaybeRelease。** 能统一约束最终析构，但需要管理器记录所属执行器/实例，明确实例退出时的投递失败行为，并审查所有使用方。范围更大，也不能解决后台业务访问本身的线程要求。
3. **后台末尾再投递一次空任务以转移强引用。** 只能覆盖成功投递路径；早退、取消和投递失败仍需处理，容易形成更多释放分支，不推荐。

### 推荐理由及约束

方案 1 利用数据读取本来不依赖管理器的事实，直接去掉不必要的后台强引用。保持现有最大重试次数、重试间隔、返回值处理和实例隔离；UI 侧对象不存在时，不再执行拖拽操作。Pipeline 的临时引用可继续依赖已有 MaybeRelease，不需要为此修改 Pipeline。

### 验证

正常读取成功、失败后重试、达到重试上限、后台读取期间销毁页面/实例、UI 投递失败；检查管理器不再因该后台任务在后台释放，并且销毁后不继续触发落点回调。

## 第 9 项：StopDrag 监听器释放

### 证据

- `adapter/ohos/capability/interaction/stop_drag_listener_impl.h:40`：析构函数已记录实例并将 callback 的释放投递到 JS 线程，原文“只有隐式析构、没有属主信息”的描述已过时。
- 同文件 `:45`、`:52`：依赖通过实例找到执行器；未处理 PostTask 返回 false 的情况。
- `frameworks/core/components_ng/manager/drag_drop/drag_drop_manager.cpp:1709`：当前停止回调捕获了 `taskExecutor` 的强引用；回调主体主要用于投递 UI 任务。

### 可选方案

1. **缩小当前停止回调的捕获（推荐先做）。** 仅捕获实例 ID；回调实际触发时建立 ContainerScope，按该 ID 获取容器/执行器，确认存在再投递。监听器保存的闭包不再长期持有 TaskExecutor，因此监听器在哪个线程析构不再通过这份捕获释放执行器。
2. **捕获执行器弱引用。** 改动也小，可去掉监听器长期保活；但执行时升级失败、实例逻辑销毁而执行器尚存活的情况仍需定义。只按弱引用是否过期判断，并不等于实例仍允许执行拖拽任务。
3. **引入属主线程管理的回调存储。** UI/JS 侧注册回调并在实例退出前清理，监听器只保存弱引用或 token。适合后续存在大量捕获 JS 引用、节点或其他线程敏感资源的回调，需要建立通用机制时采用。
4. **仅给析构 PostTask 加返回值检查。** 可改善日志和诊断，但失败后在当前线程 reset callback 仍没有解决释放线程问题，不能作为完整修复。

### 推荐理由及约束

先采用方案 1，仅调整本次报告命中的捕获来源，保留监听器已有正常路径保护。若以后需要支持任意包含线程敏感对象的 callback，再采用方案 3，并明确“容器退出前清理、队列关闭后不再接收回调”的约定。不能用永久泄漏回调或非属主线程强制清理替代生命周期设计。

### 验证

正常停止、服务延迟回调、回调从未触发但监听器释放、实例先销毁、执行器投递失败、多实例同时拖拽。该方案主要消除监听器长期强捕获的析构风险；不能将短暂执行期间所有对象的线程安全一并视为已证明。

## 第 11 项：异步快照回调

### 证据

- `frameworks/core/components_ng/event/gesture_event_hub_drag.cpp:1982`：外层回调已经弱捕获 pipeline、GestureEventHub、源 FrameNode，但仍按值捕获整个 dragDropInfo。
- `frameworks/core/components_ng/event/gesture_event_hub_types.h:51`：dragDropInfo 内含 `RefPtr<UINode> customNode`、`std::function<RefPtr<UINode>()> buildFunc`，后者还可能有自己的捕获对象。
- `gesture_event_hub_drag.cpp:2002`：finishCallback 只在成功进入 UI 任务后调用；外层早退和投递失败没有在这段代码里完成该清理。
- `frameworks/core/components_ng/render/adapter/component_snapshot.cpp:37`、`:630`：CustomizedCallback 保存回调并交给渲染服务接口，当前适配类没有对回调销毁线程做迁移。
- 同文件 `:506`：Create 会将离屏节点加入 Inspector 管理；清理回调涉及移除离屏节点，因此不能简单在任意回调线程调用 finishCallback。

### 可选方案

1. **拆分外部回调和 UI 请求状态（推荐）。** UI 侧持有预览节点、必要的 DragDropInfo 和清理状态；外部快照回调只保存实例/请求标识、弱引用，收到像素结果后投递 UI 任务。UI 任务检查请求仍有效，再读取状态并起拖。正常完成、失败、取消及实例退出都从属主线程清理，并保证只清理一次。
2. **局部把 customNode 改成弱捕获并清掉不需要的 buildFunc。** 改动更小；但必须先证明 Inspector/其他 UI 所有者在整个快照期间保活节点，并且 OnDragStart 不需要被清掉的数据。仅机械改为弱引用可能导致预览提前消失，不建议直接实施。
3. **给整个快照回调包装器增加析构转投 UI。** 可集中处理，但影响所有 ComponentSnapshot 调用者，还必须解决未收到回调、投递失败和实例退出的行为；不能只增加一个析构 PostTask 就宣称闭环。

### 推荐理由及约束

方案 1 同时明确节点所有权和完成清理；保留快照所需保活时间，避免让服务持有的闭包决定 UI 对象寿命。实现时应优先复用现有 UI/Inspector 请求管理，避免重复持有节点或形成环。取消和失败清理必须在实例执行器停止前完成；不能只依赖一个可能永远不执行的 finishCallback。

### 验证

同步快照失败后的异步成功、异步失败、回调不触发、回调晚于取消、页面销毁、多次重入起拖、UI 投递失败。验证离屏节点被移除一次、取消后不再起拖、没有遗留强引用，且正常拖拽预览行为不变。

## 实施建议

第 4～7 项的事件生命周期修复与定向验证已完成；剩余方案按 10 → 9 → 11 分开实施。第 10 项可以做局部改动，第 9 项先消除当前不必要的捕获，第 11 项需要覆盖完整异步请求生命周期。不要为这三项修改 Pipeline 的释放机制，也不要将未复现的条件性风险写成已确认崩溃。
