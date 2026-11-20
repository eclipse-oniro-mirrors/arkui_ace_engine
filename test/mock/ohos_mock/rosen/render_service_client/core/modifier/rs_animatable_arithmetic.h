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
#ifndef FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_CLIENT_CORE_MODIFIER_RS_ANIMATABLE_ARITHMETIC_H
#define FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_CLIENT_CORE_MODIFIER_RS_ANIMATABLE_ARITHMETIC_H
#include "modifier/rs_property.h"
namespace OHOS::Rosen {
template<typename T>
class RSArithmetic {
public:
    virtual ~RSArithmetic() = default;
    virtual bool IsEqual(const T&) const = 0;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_CLIENT_CORE_MODIFIER_RS_ANIMATABLE_ARITHMETIC_H
