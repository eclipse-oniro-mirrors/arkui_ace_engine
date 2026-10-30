/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#include "base/memory/referenced.h"
// Add the following two macro definitions to test the private and protected method.
#define private public
#define protected public
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/animation/evaluator.h"
#include "base/geometry/animatable_matrix4.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
const int32_t VALID_ROW0 = 0;
const int32_t VALID_COL0 = 0;
const double DEFAULT_DOUBLE0 = 0.0;
} // namespace

class AnimatableMatrix4Test : public testing::Test {};

/**
 * @tc.name: AnimatableMatrix4Test001
 * @tc.desc: Test the function operator= of the class AnimatableMatrix4.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test001, TestSize.Level1)
{
    /**
     * @tc.steps: Test the function operator= with given Matrix4 object.
     */
    AnimatableMatrix4 animatableMatrix4Obj1;
    Matrix4 matrixObj1 = Matrix4::CreateIdentity();
    animatableMatrix4Obj1 = matrixObj1;
    EXPECT_TRUE(animatableMatrix4Obj1.IsIdentityMatrix());
    matrixObj1.Set(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE0);

    /**
     * @tc.steps: Test the function MoveTo with given Matrix4 object.
     */
    animatableMatrix4Obj1.MoveTo(matrixObj1);
    EXPECT_DOUBLE_EQ(animatableMatrix4Obj1.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE0);

    /**
     * @tc.steps: Test the function operator= with given AnimatableMatrix4 object.
     */
    AnimatableMatrix4 animatableMatrix4Obj2;
    animatableMatrix4Obj2 = animatableMatrix4Obj1;
    EXPECT_DOUBLE_EQ(animatableMatrix4Obj2.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE0);
}

/**
 * @tc.name: AnimatableInitializeTest
 * @tc.desc: Test the function reload operator=
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableInitializeTest001, TestSize.Level1)
{
    /**
    * @tc.steps: initialize parameters.
    */
    AnimatableMatrix4 animatableMatrix4Obj1;
    EXPECT_TRUE(animatableMatrix4Obj1.isFirstAssign_);
}

/**
 * @tc.name: AnimatableMatrix4Test002
 * @tc.desc: Test the function AnimateTo of the class AnimatableMatrix4.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test002, TestSize.Level1)
{
    /**
     * @tc.steps: initialize parameters.
     */
    AnimatableMatrix4 animatableMatrix4Obj1;
    Matrix4 endValue = Matrix4::CreateIdentity();
    EXPECT_TRUE(animatableMatrix4Obj1.isFirstAssign_);

    /**
     * @tc.steps: Test the function AnimateTo firstly, enter the first if-branch.
     */
    animatableMatrix4Obj1.AnimateTo(endValue);
    EXPECT_FALSE(animatableMatrix4Obj1.isFirstAssign_);
    EXPECT_TRUE(animatableMatrix4Obj1.IsIdentityMatrix());
    EXPECT_EQ(animatableMatrix4Obj1.animationController_, nullptr);

    /**
     * @tc.steps: Test the function AnimateTo secondly, enter the second if-branch.
     */
    animatableMatrix4Obj1.AnimateTo(animatableMatrix4Obj1);
    EXPECT_EQ(animatableMatrix4Obj1.animationController_, nullptr);

    /**
     * @tc.steps: Test the function AnimateTo thirdly, the function will run until the end of it.
     */
    animatableMatrix4Obj1.AnimateTo(endValue);
    EXPECT_EQ(animatableMatrix4Obj1.animationController_, nullptr);
}

/**
 * @tc.name: AnimatableMatrix4Test003
 * @tc.desc: Test the function ResetController of the class AnimatableMatrix4.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test003, TestSize.Level1)
{
    AnimatableMatrix4 animatableMatrix4Obj1;
    animatableMatrix4Obj1.ResetController();
    animatableMatrix4Obj1.animationController_ = CREATE_ANIMATOR(nullptr);
    EXPECT_NE(animatableMatrix4Obj1.animationController_, nullptr);
    animatableMatrix4Obj1.ResetController();
    EXPECT_EQ(animatableMatrix4Obj1.animationController_, nullptr);
}

/**
 * @tc.name: AnimatableMatrix4Test004
 * @tc.desc: Test the function OnAnimationCallback of the class AnimatableMatrix4.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test004, TestSize.Level1)
{
    /**
     * @tc.steps: initialize parameters.
     */
    TransformOperation transformOperation;
    AnimatableMatrix4 animatableMatrix4Obj1;
    animatableMatrix4Obj1.OnAnimationCallback(transformOperation);
    bool flagEventCbk = false;
    animatableMatrix4Obj1.SetContextAndCallback(nullptr, [&flagEventCbk]() { flagEventCbk = true; });
    /**
     * @tc.steps: Set the animationCallback_ as the function which changes the value of flagEventCbk,
     *            the value will be modified when the onReadyEvent is fired.
     */
    animatableMatrix4Obj1.OnAnimationCallback(transformOperation);
    EXPECT_TRUE(flagEventCbk);
}

