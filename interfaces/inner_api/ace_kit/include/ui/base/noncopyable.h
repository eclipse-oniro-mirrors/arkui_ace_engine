/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_INTERFACES_INNER_API_ACE_KIT_INCLUDE_BASE_NON_COPYABLE_H
#define FOUNDATION_ACE_INTERFACES_INNER_API_ACE_KIT_INCLUDE_BASE_NON_COPYABLE_H

#define ACE_DISALLOW_COPY_AND_MOVE(className) \
    ACE_DISALLOW_COPY(className);             \
    ACE_DISALLOW_MOVE(className)

#define ACE_DISALLOW_COPY(className)      \
    className(const className&) = delete; \
    className& operator=(const className&) = delete

#define ACE_DISALLOW_MOVE(className) \
    className(className&&) = delete; \
    className& operator=(className&&) = delete

namespace OHOS::Ace {

class NonCopyable {
protected:
    NonCopyable() = default;
    virtual ~NonCopyable() = default;

private:
    ACE_DISALLOW_COPY_AND_MOVE(NonCopyable);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_INTERFACES_INNER_API_ACE_KIT_INCLUDE_BASE_NON_COPYABLE_H
