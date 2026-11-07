# AiCard 接口说明

> 智搜卡片高级组件 `AiCard`（`@Component export struct`，位于 `entry/src/main/ets/components/aicard/AiCard.ets`）。
> 三段布局（TitleBar / 内容区 / 推荐区 / 合规区）+ 流式打字机渲染 + 卡片淡入动画 + 折叠展开 + 渐变边框 + 超时/异常收敛。
> Config 按卡片**可视区域**拆分（TitleBar/Stream/Display/Recommend/Compliance）+ `AiCardLifecycleCallbacks` 生命周期回调（onAnimatedStart/onAnimatedEnd/onCancel）+ 外层 `onError` 异常回调。
> 本文描述对外可配置项，以源码为真相源。`@since AICardDemo-v1.2`

---

## 1. 概述

`AiCard` 是一个声明式 ArkUI 自定义组件：调用方通过 Props 注入查询词、搜索 Provider 类型与各区域 Config + 生命周期回调，组件内部自建 `AiCardController` 编排会话（骨架 → 首包 → 流式打字机 → markdown 结束 → 卡片渲染 → 推荐区 → 合规区），并在超时/异常时收敛到终态。

- **分层**：`pages → AiCard → controller → {engine, provider}`；UI 不直接 import provider/engine。
- **自启动**：`aboutToAppear` 创建 controller + 启动渐变边框动画；骨架占位组件挂载后触发 `controller.start(query, ctx)`，无需调用方显式 start。
- **Provider 选择**：通过 `searchProviderType` 枚举声明数据源类型；`DEFAULT` 时组件不启动（`aboutToAppear` 直接 return），`TEST` 时 controller 内部创建 `TestSearchProvider`（从 `test/active.json` raw 资源加载 fixture）。
- **销毁**：`aboutToDisappear` 调用 `controller.dispose()` + 停止渐变动画 + 清理推荐区 fallback 定时器。
- **渐变边框**：始终启用（写死常量，不走 config）；`aboutToAppear` 时启动 `setInterval` 旋转动画，旋转 `GRADIENT_ROTATION_COUNT_DEFAULT = 1` 圈后自动停止。

---

## 2. 快速使用

```ts
import { AiCard } from '../components/aicard/AiCard';
import {
  AiCardStreamConfig, AiCardDisplayConfig, AiCardTitleBarConfig,
  AiCardRecommendConfig, AiSearchComplianceConfig, AiCardLifecycleCallbacks,
  AiSearchCardDataPayload, AiSearchCardRawData, AiSearchDislikeFeedback,
  AiSearchError, AiSearchRecommendChip, CollapseMode, SearchProviderType,
  ThemeColorMode, AiIconType
} from '../components/aicard/common/AiCardTypes';
import { AiCardConstants } from '../components/aicard/common/AiCardConstants';

@Builder
MyCardBuilder(data: AiSearchCardDataPayload, _index: number) {
  ForEach(data.cards, (card: AiSearchCardRawData, index: number) => {
    Text(card.title ?? '').fontSize(AiCardConstants.FONT_SIZE_BODY)
  }, (card: AiSearchCardRawData, index: number) => `${index}_${card.title ?? ''}`)
}

// 在父组件 build() 中：
WithTheme({ colorMode: ThemeColorMode.LIGHT }) {
  AiCard({
    query: this.query,
    searchProviderType: SearchProviderType.TEST,
    streamConfig: { showMarkdown: true },
    displayConfig: { collapseMode: CollapseMode.WHOLE_COMPONENT },
    titleBarConfig: {
      onMore: () => {},
      aiIcon: AiIconType.AI_SEARCH
    },
    recommendConfig: {
      showRecommend: true,
      maxChips: 3,
      onRecommendClick: (chip: AiSearchRecommendChip) => {}
    },
    complianceConfig: {
      showCopy: true,
      showFeedbackButtons: true,
      onLike: (active: boolean) => {},
      onDislike: (feedback: AiSearchDislikeFeedback, active: boolean) => {},
      onCopy: (text: string) => {}
    },
    lifecycleCallbacks: {
      onAnimatedStart: () => {},
      onAnimatedEnd: () => {}
    },
    onError: (e: AiSearchError) => {},
    cardContent: this.MyCardBuilder
  })
}
```

---

## 3. Props 一览

