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

#undef UNITEST_FRIEND_CLASS
#define UNITEST_FRIEND_CLASS friend class MediaCachedImageModifierTest

#include <gtest/gtest.h>
#include <tuple>

#include "arkoala_api_generated.h"
#include "generated/test_fixtures.h"
#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/components_ng/pattern/image/image_event_hub.h"
#include "core/interfaces/native/implementation/animated_drawable_descriptor_peer.h"
#include "core/interfaces/native/implementation/pixel_map_peer.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::NG::Converter;

namespace OHOS::Ace::NG {
namespace {
const auto ATTRIBUTE_SRC_NAME("src");
const auto ATTRIBUTE_SRC_DEFAULT_VALUE("");

using imageOptions = Ark_Union_PixelMap_ResourceStr_DrawableDescriptor_ASTCResource;
} // namespace

class MediaCachedImageModifierTest
    : public ModifierTestBase<GENERATED_ArkUIMediaCachedImageModifier,
          &GENERATED_ArkUINodeModifiers::getMediaCachedImageModifier, GENERATED_ARKUI_MEDIA_CACHED_IMAGE> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();
        for (auto&& res : Fixtures::resourceInitTable) {
            AddResource(std::get<0>(res), std::get<2>(res)); // 2 - index of resource
            AddResource(std::get<1>(res), std::get<2>(res)); // 2 - index of resource
        }
    }

    RefPtr<PixelMap> CreatePixelMap(std::string& src);

    RefPtr<Animator> GetMediaAnimatorFromPattern(FrameNode* node)
    {
        CHECK_NULL_RETURN(node, nullptr);
        auto pattern = node->GetPattern<ImagePattern>();
        CHECK_NULL_RETURN(pattern, nullptr);
        return pattern->animator_;
    }
    std::vector<ImageProperties> GetImagesFromPattern(FrameNode* node)
    {
        std::vector<ImageProperties> empty;
        CHECK_NULL_RETURN(node, empty);
        auto pattern = node->GetPattern<ImagePattern>();
        CHECK_NULL_RETURN(pattern, empty);
        return pattern->images_;
    }
};

RefPtr<PixelMap> MediaCachedImageModifierTest::CreatePixelMap(std::string& src)
{
    auto voidChar = src.data();
    void* voidPtr = static_cast<void*>(voidChar);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    return pixelMap;
}

/*
 * @tc.name: setImageOptions0TestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MediaCachedImageModifierTest, setMediaCachedImageOptionsTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SRC_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_SRC_DEFAULT_VALUE) << "Default value for attribute 'src'";

    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_TRUE(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_TRUE(layoutProperty);
    auto sourceInfo = layoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo);
    ASSERT_FALSE(sourceInfo->GetPixmap());
}

/*
 * @tc.name: setMediaCachedImageOptionsTest
 * @tc.desc: Check functionality of MediaCachedImageInterface.setMediaCachedImageOptions
 * @tc.type: FUNC
 */
HWTEST_F(MediaCachedImageModifierTest, setMediaCachedImageOptionsTestPixelMap, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_TRUE(frameNode);
    std::string imagesSrc = "test";
    RefPtr<PixelMap> pixelMap = CreatePixelMap(imagesSrc);
    PixelMapPeer pixelMapPeer;
    pixelMapPeer.pixelMap = pixelMap;

    auto arkValue = ArkUnion<imageOptions, Ark_PixelMap>(&pixelMapPeer);
    modifier_->setMediaCachedImageOptions(node_, &arkValue);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_TRUE(layoutProperty);
    auto sourceInfo = layoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo);
    EXPECT_EQ(sourceInfo->GetPixmap(), pixelMap);
}

/*
 * @tc.name: setMediaCachedImageOptionsTest
 * @tc.desc: Check functionality of MediaCachedImageInterface.setMediaCachedImageOptions
 * @tc.type: FUNC
 */
HWTEST_F(MediaCachedImageModifierTest, setMediaCachedImageOptionsTestResouce, TestSize.Level1)
{
    ASSERT_NE(modifier_->setMediaCachedImageOptions, nullptr);
    imageOptions initValueSrc;

    auto checkValue = [this, &initValueSrc](
                          const std::string& input, const std::string& expectedStr, const imageOptions& value) {
        imageOptions inputValueSrc = initValueSrc;
        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueSrc = value;
        modifier_->setMediaCachedImageOptions(node, &inputValueSrc);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SRC_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, expectedStr)
            << "Input value is: " << input << ", method: setMediaCachedImageOptions, attribute: src";
    };

    for (auto& [input, value, expected] : Fixtures::testFixtureStringValidValues) {
        checkValue(
            input, expected, ArkUnion<imageOptions, Ark_ResourceStr>(ArkUnion<Ark_ResourceStr, Ark_String>(value)));
    }
    for (auto& [input, value, expected] : Fixtures::testFixtureStringResValidValues) {
        checkValue(
            input, expected, ArkUnion<imageOptions, Ark_ResourceStr>(ArkUnion<Ark_ResourceStr, Ark_Resource>(value)));
    }
}

