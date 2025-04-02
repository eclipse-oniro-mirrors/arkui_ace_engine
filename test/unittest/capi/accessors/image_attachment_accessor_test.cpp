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

#include "accessor_test_base.h"
#include "accessor_test_fixtures.h"
#include "gmock/gmock.h"
#include "node_api.h"

#include "core/interfaces/native/implementation/image_attachment_peer.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;
using namespace AccessorTestFixtures;
using namespace Converter;

namespace GeneratedModifier {
const GENERATED_ArkUIPixelMapAccessor* GetPixelMapAccessor();
}
class ImageAttachmentAccessorTest : public AccessorTestCtorBase<GENERATED_ArkUIImageAttachmentAccessor,
        &GENERATED_ArkUIAccessors::getImageAttachmentAccessor, ImageAttachmentPeer> {
public:
    void SetUp(void) override
    {
        AccessorTestCtorBase::SetUp();
        for (auto& [id, strid, res] : resourceInitTable) {
            AddResource(id, res);
            AddResource(strid, res);
        }
    }
    void* CreatePeerInstance() override
    {
        return accessor_->ctor(nullptr);
    }
    RefPtr<PixelMap> CreatePixelMap(std::string& src);
};

RefPtr<PixelMap> ImageAttachmentAccessorTest::CreatePixelMap(std::string& src)
{
    void* ptr = reinterpret_cast<void*>(src.data());
    return PixelMap::CreatePixelMap(ptr);
}

