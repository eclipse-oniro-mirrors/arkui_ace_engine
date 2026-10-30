# 键盘事件目标与焦点链误判 Issue Context

> 文档版本：v1.0
> 更新时间：2026-10-29
> 来源：`docs/context_registry.json` 主题 `KeyboardEventFocusChain`
> 关联功能域：05-03-07（Scroll/嵌套滚动基类）、04-14-02（按键事件分发）

## 问题概述

按键/方向键行为与预期不符时（子组件收不到按键、按键只触发父组件、方向键不滚动或滚动顺序与嵌套滚动预期不符、Tab 走焦无效），排查者常误判为"键盘事件优先下发给父组件"或事件分发顺序缺陷，沿错误方向修改分发逻辑无效。实际按键事件是**非指向性事件（无坐标）**，其目标完全由**焦点链**决定：事件沿当前获焦链从叶到根逐一派发，与组件树父子顺序无关。本文沉淀焦点/键盘事件核心概念，用于此类问题的快速纠偏。

典型表现：
- 嵌套容器中按方向键只触发父级滚动组件，子滚动组件无响应（预期先子后父），被误读为"事件先给了父组件"
- 给组件绑定 `onKeyEvent` 回调不触发（该组件实际未获焦，不在焦点链上）
- Tab/方向键走焦完全无反应（应用未处于焦点激活态）
- 基于错误认知修改"事件下发顺序"，或在新键盘能力（如滚动组件键盘滚动）设计中假设"事件会自动先子后父"，均不生效

## 焦点与键盘事件核心概念

排查此类问题前，先对照以下概念表（锚点均已在当前源码树核实）：

| 概念 | 含义 | 代码/文档锚点 |
|------|------|--------------|
| 焦点（focus） | 指向当前界面唯一可交互元素，是非指向性输入（键盘/遥控器/旋钮）的交互基础 | [支持焦点处理](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ui/arkts-common-events-focus-event.md) |
| 焦点链（focus chain） | 某组件获焦时，从根节点到该组件路径上**所有节点都处于焦点状态**；代码上即 FocusHub 父链 | `FocusHub::GetParentFocusHub` / `GetRootFocusHub` / `GetFocusLeaf`（`focus_hub.cpp`） |
| 获焦 | 节点处于焦点链上（自身或其后代为焦点链叶节点）；"焦点链末端/叶节点"是实际处理按键的 deepest 节点 | `lastWeakFocusNode_` 递归链（`focus_event_handler.cpp`） |
| 焦点激活态（focus active） | 控制焦点框是否**显示**的开关；获焦的组件不一定显示激活态，显示激活态的必定获焦。按键走焦的前提是处于激活态 | `isFocusActive_` 字段、`SetIsFocusActive`（`focus_manager.cpp`） |
| 走焦（focus travel） | 焦点在组件间转移；主动走焦（Tab/Shift+Tab/方向键、requestFocus、clearFocus、focusOnTouch）与被动走焦（组件删除、focusable/enabled/visibility 变更、层级页面切换） | `FocusHub::HandleFocusTravel`（`focus_hub.cpp`） |
| 走焦算法 | 容器决定"下一个焦点"的规则：线性（默认，Row/Column/Flex）、投影（wrap Flex）、自定义 | `GoToNextFocusLinear` / `GetNearestNodeByProjectArea` / `RequestNextFocusByCustomAlgorithm`（`focus_hub.cpp`） |
| 层级页面（FocusView） | Page/Dialog/Sheet/Modal/Menu/Popup/NavDestination 等焦点抢占总控；走焦被限制在当前层级页面内 | `focus_manager.cpp` FocusView 栈（`focusViewStack_`）、`GetLastFocusView` |
| 整体获焦 vs 非整体获焦 | 整体获焦=容器自身作为焦点链叶获焦后再向子孙转移（层级页面切换、requestFocusById 等）；非整体获焦=叶子组件获焦、祖先跟着获焦（TextInput 主动获焦、Tab 走焦等） | 官方文档"层级页面/容器整体获焦时的焦点链"节 |
| 按键三次分发（窗口级） | ① onKeyPreIme 回调+页面快捷键 → ② 输入法 → ③ onKeyEventDispatch/onKeyEvent 回调+走焦；一旦被消费跳过后续 | [支持键盘输入事件](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ui/arkts-interaction-development-guide-keyboard.md)；isPreIme 分支与 `DispatchTabKey`（`key_event_manager.cpp`） |
| ArkUI 内按键派发 | 取当前层级页面（`GetLastFocusView`），从焦点链**叶到根**逐一派发：子（最近获焦子节点）优先，未消费才给父；全部未消费才触发走焦 | `KeyEventManager::TriggerKeyEventDispatch`（`key_event_manager.cpp`）、`OnFocusEventScope` 先递归子节点（`focus_event_handler.cpp`）、未消费进入 `HandleFocusTravel` |
| 方向键双语义 | 方向键既是普通按键（沿焦点链派发给 onKeyEvent）也是走焦键（`IsFocusStepKey` = Tab + 4 个 DPAD）；按键未被消费时才触发方向走焦 | `IsFocusStepKey`、`RequestNextFocusByKey`（`focus_hub.cpp`） |
| 焦点跳转滚动（FOCUS_JUMP） | 走焦后把新获焦节点滚入可视区的滚动来源；由 FocusHub 沿**全部可滚动祖先**逐层触发，**不走** `HandleScroll` 嵌套滚动链 | `SCROLL_FROM_FOCUS_JUMP = 7`（`scrollable_properties.h`）、`PipelineContext::FlushFocusScroll`（`pipeline_context.cpp`）、`FocusHub::ScrollByOffset`（`focus_hub.cpp`） |

