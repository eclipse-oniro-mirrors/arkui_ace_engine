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
#ifndef ARKUI_NATIVE_NODE_SCROLL_OPTION_H
#define ARKUI_NATIVE_NODE_SCROLL_OPTION_H

#ifdef __cplusplus
extern "C" {
#endif

// Field types are those of ArkUI_Bool, ArkUI_Int32 and ArkUI_Float32. Spelling them directly keeps this
// header free of arkoala_api.h, so including it does not pull that header into more translation units.
struct ArkUIScrollToIndexOptions {
    int smooth;
    int align;
    // Pixels for animated FastScroll; preserve the legacy raw offset for non-animated calls.
    float extraOffset;
    int fastScrollMode;
};

#ifdef __cplusplus
}
#endif
#endif // ARKUI_NATIVE_NODE_SCROLL_OPTION_H
