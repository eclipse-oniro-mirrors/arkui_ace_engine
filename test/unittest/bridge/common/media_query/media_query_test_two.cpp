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

#define private public
#define protected public
#include "core/pipeline/pipeline_base.h"
#include "frameworks/bridge/common/media_query/media_query_info.h"
#include "frameworks/bridge/common/media_query/media_queryer.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr double TEST_DIP_SCALE = 2.0;
}

class MediaQueryGapTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockContainer::SetUp();
        MockPipelineContext::SetUp();
    }

    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }

    void TearDown() override
    {
        // restore default container without explicit pipeline binding
        MockContainer::SetUp();
    }
};

/**
 * @tc.name: MediaQueryGapTransferValueVp001
 * @tc.desc: Test MatchCondition with vp unit converts device value by dip scale.
 * @tc.type: FUNC
 */
HWTEST_F(MediaQueryGapTest, MediaQueryGapTransferValueVp001, TestSize.Level1)
{
    /**
     * @tc.steps: bind a pipeline with dipScale 2.0 to the container and match
     *            width conditions in vp units.
     * @tc.expected: device width is divided by dip scale before comparing, so
     *               720px screen equals 360vp.
     */
    auto pipeline = MockPipelineContext::GetCurrentContext();
    ASSERT_TRUE(pipeline);
    pipeline->dipScale_ = TEST_DIP_SCALE;
    MockContainer::SetUp(pipeline);

    Framework::MediaQueryer queryer;
    auto json = JsonUtil::Create(true);
    json->Put("width", 720);
    json->Put("height", 1280);

    // 720 / 2.0 = 360vp
    EXPECT_FALSE(queryer.MatchCondition("(width>=400vp)", json));
    EXPECT_TRUE(queryer.MatchCondition("(width>=300vp)", json));
    EXPECT_FALSE(queryer.MatchCondition("(width<=300vp)", json));
    EXPECT_TRUE(queryer.MatchCondition("(width<=400vp)", json));
    // min-/max- style also goes through the unit conversion path
    EXPECT_FALSE(queryer.MatchCondition("screen and (min-width: 400vp)", json));
    EXPECT_TRUE(queryer.MatchCondition("screen and (min-width: 300vp)", json));

    pipeline->dipScale_ = 1.0;
}

/**
 * @tc.name: MediaQueryGapTransferValueVpNoContainer001
 * @tc.desc: Test MatchCondition vp branch when container is absent.
 * @tc.type: FUNC
 */
HWTEST_F(MediaQueryGapTest, MediaQueryGapTransferValueVpNoContainer001, TestSize.Level1)
{
    /**
     * @tc.steps: tear down the container then match vp width conditions.
     * @tc.expected: without container the value is not converted, 720px screen
     *               equals 720vp.
     */
    MockContainer::TearDown();
    Framework::MediaQueryer queryer;
    auto json = JsonUtil::Create(true);
    json->Put("width", 720);
    json->Put("height", 1280);

    // no conversion: 720vp stays 720
    EXPECT_FALSE(queryer.MatchCondition("(width>=1000vp)", json));
    EXPECT_TRUE(queryer.MatchCondition("(width>=100vp)", json));

    MockContainer::SetUp();
}

/**
 * @tc.name: MediaQueryGapTransferValueUnits001
 * @tc.desc: Test MatchCondition with dpi/dpcm/default units.
 * @tc.type: FUNC
 */
HWTEST_F(MediaQueryGapTest, MediaQueryGapTransferValueUnits001, TestSize.Level1)
{
    /**
     * @tc.steps: match conditions using dpi and dpcm units plus unitless values.
     * @tc.expected: dpi multiplies by 96, dpcm by 36, unitless stays raw.
     */
    Framework::MediaQueryer queryer;
    auto json = JsonUtil::Create(true);
    json->Put("width", 720);
    json->Put("height", 1280);

    // dpi: 720 * 96 = 69120
    EXPECT_TRUE(queryer.MatchCondition("(width>=69120dpi)", json));
    EXPECT_FALSE(queryer.MatchCondition("(width>=69121dpi)", json));
    // dpcm: 720 * 36 = 25920
    EXPECT_TRUE(queryer.MatchCondition("(width>=25920dpcm)", json));
    EXPECT_FALSE(queryer.MatchCondition("(width>=25921dpcm)", json));
    // unitless: raw 720
    EXPECT_TRUE(queryer.MatchCondition("(width>=720)", json));
    EXPECT_FALSE(queryer.MatchCondition("(width>=721)", json));
}
} // namespace OHOS::Ace::NG