| 成员 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| `query` (`@Prop`) | `string` | 是 | `''` | 搜索查询词。`@Prop` 单向数据流，父组件修改即驱动新会话 |
| `searchProviderType` | `SearchProviderType` | 是 | `DEFAULT` | 搜索数据源类型枚举（见 §8）。`DEFAULT` 时组件不启动；`TEST` 时 controller 内部创建 `TestSearchProvider` |
| `streamConfig` | `AiCardStreamConfig \| null` | 否 | `null` | 内容/流式区配置（是否渲染 Markdown） |
| `displayConfig` | `AiCardDisplayConfig \| null` | 否 | `null` | 显示/折叠区配置（折叠模式、背景色、折叠/展开回调） |
| `titleBarConfig` | `AiCardTitleBarConfig \| null` | 否 | `null` | 标题区配置（更多按钮回调、AI 图标） |
| `recommendConfig` | `AiCardRecommendConfig \| null` | 否 | `null` | 推荐区配置（是否展示、标题、最大 chip 数、自定义 chips、chip 点击回调） |
| `complianceConfig` | `AiSearchComplianceConfig \| null` | 否 | `null` | 合规区配置（复制/点赞/点踩按钮显隐 + 对应回调 + 点踩标签） |
| `lifecycleCallbacks` | `AiCardLifecycleCallbacks \| null` | 否 | `null` | 生命周期回调（动画开始/动画结束/主动取消 onCancel，v1.14 新增） |
| `onError` | `((error: AiSearchError) => void) \| null` | 否 | `null` | 异常回调：连接失败/首包超时/流中断超时/provider 错误（`AiSearchErrorCode` 1001~1004）；每次会话至多一次（去重）。v1.7 起为直接外层回调，不再经 `lifecycleCallbacks` 传递 |
| `cardContent` (`@BuilderParam`) | `(data: AiSearchCardDataPayload, index: number) => void` | 否 | `DefaultCard`（空） | 卡片内容自定义构建器，接收 `{ streamingText, cards }` |

> `cardContent` 为 `@BuilderParam`，仅可绑定 `@Builder` **方法引用**（构造期稳定绑定）。运行时数据经参数直传 `@Builder`。未传时使用内部空 `DefaultCard`（实际为必填，否则卡片区为空）。
>
> `searchProviderType` 非 `@Prop`，为普通成员，建议构造期一次性赋值；运行时变更不会被 ArkUI 观测，需通过父组件 `sessionKey` 等机制重建组件（`ForEach([key], ...)` 配合）以切换会话。

---

## 4. `@BuilderParam cardContent` —— 卡片自定义插槽

### 签名

```ts
@BuilderParam cardContent: (data: AiSearchCardDataPayload, index: number) => void
```

> **`index` 参数（预留）：** 为多段 `AiSearchCardDataPayload` 交错布局演进预留（未来后端按多段逐段调用 `cardContent(payload, i)`）。当前后端恒传 `0`（单段，行为不变，功能未实现）。

### 载荷类型 `AiSearchCardDataPayload`

| 字段 | 类型 | 说明 |
|------|------|------|
| `streamingText` | `string` | 流式文本（卡片阶段传入空串 `''`） |
| `cards` | `AiSearchCardRawData[]` | 全部卡片数据，**一次性传入**（非逐条） |

### `AiSearchCardRawData`

| 字段 | 类型 | 说明 |
|------|------|------|
| `type` | `string` | 卡片类型标识 |
| `title?` | `string` | 卡片标题（可选） |
| `data` | `Record<string, Object>` | 卡片原始数据 |
| `iconUrl?` | `string` | 卡片图标 URL（可选） |

### 渲染时机与机制

1. `onMarkdownEnd`（END 事件）时 controller 一次性回调 `onCardData(cards)`（传全部 cards 数组）。
2. `AiCard.handleCardData` 一次性赋值 `this.cardList = cards` 并置 `this.cardVisible = true`；若 `collapseMode !== WHOLE_COMPONENT` 则同步置 `recommendVisible = true`。
3. `CardContainer` 经 `cardContent` @BuilderParam 一次性传入 `{ streamingText: '', cards: this.cardList }, 0` 渲染全部卡片。
4. 组件新建时 `.onAppear` + `getUIContext().animateTo()` 淡入透明度动画（`duration = CARD_ANIM_DURATION_MS = 3000`ms，`cardOpacity` 0→1），动画结束触发 `onFinish` → `onCardFinish` → `tryFireAnimatedEnd()`。

> 因此自定义 `@Builder` 内部应使用 `ForEach(data.cards, ...)` 遍历全部卡片，而非假设逐条调用。

---

## 5. Config 类型

