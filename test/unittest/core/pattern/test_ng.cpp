/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "test/unittest/core/pattern/test_ng.h"

#define private public
#define protected public
#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#undef private
#undef protected

namespace OHOS::Ace::NG {
void TestNG::SetUpTestSuite()
{
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockPipelineContext::SetUp();
}

void TestNG::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

RefPtr<PaintWrapper> TestNG::FlushLayoutTask(const RefPtr<FrameNode>& frameNode)
{
    frameNode->SetActive();
    frameNode->isLayoutDirtyMarked_ = true;
    frameNode->CreateLayoutTask();
    auto paintProperty = frameNode->GetPaintProperty<PaintProperty>();
    auto wrapper = frameNode->CreatePaintWrapper();
    if (wrapper != nullptr) {
        wrapper->FlushRender();
    }
    paintProperty->CleanDirty();
    frameNode->SetActive(false);
    return wrapper;
}

void TestNG::CreateDone(const RefPtr<FrameNode>& frameNode)
{
    auto& elementsStack = ViewStackProcessor::GetInstance()->elementsStack_;
    while (elementsStack.size() > 1) {
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
    ViewStackProcessor::GetInstance()->Finish();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    frameNode->MarkModifyDone();
    FlushLayoutTask(frameNode);
}

uint64_t TestNG::GetActions(const RefPtr<AccessibilityProperty>& accessibilityProperty)
{
    std::unordered_set<AceAction> supportAceActions = accessibilityProperty->GetSupportAction();
    uint64_t actions = 0;
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    return actions;
}

TouchEventInfo TestNG::CreateTouchEventInfo(TouchType touchType, Offset location)
{
    TouchLocationInfo touchLocationInfo(1);
    touchLocationInfo.SetTouchType(touchType);
    touchLocationInfo.SetLocalLocation(location);
    TouchEventInfo touchEventInfo("touch");
    touchEventInfo.AddTouchLocationInfo(std::move(touchLocationInfo));
    return touchEventInfo;
}
} // namespace OHOS::Ace::NG