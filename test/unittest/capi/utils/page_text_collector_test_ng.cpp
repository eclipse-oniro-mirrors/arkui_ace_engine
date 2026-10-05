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
#include "core/components_ng/base/page_text_collector.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#undef private
#undef protected

#include "base/json/json_util.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float FRAME_WIDTH = 100.0f;
constexpr float FRAME_HEIGHT = 20.0f;
} // namespace

class PageTextOverlayTest : public testing::Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        pipeline_ = MockPipelineContext::GetCurrent();
        MockContainer::SetUp(pipeline_);
        MockContainer::Current()->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>());
        pipeline_->uiTranslateManager_ = std::make_shared<UiTranslateManagerImpl>(nullptr);
    }

    void TearDown() override
    {
        nodes_.clear();
        pipeline_.Reset();
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }

    RefPtr<FrameNode> CreateNode(const std::string& tag, const RefPtr<Pattern>& pattern = nullptr)
    {
        auto node = FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(),
            pattern ? pattern : AceType::MakeRefPtr<Pattern>());
        node->AttachContext(AceType::RawPtr(pipeline_));
        node->SetActive(true);
        auto render = AceType::MakeRefPtr<MockRenderContext>();
        render->SetPaintRectWithTransform(RectF(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
        node->renderContext_ = render;
        node->GetGeometryNode()->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
        nodes_.push_back(node);
        return node;
    }

    void SetOverlay(const RefPtr<FrameNode>& node, const std::string& text)
    {
        OverlayOptions overlay;
        overlay.content = text;
        node->GetRenderContext()->UpdateOverlayText(overlay);
    }

    std::unique_ptr<JsonValue> Collect()
    {
        char* data = nullptr;
        uint32_t size = 0;
        const char* reason = nullptr;
        EXPECT_EQ(CollectPageText(pipeline_->GetInstanceId(), &data, &size, &reason), 0);
        auto json = JsonUtil::ParseJsonString(std::string(data ? data : "", size));
        std::free(data);
        return json->GetValue("texts");
    }

    RefPtr<MockPipelineContext> pipeline_;
    std::vector<RefPtr<FrameNode>> nodes_;
};

// String overlays use the same RenderContext property for every component tag.
TEST_F(PageTextOverlayTest, CommonOverlayWithoutTranslationManager)
{
    const std::vector<std::string> tags = { "Blank", "Text", "Divider", "DataPanel", "LoadingProgress",
        "PatternLock", "Circle", "Ellipse", "Line", "Polyline", "Polygon", "Path", "Rect", "XComponent",
        "ScrollBar", "ArcScrollBar", "Component3D", "EmbeddedComponent", "Particle" };
    pipeline_->uiTranslateManager_.reset();
    for (const auto& tag : tags) {
        SetOverlay(CreateNode(tag), tag + " overlay");
    }
    auto texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), static_cast<int32_t>(tags.size()));
    for (size_t index = 0; index < tags.size(); ++index) {
        auto item = texts->GetArrayItem(index);
        EXPECT_EQ(item->GetInt("id"), nodes_[index]->GetId());
        EXPECT_EQ(item->GetString("content"), tags[index] + " overlay");
        auto rect = item->GetValue("rect");
        ASSERT_EQ(rect->GetArraySize(), 4);
        EXPECT_EQ(rect->GetArrayItem(2)->GetDouble(), FRAME_WIDTH);
        EXPECT_EQ(rect->GetArrayItem(3)->GetDouble(), FRAME_HEIGHT);
    }
}

TEST_F(PageTextOverlayTest, BodyAndOverlayMergeUpdateAndReset)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto node = CreateNode("Text", pattern);
    pattern->textForDisplay_ = u"body";
    pattern->MarkPageTranslateTextDrawn();
    pipeline_->uiTranslateManager_->AddTranslateListener(WeakPtr<FrameNode>(node));
    SetOverlay(node, "overlay");
    auto texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), 1);
    EXPECT_EQ(texts->GetArrayItem(0)->GetInt("id"), node->GetId());
    EXPECT_EQ(texts->GetArrayItem(0)->GetString("content"), "body\noverlay");

    SetOverlay(node, "updated \"text\"\n");
    EXPECT_EQ(Collect()->GetArrayItem(0)->GetString("content"), "body\nupdated \"text\"\n");
    EXPECT_EQ(texts->GetArrayItem(0)->GetString("content"), "body\noverlay");
    SetOverlay(node, "");
    EXPECT_EQ(Collect()->GetArrayItem(0)->GetString("content"), "body");
    node->GetRenderContext()->ResetOverlayText();
    EXPECT_EQ(Collect()->GetArrayItem(0)->GetString("content"), "body");

    SetOverlay(node, "overlay only");
    pipeline_->uiTranslateManager_->RemoveTranslateListener(node->GetId());
    EXPECT_EQ(Collect()->GetArrayItem(0)->GetString("content"), "overlay only");
    node->GetRenderContext()->ResetOverlayText();
    EXPECT_EQ(Collect()->GetArraySize(), 0);
}

TEST_F(PageTextOverlayTest, VisibilityGeometryAndInstanceIsolation)
{
    auto parent = CreateNode("Column");
    auto node = CreateNode("Blank");
    parent->AddChild(node);
    SetOverlay(node, "overlay");
    ASSERT_EQ(Collect()->GetArraySize(), 1);
    node->SetActive(false);
    EXPECT_EQ(Collect()->GetArraySize(), 0);
    node->SetActive(true);
    parent->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    EXPECT_EQ(Collect()->GetArraySize(), 0);
    parent->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    auto render = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
    render->SetPaintRectWithTransform(RectF());
    EXPECT_EQ(Collect()->GetArraySize(), 0);
    render->SetPaintRectWithTransform(RectF(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
    EXPECT_EQ(Collect()->GetArraySize(), 1);

    // An unattached node can resolve GetContext() to the current pipeline; its instance ID must still isolate it.
    node->DetachContext(false);
    node->instanceId_ = pipeline_->GetInstanceId() + 1;
    EXPECT_EQ(Collect()->GetArraySize(), 0);
    node->instanceId_ = pipeline_->GetInstanceId();
    EXPECT_EQ(Collect()->GetArraySize(), 1);
    SetOverlay(node, "");
    EXPECT_EQ(Collect()->GetArraySize(), 0);
}
} // namespace OHOS::Ace::NG
