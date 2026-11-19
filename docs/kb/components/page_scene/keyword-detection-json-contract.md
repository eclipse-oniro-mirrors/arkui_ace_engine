# UISession PageScene 关键词检测规则 JSON 入参契约

> 基于 `arkui_ace_engine` commit `b4d0caa4e14`,Feature ID `03-09-01/Feat-09`。

## 1. 概述

系统智能化 SA 通过 `IUiContentService::RegisterPageSceneRules(ruleJson, callback)` 注册关键词检测规则,使框架按页面关键区域内的关键词和控件特征统一判断页面是否具备搜索功能。

路由开关:`globalConfig.enableKeywordsDetection` 为 `true` 时走关键词检测路径;否则走 legacy TEXT_EDITOR 路径(version=1)。

## 2. 完整 JSON 示例

```json
{
    "ruleSetId": "search-set-1",
    "globalConfig": {
        "enableKeywordsDetection": true
    },
    "rules": [
        {
            "ruleId": "search-rule-1",
            "pageType": "Navigation",
            "pageName": "HomePage",
            "regions": [
                { "x": 0, "y": 0, "width": 360, "height": 80 }
            ],
            "keywords": [ "search", "搜索" ],
            "components": [
                { "type": "Image", "maxSize": { "width": 48, "height": 48 }, "maxRatio": 2.0 },
                { "type": "Swiper", "autoPlay": true, "maxSize": { "width": 360, "height": 120 }, "maxRatio": 3.0 },
                { "type": "Search", "maxSize": { "width": 360, "height": 56 }, "maxRatio": 8.0 },
                { "type": "Button", "maxSize": { "width": 200, "height": 80 }, "maxRatio": 4.0 },
                { "type": "TextInput", "maxSize": { "width": 360, "height": 56 }, "maxRatio": 8.0 }
            ],
            "inspectorIds": [ "searchEntryBtn", "searchBox" ]
        }
    ]
}
```

## 3. 字段说明

### 3.1 顶层字段

| 字段 | 类型 | 必填 | 约束 |
|------|------|------|------|
| `ruleSetId` | string | 是 | 非空;字母数字/下划线/连字符,≤128 字节;同一 SA 最多 2 个不同值 |
| `globalConfig.enableKeywordsDetection` | bool | 是 | **必须为 `true`** 才路由到关键词检测路径;默认 `false` 走 legacy TEXT_EDITOR |
| `rules` | array | 是 | ≤10 条 |

### 3.2 rules[i] 字段

| 字段 | 类型 | 必填 | 约束 |
|------|------|------|------|
| `ruleId` | string | 是 | 非空;同一 ruleSet 内唯一;跨 ruleSet 可复用 |
| `pageType` | string | 是 | `"Navigation"` 或 `"Router"` |
| `pageName` | string | 是 | 非空;Navigation 取顶部 NavDestination 名、Router 取顶部 pageUrl |
| `regions` | array | 是 | 1~5 个;同一 rule 内不得有正面积重叠 |
| `keywords` | array | 否 | 0~5 个;每项非空字符串且 UTF-8 ≤20 字节 |
| `components` | array | 否 | 每项 `type` 唯一;同 rule 内每类 type 最多 1 份 |
| `inspectorIds` | array | 否 | 每项非空字符串;注册时去重;≤10 个 |

### 3.3 regions[i] 字段

| 字段 | 类型 | 必填 | 约束 |
|------|------|------|------|
| `x` | number | 是 | 区域左上角 x 坐标(vp) |
| `y` | number | 是 | 区域左上角 y 坐标(vp) |
| `width` | number | 是 | 正数(vp) |
| `height` | number | 是 | 正数(vp) |

### 3.4 components[i] 字段

| 字段 | 类型 | 必填 | 约束 |
|------|------|------|------|
| `type` | string | 是 | `Image` / `Swiper` / `Search` / `Button` / `TextInput` |
| `maxSize` | object | 否 | `{width, height}` 均为正数 |
| `maxRatio` | number | 否 | 正数;`width / height <= maxRatio`(含等于) |
| `autoPlay` | bool | 否 | 仅 Swiper 有效 |

### 3.5 inspectorIds 字段

| 约束 | 说明 |
|------|------|
| 每项非空字符串 | 注册时归一化去重 |
| ≤10 个 | 超限返回 `PARAM_INVALID` |
| 存在性扫描 | 对关键区域内节点 Inspector ID 做子串、英文忽略大小写模糊匹配 |
| 点击识别 | 对实际响应控件自身 Inspector ID 做区分大小写完整精确匹配 |

## 4. 容量约束汇总

