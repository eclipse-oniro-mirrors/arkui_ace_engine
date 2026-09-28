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
- 方向切换时隐藏不支持方向的新边，独立场景不吸附；未使用新接口的旧 footer 路径保持原分支。源码提交未修改公共 C API、不包含 generated 文件，也未增加系统模块依赖。

## 初次提交检查（后续复验见下文）

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
2. 在 OpenHarmony 根目录使用构建技能的 wrapper 启动并监控 `./build.sh --product-name rk3568 --build-target ace_engine`；需要 SDK 变体时使用 `./build.sh --product-name ohos-sdk`，不添加 `--build-target`。组件构建会先执行配套 SDK 构建。
3. 构建目标目录 `//foundation/arkui/ace_engine/test/unittest/core/pattern/waterflow` 下的 `water_flow_test_original`、`water_flow_test_segmented`、`water_flow_test_sw`。服务器直接运行需使用 `host_product --no-prebuilt-sdk`，以目标名选择构建，先核对 Host UT 技能所需的构建技能版本和状态跟踪脚本。仅运行本次构建产出的可执行文件，先用 `--gtest_filter=WaterFlowHeaderFooterTest.*`，预期每个目标 17 个用例全部通过；再跑各自全部回归。记录实际二进制路径、时间、过滤器和通过/失败数。
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

## 第一轮 PR 评论整改（2026-09-28）

针对实现仓 PR #89677 评论 191847721 的头文件依赖增长问题，仅调整内部依赖与存储组织：

- 将纯数据索引映射放回 `water_flow_layout_info_base.h`，边状态和父请求所有权声明放回 `water_flow_pattern.h`，移除两个将重依赖传递给下游的独立头文件。
- List 父资格判断、请求清理和默认 StickyStyle 初始化移至 `water_flow_pattern.cpp`；Pattern 默认构造也在该翻译单元实现，避免消费者依赖内部辅助类型的初始化符号。
- `StickyStyle` 使用匹配原定义的 `uint32_t` 前置声明；实际使用枚举值或 HeaderFooterUtils 的实现文件显式包含其定义。布局状态仅保存首尾两个补偿量，消费边界仍返回原 `AdjustOffset`，并同时清零。
- 未增加堆分配、系统依赖、生成物或应用侧 API；状态默认值、generation、父资格规则及补偿消费顺序保持原行为。

本地轻量扫描只递归当前仓可解析的引号 include，并计入全部宏分支，不代替编译器或正式门禁。三个被点名文件的仓内可达头文件数量变化如下：

| 文件 | 修改前 | 修改后 |
|---|---:|---:|
| `water_flow_layout_info_base.cpp` | 174 | 131 |
| `water_flow_layout_info_sw.cpp` | 158 | 111 |
| `water_flow_layout_info.cpp` | 155 | 108 |

Node 入口回归仍为 6/6；12 个 C++ 用例尚未运行，仅同步其中的补偿量设置。独立头文件编译、全量构建、正式头文件门禁复扫和 native 测试均未在本地执行，沿用用户转服务器验证的安排。

评论中的人工检视不足、TDD/XTS/L0 未完成仍待处理，未把未运行项改为通过或不涉及；PR 保持 Draft，检视闭环状态为“否”。接口仓 #36148 仅有 CI 欢迎提示，本轮不修改 SDK。

## 第二轮评论核对和服务器验证（2026-09-28）

实现仓 PR #89677 的头文件引用计数检查已于 11:16:51 通过（评论 191854620，对应 `e54aba52119b`）。随后读取到的六条代码意见按真实调用路径处理：

| 评论 | 核对与处理 |
|---|---|
| 191853749 | `BeforeCreateLayoutWrapper` 本身不调用 `BeginEdgeUpdate`，但准备 builder 时提前改树的问题成立。旧 footer 的包装和 edge 模式启用移至 generation 校验后的提交阶段；单独设置 sticky 也不再递增 header generation。 |
| 191853728 | 原 `Flush` 已先收集快照再执行，生产回调只捕获弱引用；补充递归 Flush 保护，并测试回调内注销、重新注册和递归调用。 |
| 191853732 | 原 `SetEdgeGeometry` 已对 host 判空；登记入口额外拒绝空节点和空回调。 |
| 191853740 | `FrameNode` 在声明处创建 GeometryNode，Pipeline 在布局后刷新吸附。只对有效滚动视口或开启 clipEdge 的祖先应用边界，零高度裁剪应禁止吸附；新增从零高度到有效尺寸恢复的用例，未跳过合法的零尺寸裁剪。 |
| 191853746 | `CreateChildConstraint` 清空子节点 selfIdealSize/minSize；头尾使用 MATCH_CONTENT，只有横轴被指定宽度，主轴按内容测量。保留不设高度和超视口内容的语义，补充无显式高度的容器回归；纯 Text 的实际排版仍待运行验证。 |
| 191853757 | 两个 detach 入口共用 `InvalidateGenerations`；保留每次生命周期失效的单调 token，不假定两个回调必成对出现。 |

