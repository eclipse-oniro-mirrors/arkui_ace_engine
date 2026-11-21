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

#define PRIVATE public
#define PROTECTED public
#include "base/utils/measure_util.h"
#undef PRIVATE
#undef PROTECTED

#define private public
#define protected public
#include "base/memory/referenced.h"
#include "core/common/font_manager.h"
#include "core/common/font_loader.h"
#include "core/components/common/properties/text_style.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_font_manager.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {

class MeasureUtilTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void MeasureUtilTest::SetUp() {}

void MeasureUtilTest::TearDown() {}

/**
 * @tc.name: MeasureText001
 * @tc.desc: Verify MeasureText returns non-negative value with default context
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureText001, TestSize.Level1)
{
    MeasureContext context;
    double result = MeasureUtil::MeasureText(context);
    EXPECT_GE(result, 0.0);
}

/**
 * @tc.name: MeasureText002
 * @tc.desc: Verify MeasureText returns positive width for non-empty text content
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureText002, TestSize.Level1)
{
    MeasureContext context;
    context.textContent = "Hello World";
    double result = MeasureUtil::MeasureText(context);
    EXPECT_GT(result, 0.0);
}

/**
 * @tc.name: MeasureText003
 * @tc.desc: Verify MeasureText returns positive width with all optional fields set
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureText003, TestSize.Level1)
{
    MeasureContext context;
    context.textContent = "Complete";
    context.fontWeight = "400";
    context.fontFamily = "Roboto";
    context.fontSize = Dimension(14.0, DimensionUnit::FP);
    context.letterSpacing = Dimension(1.0, DimensionUnit::VP);
    context.fontStyle = FontStyle::ITALIC;
    double result = MeasureUtil::MeasureText(context);
    EXPECT_GT(result, 0.0);
}

/**
 * @tc.name: MeasureTextSize001
 * @tc.desc: Verify MeasureTextSize returns valid Size with default context
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSize001, TestSize.Level1)
{
    MeasureContext context;
    Size result = MeasureUtil::MeasureTextSize(context);
    EXPECT_GE(result.Width(), 0.0);
    EXPECT_GE(result.Height(), 0.0);
}

/**
 * @tc.name: MeasureTextSize002
 * @tc.desc: Verify MeasureTextSize returns positive Size with text overflow ellipsis
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSize002, TestSize.Level1)
{
    MeasureContext context;
    context.textContent = "Overflow text";
    context.textOverlayFlow = TextOverflow::ELLIPSIS;
    Size result = MeasureUtil::MeasureTextSize(context);
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);
}

/**
 * @tc.name: MeasureTextSize003
 * @tc.desc: Verify MeasureTextSize returns positive Size with max lines
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSize003, TestSize.Level1)
{
    MeasureContext context;
    context.textContent = "Multi\nline\ntext";
    context.maxlines = 3;
    Size result = MeasureUtil::MeasureTextSize(context);
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);
}

/**
 * @tc.name: MeasureTextSize004
 * @tc.desc: Verify MeasureTextSize returns positive Size with all optional fields
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSize004, TestSize.Level1)
{
    MeasureContext context;
    context.textContent = "AllFields";
    context.fontWeight = "500";
    context.fontFamily = "Helvetica";
    context.fontSize = Dimension(18.0, DimensionUnit::FP);
    context.letterSpacing = Dimension(0.5, DimensionUnit::VP);
    context.lineHeight = Dimension(1.2, DimensionUnit::PERCENT);
    context.baselineOffset = Dimension(3.0, DimensionUnit::VP);
    context.constraintWidth = Dimension(300.0, DimensionUnit::VP);
    context.textIndent = Dimension(10.0, DimensionUnit::VP);
    context.textAlign = TextAlign::JUSTIFY;
    context.textOverlayFlow = TextOverflow::ELLIPSIS;
    context.maxlines = 5;
    context.fontStyle = FontStyle::ITALIC;
    context.textCase = TextCase::LOWERCASE;
    context.wordBreak = WordBreak::BREAK_WORD;
    context.isReturnActualWidth = true;
    Size result = MeasureUtil::MeasureTextSize(context);
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);
}

/**
 * @tc.name: MeasureTextSize005
 * @tc.desc: Verify MeasureTextSize returns valid Size with empty text content
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSize005, TestSize.Level1)
{
    MeasureContext context;
    context.textContent = "";
    Size result = MeasureUtil::MeasureTextSize(context);
    EXPECT_GE(result.Width(), 0.0);
    EXPECT_GE(result.Height(), 0.0);
}

/**
 * @tc.name: MeasureTextSizeWithFontFamily001
 * @tc.desc: Verify MeasureTextSize with single font family from textStyle
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSizeWithFontFamily001, TestSize.Level1)
{
    TextStyle textStyle;
    textStyle.SetFontSize(Dimension(20.0, DimensionUnit::FP));
    textStyle.SetFontFamilies(std::vector<std::string> { "sans-serif" });
    Size result = MeasureUtil::MeasureTextSizeWithFontFamily(textStyle, "SingleFamily");
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);
}

/**
 * @tc.name: MeasureTextSizeWithFontFamily002
 * @tc.desc: Verify MeasureTextSize with multiple font families joined by comma
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSizeWithFontFamily002, TestSize.Level1)
{
    TextStyle textStyle;
    textStyle.SetFontSize(Dimension(20.0, DimensionUnit::FP));
    textStyle.SetFontFamilies(std::vector<std::string> { "sans-serif", "serif", "monospace" });
    Size result = MeasureUtil::MeasureTextSizeWithFontFamily(textStyle, "MultipleFamilies");
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);
}

/**
 * @tc.name: MeasureTextSizeWithFontFamily003
 * @tc.desc: Verify MeasureTextSize falls back to app custom font when textStyle has no font families
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSizeWithFontFamily003, TestSize.Level1)
{
    NG::MockPipelineContext::SetUp();
    auto pipelineContext = NG::MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    MockContainer::SetUp(pipelineContext);
    auto fontManager = AceType::MakeRefPtr<MockFontManager>();
    pipelineContext->fontManager_ = fontManager;
    FontManager::appCustomFont_ = "AppCustomFont";

    TextStyle textStyle;
    textStyle.SetFontSize(Dimension(20.0, DimensionUnit::FP));
    Size result = MeasureUtil::MeasureTextSizeWithFontFamily(textStyle, "FallbackFont");
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);

    FontManager::appCustomFont_ = "";
    MockContainer::TearDown();
    NG::MockPipelineContext::TearDown();
}

/**
 * @tc.name: MeasureTextSizeWithFontFamily004
 * @tc.desc: Verify MeasureTextSize with no font families and null font manager
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSizeWithFontFamily004, TestSize.Level1)
{
    NG::MockPipelineContext::SetUp();
    auto pipelineContext = NG::MockPipelineContext::GetCurrent();
    ASSERT_NE(pipelineContext, nullptr);
    MockContainer::SetUp(pipelineContext);

    TextStyle textStyle;
    textStyle.SetFontSize(Dimension(20.0, DimensionUnit::FP));
    Size result = MeasureUtil::MeasureTextSizeWithFontFamily(textStyle, "NullFontManager");
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);

    MockContainer::TearDown();
    NG::MockPipelineContext::TearDown();
}

/**
 * @tc.name: MeasureTextSizeWithFontFamily005
 * @tc.desc: Verify MeasureTextSize with no font families and no pipeline context
 * @tc.type: FUNC
 */
HWTEST_F(MeasureUtilTest, MeasureTextSizeWithFontFamily005, TestSize.Level1)
{
    TextStyle textStyle;
    textStyle.SetFontSize(Dimension(20.0, DimensionUnit::FP));
    Size result = MeasureUtil::MeasureTextSizeWithFontFamily(textStyle, "NoPipeline");
    EXPECT_GT(result.Width(), 0.0);
    EXPECT_GT(result.Height(), 0.0);
}

} // namespace OHOS::Ace
