/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/implementation/pixel_map_peer.h"
#include "arkoala_api_generated.h"

#include "core/components_ng/pattern/image_animator/image_animator_event_hub.h"
#include "core/components_ng/pattern/image_animator/image_animator_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_IMAGES_NAME = "images";
    const auto ATTRIBUTE_IMAGES_DEFAULT_VALUE = "[]";
    const auto ATTRIBUTE_IMAGES_SRC_NAME = "src";
    const auto ATTRIBUTE_IMAGES_SRC_TEST_VALUE = "test";
    const std::string IMAGES_OK_STR = "img_public_ok";
    const auto ATTRIBUTE_IMAGES_LEFT_NAME = "left";
    const auto ATTRIBUTE_IMAGES_LEFT_TEST_VALUE = "100.00vp";
    const auto ATTRIBUTE_IMAGES_TOP_NAME = "top";
    const auto ATTRIBUTE_IMAGES_TOP_TEST_VALUE = "100.00vp";
    const auto ATTRIBUTE_IMAGES_WIDTH_NAME = "width";
    const auto ATTRIBUTE_IMAGES_WIDTH_TEST_VALUE = "autocalc";
    const auto ATTRIBUTE_IMAGES_HEIGHT_NAME = "height";
    const auto ATTRIBUTE_IMAGES_HEIGHT_TEST_VALUE = "100.00px";
    const auto ATTRIBUTE_IMAGES_DURATION_NAME = "duration";
    const auto ATTRIBUTE_IMAGES_DURATION_TEST_VALUE = "1";
    const auto ATTRIBUTE_ITERATIONS_NAME = "iterations";
    const auto ATTRIBUTE_ITERATIONS_DEFAULT_VALUE = "1";

    const auto ATTRIBUTE_SIZE_TEST_VALUE = 100;
} // namespace

class ImageAnimatorModifierTest : public ModifierTestBase<
    GENERATED_ArkUIImageAnimatorModifier,
    &GENERATED_ArkUINodeModifiers::getImageAnimatorModifier,
    GENERATED_ARKUI_IMAGE_ANIMATOR> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();
        AddResource(IMAGES_OK_STR, ATTRIBUTE_IMAGES_SRC_TEST_VALUE);
    }

    void InitPattern(OHOS::Ace::RefPtr<ImageAnimatorPattern> pattern)
    {
        std::vector<ImageProperties> images;
        void* voidPtr = static_cast<void*>(new char[0]);
        ImageProperties image;
        image.src = "test";
        image.pixelMap = PixelMap::CreatePixelMap(voidPtr);
        image.width = ATTRIBUTE_SIZE_TEST_VALUE;
        image.height = ATTRIBUTE_SIZE_TEST_VALUE;
        image.top = 0;
        image.left = 0;
        image.duration = 1;
        images.emplace_back(image);
        pattern->SetImages(std::move(images));
    }

    RefPtr<PixelMap> CreatePixelMap(const std::string& src);
};

RefPtr<PixelMap> ImageAnimatorModifierTest::CreatePixelMap(const std::string& src)
{
    auto voidChar = const_cast<char*>(src.c_str());
    void* voidPtr = static_cast<void*>(voidChar);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    return pixelMap;
}



