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
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_tree_request_context.h"
#include "core/components_ng/pattern/pattern.h"

#define private public
#define protected public
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#undef protected
#undef private

namespace OHOS::Ace::NG {
class InspectorIdFrameNode : public FrameNode {
    DECLARE_ACE_TYPE(InspectorIdFrameNode, FrameNode);

public:
    InspectorIdFrameNode() : FrameNode("test", 7, AceType::MakeRefPtr<Pattern>()) {}
    void SetInspectorAccessibilityId(int64_t id)
    {
        accessibilityId_ = id;
    }
};

class InspectorTreeTask03 : public testing::Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
    }
    void TearDown() override
    {
        MockPipelineContext::TearDown();
    }
};

class InspectorRichEditorPattern : public RichEditorPattern {
    DECLARE_ACE_TYPE(InspectorRichEditorPattern, RichEditorPattern);

public:
    void SetInspectorText(const std::u16string& content)
    {
        auto span = AceType::MakeRefPtr<SpanItem>();
        span->content = content;
        spans_ = { span };
    }

    const std::u16string& GetInspectorText() const { return spans_.front()->content; }
    void SetInspectorObscured(bool obscured) { textObscured_ = obscured; }

protected:
    void OnAttachToFrameNode() override {}
    void OnDetachFromFrameNode(FrameNode*) override {}
    void OnDetachFromMainTree() override {}
};

class InspectorTextFieldPattern : public TextFieldPattern {
    DECLARE_ACE_TYPE(InspectorTextFieldPattern, TextFieldPattern);

protected:
    void OnAttachToFrameNode() override {}
    void OnDetachFromFrameNode(FrameNode*) override {}
    void OnDetachFromMainTree() override {}
};

TEST_F(InspectorTreeTask03, WindowPrivacyFiltersBothEditorsAndRestoresTheCaller)
{
    auto input = AceType::MakeRefPtr<InspectorTextFieldPattern>();
    auto inputNode = FrameNode::CreateFrameNode("TextInput", 11, input);
    inputNode->SetActive(true);
    ASSERT_NE(input->GetTextContentController(), nullptr);
    input->GetTextContentController()->SetTextValue(u"input-private-value");
    auto rich = AceType::MakeRefPtr<InspectorRichEditorPattern>();
    auto richNode = FrameNode::CreateFrameNode("RichEditor", 12, rich);
    richNode->SetActive(true);
    rich->SetInspectorText(u"rich-private-value");
    for (bool childPrivate : { false, true }) {
        for (bool visible : { false, true }) {
            for (bool accessibility : { false, true }) {
                ParamConfig config;
                config.accessibilityInfo = accessibility;
                InspectorTreeRequestContext context(config, {}, { !childPrivate, "Main", 42 });
                config.inspectorContext = &context;
                auto checkEditors = [&](bool privacy) {
                    auto inputTree = JsonUtil::CreateSharedPtrJson();
                    auto richTree = JsonUtil::CreateSharedPtrJson();
                    inputNode->DumpSimplifyTreeWithParamConfig(0, inputTree, visible, config);
                    richNode->DumpSimplifyTreeWithParamConfig(0, richTree, visible, config);
                    EXPECT_EQ(
                        inputTree->GetObject("$attrs")->GetString("content"), privacy ? "" : "input-private-value");
                    EXPECT_EQ(richTree->GetObject("$attrs")->GetString("content"), privacy ? "" : "rich-private-value");
                    if (privacy) {
                        EXPECT_EQ(inputTree->ToString().find("input-private-value"), std::string::npos);
                        EXPECT_EQ(richTree->ToString().find("rich-private-value"), std::string::npos);
                    }
                };
                context.CollectWithWindowPrivacy(
                    InspectorWindowSnapshot { childPrivate, "Child", 43 }, [&]() { checkEditors(childPrivate); });
                checkEditors(!childPrivate);
                EXPECT_EQ(context.window.privacyWindow, !childPrivate);
            }
        }
    }
    EXPECT_EQ(input->GetTextValue(), "input-private-value");
    EXPECT_EQ(rich->GetInspectorText(), u"rich-private-value");
}

TEST_F(InspectorTreeTask03, RichEditorMultiLineFollowsCurrentMode)
{
    auto pattern = AceType::MakeRefPtr<InspectorRichEditorPattern>();
    auto frame = FrameNode::CreateFrameNode("RichEditor", 13, pattern);
    frame->SetActive(true);
    for (bool singleLine : { false, true, false }) {
        pattern->isSingleLineMode_ = singleLine;
        for (bool visible : { false, true }) {
            for (bool accessibility : { false, true }) {
                ParamConfig config;
                config.accessibilityInfo = accessibility;
                InspectorTreeRequestContext context(config, {}, {});
                config.inspectorContext = &context;
                auto tree = JsonUtil::CreateSharedPtrJson();
                frame->DumpSimplifyTreeWithParamConfig(0, tree, visible, config);
                auto attrs = tree->GetObject("$attrs");
                if (accessibility) {
                    EXPECT_TRUE(attrs->GetValue("multiLine")->IsBool());
                    EXPECT_EQ(attrs->GetBool("multiLine"), !singleLine);
                } else {
                    EXPECT_FALSE(attrs->Contains("multiLine"));
                }
            }
        }
    }
}

