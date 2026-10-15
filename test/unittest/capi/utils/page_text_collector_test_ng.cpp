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
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/progress/progress_model_ng.h"
#include "core/components_ng/pattern/progress/progress_pattern.h"
#include "core/components_ng/pattern/select/select_model_ng.h"
#include "core/components_ng/pattern/select/select_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_model_ng.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/slider/slider_pattern.h"
#include "core/components_ng/pattern/slider/slider_model_ng.h"
#include "core/components/slider/slider_theme.h"
#include "core/components_ng/render/paint_wrapper.h"
#include "base/subwindow/subwindow_manager.h"
#include "test/mock/frameworks/base/subwindow/mock_subwindow.h"
#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components/select/select_theme.h"
#include "core/components/text/text_theme.h"
#include "core/components/theme/icon_theme.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
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

class PageTextRenderContext : public MockRenderContext {
    DECLARE_ACE_TYPE(PageTextRenderContext, MockRenderContext);
public:
    std::pair<RectF, bool> GetPaintRectWithTranslate() override
    {
        return { GetPaintRectWithoutTransform(), false };
    }
};

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
        PrepareNode(node);
        return node;
    }

    void PrepareNode(const RefPtr<FrameNode>& node)
    {
        node->AttachContext(AceType::RawPtr(pipeline_));
        node->SetActive(true);
        auto render = AceType::MakeRefPtr<PageTextRenderContext>();
        render->SetPaintRectWithTransform(RectF(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
        node->renderContext_ = render;
        node->GetGeometryNode()->SetFrameSize(SizeF(FRAME_WIDTH, FRAME_HEIGHT));
        nodes_.push_back(node);
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
        // GetValue borrows from json; return an owning copy that survives this helper.
        return JsonUtil::ParseJsonString(json->GetValue("texts")->ToString());
    }

    RefPtr<MockPipelineContext> pipeline_;
    std::vector<RefPtr<FrameNode>> nodes_;
};

// String overlays use the same RenderContext property for every component tag.
TEST_F(PageTextOverlayTest, CommonOverlayWithoutTranslationManager)
{
    constexpr int32_t rectFieldCount = 4;
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
        ASSERT_EQ(rect->GetArraySize(), rectFieldCount); // Four entries: x, y, width, height.
        EXPECT_EQ(rect->GetArrayItem(2)->GetDouble(), FRAME_WIDTH); // Index 2 is width.
        EXPECT_EQ(rect->GetArrayItem(3)->GetDouble(), FRAME_HEIGHT); // Index 3 is height.
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
    EXPECT_EQ(Collect()->GetArrayItem(0)->GetString("content"), "body\noverlay only");
    node->GetRenderContext()->ResetOverlayText();
    EXPECT_EQ(Collect()->GetArrayItem(0)->GetString("content"), "body");
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
class PageTextCollectionTest : public PageTextOverlayTest {
public:
    RefPtr<FrameNode> Text(const std::u16string& content)
    {
        auto node = CreateNode(V2::TEXT_ETS_TAG, AceType::MakeRefPtr<TextPattern>());
        SetText(node, content);
        return node;
    }

    void SetText(const RefPtr<FrameNode>& node, const std::u16string& content)
    {
        auto pattern = node->GetPattern<TextPattern>();
        node->GetLayoutProperty<TextLayoutProperty>()->UpdateContent(content);
        pattern->textForDisplay_ = content;
        pattern->MarkPageTranslateTextDrawn();
    }

    RefPtr<FrameNode> Input(const std::string& tag, const std::u16string& content)
    {
        auto node = CreateNode(tag, AceType::MakeRefPtr<TextFieldPattern>());
        node->GetPattern<TextFieldPattern>()->contentController_->SetTextValue(content);
        return node;
    }

    RefPtr<CustomNode> Owner(std::string name)
    {
        auto owner = CustomNode::CreateCustomNode(ElementRegister::GetInstance()->MakeUniqueId(), name);
        owner->SetJSViewName(std::move(name));
        owner->AttachContext(AceType::RawPtr(pipeline_));
        return owner;
    }

    void SetUpBackground(const RefPtr<FrameNode>& host, const RefPtr<FrameNode>& root)
    {
        host->backgroundNode_ = root;
        host->GetRenderContext()->UpdateBuilderBackgroundFlag(true);
        host->GetRenderContext()->UpdateBackgroundAlign(Alignment::TOP_LEFT);
        LayoutConstraintF constraint;
        constraint.maxSize = SizeF(FRAME_WIDTH, FRAME_HEIGHT);
        root->GetGeometryNode()->SetParentLayoutConstraint(constraint);
    }

    void SetUpSliderThemes()
    {
        auto themes = AceType::MakeRefPtr<testing::NiceMock<MockThemeManager>>();
        auto themeFor = [](ThemeType type) -> RefPtr<Theme> {
            if (type == SliderTheme::TypeId()) {
                return AceType::MakeRefPtr<SliderTheme>();
            }
            if (type == TextTheme::TypeId()) {
                return AceType::MakeRefPtr<TextTheme>();
            }
            return nullptr;
        };
        ON_CALL(*themes, GetTheme(testing::_)).WillByDefault(themeFor);
        ON_CALL(*themes, GetTheme(testing::_, testing::_))
            .WillByDefault([themeFor](ThemeType type, int32_t) { return themeFor(type); });
        pipeline_->SetThemeManager(themes);
    }

    bool Contains(const std::string& content)
    {
        auto texts = Collect();
        for (int32_t index = 0; index < texts->GetArraySize(); ++index) {
            if (texts->GetArrayItem(index)->GetString("content") == content) {
                return true;
            }
        }
        return false;
    }
};

TEST_F(PageTextCollectionTest, UnregisteredTextAndInputAffixes)
{
    constexpr int32_t expectedAffixCount = 2;
    auto field = Input(V2::TEXTINPUT_ETS_TAG, u"private input");
    auto pattern = field->GetPattern<TextFieldPattern>();
    auto errorDecorator = AceType::MakeRefPtr<ErrorDecorator>(field);
    auto error = errorDecorator->textNode_.Upgrade();
    ASSERT_NE(error, nullptr);
    PrepareNode(error);
    SetText(error, u"error message");
    auto unit = Text(u"kg");
    field->GetLayoutProperty<TextFieldLayoutProperty>()->UpdateShowUnderline(true);
    auto unitArea = AceType::MakeRefPtr<UnitResponseArea>(WeakPtr<Pattern>(pattern), unit);
    unitArea->InitResponseArea();
    ASSERT_EQ(unit->GetParent(), field);
    auto placeholder = Text(u"styled placeholder");
    field->AddChild(placeholder);
    auto area = AceType::MakeRefPtr<PlaceholderResponseArea>(WeakPtr<Pattern>(pattern));
    area->placeholderNode_ = placeholder;
    pattern->placeholderResponseArea_ = area;
    pipeline_->uiTranslateManager_.reset();
    auto texts = Collect();
    // Only the error message and unit label are collected, giving two entries.
    EXPECT_EQ(texts->GetArraySize(), expectedAffixCount);
    EXPECT_TRUE(Contains("error message"));
    EXPECT_TRUE(Contains("kg"));
    EXPECT_FALSE(Contains("private input"));
    EXPECT_FALSE(Contains("styled placeholder"));
}

TEST_F(PageTextCollectionTest, CompositeInputOwnershipAndReparenting)
{
    auto atomic = Owner("AtomicServiceSearch");
    auto search = CreateNode(V2::SEARCH_ETS_TAG);
    atomic->AddChild(search);
    auto value = Input(V2::SEARCH_Field_ETS_TAG, u"search value");
    search->AddChild(value);
    auto ordinarySearch = CreateNode(V2::SEARCH_ETS_TAG);
    auto ordinary = Input(V2::SEARCH_Field_ETS_TAG, u"ordinary search");
    ordinarySearch->AddChild(ordinary);
    auto counter = Owner("CounterComponent");
    auto count = Input(V2::TEXTINPUT_ETS_TAG, u"42");
    count->UpdateInspectorId("InlineTextInput123");
    counter->AddChild(count);
    auto counterV2 = Owner("CounterV2Component");
    auto date = Input(V2::TEXTINPUT_ETS_TAG, u"2026");
    date->UpdateInspectorId("DateTextInput1123");
    counterV2->AddChild(date);
    EXPECT_TRUE(Contains("search value"));
    EXPECT_TRUE(Contains("42"));
    EXPECT_TRUE(Contains("2026"));
    EXPECT_FALSE(Contains("ordinary search"));

    atomic->RemoveChild(search);
    auto otherOwner = Owner("OtherComponent");
    otherOwner->AddChild(search);
    EXPECT_FALSE(Contains("search value"));
    auto nested = Owner("ApplicationChild");
    counter->RemoveChild(count);
    counter->AddChild(nested);
    nested->AddChild(count);
    EXPECT_FALSE(Contains("42"));
    date->UpdateInspectorId("unrelatedInput");
    EXPECT_FALSE(Contains("2026"));
}

TEST_F(PageTextCollectionTest, InputEditorAndVirtualTextRemainExcluded)
{
    auto input = Input(V2::TEXTINPUT_ETS_TAG, u"input body");
    auto password = Input(V2::TEXTINPUT_ETS_TAG, u"password body");
    password->GetLayoutProperty<TextFieldLayoutProperty>()->UpdateTextInputType(TextInputType::VISIBLE_PASSWORD);
    auto area = Input(V2::TEXTAREA_ETS_TAG, u"area body");
    input->GetLayoutProperty<TextFieldLayoutProperty>()->UpdateShowCounter(true);
    area->GetLayoutProperty<TextFieldLayoutProperty>()->UpdateShowCounter(true);
    auto editor = CreateNode(V2::RICH_EDITOR_ETS_TAG, AceType::MakeRefPtr<RichEditorPattern>());
    auto editorPattern = editor->GetPattern<RichEditorPattern>();
    editorPattern->textForDisplay_ = u"editor body";
    editorPattern->MarkPageTranslateTextDrawn();
    auto virtualText = Text(u"virtual text");
    virtualText->SetAccessibilityNodeVirtual();
    for (const auto& node : { input, password, area, editor, virtualText }) {
        pipeline_->uiTranslateManager_->AddTranslateListener(WeakPtr<FrameNode>(node));
    }
    EXPECT_EQ(Collect()->GetArraySize(), 0);
}

TEST_F(PageTextCollectionTest, ReadyTextDeduplicationAndUpdate)
{
    constexpr int32_t expectedSlashCount = 2;
    auto text = Text(u"label");
    auto pattern = text->GetPattern<TextPattern>();
    pipeline_->uiTranslateManager_->AddTranslateListener(WeakPtr<FrameNode>(text));
    EXPECT_EQ(Collect()->GetArraySize(), 1);
    pipeline_->uiTranslateManager_->RemoveTranslateListener(text->GetId());
    EXPECT_TRUE(Contains("label"));
    SetText(text, u"updated");
    EXPECT_TRUE(Contains("updated"));
    SetText(text, u"");
    EXPECT_EQ(Collect()->GetArraySize(), 0);
    auto firstSlash = Text(u"/");
    auto secondSlash = Text(u"/");
    // The two distinct slash nodes remain separate entries even though their content is identical.
    EXPECT_EQ(Collect()->GetArraySize(), expectedSlashCount);
}

TEST_F(PageTextCollectionTest, SelectModelValueUsesInternalText)
{
    auto themes = AceType::MakeRefPtr<testing::NiceMock<MockThemeManager>>();
    auto themeFor = [](ThemeType type) -> RefPtr<Theme> {
        if (type == SelectTheme::TypeId()) {
            return AceType::MakeRefPtr<SelectTheme>();
        }
        if (type == TextTheme::TypeId()) {
            return AceType::MakeRefPtr<TextTheme>();
        }
        if (type == IconTheme::TypeId()) {
            return AceType::MakeRefPtr<IconTheme>();
        }
        if (type == ScrollBarTheme::TypeId()) {
            return AceType::MakeRefPtr<ScrollBarTheme>();
        }
        return nullptr;
    };
    ON_CALL(*themes, GetTheme(testing::_)).WillByDefault(themeFor);
    ON_CALL(*themes, GetTheme(testing::_, testing::_))
        .WillByDefault([themeFor](ThemeType type, int32_t) { return themeFor(type); });
    pipeline_->SetThemeManager(themes);
    SelectModelNG model;
    model.Create({ { "first", "" }, { "second", "" } });
    model.SetValue("selected value");
    auto select = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(select, nullptr);
    auto label = select->GetPattern<SelectPattern>()->text_;
    ASSERT_NE(label, nullptr);
    for (auto frame = label; frame; frame = frame->GetAncestorNodeOfFrame(false)) {
        PrepareNode(frame);
    }
    auto textPattern = label->GetPattern<TextPattern>();
    auto modifier = AceType::MakeRefPtr<TextContentModifier>(std::nullopt, WeakPtr<Pattern>(textPattern));
    modifier->ContentChangeReport();
    EXPECT_TRUE(Contains("selected value"));
    select->GetPattern<SelectPattern>()->SetValue("updated selection");
    EXPECT_FALSE(Contains("selected value"));
    modifier->ContentChangeReport();
    EXPECT_TRUE(Contains("updated selection"));
}

TEST_F(PageTextCollectionTest, DirectLayoutContentRequiresPresentationRecord)
{
    auto text = Text(u"old label");
    auto pattern = text->GetPattern<TextPattern>();
    auto property = text->GetLayoutProperty<TextLayoutProperty>();
    property->UpdateContent(u"new label");
    EXPECT_FALSE(Contains("old label"));
    EXPECT_FALSE(Contains("new label"));
    // Exercise the real drawing-report path without a translation/content-change manager.
    pipeline_->contentChangeMgr_.Reset();
    pipeline_->uiTranslateManager_.reset();
    auto modifier = AceType::MakeRefPtr<TextContentModifier>(std::nullopt, WeakPtr<Pattern>(pattern));
    modifier->ContentChangeReport();
    EXPECT_TRUE(Contains("new label"));
    EXPECT_EQ(pattern->textForDisplay_, u"old label");
    EXPECT_EQ(pattern->GetPageTranslateTextForReport(), "old label");
    property->UpdateContent(u"");
    EXPECT_EQ(Collect()->GetArraySize(), 0);
}

// LazyColumn, LazyGrid, LazyWaterFlow and ListItemGroup use this shared preload entry.
TEST_F(PageTextCollectionTest, MountedPreloadPreservesVisibleText)
{
    auto host = CreateNode("PreloadHost");
    auto text = Text(u"visible before and after preload");
    host->AddChild(text);
    host->AttachToMainTree();
    ASSERT_TRUE(host->IsOnMainTree());
    ASSERT_TRUE(text->IsOnMainTree());
    ASSERT_TRUE(Contains("visible before and after preload"));

    FrameNode::ProcessOffscreenNode(host, true);
    ASSERT_TRUE(host->UseOffscreenProcess());
    ASSERT_TRUE(host->IsOnMainTree());
    ASSERT_TRUE(host->IsActive());
    ASSERT_TRUE(text->IsActive());
    EXPECT_TRUE(Contains("visible before and after preload"));

    // A reattached descendant can clear its own flag while the mounted ancestor retains one.
    text->OnAttachToMainTree(true);
    ASSERT_FALSE(text->UseOffscreenProcess());
    ASSERT_TRUE(host->UseOffscreenProcess());
    EXPECT_TRUE(Contains("visible before and after preload"));

    host->SetActive(false);
    EXPECT_FALSE(Contains("visible before and after preload"));
    host->SetActive(true);
    EXPECT_TRUE(Contains("visible before and after preload"));
    host->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    EXPECT_FALSE(Contains("visible before and after preload"));
}

TEST_F(PageTextCollectionTest, DetachedOffscreenPreloadStaysExcluded)
{
    auto host = CreateNode("DetachedPreloadHost");
    auto text = Text(u"detached preload");
    host->AddChild(text);
    FrameNode::ProcessOffscreenNode(host, true);
    ASSERT_TRUE(host->UseOffscreenProcess());
    ASSERT_FALSE(host->IsOnMainTree());
    ASSERT_TRUE(host->IsActive());
    EXPECT_FALSE(Contains("detached preload"));
}

TEST_F(PageTextCollectionTest, DrawReportWithManagerUpdatesBothRecords)
{
    auto text = Text(u"old label");
    auto pattern = text->GetPattern<TextPattern>();
    ASSERT_NE(pipeline_->GetContentChangeManager(), nullptr);
    text->GetLayoutProperty<TextLayoutProperty>()->UpdateContent(u"new label");
    auto modifier = AceType::MakeRefPtr<TextContentModifier>(std::nullopt, WeakPtr<Pattern>(pattern));
    modifier->ContentChangeReport();
    EXPECT_EQ(pattern->lastDrawnPageTranslateContent_, u"old label");
    EXPECT_EQ(pattern->lastDrawnPageTextContent_, u"new label");
    EXPECT_TRUE(Contains("new label"));
}

TEST_F(PageTextCollectionTest, SymbolDrawDoesNotMarkTextRecords)
{
    auto symbol = CreateNode("SymbolGlyph", AceType::MakeRefPtr<TextPattern>());
    auto pattern = symbol->GetPattern<TextPattern>();
    SetText(symbol, u"old symbol");
    symbol->GetLayoutProperty<TextLayoutProperty>()->UpdateContent(u"new symbol");
    auto modifier = AceType::MakeRefPtr<TextContentModifier>(std::nullopt, WeakPtr<Pattern>(pattern));
    modifier->ContentChangeReport();
    EXPECT_EQ(pattern->lastDrawnPageTextContent_, u"old symbol");
}

class PageTextReadOnlyParent : public UINode {
    DECLARE_ACE_TYPE(PageTextReadOnlyParent, UINode);
public:
    explicit PageTextReadOnlyParent(int32_t id) : UINode("ReadOnlyParent", id) {}
    bool IsAtomicNode() const override { return false; }
    const std::list<RefPtr<UINode>>& GetChildren(bool notDetach = false) const override
    {
        ++childrenCalls;
        return UINode::GetChildren(notDetach);
    }
    mutable int childrenCalls = 0;
};

TEST_F(PageTextCollectionTest, ScanDoesNotEnumerateChildrenOrMutateRegistry)
{
    auto parent = AceType::MakeRefPtr<PageTextReadOnlyParent>(ElementRegister::GetInstance()->MakeUniqueId());
    ElementRegister::GetInstance()->AddUINode(parent);
    auto text = Text(u"existing child");
    parent->AddChild(text);
    const auto calls = parent->childrenCalls;
    const auto listeners = pipeline_->uiTranslateManager_->listenerMap_.size();
    const auto nodes = ElementRegister::GetInstance()->GetNodeNum();
    EXPECT_TRUE(Contains("existing child"));
    EXPECT_TRUE(Contains("existing child"));
    EXPECT_EQ(parent->childrenCalls, calls);
    EXPECT_EQ(pipeline_->uiTranslateManager_->listenerMap_.size(), listeners);
    EXPECT_EQ(ElementRegister::GetInstance()->GetNodeNum(), nodes);
}

TEST_F(PageTextCollectionTest, ProgressUsesGeneratedTextOnly)
{
    constexpr double maxProgressValue = 100.0;
    constexpr double progressValue = 37.0;
    auto progress = CreateNode(V2::PROGRESS_ETS_TAG, AceType::MakeRefPtr<ProgressPattern>());
    auto text = Text(u"");
    progress->AddChild(text);
    auto paint = progress->GetPaintProperty<ProgressPaintProperty>();
    paint->UpdateMaxValue(maxProgressValue); // A full range of 100 makes the value a percentage.
    paint->UpdateValue(progressValue); // Produces the expected "37%" label.
    paint->UpdateEnableShowText(true);
    ProgressModelNG::SetText(AceType::RawPtr(progress), std::nullopt);
    SetText(text, text->GetLayoutProperty<TextLayoutProperty>()->GetContentValue(u""));
    EXPECT_TRUE(Contains("37%"));
    paint->UpdateEnableShowText(false);
    ProgressModelNG::SetText(AceType::RawPtr(progress), std::nullopt);
    SetText(text, text->GetLayoutProperty<TextLayoutProperty>()->GetContentValue(u""));
    EXPECT_EQ(Collect()->GetArraySize(), 0);
    ProgressModelNG::SetText(AceType::RawPtr(progress), "resource content");
    SetText(text, text->GetLayoutProperty<TextLayoutProperty>()->GetContentValue(u""));
    EXPECT_TRUE(Contains("resource content"));
}

TEST_F(PageTextCollectionTest, BackgroundAlignmentUsesHostWindowOffset)
{
    constexpr double expectedTextX = 160.0;
    constexpr double expectedTextY = 235.0;
    constexpr float textWidth = 20.0f;
    constexpr float textHeight = 10.0f;
    auto host = CreateNode(V2::BLANK_ETS_TAG);
    auto root = CreateNode(V2::COLUMN_ETS_TAG);
    auto text = Text(u"background position");
    root->AddChild(text);
    host->backgroundNode_ = root;
    host->GetRenderContext()->UpdateBuilderBackgroundFlag(true);
    host->GetRenderContext()->UpdateBackgroundAlign(Alignment::CENTER);
    host->GetGeometryNode()->SetFrameSize(SizeF(200.0f, 100.0f));
    root->GetGeometryNode()->SetFrameSize(SizeF(100.0f, 40.0f));
    text->GetGeometryNode()->SetFrameSize(SizeF(textWidth, textHeight));
    AceType::DynamicCast<MockRenderContext>(host->GetRenderContext())->SetPaintRectWithTransform(
        RectF(100.0f, 200.0f, 200.0f, 100.0f));
    AceType::DynamicCast<MockRenderContext>(root->GetRenderContext())->SetPaintRectWithTransform(
        RectF(0.0f, 0.0f, 100.0f, 40.0f));
    AceType::DynamicCast<MockRenderContext>(text->GetRenderContext())->SetPaintRectWithTransform(
        RectF(10.0f, 5.0f, textWidth, textHeight));
    auto texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), 1);
    auto item = texts->GetArrayItem(0);
    EXPECT_EQ(item->GetInt("id"), text->GetId());
    auto rect = item->GetValue("rect");
    // x = host origin 100 + centered offset (200 - 100) / 2 + text offset 10 = 160.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(0)->GetDouble(), expectedTextX);
    // y = host origin 200 + centered offset (100 - 40) / 2 + text offset 5 = 235.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(1)->GetDouble(), expectedTextY);
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(2)->GetDouble(), textWidth); // Index 2 is the text frame width.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(3)->GetDouble(), textHeight); // Index 3 is the text frame height.
}

