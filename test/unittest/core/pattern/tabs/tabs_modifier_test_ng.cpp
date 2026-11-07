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
#include "tabs_test_ng.h"

#include "base/memory/type_info_base.h"
#include "core/components/tab_bar/tabs_event.h"
#include "core/interfaces/native/node/tabs_modifier.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TEST_INDEX_ZERO = 0;
constexpr int32_t TEST_INDEX_VALID = 2;
constexpr int32_t TEST_INDEX_NEGATIVE = -5;
constexpr int32_t TEST_CACHED_MAX_COUNT = 3;
constexpr int32_t TEST_INVALID_CACHE_MODE = 99;
} // namespace

class TabsModifierTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TabsTestNg::SetUpTestSuite();
    }

    static void TearDownTestSuite()
    {
        TabsTestNg::TearDownTestSuite();
    }
};

/**
 * @tc.name: SetTabsIndexTest
 * @tc.desc: Test ArkUITabsModifier.setTabsIndex delegates to TabsModelNG::SetIndex(FrameNode*, int32_t)
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetTabsIndexTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setTabsIndex, nullptr);

    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);
    ASSERT_NE(swiperLayoutProperty_, nullptr);

    modifier->setTabsIndex(reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(frameNode_)), TEST_INDEX_VALID);
    EXPECT_EQ(layoutProperty_->GetIndex().value_or(-1), TEST_INDEX_VALID);
    EXPECT_EQ(swiperLayoutProperty_->GetIndex().value_or(-1), TEST_INDEX_VALID);
}

/**
 * @tc.name: SetTabsIndexNegativeTest
 * @tc.desc: Test ArkUITabsModifier.setTabsIndex clamps a negative index to zero
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetTabsIndexNegativeTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setTabsIndex, nullptr);

    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);
    ASSERT_NE(swiperLayoutProperty_, nullptr);

    modifier->setTabsIndex(reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(frameNode_)), TEST_INDEX_NEGATIVE);
    EXPECT_EQ(layoutProperty_->GetIndex().value_or(-1), TEST_INDEX_ZERO);
    EXPECT_EQ(swiperLayoutProperty_->GetIndex().value_or(-1), TEST_INDEX_ZERO);
}

/**
 * @tc.name: SetTabsIndexNullNodeTest
 * @tc.desc: Test ArkUITabsModifier.setTabsIndex with a null node does not crash
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetTabsIndexNullNodeTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setTabsIndex, nullptr);

    modifier->setTabsIndex(nullptr, TEST_INDEX_VALID);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SetTabsOnChangeEventTest
 * @tc.desc: Test ArkUITabsModifier.setTabsOnChangeEvent delegates to
 *           TabsModelNG::SetOnChangeEvent(FrameNode*, std::function<void(const BaseEventInfo*)>&&)
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetTabsOnChangeEventTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setTabsOnChangeEvent, nullptr);

    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    int32_t currentIndex = -1;
    std::function<void(const BaseEventInfo*)> callback = [&currentIndex](const BaseEventInfo* info) {
        const auto* tabInfo = TypeInfoHelper::DynamicCast<TabContentChangeEvent>(info);
        ASSERT_NE(tabInfo, nullptr);
        currentIndex = tabInfo->GetIndex();
    };
    modifier->setTabsOnChangeEvent(
        reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(frameNode_)), reinterpret_cast<void*>(&callback));

    auto changeEvent = pattern_->GetIndexChangeEvent();
    ASSERT_NE(changeEvent, nullptr);
    (*changeEvent)(TEST_INDEX_VALID);
    EXPECT_EQ(currentIndex, TEST_INDEX_VALID);
}

/**
 * @tc.name: SetTabsOnChangeEventNullCallbackTest
 * @tc.desc: Test ArkUITabsModifier.setTabsOnChangeEvent with a null callback clears the js callback
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetTabsOnChangeEventNullCallbackTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setTabsOnChangeEvent, nullptr);

    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    int32_t currentIndex = -1;
    std::function<void(const BaseEventInfo*)> callback = [&currentIndex](const BaseEventInfo* info) {
        const auto* tabInfo = TypeInfoHelper::DynamicCast<TabContentChangeEvent>(info);
        ASSERT_NE(tabInfo, nullptr);
        currentIndex = tabInfo->GetIndex();
    };
    auto nodeHandle = reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(frameNode_));
    modifier->setTabsOnChangeEvent(nodeHandle, reinterpret_cast<void*>(&callback));
    auto changeEvent = pattern_->GetIndexChangeEvent();
    ASSERT_NE(changeEvent, nullptr);
    (*changeEvent)(TEST_INDEX_VALID);
    EXPECT_EQ(currentIndex, TEST_INDEX_VALID);

    currentIndex = -1;
    modifier->setTabsOnChangeEvent(nodeHandle, nullptr);
    auto clearedEvent = pattern_->GetIndexChangeEvent();
    ASSERT_NE(clearedEvent, nullptr);
    (*clearedEvent)(TEST_INDEX_VALID);
    EXPECT_EQ(currentIndex, -1);
}

/**
 * @tc.name: SetTabsOnChangeEventNullNodeTest
 * @tc.desc: Test ArkUITabsModifier.setTabsOnChangeEvent with a null node does not crash
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetTabsOnChangeEventNullNodeTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setTabsOnChangeEvent, nullptr);

    modifier->setTabsOnChangeEvent(nullptr, nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SetCachedMaxCountForJsTest
 * @tc.desc: Test ArkUITabsModifier.setCachedMaxCountForJs delegates to
 *           TabsModelNG::SetCachedMaxCountForJs(FrameNode*, std::optional<int32_t>, TabsCacheMode)
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetCachedMaxCountForJsTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setCachedMaxCountForJs, nullptr);

    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    modifier->setCachedMaxCountForJs(reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(frameNode_)),
        TEST_CACHED_MAX_COUNT, static_cast<ArkUI_Int32>(TabsCacheMode::CACHE_LATEST_SWITCHED));
    EXPECT_EQ(layoutProperty_->GetCachedMaxCount().value_or(-1), TEST_CACHED_MAX_COUNT);
    EXPECT_EQ(layoutProperty_->GetCacheMode().value_or(TabsCacheMode::CACHE_BOTH_SIDE),
        TabsCacheMode::CACHE_LATEST_SWITCHED);
}

/**
 * @tc.name: SetCachedMaxCountForJsInvalidModeTest
 * @tc.desc: Test ArkUITabsModifier.setCachedMaxCountForJs falls back to CACHE_BOTH_SIDE for an invalid mode
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetCachedMaxCountForJsInvalidModeTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setCachedMaxCountForJs, nullptr);

    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    modifier->setCachedMaxCountForJs(reinterpret_cast<ArkUINodeHandle>(AceType::RawPtr(frameNode_)),
        TEST_CACHED_MAX_COUNT, TEST_INVALID_CACHE_MODE);
    EXPECT_EQ(layoutProperty_->GetCachedMaxCount().value_or(-1), TEST_CACHED_MAX_COUNT);
    EXPECT_EQ(layoutProperty_->GetCacheMode().value_or(TabsCacheMode::CACHE_LATEST_SWITCHED),
        TabsCacheMode::CACHE_BOTH_SIDE);
}

/**
 * @tc.name: SetCachedMaxCountForJsNullNodeTest
 * @tc.desc: Test ArkUITabsModifier.setCachedMaxCountForJs with a null node does not crash
 * @tc.type: FUNC
 */
HWTEST_F(TabsModifierTestNg, SetCachedMaxCountForJsNullNodeTest, TestSize.Level1)
{
    auto modifier = NodeModifier::GetTabsModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->setCachedMaxCountForJs, nullptr);

    modifier->setCachedMaxCountForJs(nullptr, TEST_CACHED_MAX_COUNT,
        static_cast<ArkUI_Int32>(TabsCacheMode::CACHE_BOTH_SIDE));
    EXPECT_TRUE(true);
}
} // namespace OHOS::Ace::NG
