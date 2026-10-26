# AGENTS.md

为在本仓库中工作的 AI agent 提供指引。

## ⚠️ 新需求开发入口

本工程使用 **ohos-sdd**（OpenHarmony 规范驱动开发）+ **ohos-delivery-kit（ODK）** 交付层。开发**新功能/新特性/新组件/新需求**时，必须严格遵循交付件依赖图顺序：

1. **首先**通过 `using-ohos-sdd` 能力技能发现并 invoke 对应阶段能力 skill（`ohos-propose` → `ohos-spec` → `ohos-design` → `ohos-plan` → 实现 → `ohos-review` → `ohos-validate`）。
2. 依赖骨干：`proposal → spec → design → execution-plan → code`；`evidence/{checks,reviews}` 作旁证。
3. 所有产物写入 `.codespec/changes/<change-id>/`。当前活跃变更：`.codespec/changes/aicard-component/`（基线 v1.2）、`.codespec/changes/aicard-titlebar/`（TitleBar UX 迭代 v1.1）。
4. **硬规则**：上游交付件未 Approved 不得作为下游输入；`execution-plan` 未通过前不得修改生产代码；声称完成前必跑 `ohos-validate`（`ohos-sdd validate . --level all`，0 broken 方可声明）。
5. 不要把 `ReadyForReview` 当作 `Approved`；不要"先实现再补 spec"。

## ⚠️ 禁止事项（务必遵守）

- **这不是 Node.js/npm/TypeScript 项目。** 禁止运行 `npm install`/`npm test`/`tsc`，不要假设存在 `package.json`。构建用 Hvigor（ArkTS），依赖声明在 `oh-package.json5`（JSON5）。

- **分层单向依赖（OH-ARCH-LAYERING）。** `pages → AiCard → controller → {engine, provider}`；`engine` 不得 import UI/controller；`controller` 不得 import UI 组件；UI 不直接 import provider/engine。`common` 被所有层引用。违反将破坏可测性与编译。

- **引擎层保持纯逻辑（不引 hilog）。** `StreamEngine`/`TimeoutManager`/`AiCardStateMachine` 不得 import `AiCardLogger`，以便本地 UT（off-device）可运行。DFX 埋点集中在 `AiCardController` 与 UI 层（on-device）。

- **ArkTS 严苛规则**（违反即编译失败）：
    - 禁止 `any`；禁止无类型对象字面量（`arkts-no-untyped-obj-literals`）。**含函数属性的回调对象必须用 `implements` 接口或带类型局部变量**，不可内联 `{ onResult: () => ... }` 传入函数调用。
    - `@Prop`/`@State` 命名不得与 ArkUI 通用属性冲突（如 `size`/`width`/`height`/`opacity`），否则报 "not assignable to the same property in base type 'CustomComponent'"。
    - 数组下标访问 `arr[i]` 视为 possibly undefined，须用 `!` 或 null 守卫；可选数组属性（如 `result.relevant?`）访问 `.length` 须 `?? []`。
    - `Text` 在本 SDK 无 `onTextLayout`/`TextLayoutInfo`；行数判定改用 `onAreaChange` + 显式 `lineHeight` 估算。
    - `@BuilderParam` 仅绑定 `@Builder` **方法引用**（构造期稳定绑定）；**不可**转发"内部又调用 `@BuilderParam` 的 `@Builder` 包装器"（跨组件绑定失效、运行时 undefined 崩溃 `Cannot read property bind of undefined`）；`@BuilderParam` 默认值运行时**不绑定**（未传则 undefined 崩溃），故带 `@BuilderParam` 的接口实为必填。带参 `@BuilderParam`（如 `(data: T) => void`）可经 `this.content(this.payload)` 将运行时数据作参数直传 `@Builder`（避免跨组件 state 时序问题）。
    - 组件新建时的显影动画用 `.onAppear` + `this.getUIContext()?.animateTo()`（`@State` 局部持有，初值 0）；**不用 `.animation` 属性动画**——属性动画在组件**新建**时不触发（仅在已存在组件的属性变化时触发），曾导致卡片区无显影动画的 bug。`@State` 命名避开 `opacity` 等通用属性名（用 `cardOpacity` 等）。
    - 渐变色边框用 **Column 外层 + blendMode DST_OUT 挖洞**（ADR-14）：① Column 中 `position` 子组件**不占空间**（Stack 中仍占空间→`height('100%')` 循环致 Stack 撑满父容器），故外层必须用 Column 使渐变色层不参与高度计算；② 渐变色层 `height` 用 `@State cardHeight` 数值（`onAreaChange` 回填），**不用 `height('100%')`**——position 子组件的 `height('100%')` 基准不正确（底边缺失/撑满）；③ 内容层 `borderRadius=R` 与挖洞元素 `borderRadius=R-bw` 圆角中心须对齐于 `(R,R)`，否则圆角处出现缝隙/错位；④ 渐变色层加 `hitTestBehavior(HitTestMode.None)` 事件穿透，否则拦截下层 `CardContent` 点击；⑤ 已知限制：`cardHeight` 首帧为 0，内容高度变化时渐变色边框**滞后内容层一帧**（抖动），后续可考虑 `overlay`/`clip` 方案消除。

