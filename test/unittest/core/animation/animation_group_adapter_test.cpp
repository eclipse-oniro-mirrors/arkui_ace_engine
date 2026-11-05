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

#include <string>

#include "gtest/gtest.h"

#include "core/components_ng/render/animation_utils.h"

#include "core/components_ng/animation/animation_group_adapter.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t DEFAULT_INSTANCE_ID = 1;
constexpr int32_t INVALID_INSTANCE_ID = -1;
} // namespace
} // namespace OHOS::Ace::NG

namespace OHOS::Ace {
namespace AnimationGroupTestHelper {
void SetInteractiveAnimationStatus(
    const std::shared_ptr<AnimationUtils::InteractiveAnimation>& animation, InteractiveAnimationStatus status);
void ClearInteractiveAnimationStatus(
    const std::shared_ptr<AnimationUtils::InteractiveAnimation>& animation);
void ClearAllInteractiveAnimationStatus();
} // namespace AnimationGroupTestHelper
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {

class AnimationGroupAdapterTest : public testing::Test {
public:
    void SetUp() override
    {
        AnimationGroupTestHelper::ClearAllInteractiveAnimationStatus();
    }
    void TearDown() override
    {
        auto& adapter = AnimationGroupAdapter::GetInstance();
        adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_add");
        adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr");
        adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_fin");
        adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_state");
        AnimationGroupTestHelper::ClearAllInteractiveAnimationStatus();
    }
};

AnimationGroupCreateResult MakeCreateResult(bool withAnimation)
{
    AnimationGroupCreateResult result;
    if (withAnimation) {
        result.animation = AnimationUtils::CreateInteractiveAnimation(nullptr, nullptr);
        result.errorCode = ERROR_CODE_NO_ERROR;
    } else {
        result.animation = nullptr;
        result.errorCode = ERROR_CODE_PARAM_INVALID;
    }
    return result;
}

/**
 * @tc.name: AnimationGroupAdapterAdd001
 * @tc.desc: Test AddAnimationGroup param validations and normal add.
 * @tc.type: FUNC
 */
HWTEST_F(AnimationGroupAdapterTest, AnimationGroupAdapterAdd001, TestSize.Level1)
{
    auto& adapter = AnimationGroupAdapter::GetInstance();
    // null createFn
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_null_fn", nullptr),
        ERROR_CODE_PARAM_INVALID);
    // negative instanceId
    auto createFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(true);
    };
    EXPECT_EQ(adapter.AddAnimationGroup(INVALID_INSTANCE_ID, "key_neg", createFn), ERROR_CODE_PARAM_INVALID);
    // createFn returns null animation, error code propagated
    auto failFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(false);
    };
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_fail", failFn), ERROR_CODE_PARAM_INVALID);
    // success
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_add", createFn), ERROR_CODE_NO_ERROR);
    // re-add with same key: old animation extracted and finished
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_add", createFn), ERROR_CODE_NO_ERROR);
    adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_add");
}

/**
 * @tc.name: AnimationGroupAdapterRemove001
 * @tc.desc: Test RemoveAnimationGroup branches.
 * @tc.type: FUNC
 */
HWTEST_F(AnimationGroupAdapterTest, AnimationGroupAdapterRemove001, TestSize.Level1)
{
    auto& adapter = AnimationGroupAdapter::GetInstance();
    EXPECT_EQ(adapter.RemoveAnimationGroup(INVALID_INSTANCE_ID, "any"), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "not_exist"),
        ERROR_CODE_ANIMATION_GROUP_NOT_FOUND);

    auto createFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(true);
    };
    auto emptyFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(false);
    };
    // add then re-add with failing createFn: old entry extracted (oldAnim branch), new create fails
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_empty", createFn), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_empty", emptyFn), ERROR_CODE_PARAM_INVALID);
    // old entry was consumed by the failed re-add
    EXPECT_EQ(adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_empty"),
        ERROR_CODE_ANIMATION_GROUP_NOT_FOUND);
}

/**
 * @tc.name: AnimationGroupAdapterPauseResume001
 * @tc.desc: Test Pause/Resume state machine branches.
 * @tc.type: FUNC
 */
