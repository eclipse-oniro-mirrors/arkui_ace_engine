# SendCommand 请求与完成通知耗时

采集时开启 `ace` trace tag，并同时采集应用进程和 SA 进程。
每次采集使用同一个 SA 运行实例，重启 SA 后重新采集，避免 requestId 重用。

测试顺序：先验证 callback 重载相对旧接口的新增时延不超过 1ms，再验证完成通知 IPC 耗时不超过 5ms。
两项都需在目标设备正常负载、命令执行成功且采集数据完整的前提下判断。

## 新旧接口请求链路对比（新增时延 1ms）

- start：SA 进入 `UIContentServiceProxy::SendCommand` 函数体的第一条语句，在参数检查、
  callback 注册、超时任务安排和请求 IPC 组包之前。
- end：Node 在应用 UI 任务开始、节点查询之前；Global 在命令解析、分发准备完成后，
  调用 keycode/relaxed/batch 实际处理函数之前。
- 带节点 ID 的入口使用 `Node`；不带节点 ID 的入口使用 `Global`。
  新旧 Global 均计入 JSON 解析、Pipeline 查询和 keycode 事件构造等分发准备开销。
  参数错误等提前返回路径也会结束已进入的 UI 分发 trace，采样必须另外确认命令执行成功。

| 接口 | 带节点 ID | 不带节点 ID |
| --- | --- | --- |
| 原接口 | `SendCommand.Request.Legacy.Node` | `SendCommand.Request.Legacy.Global` |
| callback 重载 | `SendCommand.Request.Callback.Node` | `SendCommand.Request.Callback.Global` |

SA 侧通过 UISession 的 HiTrace 封装开始，应用侧用已有的 `AceAsyncTraceEnd` 结束；
两端使用相同的 ACE tag、名称和 taskId。SA 的独立 ui_session 库不需要新增对 ACE 实现的依赖。
原来的完成通知 trace 保留，两段测量各自独立。

### 采样步骤

1. 在同一设备、版本、应用、SA、页面和正常负载下采集，同时开启两端的 ACE trace。
2. 选同一条命令、同一目标节点，分别用旧接口和 callback 重载调用一次。
   等前一条命令实际完成、恢复相同页面/节点状态后再调用下一条，不并发发送。
3. **每份采集文件，同一入口类型只允许一条 Legacy 和一条 Callback 请求。**
   旧接口没有 requestId，本次不修改 IPC 协议，也不支持在同一份采集中给多次同名请求配对。
   多轮测量使用独立采集文件；可轮换新旧调用顺序减少先后顺序的影响。
4. 将采集文件解码成文本，执行：

   ```bash
   python3 test/tools/UISession/send_command_trace.py request.trace request.json --metric request
   ```

5. 查看终端输出和 JSON 的 `comparisons`，在 trace 可视化工具中打开 JSON 查看请求泳道。
   展示位置统一放在 SA 进程，args 保留实际 SA、应用 PID，原始时间戳不变。

例如旧接口耗时 2ms，新接口耗时 2.7ms，新增时延是 **0.7ms**，该样本满足 1ms 门限。
比较的是 `callback_ms - legacy_ms`，不要求新接口的总耗时小于 1ms。
负差值表示该样本中新接口更快。

脚本拒绝缺失/重复起止点、同进程配对、时间倒置、请求区间重叠，或新旧接口进程对不同的比较。
仅有新接口、缺少旧接口基线也不会判为通过。请求被拒绝、UI 任务投递失败或进程退出时可能只有 start；
这类样本标记为 excluded，不伪造 end。脚本不能判断命令、页面状态和负载是否一致，采样者必须保证这些条件。

请求模式退出码：0 表示有有效新旧比较、差值均不超过 1ms 且无 excluded；
1 表示存在差值超过 1ms 的样本；2 表示采样不完整或有歧义。
不传 `--metric` 时仍按原来的完成通知 5ms 指标处理。

### Windows 真机请求链路测试步骤

测试目标：在目标设备正常负载下，对比同一命令使用旧接口和 callback 重载的请求链路耗时，
验证新增时延 `callback_ms - legacy_ms` 不超过 1ms。
以下步骤使用带节点 ID 的 `Node` 请求链路。整体顺序是：
**开始采集 → 旧接口一次 → 等待完成 → 新接口一次 → 等待完成 → 停止采集 → 导出 → 脚本分析**。
Python 在电脑上处理已经采集的文件，不负责开启手机 trace。
所有电脑命令统一在 **Windows PowerShell** 执行；手机命令在 `hdc shell` 进入后执行。
按步骤确认结果，不要把整段流程一次性粘贴运行。

