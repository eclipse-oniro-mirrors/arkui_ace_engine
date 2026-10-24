# UIObserver 容器绑定重构设计索引

本仓按关联能力的 FeatID 保留设计索引，完整原则性设计在对应能力规格中维护。

- 渲染/布局监听能力：`Func-04-11-02-Feat-02`（无感监听接口全覆盖），其中 US-6、AC-6.1～AC-6.3 对应 `willDraw` / `didLayout` 监听及 UIContext/当前实例注册范围。
- 公共架构能力：`Func-04-11-02-Feat-01`（无感监听核心架构），关联注册/注销、监听作用域和 NAPI 桥接。
- 上述 FeatID 用于定位能力；Container 绑定重构的实现与验证范围仍以本 PR 和对应方案说明为准。
- 方案定位：当前推荐设计：Container 拥有实例 Store，Stage owner 线程串行访问，Render/Layout 以独立 Listener 组合公共 NAPI callback 组件。
- 阅读重点：第 4–7 节：类图、索引关系、业务流程与改造适配表；第 5.4 节：主 JS 引擎全实例监听、DC 隔离、注册/注销、生命周期及验收矩阵；第 15–17 节：Listener 解耦、穿刺接入流程与功能兼容性评估。
- 实现与测试证据：[ace_engine PR #89744](https://gitcode.com/openharmony/arkui_ace_engine/pull/89744)；具体已实现范围及验证边界以方案标注的版本为准。

代码接入参考：[实例 Store 目录说明](../frameworks/core/common/ui_observer/README.md)。