/*
 * @tc.name: setMediaCachedImageOptionsTest
 * @tc.desc: Check functionality of MediaCachedImageInterface.setMediaCachedImageOptions
 * @tc.type: FUNC
 */
HWTEST_F(MediaCachedImageModifierTest, setMediaCachedImageOptionsTestDrawableDescriptor, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_TRUE(frameNode);
    std::string imagesSrc = "test";
    RefPtr<PixelMap> pixelMap = CreatePixelMap(imagesSrc);
    auto peer = PeerUtils::CreatePeer<DrawableDescriptorPeer>(pixelMap);

    auto drawableUnion = ArkUnion<imageOptions, Ark_DrawableDescriptor>(peer);
    modifier_->setMediaCachedImageOptions(node_, &drawableUnion);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_TRUE(layoutProperty);
    auto sourceInfo = layoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo);
    EXPECT_EQ(sourceInfo->GetPixmap(), pixelMap);
}

/*
 * @tc.name: setMediaCachedImageOptionsTestAnimation
 * @tc.desc: Check functionality of MediaCachedImageInterface.setMediaCachedImageOptions
 * @tc.type: FUNC
 */
HWTEST_F(MediaCachedImageModifierTest, setMediaCachedImageOptionsTestAnimation, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_TRUE(frameNode);
    std::string imagesSrc = "test";
    auto duration = 100;
    auto iterations = 10;
    auto pixelMap1 = CreatePixelMap(imagesSrc);
    auto pixelMap2 = CreatePixelMap(imagesSrc);
    const vector<RefPtr<PixelMap>> pixelMapArray = { pixelMap1, pixelMap2 };
    auto animatedPeer =
        PeerUtils::CreatePeer<AnimatedDrawableDescriptorPeer>(pixelMapArray, duration, iterations);
    ASSERT_TRUE(animatedPeer);
    auto baseDescriptor = AceType::DynamicCast<DrawableDescriptorPeer>(animatedPeer);
    ASSERT_TRUE(baseDescriptor);
    auto animationUnion = ArkUnion<imageOptions, Ark_DrawableDescriptor>(baseDescriptor);
    modifier_->setMediaCachedImageOptions(node_, &animationUnion);
    auto mediaAnimator = GetMediaAnimatorFromPattern(frameNode);
    ASSERT_TRUE(mediaAnimator);
    EXPECT_EQ(mediaAnimator->GetDuration(), duration);
    EXPECT_EQ(mediaAnimator->GetIteration(), iterations);
    auto images = GetImagesFromPattern(frameNode);
    ASSERT_EQ(images.size(), pixelMapArray.size());
    for (auto i = 0; i < images.size(); i++) {
        EXPECT_EQ(images[i].pixelMap, pixelMapArray[i]);
    }
}

/*
 * @tc.name: setMediaCachedImageOptionsTest
 * @tc.desc: Check functionality of MediaCachedImageInterface.setMediaCachedImageOptions
 * @tc.type: FUNC
 */
HWTEST_F(MediaCachedImageModifierTest, setMediaCachedImageOptionsTestASTCResource, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_TRUE(frameNode);
    ConvContext ctx;
    std::vector<std::string> vecSrc { "test" };
    Ark_ASTCResource resource { .column = ArkValue<Ark_Number>(frameNode->GetId()),
        .sources = ArkValue<Array_String>(vecSrc, &ctx) };
    auto arkValue = ArkUnion<imageOptions, Ark_ASTCResource>(resource);
    modifier_->setMediaCachedImageOptions(node_, &arkValue);

    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;

    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SRC_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_SRC_DEFAULT_VALUE) << "Default value for attribute 'src'";

    auto layoutProperty = frameNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_TRUE(layoutProperty);
    auto sourceInfo = layoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo);
    ASSERT_FALSE(sourceInfo->GetPixmap());
}

/*
 * @tc.name: setMediaCachedImageOptionsTestSrcInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(MediaCachedImageModifierTest, setMediaCachedImageOptionsTestSrcInvalidValues, TestSize.Level1)
{
    imageOptions initValueSrc;

    // Initial setup
    initValueSrc = ArkUnion<imageOptions, Ark_ResourceStr>(
        ArkUnion<Ark_ResourceStr, Ark_String>(std::get<1>(Fixtures::testFixtureStringValidValues[0])));

    auto checkValue = [this, &initValueSrc](const std::string& input, const imageOptions& value) {
        imageOptions inputValueSrc = initValueSrc;

        // Re-create node for 'options' attribute
        auto node = CreateNode();
        inputValueSrc = value;
        modifier_->setMediaCachedImageOptions(node, &inputValueSrc);
        auto jsonValue = GetJsonValue(node);
        auto resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_SRC_NAME);
        DisposeNode(node);
        EXPECT_EQ(resultStr, ATTRIBUTE_SRC_DEFAULT_VALUE)
            << "Input value is: " << input << ", method: setMediaCachedImageOptions, attribute: src";
    };

    // Check invalid union
    checkValue("invalid union", ArkUnion<imageOptions, Ark_Empty>(nullptr));
    // Check invalid union
    checkValue("invalid union", ArkUnion<imageOptions, Ark_Empty>(nullptr));
}
} // namespace OHOS::Ace::NG