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

#include "test/unittest/core/manager/safe_area_manager_test_ng.h"

namespace OHOS::Ace::NG {
namespace {
constexpr double TITLE_BUTTON_RIGHT_START = DISPLAY_WIDTH - 100.0f;
constexpr double TITLE_BUTTON_RIGHT_END = DISPLAY_WIDTH - 0.0f;
constexpr double TITLE_BUTTON_TOP_START = 0.0f;
constexpr double TITLE_BUTTON_TOP_END = 40.0f;
constexpr double TITLE_BUTTON_WITH_ROOT_RIGHT_END = DISPLAY_WIDTH;

NG::SafeAreaInsets titleButtonArea =
    NG::SafeAreaInsets({}, { TITLE_BUTTON_TOP_START, TITLE_BUTTON_TOP_END },
        { TITLE_BUTTON_RIGHT_START, TITLE_BUTTON_RIGHT_END }, {});

NG::SafeAreaInsets titleButtonAreaWithRoot =
    NG::SafeAreaInsets({}, { TITLE_BUTTON_TOP_START, TITLE_BUTTON_TOP_END },
        { TITLE_BUTTON_RIGHT_START, TITLE_BUTTON_WITH_ROOT_RIGHT_END }, {});
} // namespace

/**
 * @tc.name: TitleButtonStrategyApplyAndQueryTest
 * @tc.desc: ApplyDefaultImmersiveStrategy(AVOID_WINDOW_CONTROLS) records strategy;
 *           IsImmersiveStrategySet returns true (AC-1.1 / VM-1).
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonStrategyApplyAndQueryTest, TestSize.Level0)
{
    EXPECT_FALSE(safeAreaManager_->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_WINDOW_CONTROLS));
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    EXPECT_TRUE(safeAreaManager_->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_WINDOW_CONTROLS));
}

/**
 * @tc.name: TitleButtonStrategyReplaceClearsTest
 * @tc.desc: Replace with empty/other strategy clears AVOID_WINDOW_CONTROLS (AC-1.2 / VM-2).
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonStrategyReplaceClearsTest, TestSize.Level0)
{
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    ASSERT_TRUE(safeAreaManager_->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_WINDOW_CONTROLS));
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_CUTOUT });
    EXPECT_FALSE(safeAreaManager_->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_WINDOW_CONTROLS));
    EXPECT_TRUE(safeAreaManager_->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_CUTOUT));
}

/**
 * @tc.name: TitleButtonStrategyEmptyClearsTest
 * @tc.desc: Empty array clears AVOID_WINDOW_CONTROLS (AC-1.2 / VM-2).
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonStrategyEmptyClearsTest, TestSize.Level0)
{
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    ASSERT_TRUE(safeAreaManager_->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_WINDOW_CONTROLS));
    safeAreaManager_->ApplyDefaultImmersiveStrategy({});
    EXPECT_FALSE(safeAreaManager_->IsImmersiveStrategySet(ImmersiveStrategy::AVOID_WINDOW_CONTROLS));
}

/**
 * @tc.name: TitleButtonUpdateAndGetTypeTest
 * @tc.desc: UpdateTitleButtonSafeArea stores edge-normalized area;
 *           GetTitleButtonSafeArea/GetTitleButtonSafeAreaWithoutProcess return stored value.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonUpdateAndGetTypeTest, TestSize.Level0)
{
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    auto result = safeAreaManager_->GetTitleButtonSafeAreaWithoutProcess();
    EXPECT_EQ(result.right_.start, titleButtonAreaWithRoot.right_.start);
    EXPECT_EQ(result.right_.end, titleButtonAreaWithRoot.right_.end);
    EXPECT_EQ(result.top_.start, titleButtonAreaWithRoot.top_.start);
    EXPECT_EQ(result.top_.end, titleButtonAreaWithRoot.top_.end);
}

/**
 * @tc.name: TitleButtonCheckTest
 * @tc.desc: CheckTitleButtonSafeArea returns true when area differs, false when same.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonCheckTest, TestSize.Level0)
{
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    EXPECT_FALSE(safeAreaManager_->CheckTitleButtonSafeArea(titleButtonArea, rootSize));
    auto differentArea = NG::SafeAreaInsets({}, { 10.0f, 50.0f },
        { DISPLAY_WIDTH - 80.0f, DISPLAY_WIDTH }, {});
    EXPECT_TRUE(safeAreaManager_->CheckTitleButtonSafeArea(differentArea, rootSize));
}

/**
 * @tc.name: TitleButtonGetSafeAreaIncludedTest
 * @tc.desc: Strategy set + titleButtonSafeArea_ updated -> GetSafeArea includes title button area (AC-1.1 / VM-1).
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonGetSafeAreaIncludedTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    auto safeArea = safeAreaManager_->GetSafeArea();
    EXPECT_EQ(safeArea.right_.start, titleButtonAreaWithRoot.right_.start);
    EXPECT_EQ(safeArea.right_.end, titleButtonAreaWithRoot.right_.end);
    EXPECT_EQ(safeArea.top_.start, titleButtonAreaWithRoot.top_.start);
    EXPECT_EQ(safeArea.top_.end, titleButtonAreaWithRoot.top_.end);
}

/**
 * @tc.name: TitleButtonGetSafeAreaExcludedTest
 * @tc.desc: Strategy not set -> GetSafeArea does NOT include title button area (AC-2.4 / VM-8).
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonGetSafeAreaExcludedTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    auto safeArea = safeAreaManager_->GetSafeArea();
    EXPECT_EQ(safeArea.right_.start, SYSTEM_RIGHT_START);
    EXPECT_EQ(safeArea.right_.end, SYSTEM_RIGHT_END);
}

/**
 * @tc.name: TitleButtonGetSafeAreaWithoutCutoutIncludedTest
 * @tc.desc: Strategy set -> GetSafeAreaWithoutCutout includes title button area.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonGetSafeAreaWithoutCutoutIncludedTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    auto safeArea = safeAreaManager_->GetSafeAreaWithoutCutout();
    EXPECT_EQ(safeArea.right_.start, titleButtonAreaWithRoot.right_.start);
    EXPECT_EQ(safeArea.right_.end, titleButtonAreaWithRoot.right_.end);
}

/**
 * @tc.name: TitleButtonGetSafeAreaWithoutCutoutExcludedTest
 * @tc.desc: Strategy not set -> GetSafeAreaWithoutCutout does NOT include title button area.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonGetSafeAreaWithoutCutoutExcludedTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    auto safeArea = safeAreaManager_->GetSafeAreaWithoutCutout();
    EXPECT_EQ(safeArea.right_.start, SYSTEM_RIGHT_START);
    EXPECT_EQ(safeArea.right_.end, SYSTEM_RIGHT_END);
}

/**
 * @tc.name: TitleButtonGetCombinedSafeAreaIncludedTest
 * @tc.desc: Strategy set -> GetCombinedSafeArea(SYSTEM) includes title button area.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonGetCombinedSafeAreaIncludedTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    SafeAreaExpandOpts opt;
    opt.type |= SAFE_AREA_TYPE_SYSTEM;
    auto safeArea = safeAreaManager_->GetCombinedSafeArea(opt);
    EXPECT_EQ(safeArea.right_.start, titleButtonAreaWithRoot.right_.start);
    EXPECT_EQ(safeArea.right_.end, titleButtonAreaWithRoot.right_.end);
}

/**
 * @tc.name: TitleButtonGetCombinedSafeAreaExcludedTest
 * @tc.desc: Strategy not set -> GetCombinedSafeArea(SYSTEM) does NOT include title button area.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonGetCombinedSafeAreaExcludedTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    SafeAreaExpandOpts opt;
    opt.type |= SAFE_AREA_TYPE_SYSTEM;
    auto safeArea = safeAreaManager_->GetCombinedSafeArea(opt);
    EXPECT_EQ(safeArea.right_.start, SYSTEM_RIGHT_START);
    EXPECT_EQ(safeArea.right_.end, SYSTEM_RIGHT_END);
}

/**
 * @tc.name: TitleButtonSafeAreaToPaddingIncludedTest
 * @tc.desc: Strategy set -> SafeAreaToPadding includes title button area in padding.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonSafeAreaToPaddingIncludedTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    NG::OptionalSize<uint32_t> rootSize(
        static_cast<uint32_t>(DISPLAY_WIDTH), static_cast<uint32_t>(DISPLAY_HEIGHT));
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    safeAreaManager_->UpdateTitleButtonSafeArea(titleButtonArea, rootSize);
    auto padding = safeAreaManager_->SafeAreaToPadding(false, LAYOUT_SAFE_AREA_TYPE_SYSTEM);
    EXPECT_NEAR(padding.right.value_or(0), TITLE_BUTTON_RIGHT_END - TITLE_BUTTON_RIGHT_START, 1.0f);
}

/**
 * @tc.name: TitleButtonNoDataNoEffectTest
 * @tc.desc: Strategy set but no title button data -> GetSafeArea unaffected (AC-1.4 / VM-4).
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaManagerTest, TitleButtonNoDataNoEffectTest, TestSize.Level0)
{
    safeAreaManager_->SetIsFullScreen(true);
    safeAreaManager_->UpdateSystemSafeArea(systemArea);
    safeAreaManager_->ApplyDefaultImmersiveStrategy({ ImmersiveStrategy::AVOID_WINDOW_CONTROLS });
    auto safeArea = safeAreaManager_->GetSafeArea();
    EXPECT_EQ(safeArea.right_.start, SYSTEM_RIGHT_START);
    EXPECT_EQ(safeArea.right_.end, SYSTEM_RIGHT_END);
}
} // namespace OHOS::Ace::NG
