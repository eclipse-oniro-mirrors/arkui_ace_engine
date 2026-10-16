# 实例 UIObserver 业务接入

本目录由 Container 持有的 `UIObserverStore` 管理实例级监听。Render/Layout 是首个样例；NAPI 参数和 callback 引用留在 [observer/common](../../../../interfaces/napi/kits/observer/common/napi_observer_callback.h) 与 [observer/render](../../../../interfaces/napi/kits/observer/render/render_layout_observer_listener.h)。

## 文件职责

| 文件 | 职责 |
|---|---|
| `ui_observer_store.h` | 根对象、关闭状态、业务前置声明和智能指针成员 |
| `ui_observer_store.cpp` | 包含具体业务头文件，创建、析构及统一清理 |
| `ui_observer_listener_store.h` | 类型化注册、谓词注销及快照算法 |
| `render_layout_store.h` | Render 原生通知接口及 Draw/Layout 两个独立桶 |

```text
instanceId -> Container
              └── shared_ptr<UIObserverStore>
                  └── unique_ptr<RenderLayoutStore>
                      ├── DRAW   -> UIObserverListenerStore<RenderLayoutListener>
                      └── LAYOUT -> UIObserverListenerStore<RenderLayoutListener>
```

## 接入步骤

1. 确定事件作用域。实例事件进入所属 Container；Navigation 等主 JS engine 全局事件不能直接套用实例桶。
2. 新建 `<business>_store.h`，定义只依赖原生类型的业务 listener 接口与索引；复用 `UIObserverListenerStore<T>`。
3. 在根头文件前置声明业务 store，用智能指针持有；在根 cpp 中包含具体头文件、创建对象，并加入 `ClearListeners()`。
4. 在 NAPI `observer/<business>/` 新建 listener，实现 core 的业务窄接口并组合 `NapiObserverCallback`。参数构造放到 `Invoke<N>` builder，组件统一管理引用、scope 和调用；不继承旧 UIObserverListener，不暴露裸 napi_ref。
5. 同步接入 JS on/off 入口、UIObserver 注册签名、注销谓词和快照分发。Render 的 NAPI 桶只能由 `RenderLayoutObserverListener` 填充；若引入多个 runtime，先设计 backend 隔离，不能沿用单实现向下转换。
6. 将 cpp 加入 core/NAPI 构建源列表。core 根 store 的 cpp 也须加入直接编译真实 Container 的独立测试目标。跨 SO 方法检查可见性和精确导出。
7. 参考 [store 测试](../../../../test/unittest/core/common/ui_observer_store_test.cpp) 和 [NAPI listener 测试](../../../../test/unittest/interfaces/napi/render_layout_observer_listener_test.cpp)，补全新业务的参数和实例验证。

## 必须保持的约束

- Stage 模型：注册、注销、快照和销毁在目标实例 JS owner 线程串行执行；各 DC 实例访问自己的 store。实例桶不增加热路径锁。
- 先确定事件源 instanceId，再寻找 Container/executor；不要依赖异步执行时偶然的当前实例。
- Draw/Layout 在 pipeline 触发点同步进入 Handler；Stage owner JS 线程直接分发，NAPI 下游仅在不属于目标 JS 线程时投递任务。Draw 不再额外排队，回调在当前 VSync 调用栈内执行，应保持轻量并避免递归刷新。
- 快照中的 shared_ptr 保活本批 listener；回调内 on/off 影响之后取得的快照。Close 停止本批剩余通知，Container 禁止重新创建根对象。
- `napi_ref` 创建、比较和析构必须在 env 有效且属于 owner 线程时执行。组件 Reset 可幂等释放引用，但尚未自动接入 env 清理；Close 不是 env 清理协议，活动快照可能延后引用释放。
- 不改变公开 API、callback 参数、重复注册及按 JS 相等注销语义。Render 仍以一个新建空对象作为唯一实参。
- core 不包含 NAPI 头文件；不要将业务 store 继承为根对象，也不要恢复进程级实例 map 或 thread_local 存储。

关联能力：`Func-04-11-02-Feat-02`（无感监听接口全覆盖，US-6：`willDraw` / `didLayout`）和 `Func-04-11-02-Feat-01`（无感监听核心架构）。完整方案与兼容性矩阵按上述 FeatID 定位，本仓保留[设计索引](../../../../docs/ui-observer-container-bound-refactor-feasibility.md)。