手机需使用包含 trace 改动并已生效的版本。部署时可参考
[UISession 验证部署指南](UISession_Verification_Guide.md)，SO 路径和位数以进程实际加载情况为准。
打开测试应用，选一个点击后不跳页、不销毁节点的按钮。下文节点 ID `2` 仅为示例，
两条命令都要替换成当前页面的有效 ID。保持同一应用、SA、页面和正常负载；
等待启动、加载和动画稳定，不额外运行压力测试。需要预热时，在正式采集之前执行。

以下命令统一使用文件名前缀 `sendcommand_request_01`。每轮采样选择尚未使用过的名称，
并同步修改手机保存、电脑下载、原始打点检查和 Python 输入输出中的所有文件名。

#### 1. 电脑 PowerShell：检查目录、脚本和设备

将仓库中的 `test/tools/UISession/send_command_trace.py` 复制到电脑工作目录，
在 PowerShell 中打开该目录后执行：

```powershell
Get-Location
Get-Item .\send_command_trace.py
py -3 --version
hdc list targets
```

| 命令 | 作用及继续条件 |
| --- | --- |
| `Get-Location` | 显示电脑当前目录，后续下载和 JSON 输出都在这里 |
| `Get-Item .\send_command_trace.py` | 确认分析脚本存在；找不到时先修正目录或脚本路径 |
| `py -3 --version` | 确认电脑的 Python 3 可用 |
| `hdc list targets` | 确认手机已连接；本流程按仅连接一台目标设备执行 |

脚本在仓库中的位置是 `test/tools/UISession/send_command_trace.py`。
如果电脑当前目录是 `ace_engine` 仓库根目录，检查和运行脚本时都将脚本路径替换为
`.\test\tools\UISession\send_command_trace.py`；trace 和 JSON 仍可保存在电脑当前目录。
若没有 `py`，可用已安装并确认是 Python 3 的 `python` 替换后文 `py -3`。

#### 2. 电脑 PowerShell：进入手机 shell

```powershell
hdc shell
```

作用：进入手机命令环境。下面第 3～8 步在这个手机 shell 中执行，直到输入 `exit`。

#### 3. 手机 shell：连接应用并关闭诊断

```sh
hidumper -s 16666 -a "Connect"
```

作用：通过示例 SA 建立与前台应用的连接，确认连接成功后继续。

```sh
hidumper -s 16666 -a "SendCommandTraceDiagnostic off"
```

作用：关闭当前 SA 的额外诊断探针。预期输出包含 `SendCommand trace diagnostic: off`。
诊断模式的日志和文件 I/O 会干扰性能测量；如果命令不被识别，先核对所部署版本，不继续算性能。

#### 4. 手机 shell：检查等级并开始采集

```sh
/system/bin/hitrace --get_level
```

作用：查看 trace 等级。`Info` 或 `Debug` 可以继续；
若为过滤 Info 打点的等级（如 `Critical`、`Commercial`），先执行下面的调整命令：

```sh
/system/bin/hitrace --trace_level Info
```

该调整仅在等级不符合时执行。然后开始采集：

```sh
/system/bin/hitrace --trace_begin -b 16384 ace
```

- `/system/bin/hitrace`：手机上工具从根目录起的完整路径，开头的 `/` 不能省略。
- `--trace_begin`：开始记录。
- `-b 16384`：设置缓冲区大小参数为 16384 KB。
- `ace`：采集 ArkUI 打点，已包含请求链路分析所需事件。

确认出现 `OpenRecording done.` 等成功信息后再发送命令。如果报错，先保留错误输出并排查，
不要继续使用历史文件。此时只是开始记录，手机文件名在第 7 步保存时才指定。
本流程使用文本输出，不添加 `--raw`、`--record` 或 `-z`。

#### 5. 手机 shell：执行一次旧接口

用于 1ms 性能对照时，下面旧、新接口示例中的节点 ID 和 `cmd` 必须统一为同一条已验证成功的命令。
不能用旧接口的 `click` 与新接口的 `onButtonClick` 直接作为同一命令的对照样本。

```sh
hidumper -s 16666 -a 'SendCommand 19 {"cmd":"click"}'
```