### `AiCardTitleBarConfig`（标题区）

| 字段 | 类型 | 默认 | 说明 |
|------|------|------|------|
| `onMore?` | `() => void` | `undefined` | TitleBar「更多」按钮点击回调（`chevron_right` 图标按钮） |
| `aiIcon?` | `AiIconType` | `AiIconType.AI_DOC_PLAINTEXT` | TitleBar AI 图标类型枚举（见 §8） |

### `AiCardStreamConfig`（内容/流式区）

| 字段 | 类型 | 默认 | 说明 |
|------|------|------|------|
| `showMarkdown?` | `boolean` | `true` | 是否渲染 Markdown 文本区；`false` 时纯文本展示 |

### `AiCardDisplayConfig`（显示/折叠区）

| 字段 | 类型 | 默认 | 说明 |
|------|------|------|------|
| `collapseMode?` | `CollapseMode` | `WHOLE_COMPONENT` | 折叠模式（见 §7） |
| `cardBackgroundColor?` | `ResourceColor` | `$r('sys.color.comp_background_list_card')` | 卡片背景色 |
| `onCollapse?` | `() => void` | `undefined` | 内容折叠回调（由 `controller.notifyCollapseChanged(true)` 触发） |
| `onExpand?` | `() => void` | `undefined` | 内容展开回调（由 `controller.notifyCollapseChanged(false)` 触发） |

### `AiCardRecommendConfig`（推荐区）

| 字段 | 类型 | 默认 | 说明 |
|------|------|------|------|
| `showRecommend?` | `boolean` | `false` | 是否展示推荐区 |
| `title?` | `string \| Resource` | `$r('app.string.recommend_title')` | 推荐区标题 |
| `maxChips?` | `number` | `RECOMMEND_CHIP_MAX_DEFAULT = 3` | 最大 chip 数（超出时 `slice(0, max)` 截断） |
| `chips?` | `AiSearchRecommendChip[]` | 由 `relevant` 生成 | 自定义 chips；未提供时由 provider 返回的 `relevant` 项生成 |
| `onRecommendClick?` | `(chip: AiSearchRecommendChip) => void` | `undefined` | 推荐 chip 点击回调 |

### `AiSearchComplianceConfig`（合规区）

| 字段 | 类型 | 默认 | 说明 |
|------|------|------|------|
| `showCopy?` | `boolean` | `true` | 是否显示复制按钮 |
| `showFeedbackButtons?` | `boolean` | `true` | 是否显示点赞/点踩按钮 |
| `dislikeTags?` | `string[]` | `[]` | 点踩弹窗标签列表 |
| `onLike?` | `(active: boolean) => void` | `undefined` | 点赞切换，`active=true` 为赞激活 |
| `onDislike?` | `(feedback: AiSearchDislikeFeedback, active: boolean) => void` | `undefined` | 点踩确认/取消，`active=true` 为确认提交 |
| `onCopy?` | `(text: string) => void` | `undefined` | 复制当前显示文本 |

### `AiSearchRecommendChip`

| 字段 | 类型 | 说明 |
|------|------|------|
| `text` | `string` | chip 文本 |
| `icon?` | `ResourceStr` | chip 图标（可选） |

---

## 6. Callback 类型

### `AiCardLifecycleCallbacks`

| 方法 | 签名 | 触发时机 |
|------|------|----------|
| `onAnimatedStart?` | `() => void` | markdown 流式开始（`StreamStatus='START'`），首包到达、打字机启动 |
| `onAnimatedEnd?` | `() => void` | **所有动画结束点**：若推荐区显示则推荐区出现动画结束后；否则若卡片区显示则卡片淡入动画结束后；否则 markdown 打字结束后（`StreamStatus='END'`） |
| `onCancel?` | `(phase: CardCancelPhase) => void` | 主动取消：点停止按钮或宿主 `controller.cancel()`（两路径统一）；`phase` 取取消时刻阶段——`THINKING=0`（searching，未流出内容）/ `GENERATING=1`（streaming，已流出内容）；终态/已停止/已消隐守卫下不触发（v1.14 新增） |

> `onError` 已于 v1.7 移出本接口，改为 `AiCard` 直接外层回调参数（见 §3 Props 一览 `onError`）。

### `onError`（AiCard 直接外层回调）

| 签名 | 触发时机 |
|------|----------|
| `(error: AiSearchError) => void` | 异常终态（连接失败/首包超时/流中断超时/provider 错误，`AiSearchErrorCode` 1001~1004）；每次会话至多一次（去重） |