HWTEST_F(AnimationGroupAdapterTest, AnimationGroupAdapterPauseResume001, TestSize.Level1)
{
    auto& adapter = AnimationGroupAdapter::GetInstance();
    EXPECT_EQ(adapter.PauseAnimationGroup(INVALID_INSTANCE_ID, "any"), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.ResumeAnimationGroup(INVALID_INSTANCE_ID, "any"), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.PauseAnimationGroup(DEFAULT_INSTANCE_ID, "missing"), ERROR_CODE_ANIMATION_GROUP_NOT_FOUND);
    EXPECT_EQ(adapter.ResumeAnimationGroup(DEFAULT_INSTANCE_ID, "missing"), ERROR_CODE_ANIMATION_GROUP_NOT_FOUND);

    auto createFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(true);
    };
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr", createFn), ERROR_CODE_NO_ERROR);
    auto entry = adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_pr");
    ASSERT_TRUE(entry.has_value());
    auto animation = entry->interactiveAnimation;

    // status default RUNNING: pause allowed
    EXPECT_EQ(adapter.PauseAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr"), ERROR_CODE_NO_ERROR);
    // now PAUSED: pause again -> invalid state
    EXPECT_EQ(adapter.PauseAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr"),
        ERROR_CODE_ANIMATION_GROUP_INVALID_STATE);
    // resume from PAUSED
    EXPECT_EQ(adapter.ResumeAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr"), ERROR_CODE_NO_ERROR);
    // running status: resume -> invalid
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::RUNNING);
    EXPECT_EQ(adapter.ResumeAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr"),
        ERROR_CODE_ANIMATION_GROUP_INVALID_STATE);

    // ACTIVE state: pause logs warning but allowed
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::ACTIVE);
    EXPECT_EQ(adapter.PauseAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr"), ERROR_CODE_NO_ERROR);

    // INACTIVE: resume invalid
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::INACTIVE);
    EXPECT_EQ(adapter.ResumeAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr"),
        ERROR_CODE_ANIMATION_GROUP_INVALID_STATE);

    AnimationGroupTestHelper::ClearInteractiveAnimationStatus(animation);
    adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_pr");
}

/**
 * @tc.name: AnimationGroupAdapterFinish001
 * @tc.desc: Test FinishAnimationGroup branches.
 * @tc.type: FUNC
 */
