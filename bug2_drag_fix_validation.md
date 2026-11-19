# bug2.md 第 9～11 项修复记录

## 基线及范围

先提交上一批已验证修改：`b1ef6e34fab`（初始提交号 `142085a2869`；提交信息补充 AI 统计、署名和 Change-Id 后更新，树内容一致）（`fix: retain native gesture and touch events for JS object lifetime`），12 个文件，+418/-10；该批验证见 `bug2_fix_validation.md`。

本轮按 10 → 9 → 11 实施之前推荐的局部所有权调整。任务分类为拖拽稳定性；已查询 drag KB 并阅读 `docs/kb/architecture/drag-framework.md`。使用 `openharmony-cpp`、`openharmony-build` 和 `ohos-test-ut-generation` 工作流。

## 修改内容

- 第 10 项：将 UDMF 数据读取提取为 cpp 内部函数，保留原成员函数作为薄封装。后台任务不再 Upgrade manager，只有 UI 落点/重试任务升级管理器和节点。仍最多重试 3 次、间隔 800 ms。
- 第 9 项：StopDrag 服务回调仅捕获 instanceId，执行时查找容器和执行器；不再由监听器长期强持有 executor。保留原监听器析构保护。
- 第 11 项：DragDropManager 的请求集合在 UI 侧持有 DragDropInfo、事件及清理函数；服务回调和投递的 UI 闭包只携带弱引用及像素结果。在 UI 完成时先移出集合、清理离屏节点，再继续起拖；重复结果无效。
- 手势重新开始、取消、结束、EventHub 离树/上下文解绑、GestureEventHub 析构及 Pipeline Destroy 均取消对应请求。服务不回调也能由这些生命周期出口释放预览状态；取消后的结果不再起拖。
- 增加内部 `ComponentSnapshot::CreateWithCleanup`，在 UI 创建离屏节点时同步注册实际快照节点的清理函数，兼容非 FrameNode 构建器生成 Stack 包装节点的情况。原 `Create` 签名保留并委托新实现，其其他调用者行为保持不变。

## 明确的边界

- 不修改 `PipelineBase::MaybeRelease`。Pipeline 的跨线程临时强引用仍依赖已有释放保护，不能把它本身认定为跨线程析构缺陷。
- UI 投递失败、执行器不存在时，不在服务线程释放 UI 状态；由之后的手势取消/结束、重新发起或 Pipeline 销毁清理。如果这些出口尚未发生，状态继续保留在 UI 请求集合中。本轮不引入新的超时策略。
- 第 9 项消除当前 StopDrag 闭包的长期执行器捕获，不宣称所有外部回调的瞬时容器/执行器访问均已证明线程安全。
- 请求清理函数只消费一次；ComponentSnapshot 原有图像加载失败分支也可能执行 Inspector 移除，因此不将“Inspector RemoveOffscreenNode 只调用一次”作为保证。
- 未修改 `interfaces/native/`、`interfaces/napi/` 的签名、语义或 ABI；未新增依赖，未编辑生成代码；保留已有未跟踪分析资料。

## 验证

生产代码按 `out/rk3568/compile_commands.json` 的原始参数定向编译，输出到 `/tmp`，涵盖 6 个变更 cpp 的 21 个有效配置（排除同名旧 pipeline 文件）。全部成功、无编译警告；包含启用 PIXEL_MAP_SUPPORTED 的实际快照实现。命令 `python3 /tmp/bug2_compile_drag.py`，日志 `/tmp/bug2_compile_drag_final.log`。

首次编译发现新 cleanup lambda 的 AceType 限定遗漏，已修正并复验；初始脚本还错误匹配同名 legacy pipeline_context.cpp，已限制路径。

过程检查：新增 EventHub 解绑处理后，追加编译 EventHub、拖拽手势实现及新测试文件，共 11 个配置全部成功、无警告；日志 `/tmp/bug2_compile_drag_detach.log`。两个变更测试文件另已定向编译成功，日志 `/tmp/bug2_compile_drag_test.log`。

复查确认 `DragNodeDetachFromParent` 是文本预览菜单清理入口，并非通用源节点离树。因此取消逻辑放在 `EventHub::OnDetachClear` 和 `OnDetachContext`，没有改动文本预览清理行为；新增测试覆盖这一区分。

