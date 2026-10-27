# AICardDemo

`AICardDemo` 是面向 HarmonyOS / OpenHarmony ArkUI 的 **AICard 智搜卡片高级组件**演示工程。AICard 封装了智搜结果的三段布局渲染、流式打字机、乱序重组、grounding 替换与进度保持、超时与异常收敛、可配置折叠与合规区，开发者传入 `query` 与配置即可获得一致的智搜卡片展示。

工程形态为 DevEco Stage 模型 entry 应用（API 26，runtimeOS: HarmonyOS），ArkTS 编写，hypium 测试。

## 能力概览

- 三段布局：TitleBar（状态文案 + 右侧按钮）、内容区（MarkdownRender + CardContainer + RecommendZone）、ComplianceZone（【内容由AI生成】+ 复制/点赞/点踩图标）。
- 流式引擎：`setInterval` 打字机（`streamTickMs`/字，默认 40），首个字符上屏触发 `onStreamStart`。
- 乱序重组：`Map<index, chunk>` + `nextIndex` 游标按 index 升序拼接；同 index 后到覆盖、缺号不阻塞、已消费 index 的迟到重复忽略。
- grounding：`<grounding>...</grounding>` 返回体全量替换目标文本，打字机进度保持不重置；此后 `status=17` 返回体忽略，`isFinal` 仍处理。
- 超时与异常：首包 60s / 流断 30s 双定时器，连接失败/超时/provider 错误收敛到连接异常终态，`onError` 携带 E1~E4（1001~1004），`release` 恰好一次（幂等）。
- 生成控制：暂停→`cancel`+`release`+【已停止生成】；任一终态右侧按钮统一【更多】。
- 折叠（`collapseMode` 互斥二选一）：`MARKDOWN_LINES`（markdown 超 3 行折叠）/ `CARD_ZONE`（markdown 全展示，卡片区+推荐区超高折叠），折叠/展开经 controller 防抖转发 `onCollapse`/`onExpand`。
- 卡片自定义：`cardContent: @BuilderParam (data: AiSearchCardDataPayload) => void`，cards 数据经参数直传开发者 `@Builder` 拼接自定义卡片节点；卡片区/推荐区显影用 `.onAppear`+`animateTo` 串行（3s 透明度 / Y 偏移 100→0）。
- 渐变色边框：`build()` gradientBorder 分支用 Column 外层 + 双层叠加——内容层（底层，`borderRadius=CARD_BORDER_RADIUS`、带可透明 `cardBackgroundColor`）+ 渐变色边框层 Stack（顶层，`position` 绝对定位不占空间、`linearGradient` 旋转渐变、内嵌挖洞元素以 `blendMode(DST_OUT, OFFSCREEN)` 清除内容区渐变只留边框环、`hitTestBehavior(HitTestMode.None)` 事件穿透）。背景色带透明度时透出页面背景而非渐变色（ADR-14）。
- Mock provider：9 类场景（正常/乱序/grounding/isFinal/首包超时/流断超时/连接失败/onError/cancel 响应），经 AppStorage `aicard.mockScenario` 注入，不做随机。
- 测试：本地 UT 48 用例（纯逻辑）+ 设备 UI 测试（hypium + UiTest）。

## 项目结构

```text
entry/src/main/ets/
├── pages/Index.ets                      # demo 入口：query 输入 + 场景选择 + 挂载 AiCard
├── entryability/EntryAbility.ets
└── components/aicard/
    ├── common/                          # AiCardTypes / AiCardConstants / AiCardLogger
    ├── engine/                          # StreamEngine / TimeoutManager / AiCardStateMachine（纯逻辑）
    ├── controller/AiCardController.ets  # 会话编排（implements SearchCallback）
    ├── ui/                              # TitleBar / ContentView / MarkdownRender / CardContainer / RecommendZone / ComplianceZone / SkeletonLoader / IconPlaceholder
    ├── animations/AnimHelper.ets        # 卡片→推荐动画串行 + 3.5s 兜底
    ├── mocks/MockSearchProvider.ets     # 9 场景 mock
    └── AiCard.ets                       # 入口 @Component struct
```

分层单向依赖：`pages → AiCard → controller → {engine, provider}`；`engine` 不 import UI/controller；`common` 被所有层引用。

## 快速接入

