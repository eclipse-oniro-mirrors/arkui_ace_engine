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

#ifndef ARKUI_TEST_HOST_MANAGER_STUBS_H
#define ARKUI_TEST_HOST_MANAGER_STUBS_H

#include <cstdint>

// Test-controllable clock, in nanoseconds like the real one, backing the
// GetSysTimestamp() framework double defined in host_manager_stubs.cpp. The
// clock state is file-local to that TU and reachable only through these
// accessors: GetSysTimestamp mirrors the real time_util.h declaration,
// SetSysTimestamp is the test-side control point.
namespace OHOS::Ace {
int64_t GetSysTimestamp();
void SetSysTimestamp(int64_t nanoseconds);
} // namespace OHOS::Ace

#endif // ARKUI_TEST_HOST_MANAGER_STUBS_H
