/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "adapter/ohos/entrance/ui_session/include/ui_session_trace.h"

#include <atomic>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "hitrace_meter.h"
#include "securec.h"

#include "base/utils/utils.h"
#include "adapter/ohos/entrance/ui_session/include/ui_session_log.h"

namespace OHOS::Ace {
namespace {
const size_t MAX_STRING_SIZE = 512;
constexpr char COMMAND_RESULT_TRACE_FORMAT[] = "SendCommand.FinishCallbackIPC.%llu";
std::atomic<bool> g_commandTraceDiagnosticEnabled { false };

void ReportCommandTraceDiagnostic(char phase, const char* name)
{
    if (!g_commandTraceDiagnosticEnabled.load(std::memory_order_relaxed)) {
        return;
    }
    // HiTrace APIs return void. This separate probe checks the marker path, not
    // whether HiTrace wrote the real S/F record or whether the collector retained it.
    const bool tagEnabled = IsTagEnabled(HITRACE_TAG_ACE);
    const char* markerPath = "/sys/kernel/debug/tracing/trace_marker";
    int fd = open(markerPath, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        markerPath = "/sys/kernel/tracing/trace_marker";
        fd = open(markerPath, O_WRONLY | O_CLOEXEC);
    }
    if (fd < 0) {
        const int openError = errno;
        LOGI("SendCommandTraceDiagnostic phase=%{public}c name=%{public}s pid=%{public}d "
             "aceTagEnabled=%{public}d probeOpenErrno=%{public}d",
            phase, name, getpid(), tagEnabled, openError);
        return;
    }
    char marker[MAX_STRING_SIZE] = { 0 };
    int length = sprintf_s(marker, sizeof(marker), "C|%d|H:SendCommand.TraceDiagnostic|1|I39", getpid());
    ssize_t written = -1;
    int writeError = 0;
    if (length > 0) {
        written = write(fd, marker, static_cast<size_t>(length));
        writeError = written < 0 ? errno : 0;
    }
    close(fd);
    LOGI("SendCommandTraceDiagnostic phase=%{public}c name=%{public}s pid=%{public}d "
         "aceTagEnabled=%{public}d probePath=%{public}s probeBytes=%{public}zd/%{public}d "
         "probeWriteErrno=%{public}d; diagnostic capture only",
        phase, name, getpid(), tagEnabled, markerPath, written, length, writeError);
}
}

void UiSessionSetCommandTraceDiagnosticEnabled(bool enabled)
{
    g_commandTraceDiagnosticEnabled.store(enabled, std::memory_order_relaxed);
}

void UiSessionTraceBegin(const char* name)
{
    CHECK_NULL_VOID(name);
    StartTraceEx(HITRACE_LEVEL_INFO, HITRACE_TAG_ACE, name);
}

void UiSessionTraceEnd()
{
    FinishTraceEx(HITRACE_LEVEL_INFO, HITRACE_TAG_ACE);
}

void UiSessionCommandRequestTraceBegin(const char* commandType)
{
    CHECK_NULL_VOID(commandType);
    // Legacy commands have no request ID; compare one command per entry per capture.
    char name[MAX_STRING_SIZE] = { 0 };
    if (sprintf_s(name, sizeof(name), "SendCommand.Request.%s", commandType) < 0) {
        return;
    }
    // The non-format API initializes the trace marker before checking enabled tags.
    StartAsyncTraceEx(HITRACE_LEVEL_INFO, HITRACE_TAG_ACE, name, 0, "");
    ReportCommandTraceDiagnostic('S', name);
}

void UiSessionCommandResultTraceBegin(uint64_t requestId)
{
    // Keep the full request ID in the name; HiTrace's task ID is only int32_t.
    char name[MAX_STRING_SIZE] = { 0 };
    if (sprintf_s(name, sizeof(name), COMMAND_RESULT_TRACE_FORMAT,
        static_cast<unsigned long long>(requestId)) < 0) {
        return;
    }
    StartAsyncTraceEx(HITRACE_LEVEL_INFO, HITRACE_TAG_ACE, name, 0, "");
    ReportCommandTraceDiagnostic('S', name);
}

void UiSessionCommandResultTraceEnd(uint64_t requestId)
{
    char name[MAX_STRING_SIZE] = { 0 };
    if (sprintf_s(name, sizeof(name), COMMAND_RESULT_TRACE_FORMAT,
        static_cast<unsigned long long>(requestId)) < 0) {
        return;
    }
    FinishAsyncTraceEx(HITRACE_LEVEL_INFO, HITRACE_TAG_ACE, name, 0);
    ReportCommandTraceDiagnostic('F', name);
}

bool UiSessionTraceBeginWithArgv(const char* format, va_list args)
{
    char name[MAX_STRING_SIZE] = { 0 };
    if (vsnprintf_s(name, sizeof(name), sizeof(name) - 1, format, args) < 0) {
        return false;
    }
    UiSessionTraceBegin(name);
    return true;
}

UiSessionScopedTrace::UiSessionScopedTrace(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    strValid_ = UiSessionTraceBeginWithArgv(format, args);
    va_end(args);
}

UiSessionScopedTrace::~UiSessionScopedTrace()
{
    if (strValid_) {
        UiSessionTraceEnd();
    }
}
} // namespace OHOS::Ace
