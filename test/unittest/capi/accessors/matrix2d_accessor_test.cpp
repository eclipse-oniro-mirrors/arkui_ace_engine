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

#include "accessor_test_base.h"
#include "gmock/gmock.h"

#include "core/components_ng/pattern/canvas/canvas_paint_method.h"
#include "core/interfaces/native/implementation/canvas_renderer_peer_impl.h"
#include "core/interfaces/native/implementation/canvas_pattern_peer.h"
#include "core/interfaces/native/implementation/canvas_gradient_peer.h"
#include "core/interfaces/native/implementation/matrix2d_peer_impl.h"
#include "core/interfaces/native/implementation/image_bitmap_peer_impl.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

namespace {
const auto TRANSFORM_UNITY_VALUE = 1.00;
const auto TRANSFORM_ZERO_VALUE = 0.00;
const double FLT_PRECISION = 0.001;
const double DENSITY_1_25 = 1.25;
const double DENSITY_2_50 = 2.50;
const double DEFAULT_DENSITY = 1.0;

std::vector<std::vector<double>>
    ARRAY_NUMBER_TEST_PLAN = {
        { 100, 10.25, 2.35, 5.42, 12.34, 56.73 },
        { 100, 10.25, 0, 5.42, 12.34, 56.73 },
        { 100, -10.25, 0, -5.42, 12.34, -56.73 },
        { 0, 0, 0, 0, 0, 0 },
        { -100, -10.25, -2.35, -5.42, -12.34, -56.73 },
    };

std::vector<double> NUMBER_TEST_PLAN = {
    100, 0, -0.54, 0.98, 1.00, 1.01, -100, -151, -10.25, -2.35, -5.42, -12.34, -56.73, 151
};
std::vector<double> ROTATE_TEST_PLAN = { 0, M_PI / 2, M_PI, 3 * M_PI / 2, 2 * M_PI };

} // namespace

class Matrix2DAccessorTest
    : public AccessorTestBase<GENERATED_ArkUIMatrix2DAccessor,
    &GENERATED_ArkUIAccessors::getMatrix2DAccessor, Matrix2DPeer> {
public:
    void SetUp(void) override
    {
        AccessorTestBase::SetUp();
    }

    void TearDown() override
    {
        AccessorTestBaseParent::TearDown();
        ChangeDensity(DEFAULT_DENSITY);
    }

    void ChangeDensity(const double density)
    {
        auto pipelineContext =
            AceType::DynamicCast<NG::MockPipelineContext>(NG::MockPipelineContext::GetCurrentContext());
        pipelineContext->SetDensity(density);
    }
};