作用：记录旧接口的请求链路，作为对照。旧接口没有完成回调，需根据测试页面状态或应用日志
确认实际点击已经完成；等页面稳定、下一次点击条件与上次一致后继续。
这里外层单引号、JSON 内层双引号的写法用于手机 shell。

#### 6. 手机 shell：执行一次新接口并确认结果

```sh
hidumper -s 16666 -a 'SendCommandWithFinishCallback 19 {"cmd":"onButtonClick"}'
```

作用：记录带完成回调的新接口请求链路，与旧接口对比。
本份文件同一入口类型只发一次旧接口、一次新接口，不并发，也不用额外的 SendCommand 恢复页面。
不用 `SendCommandSync` 替代旧接口。如果无法保持同一节点和等价状态，重新准备一轮采样。

查看调用结果：

```sh
hilog -x | grep -F 'SendCommandWithFinishCallback' | tail -n 10
```

作用：`hilog -x` 读取现有日志后退出，`grep -F` 筛选接口相关日志，`tail -n 10` 显示最后 10 行。
按日志时间确认来自本轮调用，关注以下内容：

```text
request result=0
finish result={"result":"SUCCESS"}
```

前者只说明请求被受理，后者才说明操作成功完成。若查询时尚未完成，可以再次查询；
始终没有本轮完成结果时，先执行第 7 步保存现场并排查，不将该轮当成成功命令样本。
看到完成结果后及时停止采集，避免长时间记录。

#### 7. 手机 shell：停止采集并生成本轮文件

```sh
/system/bin/hitrace --trace_finish -o /data/local/tmp/sendcommand_request_01.trace
```

作用：停止记录，将数据保存到 `-o` 后指定的手机文件中。
该文件名需与后续下载和分析命令保持一致。

```sh
ls -l /data/local/tmp/sendcommand_request_01.trace
```

作用：核对文件已生成，并查看大小和修改时间。文件应非空。
如果停止时报 `tracing on is closed, no trace can be read`，或检查时报文件不存在，
停在这里排查；即便有同名旧文件，也不能当作本轮成功采集。不要改下载另一个历史文件继续分析。

#### 8. 手机 shell：返回电脑 PowerShell

```sh
exit
```

作用：退出手机 shell。后续命令都在电脑 PowerShell 执行。

#### 9. 电脑 PowerShell：下载并核对文件

```powershell
hdc file recv /data/local/tmp/sendcommand_request_01.trace .\sendcommand_request_01.trace
Get-Item .\sendcommand_request_01.trace | Select-Object Name,Length,LastWriteTime
```

第一条将手机的本轮文件下载到电脑当前目录；第一个路径属于手机，第二个路径属于电脑。
第二条确认下载后的名称、大小和修改时间。下载失败应回查第 7 步，不能把旧文件改名替代。

#### 10. 电脑 PowerShell：先检查原始打点

```powershell
$marks = @(Select-String -Path .\sendcommand_request_01.trace -SimpleMatch 'SendCommand.')
Write-Output ("SendCommand 标记行数: " + $marks.Count)
$marks | Select-Object -First 20 | ForEach-Object { $_.Line }
```

作用：直接搜索原始文件中的 `SendCommand.`，显示匹配行数及前 20 行。
调度日志中进程名含 `sendcommand` 不代表存在功能打点，搜索时保留末尾的点。
本例应有 `SendCommand.Request.Legacy.Node` 和 `SendCommand.Request.Callback.Node`
各一条 `S|...` 开始、一条 `F|...` 结束记录。

如果匹配数为 0，先检查本轮开始、停止和下载输出，参见下文“统计为 0 时的诊断”，
不要把后续脚本输出的 0 当成性能通过。若有名称但脚本仍无法识别，保留原始行检查格式。

#### 11. 电脑 PowerShell：分析请求链路

```powershell
py -3 .\send_command_trace.py .\sendcommand_request_01.trace .\sendcommand_request_01.json --metric request
$LASTEXITCODE
```

| 参数 | 作用 |
| --- | --- |
| `py -3` | 使用电脑上的 Python 3 |
| `.\send_command_trace.py` | 分析脚本，路径相对电脑当前目录 |
| `.\sendcommand_request_01.trace` | 刚下载的输入文件 |
| `.\sendcommand_request_01.json` | 脚本生成的分析结果，无需提前创建 |
| `--metric request` | 比较新旧请求链路的新增时延，不能省略，否则默认分析完成通知 5ms 指标 |