TEST_F(PageTextCollectionTest, NestedBackgroundUsesOuterHostPosition)
{
    constexpr double expectedTextX = 63.0;
    constexpr double expectedTextY = 69.0;
    auto host = CreateNode(V2::BLANK_ETS_TAG);
    auto root = CreateNode(V2::COLUMN_ETS_TAG);
    auto innerHost = CreateNode(V2::BLANK_ETS_TAG);
    auto innerRoot = CreateNode(V2::COLUMN_ETS_TAG);
    auto text = Text(u"nested background");
    root->AddChild(innerHost);
    innerRoot->AddChild(text);
    host->backgroundNode_ = root;
    innerHost->backgroundNode_ = innerRoot;
    host->GetRenderContext()->UpdateBuilderBackgroundFlag(true);
    innerHost->GetRenderContext()->UpdateBuilderBackgroundFlag(true);
    AceType::DynamicCast<MockRenderContext>(host->GetRenderContext())->SetPaintRectWithTransform(
        RectF(50.0f, 60.0f, FRAME_WIDTH, FRAME_HEIGHT));
    AceType::DynamicCast<MockRenderContext>(innerHost->GetRenderContext())->SetPaintRectWithTransform(
        RectF(10.0f, 5.0f, FRAME_WIDTH, FRAME_HEIGHT));
    AceType::DynamicCast<MockRenderContext>(text->GetRenderContext())->SetPaintRectWithTransform(
        RectF(3.0f, 4.0f, FRAME_WIDTH, FRAME_HEIGHT));
    auto texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), 1);
    auto rect = texts->GetArrayItem(0)->GetValue("rect");
    // x = outer host 50 + inner host 10 + text offset 3 = 63.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(0)->GetDouble(), expectedTextX);
    // y = outer host 60 + inner host 5 + text offset 4 = 69.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(1)->GetDouble(), expectedTextY);
    host->SetActive(false);
    EXPECT_EQ(Collect()->GetArraySize(), 0);
}