/**
 * @tc.name: identityTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, identityTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->identity, nullptr);
    for (const auto& actual : ARRAY_NUMBER_TEST_PLAN) {
        peer_->SetScaleX(actual[0]);
        peer_->SetScaleY(actual[1]);
        peer_->SetRotateX(actual[2]);
        peer_->SetRotateY(actual[3]);
        peer_->SetTranslateX(actual[4]);
        peer_->SetTranslateY(actual[5]);

        accessor_->identity(peer_);

        EXPECT_NEAR(peer_->GetScaleX(), TRANSFORM_UNITY_VALUE, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetScaleY(), TRANSFORM_UNITY_VALUE, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetRotateX(), TRANSFORM_UNITY_VALUE, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetRotateY(), TRANSFORM_UNITY_VALUE, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetTranslateX(), TRANSFORM_UNITY_VALUE, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetTranslateY(), TRANSFORM_UNITY_VALUE, FLT_PRECISION);
    }
}

/**
 * @tc.name: invertTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, invertTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->invert, nullptr);
    for (const auto& actual : ARRAY_NUMBER_TEST_PLAN) {
        peer_->SetScaleX(actual[0]);
        peer_->SetScaleY(actual[1]);
        peer_->SetRotateX(actual[2]);
        peer_->SetRotateY(actual[3]);
        peer_->SetTranslateX(actual[4]);
        peer_->SetTranslateY(actual[5]);
        auto param = peer_->GetTransform();

        accessor_->invert(peer_);

        auto result =
            std::all_of(actual.begin(), actual.end(), [](double i) { return NearEqual(i, TRANSFORM_ZERO_VALUE); });
        if (result) {
            EXPECT_FALSE(NearEqual(peer_->GetScaleX(), TRANSFORM_ZERO_VALUE));
            EXPECT_FALSE(NearEqual(peer_->GetScaleY(), TRANSFORM_ZERO_VALUE));
            EXPECT_FALSE(NearEqual(peer_->GetRotateX(), TRANSFORM_ZERO_VALUE));
            EXPECT_FALSE(NearEqual(peer_->GetRotateY(), TRANSFORM_ZERO_VALUE));
            EXPECT_FALSE(NearEqual(peer_->GetTranslateX(), TRANSFORM_ZERO_VALUE));
            EXPECT_FALSE(NearEqual(peer_->GetTranslateY(), TRANSFORM_ZERO_VALUE));
            continue;
        }
        EXPECT_NEAR(peer_->GetScaleX(), param.scaleY, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetScaleY(), param.scaleX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetRotateX(), param.skewY, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetRotateY(), param.skewX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetTranslateX(), param.translateY, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetTranslateY(), param.translateX, FLT_PRECISION);
    }
}

/**
 * @tc.name: translateTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, translateTest, TestSize.Level1)
{
    auto param = ARRAY_NUMBER_TEST_PLAN[0];
    ASSERT_NE(accessor_->translate, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetTranslateX(param[1]);
        peer_->SetTranslateY(param[5]);
        auto expectedX = (peer_->GetTranslateX() + actual);
        auto expectedY = (peer_->GetTranslateY() + TRANSFORM_ZERO_VALUE);
        auto tx = Converter::ArkValue<Opt_Number>(static_cast<float>(actual));
        accessor_->translate(peer_, &tx, nullptr);
        EXPECT_NEAR(peer_->GetTranslateX(), expectedX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetTranslateY(), expectedY, FLT_PRECISION);
    }
    // with density
    ChangeDensity(DENSITY_1_25);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetTranslateX(param[1]);
        peer_->SetTranslateY(param[5]);
        auto expectedX = (peer_->GetTranslateX() + TRANSFORM_ZERO_VALUE);
        auto expectedY = (peer_->GetTranslateY() + actual);
        auto ty = Converter::ArkValue<Opt_Number>(static_cast<float>(actual));
        accessor_->translate(peer_, nullptr, &ty);
        EXPECT_NEAR(peer_->GetTranslateX(), expectedX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetTranslateY(), expectedY, FLT_PRECISION);
    }
}

/**
 * @tc.name: scaleTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, scaleTest, TestSize.Level1)
{
    auto param = ARRAY_NUMBER_TEST_PLAN[0];
    ASSERT_NE(accessor_->scale, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetScaleX(param[1]);
        peer_->SetScaleY(param[5]);
        auto expectedX = (peer_->GetScaleX() * actual);
        auto expectedY = (peer_->GetScaleY() * TRANSFORM_UNITY_VALUE);
        auto sx = Converter::ArkValue<Opt_Number>(static_cast<float>(actual));
        auto sy = Converter::ArkValue<Opt_Number>(Ark_Empty());
        accessor_->scale(peer_, &sx, &sy);
        EXPECT_NEAR(peer_->GetScaleX(), expectedX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetScaleY(), expectedY, FLT_PRECISION);
    }
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetScaleX(param[1]);
        peer_->SetScaleY(param[5]);
        auto expectedX = (peer_->GetScaleX() * TRANSFORM_UNITY_VALUE);
        auto expectedY = (peer_->GetScaleY() * actual);
        auto sy = Converter::ArkValue<Opt_Number>(static_cast<float>(actual));
        accessor_->scale(peer_, nullptr, &sy);
        EXPECT_NEAR(peer_->GetScaleX(), expectedX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetScaleY(), expectedY, FLT_PRECISION);
    }
}

/**
 * @tc.name: getScaleXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, getScaleXTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getScaleX, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetScaleX(actual);
        auto result = Converter::OptConvert<float>(accessor_->getScaleX(peer_));
        ASSERT_TRUE(result.has_value());
        EXPECT_NEAR(result.value(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: setScaleXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, setScaleXTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->setScaleX, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        auto sx = Converter::ArkValue<Ark_Number>(static_cast<float>(actual));
        accessor_->setScaleX(peer_, &sx);
        EXPECT_NEAR(peer_->GetScaleX(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: getRotateYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, getRotateYTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getRotateY, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetRotateY(actual);
        auto result = Converter::OptConvert<float>(accessor_->getRotateY(peer_));
        ASSERT_TRUE(result);
        EXPECT_NEAR(result.value(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: getRotateXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, getRotateXTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getRotateX, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetRotateX(actual);
        auto result = Converter::OptConvert<float>(accessor_->getRotateX(peer_));
        ASSERT_TRUE(result);
        EXPECT_NEAR(result.value(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: getScaleYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, getScaleYTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getScaleY, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetScaleY(actual);
        auto result = Converter::OptConvert<float>(accessor_->getScaleY(peer_));
        ASSERT_TRUE(result);
        EXPECT_NEAR(result.value(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: getTranslateXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, getTranslateXTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getTranslateX, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetTranslateX(actual);
        auto result = Converter::OptConvert<float>(accessor_->getTranslateX(peer_));
        ASSERT_TRUE(result);
        EXPECT_NEAR(result.value(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: getTranslateYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, getTranslateYTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getTranslateY, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetTranslateY(actual);
        auto result = Converter::OptConvert<float>(accessor_->getTranslateY(peer_));
        ASSERT_TRUE(result);
        EXPECT_NEAR(result.value(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: rotate1Test
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, rotate1Test, TestSize.Level1)
{
    auto param = ARRAY_NUMBER_TEST_PLAN[0];
    ASSERT_NE(accessor_->rotate1, nullptr);
    for (const auto& actual : ROTATE_TEST_PLAN) {
        peer_->SetRotateX(param[1]);
        peer_->SetRotateY(param[5]);
        auto expectedX = (peer_->GetRotateX() * actual * peer_->GetDensity());
        auto expectedY = (peer_->GetRotateY() * actual * peer_->GetDensity());
        auto degree = Converter::ArkValue<Ark_Number>(static_cast<float>(actual));
        auto rx = Converter::ArkValue<Opt_Number>(static_cast<float>(TRANSFORM_UNITY_VALUE));
        auto ry = Converter::ArkValue<Opt_Number>(static_cast<float>(TRANSFORM_UNITY_VALUE));
        accessor_->rotate1(peer_, &degree, &rx, &ry);
        EXPECT_NEAR(peer_->GetRotateX(), expectedX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetRotateY(), expectedY, FLT_PRECISION);
    }
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetRotateX(param[1]);
        peer_->SetRotateY(param[5]);
        auto expectedX = (peer_->GetRotateX() * actual * peer_->GetDensity());
        auto expectedY = (peer_->GetRotateY() * TRANSFORM_UNITY_VALUE * peer_->GetDensity());
        auto degree = Converter::ArkValue<Ark_Number>(static_cast<float>(TRANSFORM_UNITY_VALUE));
        auto rx = Converter::ArkValue<Opt_Number>(static_cast<float>(actual));
        auto ry = Converter::ArkValue<Opt_Number>(static_cast<float>(TRANSFORM_UNITY_VALUE));
        accessor_->rotate1(peer_, &degree, &rx, &ry);
        EXPECT_NEAR(peer_->GetRotateX(), expectedX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetRotateY(), expectedY, FLT_PRECISION);
    }
    // with density
    ChangeDensity(DENSITY_2_50);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        peer_->SetRotateX(param[1]);
        peer_->SetRotateY(param[5]);
        auto expectedX = (peer_->GetRotateX() * TRANSFORM_UNITY_VALUE * peer_->GetDensity());
        auto expectedY = (peer_->GetRotateY() * actual * peer_->GetDensity());
        auto degree = Converter::ArkValue<Ark_Number>(static_cast<float>(TRANSFORM_UNITY_VALUE));
        auto rx = Converter::ArkValue<Opt_Number>(static_cast<float>(TRANSFORM_UNITY_VALUE));
        auto ry = Converter::ArkValue<Opt_Number>(static_cast<float>(actual));
        accessor_->rotate1(peer_, &degree, &rx, &ry);
        EXPECT_NEAR(peer_->GetRotateX(), expectedX, FLT_PRECISION);
        EXPECT_NEAR(peer_->GetRotateY(), expectedY, FLT_PRECISION);
    }
}

/**
 * @tc.name: rotate1Test
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, rotate1InvalidValuesTest, TestSize.Level1)
{
    auto param = ARRAY_NUMBER_TEST_PLAN[0];
    ASSERT_NE(accessor_->rotate1, nullptr);
    auto expectedX = param[1];
    auto expectedY = param[5];
    auto degree = Converter::ArkValue<Ark_Number>(TRANSFORM_UNITY_VALUE);
    auto optEmpty = Converter::ArkValue<Opt_Number>();
    auto optUnity = Converter::ArkValue<Opt_Number>(static_cast<float>(TRANSFORM_UNITY_VALUE));

    peer_->SetRotateX(param[1]);
    peer_->SetRotateY(param[5]);
    accessor_->rotate1(peer_, &degree, &optEmpty, &optUnity);
    EXPECT_NEAR(peer_->GetRotateX(), TRANSFORM_ZERO_VALUE, FLT_PRECISION);
    EXPECT_NEAR(peer_->GetRotateY(), expectedY, FLT_PRECISION);

    peer_->SetRotateX(param[1]);
    peer_->SetRotateY(param[5]);
    accessor_->rotate1(peer_, &degree, &optUnity, &optEmpty);
    EXPECT_NEAR(peer_->GetRotateX(), expectedX, FLT_PRECISION);
    EXPECT_NEAR(peer_->GetRotateY(), TRANSFORM_ZERO_VALUE, FLT_PRECISION);

    peer_->SetRotateX(param[1]);
    peer_->SetRotateY(param[5]);
    accessor_->rotate1(peer_, &degree, nullptr, &optUnity);
    EXPECT_NEAR(peer_->GetRotateX(), TRANSFORM_ZERO_VALUE, FLT_PRECISION);
    EXPECT_NEAR(peer_->GetRotateY(), expectedY, FLT_PRECISION);

    peer_->SetRotateX(param[1]);
    peer_->SetRotateY(param[5]);
    accessor_->rotate1(peer_, &degree, &optUnity, nullptr);
    EXPECT_NEAR(peer_->GetRotateX(), expectedX, FLT_PRECISION);
    EXPECT_NEAR(peer_->GetRotateY(), TRANSFORM_ZERO_VALUE, FLT_PRECISION);

    peer_->SetRotateX(param[1]);
    peer_->SetRotateY(param[5]);
    accessor_->rotate1(peer_, &degree, nullptr, nullptr);
    EXPECT_NEAR(peer_->GetRotateX(), TRANSFORM_ZERO_VALUE, FLT_PRECISION);
    EXPECT_NEAR(peer_->GetRotateY(), TRANSFORM_ZERO_VALUE, FLT_PRECISION);
}

/**
 * @tc.name: setRotateYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, setRotateYTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->setRotateY, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        auto sy = Converter::ArkValue<Ark_Number>(static_cast<float>(actual));
        accessor_->setRotateY(peer_, &sy);
        EXPECT_NEAR(peer_->GetRotateY(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: setRotateXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, setRotateXTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->setRotateX, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        auto sx = Converter::ArkValue<Ark_Number>(static_cast<float>(actual));
        accessor_->setRotateX(peer_, &sx);
        EXPECT_NEAR(peer_->GetRotateX(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: setScaleYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, setScaleYTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->setScaleY, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        auto sy = Converter::ArkValue<Ark_Number>(static_cast<float>(actual));
        accessor_->setScaleY(peer_, &sy);
        EXPECT_NEAR(peer_->GetScaleY(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: setTranslateXTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, setTranslateXTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->setTranslateX, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        auto tx = Converter::ArkValue<Ark_Number>(static_cast<float>(actual));
        accessor_->setTranslateX(peer_, &tx);
        EXPECT_NEAR(peer_->GetTranslateX(), actual, FLT_PRECISION);
    }
}

/**
 * @tc.name: setTranslateYTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(Matrix2DAccessorTest, setTranslateYTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->setTranslateY, nullptr);
    for (const auto& actual : NUMBER_TEST_PLAN) {
        auto ty = Converter::ArkValue<Ark_Number>(static_cast<float>(actual));
        accessor_->setTranslateY(peer_, &ty);
        EXPECT_NEAR(peer_->GetTranslateY(), actual, FLT_PRECISION);
    }
}
} // namespace OHOS::Ace::NG