创建 `AiCard`，传入 `query`、`searchProvider` 与配置/回调。demo 入口 `pages/Index.ets` 即完整示例：

```typescript
import { AiCard } from '../components/aicard/AiCard';
import { MockSearchProvider } from '../components/aicard/mocks/MockSearchProvider';
import { AiSearchCardDataPayload, AiSearchStreamingConfig, CollapseMode } from '../components/aicard/common/AiCardTypes';

@Component
struct Index {
  private provider: MockSearchProvider = new MockSearchProvider();
  private streamCfg: AiSearchStreamingConfig = {
    showMarkdown: true,
    streamTickMs: 40,
    collapseMode: CollapseMode.MARKDOWN_LINES
  };

  @Builder
  MyCardBuilder(data: AiSearchCardDataPayload) {
    // data.cards 即智搜返回的卡片数据，按需拼接自定义节点
  }

  build() {
    Column() {
      AiCard({
        query: '你好',
        searchProvider: this.provider,
        streamingConfig: this.streamCfg,
        complianceConfig: { showCopy: true, showLike: true, showDislike: true },
        cardContent: this.MyCardBuilder,   // cards 数据经参数直传 @Builder
        onStreamEnd: (text: string) => { /* markdown 全文 */ },
        onError: (e) => { /* E1~E4 */ },
        onStatusChange: (s) => { /* idle/searching/streaming/completed/error */ }
      })
    }
  }
}
```

真实智搜 provider 实现 `SearchProvider` 接口（`connect`/`search`/`cancel`/`release`）即可替换 mock。mock 场景经 `AppStorage.setOrCreate<number>('aicard.mockScenario', scenario)` 注入，或构造时传 `new MockSearchProvider(scenario)`。

## Public API

主要类型与组件（见 `common/AiCardTypes.ets`）：

| API | 用途 |
| --- | --- |
| `AiCard`（struct） | 智搜卡片入口组件，props 见下 |
| `AiSearchStreamingConfig` | `showMarkdown?`/`colorMode?`（占位）/`streamTickMs?`/`collapseMode?` |
| `CollapseMode` | `MARKDOWN_LINES` / `CARD_ZONE`（默认 MARKDOWN_LINES） |
| `AiSearchComplianceConfig` | `showCopy?`/`showLike?`/`showDislike?`/`dislikeMode?`/`dislikeTags?` |
| `SearchProvider`（接口） | `connect`/`search`/`cancel`/`release`，mock 与真实实现共用 |
| `SearchRequest`/`SearchResult`/`SearchError` | 智搜契约数据 |
| `AiSearchStatus` | `idle`/`searching`/`streaming`/`completed`/`error` |
| `AiSearchCardDataPayload` | `cardContent` @BuilderParam 的数据参数（`streamingText` + `cards`） |
| `MockSearchProvider` | 9 场景 mock 实现 |

`AiCard` 回调/插槽：`onStreamStart`、`onStreamEnd(fullText)`、`onError(error)`、`onStatusChange(status)`、`onCollapse`、`onExpand`、`onDislike(feedback)`；`cardContent: @BuilderParam (data: AiSearchCardDataPayload) => void`（卡片自定义插槽，cards 数据经参数直达 `@Builder`）。

## 错误码

| 错误码 | 数值 | 名称 | 触发 |
| --- | --- | --- | --- |
| E1 | 1001 | CONNECT_FAILED | 连接 provider 失败 |
| E2 | 1002 | FIRST_PACKET_TIMEOUT | 首包 60s 超时 |
| E3 | 1003 | STREAM_GAP_TIMEOUT | 流中断 30s 超时 |
| E4 | 1004 | PROVIDER_ERROR | provider 传输错误（onError） |

同一次会话 `onError` 至多上报一次（终态幂等去重）。

## Mock 场景

`MockSearchProvider` 9 类场景（`MockScenario` 枚举）：

| 场景 | 行为 |
| --- | --- |
| `NORMAL` | 递增 index 多次回调 + isFinal（relevant+cards） |
| `OUT_OF_ORDER` | index 乱序发出，按 index 升序拼接 |
| `GROUNDING` | `<grounding>` 全量替换 + status=17 忽略 + isFinal |
| `IS_FINAL` | 单段文本 + isFinal |
| `FIRST_PACKET_TIMEOUT` | 静默（由 controller 首包定时器触发 E2） |
| `STREAM_GAP_TIMEOUT` | 单回调后静默（由 controller 流断定时器触发 E3） |
| `CONNECT_FAIL` | connect 拒绝（E1） |
| `PROVIDER_ERROR` | 回调后 onError（E4） |
| `CANCEL_RESPONSIVE` | 周期回调，cancel 停止并触发 `cancelCalled` 钩子 |