TEST_F(PageTextCollectionTest, NestedBackgroundComposesScale)
{
    constexpr double expectedOuterScaledTextX = 26.0;
    constexpr double expectedOuterScaledTextY = 22.0;
    constexpr double expectedNestedScaledTextX = 94.0;
    constexpr double expectedNestedScaledTextY = 87.0;
    constexpr double expectedCenteredTextX = 174.0;
    constexpr double expectedCenteredTextY = 94.5;
    auto host = CreateNode(V2::BLANK_ETS_TAG);
    auto root = CreateNode(V2::COLUMN_ETS_TAG);
    auto innerHost = CreateNode(V2::BLANK_ETS_TAG);
    auto innerRoot = CreateNode(V2::COLUMN_ETS_TAG);
    auto text = Text(u"scaled nested background");
    root->AddChild(innerHost);
    innerRoot->AddChild(text);
    SetUpBackground(host, root);
    SetUpBackground(innerHost, innerRoot);
    AceType::DynamicCast<MockRenderContext>(innerHost->GetRenderContext())->SetPaintRectWithTransform(
        RectF(10.0f, 7.0f, FRAME_WIDTH, FRAME_HEIGHT));
    AceType::DynamicCast<MockRenderContext>(text->GetRenderContext())->SetPaintRectWithTransform(
        RectF(3.0f, 4.0f, FRAME_WIDTH, FRAME_HEIGHT));

    // Outer scale (2, 2), inner scale (1, 1): (10 + 3) * 2, (7 + 4) * 2.
    host->GetGeometryNode()->SetFrameSize(SizeF(200.0f, 40.0f));
    auto texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), 1);
    auto rect = texts->GetArrayItem(0)->GetValue("rect");
    // x = (inner host offset 10 + text offset 3) * outer scale 2 = 26.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(0)->GetDouble(), expectedOuterScaledTextX);
    // y = (inner host offset 7 + text offset 4) * outer scale 2 = 22.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(1)->GetDouble(), expectedOuterScaledTextY);

    // Outer scale (2, 3), inner scale (4, 0.5), with a nonzero window origin.
    host->GetGeometryNode()->SetFrameSize(SizeF(200.0f, 60.0f));
    innerHost->GetGeometryNode()->SetFrameSize(SizeF(400.0f, 10.0f));
    AceType::DynamicCast<MockRenderContext>(host->GetRenderContext())->SetPaintRectWithTransform(
        RectF(50.0f, 60.0f, 200.0f, 60.0f));
    texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), 1);
    auto item = texts->GetArrayItem(0);
    EXPECT_EQ(item->GetInt("id"), text->GetId());
    rect = item->GetValue("rect");
    // The window origin and scaled inner-host/text offsets produce x = 94 and y = 87.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(0)->GetDouble(), expectedNestedScaledTextX); // 50 + 10 * 2 + 3 * 4 * 2
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(1)->GetDouble(), expectedNestedScaledTextY); // 60 + 7 * 3 + 4 * 0.5 * 3
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(2)->GetDouble(), FRAME_WIDTH); // Index 2 is width.
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(3)->GetDouble(), FRAME_HEIGHT); // Index 3 is height.

    // Inner alignment is also expressed in the outer background's coordinate space.
    innerHost->GetRenderContext()->UpdateBackgroundAlign(Alignment::CENTER);
    innerRoot->GetGeometryNode()->SetFrameSize(SizeF(80.0f, 10.0f));
    texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), 1);
    rect = texts->GetArrayItem(0)->GetValue("rect");
    // Centering adds local offsets (10, 5) before scaling, giving the window position (174, 94.5).
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(0)->GetDouble(), expectedCenteredTextX); // 50 + 10 * 2 + (3 + 10) * 4 * 2
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(1)->GetDouble(), expectedCenteredTextY); // 60 + 7 * 3 + (4 + 5) * 0.5 * 3
}

