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

#include <vector>

#include "gtest/gtest.h"

#include "core/components/common/layout/grid_system_manager.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace {

class GridSystemManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: GridSystemManagerGetInstance001
 * @tc.desc: GetInstance returns a reference to the same singleton across calls.
 * @tc.type: FUNC
 */
HWTEST_F(GridSystemManagerTest, GridSystemManagerGetInstance001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Call GetInstance twice and compare addresses.
     * @tc.expected: step1. Both references refer to the same object.
     */
    GridSystemManager& instance1 = GridSystemManager::GetInstance();
    GridSystemManager& instance2 = GridSystemManager::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: GridSystemManagerGetSystemGridInfo001
 * @tc.desc: Static GetSystemGridInfo returns expected column counts per size type.
 * @tc.type: FUNC
 */
HWTEST_F(GridSystemManagerTest, GridSystemManagerGetSystemGridInfo001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Query SystemGridInfo for each GridSizeType.
     * @tc.expected: step1. Column counts match the system grid table (XS=2, SM=4, MD=8, LG=12).
     */
    EXPECT_EQ(2, GridSystemManager::GetSystemGridInfo(GridSizeType::XS).columns);
    EXPECT_EQ(4, GridSystemManager::GetSystemGridInfo(GridSizeType::SM).columns);
    EXPECT_EQ(8, GridSystemManager::GetSystemGridInfo(GridSizeType::MD).columns);
    EXPECT_EQ(12, GridSystemManager::GetSystemGridInfo(GridSizeType::LG).columns);
}
} // namespace OHOS::Ace
