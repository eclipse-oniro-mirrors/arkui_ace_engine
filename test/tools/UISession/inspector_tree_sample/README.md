# InspectorTree 设备输入夹具

此工程提供实际 ArkUI 组件和同步 Native Accessibility Provider；不生成抓树结果。
用匹配本分支的 OpenHarmony SDK 在 DevEco Studio 导入，选择匹配分支的 API 26 SDK（build-profile 使用 26，responseRegionList 需要包含该能力的 SDK），配置开发签名后构建并安装 entry。未安装匹配 SDK/设备时不得把输入工程当作设备验证通过。

场景选择会重建组件，重置 Provider 配置、请求计数、窗口隐私和标题。native_fields/expanded_states 覆盖实际组件及菜单；privacy_password/privacy_obscured/privacy_window 验证原生正文抑制；response_regions 提供输入事件计数及旋转/缩放/裁剪；window_titles 可修改标题和回退 Label；Web 使用本地 HTML，UIExtension 填入已安装测试扩展的真实 bundle/ability。

Provider 默认含根及子节点，ID 从 1234 开始（当前 SDK 的节点/父 ID setter 为 int32_t；不改 SDK 签名。64 位无损编码边界由 Host 的真实 ArkUI_AccessibilityElementInfo 测试覆盖）。CURRENT(-1) 根查询和 RECURSIVE_CHILDREN(-1) 全树查询均是真实 C 回调，计数包含两种调用。提供成功空、失败保留有效前缀、1100 ms 返回、两宿主各 600 ms、重复 ID、10001 节点及字节超限输入。隐藏宿主用于 Full/Visible 对照。延迟为测试输入，框架不应强制取消同步 Provider。

测试 SA 先 Connect，再调用 GetInspectorTree/GetVisibleInspectorTree，使用 --interactionInfo true --accessibilityInfo true --withResponseRegion true --withCustomContent true --caseId <场景> -tofile。观察 Provider 延迟返回时另设 --timeout 5000，测整体过期隔离时保留默认 1500。日志不打印正文；从 /data/service/el1/public/ui_sa/ 导出结果后运行 ../verify_inspector_tree.py。记录设备版本、应用版本、真实命令、回调次数及报告；窗口无 Label 分支需另用无 Label 的已安装测试应用验证。

三方无障碍导出依赖 PR #90199 及其规格适配修复；原生采集依赖 PR #90061。`--withCustomContent true` 返回宿主直接成员 `accessibilityTree`，保持 Provider 原序扁平列表、类型化成员名、64 位 ID 的十进制字符串及原始 `screenRect`，增加窗口相对 `$rect`。成功空列表为 `[]`，未注册 Provider 省略该字段；原生 `accessibilityInfo` 独立控制原生字段。

任一非超时采集失败清空整次树数据。`withResponseRegion` 或 `withCustomContent` 开启时，解码树根输出 `errorMessage`：成功为 `no error`，失败为首个固定错误原因；两个开关都关闭时失败结果为 `{}`。宿主不输出完整性或错误字段。结构检查、累计 10000 节点和最终 UTF-8 JSON 10 MiB 上限由依赖导出器检测，本需求承接结果。沿用原有 timeout，不增加 1 秒采集预算；超时不新增结果回调。

校验脚本使用 `--with-custom-content` 和 `--with-response-region` 对应本次真实请求配置。Provider 的非超时失败场景检查整次空结果；延迟场景仅在实际收到回调时检查正文，未回调的超时行为须结合调用方计时和请求记录单独核对。
