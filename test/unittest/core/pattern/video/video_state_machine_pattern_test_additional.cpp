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

#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_image_analyzer_manager.h"
#include "video_state_machine_pattern_test_common.h"

#include "base/log/dump_log.h"
#include "core/common/container.h"
#include "core/components_ng/pattern/video/video_state_machine_full_screen_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {
constexpr int32_t RET_SUCCESS_VAL = 10;
constexpr int32_t RET_FAILED_VAL = 11;
constexpr int32_t ERROR_CODE_INVALID_VAL = 401;
constexpr int32_t RESET_DURATION_MS = 65000;
const std::string VIDEO_SRC_TEST = "common/video.mp4";
const std::string VIDEO_POSTER_TEST = "common/img2.png";

struct FullScreenUpdateStateContext {
    RefPtr<VideoStateMachinePattern> pattern;
    RefPtr<VideoStateMachineFullScreenPattern> fullScreenPattern;
    RefPtr<VideoLayoutProperty> fullScreenLayout;
};

void PrepareFullScreenUpdateState(const RefPtr<FrameNode>& frameNode, FullScreenUpdateStateContext& context)
{
    ASSERT_TRUE(frameNode);
    context.pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(context.pattern);
    auto videoLayout = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_TRUE(videoLayout);
    auto videoSrcInfo = videoLayout->GetVideoSourceValue(VideoSourceInfo());
    videoSrcInfo.src_ = VIDEO_SRC_TEST;
    videoLayout->UpdateVideoSource(videoSrcInfo);
    videoLayout->UpdatePosterImageInfo(ImageSourceInfo(VIDEO_POSTER_TEST));

    context.pattern->FullScreen();
    auto fullScreenNode = context.pattern->GetFullScreenNode();
    ASSERT_TRUE(fullScreenNode);
    context.fullScreenPattern = AceType::DynamicCast<VideoStateMachineFullScreenPattern>(fullScreenNode->GetPattern());
    ASSERT_TRUE(context.fullScreenPattern);
    context.fullScreenLayout = fullScreenNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_TRUE(context.fullScreenLayout);

    context.pattern->UpdateLoop(true);
    context.pattern->UpdateMuted(true);
    context.pattern->UpdateProgressRate(1.5);
    context.pattern->SetShortcutKeyEnabled(true);
    context.pattern->SetCurrentVolume(0.35f);
    auto imageAnalyzerManager =
        std::make_shared<MockImageAnalyzerManager>(frameNode, ImageAnalyzerHolder::VIDEO_CUSTOM);
    imageAnalyzerManager->SetOverlayCreated(true);
    context.pattern->imageAnalyzerManager_ = imageAnalyzerManager;
    videoLayout->UpdateObjectFit(ImageFit::CONTAIN);
    videoLayout->UpdateControls(false);

    context.fullScreenPattern->UpdateLoop(false);
    context.fullScreenPattern->UpdateMuted(false);
    context.fullScreenPattern->UpdateAutoPlay(true);
    context.fullScreenPattern->UpdateProgressRate(0.5);
    context.fullScreenPattern->UpdateAnalyzerState(false);
    context.fullScreenPattern->SetShortcutKeyEnabled(false);
    context.fullScreenPattern->SetCurrentVolume(0.9f);
    context.fullScreenLayout->UpdateObjectFit(ImageFit::COVER);
    videoSrcInfo.src_ = "";
    context.fullScreenLayout->UpdateVideoSource(videoSrcInfo);
    context.fullScreenLayout->UpdatePosterImageInfo(ImageSourceInfo(""));
    context.fullScreenLayout->UpdateControls(true);
}

RefPtr<TestCallbackMediaPlayer> CreateAndRegisterCallbackPlayer(const RefPtr<VideoStateMachinePattern>& pattern)
{
    auto player = AceType::MakeRefPtr<TestCallbackMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    pattern->mediaPlayer_ = player;
    pattern->RegisterMediaPlayerEvent(
        AceType::WeakClaim(AceType::RawPtr(pattern)), player, VIDEO_SRC, pattern->instanceId_);
    return player;
}

void ConfigureResetSuccessChainMocks(
    const RefPtr<VideoStateMachinePattern>& pattern, RefPtr<TestCallbackMediaPlayer>& player)
{
    player = AceType::MakeRefPtr<TestCallbackMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    EXPECT_CALL(*player, SetSource(VIDEO_SRC, "", "")).WillOnce(Return(true));
    EXPECT_CALL(*player, SetSurface()).WillOnce(Return(0));
    EXPECT_CALL(*player, SetRenderFirstFrame(false)).WillOnce(Return(0));
    EXPECT_CALL(*player, PrepareAsync()).WillOnce(Return(0));
    EXPECT_CALL(*player, GetDuration(_)).WillRepeatedly(DoAll(SetArgReferee<0>(RESET_DURATION_MS), Return(0)));
    EXPECT_CALL(*player, SetLooping(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*player, SetPlaybackRate(_, _, _)).WillRepeatedly(Return(0));
    pattern->mediaPlayer_ = player;
    auto surface = AceType::DynamicCast<MockRenderSurface>(pattern->renderSurface_);
    ASSERT_TRUE(surface);
    EXPECT_CALL(*surface, SetRenderContext(_)).Times(1);
    EXPECT_CALL(*surface, IsSurfaceValid()).WillRepeatedly(Return(true));
}

void ClearControllerTasks(const RefPtr<VideoStateManager>& stateManager, ScopedRecordingTaskExecutor& taskExecutor)
{
    while (!stateManager->serialBgTaskQueue_.empty()) {
        stateManager->serialBgTaskQueue_.pop();
    }
    taskExecutor.Get()->tasks_.clear();
    stateManager->isDrainingSerialBgQueue_ = false;
    stateManager->ClearPendingCommand();
}
} // namespace

