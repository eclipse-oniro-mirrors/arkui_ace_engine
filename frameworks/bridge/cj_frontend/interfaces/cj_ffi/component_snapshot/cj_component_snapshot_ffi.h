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

#ifndef OHOS_ACE_FRAMEWORK_CJ_COMPONENT_SNAPSHOT_FFI_H
#define OHOS_ACE_FRAMEWORK_CJ_COMPONENT_SNAPSHOT_FFI_H

#include "bridge/cj_frontend/interfaces/cj_ffi/cj_macro.h"
#include "bridge/cj_frontend/interfaces/cj_ffi/cj_common_ffi.h"

extern "C" {
struct CSnapshotGetSyncRet {
    int32_t errCode;
    int64_t pixelMapId;
};

// NOTE: CSnapshotOptions and the functions taking it are shipped ABI. Do NOT
// modify this struct in place; passing it by value ties the layout to the
// calling convention (8-byte structs are passed in registers). Extend via a
// new V2 struct and a new V2 symbol instead (see CJBadgeStyleV2 precedent).
struct CSnapshotOptions {
    float scale;
    bool waitUntilRenderFinished;
};

// Mirrors the Cangjie @C structs in cj_component_snapshot.cj. Field order and
// types must stay in sync with the Cangjie definitions (C layout rules).
struct CSnapshotRegion {
    int32_t regionMode; // 0: COMMON, 1: LOCALIZED, 2: NO_REGION (NG::SnapshotRegionMode)
    double regionStart; // SnapshotRegion.left or LocalizedSnapshotRegion.start, unit: px
    double regionEnd; // SnapshotRegion.right or LocalizedSnapshotRegion.end, unit: px
    double regionTop; // unit: px
    double regionBottom; // unit: px
};

struct CColorModeOptions {
    uint32_t colorSpace; // ArkTS ColorSpace enum value, default 4 (SRGB)
    bool isAuto;
};

struct CDynamicRangeModeOptions {
    uint32_t dynamicRangeMode; // default 2 (STANDARD)
    bool isAuto;
};

struct CSnapshotOptionsV2 {
    float scale;
    bool waitUntilRenderFinished;
    CSnapshotRegion region;
    CColorModeOptions colorMode;
    CDynamicRangeModeOptions dynamicRangeMode;
};

CJ_EXPORT void FfiOHOSAceFrameworkComponentSnapshotGet(
    const char* id,
    void (*callback)(NativeOptionInt32 errCode, NativeOptionInt64 pixelMapId),
    CSnapshotOptions options);

CJ_EXPORT void FfiOHOSAceFrameworkComponentSnapshotGetV2(
    int32_t instanceId,
    const char* id,
    void (*callback)(NativeOptionInt32 errCode, NativeOptionInt64 pixelMapId),
    CSnapshotOptionsV2 options);

CJ_EXPORT void FfiOHOSAceFrameworkComponentSnapshotCreate(
    void (*builder)(),
    void (*callback)(NativeOptionInt32 errCode, NativeOptionInt64 pixelMapId),
    int32_t delay,
    bool checkImageStatus,
    CSnapshotOptions options);

CJ_EXPORT CSnapshotGetSyncRet FfiOHOSAceFrameworkComponentSnapshotGetSync(
    const char* id,
    CSnapshotOptions options);
}

#endif // OHOS_ACE_FRAMEWORK_CJ_COMPONENT_SNAPSHOT_FFI_H