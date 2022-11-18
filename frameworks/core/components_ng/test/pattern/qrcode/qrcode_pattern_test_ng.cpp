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

#include <optional>

#include "gtest/gtest.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/qrcode/qrcode_model_ng.h"
#include "core/components_ng/pattern/qrcode/qrcode_paint_property.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const std::string CREATE_VALUE = "Hello World";
constexpr Color QR_CODE_COLOR_VALUE = Color(0XFFFF0000);
} // namespace

class QRCodePropertyTestNg : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: QRCodePaintPropertyTest001
 * @tc.desc: Set QRCode value into QRCodePaintProperty and get it.
 * @tc.type: FUNC
 */
HWTEST_F(QRCodePropertyTestNg, QRCodePaintPropertyTest001, TestSize.Level1)
{
    QRCodeModelNG qrCodeModelNG;
    qrCodeModelNG.Create(CREATE_VALUE);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);
    auto qrcodePaintProperty = frameNode->GetPaintProperty<QRCodePaintProperty>();
    EXPECT_FALSE(qrcodePaintProperty == nullptr);
    EXPECT_EQ(qrcodePaintProperty->GetValue(), CREATE_VALUE);
}

/**
 * @tc.name: QRCodePaintPropertyTest002
 * @tc.desc: Set QRCodeColor value into QRCodePaintProperty and get it.
 * @tc.type: FUNC
 */
HWTEST_F(QRCodePropertyTestNg, QRCodePaintPropertyTest002, TestSize.Level1)
{
    QRCodeModelNG qrCodeModelNG;
    qrCodeModelNG.Create(CREATE_VALUE);
    qrCodeModelNG.SetQRCodeColor(QR_CODE_COLOR_VALUE);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_FALSE(frameNode == nullptr);
    auto qrcodePaintProperty = frameNode->GetPaintProperty<QRCodePaintProperty>();
    EXPECT_FALSE(qrcodePaintProperty == nullptr);
    EXPECT_EQ(qrcodePaintProperty->GetColorValue(), QR_CODE_COLOR_VALUE);
}

} // namespace OHOS::Ace::NG