TEST_F(PageTextCollectionTest, BackgroundHostStateAndReplacement)
{
    auto host = Text(u"host");
    auto root = CreateNode(V2::COLUMN_ETS_TAG);
    auto background = Text(u"background");
    root->AddChild(background);
    root->useOffscreenProcess_ = true;
    host->backgroundNode_ = root;
    host->GetRenderContext()->UpdateBuilderBackgroundFlag(true);
    EXPECT_TRUE(Contains("background"));
    host->SetActive(false);
    EXPECT_EQ(Collect()->GetArraySize(), 0);
    host->SetActive(true);
    host->GetRenderContext()->UpdateBuilderBackgroundFlag(false);
    EXPECT_FALSE(Contains("background"));
    host->GetRenderContext()->UpdateBuilderBackgroundFlag(true);
    EXPECT_TRUE(Contains("background"));
    SetText(background, u"new background");
    EXPECT_FALSE(Contains("background"));
    EXPECT_TRUE(Contains("new background"));
    host->SetBackgroundFunction({});
    EXPECT_FALSE(Contains("new background"));
}
class PageTextFloatingTest : public PageTextCollectionTest {
public:
    void SetUp() override
    {
        PageTextCollectionTest::SetUp();
        // Main window: screen origin (100, 200), width 800, height 600.
        pipeline_->displayWindowRectInfo_ = Rect(mainWindowX, mainWindowY, windowWidth, windowHeight);
        MockPipelineContext::SetUp();
        childPipeline_ = MockPipelineContext::GetCurrent();
        MockPipelineContext::pipeline_ = pipeline_;
        childPipeline_->instanceId_ = CHILD_ID;
        // Child window: screen origin (130, 240), offset (30, 40) from the main window, same size.
        childPipeline_->displayWindowRectInfo_ = Rect(childWindowX, childWindowY, windowWidth, windowHeight);
        auto root = CreateNode("SubwindowRoot");
        root->AttachContext(AceType::RawPtr(childPipeline_));
        childPipeline_->overlayManager_ = AceType::MakeRefPtr<OverlayManager>(root);
        childContainer_ = AceType::MakeRefPtr<testing::NiceMock<MockContainer>>(childPipeline_);
        childContainer_->SetTaskExecutor(AceType::MakeRefPtr<MockTaskExecutor>());
        ON_CALL(*childContainer_, GetInstanceId()).WillByDefault(testing::Return(CHILD_ID));
        MockContainer::SetGetContainerCallback([main = MockContainer::Current(), child = childContainer_](int32_t id) {
            return id == CHILD_ID ? child : main;
        });
        subwindow_ = AceType::MakeRefPtr<testing::NiceMock<MockSubwindow>>();
        ON_CALL(*subwindow_, GetShown()).WillByDefault([this]() { return shown_; });
        ON_CALL(*subwindow_, GetChildContainerId()).WillByDefault(testing::Return(CHILD_ID));
        ON_CALL(*subwindow_, GetOverlayManager()).WillByDefault(testing::Return(childPipeline_->overlayManager_));
        auto manager = SubwindowManager::GetInstance();
        manager->AddParentContainerId(CHILD_ID, pipeline_->GetInstanceId());
        manager->AddInstanceSubwindowMap(CHILD_ID, subwindow_);
    }

