# frameChildren_ 成环导致递归异常 Issue Context

> 文档版本：v1.0
> 更新时间：2026-11-26
> 来源：`docs/context_registry.json` 主题 `FrameChildrenCycleCrash`
> 关联功能域：04-04-01

## 问题概述

`RebuildRenderContextTree(true)` 在非 `FlushRebuildRenderTree` 阶段同步更新 `frameChildren_` 时，若 UINode 树已发生结构变更（如 CAPI 父子反转）但相关节点 `frameChildren_` 尚未全部刷新，则 `frameChildren_` 树可能成环（C→B→C→B→...）。`IsOutOfTouchTestRegion` 等递归消费方遍历成环节点时无限递归，导致异常。

典型表现：
- CAPI 接口交换父子节点后，更新子组件 visibility 触发父节点 `RebuildRenderContextTree(true)`
- 父节点 `frameChildren_` 被更新（含新子节点），但新子节点 `frameChildren_` 未刷新（仍含原父节点）→ 成环
- 用户点击触发 `IsOutOfTouchTestRegion` 递归遍历 `frameChildren_` → 无限递归
- 崩溃堆栈指向 `FrameNode::IsOutOfTouchTestRegion`（`frame_node.cpp:4416-4418`）

## 关联模块

| kind | role | name | evidence | confidence |
|------|------|------|----------|------------|
| architecture | symptom_surface | FrameNode::IsOutOfTouchTestRegion | 递归遍历 frameChildren_ 无环检测 → 无限递归（`frame_node.cpp:4416-4418`） | verified |
| architecture | trigger | CAPI 节点交换（node_model.cpp AddChild/RemoveChild） | DoAddChild/RemoveChild → MarkNeedSyncRenderTree(true) | verified |
| architecture | root_cause_owner | FrameNode::RebuildRenderContextTree(true) 非 vsync 阶段同步更新 frameChildren_ | needRebuildFrameChildren=true 时更新 frameChildren_ + ProcessRenderTreeDiff | verified |
| architecture | fix_location | layout_property.cpp visibility 非 GONE 路径 | 改为 RebuildRenderContextTree(false) → 不更新 frameChildren_ | verified |
| architecture | dependency | FlushRebuildRenderTree（pipeline_context.cpp:1343） | vsync 阶段统一刷新，先于 FlushTouchEvents 和 FlushLayoutTask | verified |
| architecture | dependency | needSyncRenderTree_ 守卫（frame_node.cpp:3803） | false 时 RebuildRenderContextTree 直接返回；普通属性更新不设置此标志 | verified |

kind: `component` / `capability` / `architecture`
role: `symptom_surface` / `trigger` / `root_cause_owner` / `fix_location` / `dependency`

## 根因分类

| 根因类别 | 触发条件 | 典型场景 |
|----------|----------|----------|
| 非 vsync 阶段同步更新 frameChildren_ 导致成环 | UINode 树结构变更（CAPI 父子反转）后，`RebuildRenderContextTree(true)` 在非 `FlushRebuildRenderTree` 阶段被触发，更新部分节点 `frameChildren_`，未刷新节点仍含旧引用 → 成环 | CAPI 交换 B↔C 父子关系 + D visibility 变更 → C.frameChildren_ 含 B，B.frameChildren_ 仍含 C → 成环 |
| 递归消费方无环检测 | `frameChildren_` 递归消费方（如 `IsOutOfTouchTestRegion`）无 visited 集合或深度限制 | 任何 frameChildren_ 不一致都可能导致无限递归 |

## 排查路径

### 快速判断

1. 确认崩溃堆栈是否位于 `IsOutOfTouchTestRegion`（`frame_node.cpp:4416`）的递归调用
2. 确认崩溃前是否执行过 CAPI 节点操作（`OH_ArkUI_Node_AddChild`/`RemoveChild` 等交换/移动节点）
3. 确认 CAPI 操作是否涉及父子反转（X 原是 Y 父节点，操作后 Y 成为 X 父节点）
4. 确认崩溃前是否有属性变更触发了 `RebuildRenderContextTree`
5. 检查 frameChildren_ 是否存在成环（dump 树结构或调试日志）

## 修复方案

| 根因类别 | 修复策略 | 关键代码改动点 | 修复/缓解变更 | 关系证据 |
|----------|----------|---------------|---------------|----------|
| 非 vsync 阶段同步更新 frameChildren_ 导致成环 | visibility 非 GONE 路径传 `false`，不更新 frameChildren_ | `layout_property.cpp:1288` | 已修复 visibility 非 GONE 路径 | RebuildRenderContextTree(false) 不更新 frameChildren_ |
| 同上 | **不建议在非 vsync 阶段触发 frameChildren_ 更新**；新增调用点时应传 `false`，将 frameChildren_ 更新延迟到 FlushRebuildRenderTree 统一处理 | 全部 RebuildRenderContextTree() 调用点 | 待实施 | FlushRebuildRenderTree(line 1343) 先于 FlushTouchEvents(line 1357)，消费方读到一致 frameChildren_ |

## 预防措施

- **不建议在非 `FlushRebuildRenderTree` 阶段触发 `frameChildren_` 更新**（即 `RebuildRenderContextTree` 应传 `false`）
- 新增 `RebuildRenderContextTree` 调用点时，应传 `false`（仅同步 RSNode），将 `frameChildren_` 更新延迟到 `FlushRebuildRenderTree`
- `renderContext_->RebuildFrame(this, children)` 总是同步执行（不受 `needRebuildFrameChildren` 影响），RSNode 层级立即正确，无需依赖 `frameChildren_` 同步更新

## 相关主题

- [render-tree-sync](../../architecture/render-tree-sync.md) — 渲染树同步机制代码型 KB，覆盖 ProcessRenderTreeDiff 全链路
- [basic-render-pipeline](../../architecture/basic-render-pipeline.md) — 基础渲染管线，FlushRebuildRenderTree 在 vsync 中的位置
- [layout-framework](../../architecture/layout-framework.md) — 布局框架，SyncGeometryNode 中 RebuildRenderContextTree 调用
- [ispending-state-render-tree-diff](../lifecycle/ispending-state-render-tree-diff.md) — 同帧下树重新上树 isPendingState_ 状态错乱，相关 render tree diff 问题
