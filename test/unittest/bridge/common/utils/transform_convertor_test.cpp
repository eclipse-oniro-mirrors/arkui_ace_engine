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

#include "gtest/gtest.h"

#include "frameworks/bridge/common/dom/dom_type.h"
#include "core/animation/curve.h"
#include "frameworks/bridge/common/utils/transform_convertor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
const double TEST_TIME = 0.5;
}

class TransformConvertorTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {}
    static void TearDownTestSuite()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/**
 * @tc.name: TransformConvertorTest001
 * @tc.desc: Test Convert with unknown key and all translate keys.
 * @tc.type: FUNC
 */
HWTEST_F(TransformConvertorTest, TransformConvertorTest001, TestSize.Level1)
{
    TransformConvertor convertor;
    // unknown key: iter == end branch
    convertor.Convert("unknownKey", "10", TEST_TIME);
    EXPECT_TRUE(convertor.operationList_.empty());

    // translate with two params
    convertor.Convert(DOM_TRANSLATE, "10vp 20vp", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    // translate with one param
    convertor.Convert(DOM_TRANSLATE, "30vp", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    // translate with invalid count: no branch matched
    convertor.Convert(DOM_TRANSLATE, "1vp 2vp 3vp", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    // translate3d with three params
    convertor.Convert(DOM_TRANSLATE_3D, "1vp 2vp 3vp", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 2);

    // translate3d with wrong count, ignored
    convertor.Convert(DOM_TRANSLATE_3D, "1vp 2vp", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 2);

    convertor.Convert(DOM_TRANSLATE_X, "5vp", TEST_TIME);
    convertor.Convert(DOM_TRANSLATE_Y, "6vp", TEST_TIME);
    convertor.Convert(DOM_TRANSLATE_Z, "7vp", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 5);
}

/**
 * @tc.name: TransformConvertorTest002
 * @tc.desc: Test Convert with rotate keys.
 * @tc.type: FUNC
 */
HWTEST_F(TransformConvertorTest, TransformConvertorTest002, TestSize.Level1)
{
    TransformConvertor convertor;
    convertor.Convert(DOM_ROTATE, "45", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);
    convertor.Convert(DOM_ROTATE_X, "50", TEST_TIME);
    convertor.Convert(DOM_ROTATE_Y, "60", TEST_TIME);
    convertor.Convert(DOM_ROTATE_Z, "70", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 4);

    // rotate3d with four params
    convertor.Convert(DOM_ROTATE_3D, "1 2 3 45", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 5);

    // rotate3d with wrong count, ignored
    convertor.Convert(DOM_ROTATE_3D, "1 2 3", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 5);
}

/**
 * @tc.name: TransformConvertorTest003
 * @tc.desc: Test Convert with scale keys.
 * @tc.type: FUNC
 */
HWTEST_F(TransformConvertorTest, TransformConvertorTest003, TestSize.Level1)
{
    TransformConvertor convertor;
    // scale with two values
    convertor.Convert(DOM_SCALE, "2 3", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    // scale with one value (no space)
    convertor.Convert(DOM_SCALE, "1.5", TEST_TIME + 0.1);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    // scale with space but wrong count, ignored
    convertor.Convert(DOM_SCALE, "1 2 3", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    convertor.Convert(DOM_SCALE_X, "2", TEST_TIME);
    convertor.Convert(DOM_SCALE_Y, "3", TEST_TIME);
    convertor.Convert(DOM_SCALE_Z, "4", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 4);

    // scale3d with three params
    convertor.Convert(DOM_SCALE_3D, "1 2 3", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 5);

    // scale3d with wrong count, ignored
    convertor.Convert(DOM_SCALE_3D, "1 2", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 5);
}

/**
 * @tc.name: TransformConvertorTest004
 * @tc.desc: Test Convert with skew/matrix/perspective keys.
 * @tc.type: FUNC
 */
HWTEST_F(TransformConvertorTest, TransformConvertorTest004, TestSize.Level1)
{
    TransformConvertor convertor;
    // skew with two params
    convertor.Convert(DOM_SKEW, "10 20", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    // skew with one param
    convertor.Convert(DOM_SKEW, "30", TEST_TIME + 0.1);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    // skew with wrong count, ignored
    convertor.Convert(DOM_SKEW, "1 2 3", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 1);

    convertor.Convert(DOM_SKEW_X, "5", TEST_TIME);
    convertor.Convert(DOM_SKEW_Y, "6", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 3);

    // matrix3d with 16 params
    std::string matrix3dValue = "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";
    convertor.Convert(DOM_MATRIX_3D, matrix3dValue, TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 4);

    // matrix3d with wrong count, ignored
    convertor.Convert(DOM_MATRIX_3D, "1 2 3", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 4);

    // matrix with 6 params
    convertor.Convert(DOM_MATRIX, "1 0 0 1 10 10", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 5);

    // matrix with wrong count, ignored
    convertor.Convert(DOM_MATRIX, "1 2", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 5);

    convertor.Convert(DOM_PERSPECTIVE, "20vp", TEST_TIME);
    EXPECT_EQ(convertor.operationList_.size(), 6);
}

/**
 * @tc.name: TransformConvertorTest005
 * @tc.desc: Test InsertIdentityKeyframe and AddAnimationToTweenOption branches.
 * @tc.type: FUNC
 */
HWTEST_F(TransformConvertorTest, TransformConvertorTest005, TestSize.Level1)
{
    TransformConvertor convertor;
    // no animation: empty operationList_ loop branch false
    TweenOption emptyOption;
    convertor.AddAnimationToTweenOption(emptyOption);

    convertor.InsertIdentityKeyframe(0.0);
    convertor.InsertIdentityKeyframe(1.0);
    EXPECT_EQ(convertor.noneKeyframeTimes_.size(), 2);

    // animation exists and noneKeyframes not empty
    convertor.Convert(DOM_TRANSLATE, "10vp 20vp", TEST_TIME);
    TweenOption option;
    convertor.AddAnimationToTweenOption(option);

    // no none keyframe now: noneKeyframes.empty() true inside loop
    convertor.noneKeyframeTimes_.clear();
    convertor.AddAnimationToTweenOption(option);
}

/**
 * @tc.name: TransformConvertorTest006
 * @tc.desc: Test ApplyCurve with empty/full list and null animation, ClearAnimations.
 * @tc.type: FUNC
 */
HWTEST_F(TransformConvertorTest, TransformConvertorTest006, TestSize.Level1)
{
    TransformConvertor convertor;
    auto linearCurve = AceType::MakeRefPtr<LinearCurve>();
    // empty list: loop not entered
    convertor.ApplyCurve(linearCurve);

    convertor.Convert(DOM_TRANSLATE, "1vp 2vp", TEST_TIME);
    convertor.ApplyCurve(linearCurve);

    // null animation inside list: animation false branch
    convertor.operationList_.push_back(nullptr);
    convertor.ApplyCurve(AceType::MakeRefPtr<DecelerationCurve>());

    // second AddKeyframe on same type: animation already exists branch
    convertor.ClearAnimations();
    EXPECT_TRUE(convertor.operationList_.empty());
    EXPECT_TRUE(convertor.operationMap_.empty());
    convertor.Convert(DOM_TRANSLATE, "1vp 2vp", TEST_TIME);
    convertor.Convert(DOM_TRANSLATE, "3vp 4vp", TEST_TIME + 0.5);
    EXPECT_EQ(convertor.operationList_.size(), 1);
}
} // namespace OHOS::Ace
