# bug2.md 第 4～7 项修复与验证

基线：`b279759fa16`。本次修复动态前端事件对象生命周期，不修改第 9～11 项代码。

## 改动

- 新增 `JsiNativeEvent::Bind`：保留原生指针槽位的事件地址，同时通过 finalizer hint 持有 shared_ptr；JS 对象回收时释放这份所有权。
- CommonBridge 的两个事件工厂和 tapLocation 分别绑定所有权，保留派生事件类型，支持仅保存子对象的情形。
- onTouchTestDone 使用同一绑定机制。
- CreateJSEventInfo 创建 TouchEventInfo 堆副本。Span 和无障碍悬浮回调在返回后同步 stopPropagation、preventDefault，保留原有同步传播行为。
- 在既有 jsi_class_base_test 目标中添加 4 项所有权单测，仅增加 sources，无新增依赖。

代码、测试及构建清单共 9 个文件：

| 文件 | 改动行数 |
|---|---|
| `frameworks/bridge/declarative_frontend/engine/jsi/jsi_native_event.h` | +43 / -0 |
| `frameworks/bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_common_bridge.cpp` | +4 / -6 |
| `frameworks/bridge/declarative_frontend/engine/functions/js_touch_test_done_function.cpp` | +2 / -1 |
| `frameworks/bridge/declarative_frontend/engine/functions/js_touch_function.cpp` | +3 / -1 |
| `frameworks/bridge/declarative_frontend/engine/functions/js_touch_function.h` | +1 / -0 |
| `frameworks/bridge/declarative_frontend/style_string/js_span_object.cpp` | +5 / -1 |
| `frameworks/bridge/declarative_frontend/engine/functions/js_accessibility_function.cpp` | +5 / -1 |
| `test/unittest/bridge/jsi/jsi_native_event_test.cpp` | +188 / -0 |
| `test/unittest/bridge/jsi/BUILD.gn` | +1 / -0 |

## 编译

1. 源码定向编译：`python3 /tmp/bug2_compile_changed.py`，使用 `out/rk3568/compile_commands.json` 中原始参数，输出改到 `/tmp/bug2_verify_*.o`，未覆盖构建目录对象文件。共 9 个翻译单元（4 个文件各 2 种前端构建配置，加 CommonBridge 1 个），全部成功、无编译警告。日志：`/tmp/bug2_compile_changed.log`、`/tmp/bug2_verify_*.log`。
2. 单测构建（OpenHarmony 根目录）：

```bash
./build.sh --export-para PYCACHE_ENABLE:true --product-name rk3568 --build-target jsi_class_base_test --ccache
```

首次构建因新增测试缺少 KeyCode 定义头文件失败，已补上 `core/event/key_event.h`。随后运行：

```bash
./build.sh --export-para PYCACHE_ENABLE:true --product-name rk3568 --build-target jsi_class_base_test --ccache --fast-rebuild
```

退出码 0，输出 `build successful`。日志：`/tmp/bug2_jsi_rebuild.log` 和 `out/rk3568/build.log`。构建系统仍有既有 GN/Ninja 警告；本次修改源码的定向编译无警告。未执行完整 ace_engine 整库链接。

## 设备测试

测试二进制：`out/rk3568/tests/unittest/ace_engine/Bridge-JsiClassBase/jsi_class_base_test`，ARM 32 位；在已连接设备上运行，未替换设备系统库。

```bash
hdc shell 'LD_LIBRARY_PATH=/system/lib/platformsdk:/system/lib /data/local/tmp/ace_bug2_event_20261118/jsi_class_base_test --gtest_filter=JsiNativeEventTest.* --gtest_output=xml:/data/local/tmp/ace_bug2_event_20261118/results.xml'
```

结果：4 passed，0 failed，0 errors，0 disabled；运行 132 ms。XML 已取回并解析确认，位于 `/tmp/bug2_event_results.xml`；终端日志 `/tmp/bug2_device_test.log`。

| 用例 | 验证内容 |
|---|---|
| Bind_001 | 原生调用方释放后，JS 对象保留事件数据和 TapGestureEvent 派生类型；GC 后释放 |
| Bind_002 | 父对象回收后，单独保存的 tapLocation 仍能读取数据；最后回收后释放 |
| Bind_003 | 子对象先回收时，保存的父事件仍有效；最后回收后释放 |
| Bind_004 | 触摸事件副本独立于原始栈对象，支持传播标志修改，栈退出后仍有效，GC 后释放 |

第一次设备执行的 4 个用例均完成，但默认 XML 路径 `/./jsi_class_base_test.xml` 不可写导致报告阶段退出；指定上述可写路径后完整运行成功。

## 验证边界与约束

- 单测直接验证真实 Ark VM 中的所有权绑定及 GC，不等同于 4 条完整 UI 事件链的端到端复现；Span/无障碍回调标志写回已完成源码检查和编译，尚未通过 UI 场景自动化验证。
- `git diff --check` 通过。
- 未修改公开 API 签名、ABI 布局、错误码、依赖、生成文件。
- 保留工作区既有改动与未跟踪文件。多实例 KB 的 +3 行属于前一轮 Pipeline 纠正记录，不计入以上 9 个修复文件。
- 另新增 `bug2_remaining_solutions.md`，记录第 9～11 项的备选方案、推荐方案与验证场景。