/*
 * @tc.name: setImagesTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setImagesTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_IMAGES_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_DEFAULT_VALUE);
}

/*
 * @tc.name: setImagesTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setImagesTestValidValues, TestSize.Level1)
{
    auto array = new Ark_ImageFrameInfo[] {
        {
            .src = Converter::ArkUnion<Ark_Union_String_Resource_PixelMap, Ark_String>(
                ATTRIBUTE_IMAGES_SRC_TEST_VALUE),
            .width = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("auto"),
            .height = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("100px"),
            .top = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(ATTRIBUTE_SIZE_TEST_VALUE),
            .left = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(ATTRIBUTE_SIZE_TEST_VALUE),
            .duration = Converter::ArkValue<Opt_Number>(1),
        },
        {
            .src = Converter::ArkUnion<Ark_Union_String_Resource_PixelMap, Ark_Resource>(
                CreateResource(IMAGES_OK_STR.c_str(), Converter::ResourceType::STRING)),
            .width = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("auto"),
            .height = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("100px"),
            .top = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(ATTRIBUTE_SIZE_TEST_VALUE),
            .left = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(ATTRIBUTE_SIZE_TEST_VALUE),
            .duration = Converter::ArkValue<Opt_Number>(1),
        },
    };
    Array_ImageFrameInfo initValueImages = { .array = array, .length = 2 };

    modifier_->setImages(node_, &initValueImages);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    std::unique_ptr<JsonValue> resultImages = GetAttrValue<std::unique_ptr<JsonValue>>(
        jsonValue, ATTRIBUTE_IMAGES_NAME);
    
    if (resultImages->IsArray()) {
        int32_t count = resultImages->GetArraySize();
        for (int i = 0; i < count; i++) {
            auto resultImage = resultImages->GetArrayItem(i);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_SRC_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_SRC_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_LEFT_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_LEFT_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_TOP_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_TOP_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_WIDTH_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_WIDTH_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_HEIGHT_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_HEIGHT_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_DURATION_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_DURATION_TEST_VALUE);
        }
    }
}

/*
 * @tc.name: setImagesTestPixelMap
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setImagesTestPixelMap, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<PixelMap> pixelMap = CreatePixelMap(ATTRIBUTE_IMAGES_NAME);
    PixelMapPeer pixelMapPeer;
    pixelMapPeer.pixelMap = pixelMap;
    Ark_Materialized px;
    px.ptr = &pixelMapPeer;
    auto array = new Ark_ImageFrameInfo[] {
        {
            .src = Converter::ArkUnion<Ark_Union_String_Resource_PixelMap, Ark_PixelMap>(px),
            .width = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("auto"),
            .height = Converter::ArkUnion<Opt_Union_Number_String, Ark_String>("100px"),
            .top = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(ATTRIBUTE_SIZE_TEST_VALUE),
            .left = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(ATTRIBUTE_SIZE_TEST_VALUE),
            .duration = Converter::ArkValue<Opt_Number>(1),
        },
    };
    Array_ImageFrameInfo initValueImages = { .array = array, .length = 1 };

    modifier_->setImages(node_, &initValueImages);

    auto imageAnimatorPattern_ = frameNode->GetPattern<ImageAnimatorPattern>();
    EXPECT_NE(imageAnimatorPattern_, nullptr);
    EXPECT_EQ(imageAnimatorPattern_->GetImagesSize(), 1);
    auto imageProperties = imageAnimatorPattern_->GetImage(0);
    EXPECT_EQ(imageProperties.pixelMap, pixelMap);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    std::unique_ptr<JsonValue> resultImages = GetAttrValue<std::unique_ptr<JsonValue>>(
        jsonValue, ATTRIBUTE_IMAGES_NAME);
    if (resultImages->IsArray()) {
        int32_t count = resultImages->GetArraySize();
        for (int i = 0; i < count; i++) {
            auto resultImage = resultImages->GetArrayItem(i);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_LEFT_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_LEFT_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_TOP_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_TOP_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_WIDTH_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_WIDTH_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_HEIGHT_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_HEIGHT_TEST_VALUE);
            resultStr = GetAttrValue<std::string>(resultImage, ATTRIBUTE_IMAGES_DURATION_NAME);
            EXPECT_EQ(resultStr, ATTRIBUTE_IMAGES_DURATION_TEST_VALUE);
        }
    }
}

/*
 * @tc.name: setIterationsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setIterationsTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ITERATIONS_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_ITERATIONS_DEFAULT_VALUE);
}

// Valid values for attribute 'iterations' of method 'iterations'
static std::vector<std::tuple<std::string, Ark_Number, std::string>> iterationsIterationsValidValues = {
    { "-1", Converter::ArkValue<Ark_Number>(-1), "-1" },
    { "0", Converter::ArkValue<Ark_Number>(0), "0" },
    { "1", Converter::ArkValue<Ark_Number>(1), "1" },
    { "10", Converter::ArkValue<Ark_Number>(10), "10" },
    { "50.5", Converter::ArkValue<Ark_Number>(50.5), "50" },
};

/*
 * @tc.name: setIterationsTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setIterationsTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;

    // Initial setup
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<ImageAnimatorPattern>();
    ASSERT_TRUE(pattern);
    InitPattern(pattern);

    // Verifying attribute's  values
    for (auto& [input, value, expected]: iterationsIterationsValidValues) {
        modifier_->setIterations(node_, &value);
        pattern->OnModifyDone();
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ITERATIONS_NAME);
        EXPECT_EQ(resultStr, expected) << "Passed value is: " << input;
    }
}

// Invalid values for attribute 'iterations' of method 'iterations'
static std::vector<std::tuple<std::string, Ark_Number>> iterationsIterationsInvalidValues = {
    { "1", Converter::ArkValue<Ark_Number>(-2) },
    { "1", Converter::ArkValue<Ark_Number>(-55.5) },
};

/*
 * @tc.name: setIterationsTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setIterationsTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    std::string resultStr;
    std::string expectedStr;

    // Initial setup
    auto frameNode = reinterpret_cast<FrameNode *>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<ImageAnimatorPattern>();
    ASSERT_TRUE(pattern);
    InitPattern(pattern);

    // Verifying attribute's  values
    for (auto& [imput, value]: iterationsIterationsInvalidValues) {
        modifier_->setIterations(node_, &value);
        pattern->OnModifyDone();
        jsonValue = GetJsonValue(node_);
        resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_ITERATIONS_NAME);
        expectedStr = ATTRIBUTE_ITERATIONS_DEFAULT_VALUE;
        EXPECT_EQ(resultStr, expectedStr) << "Passed value is: " << imput;
    }
}

/*
 * @tc.name: setOnStartTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setOnStartTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnStart, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ImageAnimatorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    static constexpr int32_t contextId = 123;
    static bool isCalled = false;
    auto checkCallback = [](const Ark_Int32 resourceId) {
        isCalled = true;
        EXPECT_EQ(resourceId, contextId);
    };
    // setup the callback object via C-API
    Callback_Void arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    modifier_->setOnStart(node_, &arkCallback);

    isCalled = false;
    eventHub->GetStartEvent()();
    ASSERT_TRUE(isCalled);
}

/*
 * @tc.name: setOnPauseTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setOnPauseTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnPause, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ImageAnimatorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    static constexpr int32_t contextId = 123;
    static bool isCalled = false;
    auto checkCallback = [](const Ark_Int32 resourceId) {
        isCalled = true;
        EXPECT_EQ(resourceId, contextId);
    };
    // setup the callback object via C-API
    Callback_Void arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    modifier_->setOnPause(node_, &arkCallback);

    isCalled = false;
    eventHub->GetPauseEvent()();
    ASSERT_TRUE(isCalled);
}

/*
 * @tc.name: setOnRepeatTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setOnRepeatTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnRepeat, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ImageAnimatorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    static constexpr int32_t contextId = 123;
    static bool isCalled = false;
    auto checkCallback = [](const Ark_Int32 resourceId) {
        isCalled = true;
        EXPECT_EQ(resourceId, contextId);
    };
    // setup the callback object via C-API
    Callback_Void arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    modifier_->setOnRepeat(node_, &arkCallback);

    isCalled = false;
    eventHub->GetRepeatEvent()();
    ASSERT_TRUE(isCalled);
}

/*
 * @tc.name: setOnCancelTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setOnCancelTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnCancel, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ImageAnimatorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    static constexpr int32_t contextId = 123;
    static bool isCalled = false;
    auto checkCallback = [](const Ark_Int32 resourceId) {
        isCalled = true;
        EXPECT_EQ(resourceId, contextId);
    };
    // setup the callback object via C-API
    Callback_Void arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    modifier_->setOnCancel(node_, &arkCallback);

    isCalled = false;
    eventHub->GetCancelEvent()();
    ASSERT_TRUE(isCalled);
}

/*
 * @tc.name: setOnFinishTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnimatorModifierTest, setOnFinishTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnFinish, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto eventHub = frameNode->GetEventHub<ImageAnimatorEventHub>();
    ASSERT_NE(eventHub, nullptr);

    static constexpr int32_t contextId = 123;
    static bool isCalled = false;
    auto checkCallback = [](const Ark_Int32 resourceId) {
        isCalled = true;
        EXPECT_EQ(resourceId, contextId);
    };
    // setup the callback object via C-API
    Callback_Void arkCallback = Converter::ArkValue<Callback_Void>(checkCallback, contextId);
    modifier_->setOnFinish(node_, &arkCallback);

    isCalled = false;
    eventHub->GetStopEvent()();
    ASSERT_TRUE(isCalled);
}
} // namespace OHOS::Ace::NG