输入须为 HiTrace/ftrace 文本，二进制或压缩文件需先转换。
JSON 可以在 trace 可视化工具中打开，查看派生的跨进程异步泳道；原始 trace 不会被修改。
`$LASTEXITCODE` 紧接 Python 命令执行，用于读取退出码：0 为有效比较均满足 1ms 门限，
1 为存在超限比较，2 为数据不完整或有歧义。

#### 结果示例

以下输出用于说明结果字段；耗时和记录数量以实际采集为准：

```text
Input: lines=86574, trace_records=86564, send_command_records=9, request_markers=4, finish_markers=2, diagnostic_probes=0, unrecognized_markers=0
Node: legacy_ms=0.889, callback_ms=1.656, delta_ms=0.767
comparisons=1
paired=2, over_1ms=0, excluded=0
Analysis JSON: sendcommand_request_01.json
```

| 输出 | 含义 |
| --- | --- |
| `legacy_ms=0.889` | 旧接口请求链路耗时 0.889ms |
| `callback_ms=1.656` | 新接口请求链路耗时 1.656ms，表示请求阶段，不是完成通知返回耗时 |
| `delta_ms=0.767` | 新增时延为 1.656 − 0.889 = 0.767ms，小于 1ms |
| `comparisons=1`、`paired=2` | 一组有效对比，两次请求各自都有完整起止点 |
| `over_1ms=0`、`excluded=0` | 没有差值超过 1ms 的比较，也没有被排除的样本 |
| `diagnostic_probes=0`、`unrecognized_markers=0` | 没有识别到额外探针或请求标记格式错误 |

#### 测试通过标准（1ms）

**判断的是新旧请求链路的差值：`delta_ms = callback_ms - legacy_ms <= 1`，等于 1ms 也满足门限。**
单轮按上文各发送一次旧、新 Node 接口时，以下条件须同时满足：

| 检查项 | 通过条件 |
| --- | --- |
| 测试环境 | 同一目标设备、版本、应用和 SA，正常负载，诊断开关为 off |
| 对照条件 | 相同节点、相同命令及参数，页面状态可比；先一条完成后再发下一条 |
| 业务执行 | 旧接口实际操作成功；新接口 `request result=0`，收到一次成功的 `finish result`，操作符合预期 |
| 起止记录 | `request_markers=4`、`paired=2`、`comparisons=1`，形成一组完整的新旧请求对照 |
| 数据有效性 | `excluded=0`、`diagnostic_probes=0`、`unrecognized_markers=0`，没有 `INCOMPLETE` |
| 新增时延 | `delta_ms <= 1`，JSON 的比较结果中 `within_1ms` 为 true，汇总 `over_1ms=0` |
| 脚本结果 | 使用 `--metric request`，Python 命令之后的 `$LASTEXITCODE` 为 0 |

判读示例（均假设采样条件和业务结果有效）：

| 旧接口耗时 | 新接口耗时 | 新增时延 | 该样本结论 |
| --- | --- | --- | --- |
| 0.889ms | 1.656ms | 0.767ms | 通过：差值不超过 1ms |
| 0.872ms | 3.746ms | 2.874ms | 不通过：差值超过 1ms |

**不通过：** 有效比较的 `delta_ms > 1`，即 `over_1ms > 0`。
**不能判定通过：** `comparisons=0`、缺失或重复标记、存在 excluded、诊断开启、命令失败，
或者新旧命令及测试条件不一致。这些情况需要排查后重新采样；`over_1ms=0` 本身不代表通过。

1ms 限制针对新增时延，不是新接口的总耗时。`finish_markers=2` 仅表示存在完成通知打点，
该部分的 5ms 指标需使用 `--metric finish` 单独分析。

#### 重复采样

多轮采样选择尚未使用的独立文件名，手机导出、电脑接收、原始打点检查和脚本输入输出名称同步修改。
建议每类场景采集至少 30 组，可交替新旧接口的调用顺序；这是测试建议，不是规格规定的次数。
完成约定的场景和轮次后，确认所有有效比较均满足上述条件，才能判定该场景的请求链路新增时延测试通过。
保留全部有效样本及超过 1ms 的结果，记录设备版本、负载、命令、成功状态和耗时，
不能用删除超限样本或只报告平均值的方式宣称满足“不超过 1ms”。
无节点 ID 的 `Global` 接口另用相同的 keycode/relaxed 命令按上述流程对比；
各命令类型分别记录，不将旧 Node 和新 Global 混为一组。

### 测量边界

