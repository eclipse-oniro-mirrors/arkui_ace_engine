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
#include <vector>

#include "gtest/gtest.h"

#include "base/resource/shared_image_manager.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
class SharedImageManagerTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};
} // namespace

/**
 * @tc.name: GetFormSharedImageCacheThreshold_001
 * @tc.desc: Verify the default shared image cache threshold equals DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD.
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, GetFormSharedImageCacheThreshold_001, TestSize.Level1)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    EXPECT_EQ(manager->GetImageCacheThreshold(), DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD);
}

/**
 * @tc.name: SetImageCacheThreshold_001
 * @tc.desc: Verify SetImageCacheThreshold changes the threshold value.
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, SetImageCacheThreshold_001, TestSize.Level1)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    EXPECT_EQ(manager->GetImageCacheThreshold(), DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD);
    manager->SetImageCacheThreshold(FORM_MULTI_IMAGE_CACHE_THRESHOLD);
    EXPECT_EQ(manager->GetImageCacheThreshold(), FORM_MULTI_IMAGE_CACHE_THRESHOLD);
    manager->SetImageCacheThreshold(DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD);
    EXPECT_EQ(manager->GetImageCacheThreshold(), DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD);
}

/**
 * @tc.name: SharedImageManagerAddSharedImage_001
 * @tc.desc: Add a new shared image and verify it is stored in the map.
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, SharedImageManagerAddSharedImage_001, TestSize.Level1)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    std::vector<uint8_t> data = { 0x01, 0x02, 0x03 };
    manager->AddSharedImage("img1", std::vector<uint8_t>(data));
    const auto& map = manager->GetSharedImageMap();
    EXPECT_EQ(map.size(), 1u);
    EXPECT_NE(map.find("img1"), map.end());
    EXPECT_EQ(map.at("img1"), data);
}

/**
 * @tc.name: SharedImageManagerAddSharedImage_002
 * @tc.desc: Add an existing image again and verify size unchanged and content updated.
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, SharedImageManagerAddSharedImage_002, TestSize.Level1)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    std::vector<uint8_t> data1 = { 0x01 };
    std::vector<uint8_t> data2 = { 0x02, 0x03 };
    manager->AddSharedImage("img1", std::vector<uint8_t>(data1));
    manager->AddSharedImage("img1", std::vector<uint8_t>(data2));
    const auto& map = manager->GetSharedImageMap();
    EXPECT_EQ(map.size(), 1u);
    EXPECT_EQ(map.at("img1"), data2);
}

/**
 * @tc.name: SharedImageManagerAddSharedImage_003
 * @tc.desc: Add up to cache threshold distinct images and verify size reaches threshold.
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, SharedImageManagerAddSharedImage_003, TestSize.Level1)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    for (int i = 0; i < DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD; ++i) {
        std::string name = "img" + std::to_string(i);
        manager->AddSharedImage(name, std::vector<uint8_t>{ static_cast<uint8_t>(i) });
    }
    EXPECT_EQ(manager->GetSharedImageMap().size(),
        static_cast<size_t>(DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD));
}

/**
 * @tc.name: SharedImageManagerAddSharedImage_004
 * @tc.desc: Add threshold+1 images at default threshold (exceed boundary, no crash).
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, SharedImageManagerAddSharedImage_004, TestSize.Level2)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    int32_t exceedCount = DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD + 1;
    for (int i = 0; i < exceedCount; ++i) {
        std::string name = "img" + std::to_string(i);
        manager->AddSharedImage(name, std::vector<uint8_t>{ static_cast<uint8_t>(i) });
    }
    EXPECT_EQ(manager->GetSharedImageMap().size(), static_cast<size_t>(exceedCount));
}

/**
 * @tc.name: SharedImageManagerAddSharedImage_005
 * @tc.desc: Add up to multi image cache threshold images at threshold 30 (boundary, no exceed).
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, SharedImageManagerAddSharedImage_005, TestSize.Level1)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    manager->SetImageCacheThreshold(FORM_MULTI_IMAGE_CACHE_THRESHOLD);
    for (int i = 0; i < FORM_MULTI_IMAGE_CACHE_THRESHOLD; ++i) {
        std::string name = "img" + std::to_string(i);
        manager->AddSharedImage(name, std::vector<uint8_t>{ static_cast<uint8_t>(i) });
    }
    EXPECT_EQ(manager->GetSharedImageMap().size(),
        static_cast<size_t>(FORM_MULTI_IMAGE_CACHE_THRESHOLD));
}

/**
 * @tc.name: SharedImageManagerAddSharedImage_006
 * @tc.desc: Add threshold+1 images at threshold 30 (exceed boundary, no crash).
 * @tc.type: FUNC
 */
HWTEST_F(SharedImageManagerTest, SharedImageManagerAddSharedImage_006, TestSize.Level2)
{
    auto taskExecutor = AceType::MakeRefPtr<MockTaskExecutor>(true);
    auto manager = AceType::MakeRefPtr<SharedImageManager>(taskExecutor);
    manager->SetImageCacheThreshold(FORM_MULTI_IMAGE_CACHE_THRESHOLD);
    int32_t exceedCount = FORM_MULTI_IMAGE_CACHE_THRESHOLD + 1;
    for (int i = 0; i < exceedCount; ++i) {
        std::string name = "img" + std::to_string(i);
        manager->AddSharedImage(name, std::vector<uint8_t>{ static_cast<uint8_t>(i) });
    }
    EXPECT_EQ(manager->GetSharedImageMap().size(), static_cast<size_t>(exceedCount));
}
} // namespace OHOS::Ace
