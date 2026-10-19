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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "text_base.h"

#include "core/components_ng/pattern/text/text_layout_property.h"

namespace OHOS::Ace::NG {

class TextExternalParagraphTest : public TextBases {};

/**
 * @tc.name: SetExternalParagraph001
 * @tc.desc: Test SetExternalParagraph with a new pointer
 * @tc.type: FUNC
 */
HWTEST_F(TextExternalParagraphTest, SetExternalParagraph001, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextLayoutProperty> layoutProperty = AceType::MakeRefPtr<TextLayoutProperty>();
    frameNode->SetLayoutProperty(layoutProperty);
    auto pattern = frameNode->GetPattern<TextPattern>();
    ASSERT_NE(pattern, nullptr);

    int32_t dummy = 0;
    void* ptr = &dummy;
    pattern->SetExternalParagraph(ptr);
    EXPECT_TRUE(pattern->GetExternalParagraph().has_value());
    EXPECT_EQ(pattern->GetExternalParagraph().value(), ptr);
}

/**
 * @tc.name: SetExternalParagraph002
 * @tc.desc: Test SetExternalParagraph with same pointer (early return)
 * @tc.type: FUNC
 */
HWTEST_F(TextExternalParagraphTest, SetExternalParagraph002, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextLayoutProperty> layoutProperty = AceType::MakeRefPtr<TextLayoutProperty>();
    frameNode->SetLayoutProperty(layoutProperty);
    auto pattern = frameNode->GetPattern<TextPattern>();
    ASSERT_NE(pattern, nullptr);

    int32_t dummy = 0;
    void* ptr = &dummy;
    pattern->SetExternalParagraph(ptr);
    ASSERT_TRUE(pattern->GetExternalParagraph().has_value());

    pattern->SetExternalParagraph(ptr);
    EXPECT_TRUE(pattern->GetExternalParagraph().has_value());
    EXPECT_EQ(pattern->GetExternalParagraph().value(), ptr);
}

/**
 * @tc.name: SetExternalParagraph003
 * @tc.desc: Test SetExternalParagraph with nullptr (clear)
 * @tc.type: FUNC
 */
HWTEST_F(TextExternalParagraphTest, SetExternalParagraph003, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextLayoutProperty> layoutProperty = AceType::MakeRefPtr<TextLayoutProperty>();
    frameNode->SetLayoutProperty(layoutProperty);
    auto pattern = frameNode->GetPattern<TextPattern>();
    ASSERT_NE(pattern, nullptr);

    int32_t dummy = 0;
    void* ptr = &dummy;
    pattern->SetExternalParagraph(ptr);
    ASSERT_TRUE(pattern->GetExternalParagraph().has_value());

    pattern->SetExternalParagraph(nullptr);
    EXPECT_FALSE(pattern->GetExternalParagraph().has_value());
}

/**
 * @tc.name: ReleaseExternalParagraphRef001
 * @tc.desc: Test ReleaseExternalParagraphRef when externalParagraph_ is empty
 * @tc.type: FUNC
 */
HWTEST_F(TextExternalParagraphTest, ReleaseExternalParagraphRef001, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextLayoutProperty> layoutProperty = AceType::MakeRefPtr<TextLayoutProperty>();
    frameNode->SetLayoutProperty(layoutProperty);
    auto pattern = frameNode->GetPattern<TextPattern>();
    ASSERT_NE(pattern, nullptr);

    EXPECT_FALSE(pattern->GetExternalParagraph().has_value());
    pattern->ReleaseExternalParagraphRef();
    EXPECT_FALSE(pattern->GetExternalParagraph().has_value());
}

/**
 * @tc.name: ReleaseExternalParagraphRef002
 * @tc.desc: Test ReleaseExternalParagraphRef with a valid paragraph
 * @tc.type: FUNC
 */
HWTEST_F(TextExternalParagraphTest, ReleaseExternalParagraphRef002, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 0, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextLayoutProperty> layoutProperty = AceType::MakeRefPtr<TextLayoutProperty>();
    frameNode->SetLayoutProperty(layoutProperty);
    auto pattern = frameNode->GetPattern<TextPattern>();
    ASSERT_NE(pattern, nullptr);

    int32_t dummy = 0;
    void* ptr = &dummy;
    pattern->SetExternalParagraph(ptr);
    EXPECT_TRUE(pattern->GetExternalParagraph().has_value());

    pattern->ReleaseExternalParagraphRef();
    EXPECT_FALSE(pattern->GetExternalParagraph().has_value());
}

} // namespace OHOS::Ace::NG