    void TearDown() override
    {
        nodes_.clear();
        childPipeline_->overlayManager_.Reset();
        SubwindowManager::GetInstance()->instanceSubwindowMap_.erase(CHILD_ID);
        SubwindowManager::GetInstance()->RemoveParentContainerId(CHILD_ID);
        subwindow_.Reset();
        MockContainer::SetGetContainerCallback({});
        childContainer_.Reset();
        childPipeline_.Reset();
        PageTextCollectionTest::TearDown();
    }

    RefPtr<FrameNode> Menu(const RefPtr<FrameNode>& target, const std::u16string& content)
    {
        auto menu = CreateNode(V2::MENU_WRAPPER_ETS_TAG, AceType::MakeRefPtr<MenuWrapperPattern>(target->GetId()));
        auto themes = AceType::MakeRefPtr<testing::NiceMock<MockThemeManager>>();
        auto themeFor = [](ThemeType type) -> RefPtr<Theme> {
            if (type == SelectTheme::TypeId()) { return AceType::MakeRefPtr<SelectTheme>(); }
            if (type == TextTheme::TypeId()) { return AceType::MakeRefPtr<TextTheme>(); }
            if (type == IconTheme::TypeId()) { return AceType::MakeRefPtr<IconTheme>(); }
            return nullptr;
        };
        ON_CALL(*themes, GetTheme(testing::_)).WillByDefault(themeFor);
        ON_CALL(*themes, GetTheme(testing::_, testing::_))
            .WillByDefault([themeFor](ThemeType type, int32_t) { return themeFor(type); });
        pipeline_->SetThemeManager(themes);
        childPipeline_->SetThemeManager(themes);
        MenuItemProperties properties;
        properties.content = UtfUtils::Str16ToStr8(content);
        MenuItemModelNG model;
        model.Create(properties);
        auto item = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
        EXPECT_NE(item, nullptr);
        menu->AddChild(item);
        item->MarkModifyDone();
        auto text = item->GetPattern<MenuItemPattern>()->GetContentNode();
        EXPECT_NE(text, nullptr);
        if (text) {
            for (auto frame = text; frame && frame != menu; frame = frame->GetAncestorNodeOfFrame(false)) {
                PrepareNode(frame);
            }
            text->GetPattern<TextPattern>()->MarkPageTranslateTextDrawn();
        }
        menu->AttachContext(AceType::RawPtr(childPipeline_), true);
        menu->GetPattern<MenuWrapperPattern>()->menuStatus_ = MenuStatus::SHOW;
        return menu;
    }

