/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "accessor_test_base.h"
#include "node_api.h"
#include "core/interfaces/native/implementation/text_controller_peer_impl.h"
#include "core/components_ng/pattern/text/span/span_string.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/validators.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

namespace GeneratedModifier {
const GENERATED_ArkUILayoutManagerAccessor* GetLayoutManagerAccessor();
const GENERATED_ArkUIStyledStringAccessor* GetStyledStringAccessor();
}

namespace {
class MockLayoutInfo : public LayoutInfoInterface {
public:
    virtual size_t GetLineCount() const
    {
        return lineCount;
    }

    void SetLineCount(size_t value)
    {
        lineCount = value;
    }

private:
    size_t lineCount = -1;
};

static std::string g_actualSpanString;
static RefPtr<MockLayoutInfo> g_layoutInfo;

class MockTextController : public TextController {
public:
    MOCK_METHOD(void, CloseSelectionMenu, ());

    void SetStyledString(const RefPtr<SpanStringBase>& value) override
    {
        SpanString* spanString = DynamicCast<SpanString>(value.GetRawPtr());
        if (spanString) {
            g_actualSpanString = spanString->GetString();
        }
    }

    WeakPtr<LayoutInfoInterface> GetLayoutInfoInterface() override
    {
        return g_layoutInfo->GetLayoutInfoInterface();
    }
};
}

class TextControllerAccessorTest : public AccessorTestBase<
    GENERATED_ArkUITextControllerAccessor,
    &GENERATED_ArkUIAccessors::getTextControllerAccessor,
    TextControllerPeer> {
public:
    void SetUp() override
    {
        AccessorTestBase::SetUp();
        mockController_ = Referenced::MakeRefPtr<MockTextController>();
        ASSERT_NE(mockController_, nullptr);
        peer_->controller = mockController_;
    }

    void TearDown() override
    {
        AccessorTestBase::TearDown();
        mockController_ = nullptr;
    }

    RefPtr<MockTextController> mockController_ = nullptr;
};

/**
 * @tc.name: closeSelectionMenuTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextControllerAccessorTest, closeSelectionMenuTest, TestSize.Level1)
{
    EXPECT_CALL(*mockController_, CloseSelectionMenu()).Times(1);
    accessor_->closeSelectionMenu(peer_);
}

/**
 * @tc.name: setStyledStringTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextControllerAccessorTest, setStyledStringTest, TestSize.Level1)
{
    const std::string expectedStrValue = "String value";
    Ark_StyledString styledString;
    auto value = Converter::ArkUnion<Ark_Union_String_ImageAttachment_CustomSpan, Ark_String>(expectedStrValue);
    auto styles = Converter::ArkValue<Opt_Array_StyleOptions>();

    auto styledStringAccessor = GeneratedModifier::GetStyledStringAccessor();
    ASSERT_NE(styledStringAccessor, nullptr);
    styledString.ptr = styledStringAccessor->ctor(&value, &styles);
    accessor_->setStyledString(peer_, &styledString);
    EXPECT_EQ(g_actualSpanString, expectedStrValue);
    styledStringAccessor->destroyPeer(reinterpret_cast<StyledStringPeer*>(styledString.ptr));
}

/**
 * @tc.name: getLayoutManagerTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextControllerAccessorTest, getLayoutManagerTest, TestSize.Level1)
{
    g_layoutInfo = new MockLayoutInfo();
    const size_t expectedLineCount = 143;
    g_layoutInfo->SetLineCount(expectedLineCount);

    Ark_NativePointer manager = accessor_->getLayoutManager(peer_); // Create LayoutManager peer
    ASSERT_NE(manager, nullptr);

    auto layoutManagerAccessor = GeneratedModifier::GetLayoutManagerAccessor();
    ASSERT_NE(layoutManagerAccessor, nullptr);
    auto layoutManagerPeer = reinterpret_cast<LayoutManagerPeer*>(manager);
    EXPECT_EQ(layoutManagerAccessor->getLineCount(layoutManagerPeer), expectedLineCount);
    layoutManagerAccessor->destroyPeer(layoutManagerPeer); // Destroy LayoutManager peer
}

} // namespace OHOS::Ace::NG