回调延迟经 `MockOptions.chunkDelayMs` 注入（默认 200，测试缩短）。mock 不做随机。

## 构建

命令行构建（PowerShell，工程根目录）：

```powershell
$env:NODE_HOME="D:\Program Files\Huawei\DevEco Studio\tools\node"
$env:DEVECO_SDK_HOME="D:\Program Files\Huawei\DevEco Studio\sdk"
& "D:\Program Files\Huawei\DevEco Studio\tools\node\node.exe" `
  "D:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js" assembleHap --no-daemon
```

- 成功标志：末行 `BUILD SUCCESSFUL`。
- 产物：`entry/build/default/outputs/default/entry-default-unsigned.hap`（未配置签名，`Will skip sign` 为无害告警）。
- 亦可在 DevEco Studio 中直接 Run。

## 测试

**本地单元测试**（无需设备）：

```powershell
& "D:\Program Files\Huawei\DevEco Studio\tools\node\node.exe" `
  "D:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js" test --no-daemon
```

结果文件 `entry/.test/default/intermediates/test/coverage_data/test_result.txt`，成功标志 `Tests run: 48, Failure: 0, Error: 0, Pass: 48`。

**设备 UI 测试**（需模拟器/真机）：

```powershell
& "D:\Program Files\Huawei\DevEco Studio\tools\node\node.exe" `
  "D:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js" onDeviceTest --no-daemon
```

无设备时仅完成 `OhosTestCompileArkTS` 编译，运行需连接设备后在 DevEco Run ohosTest。

## 开发约束

- 新需求遵循 ohos-sdd 工作流：`proposal → spec → design → execution-plan → code`，产物写入 `.codespec/changes/<id>/`，声称完成前跑 `ohos-sdd validate . --level all`。
- 引擎层（`engine/*`）保持纯逻辑，不引 hilog；DFX 埋点在 controller/UI。
- `engine` 不 import UI/controller；`controller` 不 import UI；UI 不直接 import provider/engine。
- ArkTS 严苛：禁止 `any`；含函数属性的回调对象用 `implements`；`@Prop` 名避开 ArkUI 通用属性；`Text` 行数判定用 `onAreaChange`+`lineHeight`。
- 全部 UX 样式值/阈值/时长收敛在 `common/AiCardConstants.ets`，UX 值到位仅改常量。

## 已知限制

- `Text` 无 `onTextLayout`，行数判定退化为 `onAreaChange` + 显式 `lineHeight` 估算（等价语义）。
- 渐变色边框高度依赖 `@State cardHeight`（`onAreaChange` 回填外层 Column 实际高度）：首帧 `cardHeight=0`，渐变色层不可见；首帧布局完成后 `onAreaChange` 回填实际高度触发重绘，渐变色边框随即显示。因此内容高度变化时存在**一帧渐变色边框与背景不贴合的抖动**（渐变色层滞后内容层一帧）。后续可考虑用 `overlay` 或 `clip` 方案消除（当前因 overlay CustomBuilder 在 60fps 旋转动画下销毁重建性能差、且 Stack 中 `height('100%')` 基准不正确而未采用）。
- 卡片自定义经 `cardContent: @BuilderParam (data: AiSearchCardDataPayload) => void` 实现：组件在卡片可见时以 `this.content(this.cardPayload)` 将 cards 数据作参数直传该 `@Builder`（不依赖页面 state，无 `onCardData` 数据回调）。`@BuilderParam` 默认值运行时不绑定，故 `cardContent` 实际为必填（未配置则卡片区空）。卡片/推荐显影动画用 `.onAppear`+`animateTo`（`.animation` 属性动画在组件新建时不触发）。
- `colorMode`（深色模式）为占位枚举，本期固定浅色；`retryConfig`/流光特效等不实现。
- 设备 UI 测试（AC-9.2）运行需模拟器/真机，命令行环境仅提供编译证据。

## 许可证

Apache-2.0