- **实现不得扩范围。** 只能修改 `execution-plan.md` 受影响文件清单中声明的文件；如需扩大范围，停止并修订 Plan。

- **生成文件不得手工合入。** 本工程无 bridge/IDL/CAPI 生成器产出；全部为手写源码与测试。

- **固定文案必须适配国际化（i18n）。** 凡代码/UI 中出现的**固定文本内容**（按钮、标签、标题、提示语、占位符、异常文案、合规声明等用户可见字符串），一律不得硬编码中文/英文字面量，必须通过资源文件（`resources/base/element/string.json` + `resources/{locale}/element/string.json`，如 `zh-CN`/`en_US`）+ `$r('app.string.<key>')` 引用；新增固定文案须同步补齐 `zh-CN` 与 `en_US` 两套资源键。临时调试日志或 DFX tag 不受此约束。

- **固定默认 UX 颜色必须配置深浅色。** 凡代码/UI 中出现的**固定默认颜色值**（背景、文字、边框、分割线、骨架、图标、状态色等），一律不得硬编码 `0xRRGGBB`/`#RRGGBB` 或 `Color.Black` 等裸色值，必须通过资源文件（`resources/base/element/color.json` + `resources/dark/element/color.json` + `resources/light/element/color.json`）+ `$r('app.color.<key>')` 引用，并保证 `dark` 与 `light` 两套均提供同名色键（值可不同）。仅纯逻辑层（`engine/*` 无 UI）或非视觉的临时调试色不受此约束。

## 项目简介

`AICardDemo` 是一个 **HarmonyOS / OpenHarmony ArkUI** 应用工程（Stage 模型，API 26），用于演示 **AICard 智搜卡片高级组件**：三段布局（TitleBar/内容区/合规区）、流式打字机渲染、乱序重组、grounding 替换与进度保持、超时与异常收敛、可配置折叠（`collapseMode`）、卡片自定义（`cardContent` @BuilderParam，cards 数据经参数直达 `@Builder`）、Mock 智搜 provider 与 e2e 自动化测试。使用 **ArkTS**（`.ets`）编写，hypium 测试。

这**不是** Node.js 项目；请勿运行 npm/tsc。

## 工具链

- **构建：** Hvigor（HarmonyOS 构建系统），由 DevEco Studio 驱动。本工程根目录**未提交 `hvigorw*` 包装脚本**；命令行构建使用 DevEco 自带工具：
    - node：`D:\Program Files\Huawei\DevEco Studio\tools\node\node.exe`
    - hvigor wrapper：`D:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js`
    - 需设置环境变量 `NODE_HOME` 与 `DEVECO_SDK_HOME`（均指向 DevEco 安装目录下对应路径）。
- **包管理器：** ohpm；依赖声明在 `oh-package.json5`（JSON5）。`@ohos/hypium`（1.0.25）与 `@ohos/hamock` 为 devDependency。
- **SDK：** `compatibleSdkVersion`/`targetSdkVersion` = `26.0.0`，`runtimeOS: HarmonyOS`，Stage 模型。
- 严格模式：`caseSensitiveCheck: true`、`useNormalizedOHMUrl: true`（见根 `build-profile.json5`）。

## 仓库结构

单 Hvigor 工程（`entry` 模块）：

```text
.
├── entry/
│   ├── src/main/ets/
│   │   ├── pages/Index.ets              # demo 验收入口（query 输入 + 场景选择 + 挂载 AiCard）
│   │   ├── entryability/EntryAbility.ets
│   │   └── components/aicard/
│   │       ├── common/                   # 类型/常量/DFX 基座（AiCardTypes/AiCardConstants/AiCardLogger）
│   │       ├── engine/                   # 纯逻辑引擎（StreamEngine/TimeoutManager/AiCardStateMachine）
│   │       ├── controller/               # 会话编排（AiCardController，implements SearchCallback）
│   │       ├── ui/                       # UI 区域组件（TitleBar/ContentView/MarkdownRender/CardContainer/RecommendZone/ComplianceZone/SkeletonLoader/IconPlaceholder）
│   │       ├── animations/AnimHelper.ets
│   │       ├── mocks/MockSearchProvider.ets
│   │       └── AiCard.ets                # 入口 @Component struct
│   ├── src/test/                         # 本地单元测试（hypium，纯逻辑）
│   └── src/ohosTest/ets/test/            # 设备 UI 测试（hypium + UiTest）
├── .codespec/changes/aicard-component/   # SDD 交付件（proposal/spec/design/execution-plan/evidence）
├── build-profile.json5 / oh-package.json5 / hvigorfile.ts
└── aicard.txt                            # 原始 PRD
```