    static constexpr int32_t CHILD_ID = 100001;
    static constexpr double mainWindowX = 100.0;
    static constexpr double mainWindowY = 200.0;
    static constexpr double childWindowX = 130.0;
    static constexpr double childWindowY = 240.0;
    static constexpr double windowWidth = 800.0;
    static constexpr double windowHeight = 600.0;
    RefPtr<MockPipelineContext> childPipeline_;
    RefPtr<MockContainer> childContainer_;
    RefPtr<MockSubwindow> subwindow_;
    bool shown_ = true;
};

TEST_F(PageTextFloatingTest, ContextMenusUseOwnerScopeAndWindowCoordinates)
{
    constexpr int32_t expectedMenuCount = 2;
    constexpr double movedChildWindowX = 150.0;
    constexpr double movedChildWindowY = 260.0;
    auto button = CreateNode(V2::BUTTON_ETS_TAG);
    auto longPressMenu = Menu(button, u"T561_155_A");
    auto controlledMenu = Menu(button, u"T561_272_A");
    ASSERT_TRUE(Contains("T561_155_A"));
    ASSERT_TRUE(Contains("T561_272_A"));
    auto texts = Collect();
    // The long-press menu and controlled menu each contribute one text entry.
    ASSERT_EQ(texts->GetArraySize(), expectedMenuCount);
    auto rect = texts->GetArrayItem(0)->GetValue("rect");
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(0)->GetDouble(), childWindowX - mainWindowX);
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(1)->GetDouble(), childWindowY - mainWindowY);
    EXPECT_DOUBLE_EQ(rect->GetArrayItem(2)->GetDouble(), FRAME_WIDTH); // Index 2 is width.
    // Move the child window to screen origin (150, 260), retaining its size of 800 by 600.
    childPipeline_->displayWindowRectInfo_ =
        Rect(movedChildWindowX, movedChildWindowY, windowWidth, windowHeight);
    auto moved = Collect();
    auto movedRect = moved->GetArrayItem(0)->GetValue("rect");
    EXPECT_DOUBLE_EQ(movedRect->GetArrayItem(0)->GetDouble(), movedChildWindowX - mainWindowX);
    EXPECT_DOUBLE_EQ(movedRect->GetArrayItem(1)->GetDouble(), movedChildWindowY - mainWindowY);
    controlledMenu->GetPattern<MenuWrapperPattern>()->menuStatus_ = MenuStatus::HIDE;
    EXPECT_FALSE(Contains("T561_272_A"));
    EXPECT_TRUE(Contains("T561_155_A"));
    shown_ = false;
    EXPECT_EQ(Collect()->GetArraySize(), 0);
}