> **`onAnimatedEnd` 行为说明**：该回调在卡片全部动画播完时触发，含 fallback 链：
> 1. 若 `recommendVisible = true` → 等推荐区 `animateTo` 动画结束（`onRecommendAnimEnd`）
> 2. 否则若 `cardVisible = true` → 等卡片淡入动画结束（`onCardFinish`）
> 3. 否则 → markdown 打字结束即触发
>
> 若推荐区预期显示（`showRecommend !== false` 且有 chips/relevant）但未出现（如内容短未触发折叠按钮），有 `RECOMMEND_FALLBACK_TIMEOUT_MS = 3500ms` 兜底超时强制触发。
>
> 回调保证仅触发一次（`animatedEndFired` flag）。

### 相关数据类型

```ts
interface AiSearchError {
  code: AiSearchErrorCode;   // 1001 | 1002 | 1003 | 1004
  message: string;
}

interface AiSearchDislikeFeedback {
  tags: string[];
  description: string;
}
```

### 错误码

| code | 常量 | 含义 |
|------|------|------|
| `1001` | `ERROR_CODE_CONNECT_FAILED` | 连接 provider 失败 |
| `1002` | `ERROR_CODE_FIRST_PACKET_TIMEOUT` | 首包超时（60s） |
| `1003` | `ERROR_CODE_STREAM_GAP_TIMEOUT` | 流间隙超时（30s） |
| `1004` | `ERROR_CODE_PROVIDER_ERROR` | provider 传输错误 |

---

## 7. `CollapseMode` 折叠模式

```ts
enum CollapseMode {
  WHOLE_COMPONENT = 0,  // 整体折叠（默认）
  MARKDOWN_AREA   = 1,  // markdown 区折叠
  NONE            = 2,  // 不折叠
}
```

| 值 | 行为 |
|----|------|
| `WHOLE_COMPONENT` | ContentView 推荐区上方内容（MarkdownRender + CardContainer）总高度超 `CONTENT_COLLAPSE_HEIGHT = 200`vp 时，整体折叠为 200vp（`clip` 裁剪），下方显示「查看全部/收起」ToggleButton（`chevron_down/up`）。`onAreaChange` 只记录最大值防闪变；折叠后超出高度内容被 clip 裁剪不可见。ToggleButton 显影动画结束后触发 `onToggleAnimationEnd` → `recommendVisible = true`。 |
| `MARKDOWN_AREA` | markdown 流式文本超 `MARKDOWN_COLLAPSE_MAX_LINES = 3` 行时，逐字测算截断 + 自行拼接 `"..."`（非系统 `textOverflow`）+ `symbol chevron_down/up` 折叠/展开图标同行显示；展开态按钮位置自适应（同行末尾或下方）；首次折叠时调用 `controller.skipToEnd()` 跳过打字机剩余 tick。 |
| `NONE` | 内容完整展示，不折叠、不测算。 |

> `WHOLE_COMPONENT` 折叠逻辑位于 `ContentView`；`MARKDOWN_AREA` 测算在 `AiCard.handleDisplayUpdate` UI 层拦截（engine 层不引 `getMeasureUtils`，保持纯逻辑）。
> `MARKDOWN_AREA` 模式下 `handleCardData` 会同步置 `recommendVisible = true`（不依赖整体折叠的 Toggle 动画）。

---

## 8. 枚举与数据源契约

### `SearchProviderType`（数据源类型）

```ts
enum SearchProviderType {
  DEFAULT = 0,  // 默认：不启动会话（aboutToAppear 直接 return）
  TEST    = 1,  // 测试：controller 内部创建 TestSearchProvider
}
```

- `DEFAULT`：组件仅渲染 UI 壳，不连接任何 provider，适合集成方自行注入 provider 的场景（通过 `AiCardControllerOptions.provider` 直接构造 controller）。
- `TEST`：controller 在 `start()` 时 `new TestSearchProvider()`，从 `test/active.json` raw 资源加载 `TestScenarioFixture`，按 `actions` 逐条定时（默认 `chunkDelayMs = 200ms`）回调 `onResult`/`onError`。

### `AiIconType`（AI 图标类型）

```ts
enum AiIconType {
  AI_SEARCH       = 0,  // sys.media.AI_search（Image）
  AI_DOC_PLAINTEXT = 1,  // sys.symbol.AI_doc_plaintext（SymbolGlyph）
  SMART_ICON       = 2,  // sys.media.AI_search（Image，占位）
}
```

### `ThemeColorMode`（主题色模式）