| 维度 | 上限 |
|------|------|
| 同一 SA ruleSet 数 | 2 |
| 单 ruleSet rules 数 | 10 |
| 单 rule regions 数 | 5 |
| 单 rule keywords 数 | 5 |
| 单 keyword UTF-8 字节数 | 20 |
| 单 rule components 数 | 5(每类 type 最多 1 份) |
| 单 rule inspectorIds 数 | 10 |

## 5. 匹配规则

### 5.1 关键词匹配

- 仅顶部页面配置关键区域内可见信息参与
- 子串匹配、英文忽略大小写
- 不可见/被滚动裁剪出可视区文本不参与
- 命中即早停

匹配属性:

| 来源 | 说明 |
|------|------|
| 可见文本 | 节点 text |
| 按钮文字 | Button 节点 buttonText |
| placeholder | TextInput/Search 的 placeholderValue |
| 输入值 | TextInput/Search 的 value |
| 自定义控件名 | CustomNode 的 customName |
| 子树文本 | 节点 subtreeText |
| 无障碍描述 | accessibilityDesc / accessibilityText |
| Inspector ID | 关键区域内节点 Inspector ID 子串模糊匹配 |

### 5.2 控件特征匹配

- 五类控件按类型 OR,同类型 `maxSize`/`maxRatio`/`autoPlay` 按 AND
- `maxRatio = width / height <= maxRatio` 含等于
- 最终可见高度为 0 时该控件本次不命中
- 区域采用最终可见矩形中心点落入判定

### 5.3 命中逻辑

- 关键词命中 OR 控件特征命中 → `hasSearch=true`
- 首次具备搜索功能上报一次 `true`;证据变化状态不变不重复
- 特征消失/路由离开/后台上报一次 `false`;恢复后允许再次 `true`

## 6. 错误码

| 错误码 | 值 | 说明 |
|--------|----|------|
| `NO_ERROR` | 0 | 成功 |
| `PARAM_INVALID` | 4 | 容量超限/字段非法/区域重叠/重复 ruleSetId/第三个 ruleSet |
| `LAST_UNFINISH` | 5 | 存在未完成的注册请求 |
| `NOT_CONNECTED` | - | SA 未连接 |

任一字段违反约束整体返回 `PARAM_INVALID`,不保留部分规则。

## 7. 最简入参示例

### 7.1 只关键词

```json
{
    "ruleSetId": "search-set-1",
    "globalConfig": { "enableKeywordsDetection": true },
    "rules": [
        {
            "ruleId": "rule-1",
            "pageType": "Navigation",
            "pageName": "HomePage",
            "regions": [ { "x": 0, "y": 0, "width": 360, "height": 80 } ],
            "keywords": [ "搜索" ]
        }
    ]
}
```

### 7.2 只控件特征

```json
{
    "ruleSetId": "search-set-1",
    "globalConfig": { "enableKeywordsDetection": true },
    "rules": [
        {
            "ruleId": "rule-1",
            "pageType": "Router",
            "pageName": "Index",
            "regions": [ { "x": 0, "y": 0, "width": 360, "height": 120 } ],
            "components": [
                { "type": "Search", "maxSize": { "width": 360, "height": 56 }, "maxRatio": 8.0 }
            ]
        }
    ]
}
```

### 7.3 双 ruleSet(同一 SA)

```json
[
    {
        "ruleSetId": "search-set-a",
        "globalConfig": { "enableKeywordsDetection": true },
        "rules": [
            {
                "ruleId": "rule-a1",
                "pageType": "Navigation",
                "pageName": "HomePage",
                "regions": [ { "x": 0, "y": 0, "width": 360, "height": 80 } ],
                "keywords": [ "搜索" ]
            }
        ]
    },
    {
        "ruleSetId": "search-set-b",
        "globalConfig": { "enableKeywordsDetection": true },
        "rules": [
            {
                "ruleId": "rule-b1",
                "pageType": "Router",
                "pageName": "DetailPage",
                "regions": [ { "x": 0, "y": 0, "width": 200, "height": 60 } ],
                "keywords": [ "find" ],
                "inspectorIds": [ "detailSearchBtn" ]
            }
        ]
    }
}
```

> 双 ruleSet 分两次调用 `RegisterPageSceneRules`,每次一个 JSON 对象(非数组)。

## 8. 范围边界

### 包含

- Navigation/Router 页面
- 主页面、类弹窗和当前 UIContent 的 ArkUI 子窗
- 关键词、五类组件特征
- presence true/false 事件
- 响应控件 Inspector ID 优先及区域/关键词回退
- 事件聚合(一份 JSON / 一次异步 IPC,按 ruleSet 分别 callback)

### 不包含

- Web、UIExtension、多窗口、多实例
- 截图
- 扩展 onClick/onTouch 之外的响应类型
- 改变应用 UI、组件、回调或无障碍行为
- 新增 Public/System API 或新错误码