## 根因分类

| 根因类别 | 触发条件 | 典型场景 |
|----------|----------|----------|
| A. 按键目标由焦点链决定，被误判为分发顺序问题 | 只看组件树父子关系，未确认当前焦点链叶节点是谁 | 嵌套滚动中仅父滚动组件获焦，方向键只触发父组件，误以为"事件优先下发给父组件" |
| B. 焦点激活态缺失 | 应用未进入焦点激活态（从未按过 Tab、被点击退出激活态） | 方向键/Tab 走焦完全无响应；焦点框不显示但组件其实已获焦 |
| C. 目标组件获焦能力/焦点链位置误解 | 目标组件不可获焦或从未处于焦点链叶（focusable 默认值、容器叶子无交互能力、WaterFlow 默认不可获焦） | 给 Text 绑 onKeyEvent 无反应；子滚动容器从未获焦，按键永远先落到别处 |
| D. 误认滚动组件会直接消费方向键/嵌套滚动自动生效 | 期望方向键直接驱动滚动并走嵌套滚动先子后父链路 | 基线代码中 scrollable 不处理 KEY_DPAD；方向键驱动滚动走的是"走焦+FOCUS_JUMP 祖先链"通道，不经过 `HandleScroll` 嵌套分发 |

> 覆盖范围说明：A–D 四类均基于当前源码树逐条核实；案例场景以嵌套滚动方向键为主线，其他键盘事件问题（快捷键、输入法抢占等）仅在三处分发概念中覆盖，不展开。

## 排查路径

### 快速判断

1. **确认事件实际到达了谁**：在可疑的父子组件上各挂 `onKeyEvent` 打日志（回调不触发 ≠ 事件没到，先确认焦点）
2. **画出当前焦点链**：确认焦点链叶节点（谁真正获焦）、链上有哪些祖先；可用 `onFocus`/`onBlur` 打点或 hidumper 焦点信息
3. **确认焦点激活态**：是否按过 Tab 激活、是否被点击退出；激活态只影响焦点框显示与按键走焦，不影响 onKeyEvent 派发
4. **确认目标组件获焦能力**：查 focusable 默认值（WaterFlow 容器为 false；Text/Image 需显式 focusable(true)；容器作为叶子且无交互不可获焦）
5. **方向键滚动场景先分辨通道**：是"组件消费按键直接滚动"，还是"走焦后 FOCUS_JUMP 滚动"；两者嵌套滚动行为完全不同

### 详细排查

#### 类别 A 排查：按键目标由焦点链决定

| 步骤 | 操作 | 预期结果 | 失败则 |
|------|------|----------|--------|
| 1 | 确认当前层级页面（`GetLastFocusView`）与焦点链叶节点 | 叶节点是实际处理按键的 deepest 获焦节点 | 叶节点不是预期组件 → 类别 C |
| 2 | 检查 `OnFocusEventScope` 递归路径 | 事件先派发给最近获焦子节点（`lastWeakFocusNode_`），子消费则父不处理 | 子组件未消费时父组件必然能收到（叶→根冒泡） |
| 3 | 若"父组件收到了事件"，确认它是作为焦点链祖先收到的（冒泡），而非"优先下发" | 父收到 = 叶节点未消费后冒泡的结果 | 纠正"父优先/子优先下发顺序"的错误心智模型 |