本轮新增 5 个 C++ 用例，总数 17；尚无 native 执行通过证据。Node 入口检查在服务器 Node 18.20.1 与本地 Node 24.16.0 均为 6 passed / 0 failed。

服务器工作区为 `/home/yanshuifeng/workspace/OpenHarmony`。引擎 `e54aba52119b` 与接口 `4e4842057506` 已通过 Git bundle 同步到 `codex/waterflow-header-footer-sticky`；原 `0923` 分支及其他 SDD 未提交内容保留，并核对文件摘要一致。第一轮配套 SDK 构建完成，组件编译暴露 `AdjustOffset` 被基类同名成员遮蔽，已将类型引用改为完整命名空间。

标准生成链已产生三个新 modifier 槽位。静态 WaterFlow peer 原本被 `instructive-config.json` 排除部署，且 `arkgen-config.json` 的 options hook 使用了大小写不匹配的类名；本轮修正生成源配置，保留旧 options hook，通过生成链部署新方法，不手改 generated 文件。

第二轮组件构建进入静态 ArkTS 编译后失败：`WaterFlowModifier` 的 6 处调用找不到 peer 的新方法。部署后的源 peer 已包含这三个方法，但预处理副本仍旧。生成的 `toolchain.ninja` 证实，`arkoala_annotate` 和 `arkoala_process` 的自定义输出与 GN 自动目标 stamp 同名，分别生成重复规则和自身依赖，未触发预处理更新。两个 action 的输出及脚本参数改用 `_output.stamp`，保留现有依赖关系，不新增依赖、不清理缓存。该构建规则问题也存在于实施基线，本次新 peer 使陈旧副本问题显现。

第三轮已完成框架静态 ArkTS 的 `components_compile_abc`，生成 7,080,064 字节 `modules_static.abc`，原 6 处缺方法错误消失。后续 C++ 编译发现 `GetCalcLayoutConstraint()` 返回的 unique_ptr 被复制，以及动态 bridge 调用了不存在的 `NumberValue`；分别改用常量引用和项目既有 `ToNumber(vm)->Value()`。第四轮这两个编译单元均成功产出新的目标文件。

第四轮链接 `libace_compatible` 时发现公共布局工具反向引用 WaterFlow 组件库的 `ConsumeParentAdjustOffset`。该小型状态消费逻辑改为内联，只增加无外部 include 的 `lazy_layout_offset.h`，保留先读取再清零和 parentDriven 判断；未增加虚函数或链接依赖。`libace.map` 按实际跨组件调用补齐 `LayoutProperty::SetLazyLayoutRequest`、`LazyLayoutUtils::ConsumeNestedLazyAdjustOffset`、`PipelineContext::GetScrollViewportCoordinator` 三个内部方法的导出。第五轮公共库链接成功，并用 `llvm-nm -D -C` 验证这三个符号实际导出。

第五轮 WaterFlow 库链接还暴露 `HeaderFooterUtils::MeasureHeaderFooter` 未导出，以及 List/Grid/WaterFlow 的 V2 标签常量不可见。补齐测量函数白名单；按现有组件化源码约定采用局部 List/Grid 标签常量，并复用 `water_flow_constants.h` 的 WaterFlow 标签，避免新增全局常量导出。

**第六轮完整组件构建成功**：2026-09-28 12:46:27，PID 376612，907/907 个任务，耗时 4 分 08 秒；已核对进程结束及最终 `rk3568 build success` / `build successful` 标记。实际命令由构建技能 wrapper 启动：`./build.sh --export-para PYCACHE_ENABLE:true --product-name rk3568 --build-target ace_engine --ccache`。新产物为 `libace_compatible.z.so`（40,795,692 字节）、`libarkui_waterflow.z.so`（674,672 字节）与 `modules_static.abc`（7,080,064 字节）；四个内部方法均通过 `llvm-nm -D -C` 确认导出。9 个修改代码/配置/构建输入与本地摘要一致。

该轮原生编译日志有 2 条 warning，未定位到本次修改文件；未运行基线全量对照，不声称全库零告警。构建后检查仍提示组件输出缺少 init/SELinux 配置，不作为完整系统镜像校验通过。17 个 native 用例仍未编译/运行；Node 6/6、组件编译成功和功能验收分别记录。

构建日志和原始生成差异保存在服务器 `/home/yanshuifeng/workspace/waterflow-validation-20260928/`。本地完整 SDD 校验仍为 44 项、2 项 Level D 未闭合；本报告不声明功能验收完成。Host UT 暂未启动：当前构建技能为 0.2.0，Host UT 技能要求至少 0.3.0 及 `build_state.py`、`run_build_session.sh`，两机均缺少这两个脚本。