本区间包括 SA 入口准备、请求 IPC、应用分发以及等待 UI 线程的时间。
Global 还包括命令解析与事件准备，新旧入口都测到实际处理函数调用之前；Node 测到 UI 任务开始。
本区间不包括实际命令执行、动画完成或返回通知链路，不能据此单独认定整个功能新增开销符合 AC-13。
完整性能验收还需在目标设备上重复测量，统计分位数与异常值，并验证命令结果、完成通知和资源指标。

## 完成通知 IPC（5ms）

- start：应用侧 `UiReportProxy::ReportCommandResult` 开始组装返回 IPC 数据。
- end：SA 的 `UiReportStub::OnRemoteRequest` 收到 `REPORT_COMMAND_RESULT`，成功读出 requestId 和 resultJson。
- 名称：`SendCommand.FinishCallbackIPC.<requestId>`，成功和失败结果均打点，不记录结果正文。

测量区间包含返回数据封包、IPC 传输/调度和 SA 解包。
结果 JSON 已在 start 前生成；SA 匹配 pending 状态及执行业务 callback 在 end 后进行。
因此该区间是 AC-13 的完成通知 IPC 指标，不能单独证明完整 AC-13 达标。
迟到结果也记录接收 end，但不表示业务 callback 被触发；原来的超时、断链和回调过滤规则保持不变。
未送达或解包失败时没有 end，不伪造完成时间。

### 使用已有采集文件分析完成通知

如果请求链路采集文件包含完整的 `SendCommand.FinishCallbackIPC.<requestId>` 起止记录，
且采集时关闭了诊断、命令执行成功，可以直接复用该文件，无需为了切换指标重新采集。
例如，在电脑 **PowerShell** 的脚本和 trace 所在目录执行：

```powershell
py -3 .\send_command_trace.py .\sendcommand_request_01.trace .\sendcommand_request_01_finish.json --metric finish
$finishResult = Get-Content .\sendcommand_request_01_finish.json -Raw | ConvertFrom-Json
$finishResult.samples | Select-Object request_id, app_pid, sa_pid, duration_ms, within_5ms
```

第一条选择完成通知 IPC 的 5ms 指标，输出新 JSON，保留原来的请求链路分析结果。
后两条读取 JSON，显示每个请求的实测毫秒数和是否满足门限。
`finish_markers=2` 表示识别到两条标记，是否能配成有效样本仍以 `paired` 和 `excluded` 为准。

### Windows 真机完成通知 IPC 测试步骤（5ms）

测试目标：测量一条命令的完成结果从应用上报到 SA 收到并解包的耗时，验证该区间是否不超过 5ms。
本流程只需调用带完成回调的新接口，不需要执行旧接口或计算新旧耗时差。
测量边界见本节开头；结果 JSON 的生成以及 SA 接收后的 pending 匹配和业务 callback 调用不在该区间。
不能仅凭本区间达标认定“UI 最终完成到业务 callback 触发”的完整指标通过。

准备条件：

- 使用已部署并生效的 trace 版本，设备处于正常负载，测试应用保持前台，页面和动画稳定。
- 沿用已经验证能成功触发完成回调的命令。下文 `19` 和 `onButtonClick` 为示例，需替换成当前页面
  的有效节点 ID 和命令；避免使用会退出进程或断开连接的操作作为正常性能样本。
- 电脑安装 Python 3、`hdc`，将 `test/tools/UISession/send_command_trace.py` 复制到电脑工作目录。
- 本例统一使用 `sendcommand_finish_01.trace` 和 `sendcommand_finish_01.json`；再次采样时选择新名称，
  同步修改手机保存、电脑下载和脚本分析中的文件名。
- 按步骤确认结果后继续，开始采集至保存文件期间保持同一应用和 SA 运行实例。

#### 1. 电脑 PowerShell：检查目录、脚本和连接

在存放脚本的电脑目录中执行：

```powershell
Get-Location
Get-Item .\send_command_trace.py
py -3 --version
hdc list targets
```

依次用于显示当前目录、确认脚本存在、确认 Python 3 可用、确认手机已连接。
本例按仅连接一台目标设备执行，后续 trace 下载和 JSON 输出均在该目录。
若电脑当前目录是仓库根目录，检查和运行脚本时将脚本路径改为
`.\test\tools\UISession\send_command_trace.py`。

#### 2. 电脑 PowerShell：进入手机 shell

```powershell
hdc shell
```

作用：进入手机命令环境。第 3～8 步都在这个手机 shell 内执行。

#### 3. 手机 shell：连接应用并关闭诊断