/**
 * @tc.name: FullScreen_SharedStateAndExit001
 * @tc.desc: Test fullscreen shares playback resources and restores them on exit.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, FullScreen_SharedStateAndExit001, TestSize.Level1)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    auto mediaPlayer = pattern->mediaPlayer_;
    auto renderSurface = pattern->renderSurface_;
    auto renderContext = pattern->renderContextForMediaPlayer_;
    auto stateManager = pattern->stateManager_;
    ASSERT_TRUE(mediaPlayer);
    ASSERT_TRUE(renderSurface);
    ASSERT_TRUE(renderContext);
    ASSERT_TRUE(stateManager);

    stateManager->state_ = VideoPlaybackState::PREPARED;
    stateManager->ClearPendingCommand();
    stateManager->SetPendingCommand(VideoPlaybackCommand::PLAY, nullptr);
    pattern->FullScreen();

    auto fullScreenNode = pattern->GetFullScreenNode();
    ASSERT_TRUE(fullScreenNode);
    auto fullScreenPattern = AceType::DynamicCast<VideoStateMachineFullScreenPattern>(fullScreenNode->GetPattern());
    ASSERT_TRUE(fullScreenPattern);
    EXPECT_EQ(fullScreenPattern->stateManager_.GetRawPtr(), stateManager.GetRawPtr());
    EXPECT_EQ(stateManager->GetCurrentPattern().GetRawPtr(), fullScreenPattern.GetRawPtr());
    EXPECT_EQ(fullScreenPattern->mediaPlayer_.GetRawPtr(), mediaPlayer.GetRawPtr());
    EXPECT_EQ(fullScreenPattern->renderSurface_.GetRawPtr(), renderSurface.GetRawPtr());
    EXPECT_EQ(fullScreenPattern->renderContextForMediaPlayer_.GetRawPtr(), renderContext.GetRawPtr());
    EXPECT_FALSE(pattern->mediaPlayer_);
    EXPECT_FALSE(pattern->renderSurface_);
    EXPECT_FALSE(pattern->renderContextForMediaPlayer_);
    EXPECT_EQ(pattern->GetTargetVideoPattern().GetRawPtr(), fullScreenPattern.GetRawPtr());
    EXPECT_EQ(fullScreenPattern->GetTargetVideoPattern().GetRawPtr(), fullScreenPattern.GetRawPtr());

    EXPECT_TRUE(fullScreenPattern->ExitFullScreen());
    EXPECT_EQ(stateManager->GetCurrentPattern().GetRawPtr(), pattern.GetRawPtr());
    EXPECT_EQ(pattern->mediaPlayer_.GetRawPtr(), mediaPlayer.GetRawPtr());
    EXPECT_EQ(pattern->renderSurface_.GetRawPtr(), renderSurface.GetRawPtr());
    EXPECT_EQ(pattern->renderContextForMediaPlayer_.GetRawPtr(), renderContext.GetRawPtr());
    EXPECT_FALSE(fullScreenPattern->mediaPlayer_);
    EXPECT_FALSE(fullScreenPattern->renderSurface_);
    EXPECT_FALSE(fullScreenPattern->renderContextForMediaPlayer_);
    EXPECT_EQ(stateManager->GetPendingCommand(), VideoPlaybackCommand::PLAY);
    EXPECT_EQ(pattern->GetTargetVideoPattern().GetRawPtr(), pattern.GetRawPtr());
    EXPECT_FALSE(pattern->IsFullScreen());
}

/**
 * @tc.name: FullScreen_UpdatePlaybackState001
 * @tc.desc: Test fullscreen UpdateState synchronizes playback properties.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, FullScreen_UpdatePlaybackState001, TestSize.Level1)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    FullScreenUpdateStateContext context;
    PrepareFullScreenUpdateState(frameNode, context);
    ASSERT_TRUE(context.fullScreenPattern);
    ASSERT_TRUE(context.fullScreenLayout);
    auto& pattern = context.pattern;
    auto& fullScreenPattern = context.fullScreenPattern;

    fullScreenPattern->UpdateState();
    EXPECT_TRUE(fullScreenPattern->GetLoop());
    EXPECT_TRUE(fullScreenPattern->GetMuted());
    EXPECT_FALSE(fullScreenPattern->GetAutoPlay());
    EXPECT_DOUBLE_EQ(fullScreenPattern->GetProgressRate(), 1.5);
    EXPECT_EQ(fullScreenPattern->isAnalyzerCreated_, pattern->GetAnalyzerState());
    EXPECT_TRUE(fullScreenPattern->GetShortcutKeyEnabled());
    EXPECT_FLOAT_EQ(fullScreenPattern->GetCurrentVolume(), 0.35f);
    EXPECT_TRUE(fullScreenPattern->ExitFullScreen());
}

/**
 * @tc.name: FullScreen_UpdateLayoutState001
 * @tc.desc: Test fullscreen UpdateState synchronizes layout properties.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, FullScreen_UpdateLayoutState001, TestSize.Level1)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    FullScreenUpdateStateContext context;
    PrepareFullScreenUpdateState(frameNode, context);
    ASSERT_TRUE(context.fullScreenPattern);
    ASSERT_TRUE(context.fullScreenLayout);
    auto& fullScreenPattern = context.fullScreenPattern;
    auto& fullScreenLayout = context.fullScreenLayout;

    fullScreenPattern->UpdateState();
    EXPECT_EQ(fullScreenLayout->GetObjectFit().value(), ImageFit::CONTAIN);
    EXPECT_EQ(fullScreenLayout->GetVideoSource().value().src_, VIDEO_SRC_TEST);
    EXPECT_EQ(fullScreenLayout->GetPosterImageInfo().value(), ImageSourceInfo(VIDEO_POSTER_TEST));
    EXPECT_FALSE(fullScreenLayout->GetControls().value());
    EXPECT_TRUE(fullScreenPattern->ExitFullScreen());
}

/**
 * @tc.name: OnInjectionEvent_Play001
 * @tc.desc: Test OnInjectionEvent routes the play command.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, OnInjectionEvent_Play001, TestSize.Level1)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    pattern->stateManager_->state_ = VideoPlaybackState::PREPARED;
    pattern->stateManager_->ClearPendingCommand();

    auto ret = pattern->OnInjectionEvent(R"({"cmd":"setVideoPlayerStatus","value":"play"})");
    EXPECT_EQ(ret, RET_SUCCESS_VAL);
    EXPECT_EQ(pattern->currentInjectedStatusCmd_, "play");
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::PLAY);
}

/**
 * @tc.name: OnInjectionEvent_Pause001
 * @tc.desc: Test OnInjectionEvent routes the pause command.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, OnInjectionEvent_Pause001, TestSize.Level1)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    pattern->stateManager_->state_ = VideoPlaybackState::PLAYING;
    pattern->stateManager_->ClearPendingCommand();

    auto ret = pattern->OnInjectionEvent(R"({"cmd":"setVideoPlayerStatus","value":"paused"})");
    EXPECT_EQ(ret, RET_SUCCESS_VAL);
    EXPECT_EQ(pattern->currentInjectedStatusCmd_, "pause");
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::PAUSE);
}

/**
 * @tc.name: OnInjectionEvent_PlaybackSpeed001
 * @tc.desc: Test OnInjectionEvent handles unchanged and changed playback speeds.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, OnInjectionEvent_PlaybackSpeed001, TestSize.Level1)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    pattern->UpdateProgressRate(1.0);
    pattern->SetIsProgressInjectCmd(false);

    auto unchangedRet = pattern->OnInjectionEvent(R"({"cmd":"setVideoPlaybackSpeed","value":1.0})");
    EXPECT_EQ(unchangedRet, RET_FAILED_VAL);
    EXPECT_DOUBLE_EQ(pattern->GetLastSetProgressRate(), 1.0);
    EXPECT_FALSE(pattern->GetsIsProgressInjectCmd());

    auto changedRet = pattern->OnInjectionEvent(R"({"cmd":"setVideoPlaybackSpeed","value":2.0})");
    EXPECT_EQ(changedRet, RET_SUCCESS_VAL);
    EXPECT_DOUBLE_EQ(pattern->GetProgressRate(), 2.0);
    EXPECT_TRUE(pattern->GetsIsProgressInjectCmd());
}

/**
 * @tc.name: MediaPlayerCallback_PositionAndSeekDone001
 * @tc.desc: Test position and seek-done callbacks update progress and seeking state.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, MediaPlayerCallback_PositionAndSeekDone001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = CreateAndRegisterCallbackPlayer(pattern);
    ASSERT_TRUE(player->positionUpdatedEvent_);
    ASSERT_TRUE(player->seekDoneEvent_);

    auto eventHub = pattern->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(eventHub);
    int32_t updateCount = 0;
    eventHub->SetOnUpdate([&updateCount](const std::string&) { updateCount++; });
    pattern->stateManager_->state_ = VideoPlaybackState::PLAYING;
    pattern->duration_ = 100;
    pattern->currentPos_ = 0;
    pattern->SetIsSeeking(false);

    player->positionUpdatedEvent_(50);
    EXPECT_EQ(pattern->currentPos_, 50u);
    EXPECT_EQ(updateCount, 1);

    pattern->SetIsSeeking(true);
    player->seekDoneEvent_(25);
    EXPECT_FALSE(pattern->GetIsSeeking());
    EXPECT_EQ(pattern->currentPos_, 25u);
    EXPECT_EQ(updateCount, 2);
}

/**
 * @tc.name: MediaPlayerCallback_StateChangedSequence001
 * @tc.desc: Test state-changed callbacks drive start, pause, and stop transitions.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, MediaPlayerCallback_StateChangedSequence001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = CreateAndRegisterCallbackPlayer(pattern);
    ASSERT_TRUE(player->stateChangedEvent_);

    auto eventHub = pattern->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(eventHub);
    int32_t startCount = 0;
    int32_t pauseCount = 0;
    int32_t stopCount = 0;
    eventHub->SetOnStart([&startCount](const std::string&) { startCount++; });
    eventHub->SetOnPause([&pauseCount](const std::string&) { pauseCount++; });
    eventHub->SetOnStop([&stopCount](const std::string&) { stopCount++; });

    pattern->stateManager_->state_ = VideoPlaybackState::PREPARED;
    pattern->stateManager_->SetPendingCommand(VideoPlaybackCommand::PLAY, nullptr);
    player->stateChangedEvent_(PlaybackStatus::STARTED);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::PLAYING);
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::NONE);
    EXPECT_EQ(startCount, 1);

    pattern->stateManager_->SetPendingCommand(VideoPlaybackCommand::PAUSE, nullptr);
    player->stateChangedEvent_(PlaybackStatus::PAUSED);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::PAUSED);
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::NONE);
    EXPECT_EQ(pauseCount, 1);

    pattern->currentPos_ = 20;
    pattern->stateManager_->SetPendingCommand(VideoPlaybackCommand::STOP, nullptr);
    player->stateChangedEvent_(PlaybackStatus::STOPPED);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::STOPPED);
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::NONE);
    EXPECT_EQ(pattern->currentPos_, 0u);
    EXPECT_EQ(pattern->currentPlaybackStatus_, PlaybackStatus::STOPPED);
    EXPECT_EQ(stopCount, 1);
}

/**
 * @tc.name: MediaPlayerCallback_ErrorEvents001
 * @tc.desc: Test media player error callbacks update error state and event data.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, MediaPlayerCallback_ErrorEvents001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = CreateAndRegisterCallbackPlayer(pattern);
    ASSERT_TRUE(player->errorEvent_);
    ASSERT_TRUE(player->videoErrorEvent_);

    auto eventHub = pattern->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(eventHub);
    int32_t errorCount = 0;
    eventHub->SetOnError([&errorCount](const std::string&) { errorCount++; });
    pattern->stateManager_->state_ = VideoPlaybackState::PREPARED;

    player->errorEvent_();
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::ERROR);
    EXPECT_EQ(pattern->lastErrorCode_, ERROR_CODE_VIDEO_SOURCE_INVALID);
    EXPECT_EQ(pattern->lastErrorMessage_, "Unknown error");
    EXPECT_EQ(errorCount, 1);

    player->videoErrorEvent_(ERROR_CODE_INVALID_VAL, "bad source");
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::ERROR);
    EXPECT_EQ(pattern->lastErrorCode_, ERROR_CODE_INVALID_VAL);
    EXPECT_EQ(pattern->lastErrorMessage_, "bad source");
    EXPECT_EQ(errorCount, 2);
}

/**
 * @tc.name: MediaPlayerCallback_ResolutionAndStartRender001
 * @tc.desc: Test resolution and first-frame callbacks update video and poster layout.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, MediaPlayerCallback_ResolutionAndStartRender001, TestSize.Level0)
{
    TestProperty testProperty;
    testProperty.posterUrl = VIDEO_POSTER_URL;
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = CreateAndRegisterCallbackPlayer(pattern);
    player->videoWidth_ = 640;
    player->videoHeight_ = 360;
    ASSERT_TRUE(player->resolutionChangeEvent_);
    ASSERT_TRUE(player->startRenderFrameEvent_);

    auto videoLayout = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_TRUE(videoLayout);
    videoLayout->ResetVideoSize();
    player->resolutionChangeEvent_();
    ASSERT_TRUE(videoLayout->GetVideoSize().has_value());
    EXPECT_EQ(videoLayout->GetVideoSize().value(), SizeF(640.0f, 360.0f));

    auto video = AceType::DynamicCast<VideoNode>(frameNode);
    ASSERT_TRUE(video);
    auto image = AceType::DynamicCast<FrameNode>(video->GetPreviewImage());
    ASSERT_TRUE(image);
    auto posterLayout = image->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_TRUE(posterLayout);
    posterLayout->UpdateVisibility(VisibleType::VISIBLE);
    videoLayout->ResetVideoSize();
    player->videoWidth_ = 1280;
    player->videoHeight_ = 720;
    pattern->stateManager_->ClearPendingCommand();
    pattern->isInitialState_ = true;

    player->startRenderFrameEvent_();
    EXPECT_FALSE(pattern->isInitialState_);
    EXPECT_EQ(posterLayout->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::INVISIBLE);
    ASSERT_TRUE(videoLayout->GetVideoSize().has_value());
    EXPECT_EQ(videoLayout->GetVideoSize().value(), SizeF(1280.0f, 720.0f));
}

/**
 * @tc.name: PrepareSurface_Branches001
 * @tc.desc: Test PrepareSurface handles missing, invalid, and valid surface branches.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, PrepareSurface_Branches001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    pattern->mediaPlayer_.Reset();
    pattern->PrepareSurface();

    ScopedExtSurfaceEnabled extSurface(false);
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, SetRenderSurface(_)).Times(1);
    EXPECT_CALL(*player, SetSurface()).WillOnce(Return(-1)).WillOnce(Return(0));
    pattern->mediaPlayer_ = player;
    auto surface = AceType::DynamicCast<MockRenderSurface>(pattern->renderSurface_);
    ASSERT_TRUE(surface);
    EXPECT_CALL(*surface, SetRenderContext(pattern->renderContextForMediaPlayer_)).Times(2);
    EXPECT_CALL(*surface, IsSurfaceValid()).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*surface, InitSurface()).Times(1);

    pattern->PrepareSurface();
    pattern->PrepareSurface();
}

/**
 * @tc.name: UpdateLooping_ExecuteBackgroundTask001
 * @tc.desc: Test UpdateLooping executes its queued background task.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, UpdateLooping_ExecuteBackgroundTask001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    EXPECT_CALL(*player, SetLooping(true)).WillOnce(Return(0));
    pattern->mediaPlayer_ = player;
    pattern->loop_ = true;

    pattern->UpdateLooping();
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));
    EXPECT_TRUE(pattern->loop_);
    EXPECT_TRUE(pattern->stateManager_->serialBgTaskQueue_.empty());
}

/**
 * @tc.name: UpdateMuted_ExecuteMutedAndNormalTasks001
 * @tc.desc: Test UpdateMuted executes muted and normal-volume tasks in order.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, UpdateMuted_ExecuteMutedAndNormalTasks001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    Sequence sequence;
    EXPECT_CALL(*player, SetMediaMuted(0, true)).InSequence(sequence);
    EXPECT_CALL(*player, SetVolume(0.0f, 0.0f)).InSequence(sequence);
    EXPECT_CALL(*player, SetMediaMuted(0, false)).InSequence(sequence);
    EXPECT_CALL(*player, SetVolume(0.5f, 0.5f)).InSequence(sequence);
    pattern->mediaPlayer_ = player;
    pattern->currentVolume_ = 0.5f;

    pattern->muted_ = true;
    pattern->UpdateMuted();
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));
    pattern->muted_ = false;
    pattern->UpdateMuted();
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));
    EXPECT_TRUE(pattern->stateManager_->serialBgTaskQueue_.empty());
}

/**
 * @tc.name: Stop_SuccessTaskBody001
 * @tc.desc: Test Stop executes successfully and completes on the stopped callback.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, Stop_SuccessTaskBody001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    EXPECT_CALL(*player, Stop()).WillOnce(Return(0));
    pattern->mediaPlayer_ = player;
    auto eventHub = pattern->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(eventHub);
    int32_t stopCount = 0;
    eventHub->SetOnStop([&stopCount](const std::string&) { stopCount++; });

    pattern->stateManager_->state_ = VideoPlaybackState::PLAYING;
    pattern->stateManager_->ClearPendingCommand();
    pattern->currentPos_ = 30;
    pattern->SetIsSeeking(true);
    pattern->Stop();
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::STOP);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));
    EXPECT_TRUE(pattern->GetIsSeeking());
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_FALSE(pattern->GetIsSeeking());
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::STOP);

    pattern->OnPlayerStatus(PlaybackStatus::STOPPED);
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::STOPPED);
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::NONE);
    EXPECT_EQ(pattern->currentPos_, 0u);
    EXPECT_EQ(stopCount, 1);
}

/**
 * @tc.name: Stop_FailureClearsPending001
 * @tc.desc: Test Stop failure clears pending state and reports the callback error.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, Stop_FailureClearsPending001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    EXPECT_CALL(*player, Stop()).WillOnce(Return(-1));
    pattern->mediaPlayer_ = player;
    pattern->stateManager_->state_ = VideoPlaybackState::PLAYING;
    pattern->stateManager_->ClearPendingCommand();
    pattern->SetIsSeeking(true);

    bool callbackCalled = false;
    bool callbackSuccess = true;
    std::string callbackReason;
    auto callback = [&callbackCalled, &callbackSuccess, &callbackReason](bool success, const std::string& reason) {
        callbackCalled = true;
        callbackSuccess = success;
        callbackReason = reason;
    };
    pattern->Stop(callback);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(callbackSuccess);
    EXPECT_EQ(callbackReason, "Media player Stop() failed");
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::NONE);
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::PLAYING);
    EXPECT_TRUE(pattern->GetIsSeeking());
}

/**
 * @tc.name: UpdateSpeed_SuccessResultWithoutInjection001
 * @tc.desc: Test UpdateSpeed records a successful rate without injection mode.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, UpdateSpeed_SuccessResultWithoutInjection001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    EXPECT_CALL(*player, SetPlaybackRate(2.0f, _, _)).WillOnce(Return(0));
    pattern->mediaPlayer_ = player;
    pattern->progressRate_ = 2.0;
    pattern->lastProgressRate_ = 0.0;
    pattern->lastSetProgressRate_ = 1.0;
    pattern->isProgressInjectCmd_ = false;

    pattern->UpdateSpeed();
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_DOUBLE_EQ(pattern->lastProgressRate_, 2.0);
    EXPECT_DOUBLE_EQ(pattern->lastSetProgressRate_, 2.0);
    EXPECT_FALSE(pattern->isProgressInjectCmd_);
}

/**
 * @tc.name: ResetMediaPlayerOnBg_SuccessChain001
 * @tc.desc: Test background reset chains through prepare and restores playback state.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, ResetMediaPlayerOnBg_SuccessChain001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    ScopedExtSurfaceEnabled extSurface(false);
    RefPtr<TestCallbackMediaPlayer> player;
    ConfigureResetSuccessChainMocks(pattern, player);
    pattern->videoSrcInfo_.src_ = VIDEO_SRC;
    pattern->showFirstFrame_ = false;
    pattern->stateManager_->state_ = VideoPlaybackState::PREPARED;
    pattern->stateManager_->ClearPendingCommand();

    auto eventHub = pattern->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(eventHub);
    int32_t preparedCount = 0;
    eventHub->SetOnPrepared([&preparedCount](const std::string&) { preparedCount++; });
    bool callbackCalled = false;
    bool callbackSuccess = false;
    auto callback = [&callbackCalled, &callbackSuccess](bool success, const std::string&) {
        callbackCalled = true;
        callbackSuccess = success;
    };
    pattern->ResetMediaPlayerOnBg(callback);
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::RESET);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));
    EXPECT_EQ(player->resetCount_, 1);
    EXPECT_EQ(player->createCount_, 0);
    ASSERT_TRUE(player->stateChangedEvent_);

    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::CREATED);
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::PREPARE);
    EXPECT_EQ(pattern->stateManager_->GetOriginalIntent(), VideoPlaybackCommand::RESET);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::BACKGROUND));

    player->stateChangedEvent_(PlaybackStatus::PREPARED);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_EQ(pattern->stateManager_->GetCurrentState(), VideoPlaybackState::PREPARED);
    EXPECT_EQ(pattern->stateManager_->GetPendingCommand(), VideoPlaybackCommand::NONE);
    EXPECT_EQ(pattern->stateManager_->GetOriginalIntent(), VideoPlaybackCommand::NONE);
    EXPECT_EQ(pattern->duration_, 65u);
    EXPECT_EQ(preparedCount, 1);
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(callbackSuccess);
}

/**
 * @tc.name: DumpInfo_TextAndJson001
 * @tc.desc: Test DumpInfo emits consistent text, render context, and JSON data.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, DumpInfo_TextAndJson001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    pattern->duration_ = 65;
    pattern->loop_ = true;
    pattern->muted_ = false;
    pattern->progressRate_ = 1.5;

    auto expected = pattern->GetDumpInfo();
    EXPECT_NE(expected.find("Video[" + std::to_string(pattern->hostId_) + "]"), std::string::npos);
    EXPECT_NE(expected.find("duration: 65"), std::string::npos);
    EXPECT_NE(expected.find("loop: 1"), std::string::npos);
    EXPECT_NE(expected.find("muted: 0"), std::string::npos);
    EXPECT_NE(expected.find("Speed: 1.500000"), std::string::npos);

    auto& dumpLog = DumpLog::GetInstance();
    dumpLog.description_.clear();
    ASSERT_TRUE(pattern->renderContextForMediaPlayer_);
    pattern->DumpInfo();
    ASSERT_EQ(dumpLog.description_.size(), 2u);
    EXPECT_EQ(dumpLog.description_[0], expected + "\n");
    auto renderContext = pattern->renderContextForMediaPlayer_;
    pattern->renderContextForMediaPlayer_.Reset();
    pattern->DumpInfo();
    ASSERT_EQ(dumpLog.description_.size(), 3u);
    EXPECT_EQ(dumpLog.description_[2], expected + "\n");
    pattern->renderContextForMediaPlayer_ = renderContext;

    auto json = JsonUtil::Create();
    pattern->DumpInfo(json);
    ASSERT_TRUE(json->Contains("VideoPattern"));
    EXPECT_EQ(json->GetString("VideoPattern"), expected);
}

/**
 * @tc.name: DumpSimplifyInfo_NestedJson001
 * @tc.desc: Test DumpSimplifyInfo nests video details under CanvasPattern.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, DumpSimplifyInfo_NestedJson001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    pattern->duration_ = 90;
    pattern->loop_ = false;
    pattern->muted_ = true;
    pattern->progressRate_ = 2.0;
    auto expected = pattern->GetDumpInfo();

    auto json = JsonUtil::CreateSharedPtrJson();
    pattern->DumpSimplifyInfo(json);
    ASSERT_TRUE(json->Contains("CanvasPattern"));
    auto videoJson = json->GetObject("CanvasPattern");
    ASSERT_TRUE(videoJson);
    EXPECT_EQ(videoJson->GetString("VideoPattern"), expected);
}

/**
 * @tc.name: BindControllerAsync_MediaCommandRoutes001
 * @tc.desc: Test async controller routes start, pause, stop, and reset commands.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, BindControllerAsync_MediaCommandRoutes001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    pattern->mediaPlayer_ = player;
    pattern->showFirstFrame_ = true;
    auto controller = pattern->videoControllerAsync_;
    ASSERT_TRUE(controller);
    ASSERT_TRUE(controller->IsBound());
    auto stateManager = pattern->stateManager_;

    stateManager->state_ = VideoPlaybackState::PREPARED;
    ClearControllerTasks(stateManager, taskExecutor);
    controller->Start(nullptr);
    EXPECT_EQ(stateManager->GetPendingCommand(), VideoPlaybackCommand::PLAY);
    ASSERT_EQ(stateManager->serialBgTaskQueue_.size(), 1u);
    EXPECT_EQ(stateManager->serialBgTaskQueue_.front().name, "ArkUIVideoPlay");

    stateManager->state_ = VideoPlaybackState::PLAYING;
    ClearControllerTasks(stateManager, taskExecutor);
    controller->Pause(nullptr);
    EXPECT_EQ(stateManager->GetPendingCommand(), VideoPlaybackCommand::PAUSE);
    ASSERT_EQ(stateManager->serialBgTaskQueue_.size(), 1u);
    EXPECT_EQ(stateManager->serialBgTaskQueue_.front().name, "ArkUIVideoPause");

    stateManager->state_ = VideoPlaybackState::PLAYING;
    ClearControllerTasks(stateManager, taskExecutor);
    controller->Stop(nullptr);
    EXPECT_EQ(stateManager->GetPendingCommand(), VideoPlaybackCommand::STOP);
    ASSERT_EQ(stateManager->serialBgTaskQueue_.size(), 1u);
    EXPECT_EQ(stateManager->serialBgTaskQueue_.front().name, "ArkUIVideoStop");

    stateManager->state_ = VideoPlaybackState::CREATED;
    ClearControllerTasks(stateManager, taskExecutor);
    controller->Reset(nullptr);
    EXPECT_EQ(stateManager->GetPendingCommand(), VideoPlaybackCommand::RESET);
    ASSERT_EQ(stateManager->serialBgTaskQueue_.size(), 1u);
    EXPECT_EQ(stateManager->serialBgTaskQueue_.front().name, "ArkUIVideoMediaPlayerReset");
}

/**
 * @tc.name: BindControllerAsync_SeekRoute001
 * @tc.desc: Test async controller routes seek to the media player.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, BindControllerAsync_SeekRoute001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    EXPECT_CALL(*player, Seek(5500, SeekMode::SEEK_CLOSEST)).WillOnce(Return(0));
    pattern->mediaPlayer_ = player;
    pattern->showFirstFrame_ = true;
    auto controller = pattern->videoControllerAsync_;
    ASSERT_TRUE(controller);
    ASSERT_TRUE(controller->IsBound());
    auto stateManager = pattern->stateManager_;

    stateManager->state_ = VideoPlaybackState::PREPARED;
    ClearControllerTasks(stateManager, taskExecutor);
    controller->SeekTo(5.5f, SeekMode::SEEK_CLOSEST);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_TRUE(pattern->GetIsSeeking());
    EXPECT_TRUE(controller->IsBound());
}

/**
 * @tc.name: BindControllerAsync_FullscreenRoute001
 * @tc.desc: Test async controller routes fullscreen entry and exit requests.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, BindControllerAsync_FullscreenRoute001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    ScopedRecordingTaskExecutor taskExecutor(MockPipelineContext::GetCurrent());
    auto player = AceType::MakeRefPtr<TestableMockMediaPlayer>();
    EXPECT_CALL(*player, IsMediaPlayerValid()).WillRepeatedly(Return(true));
    pattern->mediaPlayer_ = player;
    pattern->showFirstFrame_ = true;
    auto controller = pattern->videoControllerAsync_;
    ASSERT_TRUE(controller);
    ASSERT_TRUE(controller->IsBound());
    auto stateManager = pattern->stateManager_;

    stateManager->state_ = VideoPlaybackState::PREPARED;
    ClearControllerTasks(stateManager, taskExecutor);
    pattern->SetIsSeeking(true);
    controller->RequestFullscreen(true);
    ASSERT_TRUE(taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI));
    EXPECT_TRUE(pattern->IsFullScreen());
    controller->RequestFullscreen(false);
    while (taskExecutor.Get()->RunNext(TaskExecutor::TaskType::UI)) {
    }
    EXPECT_FALSE(pattern->IsFullScreen());
}

/**
 * @tc.name: SetVideoControllerAsync_ReplacesBinding001
 * @tc.desc: Test replacing an async controller updates old and new bindings.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, SetVideoControllerAsync_ReplacesBinding001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    auto original = pattern->videoControllerAsync_;
    ASSERT_TRUE(original);
    ASSERT_TRUE(original->IsBound());
    auto replacement = AceType::MakeRefPtr<VideoControllerAsync>();

    pattern->SetVideoControllerAsync(replacement);
    EXPECT_EQ(pattern->videoControllerAsync_.GetRawPtr(), replacement.GetRawPtr());
    EXPECT_FALSE(original->IsBound());
    EXPECT_TRUE(replacement->IsBound());

    pattern->SetVideoControllerAsync(replacement);
    EXPECT_TRUE(replacement->IsBound());
    pattern->SetVideoControllerAsync(original);
    EXPECT_TRUE(original->IsBound());
    EXPECT_FALSE(replacement->IsBound());
}

/**
 * @tc.name: OnDirtyLayoutWrapperSwap_Api12PixelRounding001
 * @tc.desc: Test API 12 layout swapping rounds media-player bounds to pixels.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, OnDirtyLayoutWrapperSwap_Api12PixelRounding001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_TRUE(videoLayoutProperty);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, videoLayoutProperty);
    layoutWrapper->skipMeasureContent_ = false;
    layoutWrapper->layoutAlgorithm_ =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(AceType::MakeRefPtr<LayoutAlgorithm>());
    layoutWrapper->layoutAlgorithm_->skipMeasure_ = false;
    auto renderContext = AceType::MakeRefPtr<MockRenderContext>();
    pattern->renderContextForMediaPlayer_ = renderContext;
    DirtySwapConfig config;
    config.skipMeasure = false;

    MockContainer::SetUp();
    auto container = Container::Current();
    auto oldVersion = container->GetApiTargetVersion();
    container->SetApiTargetVersion(12);
    videoLayoutProperty->UpdateObjectFit(ImageFit::FILL);
    videoLayoutProperty->UpdateVideoSize(SizeF(50.0f, 50.0f));
    geometryNode->SetContentSize(SizeF(100.4f, 80.4f));
    EXPECT_CALL(*renderContext, SetBounds(0.0f, 0.0f, 100.0f, 80.0f)).Times(1);
    EXPECT_FALSE(pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.Width(), 100.0f);
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.Height(), 80.0f);

    videoLayoutProperty->UpdateObjectFit(ImageFit::CONTAIN);
    videoLayoutProperty->UpdateVideoSize(SizeF(50.8f, 100.0f));
    geometryNode->SetContentSize(SizeF(52.0f, 100.0f));
    EXPECT_CALL(*renderContext, SetBounds(1.0f, 0.0f, 51.0f, 100.0f)).Times(1);
    EXPECT_FALSE(pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.GetX(), 1.0f);
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.Width(), 51.0f);

    videoLayoutProperty->UpdateVideoSize(SizeF(100.0f, 50.8f));
    geometryNode->SetContentSize(SizeF(100.0f, 52.0f));
    EXPECT_CALL(*renderContext, SetBounds(0.0f, 1.0f, 100.0f, 51.0f)).Times(1);
    EXPECT_FALSE(pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));
    container->SetApiTargetVersion(oldVersion);
    MockContainer::TearDown();
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.GetY(), 1.0f);
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.Height(), 51.0f);
}

/**
 * @tc.name: OnDirtyLayoutWrapperSwap_InvalidFitUsesCover001
 * @tc.desc: Test layout swapping uses cover bounds for an invalid image fit.
 * @tc.type: FUNC
 */
