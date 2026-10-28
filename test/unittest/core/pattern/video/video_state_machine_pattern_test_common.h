/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef VIDEO_STATE_MACHINE_PATTERN_TEST_COMMON_H
#define VIDEO_STATE_MACHINE_PATTERN_TEST_COMMON_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock-actions.h"
#include "gtest/gtest.h"

#define private public
#define protected public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_media_player.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_surface.h"

#include "base/geometry/ng/size_t.h"
#include "base/json/json_util.h"
#include "base/memory/ace_type.h"
#include "base/resource/internal_resource.h"
#include "core/common/ai/image_analyzer_mgr.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/video/video_theme.h"
#include "core/components_ng/pattern/video/video_utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/manager/full_screen/full_screen_manager.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/video/video_event_hub.h"
#include "core/components_ng/pattern/video/video_full_screen_node.h"
#include "core/components_ng/pattern/video/video_full_screen_pattern.h"
#include "core/components_ng/pattern/video/video_layout_algorithm.h"
#include "core/components_ng/pattern/video/video_layout_property.h"
#include "core/components_ng/pattern/video/video_model_ng.h"
#include "core/components_ng/pattern/video/video_model_static.h"
#include "core/components_ng/pattern/video/video_node.h"
#include "core/components_ng/pattern/video/video_state_machine_pattern.h"
#include "core/components_ng/pattern/video/video_state_manager.h"
#include "core/components_ng/pattern/video/video_styles.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/image/image_source_info.h"
#include "video_test_property_common.h"

namespace OHOS::Ace::NG {

class TestableMockMediaPlayer : public MockMediaPlayer {
    DECLARE_ACE_TYPE(TestableMockMediaPlayer, MockMediaPlayer);
public:
    int32_t videoWidth_ = DEFAULT_WIDTH;
    int32_t videoHeight_ = DEFAULT_HEIGHT;

    TestableMockMediaPlayer()
    {
        ON_CALL(*this, GetVideoWidth()).WillByDefault(testing::Invoke([this]() { return videoWidth_; }));
        ON_CALL(*this, GetVideoHeight()).WillByDefault(testing::Invoke([this]() { return videoHeight_; }));
    }

    MOCK_METHOD0(GetVideoWidth, int32_t());
    MOCK_METHOD0(GetVideoHeight, int32_t());
};

class TestCallbackMediaPlayer : public TestableMockMediaPlayer {
    DECLARE_ACE_TYPE(TestCallbackMediaPlayer, TestableMockMediaPlayer);
public:
    void CreateMediaPlayer() override
    {
        createCount_++;
    }

    void ResetMediaPlayer() override
    {
        resetCount_++;
    }

    void RegisterMediaPlayerEvent(PositionUpdatedEvent&& positionUpdatedEvent,
        StateChangedEvent&& stateChangedEvent, CommonEvent&& errorEvent, CommonEvent&& resolutionChangeEvent,
        CommonEvent&& startRenderFrameEvent, VideoErrorEvent&& videoErrorEvent = {},
        SeekDoneEvent&& seekDoneEvent = {}) override
    {
        positionUpdatedEvent_ = std::move(positionUpdatedEvent);
        stateChangedEvent_ = std::move(stateChangedEvent);
        errorEvent_ = std::move(errorEvent);
        resolutionChangeEvent_ = std::move(resolutionChangeEvent);
        startRenderFrameEvent_ = std::move(startRenderFrameEvent);
        videoErrorEvent_ = std::move(videoErrorEvent);
        seekDoneEvent_ = std::move(seekDoneEvent);
    }

    int32_t createCount_ = 0;
    int32_t resetCount_ = 0;
    PositionUpdatedEvent positionUpdatedEvent_;
    StateChangedEvent stateChangedEvent_;
    CommonEvent errorEvent_;
    CommonEvent resolutionChangeEvent_;
    CommonEvent startRenderFrameEvent_;
    VideoErrorEvent videoErrorEvent_;
    SeekDoneEvent seekDoneEvent_;
};

class RecordingTaskExecutor : public MockTaskExecutor {
    DECLARE_ACE_TYPE(RecordingTaskExecutor, MockTaskExecutor);
public:
    struct RecordedTask {
        TaskExecutor::Task task;
        TaskExecutor::TaskType type;
        std::string name;
    };

    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW,
        VsyncBarrierOption barrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (!task) {
            return false;
        }
        tasks_.push_back({ std::move(task), type, name });
        return true;
    }

    bool OnPostTaskWithoutTraceId(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW) const override
    {
        return OnPostTask(std::move(task), type, delayTime, name, priorityType);
    }

    bool RunNext(TaskType type)
    {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [type](const RecordedTask& recorded) {
            return recorded.type == type;
        });
        if (it == tasks_.end()) {
            return false;
        }
        auto task = std::move(it->task);
        tasks_.erase(it);
        task();
        return true;
    }

    mutable std::vector<RecordedTask> tasks_;
};

class ScopedRecordingTaskExecutor {
public:
    explicit ScopedRecordingTaskExecutor(const RefPtr<PipelineContext>& context)
        : context_(context), originalExecutor_(context->GetTaskExecutor()),
          executor_(AceType::MakeRefPtr<RecordingTaskExecutor>())
    {
        context_->taskExecutor_ = executor_;
    }

    ~ScopedRecordingTaskExecutor()
    {
        context_->taskExecutor_ = originalExecutor_;
    }

    const RefPtr<RecordingTaskExecutor>& Get() const
    {
        return executor_;
    }

private:
    RefPtr<PipelineContext> context_;
    RefPtr<TaskExecutor> originalExecutor_;
    RefPtr<RecordingTaskExecutor> executor_;
};

class ScopedExtSurfaceEnabled {
public:
    explicit ScopedExtSurfaceEnabled(bool enabled) : original_(SystemProperties::GetExtSurfaceEnabled())
    {
        SystemProperties::SetExtSurfaceEnabled(enabled);
    }

    ~ScopedExtSurfaceEnabled()
    {
        SystemProperties::SetExtSurfaceEnabled(original_);
    }

private:
    bool original_;
};

namespace {
constexpr double VIDEO_PROGRESS_RATE = 1.0;
constexpr bool MUTED_VALUE = false;
constexpr bool AUTO_PLAY = false;
constexpr bool CONTROL_VALUE = true;
constexpr bool LOOP_VALUE = false;
constexpr bool SHOW_FIRST_FRAME = false;

const ImageFit VIDEO_IMAGE_FIT = ImageFit::COVER;
const std::string VIDEO_SRC = "common/video.mp4";
const std::string VIDEO_POSTER_URL = "common/img2.png";

constexpr float MAX_WIDTH = 400.0f;
constexpr float MAX_HEIGHT = 400.0f;
constexpr float VIDEO_WIDTH = 300.0f;
constexpr float VIDEO_HEIGHT = 300.0f;

const SizeF MAX_SIZE(MAX_WIDTH, MAX_HEIGHT);
const SizeF VIDEO_SIZE(VIDEO_WIDTH, VIDEO_HEIGHT);

TestProperty g_testProperty;
} // namespace

class VideoStateMachinePatternTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

protected:
    static RefPtr<FrameNode> CreateVideoNode(TestProperty& testProperty);
};

} // namespace OHOS::Ace::NG

#endif // VIDEO_STATE_MACHINE_PATTERN_TEST_COMMON_H
