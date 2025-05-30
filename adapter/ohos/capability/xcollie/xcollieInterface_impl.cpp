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

#include "xcollieInterface_impl.h"

#include "xcollie/xcollie.h"

namespace OHOS::Ace {
XcollieInterface& XcollieInterface::GetInstance()
{
    static XcollieInterfaceImpl instance;
    return instance;
}

int32_t XcollieInterfaceImpl::SetTimerCount(const std::string& name, uint32_t timeLimit, int32_t countLimit)
{
    return HiviewDFX::XCollie::GetInstance().SetTimerCount(name, timeLimit, countLimit);
}

void XcollieInterfaceImpl::TriggerTimerCount(const std::string& name, bool bTrigger, const std::string& message)
{
    HiviewDFX::XCollie::GetInstance().TriggerTimerCount(name, bTrigger, message);
}
} //namespace OHOS::Ace