```ts
enum ThemeColorMode {
  LIGHT = 0,
  DARK  = 1,
}
```

> Demo 页面通过 `WithTheme({ colorMode: ThemeColorMode.LIGHT })` 包裹 `AiCard` 强制浅色主题。

### `TestScenarioFixture`（测试 fixture 契约）

```ts
interface TestScenarioFixture {
  connectFail?: boolean;          // true 则 connect 抛异常
  actions: TestFixtureAction[];  // 逐条按 chunkDelayMs 间隔回调
}

interface TestFixtureAction {
  result?: TestFixtureResult;  // 命中则 callback.onResult(toSearchResult(result))
  error?: SearchError;         // 命中则 callback.onError(error)
}

interface TestFixtureResult {
  index: number;
  status: number;
  isFinal: boolean;
  streamingText: string;
  relevant?: AiSearchRelevantItem[];
  cards?: AiSearchCardRawData[];
}
```

### `SearchProvider`（数据源接口）

```ts
interface SearchProvider {
  connect(context: Context): Promise<void>;
  search(request: SearchRequest, callback: SearchCallback): void;
  cancel(requestId: string): void;
  release(context: Context): void;
}

interface SearchRequest {
  requestId: string;
  query: string;
  model?: string;        // controller 默认传 'CLOUD'
  scene?: string;
  thinkMode?: string;
  referenceType?: string; // controller 默认传 '1'
}

interface SearchCallback {
  onResult(result: SearchResult): void;
  onError(error: SearchError): void;
}

interface SearchResult {
  requestId: string;
  status: number;
  isFinal: boolean;
  streamingText: string;
  reference?: Object[];
  relevant?: AiSearchRelevantItem[];   // 推荐项
  cards?: AiSearchCardRawData[];       // 卡片数据（isFinal 时一次性传入全部）
  index: number;
}
```

- `isFinal=true` 的结果携带 `cards` 与 `relevant`；`onMarkdownEnd`（END）时 controller 一次性回调 `onCardData(cards)`。
- 工程内置 `TestSearchProvider`（`mocks/TestSearchProvider.ets`），通过 `test/active.json` fixture 驱动；集成方可自行实现 `SearchProvider` 接口并通过 `AiCardController` 直接注入。

---

## 9. 组件行为时序

```
aboutToAppear
  └─ 若 searchProviderType === DEFAULT → 直接 return（不启动）
  └─ 创建 AiCardController（注入 searchProviderType + 回调）
  └─ 启动渐变边框动画（setInterval 旋转，旋转 1 圈后自停）
  └─ 初始化 DislikeDialog

SkeletonLoader.onAppear
  └─ handleSkeletonShown → controller.start(query, ctx)
        └─ 若 TEST → new TestSearchProvider → load test/active.json
        └─ provider.connect → provider.search
              └─ 首包到达 → onAnimatedStart（骨架消失，TitleBar=生成中）
              └─ StreamEngine 打字机 tick → onDisplayUpdate(text)
              └─ isFinal → onMarkdownEnd(fullText) → tryFireAnimatedEnd()
              └─ END → onCardData(cards)【一次性】
                    └─ AiCard.handleCardData: cardList=cards, cardVisible=true
                          └─ 若 collapseMode !== WHOLE_COMPONENT → recommendVisible=true
                    └─ CardContainer 一次性渲染 + animateTo 淡入（3000ms）
                    └─ onCardFinish → tryFireAnimatedEnd()
                    └─ 【WHOLE_COMPONENT 模式】ToggleButton 显影 animateTo
                          └─ onToggleAnimationEnd → recommendVisible=true, clearRecommendFallback
                          └─ RecommendZone onAppear → animateTo → onRecommendAnimEnd → tryFireAnimatedEnd()
                    └─ onAnimatedEnd 触发（所有动画结束）

超时/异常
  └─ 首包 60s / 流间隙 30s → finalize('error', code) → onError
  └─ provider 错误 → finalize('error', 1004)
  └─ 用户点停止 → pause() → finalize('stopped')

aboutToDisappear
  └─ controller.dispose() + 停止渐变动画 + 清理推荐区 fallback 定时器
```

---

## 10. 资源依赖

组件通过 `$r` 引用以下资源键，集成时须确保存在（详见 `AiCardConstants.ets`）：

### 字符串（`resources/base|zh-CN|en_US/element/string.json`）