namespace {
const CalcLength TEST_CALC_LENGHT(123.0_vp);
const auto TEST_DIMENSION = TEST_CALC_LENGHT.GetDimension();

const MarginProperty MARGIN_PADDING_PROPERTY = {
    .left = TEST_CALC_LENGHT,
    .right = TEST_CALC_LENGHT,
    .top = TEST_CALC_LENGHT,
    .bottom = TEST_CALC_LENGHT
};
const BorderRadiusProperty BORDER_RADIUES_PROPERTY(TEST_DIMENSION);

Opt_ImageAttachmentLayoutStyle getImageLayoutStyleFilled()
{
    const Ark_Padding arkPadding = {
        .left = ArkValue<Opt_Length>(TEST_DIMENSION),
        .top = ArkValue<Opt_Length>(TEST_DIMENSION),
        .right = ArkValue<Opt_Length>(TEST_DIMENSION),
        .bottom = ArkValue<Opt_Length>(TEST_DIMENSION),
    };
    const Ark_BorderRadiuses arkBorderRadiuses = {
        .bottomLeft = ArkValue<Opt_Length>(TEST_DIMENSION),
        .bottomRight = ArkValue<Opt_Length>(TEST_DIMENSION),
        .topLeft = ArkValue<Opt_Length>(TEST_DIMENSION),
        .topRight = ArkValue<Opt_Length>(TEST_DIMENSION)
    };
    const Ark_ImageAttachmentLayoutStyle imageLayoutStyle {
        .margin = ArkUnion<Opt_Union_LengthMetrics_Margin, Ark_Padding>(arkPadding),
        .padding = ArkUnion<Opt_Union_LengthMetrics_Padding, Ark_Padding>(arkPadding),
        .borderRadius = ArkUnion<Opt_Union_LengthMetrics_BorderRadiuses, Ark_BorderRadiuses>(arkBorderRadiuses),
    };
    return ArkValue<Opt_ImageAttachmentLayoutStyle>(imageLayoutStyle);
}
Opt_ImageAttachmentLayoutStyle getImageLayoutStyleLengthMetrics()
{
    const Ark_LengthMetrics lengthMetrics = ArkValue<Ark_LengthMetrics>(TEST_DIMENSION);
    const Ark_ImageAttachmentLayoutStyle imageLayoutStyle {
        .margin = ArkUnion<Opt_Union_LengthMetrics_Margin, Ark_LengthMetrics>(lengthMetrics),
        .padding = ArkUnion<Opt_Union_LengthMetrics_Padding, Ark_LengthMetrics>(lengthMetrics),
        .borderRadius = ArkUnion<Opt_Union_LengthMetrics_BorderRadiuses, Ark_LengthMetrics>(lengthMetrics),
    };
    return ArkValue<Opt_ImageAttachmentLayoutStyle>(imageLayoutStyle);
};

Opt_ImageAttachmentLayoutStyle getImageLayoutStyleOptional()
{
    Ark_ImageAttachmentLayoutStyle imageLayoutStyle = {
        .margin = ArkUnion<Opt_Union_LengthMetrics_Margin>(Ark_Empty()),
        .padding = ArkUnion<Opt_Union_LengthMetrics_Padding>(Ark_Empty()),
        .borderRadius = ArkUnion<Opt_Union_LengthMetrics_BorderRadiuses>(Ark_Empty()),
    };
    return ArkValue<Opt_ImageAttachmentLayoutStyle>(imageLayoutStyle);
};
} // namespace
/**
 * @tc.name: ctorTestPixelMap
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestPixelMap, TestSize.Level1)
{
    std::string imagesSrc = "test";
    RefPtr<PixelMap> pixelMap = CreatePixelMap(imagesSrc);
    auto arkPixelMap = GeneratedModifier::GetPixelMapAccessor()->ctor();
    arkPixelMap->pixelMap = pixelMap;

    Ark_ImageAttachmentInterface value {
        .value = arkPixelMap,
    };
    auto peer = accessor_->ctor(&value);
    EXPECT_EQ(peer->span->GetImageSpanOptions().imagePixelMap, pixelMap);
    accessor_->destroyPeer(peer);
    GeneratedModifier::GetPixelMapAccessor()->destroyPeer(arkPixelMap);
}

/**
 * @tc.name: ctorTestSize
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestSize, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureDimensionAnyValidValues) {
        Ark_SizeOptions size {
            .width = ArkValue<Opt_Length>(test),
            .height = ArkValue<Opt_Length>(test),
        };
        Ark_ImageAttachmentInterface value {
            .size = ArkValue<Opt_SizeOptions>(size),
        };
        auto peer = accessor_->ctor(&value);
        ASSERT_TRUE(peer->span->GetImageAttribute());
        ASSERT_TRUE(peer->span->GetImageAttribute()->size);
        if (expected.IsNonNegative()) {
            ASSERT_TRUE(peer->span->GetImageAttribute()->size->width);
            ASSERT_TRUE(peer->span->GetImageAttribute()->size->height);
            EXPECT_EQ(peer->span->GetImageAttribute()->size->width->ToString(), expected.ToString());
            EXPECT_EQ(peer->span->GetImageAttribute()->size->height->ToString(), expected.ToString());
        } else {
            ASSERT_FALSE(peer->span->GetImageAttribute()->size->width);
            ASSERT_FALSE(peer->span->GetImageAttribute()->size->height);
        }
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: ctorTestSizeResources
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestSizeResources, TestSize.Level1)
{
    for (auto& [num_id, str_id, expected] : resourceInitTable) {
        auto expectPointer = std::get_if<Dimension>(&expected);
        ASSERT_TRUE(expectPointer);
        auto sizeResource = ArkValue<Ark_Length>(Ark_Length { .type = Ark_Tag::ARK_TAG_RESOURCE, .resource = num_id });
        Ark_SizeOptions size {
            .width = ArkValue<Opt_Length>(sizeResource),
            .height = ArkValue<Opt_Length>(sizeResource),
        };
        Ark_ImageAttachmentInterface value {
            .size = ArkValue<Opt_SizeOptions>(size),
        };
        auto peer = accessor_->ctor(&value);
        ASSERT_TRUE(peer->span->GetImageAttribute());
        ASSERT_TRUE(peer->span->GetImageAttribute()->size);
        if (expectPointer->IsNonNegative()) {
            ASSERT_TRUE(peer->span->GetImageAttribute()->size->width);
            ASSERT_TRUE(peer->span->GetImageAttribute()->size->height);
            EXPECT_EQ(peer->span->GetImageAttribute()->size->width->ToString(), expectPointer->ToString());
            EXPECT_EQ(peer->span->GetImageAttribute()->size->height->ToString(), expectPointer->ToString());
        } else {
            ASSERT_FALSE(peer->span->GetImageAttribute()->size->width);
            ASSERT_FALSE(peer->span->GetImageAttribute()->size->height);
        }
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: ctorTestVerticalAlignValidValues
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestVerticalAlignValidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureVerticalAlignValidValues) {
        Ark_ImageAttachmentInterface value {
            .verticalAlign = ArkValue<Opt_ImageSpanAlignment>(test),
        };
        auto peer = accessor_->ctor(&value);
        ASSERT_TRUE(peer->span->GetImageAttribute());
        ASSERT_TRUE(peer->span->GetImageAttribute()->verticalAlign);
        EXPECT_EQ(peer->span->GetImageAttribute()->verticalAlign, expected);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: ctorTestVerticalAlignInvalidValues
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestVerticalAlignInvalidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureVerticalAlignInvalidValues) {
        Ark_ImageAttachmentInterface value {
            .verticalAlign = ArkValue<Opt_ImageSpanAlignment>(test),
        };
        auto peer = accessor_->ctor(&value);
        ASSERT_TRUE(peer->span->GetImageAttribute());
        ASSERT_FALSE(peer->span->GetImageAttribute()->verticalAlign);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: ctorTestObjectFitValidValues
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestObjectFitValidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureObjectFitValidValues) {
        Ark_ImageAttachmentInterface value {
            .objectFit = ArkValue<Opt_ImageFit>(test),
        };
        auto peer = accessor_->ctor(&value);
        ASSERT_TRUE(peer->span->GetImageAttribute());
        ASSERT_TRUE(peer->span->GetImageAttribute()->objectFit);
        EXPECT_EQ(peer->span->GetImageAttribute()->objectFit, expected);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: ctorTestObjectFitInvalidValues
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestObjectFitInvalidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureObjectFitInvalidValues) {
        Ark_ImageAttachmentInterface value {
            .objectFit = ArkValue<Opt_ImageFit>(test),
        };
        auto peer = accessor_->ctor(&value);
        ASSERT_TRUE(peer->span->GetImageAttribute());
        ASSERT_FALSE(peer->span->GetImageAttribute()->objectFit);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: ctorTestImageStyleMargins
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestImageStyleOptional, TestSize.Level1)
{
    const Ark_ImageAttachmentInterface value {
        .layoutStyle = getImageLayoutStyleOptional(),
    };
    auto peer = accessor_->ctor(&value);
    ASSERT_TRUE(peer->span->GetImageAttribute());
    ASSERT_FALSE(peer->span->GetImageAttribute()->marginProp);
    ASSERT_FALSE(peer->span->GetImageAttribute()->paddingProp);
    ASSERT_FALSE(peer->span->GetImageAttribute()->borderRadius);
    accessor_->destroyPeer(peer);
};

/**
 * @tc.name: ctorTestImageStyleLengthMetrics
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestImageStyleLengthMetrics, TestSize.Level1)
{
    Ark_ImageAttachmentInterface value {
        .layoutStyle = getImageLayoutStyleLengthMetrics(),
    };
    auto peer = accessor_->ctor(&value);
    ASSERT_TRUE(peer->span->GetImageAttribute());
    ASSERT_TRUE(peer->span->GetImageAttribute()->marginProp);
    EXPECT_EQ(peer->span->GetImageAttribute()->marginProp, MARGIN_PADDING_PROPERTY);
    ASSERT_TRUE(peer->span->GetImageAttribute()->paddingProp);
    EXPECT_EQ(peer->span->GetImageAttribute()->paddingProp, MARGIN_PADDING_PROPERTY);
    ASSERT_TRUE(peer->span->GetImageAttribute()->borderRadius);
    EXPECT_EQ(peer->span->GetImageAttribute()->borderRadius, BORDER_RADIUES_PROPERTY);
    accessor_->destroyPeer(peer);
};

/**
 * @tc.name: ctorTestImageStyleMargins
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, ctorTestImageStyleFilled, TestSize.Level1)
{
    const Ark_ImageAttachmentInterface value {
        .layoutStyle = getImageLayoutStyleFilled(),
    };
    auto peer = accessor_->ctor(&value);
    ASSERT_TRUE(peer->span->GetImageAttribute());
    ASSERT_TRUE(peer->span->GetImageAttribute()->marginProp);
    EXPECT_EQ(peer->span->GetImageAttribute()->marginProp, MARGIN_PADDING_PROPERTY);
    ASSERT_TRUE(peer->span->GetImageAttribute()->paddingProp);
    EXPECT_EQ(peer->span->GetImageAttribute()->paddingProp, MARGIN_PADDING_PROPERTY);
    ASSERT_TRUE(peer->span->GetImageAttribute()->borderRadius);
    EXPECT_EQ(peer->span->GetImageAttribute()->borderRadius, BORDER_RADIUES_PROPERTY);
    accessor_->destroyPeer(peer);
};

/**
 * @tc.name: getValueTest
 * @tc.desc: Check the functionality of getValue
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getValueTest, TestSize.Level1)
{
    std::string imagesSrc = "test";
    RefPtr<PixelMap> pixelMap = CreatePixelMap(imagesSrc);
    auto arkPixelMap = GeneratedModifier::GetPixelMapAccessor()->ctor();
    arkPixelMap->pixelMap = pixelMap;

    Ark_ImageAttachmentInterface value {
        .value = arkPixelMap,
    };
    auto peer = accessor_->ctor(&value);
    auto pixelMapPeerValue = accessor_->getValue(peer);
    ASSERT_TRUE(pixelMapPeerValue);
    EXPECT_EQ(pixelMapPeerValue->pixelMap, pixelMap);
    accessor_->destroyPeer(peer);
    GeneratedModifier::GetPixelMapAccessor()->destroyPeer(arkPixelMap);
}

/**
 * @tc.name: ctorTestSize
 * @tc.desc: Check the functionality of ctor
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getSizeTest, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureDimensionAnyValidValues) {
        Ark_SizeOptions size {
            .width = ArkValue<Opt_Length>(test),
            .height = ArkValue<Opt_Length>(test),
        };
        Ark_ImageAttachmentInterface value {
            .size = ArkValue<Opt_SizeOptions>(size),
        };
        auto peer = accessor_->ctor(&value);
        if (expected.IsNonNegative()) {
            EXPECT_THAT(accessor_->getSize(peer), CompareArkSize(size))
                << "Passed value is: " << input;
        } else {
            Ark_SizeOptions emptySize {
                .width = ArkValue<Opt_Length>(test),
                .height = ArkValue<Opt_Length>(test),
            };
            EXPECT_THAT(accessor_->getSize(peer), CompareArkSize(emptySize))
                << "Passed value is: " << input;;
        }
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: getVerticalAlignValidValues
 * @tc.desc: Check the functionality of getVerticalAlign
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getVerticalAlignTestValidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureVerticalAlignValidValues) {
        Ark_ImageAttachmentInterface value {
            .verticalAlign = ArkValue<Opt_ImageSpanAlignment>(test),
        };
        auto peer = accessor_->ctor(&value);
        EXPECT_EQ(accessor_->getVerticalAlign(peer), test);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: getVerticalAlignInvalidValues
 * @tc.desc: Check the functionality of getVerticalAlign
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getVerticalAlignTestInvalidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureVerticalAlignInvalidValues) {
        Ark_ImageAttachmentInterface value {
            .verticalAlign = ArkValue<Opt_ImageSpanAlignment>(test),
        };
        auto peer = accessor_->ctor(&value);
        EXPECT_EQ(accessor_->getVerticalAlign(peer), INVALID_ENUM_VAL<Ark_ImageSpanAlignment>);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: getObjectFitTestValidValues
 * @tc.desc: Check the functionality of getObjectFit
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getObjectFitTestValidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureObjectFitValidValues) {
        Ark_ImageAttachmentInterface value {
            .objectFit = ArkValue<Opt_ImageFit>(test),
        };
        auto peer = accessor_->ctor(&value);
        EXPECT_EQ(accessor_->getObjectFit(peer), test);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: getObjectFitInvalidValues
 * @tc.desc: Check the functionality of getObjectFit
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getObjectFitTestInvalidValues, TestSize.Level1)
{
    for (auto& [input, test, expected] : testFixtureObjectFitInvalidValues) {
        Ark_ImageAttachmentInterface value {
            .objectFit = ArkValue<Opt_ImageFit>(test),
        };
        auto peer = accessor_->ctor(&value);
        EXPECT_EQ(accessor_->getObjectFit(peer), INVALID_ENUM_VAL<Ark_ImageFit>);
        accessor_->destroyPeer(peer);
    }
}

/**
 * @tc.name: getObjectFitInvalidValues
 * @tc.desc: Check the functionality of getObjectFit
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getLayoutStyleTestOptional, TestSize.Level1)
{
    auto expected = OptConvert<Ark_ImageAttachmentLayoutStyle>(getImageLayoutStyleOptional());
    const Ark_ImageAttachmentInterface value {
        .layoutStyle = getImageLayoutStyleOptional(),
    };
    auto peer = accessor_->ctor(&value);
    auto arkGetValue = accessor_->getLayoutStyle(peer);
    ASSERT_TRUE(expected);
    EXPECT_THAT(arkGetValue.margin, CompareOptMarginsPaddings(expected->margin));
    EXPECT_THAT(arkGetValue.padding, CompareOptMarginsPaddings(expected->padding));
    EXPECT_THAT(arkGetValue.borderRadius, CompareOptBorderRadius(expected->borderRadius));
    accessor_->destroyPeer(peer);
}

/**
 * @tc.name: getObjectFitInvalidValues
 * @tc.desc: Check the functionality of getObjectFit
 * @tc.type: FUNC
 */
HWTEST_F(ImageAttachmentAccessorTest, getLayoutStyleTestFilled, TestSize.Level1)
{
    auto expected = OptConvert<Ark_ImageAttachmentLayoutStyle>(getImageLayoutStyleFilled());
    const Ark_ImageAttachmentInterface value {
        .layoutStyle = getImageLayoutStyleFilled(),
    };
    auto peer = accessor_->ctor(&value);
    auto arkGetValue = accessor_->getLayoutStyle(peer);
    ASSERT_TRUE(expected);
    EXPECT_THAT(arkGetValue.margin, CompareOptMarginsPaddings(expected->margin));
    EXPECT_THAT(arkGetValue.padding, CompareOptMarginsPaddings(expected->padding));
    EXPECT_THAT(arkGetValue.borderRadius, CompareOptBorderRadius(expected->borderRadius));
    accessor_->destroyPeer(peer);
}
} // namespace OHOS::Ace::NG