## 实际文件清单与变更行数

统计为相对上述实施基线的新增/删除行；本交接文档自身不计入下表。

| 仓 / 文件 | 新增 | 删除 |
|---|---:|---:|
| engine: `build/libace.map` | 4 | 0 |
| engine: `examples/ScrollableComponentTest/entry/src/main/ets/pages/waterflow/WaterFlowHeaderFooterDraft.ets` | 101 | 0 |
| engine: `examples/ScrollableComponentTest/entry/src/main/ets/pages/waterflow/waterFlowIndex.ets` | 1 | 0 |
| engine: `examples/ScrollableComponentTest/entry/src/main/resources/base/profile/main_pages.json` | 1 | 0 |
| engine: `frameworks/bridge/arkts_frontend/arkoala_generator/generation_config/arkgen-config.json` | 1 | 1 |
| engine: `frameworks/bridge/arkts_frontend/arkoala_generator/generation_config/instructive-config.json` | 0 | 1 |
| engine: `frameworks/bridge/arkts_frontend/koala_projects/arkoala-arkts/BUILD.gn` | 6 | 4 |
| engine: `frameworks/bridge/declarative_frontend/ark_component/components/arkwaterflow.js` | 56 | 0 |
| engine: `frameworks/bridge/declarative_frontend/ark_component/src/ArkWaterFlow.ts` | 46 | 0 |
| engine: `frameworks/bridge/declarative_frontend/ark_modifier/src/water_flow_modifier.ts` | 15 | 0 |
| engine: `frameworks/core/components_ng/layout/layout_property.cpp` | 20 | 3 |
| engine: `frameworks/core/components_ng/layout/layout_property.h` | 3 | 0 |
| engine: `frameworks/core/components_ng/pattern/lazy_layout/lazy_layout_utils.cpp` | 23 | 0 |
| engine: `frameworks/core/components_ng/pattern/lazy_layout/lazy_layout_utils.h` | 2 | 0 |
| engine: `frameworks/core/components_ng/pattern/list/list_layout_algorithm.cpp` | 1 | 19 |
| engine: `frameworks/core/components_ng/pattern/scroll/scroll_layout_algorithm.cpp` | 1 | 7 |
| engine: `frameworks/core/components_ng/pattern/scrollable/scroll_viewport_coordinator.h` | 74 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/bridge/waterflow/arkts_native_water_flow_bridge.cpp` | 106 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/bridge/waterflow/arkts_native_water_flow_bridge.h` | 7 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/bridge/waterflow/water_flow_static_modifier.cpp` | 56 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_info_sw.cpp` | 7 | 4 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_info_sw.h` | 2 | 2 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_sw.cpp` | 41 | 6 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_algorithm.cpp` | 20 | 4 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_algorithm.h` | 1 | 1 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_info.cpp` | 10 | 8 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/top_down/water_flow_segmented_layout.cpp` | 35 | 15 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_algorithm_base.cpp` | 157 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_algorithm_base.h` | 5 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_info_base.h` | 39 | 4 |
| engine: `frameworks/core/components_ng/pattern/waterflow/layout/water_flow_layout_utils.cpp` | 2 | 19 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_accessibility_property.cpp` | 1 | 1 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_ng.cpp` | 27 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_ng.h` | 3 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_static.cpp` | 18 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_model_static.h` | 3 | 0 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_pattern.cpp` | 345 | 5 |
| engine: `frameworks/core/components_ng/pattern/waterflow/water_flow_pattern.h` | 76 | 0 |
| engine: `frameworks/core/pipeline_ng/pipeline_context.cpp` | 15 | 0 |
| engine: `frameworks/core/pipeline_ng/pipeline_context.h` | 4 | 0 |
| engine: `test/mock/frameworks/core/pipeline/mock_pipeline_context.cpp` | 9 | 0 |
| engine: `test/unittest/bridge/declarative_frontend/ark_component/water_flow_edges_test.mjs` | 68 | 0 |
| engine: `test/unittest/core/pattern/waterflow/BUILD.gn` | 1 | 0 |
| engine: `test/unittest/core/pattern/waterflow/water_flow_header_footer_test.cpp` | 422 | 0 |
| SDK: `api/@internal/component/ets/water_flow.d.ts` | 39 | 0 |
| SDK: `api/arkui/component/waterFlow.static.d.ets` | 40 | 1 |
| SDK: `zh-cn/api/@internal/component/ets/water_flow.d.ts` | 39 | 0 |
| SDK: `zh-cn/api/arkui/component/waterFlow.static.d.ets` | 40 | 1 |