## 构建与验证命令

所有命令在**工程根目录** `D:\DevEcoStudioProjects\AICardDemo` 执行（PowerShell）。先设环境变量：

```powershell
$env:NODE_HOME="D:\Program Files\Huawei\DevEco Studio\tools\node"
$env:DEVECO_SDK_HOME="D:\Program Files\Huawei\DevEco Studio\sdk"
$hvigor="D:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js"
$node="D:\Program Files\Huawei\DevEco Studio\tools\node\node.exe"
```

**1. 构建 HAP（验证 ArkTS 编译 + 打包）：**

```powershell
& $node $hvigor assembleHap --no-daemon
```

- ArkTS 编译阶段为 `CompileArkTS`；产物 `entry/build/default/outputs/default/entry-default-unsigned.hap`。
- 成功标志：末行 `BUILD SUCCESSFUL`。`Will skip sign 'hos_hap'`（未配置签名）为无害告警。

**2. 本地单元测试（UT，无需设备）：**

```powershell
& $node $hvigor test --no-daemon
```

- 编译阶段 `UnitTestArkTS`；结果文件 `entry/.test/default/intermediates/test/coverage_data/test_result.txt`。
- 成功标志：末行 `Tests run: N, Failure: 0, Error: 0, Pass: N`。注意 `hvigorw test` 在存在失败用例时仍可能输出 `BUILD SUCCESSFUL`；**必须解析 `test_result.txt` 的 `Failure/Error` 统计**。

**3. 设备 UI 测试（需真机/模拟器）：**

```powershell
& $node $hvigor onDeviceTest --no-daemon
```

- 编译阶段 `:entry:ohosTest@OhosTestCompileArkTS`；无设备时失败于 `GenerateDeviceCoverage`（`ErrorCode: 00507001`），但 ArkTS 编译已先行完成，可作为编译证据。
- 设备运行：在 DevEco Studio 连接模拟器/真机后 Run ohosTest，或 `hdc shell aa test` 启动 `OpenHarmonyTestRunner`。

## 测试

测试使用 **`@ohos/hypium`**。两套源码集（`entry/build-profile.json5` 的 `targets`：`default`、`ohosTest`）：

- `entry/src/test/*.test.ets` —— **本地单元测试**（纯逻辑，无需设备）。套件经 `List.test.ets` 注册：`localUnitTest`/`StreamEngine`/`TimeoutManager`/`AiCardStateMachine`/`AiCardController`/`MockSearchProvider`。
- `entry/src/ohosTest/ets/test/*.test.ets` —— **设备 UI 测试**（需真机/模拟器），用 `Driver`/`ON`（`@ohos.UiTest`）点击场景按钮 + 文本断言。

### UT 约定

- 引擎层（`engine/*`）保持纯逻辑、不引 hilog，确保本地 UT 可运行；DFX 埋点在 controller/UI。
- `requestId` 生成以 `() => string` 注入 controller（生产用 `@kit.ArkTS` 的 `util.generateRandomUUID()`，UT 注入固定值），避免 controller 直接依赖系统模块。
- 异步用例用 hypium 的 `done` 回调（`it(name, 0, (done) => {...})`，`func.length>0` 即异步）+ 小阈值真实定时器（`TimeoutManager` 注入 50/100ms、`StreamEngine` 注入 2ms tick）。
- 回调对象用 `implements SearchCallback`/`implements SearchProvider` 的类（如 `FakeProvider`/`RecordingCallback`），不可内联函数属性字面量。
- `Context` 在 UT 用 `nullContext()`（`const c: Context|null = null; return c!;`）。

### 当前覆盖

本地 UT 48 用例：StreamEngine(14)、TimeoutManager(5)、AiCardStateMachine(9)、AiCardController(10)、MockSearchProvider(9)、模板(1)。覆盖 AC-2.x/3.x/4.1/7.1/8.2/9.1。设备 UI 测试 `AiCardFlow.test.ets` 覆盖主流程/异常态/暂停（AC-9.2，编译已验证，运行需设备）。

## 代码风格

2 空格缩进；字符串单引号；`if`/`for` 必须花括号；`else`/`catch` 与 `}` 同行。优先 `T[]` 而非 `Array<T>`；优先数组方法而非 `for` 循环；`Number.isNaN()` 而非 `== NaN`。避免 `any`/`ESObject`。类成员加访问修饰符（`private`/`public`/`readonly`）。新增 `.ets` 文件遵循现有命名与分层。

## SDD 交付件

所有需求/设计/计划/证据产物在 `.codespec/changes/aicard-component/`：`proposal.md`/`spec.md`/`design.md`/`execution-plan.md` + `evidence/{checks,reviews}/`。变更经 `ohos-sdd validate . --level all` 守门（Level A 结构 / B 锚点 / C 依赖一致 / D 归档就绪 / E profile）。