TEST_F(PageTextFloatingTest, TipsUseDisplayedPopupAndTargetOwnership)
{
    auto button = CreateNode(V2::BUTTON_ETS_TAG);
    auto popup = CreateNode(V2::POPUP_ETS_TAG, AceType::MakeRefPtr<BubblePattern>(button->GetId(), button->GetTag()));
    auto text = Text(u"T561_075_A");
    popup->AddChild(text);
    popup->AttachContext(AceType::RawPtr(childPipeline_), true);
    PopupInfo info;
    info.popupId = popup->GetId();
    info.popupNode = popup;
    info.target = button;
    info.isTips = true;
    info.isCurrentOnShow = true;
    childPipeline_->overlayManager_->popupMap_[button->GetId()] = info;
    EXPECT_TRUE(Contains("T561_075_A"));
    childPipeline_->overlayManager_->popupMap_[button->GetId()].isCurrentOnShow = false;
    EXPECT_FALSE(Contains("T561_075_A"));
    childPipeline_->overlayManager_->popupMap_[button->GetId()].isCurrentOnShow = true;
    EXPECT_TRUE(Contains("T561_075_A"));
    childPipeline_->overlayManager_->popupMap_.clear();
    EXPECT_FALSE(Contains("T561_075_A"));
}

TEST_F(PageTextFloatingTest, SubwindowDoesNotExposeUnrelatedOrCachedText)
{
    auto button = CreateNode(V2::BUTTON_ETS_TAG);
    auto menu = Menu(button, u"owned menu");
    auto unrelated = Text(u"unrelated subwindow text");
    unrelated->AttachContext(AceType::RawPtr(childPipeline_));
    EXPECT_TRUE(Contains("owned menu"));
    EXPECT_FALSE(Contains("unrelated subwindow text"));
    SubwindowManager::GetInstance()->RemoveParentContainerId(CHILD_ID);
    SubwindowManager::GetInstance()->AddParentContainerId(CHILD_ID, pipeline_->GetInstanceId() + 1);
    EXPECT_FALSE(Contains("owned menu"));
    SubwindowManager::GetInstance()->RemoveParentContainerId(CHILD_ID);
    SubwindowManager::GetInstance()->AddParentContainerId(CHILD_ID, pipeline_->GetInstanceId());
    button->instanceId_ = pipeline_->GetInstanceId() + 1;
    EXPECT_FALSE(Contains("owned menu"));
}

TEST_F(PageTextFloatingTest, MainWindowPopupAndMenuRetainExistingScope)
{
    auto button = CreateNode(V2::BUTTON_ETS_TAG);
    auto menu = Menu(button, u"same window menu");
    menu->AttachContext(AceType::RawPtr(pipeline_), true);
    auto popup = CreateNode(V2::POPUP_ETS_TAG, AceType::MakeRefPtr<BubblePattern>(button->GetId(), button->GetTag()));
    popup->AddChild(Text(u"same window popup"));
    PopupInfo info;
    info.popupNode = popup;
    info.target = button;
    info.isCurrentOnShow = true;
    pipeline_->GetOverlayManager()->popupMap_[button->GetId()] = info;
    shown_ = false;
    EXPECT_TRUE(Contains("same window menu"));
    EXPECT_TRUE(Contains("same window popup"));
    menu->GetPattern<MenuWrapperPattern>()->menuStatus_ = MenuStatus::HIDE;
    pipeline_->GetOverlayManager()->popupMap_[button->GetId()].isCurrentOnShow = false;
    EXPECT_FALSE(Contains("same window menu"));
    EXPECT_FALSE(Contains("same window popup"));
}

