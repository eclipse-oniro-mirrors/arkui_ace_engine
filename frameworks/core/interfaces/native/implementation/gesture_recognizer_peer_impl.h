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
#ifndef FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_GESTURE_RECOGNIZER_PEER_IMPL_H
#define FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_GESTURE_RECOGNIZER_PEER_IMPL_H

#include <optional>
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/gestures/recognizers/gesture_recognizer.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"

#include "arkoala_api_generated.h"

struct GestureRecognizerPeer {
    OHOS::Ace::RefPtr<OHOS::Ace::NG::NGGestureRecognizer> GetRecognizer() const
    {
        return recognizer_;
    }

    void SetRecognizer(const OHOS::Ace::RefPtr<OHOS::Ace::NG::NGGestureRecognizer>& recognizer)
    {
        recognizer_ = recognizer;
    }
private:
     OHOS::Ace::RefPtr<OHOS::Ace::NG::NGGestureRecognizer> recognizer_;
};
#endif //FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_GESTURE_RECOGNIZER_PEER_IMPL_H