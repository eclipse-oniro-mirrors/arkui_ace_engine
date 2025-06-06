/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef TEST_UNITTEST_CORE_EVENT_FOCUS_API_TEST_H
#define TEST_UNITTEST_CORE_EVENT_FOCUS_API_TEST_H
#include <cstddef>
#include <cstdint>

#include "gtest/gtest.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/unittest/core/event/focus_core/focus_test_base.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/theme/app_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/event/touch_event.h"
#include "core/components_ng/manager/focus/focus_view.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/key_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
struct FocusAreaNodeOffset {
    float nodeAx;
    float nodeAy;
    float nodeBx;
    float nodeBy;
    float nodeCx;
    float nodeDx;
    float nodeCy;
    float nodeDy;
};
struct FocusAreaTestCase {
    bool isFocusable;
    bool isFocusBrotherNode;
    FocusStep StepMethod;
    FocusAreaNodeOffset nodeOffset;
    FocusAreaTestCase(bool isFocusable, bool isFocusBrotherNode, FocusStep StepMethod, FocusAreaNodeOffset nodeOffset)
        : isFocusable(isFocusable), isFocusBrotherNode(isFocusBrotherNode), StepMethod(StepMethod),
          nodeOffset { nodeOffset.nodeAx, nodeOffset.nodeAy, nodeOffset.nodeBx, nodeOffset.nodeBy, nodeOffset.nodeCx,
              nodeOffset.nodeCy, nodeOffset.nodeDx, nodeOffset.nodeDy }
    {}
};

class FocusAreaTestNG : public FocusTestBaseNg, public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    static RefPtr<FrameNode> CreatNode(float nodex, float nodey);
    RefPtr<FrameNode> CreatRootNode();
    void InitFocusareaTest();
};

} // namespace OHOS::Ace::NG

#endif // TEST_UNITTEST_CORE_EVENT_FOCUS_WINDOW_TEST_H