HWTEST_F(AnimationGroupAdapterTest, AnimationGroupAdapterFinish001, TestSize.Level1)
{
    auto& adapter = AnimationGroupAdapter::GetInstance();
    EXPECT_EQ(adapter.FinishAnimationGroup(INVALID_INSTANCE_ID, "any",
        InteractiveAnimationFinishPosition::TO_START), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.FinishAnimationGroup(DEFAULT_INSTANCE_ID, "missing",
        InteractiveAnimationFinishPosition::TO_END), ERROR_CODE_ANIMATION_GROUP_NOT_FOUND);

    auto createFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(true);
    };
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_fin", createFn), ERROR_CODE_NO_ERROR);
    auto entry = adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_fin");
    ASSERT_TRUE(entry.has_value());
    auto animation = entry->interactiveAnimation;

    // INACTIVE: invalid state
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::INACTIVE);
    EXPECT_EQ(adapter.FinishAnimationGroup(DEFAULT_INSTANCE_ID, "key_fin",
        InteractiveAnimationFinishPosition::TO_START), ERROR_CODE_ANIMATION_GROUP_INVALID_STATE);

    // ACTIVE: warning branch, then success
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::ACTIVE);
    EXPECT_EQ(adapter.FinishAnimationGroup(DEFAULT_INSTANCE_ID, "key_fin",
        InteractiveAnimationFinishPosition::TO_START), ERROR_CODE_NO_ERROR);

    // entry with null animation: skip state checks
    AnimationGroupTestHelper::ClearInteractiveAnimationStatus(animation);
    EXPECT_EQ(adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_fin"), ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: AnimationGroupAdapterGetState001
 * @tc.desc: Test GetAnimationGroupState and HasAnimationGroup branches.
 * @tc.type: FUNC
 */
HWTEST_F(AnimationGroupAdapterTest, AnimationGroupAdapterGetState001, TestSize.Level1)
{
    auto& adapter = AnimationGroupAdapter::GetInstance();
    ArkUI_Int32 state = -1;
    bool exists = true;
    EXPECT_EQ(adapter.GetAnimationGroupState(DEFAULT_INSTANCE_ID, "any", nullptr), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.GetAnimationGroupState(INVALID_INSTANCE_ID, "any", &state), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.GetAnimationGroupState(DEFAULT_INSTANCE_ID, "missing", &state),
        ERROR_CODE_ANIMATION_GROUP_NOT_FOUND);
    EXPECT_EQ(adapter.HasAnimationGroup(DEFAULT_INSTANCE_ID, "any", nullptr), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.HasAnimationGroup(INVALID_INSTANCE_ID, "any", &exists), ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(adapter.HasAnimationGroup(DEFAULT_INSTANCE_ID, "missing", &exists), ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);

    auto createFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(true);
    };
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_state", createFn), ERROR_CODE_NO_ERROR);
    auto entry = adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_state");
    ASSERT_TRUE(entry.has_value());
    auto animation = entry->interactiveAnimation;

    // RUNNING status
    EXPECT_EQ(adapter.GetAnimationGroupState(DEFAULT_INSTANCE_ID, "key_state", &state), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state, static_cast<ArkUI_Int32>(AnimationGroupState::RUNNING));

    // PAUSED status
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::PAUSED);
    EXPECT_EQ(adapter.GetAnimationGroupState(DEFAULT_INSTANCE_ID, "key_state", &state), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state, static_cast<ArkUI_Int32>(AnimationGroupState::PAUSED));

    // INACTIVE status
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::INACTIVE);
    EXPECT_EQ(adapter.GetAnimationGroupState(DEFAULT_INSTANCE_ID, "key_state", &state), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state, static_cast<ArkUI_Int32>(AnimationGroupState::INACTIVE));

    // ACTIVE status maps to RUNNING
    AnimationGroupTestHelper::SetInteractiveAnimationStatus(animation, InteractiveAnimationStatus::ACTIVE);
    EXPECT_EQ(adapter.GetAnimationGroupState(DEFAULT_INSTANCE_ID, "key_state", &state), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state, static_cast<ArkUI_Int32>(AnimationGroupState::RUNNING));

    // cleared status falls back to RUNNING default in mock
    AnimationGroupTestHelper::ClearInteractiveAnimationStatus(animation);
    EXPECT_EQ(adapter.GetAnimationGroupState(DEFAULT_INSTANCE_ID, "key_state", &state), ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state, static_cast<ArkUI_Int32>(AnimationGroupState::RUNNING));

    EXPECT_EQ(adapter.HasAnimationGroup(DEFAULT_INSTANCE_ID, "key_state", &exists), ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(exists);

    adapter.RemoveAnimationGroup(DEFAULT_INSTANCE_ID, "key_state");
}

/**
 * @tc.name: AnimationGroupAdapterRemoveIfMatched001
 * @tc.desc: Test RemoveIfMatched branches.
 * @tc.type: FUNC
 */
HWTEST_F(AnimationGroupAdapterTest, AnimationGroupAdapterRemoveIfMatched001, TestSize.Level1)
{
    auto& adapter = AnimationGroupAdapter::GetInstance();
    // instance not found
    adapter.RemoveIfMatched(DEFAULT_INSTANCE_ID, "missing", 1);

    auto createFn = [](int32_t, const std::string&, int64_t) {
        return MakeCreateResult(true);
    };
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_rim", createFn), ERROR_CODE_NO_ERROR);
    auto entry = adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_rim");
    ASSERT_TRUE(entry.has_value());
    int64_t generation = entry->generation;

    // key exists but generation mismatch
    adapter.RemoveIfMatched(DEFAULT_INSTANCE_ID, "key_rim", generation + 100);
    EXPECT_TRUE(adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_rim").has_value());

    // second key to keep instance map non-empty after matched remove
    EXPECT_EQ(adapter.AddAnimationGroup(DEFAULT_INSTANCE_ID, "key_rim2", createFn), ERROR_CODE_NO_ERROR);

    // matched: erase key, map not empty so instance kept
    adapter.RemoveIfMatched(DEFAULT_INSTANCE_ID, "key_rim", generation);
    EXPECT_FALSE(adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_rim").has_value());
    EXPECT_TRUE(adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_rim2").has_value());

    // last key removed: instance map erased
    auto entry2 = adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_rim2");
    ASSERT_TRUE(entry2.has_value());
    adapter.RemoveIfMatched(DEFAULT_INSTANCE_ID, "key_rim2", entry2->generation);
    EXPECT_FALSE(adapter.FindEntry(DEFAULT_INSTANCE_ID, "key_rim2").has_value());
}
} // namespace OHOS::Ace::NG