TEST_F(InspectorTreeTask03, RichEditorPasswordIsProtectedWhenShownOrHidden)
{
    auto pattern = AceType::MakeRefPtr<InspectorRichEditorPattern>();
    auto frame = FrameNode::CreateFrameNode("RichEditor", 9, pattern);
    frame->SetActive(true);
    pattern->isSingleLineMode_ = true;
    pattern->SetInspectorText(u"inspector-password-value");
    auto layout = frame->GetLayoutProperty<RichEditorLayoutProperty>();
    for (auto type : { TextInputType::TEXT, TextInputType::VISIBLE_PASSWORD,
             TextInputType::NEW_PASSWORD, TextInputType::NUMBER_PASSWORD }) {
        layout->UpdateTextInputType(type);
        const bool password = type != TextInputType::TEXT;
        ASSERT_EQ(pattern->IsInPasswordMode(), password);
        for (bool obscured : { false, true }) {
            pattern->SetInspectorObscured(obscured);
            ASSERT_EQ(pattern->IsPasswordObscured(), password && obscured);
            for (bool visible : { false, true }) {
                for (bool accessibility : { false, true }) {
                    SCOPED_TRACE(testing::Message() << "type=" << static_cast<int>(type) << ", obscured=" << obscured
                        << ", visible=" << visible << ", accessibility=" << accessibility);
                    ParamConfig config;
                    config.accessibilityInfo = accessibility;
                    InspectorTreeRequestContext context(config, {}, {});
                    config.inspectorContext = &context;
                    auto tree = JsonUtil::CreateSharedPtrJson();
                    frame->DumpSimplifyTreeWithParamConfig(0, tree, visible, config);
                    auto attrs = tree->GetObject("$attrs");
                    EXPECT_EQ(attrs->GetString("content"), password ? "" : "inspector-password-value");
                    if (accessibility) {
                        EXPECT_TRUE(attrs->GetValue("password")->IsBool());
                        EXPECT_EQ(attrs->GetBool("password"), password);
                        EXPECT_TRUE(attrs->GetValue("multiLine")->IsBool());
                        EXPECT_FALSE(attrs->GetBool("multiLine"));
                    } else {
                        EXPECT_FALSE(attrs->Contains("password"));
                        EXPECT_FALSE(attrs->Contains("multiLine"));
                    }
                    if (password) {
                        EXPECT_EQ(tree->ToString().find("inspector-password-value"), std::string::npos);
                    }
                    EXPECT_EQ(pattern->GetInspectorText(), u"inspector-password-value");
                    EXPECT_EQ(pattern->GetTextObscured(), obscured);
                }
            }
        }
    }
}

TEST_F(InspectorTreeTask03, NativeIdsAreLosslessAndGated)
{
    auto frame = AceType::MakeRefPtr<InspectorIdFrameNode>();
    frame->InitializePatternAndContext();
    ParamConfig config;
    config.accessibilityInfo = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    for (int64_t id : { 0LL, 9007199254740991LL, 9007199254740992LL, 9007199254740993LL, 9223372036854775807LL }) {
        frame->SetInspectorAccessibilityId(id);
        auto json = JsonUtil::CreateSharedPtrJson();
        frame->DumpSimplifyTreeNode(json, config);
        EXPECT_TRUE(json->GetValue("accessibilityId")->IsString());
        EXPECT_EQ(json->GetString("accessibilityId"), std::to_string(id));
        EXPECT_EQ(json->GetInt("$ID"), 7);
        EXPECT_TRUE(json->GetValue("accessibilityGroup")->IsBool());
    }
    config.accessibilityInfo = false;
    auto disabled = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(disabled, config);
    EXPECT_FALSE(disabled->Contains("accessibilityId"));
    config.accessibilityInfo = true;
    frame->SetInspectorAccessibilityId(-1);
    auto invalid = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(invalid, config);
    EXPECT_FALSE(invalid->Contains("accessibilityId"));
}

TEST_F(InspectorTreeTask03, FalseInteractionAndAbsentRegions)
{
    auto frame = FrameNode::CreateFrameNode("test", 8, AceType::MakeRefPtr<Pattern>());
    frame->GetOrCreateGestureEventHub()->SetTouchable(false);
    ParamConfig config;
    config.interactionInfo = true;
    config.withResponseRegion = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    auto json = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(json, config);
    EXPECT_TRUE(json->GetValue("touchable")->IsBool());
    EXPECT_FALSE(json->GetBool("touchable"));
    EXPECT_TRUE(json->GetValue("checkable")->IsBool());
    EXPECT_FALSE(json->Contains("checked"));
    EXPECT_FALSE(json->Contains("responseRegion"));
    EXPECT_FALSE(json->Contains("responseRegionMap"));
    EXPECT_FALSE(json->Contains("mouseResponseRegion"));
}
} // namespace OHOS::Ace::NG
