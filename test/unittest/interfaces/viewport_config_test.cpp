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

#include "base/memory/ace_type.h"
#include "interfaces/inner_api/ace/viewport_config.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
class ViewportConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ViewportConfigTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "ViewportConfigTest SetUpTestCase";
}

void ViewportConfigTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "ViewportConfigTest TearDownTestCase";
}

void ViewportConfigTest::SetUp()
{
    GTEST_LOG_(INFO) << "ViewportConfigTest SetUp";
}

void ViewportConfigTest::TearDown()
{
    GTEST_LOG_(INFO) << "ViewportConfigTest TearDown";
}

/**
* @tc.name: ViewportConfigTest001
* @tc.desc: Verify SetSize, SetPosition, SetDensity and SetOrientation api for ViewportConfig
* @tc.type: FUNC
 */
HWTEST_F(ViewportConfigTest, ViewportConfigTest001, TestSize.Level1)
{
    ViewportConfig viewConfig;
    viewConfig.SetSize(1, 2);
    ASSERT_EQ(viewConfig.Width(), 1);
    ASSERT_EQ(viewConfig.Height(), 2);

    viewConfig.SetPosition(1, 2);
    ASSERT_EQ(viewConfig.Left(), 1);
    ASSERT_EQ(viewConfig.Top(), 2);

    viewConfig.SetDensity(1.0f);
    ASSERT_EQ(viewConfig.Density(), 1.0f);

    viewConfig.SetOrientation(1);
    ASSERT_EQ(viewConfig.Orientation(), 1);

    viewConfig.SetTransformHint(90);
    ASSERT_EQ(viewConfig.TransformHint(), 90);

    viewConfig.SetDisplayId(0);
    ASSERT_EQ(viewConfig.DisplayId(), 0);
}

/**
* @tc.name: ViewportConfigTest002
* @tc.desc: Verify ToString api for ViewportConfig
* @tc.type: FUNC
 */
HWTEST_F(ViewportConfigTest, ViewportConfigTest002, TestSize.Level1)
{
    ViewportConfig viewConfig = ViewportConfig(0, 1, 1.0f);
    auto toString = viewConfig.ToString();
    ViewportConfig viewConfigTest1 = ViewportConfig(0, 1, 1.0f);
    ViewportConfig viewConfigTest2 = ViewportConfig(0, 0, 1.0f);
    if (viewConfig == viewConfigTest1) {
        GTEST_LOG_(INFO) << "ViewportConfigTest operator==";
    }
    if (viewConfig != viewConfigTest2) {
        GTEST_LOG_(INFO) << "ViewportConfigTest operator!=";
    }
    ASSERT_EQ(
        toString, "Viewport config: size: (0, 1) orientation: 0 density: 1.000000 position: (0, 0) transformHint: 0 displayId: 0");
}

/**
* @tc.name: ViewportConfigTest003
* @tc.desc: Verify SetKeyFrameConfig, GetKeyFrameConfig api for ViewportConfig
* @tc.type: FUNC
 */
HWTEST_F(ViewportConfigTest, ViewportConfigTest003, TestSize.Level1)
{
    ViewportConfig viewConfig;
    viewConfig.SetKeyFrameConfig(true, 100, 200);
    ASSERT_EQ(viewConfig.GetKeyFrameConfig().enableKeyFrame_, true);
    ASSERT_EQ(viewConfig.GetKeyFrameConfig().animationDuration_, 100);
    ASSERT_EQ(viewConfig.GetKeyFrameConfig().animationDelay_, 200);
}
} // namespace OHOS::Ace