HWTEST_F(VideoStateMachinePatternTestNg, OnDirtyLayoutWrapperSwap_InvalidFitUsesCover001, TestSize.Level0)
{
    auto frameNode = CreateVideoNode(g_testProperty);
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<VideoStateMachinePattern>();
    ASSERT_TRUE(pattern);
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_TRUE(videoLayoutProperty);
    videoLayoutProperty->UpdateVideoSize(SizeF(100.0f, 200.0f));
    videoLayoutProperty->UpdateObjectFit(static_cast<ImageFit>(999));
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    geometryNode->SetContentSize(SizeF(400.0f, 200.0f));
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, videoLayoutProperty);
    layoutWrapper->skipMeasureContent_ = false;
    layoutWrapper->layoutAlgorithm_ =
        AceType::MakeRefPtr<LayoutAlgorithmWrapper>(AceType::MakeRefPtr<LayoutAlgorithm>());
    layoutWrapper->layoutAlgorithm_->skipMeasure_ = false;
    auto renderContext = AceType::MakeRefPtr<MockRenderContext>();
    pattern->renderContextForMediaPlayer_ = renderContext;
    DirtySwapConfig config;
    config.skipMeasure = false;

    EXPECT_CALL(*renderContext, SetBounds(0.0f, -300.0f, 400.0f, 800.0f)).Times(1);
    EXPECT_FALSE(pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.GetY(), -300.0f);
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.Width(), 400.0f);
    EXPECT_FLOAT_EQ(pattern->lastMediaPlayerBounds_.Height(), 800.0f);
}

} // namespace OHOS::Ace::NG