```sh
hidumper -s 16666 -a "Connect"
hidumper -s 16666 -a "SendCommandTraceDiagnostic off"
```

第一条建立示例 SA 与前台应用的连接；第二条关闭额外诊断日志和探针，正常 trace 仍会记录。
确认连接成功，并看到 `SendCommand trace diagnostic: off` 后继续。
如果命令不支持或连接失败，先排查，不将后续数据作为正式性能样本。

#### 4. 手机 shell：检查等级并开始采集

```sh
/system/bin/hitrace --get_level
```

作用：查看 trace 等级。`Info` 或 `Debug` 可以继续；若为 `Critical`、`Commercial` 等会过滤
Info 打点的等级，先调整：

```sh
/system/bin/hitrace --trace_level Info
```

然后开始采集：

```sh
/system/bin/hitrace --trace_begin -b 16384 ace
```

- `--trace_begin`：开始记录。
- `-b 16384`：设置缓冲区大小参数为 16384 KB。
- `ace`：开启 ArkUI 打点，包含完成通知 IPC 事件。

确认出现 `OpenRecording done.` 等成功信息后再发命令。报错时先处理错误，不能继续分析历史文件。
本流程使用文本输出，不添加 `--raw`、`--record` 或 `-z`。

#### 5. 手机 shell：执行一次带完成回调的新接口

```sh
hidumper -s 16666 -a 'SendCommandWithFinishCallback 19 {"cmd":"onButtonClick"}'
```

作用：发送一条命令，产生一次完成结果上报。外层单引号、JSON 内层双引号的写法用于手机 shell。
本轮先只发一次，便于核对一条命令、一组 IPC 起止点和一次业务回调。

#### 6. 手机 shell：确认命令完成

```sh
hilog -x | grep -F 'SendCommandWithFinishCallback' | tail -n 10
```

作用：读取已有日志并退出，筛选接口相关记录，显示最后 10 行。
核对日志时间属于本轮调用，并确认请求受理和完成结果：

```text
request result=0
finish result={"result":"SUCCESS"}
```

`request result=0` 只说明受理；`finish result` 成功且符合预期回调次数才是正常成功样本。
失败结果也可能产生完整 trace，脚本不会读取业务结果来判断命令成功与否。
尚未完成可再次查询；始终无完成结果时，先保存采集现场再排查，不计算为成功样本。

#### 7. 手机 shell：停止并保存采集

```sh
/system/bin/hitrace --trace_finish -o /data/local/tmp/sendcommand_finish_01.trace
ls -l /data/local/tmp/sendcommand_finish_01.trace
```

第一条停止采集，`-o` 指定手机端文件名；第二条核对文件存在、大小非零及修改时间。
收到完成结果后及时保存。若报 `tracing on is closed` 或文件不存在，应停止后续分析并排查，不能用旧文件替代。

#### 8. 手机 shell：返回电脑

```sh
exit
```

作用：退出手机 shell。后续命令全部在电脑 PowerShell 执行。

#### 9. 电脑 PowerShell：下载采集文件

```powershell
hdc file recv /data/local/tmp/sendcommand_finish_01.trace .\sendcommand_finish_01.trace
Get-Item .\sendcommand_finish_01.trace | Select-Object Name,Length,LastWriteTime
```

第一条将手机文件下载到电脑当前目录，第二条核对名称、大小和时间。下载源文件必须与第 7 步一致。

#### 10. 电脑 PowerShell：检查完成通知打点

```powershell
$finishMarks = @(Select-String -Path .\sendcommand_finish_01.trace -SimpleMatch 'SendCommand.FinishCallbackIPC.')
Write-Output ("完成通知标记行数: " + $finishMarks.Count)
$finishMarks | ForEach-Object { $_.Line }
```

作用：检查原始文件中的完成通知记录。一次正常调用预期有同一 requestId 的两条标记：

- `S|...|H:SendCommand.FinishCallbackIPC.<requestId>|0...`：应用侧开始上报。
- `F|...|H:SendCommand.FinishCallbackIPC.<requestId>|0...`：SA 收到并解包。

两条记录的 PID 不同，名称中的 requestId 相同；两条标记代表一次 IPC 样本，不是两次 callback。
如果为 0 或只有一端，先检查采集和完成结果，不能据此判断耗时为 0 或性能通过。

#### 11. 电脑 PowerShell：分析 5ms 指标

```powershell
py -3 .\send_command_trace.py .\sendcommand_finish_01.trace .\sendcommand_finish_01.json --metric finish
$LASTEXITCODE
```