class PageTextOtherThreadExecutor : public MockTaskExecutor {
public:
    bool WillRunOnCurrentThread(TaskType type) const override
    {
        return type != TaskType::UI;
    }
};

TEST_F(PageTextFloatingTest, ChildWindowOnAnotherUIThreadIsNotRead)
{
    auto button = CreateNode(V2::BUTTON_ETS_TAG);
    auto menu = Menu(button, u"other thread menu");
    childContainer_->SetTaskExecutor(AceType::MakeRefPtr<PageTextOtherThreadExecutor>());
    EXPECT_FALSE(Contains("other thread menu"));
}

TEST_F(PageTextCollectionTest, SliderPaintMethodSelectsCustomAndGeneratedText)
{
    constexpr int32_t expectedTextDrawCount = 2;
    auto slider = CreateNode(V2::SLIDER_ETS_TAG, AceType::MakeRefPtr<SliderPattern>());
    auto pattern = slider->GetPattern<SliderPattern>();
    slider->GetRenderContext()->SetHostNode(slider);
    SetUpSliderThemes();
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    std::vector<std::u16string> paragraphTexts;
    // AddText runs twice: once for the custom tip and once for the generated percentage.
    EXPECT_CALL(*paragraph, AddText(testing::_)).Times(expectedTextDrawCount)
        .WillRepeatedly([&paragraphTexts](const std::u16string& text) { paragraphTexts.push_back(text); });
    auto tip = AceType::MakeRefPtr<SliderTipModifier>(slider, []() { return std::pair<OffsetF, float>(); });
    pattern->sliderTipModifier_ = tip;
    SliderPaintMethod::TipParameters tips;
    tips.isDrawTip_ = true;
    SliderPaintMethod method(nullptr, SliderContentModifier::Parameters(), FRAME_WIDTH, 0.0f, tip, tips,
        TextDirection::LTR);
    auto paint = slider->GetPaintProperty<SliderPaintProperty>();
    auto wrapper = AceType::MakeRefPtr<PaintWrapper>(slider->GetRenderContext(), slider->GetGeometryNode(), paint);
    SliderModelNG::SetShowTips(AceType::RawPtr(slider), true, "T561_053_A");
    method.UpdateOverlayModifier(AceType::RawPtr(wrapper));
    tip->sizeScale_->Set(1.0f);
    tip->opacityScale_->Set(1.0f);
    EXPECT_TRUE(Contains("T561_053_A"));
    SliderModelNG::SetShowTips(AceType::RawPtr(slider), true, std::nullopt);
    paint->UpdateMin(0.0f);
    paint->UpdateMax(100.0f);
    paint->UpdateValue(47.0f);
    pattern->CalcSliderValue();
    pattern->showTips_ = true;
    pattern->bubbleFlag_ = true;
    pattern->valueChangeFlag_ = true;
    pattern->UpdateTipsValue();
    method.UpdateOverlayModifier(AceType::RawPtr(wrapper));
    EXPECT_TRUE(Contains("47%"));
    ASSERT_EQ(paragraphTexts.size(), expectedTextDrawCount);
    EXPECT_EQ(paragraphTexts[0], u"T561_053_A");
    EXPECT_EQ(paragraphTexts[1], u"47%");
    auto texts = Collect();
    ASSERT_EQ(texts->GetArraySize(), 1);
    EXPECT_EQ(texts->GetArrayItem(0)->GetInt("id"), slider->GetId());
    // The rect array is [x, y, width, height], so index 2 contains the slider frame width.
    EXPECT_DOUBLE_EQ(texts->GetArrayItem(0)->GetValue("rect")->GetArrayItem(2)->GetDouble(), FRAME_WIDTH);
    MockParagraph::TearDown();
}

TEST_F(PageTextCollectionTest, SliderTipsCollectCustomAndGeneratedContentOnlyWhileVisible)
{
    auto slider = CreateNode(V2::SLIDER_ETS_TAG, AceType::MakeRefPtr<SliderPattern>());
    auto pattern = slider->GetPattern<SliderPattern>();
    SliderModelNG::SetShowTips(AceType::RawPtr(slider), true, "T561_053_A");
    EXPECT_FALSE(Contains("T561_053_A"));
    auto tip = AceType::MakeRefPtr<SliderTipModifier>(slider, []() { return std::pair<OffsetF, float>(); });
    pattern->sliderTipModifier_ = tip;
    tip->SetContent("T561_053_A");
    tip->tipFlag_->Set(true);
    tip->sizeScale_->Set(1.0f);
    tip->opacityScale_->Set(1.0f);
    EXPECT_FALSE(Contains("T561_053_A"));
    tip->SetParagraph(MockParagraph::GetOrCreateMockParagraph());
    EXPECT_TRUE(Contains("T561_053_A"));

    SliderModelNG::SetShowTips(AceType::RawPtr(slider), true, std::nullopt);
    pattern->showTips_ = true;
    pattern->bubbleFlag_ = true;
    pattern->valueChangeFlag_ = true;
    pattern->valueRatio_ = 0.47f;
    pattern->UpdateTipsValue();
    auto paint = slider->GetPaintProperty<SliderPaintProperty>();
    ASSERT_EQ(paint->GetContentValue(""), "47%");
    tip->SetContent(paint->GetContentValue(""));
    EXPECT_TRUE(Contains("47%"));
    EXPECT_FALSE(Contains("T561_053_A"));
    tip->tipFlag_->Set(false);
    // A delayed disappearance can still draw the tip while its scale/opacity remain visible.
    EXPECT_TRUE(Contains("47%"));
    tip->sizeScale_->Set(0.6f);
    tip->opacityScale_->Set(0.0f);
    EXPECT_FALSE(Contains("47%"));
    MockParagraph::TearDown();
}
} // namespace OHOS::Ace::NG
