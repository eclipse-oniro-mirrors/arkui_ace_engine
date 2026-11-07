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

#ifndef TEST_UNITTEST_INTERFACES_MOCK_PIXELMAP_NATIVE_IMPL_H
#define TEST_UNITTEST_INTERFACES_MOCK_PIXELMAP_NATIVE_IMPL_H

#include <memory>

// Test-only substitute for the image_framework header "pixelmap_native_impl.h".
// node_image_collection.cpp never dereferences Media::PixelMap; it only moves
// shared_ptr instances into OH_PixelmapNative, so a minimal definition is
// sufficient for the unit test. node_model.h already forward-declares
// OHOS::Media::PixelMap, which stays compatible with this definition.
namespace OHOS::Media {
class PixelMap {
public:
    PixelMap() = default;
    virtual ~PixelMap() = default;
};
} // namespace OHOS::Media

struct OH_PixelmapNative {
    explicit OH_PixelmapNative(std::shared_ptr<OHOS::Media::PixelMap> pixelMap) : pixelMap(std::move(pixelMap)) {}
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
};

#endif // TEST_UNITTEST_INTERFACES_MOCK_PIXELMAP_NATIVE_IMPL_H