单测构建（OpenHarmony 根目录）：

```bash
./build.sh --export-para PYCACHE_ENABLE:true --product-name rk3568 --build-target drag_drop_manager_test_ng --ccache
./build.sh --export-para PYCACHE_ENABLE:true --product-name rk3568 --build-target drag_drop_manager_test_ng --ccache --fast-rebuild
```

首次从组件目录调用构建脚本导致 GN 导入检查失败；从 OpenHarmony 根目录重跑后成功。因随后补充离树处理与请求清理，执行第二条命令确保产物包含最终修改；本轮未再次修改 GN 配置。日志 `/tmp/bug2_drag_build.log`、`/tmp/bug2_drag_rebuild.log`。构建系统包含既有 GN/Ninja 警告；变更源码定向编译未产生警告。

先行设备检查：首轮二进制运行后台读取、重试策略及两条 StopDrag 回归，4 passed、0 failed。日志 `/tmp/bug2_drag_smoke.log`。最终二进制重新发送到设备后运行：

```bash
hdc shell 'LD_LIBRARY_PATH=/system/lib/platformsdk:/system/lib /data/local/tmp/ace_bug2_drag_20261119/drag_drop_manager_test_ng --gtest_filter=DragDropLifetimeTest.*:DragDropManagerTestNgCoverage.DragDropManagerTestNgCoverage065* --gtest_output=xml:/data/local/tmp/ace_bug2_drag_20261119/results.xml'
```

两次正式构建均退出 0、显示 `build successful`。最终设备结果：20 passed、0 failed、0 errors、0 disabled；新增生命周期用例 10 个，既有停止动画相关用例 10 个。已取回并解析 XML `/tmp/bug2_drag_results.xml`；终端日志 `/tmp/bug2_drag_device_test.log`。

新增用例覆盖完成后释放捕获、重复完成、取消后迟到结果、清理重入、服务无响应时销毁、手势结束/取消、节点离树/上下文解绑与预览菜单清理的区别、真实工作线程阻塞取数期间 UI 释放 manager、executor 替换和实例消失、原重试间隔/上限、正常停止动画的 UI 调度及去重。

验证边界：单测使用现有 Pipeline/渲染服务 mock，未执行真实 RS 异步快照或完整 UI 拖拽端到端场景；实际 Pipeline Destroy 的接入经源码检查和生产编译，不能将 mock 下的管理器取消测试等同于真实 Pipeline 销毁测试。未执行完整 ace_engine 整库链接。

`git diff --check` 通过。

## 修改文件统计

以下统计相对第一批提交，不含本验证记录自身。本轮修复、测试与验证记录一并提交。

| 文件 | 新增 / 删除行 |
|---|---|
| `bug2_remaining_solutions.md` | +1 / -1 |
| `frameworks/core/components_ng/event/event_hub.cpp` | +6 / -0 |
| `frameworks/core/components_ng/event/gesture_event_hub.cpp` | +4 / -1 |
| `frameworks/core/components_ng/event/gesture_event_hub.h` | +5 / -0 |
| `frameworks/core/components_ng/event/gesture_event_hub_drag.cpp` | +63 / -30 |
| `frameworks/core/components_ng/manager/drag_drop/drag_drop_manager.cpp` | +57 / -15 |
| `frameworks/core/components_ng/manager/drag_drop/drag_drop_manager.h` | +10 / -0 |
| `frameworks/core/components_ng/render/adapter/component_snapshot.cpp` | +15 / -0 |
| `frameworks/core/components_ng/render/adapter/component_snapshot.h` | +3 / -0 |
| `frameworks/core/pipeline_ng/pipeline_context.cpp` | +3 / -0 |
| `test/unittest/core/manager/BUILD.gn` | +1 / -0 |
| `test/unittest/core/manager/drag_drop_manager_test_ng_coverage.cpp` | +4 / -0 |
| `frameworks/core/components_ng/manager/drag_drop/drag_snapshot_request.h` | +87 / -0 |
| `test/unittest/core/manager/drag_drop_lifetime_test_ng.cpp` | +340 / -0 |