关键代码定位：
- `frameworks/core/common/key_event_manager.cpp` `KeyEventManager::TriggerKeyEventDispatch`：入口先取 `GetLastFocusView()`，再经 `DispatchKeyEventNG` 调 `focusNodeHub->HandleEvent(event)`
- `frameworks/core/components_ng/event/focus_event_handler.cpp` `OnFocusEventScope`：先递归 `lastWeakFocusNode_`（沿获焦链向叶），子消费则父不处理，否则自己处理——"子优先、未消费冒泡"的唯一事实来源
- `frameworks/core/components_ng/event/focus_event_handler.cpp` `OnFocusEventNode`：按键未被消费才进入 `HandleFocusTravel` 走焦

#### 类别 B 排查：焦点激活态缺失

| 步骤 | 操作 | 预期结果 | 失败则 |
|------|------|----------|--------|
| 1 | 按一次 Tab 键再按方向键 | 首次 Tab 仅激活焦点系统（不走焦），此后方向键走焦生效 | 仍无效 → 类别 C |
| 2 | 检查 `HandleFocusTravel` 开头拦截条件 | `!pipeline->GetIsFocusActive()` 时走焦直接返回 false | 确认是否被点击退出了激活态 |
| 3 | 区分"走焦无效"与"onKeyEvent 无效" | 激活态只影响走焦与焦点框；onKeyEvent 派发不依赖激活态 | onKeyEvent 也不触发 → 类别 A/C |

关键代码定位：
- `frameworks/core/components_ng/event/focus_hub.cpp` `FocusHub::HandleFocusTravel` 开头：未激活返回 false；`IsTabJustTriggerOnKeyEvent` 时仅回滚到上次焦点不走焦
- `frameworks/core/components_ng/manager/focus/focus_manager.cpp` `HandleKeyForExtendOrActivateFocus` / `ExtendOrActivateFocus`：Tab 首次按下只 `SetIsFocusActive(true)` 激活
- `frameworks/core/pipeline_ng/pipeline_context.cpp` `PipelineContext::SetIsFocusActive`

#### 类别 C 排查：获焦能力与焦点链位置

| 步骤 | 操作 | 预期结果 | 失败则 |
|------|------|----------|--------|
| 1 | 查目标组件 focusable 默认值 | Scroll/List/Grid/Column/Row 默认 true；WaterFlow 默认 false；Text/Image 需显式 focusable(true) | 不可获焦组件绑按键回调永远不触发 |
| 2 | 容器作为叶子时检查交互能力 | 无交互的叶子容器（如空 Column）即使 focusable(true) 也不可获焦；需加可获焦子节点或配置 onClick/TapGesture | 补齐交互或子节点后重试 |
| 3 | 检查 enabled/visibility | enabled(false)、Visibility.None/Hidden 均不可获焦；获焦中被置 false 会自动失焦并转移焦点 | 排除属性被动走焦干扰 |

关键代码定位：
- `frameworks/core/components_ng/event/focus_hub.cpp` `IsFocusable` / `IsFocusableScope` / `IsFocusableNode` / `IsFocusableByTab` / `IsFocusableWholePath`
- 官方文档"组件获焦能力说明"两张表（基础/容器组件 focusable 默认值）

#### 类别 D 排查：方向键驱动滚动的真实通道

| 步骤 | 操作 | 预期结果 | 失败则 |
|------|------|----------|--------|
| 1 | 在 `pattern/scrollable/` 下搜按键处理 | 基线代码中 scrollable **不消费** KEY_DPAD/KeyEvent，方向键不直接驱动滚动 | 若引入键盘滚动新能力，需自行实现按键消费与边界传递语义 |
| 2 | 观察方向键按下后的行为链 | 方向键未消费 → `RequestNextFocus` 方向走焦 → 焦点变化后 `FlushFocusScroll` → `TriggerFocusScroll` 滚动 | 走焦失败 → 类别 B/C |
| 3 | 检查 FOCUS_JUMP 滚动的嵌套行为 | `ScrollByOffset` 沿全部可滚动祖先逐层调用 scrollFunc，**不走** `HandleScroll` 嵌套滚动链（PARENT_FIRST/SELF_FIRST 不适用于焦点跳转滚动） | 需要严格嵌套滚动语义时应走显式按键消费+嵌套传递设计 |