| key | 用途 |
|-----|------|
| `titlebar_thinking` / `titlebar_generating` / `titlebar_found` / `titlebar_error` / `titlebar_stopped` | TitleBar 状态文案 |
| `compliance_text` | 合规声明文案 |
| `compliance_copied` | 复制成功 toast |
| `markdown_expand` / `markdown_collapse` | markdown 折叠/展开文案 |
| `card_expand` / `card_collapse` | 整体折叠/展开 Button 文案 |
| `recommend_title` | 推荐区标题 |
| `compliance_dialog_feedback_title` / `compliance_dialog_description_title` / `compliance_dialog_description_placeholder` / `compliance_dialog_cancel` / `compliance_dialog_confirm` | 点踩弹窗文案 |
| `compliance_dislike_reason_1..8` | 点踩弹窗 8 个预定义原因 |

### 颜色（`resources/base|dark/element/color.json`，`base` 为浅色默认）

| key | 用途 |
|-----|------|
| `markdown_skeleton` | 骨架色 |
| `markdown_text_primary` / `markdown_text_secondary` | 文本主/次色 |
| `aicard_gradient_0..4` | 卡片渐变边框色 |
| `titlebar_gradient_start` / `mid` / `end` | TitleBar 渐变色 |
| `compliance_icon_bg` | 合规区图标背景 |
| `collapse_button_bg` | 折叠图标按钮背景（base + dark） |
| `recommend_chip_bg` | 推荐区 chip 背景 |

### 系统资源（`$r('sys.*')`）

- 媒体：`sys.media.AI_search`（默认 AI 图标）
- 符号：`sys.symbol.AI_doc_plaintext` / `sys.symbol.chevron_down` / `sys.symbol.chevron_up` / `sys.symbol.chevron_right` / `sys.symbol.stop_circle_fill` / `sys.symbol.plus_square_on_square` / `sys.symbol.hand_thumbsup` / `sys.symbol.hand_thumbsup_fill` / `sys.symbol.hand_thumbsdown` / `sys.symbol.hand_thumbsdown_fill`
- 颜色：`sys.color.comp_background_list_card` / `sys.color.icon_primary` / `sys.color.icon_secondary` / `sys.color.icon_fourth` / `sys.color.icon_emphasize` / `sys.color.font_primary` / `sys.color.font_secondary` / `sys.color.font_fourth` / `sys.color.font_on_primary` / `sys.color.comp_background_tertiary` / `sys.color.comp_divider` / `sys.color.background_primary`
- 字号/圆角：`sys.float.Body_S` / `sys.float.Body_M` / `sys.float.Caption_M` / `sys.float.corner_radius_level2`

---

## 11. 约束与注意事项

- **非 Node 工程**：构建用 Hvigor（ArkTS），依赖声明在 `oh-package.json5`；勿运行 `npm`/`tsc`。
- **`@BuilderParam` 绑定规则**：`cardContent` 仅绑定 `@Builder` 方法引用；不可转发「内部又调用 `@BuilderParam` 的 `@Builder` 包装器」（跨组件绑定失效）。
- **i18n / 颜色规则**：固定文案不得硬编码，须经 `$r('app.string.<key>')` 引用并提供 `zh-CN`/`en_US` 双套；固定默认颜色须经 `$r('app.color.<key>')` 引用并提供 `dark`/`base` 双套。
- **分层单向依赖**：UI 不直接 import provider/engine；`controller` 不 import UI 组件；`engine` 保持纯逻辑（不引 hilog）。
- **`searchProviderType` 非 `@Prop`**：为普通成员，建议构造期一次性赋值；运行时变更不会被 ArkUI 观测，需通过父组件 `sessionKey`（`ForEach([key], ...)`）机制重建组件以切换会话。
- **渐变边框写死**：渐变边框始终启用（`aboutToAppear` 启动 `setInterval` 旋转动画），不提供 config 控制；旋转圈数写死 `GRADIENT_ROTATION_COUNT_DEFAULT = 1`（旋转 1 圈后自动停止）。
- **`streamTickMs` 不可配**：打字机 tick 间隔硬编码为 `STREAM_TICK_MS_DEFAULT = 40`ms，不从 config 接收。
- **`onAnimatedEnd` fallback 链**：该回调触发时机取决于推荐区/卡片区是否显示，非仅 markdown 打字结束；推荐区预期显示但未出现时有 3500ms 兜底超时。
- **`WithTheme` 主题包裹**：Demo 使用 `WithTheme({ colorMode: ThemeColorMode.LIGHT })` 强制浅色；集成方按需选择 `LIGHT`/`DARK`。
