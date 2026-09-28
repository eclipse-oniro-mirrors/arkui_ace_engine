# WaterFlow header/footer/sticky draftCode — 20260809915342

目标版本：OpenHarmony 7.1，ArkTS since `26.1.0`。规格基线：[design-docs PR 785](https://gitcode.com/OpenHarmonyAI/design-docs/pull/785)。

2026-09-28 用户授权按既有 SDD 规格和计划实施，并明确当前没有构建环境，允许先提交，在另一服务器构建验证。本提交是待验证的 draftCode，不是功能验收或发布完成声明。

## 配套仓与基线

| 仓 | 开发分支 | 开始实施时的基线 |
|---|---|---|
| arkui_ace_engine | codex/waterflow-header-footer-sticky | 158746c316d4481ffa5483fd45884644390ff926 |
| interface_sdk-js | codex/waterflow-header-footer-sticky | bc904c119fd6606cca84b62990558139b0d7b11c |

两仓必须配套使用。静态声明新增 header/footer/sticky 后，服务器需要运行原有 `frameworks/bridge/arkts_frontend/arkoala_generator` 生成链，生成新的 WaterFlow modifier 函数表和 ArkTS peer，再编译。不要使用仍缺少三个槽位的旧 generated 头文件。本地未运行生成器、未手改生成文件。

## 本轮源码

- 独立 WaterFlow 的普通头尾；新 footer 接管、undefined 清空和异步 builder generation 校验；普通、分段、滑窗布局的槽位映射及测量。
- 外层 Scroll/List/滚动 Grid/WaterFlow 可视区域吸附；单 Pipeline 弱引用登记，父节点只移位也刷新几何。遇到不支持的最近滚动父组件不借用更外层；非滚动 Grid 继续作为布局中间层。
- 父驱动懒加载与 sticky 分开判断；有效 ViewPosReference 决定测量范围，沿用半屏预测规则；按来源维护中间节点请求，父级补偿仅消费一次。
- 动态 ArkTS 的声明式入口、attributeModifier 与延迟代理；静态 modifier 接线；SDK 中英文动态和静态声明。WaterFlowModifier 通过现有继承/implements 获得接口，未重复声明。
- 方向切换时隐藏不支持方向的新边，独立场景不吸附；未使用新接口的旧 footer 路径保持原分支。没有修改公共 C API、generated 文件或增加系统模块依赖。

## 实际检查

| 检查 | 实际结果 |
|---|---|
| Node v24.16.0，`node --test test/unittest/bridge/declarative_frontend/ark_component/water_flow_edges_test.mjs` | 6 tests，6 passed，0 failed；检查动态入口、链式调用、undefined 透传与 reset |
| `node --check frameworks/bridge/declarative_frontend/ark_component/components/arkwaterflow.js` | PASS，仅 JavaScript 语法 |
| 两仓 `git diff --check` | PASS；新增代理段的 CRLF 空白告警已修复，未规范化整份旧文件 |
| SDK 声明合同检查 | 4 个文件、12 个签名，header/footer 为 CustomBuilder 或 undefined、sticky 为 StickyStyle 或 undefined、返回 this、since 26.1.0，两范式与中文副本一致 |
| SDD 全量门禁 | 44 项检查，2 项 Level D 未闭合：manifest 为 implementing，任务与 native AC 未完成；不声明可归档 |
| 公共 C API / generated 差异检查 | 无修改；不代表二进制构建或 ABI 检测已执行 |
| C++/ArkTS 编译、正式 CodeCheck | NotRun：本地无构建环境，用户允许转服务器验证 |
| C++ gtest | NotRun；新增 12 个 `WaterFlowHeaderFooterTest.WFHF_*` 用例源码，加入三个 WaterFlow 测试目标的 COMMON_SRC |
| SDK 静态生成、静态类型检查、示例构建 | NotRun，须使用配套 SDK 刷新生成物 |
| 设备、读屏/键鼠、性能和 SpecTest | NotRun，不能用 JS 检查代替 |

没有修改 Python 源码或脚本。以上 Node 测试不执行 native 布局，也不证明 25 项 AC 已通过。

## 服务器验证

1. 应用两个仓库的配套提交，使用本次 SDK 输入执行标准静态代码生成和全量 ACE 构建。生成目标为 `//foundation/arkui/ace_engine/frameworks/bridge/arkts_frontend/arkoala_generator:runner_gen_code`。避免跳过 SDK/生成步骤的增量构建。
2. 在 OpenHarmony 根目录按现有环境运行 `./build.sh --product-name rk3568 --build-target ace_engine`；需要 SDK 变体时运行 `./build.sh --product-name ohos-sdk --build-target ace_engine`。本地均未执行。
3. 构建目标目录 `//foundation/arkui/ace_engine/test/unittest/core/pattern/waterflow` 下的 `water_flow_test_original`、`water_flow_test_segmented`、`water_flow_test_sw`。仅运行本次构建产出的可执行文件，先用 `--gtest_filter=WaterFlowHeaderFooterTest.*`，预期每个目标 12 个用例全部通过；再跑各自全部回归。记录实际二进制路径、时间、过滤器和通过/失败数。
4. 构建 ScrollableComponentTest，打开 `pages/waterflow/WaterFlowHeaderFooterDraft`。页面提供 Scroll、List、Grid、WaterFlow、独立五种场景，以及 sticky、布局模式、头尾清除和点击检查。示例使用 ForEach，不作为懒创建数量证明。
5. 用 LazyForEach/Repeat 的构建回调补齐父驱动首屏及上下半屏 idle 记录；按下表复核剩余矩阵，尤其多列、分组、跨行、重新挂载、尺寸变化、snap、chainAnimation 与静态 ArkTS。

## 验收覆盖与待补证据

| AC / 任务 | 当前落点 | 待补 |
|---|---|---|
| AC-01/02/10/11/12，TASK-1 | EdgeState、统一映射、模型、三布局入口；FlowAndIndices/Empty/StaleBuilder/DetachInvalidatesBuilder/FooterTakeover | 原有 constructor 优先级与 Sections 组合、数据增删、Scroller/事件全回归 |
| AC-03/04/05/06/08/09/17/18/19，TASK-4 | Pipeline coordinator 和父视口几何；Standalone/ParentOnlyMove/ExplicitZeroZIndex | 四父类实际布局、嵌套/裁剪/跨列/交互及设备矩阵 |
| AC-07/13/14，TASK-3 | 三布局、DirectionReset；测试源共用三个目标 | 三目标实际编译和全部回归，短/空/超高/只一边参数矩阵 |
| AC-15，TASK-2 | 动静态 SDK、bridge、modifier；JS 6/6 | 静态生成、SDK 编译、两范式一致性、非法枚举 native 检查 |
| AC-16，TASK-6 | 单份真实节点、框架包装层、可访问性集合计数 | 键盘焦点、读屏顺序、命中、回收后无残留的实机验证 |
| AC-20/21/22/23/24/25，TASK-5 | 父范围、deadline、独立模式隔离；共享请求/一次补偿/半屏规则用例 | 父懒加载完整矩阵、预测截止恢复、真实构建数量和性能证据 |
| TASK-7 | 本报告与本地 SDD 执行记录 | 独立评审、正式检查、spec-for-validation、metadata_tracking 及最终归档 |

TASK-1～5 处于 DraftCode，TASK-6 为 DraftTests/部分脚本检查通过，TASK-7 未闭合。全部 native AC 的验证状态均为 Pending；不得把本提交当作 Approved/Done。服务器失败需区分新增问题、既有基线和环境问题，并回填 SDD。

## 实际文件清单与变更行数

统计为相对上述实施基线的新增/删除行；本交接文档自身不计入下表。

| 仓 / 文件 | 新增 | 删除 |
|---|---:|---:|
| engine: `examples/ScrollableComponentTest/entry/src/main/ets/pages/waterflow/WaterFlowHeaderFooterDraft.ets` | 101 | 0 |
| engine: `examples/ScrollableComponentTest/entry/src/main/ets/pages/waterflow/waterFlowIndex.ets` | 1 | 0 |
| engine: `examples/ScrollableComponentTest/entry/src/main/resources/base/profile/main_pages.json` | 1 | 0 |
| engine: `frameworks/bridge/declarative_frontend/ark_component/components/arkwaterflow.js` | 56 | 0 |
| engine: `frameworks/bridge/declarative_frontend/ark_component/src/ArkWaterFlow.ts` | 46 | 0 |
| engine: `frameworks/bridge/declarative_frontend/ark_modifier/src/water_flow_modifier.ts` | 15 | 0 |
| engine: `frameworks/core/components_ng/layout/layout_property.cpp` | 20 | 3 |
| engine: `frameworks/core/components_ng/layout/layout_property.h` | 3 | 0 |
| engine: `frameworks/core/components_ng/pattern/lazy_layout/lazy_layout_utils.cpp` | 23 | 0 |
| engine: `frameworks/core/components_ng/pattern/lazy_layout/lazy_layout_utils.h` | 2 | 0 |
| engine: `frameworks/core/components_ng/pattern/list/list_layout_algorithm.cpp` | 1 | 19 |
| engine: `frameworks/core/components_ng/pattern/scroll/scroll_layout_algorithm.cpp` | 1 | 7 |
| engine: `frameworks/core/components_ng/pattern/scrollable/scroll_viewport_coordinator.h` | 65 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/bridge/waterflow/arkts_native_water_flow_bridge.cpp` | 105 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/bridge/waterflow/arkts_native_water_flow_bridge.h` | 7 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/bridge/waterflow/water_flow_static_modifier.cpp` | 55 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_info_sw.cpp` | 7 | 4 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_info_sw.h` | 2 | 2 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_sw.cpp` | 41 | 6 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_algorithm.cpp` | 20 | 4 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_algorithm.h` | 1 | 1 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_info.cpp` | 10 | 8 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_segmented_layout.cpp` | 35 | 15 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_algorithm_base.cpp` | 155 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_algorithm_base.h` | 5 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_info_base.h` | 23 | 4 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_utils.cpp` | 2 | 19 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_accessibility_property.cpp` | 1 | 1 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_edge_state.h` | 62 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_ng.cpp` | 26 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_ng.h` | 3 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_static.cpp` | 18 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_static.h` | 3 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_parent_context.h` | 77 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_pattern.cpp` | 285 | 5 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_pattern.h` | 19 | 0 |
| engine: `frameworks/core/pipeline_ng/pipeline_context.cpp` | 15 | 0 |
| engine: `frameworks/core/pipeline_ng/pipeline_context.h` | 4 | 0 |
| engine: `test/mock/frameworks/core/pipeline/mock_pipeline_context.cpp` | 9 | 0 |
| engine: `test/unittest/bridge/declarative_frontend/ark_component/water_flow_edges_test.mjs` | 68 | 0 |
| engine: `test/unittest/core/pattern/waterflow/BUILD.gn` | 1 | 0 |
| engine: `test/unittest/core/pattern/waterflow/water_flow_header_footer_test.cpp` | 296 | 0 |
| SDK: `api/@internal/component/ets/water_flow.d.ts` | 39 | 0 |
| SDK: `api/arkui/component/waterFlow.static.d.ets` | 40 | 1 |
| SDK: `zh-cn/api/@internal/component/ets/water_flow.d.ts` | 39 | 0 |
| SDK: `zh-cn/api/arkui/component/waterFlow.static.d.ets` | 40 | 1 |