/**
 * @tc.name: animationControllerTest
 * @tc.desc: Test the function ResetController of the class AnimatableMatrix4.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimationControllerTest001, TestSize.Level1)
{
    AnimatableMatrix4 animatableMatrix4Obj1;
    animatableMatrix4Obj1.ResetController();
    animatableMatrix4Obj1.animationController_ = CREATE_ANIMATOR(nullptr);
    EXPECT_NE(animatableMatrix4Obj1.animationController_, nullptr);
}

/**
 * @tc.name: AnimatableMatrix4Test005
 * @tc.desc: Test the function ResetAnimatableMatrix of the class AnimatableMatrix4.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test005, TestSize.Level1)
{
    AnimatableMatrix4 animatableMatrix4Obj1;
    animatableMatrix4Obj1.isFirstAssign_ = false;
    animatableMatrix4Obj1.ResetAnimatableMatrix();
    EXPECT_TRUE(animatableMatrix4Obj1.isFirstAssign_);
}

/**
 * @tc.name: AnimatableMatrix4Test006
 * @tc.desc: Test the function reload operator=
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test006, TestSize.Level1)
{
    AnimatableMatrix4 animatableMatrix4Obj1;
    AnimatableMatrix4 animatableMatrix4Obj2;
    ASSERT_EQ(animatableMatrix4Obj1.animationCallback_, nullptr);
    ASSERT_EQ(animatableMatrix4Obj2.animationCallback_, nullptr);
    // true true, both animationCallback_ and pipelineContext nullptr
    animatableMatrix4Obj2 = animatableMatrix4Obj1;
    ASSERT_EQ(animatableMatrix4Obj2.animationCallback_, nullptr);
    // false true, animationCallback_ not nullptr, pipelineContext  nullptr
    bool flagEventCbk = false;
    animatableMatrix4Obj2.SetContextAndCallback(nullptr, [&flagEventCbk]() { flagEventCbk = true; });
    animatableMatrix4Obj2 = animatableMatrix4Obj1;
    ASSERT_NE(animatableMatrix4Obj2.animationCallback_, nullptr);
}

/**
 * @tc.name: AnimatableMatrix4Test007
 * @tc.desc: Test AnimateTo with evaluator and stopCallback, running controller reset.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test007, TestSize.Level1)
{
    AnimatableMatrix4 animatableMatrix4Obj;
    animatableMatrix4Obj.SetEvaluator(AceType::MakeRefPtr<LinearEvaluator<TransformOperation>>());
    animatableMatrix4Obj.SetContextAndCallback(nullptr, []() {});
    animatableMatrix4Obj.SetAnimationStopCallback([]() {});
    AnimationOption finishOption;
    finishOption.SetDuration(100);
    finishOption.SetOnFinishEvent([]() {});
    animatableMatrix4Obj.SetAnimationOption(finishOption);
    animatableMatrix4Obj.MoveTo(Matrix4::CreateIdentity());

    Matrix4 endValue = Matrix4::CreateIdentity();
    endValue.Set(VALID_ROW0, VALID_COL0, 10.0);
    // different value: run through AnimateTo fully, evaluator_ and stopCallback_ branches
    animatableMatrix4Obj.AnimateTo(endValue);
    EXPECT_NE(animatableMatrix4Obj.animationController_, nullptr);

    // same value with evaluator_: !evaluator_ false branch, then ResetController stop branch
    animatableMatrix4Obj.AnimateTo(endValue);
    EXPECT_NE(animatableMatrix4Obj.animationController_, nullptr);

    animatableMatrix4Obj.SetEvaluator(nullptr);
    // same value without evaluator_: early return branch
    animatableMatrix4Obj.MoveTo(endValue);
    animatableMatrix4Obj.animationController_ = nullptr;
    animatableMatrix4Obj.AnimateTo(endValue);
    EXPECT_EQ(animatableMatrix4Obj.animationController_, nullptr);
}

/**
 * @tc.name: AnimatableMatrix4Test008
 * @tc.desc: Test operator= with valid pipeline context and explicit animation option.
 * @tc.type: FUNC
 */
HWTEST_F(AnimatableMatrix4Test, AnimatableMatrix4Test008, TestSize.Level1)
{
    NG::MockPipelineContext::SetUp();
    auto pipelineContext = NG::MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);

    AnimationOption explicitOption;
    explicitOption.SetDuration(200);
    pipelineContext->SaveExplicitAnimationOption(explicitOption);

    AnimationOption srcOption;
    srcOption.SetDuration(100);
    AnimatableMatrix4 srcMatrix(
        Matrix4::CreateIdentity(), srcOption);
    AnimatableMatrix4 dstMatrix;
    dstMatrix.SetContextAndCallback(AceType::WeakClaim(AceType::RawPtr(pipelineContext)), []() {});
    // explicit animation valid branch
    dstMatrix = srcMatrix;
    EXPECT_FALSE(dstMatrix.isFirstAssign_);

    pipelineContext->ClearExplicitAnimationOption();
    // animationOption_ valid branch
    AnimatableMatrix4 dstMatrix2;
    dstMatrix2.SetContextAndCallback(AceType::WeakClaim(AceType::RawPtr(pipelineContext)), []() {});
    dstMatrix2.isFirstAssign_ = false;
    dstMatrix2 = srcMatrix;

    // animationOption_ invalid branch: reset controller and assign directly
    AnimatableMatrix4 plainSrc(Matrix4::CreateIdentity());
    AnimatableMatrix4 dstMatrix3;
    dstMatrix3.SetContextAndCallback(AceType::WeakClaim(AceType::RawPtr(pipelineContext)), []() {});
    dstMatrix3.animationController_ = CREATE_ANIMATOR(nullptr);
    dstMatrix3 = plainSrc;
    EXPECT_EQ(dstMatrix3.animationController_, nullptr);
    EXPECT_FALSE(dstMatrix3.isFirstAssign_);

    NG::MockPipelineContext::TearDown();
}
} // namespace OHOS::Ace