第一条运行分析脚本：`.trace` 是输入，`.json` 是自动生成的结果，无需提前创建。
`--metric finish` 选择完成通知 IPC 的 5ms 指标。
第二条需紧接 Python 命令执行，读取脚本退出码：0 为样本有效且均不超过 5ms，1 为存在超限样本，
2 为数据不完整或有歧义。finish 模式不输出请求链路的 `comparisons`、`delta_ms`。

例如一条有效且未超限的样本会显示以下汇总，具体计数以实际采集为准：

```text
paired=1, over_5ms=0, excluded=0
Analysis JSON: sendcommand_finish_01.json
```

#### 12. 电脑 PowerShell：查看具体耗时

```powershell
$finishResult = Get-Content .\sendcommand_finish_01.json -Raw | ConvertFrom-Json
$finishResult.samples | Select-Object request_id, app_pid, sa_pid, duration_ms, within_5ms
$finishResult.excluded | Format-List
```

第一条读取分析 JSON；第二条显示各请求的耗时；第三条显示被排除样本的原因，正常应无条目。

| 字段 | 含义 |
| --- | --- |
| `request_id` | 用于匹配应用和 SA 起止点的请求编号 |
| `app_pid`、`sa_pid` | 两端实际进程 ID |
| `duration_ms` | 完成通知 IPC 实测耗时，单位 ms |
| `within_5ms` | `duration_ms <= 5` 时为 True，否则为 False |

#### 测试通过标准（5ms）

**判断的是完成通知 IPC 的实际耗时：`duration_ms <= 5`，等于 5ms 也满足门限。**
单轮按上文只发送一条 callback 命令时，以下条件须同时满足：

| 检查项 | 通过条件 |
| --- | --- |
| 测试环境 | 目标设备正常负载，采集期间应用和 SA 未重启，诊断开关为 off |
| 业务执行 | 本轮 `request result=0`，收到一次成功的 `finish result`，且应用实际操作符合预期 |
| 起止记录 | `finish_markers=2`、`paired=1`，同一 requestId 的应用 start 与 SA end 成功配对 |
| 数据有效性 | `excluded=0`、`diagnostic_probes=0`、`unrecognized_markers=0`，没有 `INCOMPLETE` |
| 耗时 | `duration_ms <= 5`、`within_5ms=True`，汇总 `over_5ms=0` |
| 脚本结果 | 使用 `--metric finish`，Python 命令之后的 `$LASTEXITCODE` 为 0 |

例如 JSON 显示：

```text
duration_ms : 0.786
within_5ms  : True
```

表示该条通知耗时 0.786ms，小于 5ms；其余条件也满足时，这条样本通过。
`request_id`、`app_pid`、`sa_pid` 是请求和进程标识，不参与 5ms 大小比较。

**不通过：** 有效样本的 `duration_ms > 5`，即 `within_5ms=False`、`over_5ms > 0`。
**不能判定通过：** 无有效配对、缺失或重复标记、存在 excluded、诊断开启、命令失败或没有实际业务回调。
这些情况需要排查后重新采样；单看 `within_5ms=True` 或完整起止标记不足以确认测试通过。

5ms 是通知区间的实测耗时，无需减去旧接口耗时，也不是 5000ms 等待超时。
上述通过结论仅针对本节定义的完成通知 IPC 区间；完整需求的测量边界见本节开头。

#### 重复采样

多轮验证每轮使用独立文件，记录命令、设备版本、负载、执行结果和耗时，保留超限样本。
完成约定的场景和轮次后，确认所有有效样本均满足上述条件，才能判定该场景的 IPC 耗时测试通过。
同一采集期间不要重启应用或 SA，避免请求编号复用造成配对歧义。
可在 trace 可视化工具中打开生成的 JSON，查看 `SendCommand.FinishCallbackIPC` 异步泳道。

### 查看完成通知异步泳道

系统 HiTrace 的 S/F 按 PID 配对，应用和 SA 的不同 PID 无法直接闭合。
将采集文件解码为 HiTrace/ftrace 文本后，运行：

```bash
python3 test/tools/UISession/send_command_trace.py capture.trace send_command_ipc.json
```

