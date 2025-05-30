/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ACE_FRAMEWORK_CJ_PIXEL_UNIT_CONVERT_FFI_H
#define OHOS_ACE_FRAMEWORK_CJ_PIXEL_UNIT_CONVERT_FFI_H

#include "cj_common_ffi.h"

extern "C" {
CJ_EXPORT double FfiOHOSAceFrameworkVp2Px(double value);
CJ_EXPORT double FfiOHOSAceFrameworkPx2Vp(double value);
CJ_EXPORT double FfiOHOSAceFrameworkFp2Px(double value);
CJ_EXPORT double FfiOHOSAceFrameworkPx2Fp(double value);
CJ_EXPORT double FfiOHOSAceFrameworkLpx2Px(double value);
CJ_EXPORT double FfiOHOSAceFrameworkPx2Lpx(double value);
}
#endif //OHOS_ACE_FRAMEWORK_CJ_PIXEL_UNIT_CONVERT_FFI_H