关键代码定位：
- `frameworks/core/pipeline_ng/pipeline_context.cpp` `FlushFocusScroll` → `TriggerFocusScroll`
- `frameworks/core/components_ng/event/focus_hub.cpp` `TriggerFocusScroll` / `ScrollByOffset`：`while (parent)` 逐层祖先滚动，不消费按键
- `frameworks/core/components_ng/pattern/scroll/scroll_pattern.cpp` `OnScrollCallback` 对 `SCROLL_FROM_FOCUS_JUMP` 直接 `UpdateCurrentOffset`，绕过嵌套分派
- `frameworks/core/components_ng/pattern/scrollable/scrollable_properties.h` `SCROLL_FROM_FOCUS_JUMP = 7`
- 对照：手势/通用嵌套滚动链在 `frameworks/core/components_ng/pattern/scrollable/scrollable_pattern.cpp` `HandleScroll`（`HandleScrollParentFirst` 以 `NestedState::CHILD_OVER_SCROLL` 传父），与键盘通道互不交叉

## 修复方案

| 根因类别 | 修复策略 | 关键代码改动点 | 修复/缓解变更 | 关系证据 |
|----------|----------|---------------|---------------|----------|
| A | 让目标组件真正处于焦点链叶：`focusable(true)`/`defaultFocus`/`FocusController.requestFocus`；或父组件用 `onKeyEventDispatch`+`UIContext.dispatchKeyEvent(id, event)` 重分发到指定子组件 | 应用侧属性/接口调整，引擎无改动 | 无公开变更（机制澄清） | master 源码行为，见类别 A 关键代码定位 |
| B | 进入焦点激活态：首次按 Tab 激活，或 `FocusController.activate(true)`；注意触屏/鼠标点击会退出激活态 | 应用侧调用 | 无公开变更（机制澄清） | `focus_manager.cpp` 激活态链路 |
| C | 按获焦能力三分类补齐：显式 focusable、为叶子容器添加可获焦子节点或 onClick/TapGesture | 应用侧调整 | 无公开变更（机制澄清） | 官方文档获焦能力表 |
| D | 键盘滚动类新能力设计原则：在获焦滚动组件的按键处理中**显式消费**方向键并驱动自身滚动；到边界后的"先子后父"需显式走嵌套滚动传递（或消费语义交由冒泡），不能假设事件会自动先子后父 | 滚动组件 Pattern 按键处理（新能力范畴） | 开发中，未合入 | 基线 `pattern/scrollable/` 无按键处理（本页核实）；FOCUS_JUMP 通道见类别 D |

## 关联变更

| 变更编号 | 变更简述 | 根因类别 | 变更关系 | 证据 | 确信度 |
|----------|----------|----------|----------|------|--------|
| CHG-01 | 滚动组件键盘滚动能力开发中踩中本页误区：将"仅父组件获焦导致方向键只滚动父组件"误判为"键盘事件优先下发给父组件"，排查方向偏移后返工 | D | related | 案例为开发者复盘口述（已脱敏）；误判机制本身已对照 master 源码逐条验证 | user_claimed |

> 本页为概念澄清与机制导航型 KB，无引入/修复本问题的公开 PR；`enableScrollWithKeyboard` 等键盘滚动 API 合入后应回填此处并补充其嵌套滚动语义说明。

## 预防措施

- **排查键盘问题第一步先画焦点链**：叶节点是谁、链上有哪些祖先、激活态与否；在确认焦点状态前，不要对"事件下发顺序"下结论
- **牢记方向键双语义**：按键沿焦点链派发，未消费才走焦；走焦后的 FOCUS_JUMP 滚动沿祖先链逐层触发，不经过 `HandleScroll` 嵌套滚动分发
- **区分获焦与焦点框**：获焦组件不一定显示激活态；走焦无响应先查激活态，onKeyEvent 无响应先查获焦
- **新键盘能力开发前通读两份官方文档**：《支持焦点处理》《支持键盘输入事件》，再读 `focus_event_handler.cpp` 的派发递归
- **AI 辅助开发验证清单**：结论"事件先到了 X 组件"必须有打点证据（onKeyEvent/onFocus 日志），禁止由"父组件行为先发生"反推分发顺序

## 相关主题

- `docs/kb/issues/interaction/nested-scroll-offset-no-response.md` — 嵌套滚动偏移量不响应（同目录问题 KB，手势通道嵌套滚动）
- `docs/kb/capabilities/text-shortcuts.md` — 按键事件分发与文本快捷键源码路由（`key_event.cpp`）
- `docs/kb/components/container/scroll.md` — Scroll 组件 KB（ScrollablePattern 路由）
- `specs/05-ui-components/03-scroll-container-components/07-scroll` — Scroll/嵌套滚动 Spec 域
- [支持焦点处理（OpenHarmony docs）](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ui/arkts-common-events-focus-event.md)
- [支持键盘输入事件（OpenHarmony docs）](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ui/arkts-interaction-development-guide-keyboard.md)