在 trace 可视化工具中打开输出 JSON，查看 `SendCommand.FinishCallbackIPC` 异步泳道。
这是按完整 requestId 关联后生成的展示泳道，原始 trace 文件不修改。
起止时间保留采集值，展示位置统一放在应用进程，args 中保留实际 app_pid、sa_pid、request_id 和 duration_ms。
脚本只接受每个 requestId 恰有一个 start/end 且时间顺序正确的跨进程样本。
缺失、重复、同进程或时间倒置的样本列入 excluded，不参与耗时判断。

退出码：0 表示已配对样本均不超过 5ms 且无 excluded；1 表示存在超过 5ms 的样本；
2 表示没有有效样本或存在 excluded，需要检查采集、重启、重复通知和断链等情况。
该判断不替代设备上的命令成功、callback 次数以及 AC-13 其余指标验收。

## 统计为 0 时的诊断

脚本打印输入计数、每项 excluded 的原因及起止点数量，并在 JSON 中保存 `diagnostics`。

| 输出 | 含义 |
| --- | --- |
| `no HiTrace/ftrace marker records` | 没有支持的文本标记记录；确认输入是解码后的 trace 文本 |
| `no SendCommand. markers` | 有其它 trace，但没有 SendCommand 打点；尚未测出耗时 |
| `none match --metric request/finish` | 有 SendCommand 打点，所选指标没有对应标记 |
| `unrecognized marker format` | 名称存在但格式不支持；按给出的行号检查原始文件 |
| `missing start/end` | 仅采到一端，检查对应进程的打点和采集区间 |
| `duplicate endpoints` | 同类请求重复，无法唯一关联；每份采集各执行一次新旧请求 |
| `no valid legacy/callback comparison` | 缺少完整的新旧基线，不能计算新增时延 |
| `diagnostic capture` | 包含探针；关闭诊断并重新采集性能数据 |

`comparisons=0, paired=0, excluded=0` 不表示延迟为 0 或性能通过。
脚本返回 2 并提示没有性能结论。生成的 JSON 可以只有诊断信息，没有可显示的异步泳道。
UTF-8 BOM 和 Windows CRLF 可直接处理；二进制 trace 需要先解码。

## SA 打点入口诊断（仅排障）

更新 `libui_session.z.so` 和示例 SA `libui_sa.z.so` 后，诊断开关默认关闭。
它只控制当前 SA 进程，不依赖 Connect，也不会修改系统 HiTrace 开关。
在 Windows CMD 中执行：

```bat
hdc shell "hidumper -s 16666 -a 'SendCommandTraceDiagnostic on'"
```

在正在采集 trace 时执行原来的 SendCommand/SendCommandWithFinishCallback 命令。
请求 start 和完成通知 end 调用后，会输出名称、phase、PID、ACE tag 状态和独立探针结果：

```bat
hdc shell "hilog -x" | findstr /L /C:"SendCommandTraceDiagnostic"
```

- `phase=S name=SendCommand.Request.Callback.Node`：执行到了 SA 请求打点调用。
- `phase=F name=SendCommand.FinishCallbackIPC.<id>`：执行到了 SA 接收完成通知的打点调用。
- `aceTagEnabled=0`：当次查询的 ACE tag 未启用，普通 HiTrace 记录可能被过滤。
- `probeOpenErrno`：探针无法打开 debugfs/tracefs 的 trace_marker，数值为 errno。
- `probeBytes=实际/期望` 和 `probeWriteErrno`：独立探针的写入结果；短写也需关注。

探针仅写 `SendCommand.TraceDiagnostic` 计数器，不伪造 Request/FinishCallbackIPC 起止点。
HiTrace 的 Start/Finish API 返回 void，**探针写入成功不能证明 HiTrace 原始 S/F 写入成功，
也不能证明采集器保留了记录**。它用于区分调用未到达、tag 关闭、marker 路径不可写等情况。
若探针可见而正常标记不可见，继续核对 HiTrace 的等级/过滤状态与实际加载版本。

排障后关闭诊断，并重新开始一份性能采集：

```bat
hdc shell "hidumper -s 16666 -a 'SendCommandTraceDiagnostic off'"
```

诊断模式有日志和文件 I/O 开销，不能用于 1ms/5ms 性能验收。
脚本发现探针会返回 2；若探针本身丢失，脚本无法推断诊断是否开启，采样者仍须确保先关闭。

分析脚本回归（无需手机）：

```bash
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s test/tools/UISession -p 'test_send_command_trace.py' -v
```

`test_send_command_trace.py` 验证跨进程配对、1ms/5ms 阈值、缺失或重复标记及诊断提示，
防止无有效样本时误判性能通过；不能替代设备采集验证。
