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

#include "base/json/json_util.h"
#include "base/memory/ace_type.h"
#include "base/utils/time_util.h"
#include "base/utils/utf_helper.h"

#define private public
#define protected public
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/manager/page_scene/page_scene_rule_manager.h"
#undef private
#undef protected

#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t PAGE_SCENE_NO_ERROR = 0;
constexpr int32_t PAGE_SCENE_PARAM_INVALID = 4;
constexpr int32_t PAGE_SCENE_LAST_UNFINISH = 5;
constexpr int32_t TEST_PROCESS_ID = 1001;
constexpr int32_t SECOND_PROCESS_ID = 1002;
constexpr int32_t TEST_ROOT_ID = 2000;
constexpr int32_t TEST_TEXT_INPUT_ID = 2001;
constexpr int32_t TEST_TEXT_AREA_ID = 2002;
constexpr int32_t TEST_SEARCH_ID = 2003;
constexpr int32_t TEST_SEARCH_FIELD_ID = 2004;
constexpr int32_t TEST_RICH_EDITOR_ID = 2005;
constexpr int32_t TEST_PIPELINE_ROOT_ID = 2006;
constexpr int32_t TEST_STAGE_ID = 2007;
constexpr int32_t TEST_DIALOG_ID = 2008;
constexpr int32_t TEST_OVERLAY_ID = 2009;
constexpr int32_t TEST_DIALOG_INPUT_ID = 2010;
constexpr int32_t TEST_OVERLAY_INPUT_ID = 2011;
constexpr int32_t TEST_ATOMIC_NODE_ID = 2012;
constexpr int32_t TEST_ATOMIC_ROOT_ID = 2013;
constexpr int32_t TEST_STAGE_WRAPPER_ID = 2014;
constexpr int32_t TEST_MENU_BAR_ID = 2015;
constexpr int32_t TEST_MENU_INPUT_ID = 2016;
constexpr int32_t MATCH_THRESHOLD = 2;
constexpr int32_t INVALID_THRESHOLD = 0;
constexpr int32_t DEFAULT_MIN_REPORT_INTERVAL_MS = 500;
constexpr int32_t ONE_MATCHED_NODE = 1;
constexpr int32_t TWO_MATCHED_NODES = 2;
constexpr int32_t THREE_MATCHED_NODES = 3;
constexpr float ROOT_X = 0.0f;
constexpr float ROOT_Y = 0.0f;
constexpr float ROOT_WIDTH = 720.0f;
constexpr float ROOT_HEIGHT = 1280.0f;
constexpr float FIRST_NODE_X = 10.0f;
constexpr float FIRST_NODE_Y = 20.0f;
constexpr float SECOND_NODE_X = 30.0f;
constexpr float SECOND_NODE_Y = 80.0f;
constexpr float THIRD_NODE_X = 50.0f;
constexpr float THIRD_NODE_Y = 140.0f;
constexpr float INPUT_WIDTH = 200.0f;
constexpr float INPUT_HEIGHT = 40.0f;
constexpr float OFFSCREEN_NODE_Y = ROOT_HEIGHT + INPUT_HEIGHT;
constexpr float MOVED_NODE_OFFSET = 100.0f;
const std::string TEST_PAGE_NAME = "pages/Index";
const std::string TEST_RULE_SET_ID = "default_scene_rules";
const std::string TEST_RULE_ID = "text_editor_rule_001";
const std::string TEXT_EDITOR_EVENT = "TEXT_EDITOR";
const std::string TEXT_EDITOR_EXIT_EVENT = "TEXT_EDITOR_EXIT";
const std::string FIRST_INPUT_TEXT = "account";
const std::string SECOND_INPUT_TEXT = "password";
const std::string FIRST_PLACEHOLDER_TEXT = "input account";
const std::u16string FIRST_PLACEHOLDER_TEXT_U16 = u"input account";
const std::string EMPTY_RULES_JSON = R"({
    "version": 1,
    "ruleSetId": "empty_scene_rules",
    "rules": []
})";

std::string BuildRuleJson(bool includeUnfocusable = false, bool includeText = false,
    bool deduplicate = true, int32_t minReportIntervalMs = DEFAULT_MIN_REPORT_INTERVAL_MS,
    bool onlyVisible = true, bool rectCulling = false, bool includeRect = true)
{
    return std::string(R"({
        "version": 1,
        "ruleSetId": "default_scene_rules",
        "globalConfig": {
            "includeUnfocusableTextInput": )") + (includeUnfocusable ? "true" : "false") + R"(
        },
        "sourceConfig": {
            "arkui": true,
            "web": false,
            "uiExtension": false
        },
        "webRules": {
            "reserved": true
        },
        "rules": [
            {
                "ruleId": "text_editor_rule_001",
                "sceneType": "TEXT_EDITOR",
                "enabled": true,
                "scope": {
                    "onlyVisible":)" + (onlyVisible ? "true" : "false") + R"(,
                    "rectCulling":)" + (rectCulling ? "true" : "false") + R"(
                },
                "selector": {
                    "nodeTypes": ["TextInput", "TextArea", "Search", "RichEditor"]
                },
                "condition": {
                    "operator": "COUNT_GTE",
                    "threshold": )" + std::to_string(MATCH_THRESHOLD) + R"(
                },
                "report": {
                    "includeNodeIds": true,
                    "includeNodeTypes": true,
                    "includeRect": )" + (includeRect ? "true" : "false") + R"(,
                    "includeFocusable": true,
                    "includeText": )" + (includeText ? "true" : "false") + R"(
                },
                "policy": {
                    "reportOnRegister": true,
                    "deduplicate": )" + (deduplicate ? "true" : "false") + R"(,
                    "minReportIntervalMs": )" + std::to_string(minReportIntervalMs) + R"(
                }
            }
        ]
    })";
}

std::string BuildInvalidRuleJson(const std::string& sceneType, const std::string& operatorName, int32_t threshold)
{
    return std::string(R"({
        "version": 1,
        "ruleSetId": "default_scene_rules",
        "rules": [
            {
                "ruleId": "text_editor_rule_001",
                "sceneType": ")") + sceneType + R"(",
                "selector": {
                    "nodeTypes": ["TextInput"]
                },
                "condition": {
                    "operator": ")" + operatorName + R"(",
                    "threshold": )" + std::to_string(threshold) + R"(
                }
            }
        ]
    })";
}

std::string BuildUnsupportedVersionRuleJson()
{
    return R"({
        "version": 2,
        "ruleSetId": "default_scene_rules",
        "rules": [
            {
                "ruleId": "text_editor_rule_001",
                "sceneType": "TEXT_EDITOR",
                "selector": {
                    "nodeTypes": ["TextInput"]
                },
                "condition": {
                    "operator": "COUNT_GTE",
                    "threshold": 2
                }
            }
        ]
    })";
}

RefPtr<FrameNode> CreateTestNodeWithPattern(
    const std::string& tag, int32_t nodeId, const RectF& paintRect, const RefPtr<Pattern>& pattern,
    bool focusable = true)
{
    auto node = FrameNode::CreateFrameNode(tag, nodeId, pattern);
    if (!node) {
        return nullptr;
    }
    node->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    node->SetActive(true);

    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    geometryNode->SetFrameOffset(OffsetF(paintRect.GetX(), paintRect.GetY()));
    geometryNode->SetFrameSize(SizeF(paintRect.Width(), paintRect.Height()));
    node->SetGeometryNode(geometryNode);
    auto renderContext = AceType::MakeRefPtr<MockRenderContext>();
    renderContext->SetPaintRectWithTransform(paintRect);
    node->renderContext_ = renderContext;

    auto focusHub = node->GetOrCreateFocusHub();
    focusHub->SetFocusType(FocusType::NODE);
    focusHub->SetFocusable(focusable);
    return node;
}

RefPtr<FrameNode> CreateTestNode(
    const std::string& tag, int32_t nodeId, const RectF& paintRect, bool focusable = true)
{
    return CreateTestNodeWithPattern(tag, nodeId, paintRect, AceType::MakeRefPtr<Pattern>(), focusable);
}

RefPtr<FrameNode> CreatePageRoot()
{
    return CreateTestNode(V2::PAGE_ETS_TAG, TEST_ROOT_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
}

DumpStartNodeSet BuildStartNodes(const RefPtr<FrameNode>& root)
{
    DumpStartNodeSet set;
    set.dumpBeginNode = root;
    if (root) {
        set.pageStartNodes.push_back(root);
    }
    return set;
}

RefPtr<FrameNode> CreateTextInputNode(int32_t nodeId, const RectF& paintRect, bool focusable = true)
{
    return CreateTestNode(V2::TEXTINPUT_ETS_TAG, nodeId, paintRect, focusable);
}

RefPtr<FrameNode> CreateTextFieldNode(
    const std::string& tag, int32_t nodeId, const RectF& paintRect, const std::string& text, bool focusable = true,
    const std::u16string& placeholder = u"", TextInputType inputType = TextInputType::UNSPECIFIED,
    TextContentType contentType = TextContentType::UNSPECIFIED)
{
    auto node = CreateTestNodeWithPattern(tag, nodeId, paintRect, AceType::MakeRefPtr<TextFieldPattern>(), focusable);
    if (node) {
        auto pattern = node->GetPattern<TextFieldPattern>();
        if (pattern) {
            pattern->SetTextInputFlag(tag != V2::TEXTAREA_ETS_TAG);
            pattern->UpdateEditingValue(text, static_cast<int32_t>(text.length()));
        }
        auto layoutProperty = node->GetLayoutProperty<TextFieldLayoutProperty>();
        if (layoutProperty) {
            layoutProperty->UpdatePlaceholder(placeholder);
            if (tag != V2::TEXTAREA_ETS_TAG) {
                layoutProperty->UpdateMaxLines(1);
            }
            layoutProperty->UpdateTextInputType(inputType);
            layoutProperty->UpdateTextContentType(contentType);
        }
    }
    return node;
}

RefPtr<FrameNode> CreateRichEditorNode(int32_t nodeId, const RectF& paintRect, const std::string& placeholder)
{
    auto node = CreateTestNodeWithPattern(
        V2::RICH_EDITOR_ETS_TAG, nodeId, paintRect, AceType::MakeRefPtr<RichEditorPattern>());
    if (node) {
        auto layoutProperty = node->GetLayoutProperty<RichEditorLayoutProperty>();
        if (layoutProperty) {
            layoutProperty->UpdatePlaceholder(UtfUtils::Str8ToStr16(placeholder));
        }
    }
    return node;
}

void AddChild(const RefPtr<FrameNode>& parent, const RefPtr<FrameNode>& child)
{
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    child->MountToParent(parent);
}

void UpdateNodeRect(const RefPtr<FrameNode>& node, const RectF& paintRect)
{
    ASSERT_NE(node, nullptr);
    auto geometryNode = node->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameOffset(OffsetF(paintRect.GetX(), paintRect.GetY()));
    geometryNode->SetFrameSize(SizeF(paintRect.Width(), paintRect.Height()));
    auto renderContext = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
    ASSERT_NE(renderContext, nullptr);
    renderContext->SetPaintRectWithTransform(paintRect);
}

// ---- Search scene (RR-30095586) rule JSON builders for TASK-1 unit tests ----
const std::string SEARCH_RULE_SET_ID_A = "search-set-a";
const std::string SEARCH_RULE_SET_ID_B = "search-set-b";
const std::string SEARCH_RULE_SET_ID_C = "search-set-c";
const std::string SEARCH_RULE_ID = "search-rule-1";

std::string BuildValidSearchRuleJson(const std::string& ruleSetId = SEARCH_RULE_SET_ID_A)
{
    return std::string(R"({
        "ruleSetId": ")") + ruleSetId + R"(",
        "rules": [
            {
                "ruleId": "search-rule-1",
                "pageType": "Navigation",
                "pageName": "HomePage",
                "regions": [
                    { "x": 0, "y": 0, "width": 360, "height": 80 }
                ],
                "keywords": [ "search", "搜索" ],
                "components": [
                    { "type": "Image", "maxSize": { "width": 48, "height": 48 }, "maxRatio": 2.0 },
                    { "type": "Swiper", "autoPlay": true, "maxSize": { "width": 360, "height": 120 }, "maxRatio": 3.0 }
                ],
                "inspectorIds": [ "searchEntryBtn", "searchBox" ]
            }
        ]
    })";
}
} // namespace

class PageSceneRuleManagerTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void PageSceneRuleManagerTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
}

void PageSceneRuleManagerTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: PageSceneRuleManager_RegisterRuleSet001
 * @tc.desc: Test valid rule registration, duplicate registration, includeText rule, and invalid rules.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_RegisterRuleSet001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    EXPECT_EQ(manager.RegisterRuleSet(TEST_PROCESS_ID, "invalid json"), PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.RegisterRuleSet(TEST_PROCESS_ID, EMPTY_RULES_JSON), PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.RegisterRuleSet(TEST_PROCESS_ID, BuildRuleJson()), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterRuleSet(TEST_PROCESS_ID, BuildRuleJson()), PAGE_SCENE_LAST_UNFINISH);

    EXPECT_EQ(manager.RegisterRuleSet(SECOND_PROCESS_ID, BuildRuleJson(false, true)), PAGE_SCENE_NO_ERROR);
    manager.ClearProcess(SECOND_PROCESS_ID);
    EXPECT_EQ(manager.RegisterRuleSet(SECOND_PROCESS_ID, BuildUnsupportedVersionRuleJson()), PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.RegisterRuleSet(
        SECOND_PROCESS_ID, BuildInvalidRuleJson("TEXT_EDITOR", "COUNT_GT", MATCH_THRESHOLD)), PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.RegisterRuleSet(
        SECOND_PROCESS_ID, BuildInvalidRuleJson("UNKNOWN_SCENE", "COUNT_GTE", MATCH_THRESHOLD)),
        PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.RegisterRuleSet(
        SECOND_PROCESS_ID, BuildInvalidRuleJson("TEXT_EDITOR", "COUNT_GTE", INVALID_THRESHOLD)),
        PAGE_SCENE_PARAM_INVALID);
}

/**
 * @tc.name: PageSceneRuleManager_BeginGetPageScene001
 * @tc.desc: Test registered ruleSet Get, one-shot ruleJson Get, pending busy, and unregister cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_BeginGetPageScene001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    std::string ruleJson;

    EXPECT_EQ(manager.BeginGetPageScene(TEST_PROCESS_ID, "", ruleJson), PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.RegisterRuleSet(TEST_PROCESS_ID, BuildRuleJson()), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.BeginGetPageScene(TEST_PROCESS_ID, TEST_RULE_SET_ID, ruleJson), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(ruleJson, BuildRuleJson());
    EXPECT_EQ(manager.BeginGetPageScene(TEST_PROCESS_ID, TEST_RULE_SET_ID, ruleJson), PAGE_SCENE_LAST_UNFINISH);

    manager.CompleteGetPageScene(TEST_PROCESS_ID);
    ruleJson.clear();
    EXPECT_EQ(manager.BeginGetPageScene(TEST_PROCESS_ID, BuildRuleJson(), ruleJson), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(ruleJson, BuildRuleJson());
    manager.CompleteGetPageScene(TEST_PROCESS_ID);

    EXPECT_EQ(manager.UnregisterRuleSet(TEST_PROCESS_ID, TEST_RULE_SET_ID), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.BeginGetPageScene(TEST_PROCESS_ID, TEST_RULE_SET_ID, ruleJson), PAGE_SCENE_PARAM_INVALID);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene001
 * @tc.desc: Test TEXT_EDITOR matching JSON includes currentPageName, rect, focusable and no text payload by default.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene001, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(pageRoot, CreateTestNode(
        V2::TEXTAREA_ETS_TAG, TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));

    PageSceneRuleManager manager;
    auto result = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->matched);
    EXPECT_EQ(result->matchedCount, TWO_MATCHED_NODES);
    EXPECT_EQ(result->ruleSetId, TEST_RULE_SET_ID);
    EXPECT_EQ(result->ruleId, TEST_RULE_ID);
    EXPECT_EQ(result->sceneType, "TEXT_EDITOR");
    EXPECT_EQ(result->eventName, "TEXT_EDITOR");
    EXPECT_EQ(result->pageName, TEST_PAGE_NAME);

    auto sceneJson = JsonUtil::ParseJsonString(result->sceneJson);
    ASSERT_TRUE(sceneJson);
    EXPECT_EQ(sceneJson->GetString("currentPageName"), TEST_PAGE_NAME);
    EXPECT_TRUE(sceneJson->GetBool("matched"));
    EXPECT_EQ(sceneJson->GetInt("matchedCount"), TWO_MATCHED_NODES);
    EXPECT_EQ(sceneJson->GetValue("source")->GetString("type"), "ARKUI");
    EXPECT_TRUE(sceneJson->GetValue("text")->IsNull());
    EXPECT_TRUE(sceneJson->GetValue("tree")->IsNull());

    auto nodesJson = sceneJson->GetValue("nodes");
    ASSERT_TRUE(nodesJson);
    ASSERT_TRUE(nodesJson->IsArray());
    ASSERT_EQ(nodesJson->GetArraySize(), TWO_MATCHED_NODES);
    auto firstNode = nodesJson->GetArrayItem(0);
    ASSERT_TRUE(firstNode);
    EXPECT_EQ(firstNode->GetInt("nodeId"), TEST_TEXT_INPUT_ID);
    EXPECT_EQ(firstNode->GetString("nodeType"), "TextInput");
    EXPECT_TRUE(firstNode->GetBool("focusable"));
    EXPECT_TRUE(firstNode->GetValue("text")->IsNull());
    auto rectJson = firstNode->GetValue("rect");
    ASSERT_TRUE(rectJson);
    EXPECT_EQ(rectJson->GetDouble("x"), FIRST_NODE_X);
    EXPECT_EQ(rectJson->GetDouble("y"), FIRST_NODE_Y);
    EXPECT_EQ(rectJson->GetDouble("width"), INPUT_WIDTH);
    EXPECT_EQ(rectJson->GetDouble("height"), INPUT_HEIGHT);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene002
 * @tc.desc: Test unfocusable text input filtering and includeUnfocusableTextInput configuration.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene002, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT), false));

    PageSceneRuleManager manager;
    auto unmatchedResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(pageRoot), TEST_PAGE_NAME, true);
    ASSERT_TRUE(unmatchedResult.has_value());
    EXPECT_FALSE(unmatchedResult->matched);
    EXPECT_EQ(unmatchedResult->matchedCount, ONE_MATCHED_NODE);

    auto matchedResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(true), BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(matchedResult.has_value());
    EXPECT_TRUE(matchedResult->matched);
    EXPECT_EQ(matchedResult->matchedCount, TWO_MATCHED_NODES);

    auto sceneJson = JsonUtil::ParseJsonString(matchedResult->sceneJson);
    ASSERT_TRUE(sceneJson);
    auto nodesJson = sceneJson->GetValue("nodes");
    ASSERT_TRUE(nodesJson);
    ASSERT_EQ(nodesJson->GetArraySize(), TWO_MATCHED_NODES);
    auto secondNode = nodesJson->GetArrayItem(1);
    ASSERT_TRUE(secondNode);
    EXPECT_FALSE(secondNode->GetBool("focusable"));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene003
 * @tc.desc: Test Search with inner SearchField, RichEditor matching,
 * duplicate report suppression, and ClearProcess reset.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene003, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    auto searchNode = CreateTestNode(
        V2::SEARCH_ETS_TAG, TEST_SEARCH_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    AddChild(searchNode, CreateTextFieldNode(V2::SEARCH_Field_ETS_TAG, TEST_SEARCH_FIELD_ID,
        RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT), FIRST_INPUT_TEXT, true,
        FIRST_PLACEHOLDER_TEXT_U16, TextInputType::PHONE, TextContentType::PHONE_NUMBER));
    AddChild(pageRoot, searchNode);
    AddChild(pageRoot, CreateRichEditorNode(
        TEST_RICH_EDITOR_ID, RectF(THIRD_NODE_X, THIRD_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT), "rich placeholder"));

    PageSceneRuleManager manager;
    EXPECT_FALSE(manager.IsTextInputNodeType(V2::SEARCH_Field_ETS_TAG));
    EXPECT_TRUE(manager.IsTextInputNodeType(V2::RICH_EDITOR_ETS_TAG));
    EXPECT_FALSE(manager.IsTextInputNodeType(V2::TEXT_ETS_TAG));

    auto result = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(false, true), BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->matched);
    EXPECT_EQ(result->matchedCount, TWO_MATCHED_NODES);

    auto sceneJson = JsonUtil::ParseJsonString(result->sceneJson);
    ASSERT_TRUE(sceneJson);
    auto nodesJson = sceneJson->GetValue("nodes");
    ASSERT_TRUE(nodesJson);
    auto firstNode = nodesJson->GetArrayItem(0);
    ASSERT_TRUE(firstNode);
    EXPECT_EQ(firstNode->GetInt("nodeId"), TEST_SEARCH_ID);
    EXPECT_EQ(firstNode->GetString("nodeType"), "Search");
    EXPECT_EQ(firstNode->GetString("text"), FIRST_INPUT_TEXT);
    EXPECT_EQ(firstNode->GetString("placeholder"), FIRST_PLACEHOLDER_TEXT);
    EXPECT_EQ(firstNode->GetString("contentType"), "PHONE_NUMBER");
    EXPECT_EQ(firstNode->GetString("inputType"), "PhoneNumber");

    auto secondNode = nodesJson->GetArrayItem(1);
    ASSERT_TRUE(secondNode);
    EXPECT_EQ(secondNode->GetString("nodeType"), "RichEditor");
    EXPECT_EQ(secondNode->GetString("text"), "");
    EXPECT_EQ(secondNode->GetString("placeholder"), "rich placeholder");
    EXPECT_EQ(secondNode->GetString("contentType"), "");
    EXPECT_EQ(secondNode->GetString("inputType"), "");

    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, result.value()));
    EXPECT_FALSE(manager.ShouldReport(TEST_PROCESS_ID, result.value()));
    manager.ClearProcess(TEST_PROCESS_ID);
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, result.value()));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene004
 * @tc.desc: Test inner SearchField is not counted as another input control under Search.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene004, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    auto searchNode = CreateTestNode(
        V2::SEARCH_ETS_TAG, TEST_SEARCH_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    AddChild(searchNode, CreateTestNode(
        V2::SEARCH_Field_ETS_TAG, TEST_SEARCH_FIELD_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(pageRoot, searchNode);

    PageSceneRuleManager manager;
    auto result = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(pageRoot), TEST_PAGE_NAME, true);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->matched);
    EXPECT_EQ(result->matchedCount, ONE_MATCHED_NODE);

    auto sceneJson = JsonUtil::ParseJsonString(result->sceneJson);
    ASSERT_TRUE(sceneJson);
    auto nodesJson = sceneJson->GetValue("nodes");
    ASSERT_TRUE(nodesJson);
    ASSERT_EQ(nodesJson->GetArraySize(), ONE_MATCHED_NODE);
    auto firstNode = nodesJson->GetArrayItem(0);
    ASSERT_TRUE(firstNode);
    EXPECT_EQ(firstNode->GetInt("nodeId"), TEST_SEARCH_ID);
    EXPECT_EQ(firstNode->GetString("nodeType"), "Search");
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene005
 * @tc.desc: Test TEXT_EDITOR matching JSON includes placeholder or input text when includeText is true.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene005, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    AddChild(pageRoot, CreateTextFieldNode(
        V2::TEXTINPUT_ETS_TAG, TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT),
        "", true, FIRST_PLACEHOLDER_TEXT_U16, TextInputType::EMAIL_ADDRESS, TextContentType::EMAIL_ADDRESS));
    AddChild(pageRoot, CreateTextFieldNode(
        V2::TEXTAREA_ETS_TAG, TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT),
        SECOND_INPUT_TEXT, true, u"", TextInputType::NUMBER, TextContentType::PHONE_NUMBER));

    PageSceneRuleManager manager;
    auto result = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(false, true), BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->matched);
    EXPECT_EQ(result->matchedCount, TWO_MATCHED_NODES);

    auto sceneJson = JsonUtil::ParseJsonString(result->sceneJson);
    ASSERT_TRUE(sceneJson);
    EXPECT_TRUE(sceneJson->GetValue("tree")->IsNull());
    auto nodesJson = sceneJson->GetValue("nodes");
    ASSERT_TRUE(nodesJson);
    ASSERT_TRUE(nodesJson->IsArray());
    ASSERT_EQ(nodesJson->GetArraySize(), TWO_MATCHED_NODES);
    auto firstNode = nodesJson->GetArrayItem(0);
    auto secondNode = nodesJson->GetArrayItem(1);
    ASSERT_TRUE(firstNode);
    ASSERT_TRUE(secondNode);
    EXPECT_EQ(firstNode->GetString("text"), "");
    EXPECT_EQ(firstNode->GetString("placeholder"), FIRST_PLACEHOLDER_TEXT);
    EXPECT_EQ(firstNode->GetString("contentType"), "EMAIL_ADDRESS");
    EXPECT_EQ(firstNode->GetString("inputType"), "Email");
    EXPECT_EQ(secondNode->GetString("text"), SECOND_INPUT_TEXT);
    EXPECT_EQ(secondNode->GetString("placeholder"), "");
    EXPECT_EQ(secondNode->GetString("contentType"), "PHONE_NUMBER");
    EXPECT_EQ(secondNode->GetString("inputType"), "NUMBER");
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene006
 * @tc.desc: Test exit event is reported once after a reported TEXT_EDITOR scene becomes unmatched.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene006, TestSize.Level1)
{
    auto unmatchedRoot = CreatePageRoot();
    AddChild(unmatchedRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));

    PageSceneRuleManager manager;
    auto initialUnmatchedResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(unmatchedRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(initialUnmatchedResult.has_value());
    EXPECT_FALSE(initialUnmatchedResult->matched);
    EXPECT_EQ(initialUnmatchedResult->eventName, TEXT_EDITOR_EXIT_EVENT);
    EXPECT_FALSE(manager.ShouldReport(TEST_PROCESS_ID, initialUnmatchedResult.value()));

    auto matchedRoot = CreatePageRoot();
    AddChild(matchedRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(matchedRoot, CreateTextInputNode(
        TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    auto matchedResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(matchedRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(matchedResult.has_value());
    EXPECT_TRUE(matchedResult->matched);
    EXPECT_EQ(matchedResult->eventName, TEXT_EDITOR_EVENT);
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, matchedResult.value()));

    auto exitResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(unmatchedRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(exitResult.has_value());
    EXPECT_FALSE(exitResult->matched);
    EXPECT_EQ(exitResult->matchedCount, ONE_MATCHED_NODE);
    EXPECT_EQ(exitResult->eventName, TEXT_EDITOR_EXIT_EVENT);
    auto exitJson = JsonUtil::ParseJsonString(exitResult->sceneJson);
    ASSERT_TRUE(exitJson);
    EXPECT_FALSE(exitJson->GetBool("matched"));
    EXPECT_EQ(exitJson->GetString("eventName"), TEXT_EDITOR_EXIT_EVENT);
    EXPECT_EQ(exitJson->GetInt("matchedCount"), ONE_MATCHED_NODE);
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, exitResult.value()));
    EXPECT_FALSE(manager.ShouldReport(TEST_PROCESS_ID, exitResult.value()));

    auto getResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(unmatchedRoot), TEST_PAGE_NAME, true);
    ASSERT_TRUE(getResult.has_value());
    EXPECT_FALSE(getResult->matched);
    EXPECT_EQ(getResult->eventName, TEXT_EDITOR_EVENT);

    auto reenterResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(matchedRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(reenterResult.has_value());
    EXPECT_TRUE(reenterResult->matched);
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, reenterResult.value()));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene007
 * @tc.desc: Test rectCulling filters inputs outside the page viewport independently of onlyVisible.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene007, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, OFFSCREEN_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));

    PageSceneRuleManager manager;
    auto visibleOnlyResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(false, false, true, DEFAULT_MIN_REPORT_INTERVAL_MS, true, true),
        BuildStartNodes(pageRoot), TEST_PAGE_NAME, true);
    ASSERT_TRUE(visibleOnlyResult.has_value());
    EXPECT_FALSE(visibleOnlyResult->matched);
    EXPECT_EQ(visibleOnlyResult->matchedCount, ONE_MATCHED_NODE);
    ASSERT_EQ(visibleOnlyResult->nodeIds.size(), ONE_MATCHED_NODE);
    EXPECT_EQ(visibleOnlyResult->nodeIds.front(), TEST_TEXT_INPUT_ID);

    auto componentVisibleResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(false, false, true, DEFAULT_MIN_REPORT_INTERVAL_MS, true, false),
        BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(componentVisibleResult.has_value());
    EXPECT_TRUE(componentVisibleResult->matched);
    EXPECT_EQ(componentVisibleResult->matchedCount, TWO_MATCHED_NODES);

    auto allNodesResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(false, false, true, DEFAULT_MIN_REPORT_INTERVAL_MS, false, false),
        BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(allNodesResult.has_value());
    EXPECT_TRUE(allNodesResult->matched);
    EXPECT_EQ(allNodesResult->matchedCount, TWO_MATCHED_NODES);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene014
 * @tc.desc: Test onlyVisible filters a node when its own opacity or an ancestor opacity is zero.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene014, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    auto transparentNode = CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    auto visibleNode = CreateTextInputNode(
        TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    AddChild(pageRoot, transparentNode);
    AddChild(pageRoot, visibleNode);
    transparentNode->GetRenderContext()->UpdateOpacity(0.0);

    PageSceneRuleManager manager;
    auto onlyVisibleResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(pageRoot), TEST_PAGE_NAME, true);
    ASSERT_TRUE(onlyVisibleResult.has_value());
    EXPECT_FALSE(onlyVisibleResult->matched);
    EXPECT_EQ(onlyVisibleResult->matchedCount, ONE_MATCHED_NODE);

    auto rectOnlyResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(false, false, true, DEFAULT_MIN_REPORT_INTERVAL_MS, false, true),
        BuildStartNodes(pageRoot), TEST_PAGE_NAME, true);
    ASSERT_TRUE(rectOnlyResult.has_value());
    EXPECT_TRUE(rectOnlyResult->matched);
    EXPECT_EQ(rectOnlyResult->matchedCount, TWO_MATCHED_NODES);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene008
 * @tc.desc: Test coordinate-only changes are deduplicated when the matched node ID list is unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene008, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    auto textInputNode = CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    auto textAreaNode = CreateTextInputNode(
        TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    AddChild(pageRoot, textInputNode);
    AddChild(pageRoot, textAreaNode);

    PageSceneRuleManager manager;
    auto initialResult = manager.MatchPageScene(TEST_PROCESS_ID,
        BuildRuleJson(false, false, true, 0), BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(initialResult.has_value());
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, initialResult.value()));

    UpdateNodeRect(textInputNode, RectF(
        FIRST_NODE_X + MOVED_NODE_OFFSET, FIRST_NODE_Y + MOVED_NODE_OFFSET, INPUT_WIDTH, INPUT_HEIGHT));
    UpdateNodeRect(textAreaNode, RectF(
        SECOND_NODE_X + MOVED_NODE_OFFSET, SECOND_NODE_Y + MOVED_NODE_OFFSET, INPUT_WIDTH, INPUT_HEIGHT));
    auto movedResult = manager.MatchPageScene(TEST_PROCESS_ID,
        BuildRuleJson(false, false, true, 0), BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);

    ASSERT_TRUE(movedResult.has_value());
    EXPECT_EQ(movedResult->nodeIds, initialResult->nodeIds);
    EXPECT_FALSE(manager.ShouldReport(TEST_PROCESS_ID, movedResult.value()));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene009
 * @tc.desc: Test leaving the viewport or replacing an attached input changes node IDs and produces a new state.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene009, TestSize.Level1)
{
    auto matchedRoot = CreatePageRoot();
    AddChild(matchedRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    auto textAreaNode = CreateTextInputNode(
        TEST_TEXT_AREA_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    AddChild(matchedRoot, textAreaNode);

    PageSceneRuleManager manager;
    auto initialResult = manager.MatchPageScene(TEST_PROCESS_ID,
        BuildRuleJson(false, false, true, 0, true, true), BuildStartNodes(matchedRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(initialResult.has_value());
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, initialResult.value()));

    UpdateNodeRect(textAreaNode, RectF(SECOND_NODE_X, OFFSCREEN_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    auto offscreenResult = manager.MatchPageScene(TEST_PROCESS_ID,
        BuildRuleJson(false, false, true, 0, true, true), BuildStartNodes(matchedRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(offscreenResult.has_value());
    EXPECT_FALSE(offscreenResult->matched);
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, offscreenResult.value()));

    auto replacedRoot = CreatePageRoot();
    AddChild(replacedRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(replacedRoot, CreateTextInputNode(
        TEST_SEARCH_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    auto replacedResult = manager.MatchPageScene(TEST_PROCESS_ID,
        BuildRuleJson(false, false, true, 0, true, true), BuildStartNodes(replacedRoot), TEST_PAGE_NAME, false);
    ASSERT_TRUE(replacedResult.has_value());
    EXPECT_TRUE(replacedResult->matched);
    EXPECT_EQ(replacedResult->nodeIds, (std::vector<int32_t> { TEST_TEXT_INPUT_ID, TEST_SEARCH_ID }));
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, replacedResult.value()));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene010
 * @tc.desc: Test traversal from pipeline root includes inputs under page, dialog and overlay branches.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene010, TestSize.Level1)
{
    auto rootNode = CreateTestNode(
        V2::ROOT_ETS_TAG, TEST_PIPELINE_ROOT_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto stageNode = CreateTestNode(
        V2::STAGE_ETS_TAG, TEST_STAGE_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto pageRoot = CreatePageRoot();
    auto dialogNode = CreateTestNode(
        V2::DIALOG_ETS_TAG, TEST_DIALOG_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto overlayNode = CreateTestNode(
        V2::OVERLAY_ETS_TAG, TEST_OVERLAY_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));

    AddChild(rootNode, stageNode);
    AddChild(stageNode, pageRoot);
    AddChild(rootNode, dialogNode);
    AddChild(rootNode, overlayNode);
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(dialogNode, CreateTestNode(
        V2::TEXTAREA_ETS_TAG, TEST_DIALOG_INPUT_ID,
        RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(overlayNode, CreateTestNode(
        V2::SEARCH_ETS_TAG, TEST_OVERLAY_INPUT_ID,
        RectF(THIRD_NODE_X, THIRD_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));

    PageSceneRuleManager manager;
    auto pageResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(pageRoot), TEST_PAGE_NAME, true);
    ASSERT_TRUE(pageResult.has_value());
    EXPECT_FALSE(pageResult->matched);
    EXPECT_EQ(pageResult->matchedCount, ONE_MATCHED_NODE);
    EXPECT_EQ(pageResult->nodeIds, (std::vector<int32_t> { TEST_TEXT_INPUT_ID }));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene010_More
 * @tc.desc: Test traversal with production-style start nodes includes inputs under
 *           page, dialog and overlay branches; sceneJson reflects all three.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene010_More, TestSize.Level1)
{
    auto rootNode = CreateTestNode(
        V2::ROOT_ETS_TAG, TEST_PIPELINE_ROOT_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto stageNode = CreateTestNode(
        V2::STAGE_ETS_TAG, TEST_STAGE_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto pageRoot = CreatePageRoot();
    auto dialogNode = CreateTestNode(
        V2::DIALOG_ETS_TAG, TEST_DIALOG_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto overlayNode = CreateTestNode(
        V2::OVERLAY_ETS_TAG, TEST_OVERLAY_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));

    AddChild(rootNode, stageNode);
    AddChild(stageNode, pageRoot);
    AddChild(rootNode, dialogNode);
    AddChild(rootNode, overlayNode);
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(dialogNode, CreateTestNode(
        V2::TEXTAREA_ETS_TAG, TEST_DIALOG_INPUT_ID,
        RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(overlayNode, CreateTestNode(
        V2::SEARCH_ETS_TAG, TEST_OVERLAY_INPUT_ID,
        RectF(THIRD_NODE_X, THIRD_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));

    // Production-style start nodes: page branch (pageRoot) + overlay branches
    // (dialog, overlay). This mirrors GetDumpStartNodes output and avoids the
    // double count that would occur if rootNode were both a page start node
    // and an overlay parent.
    DumpStartNodeSet branchStartNodes;
    branchStartNodes.dumpBeginNode = rootNode;
    branchStartNodes.pageStartNodes.push_back(pageRoot);
    branchStartNodes.overlayNodes.push_back(dialogNode);
    branchStartNodes.overlayNodes.push_back(overlayNode);
    PageSceneRuleManager manager;
    auto rootResult = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), branchStartNodes, TEST_PAGE_NAME, true);
    ASSERT_TRUE(rootResult.has_value());
    EXPECT_TRUE(rootResult->matched);
    EXPECT_EQ(rootResult->matchedCount, THREE_MATCHED_NODES);
    EXPECT_EQ(rootResult->nodeIds,
        (std::vector<int32_t> { TEST_TEXT_INPUT_ID, TEST_DIALOG_INPUT_ID, TEST_OVERLAY_INPUT_ID }));

    auto sceneJson = JsonUtil::ParseJsonString(rootResult->sceneJson);
    ASSERT_TRUE(sceneJson);
    EXPECT_EQ(sceneJson->GetInt("matchedCount"), THREE_MATCHED_NODES);
    auto nodesJson = sceneJson->GetValue("nodes");
    ASSERT_TRUE(nodesJson);
    ASSERT_EQ(nodesJson->GetArraySize(), THREE_MATCHED_NODES);
    EXPECT_EQ(nodesJson->GetArrayItem(0)->GetInt("nodeId"), TEST_TEXT_INPUT_ID);
    EXPECT_EQ(nodesJson->GetArrayItem(1)->GetInt("nodeId"), TEST_DIALOG_INPUT_ID);
    EXPECT_EQ(nodesJson->GetArrayItem(2)->GetInt("nodeId"), TEST_OVERLAY_INPUT_ID);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene011
 * @tc.desc: Test empty traversal roots preserve the historical nullopt result.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene011, TestSize.Level1)
{
    PageSceneRuleManager manager;
    DumpStartNodeSet emptyStartNodes;
    emptyStartNodes.dumpBeginNode = CreatePageRoot();
    emptyStartNodes.atomicServiceRoot = CreateTestNode(
        V2::COLUMN_ETS_TAG, TEST_ATOMIC_ROOT_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    emptyStartNodes.pageStartNodes.push_back(nullptr);
    emptyStartNodes.overlayNodes.push_back(nullptr);

    auto result = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), emptyStartNodes, TEST_PAGE_NAME, true);
    EXPECT_FALSE(result.has_value());
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene012
 * @tc.desc: Test AtomicService page is counted once and its menu bar is outside PageScene scope.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene012, TestSize.Level1)
{
    auto rootNode = CreateTestNode(
        V2::ROOT_ETS_TAG, TEST_PIPELINE_ROOT_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto atomicNode = CreateTestNode(
        V2::ATOMIC_SERVICE_ETS_TAG, TEST_ATOMIC_NODE_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto atomicRoot = CreateTestNode(
        V2::COLUMN_ETS_TAG, TEST_ATOMIC_ROOT_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto stageWrapper = CreateTestNode(
        V2::COLUMN_ETS_TAG, TEST_STAGE_WRAPPER_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto stageNode = CreateTestNode(
        V2::STAGE_ETS_TAG, TEST_STAGE_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    auto pageRoot = CreatePageRoot();
    auto menuBar = CreateTestNode(
        V2::ROW_ETS_TAG, TEST_MENU_BAR_ID, RectF(ROOT_X, ROOT_Y, ROOT_WIDTH, ROOT_HEIGHT));
    atomicRoot->UpdateInspectorId("AtomicServiceContainerId");
    menuBar->UpdateInspectorId("AtomicServiceMenubarRowId");
    AddChild(rootNode, atomicNode);
    AddChild(atomicNode, atomicRoot);
    AddChild(atomicRoot, stageWrapper);
    AddChild(stageWrapper, stageNode);
    AddChild(stageNode, pageRoot);
    AddChild(atomicRoot, menuBar);
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    AddChild(menuBar, CreateTextInputNode(
        TEST_MENU_INPUT_ID, RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    auto startNodes =
        DumpUtil::CollectInspectorStartNodes(rootNode, nullptr, pageRoot, overlayManager);

    PageSceneRuleManager manager;
    auto result = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), startNodes, TEST_PAGE_NAME, true);

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->matched);
    EXPECT_EQ(result->matchedCount, ONE_MATCHED_NODE);
    EXPECT_EQ(result->nodeIds, (std::vector<int32_t> { TEST_TEXT_INPUT_ID }));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene013
 * @tc.desc: Test rect is omitted and not required when onlyVisible and includeRect are false.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene013, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    AddChild(pageRoot, CreateTextInputNode(
        TEST_TEXT_INPUT_ID, RectF(FIRST_NODE_X, OFFSCREEN_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT)));

    PageSceneRuleManager manager;
    auto result = manager.MatchPageScene(TEST_PROCESS_ID,
        BuildRuleJson(false, false, true, DEFAULT_MIN_REPORT_INTERVAL_MS, false, false, false),
        BuildStartNodes(pageRoot), TEST_PAGE_NAME, true);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matchedCount, ONE_MATCHED_NODE);
    auto sceneJson = JsonUtil::ParseJsonString(result->sceneJson);
    ASSERT_TRUE(sceneJson);
    auto nodeJson = sceneJson->GetValue("nodes")->GetArrayItem(0);
    ASSERT_TRUE(nodeJson);
    EXPECT_TRUE(nodeJson->GetValue("rect")->IsNull());
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageScene015
 * @tc.desc: Test enableAutoFill is carried by default for TextInput/TextArea without rule config,
 *           omitted when unset, and never carried for Search.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageScene015, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    auto textInputNode = CreateTextFieldNode(V2::TEXTINPUT_ETS_TAG, TEST_TEXT_INPUT_ID,
        RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT), FIRST_INPUT_TEXT);
    ASSERT_NE(textInputNode, nullptr);
    textInputNode->GetLayoutProperty<TextFieldLayoutProperty>()->UpdateEnableAutoFill(false);
    AddChild(pageRoot, textInputNode);
    AddChild(pageRoot, CreateTextFieldNode(V2::TEXTAREA_ETS_TAG, TEST_TEXT_AREA_ID,
        RectF(SECOND_NODE_X, SECOND_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT), SECOND_INPUT_TEXT));
    auto searchNode = CreateTestNode(
        V2::SEARCH_ETS_TAG, TEST_SEARCH_ID, RectF(THIRD_NODE_X, THIRD_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    auto searchFieldNode = CreateTextFieldNode(V2::SEARCH_Field_ETS_TAG, TEST_SEARCH_FIELD_ID,
        RectF(THIRD_NODE_X, THIRD_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT), FIRST_INPUT_TEXT);
    ASSERT_NE(searchFieldNode, nullptr);
    searchFieldNode->GetLayoutProperty<TextFieldLayoutProperty>()->UpdateEnableAutoFill(true);
    AddChild(searchNode, searchFieldNode);
    AddChild(pageRoot, searchNode);

    PageSceneRuleManager manager;
    // includeText is false here: enableAutoFill is carried by default without extra rule configuration.
    auto result = manager.MatchPageScene(
        TEST_PROCESS_ID, BuildRuleJson(), BuildStartNodes(pageRoot), TEST_PAGE_NAME, false);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->matched);
    EXPECT_EQ(result->matchedCount, THREE_MATCHED_NODES);

    auto sceneJson = JsonUtil::ParseJsonString(result->sceneJson);
    ASSERT_TRUE(sceneJson);
    auto nodesJson = sceneJson->GetValue("nodes");
    ASSERT_TRUE(nodesJson);
    ASSERT_EQ(nodesJson->GetArraySize(), THREE_MATCHED_NODES);

    auto firstNode = nodesJson->GetArrayItem(0);
    ASSERT_TRUE(firstNode);
    EXPECT_EQ(firstNode->GetInt("nodeId"), TEST_TEXT_INPUT_ID);
    EXPECT_TRUE(firstNode->GetValue("enableAutoFill")->IsBool());
    EXPECT_FALSE(firstNode->GetBool("enableAutoFill"));
    EXPECT_TRUE(firstNode->GetValue("text")->IsNull());

    auto secondNode = nodesJson->GetArrayItem(1);
    ASSERT_TRUE(secondNode);
    EXPECT_EQ(secondNode->GetInt("nodeId"), TEST_TEXT_AREA_ID);
    EXPECT_TRUE(secondNode->GetValue("enableAutoFill")->IsNull());

    // Search has no enableAutoFill API: even if its internal SearchField property is set,
    // the Search node does not carry the field.
    auto thirdNode = nodesJson->GetArrayItem(2);
    ASSERT_TRUE(thirdNode);
    EXPECT_EQ(thirdNode->GetInt("nodeId"), TEST_SEARCH_ID);
    EXPECT_TRUE(thirdNode->GetValue("enableAutoFill")->IsNull());
}

/**
 * @tc.name: PageSceneRuleManager_SearchRegister001
 * @tc.desc: Valid search rule registers through RegisterRuleSet routing and direct API, retrievable.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchRegister001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    EXPECT_EQ(manager.RegisterRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    EXPECT_TRUE(manager.HasRegisteredSearchRule(TEST_PROCESS_ID));
    auto ruleSet = manager.GetSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A);
    ASSERT_TRUE(ruleSet.has_value());
    EXPECT_EQ(ruleSet->ruleSetId, SEARCH_RULE_SET_ID_A);
    ASSERT_EQ(ruleSet->rules.size(), 1U);
    EXPECT_EQ(ruleSet->rules[0].ruleId, SEARCH_RULE_ID);
    EXPECT_EQ(ruleSet->rules[0].pageType, "Navigation");
    ASSERT_EQ(ruleSet->rules[0].regions.size(), 1U);
    EXPECT_EQ(ruleSet->rules[0].keywords.size(), 2U);
    ASSERT_EQ(ruleSet->rules[0].inspectorIds.size(), 2U);
    // Duplicate registration of the same ruleSetId is rejected (AC-9.2).
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_PARAM_INVALID);
}

/**
 * @tc.name: PageSceneRuleManager_SearchRegister002
 * @tc.desc: Invalid search configs are rejected with PARAM_INVALID, whole config fails.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchRegister002, TestSize.Level1)
{
    PageSceneRuleManager manager;
    auto expectInvalid = [&manager](const std::string& json) {
        EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, json), PAGE_SCENE_PARAM_INVALID);
    };
    expectInvalid(R"({ "ruleSetId": "", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 },
        { "x":5, "y":5, "width":10, "height":10 }] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[
        { "x":0, "y":0, "width":10, "height":10 },
        { "x":20, "y":0, "width":10, "height":10 },
        { "x":40, "y":0, "width":10, "height":10 },
        { "x":60, "y":0, "width":10, "height":10 },
        { "x":80, "y":0, "width":10, "height":10 },
        { "x":100, "y":0, "width":10, "height":10 } ] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Dialog",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "keywords":["a", "b", "c", "d", "e", "f"] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "keywords":["中文字搜索功能测试超限"] }] })");
}

/**
 * @tc.name: PageSceneRuleManager_SearchRegister002_More
 * @tc.desc: Invalid component/inspectorId configs are rejected; duplicate ruleId
 *           within a ruleSet is rejected and no partial acceptance is kept.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchRegister002_More, TestSize.Level1)
{
    PageSceneRuleManager manager;
    auto expectInvalid = [&manager](const std::string& json) {
        EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, json), PAGE_SCENE_PARAM_INVALID);
    };
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "components":[ { "type":"Image", "maxSize":{ "width":0, "height":48 } } ] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "components":[ { "type":"Image", "maxRatio":"abc" } ] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "components":[ { "type":"Swiper", "autoPlay":"true" } ] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "components":[ { "type":"Image" }, { "type":"Image" } ] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "inspectorIds":[ "" ] }] })");
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [{ "ruleId":"r1", "pageType":"Navigation",
        "pageName":"P", "regions":[{ "x":0, "y":0, "width":10, "height":10 }],
        "inspectorIds":[ 123 ] }] })");
    // Duplicate ruleId within the same ruleSet is rejected (AC-1.5).
    expectInvalid(R"({ "ruleSetId": "s1", "rules": [
        { "ruleId":"r1", "pageType":"Navigation", "pageName":"P",
          "regions":[{ "x":0, "y":0, "width":10, "height":10 }] },
        { "ruleId":"r1", "pageType":"Router", "pageName":"Q",
          "regions":[{ "x":0, "y":0, "width":10, "height":10 }] }] })");
    // No partial acceptance: a prior invalid config leaves the process unregistered.
    EXPECT_FALSE(manager.HasRegisteredSearchRule(TEST_PROCESS_ID));
}

/**
 * @tc.name: PageSceneRuleManager_SearchDualRuleSet001
 * @tc.desc: Two independent ruleSets per process register; duplicate and third are rejected;
 *           cross-ruleSet ruleId reuse and region overlap are allowed (AC-1.15/1.16/1.17).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchDualRuleSet001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_A)),
        PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_B)),
        PAGE_SCENE_NO_ERROR);
    // A third ruleSet for the same SA is rejected (AC-1.15).
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_C)),
        PAGE_SCENE_PARAM_INVALID);
    // Each ruleSet is independently retrievable.
    EXPECT_TRUE(manager.GetSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A).has_value());
    EXPECT_TRUE(manager.GetSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_B).has_value());
    // Cross-ruleSet same ruleId is allowed (AC-1.17).
    EXPECT_EQ(manager.RegisterSearchRuleSet(SECOND_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_B)),
        PAGE_SCENE_NO_ERROR);
    EXPECT_TRUE(manager.GetSearchRuleSet(SECOND_PROCESS_ID, SEARCH_RULE_SET_ID_B).has_value());
}

/**
 * @tc.name: PageSceneRuleManager_SearchUnregister001
 * @tc.desc: Unregister one ruleSet keeps the other; idempotent per ruleSet (AC-10.1).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchUnregister001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_A)),
        PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_B)),
        PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A), PAGE_SCENE_NO_ERROR);
    EXPECT_FALSE(manager.GetSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A).has_value());
    EXPECT_TRUE(manager.GetSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_B).has_value());
    EXPECT_EQ(manager.UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A), PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.UnregisterSearchRuleSet(TEST_PROCESS_ID, ""), PAGE_SCENE_PARAM_INVALID);
}

/**
 * @tc.name: PageSceneRuleManager_SearchClearProcess001
 * @tc.desc: ClearProcess removes all search ruleSets of a process.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClearProcess001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_A)),
        PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_B)),
        PAGE_SCENE_NO_ERROR);
    manager.ClearProcess(TEST_PROCESS_ID);
    EXPECT_FALSE(manager.HasRegisteredSearchRule(TEST_PROCESS_ID));
    EXPECT_FALSE(manager.GetSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_B).has_value());
}

SearchRuleSet BuildDirectSearchRuleSet()
{
    SearchRuleSet ruleSet;
    ruleSet.ruleSetId = SEARCH_RULE_SET_ID_A;
    SearchSceneRule rule;
    rule.ruleId = SEARCH_RULE_ID;
    rule.pageType = "Navigation";
    rule.pageName = "HomePage";
    rule.regions.emplace_back(PageSceneRectInfo { 0.0f, 0.0f, 360.0f, 80.0f });
    rule.keywords.emplace_back("搜索");
    ruleSet.rules.emplace_back(rule);
    return ruleSet;
}

SearchSceneSnapshot BuildDirectSnapshot()
{
    SearchSceneSnapshot snapshot;
    snapshot.pageName = "HomePage";
    snapshot.navDestinationName = "HomePage";
    snapshot.pageUrl = "pages/Home";
    SearchSceneNodeFeature node;
    node.nodeId = 1;
    node.nodeType = "Text";
    node.visibleRect = RectF(10.0f, 20.0f, 200.0f, 40.0f);
    node.keywordSources.emplace_back(SearchKeywordSource { "text", "打开搜索功能" });
    snapshot.nodes.emplace_back(node);
    return snapshot;
}

/**
 * @tc.name: PageSceneRuleManager_SearchMatcher001
 * @tc.desc: Keyword match (substring, case-insensitive), inspectorId fuzzy source and
 *           keyword OR component; component size/ratio/autoPlay AND constraints.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchMatcher001, TestSize.Level1)
{
    SearchSnapshotMatcher matcher;
    auto ruleSet = BuildDirectSearchRuleSet();
    auto snapshot = BuildDirectSnapshot();
    auto hits = matcher.Match(snapshot, ruleSet);
    ASSERT_EQ(hits.size(), 1U);
    EXPECT_EQ(hits[0].evidenceType, "keyword");
    EXPECT_EQ(hits[0].source, "text");
    EXPECT_EQ(hits[0].matchedKeyword, "搜索");

    // Case-insensitive English substring (AC-2.7).
    snapshot.nodes[0].keywordSources.clear();
    snapshot.nodes[0].keywordSources.emplace_back(SearchKeywordSource { "text", "SEARCH" });
    auto caseRule = ruleSet;
    caseRule.rules[0].keywords.clear();
    caseRule.rules[0].keywords.emplace_back("search");
    hits = matcher.Match(snapshot, caseRule);
    ASSERT_EQ(hits.size(), 1U);

    // inspectorId substring fuzzy source (AC-7.13 new semantics).
    snapshot.nodes[0].keywordSources.clear();
    snapshot.nodes[0].inspectorId = "searchEntryBtn";
    hits = matcher.Match(snapshot, caseRule);
    ASSERT_EQ(hits.size(), 1U);
    EXPECT_EQ(hits[0].source, "inspectorId");

    // Page mismatch: rule does not participate (AC-4.6).
    auto pageRule = ruleSet;
    pageRule.rules[0].pageName = "OtherPage";
    hits = matcher.Match(snapshot, pageRule);
    EXPECT_TRUE(hits.empty());
}

/**
 * @tc.name: PageSceneRuleManager_SearchMatcher002
 * @tc.desc: Component features: maxSize/maxRatio inclusive, zero height excluded,
 *           same-type AND / different-type OR.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchMatcher002, TestSize.Level1)
{
    SearchSnapshotMatcher matcher;
    SearchRuleSet ruleSet;
    ruleSet.ruleSetId = "comp-set";
    SearchSceneRule rule;
    rule.ruleId = "comp-rule";
    rule.pageType = "Router";
    rule.pageName = "pages/Home";
    rule.regions.emplace_back(PageSceneRectInfo { 0.0f, 0.0f, 360.0f, 1280.0f });
    SearchComponentFeature imageFeature;
    imageFeature.type = "Image";
    imageFeature.maxWidth = 48.0f;
    imageFeature.maxHeight = 48.0f;
    imageFeature.maxRatio = 2.0f;
    rule.components.emplace_back(imageFeature);
    ruleSet.rules.emplace_back(rule);

    SearchSceneSnapshot snapshot;
    snapshot.navDestinationName = "HomePage";
    snapshot.pageUrl = "pages/Home";
    // Image 30x20 center in region -> hits; maxRatio inclusive at 2.0.
    SearchSceneNodeFeature imageNode;
    imageNode.nodeId = 2;
    imageNode.nodeType = "Image";
    imageNode.visibleRect = RectF(0.0f, 0.0f, 40.0f, 20.0f);
    snapshot.nodes.emplace_back(imageNode);
    auto hits = matcher.Match(snapshot, ruleSet);
    ASSERT_EQ(hits.size(), 1U);
    EXPECT_EQ(hits[0].evidenceType, "component");
    EXPECT_EQ(hits[0].source, "Image");

    // maxSize violated -> no hit (same-type AND).
    snapshot.nodes[0].visibleRect = RectF(0.0f, 0.0f, 60.0f, 20.0f);
    hits = matcher.Match(snapshot, ruleSet);
    EXPECT_TRUE(hits.empty());

    // Zero visible height excluded (AC-3.8).
    snapshot.nodes[0].visibleRect = RectF(0.0f, 0.0f, 40.0f, 0.0f);
    hits = matcher.Match(snapshot, ruleSet);
    EXPECT_TRUE(hits.empty());

    // Region containment is inclusive on the boundary (AC-3.10): node center on the
    // region right edge still matches while maxSize/maxRatio constraints hold.
    snapshot.nodes[0].visibleRect = RectF(340.0f, 0.0f, 40.0f, 20.0f); // center (360,10) on right edge
    hits = matcher.Match(snapshot, ruleSet);
    ASSERT_EQ(hits.size(), 1U);
}

/**
 * @tc.name: PageSceneRuleManager_SearchPresence001
 * @tc.desc: true once, no repeat on evidence change, one false on vanish/exit,
 *           re-appearance allows another true (AC-5, AC-6).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchPresence001, TestSize.Level1)
{
    SearchPresenceStateMachine sm;
    SearchMatchEvidence evidence;
    evidence.ruleSetId = "s1";
    evidence.ruleId = "r1";
    std::vector<SearchMatchEvidence> hits { evidence };
    auto evt = sm.Update("1001:Home", "Home", true, hits);
    ASSERT_TRUE(evt.has_value());
    EXPECT_TRUE(evt->isTrue);
    EXPECT_EQ(evt->ruleIds.size(), 1U);
    EXPECT_EQ(evt->pageName, "Home");
    EXPECT_EQ(evt->hits.size(), 1U);
    EXPECT_EQ(evt->hits[0].ruleSetId, "s1");

    // Evidence changes but state PRESENT: no repeat (AC-5.2).
    EXPECT_FALSE(sm.Update("1001:Home", "Home", true, hits).has_value());

    // Feature vanish: one false (AC-6.1); idempotent on repeated no-match.
    evt = sm.Update("1001:Home", "Home", false, {});
    ASSERT_TRUE(evt.has_value());
    EXPECT_FALSE(evt->isTrue);
    EXPECT_EQ(evt->ruleIds.size(), 1U);
    EXPECT_FALSE(sm.Update("1001:Home", "Home", false, {}).has_value());

    // Re-appearance: another true (AC-6.3).
    evt = sm.Update("1001:Home", "Home", true, hits);
    ASSERT_TRUE(evt.has_value());
    EXPECT_TRUE(evt->isTrue);

    // Route leave: one false (AC-6.2); second exit emits nothing.
    evt = sm.Exit("1001:Home", "Home");
    ASSERT_TRUE(evt.has_value());
    EXPECT_FALSE(evt->isTrue);
    EXPECT_FALSE(sm.Exit("1001:Home", "Home").has_value());
}

/**
 * @tc.name: PageSceneRuleManager_SearchManagerMatch001
 * @tc.desc: Manager-level integration: registered search ruleSet matched against a
 *           snapshot; presence emits true; ClearProcess clears presence.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchManagerMatch001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    auto snapshot = BuildDirectSnapshot();
    auto hits = manager.MatchSearchScene(TEST_PROCESS_ID, snapshot);
    ASSERT_EQ(hits.size(), 1U);
    auto events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, hits);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_TRUE(events[0].isTrue);

    // Same page stable: no repeat event.
    auto events2 = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, hits);
    EXPECT_TRUE(events2.empty());

    // Route leave emits one false.
    auto exitEvent = manager.ExitSearchPresence(TEST_PROCESS_ID, "HomePage");
    ASSERT_TRUE(exitEvent.has_value());
    EXPECT_FALSE(exitEvent->isTrue);
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickMatcher001
 * @tc.desc: Inspector ID exact match first; case/prefix/substring/parent-child/
 *           nodeId negative cases do not match (AC-7.3, AC-7.5~7.10).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickMatcher001, TestSize.Level1)
{
    SearchClickMatcher matcher;
    SearchRuleSet ruleSet;
    ruleSet.ruleSetId = "click-set";
    SearchSceneRule rule;
    rule.ruleId = "click-rule";
    rule.pageType = "Navigation";
    rule.pageName = "HomePage";
    rule.regions.emplace_back(PageSceneRectInfo { 100.0f, 100.0f, 60.0f, 60.0f });
    rule.inspectorIds.emplace_back("paySearchBtn");
    ruleSet.rules.emplace_back(rule);
    SearchClickPageContext page { "HomePage", "pages/Home" };

    // Exact match -> inspectorId hit regardless of region (AC-7.3).
    SearchClickCandidate candidate;
    candidate.nodeId = 1;
    candidate.inspectorId = "paySearchBtn";
    candidate.operationId = "op-1";
    auto results = matcher.Match(candidate, page, ruleSet, nullptr);
    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0].matchType, "inspectorId");

    // Case difference -> not matched, no region fallback hit (null responseNode).
    candidate.inspectorId = "PAYSEARCHBTN";
    results = matcher.Match(candidate, page, ruleSet, nullptr);
    EXPECT_TRUE(results.empty());

    // Prefix only -> not matched (AC-7.6).
    candidate.inspectorId = "pay";
    results = matcher.Match(candidate, page, ruleSet, nullptr);
    EXPECT_TRUE(results.empty());

    // Substring only -> not matched (AC-7.7).
    candidate.inspectorId = "Search";
    results = matcher.Match(candidate, page, ruleSet, nullptr);
    EXPECT_TRUE(results.empty());

    // Internal numeric nodeId -> not matched (AC-7.10).
    candidate.inspectorId.clear();
    rule.inspectorIds.clear();
    rule.inspectorIds.emplace_back("12345");
    candidate.nodeId = 12345;
    results = matcher.Match(candidate, page, ruleSet, nullptr);
    EXPECT_TRUE(results.empty());
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickMatcher002
 * @tc.desc: Fallback from response control down max 10 layers by region OR keyword
 *           when ID is unconfigured/empty/unmatched (AC-7.4).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickMatcher002, TestSize.Level1)
{
    SearchClickMatcher matcher;
    SearchRuleSet ruleSet;
    ruleSet.ruleSetId = "click-set";
    SearchSceneRule rule;
    rule.ruleId = "click-rule";
    rule.pageType = "Router";
    rule.pageName = "pages/Home";
    rule.regions.emplace_back(PageSceneRectInfo { 100.0f, 100.0f, 60.0f, 60.0f });
    rule.keywords.emplace_back("搜索");
    // inspectorIds unconfigured -> fallback path.
    ruleSet.rules.emplace_back(rule);
    SearchClickPageContext page { "HomePage", "pages/Home" };
    SearchClickCandidate candidate;
    candidate.nodeId = 10;
    candidate.operationId = "op-2";

    // Response node subtree: grandchild Search node center inside region -> region hit.
    auto pageRoot = CreatePageRoot();
    auto responseNode = CreateTestNode(V2::ROW_ETS_TAG, 10, RectF(0.0f, 0.0f, 360.0f, 1280.0f));
    AddChild(pageRoot, responseNode);
    auto searchNode = CreateTestNode(V2::SEARCH_ETS_TAG, 11, RectF(110.0f, 110.0f, 40.0f, 40.0f));
    AddChild(responseNode, searchNode);
    auto results = matcher.Match(candidate, page, ruleSet, responseNode, 10);
    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0].matchType, "region");

    // Keyword hit in subtree -> keyword match type (checked first).
    auto keywordNode = CreateTestNode(V2::TEXT_ETS_TAG, 12, RectF(0.0f, 500.0f, 200.0f, 40.0f));
    AddChild(responseNode, keywordNode);
    // Add a text field child whose text matches the keyword.
    auto textNode = CreateTextFieldNode(V2::TEXTINPUT_ETS_TAG, 13, RectF(0.0f, 500.0f, 200.0f, 40.0f), "搜索入口");
    AddChild(responseNode, textNode);
    results = matcher.Match(candidate, page, ruleSet, responseNode, 10);
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results[0].matchType, "keyword");
}

namespace {
SearchClickCandidate BuildPendingSearchClick(int32_t nodeId, int64_t timestampMs)
{
    SearchClickCandidate candidate;
    candidate.nodeId = nodeId;
    candidate.inspectorId = "searchEntryBtn";
    candidate.nodeType = "Button";
    auto pressedTimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::milliseconds(timestampMs)).count();
    candidate.operationId = SearchClickObserver::BuildOperationId(1, 0, pressedTimeNs, nodeId);
    candidate.operationTimestampMs = timestampMs;
    return candidate;
}

int64_t SearchClickNowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::nanoseconds(GetSysTimestamp())).count();
}
} // namespace

/**
 * @tc.name: PageSceneRuleManager_SearchClickNoRules001
 * @tc.desc: Applications without search rules neither subscribe nor retain click candidates.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickNoRules001, TestSize.Level1)
{
    auto manager = std::make_shared<PageSceneRuleManager>();
    EXPECT_EQ(manager->RegisterSearchClickObserver(manager), 0U);
    const auto nowMs = SearchClickNowMs();
    for (int32_t index = 0; index < 10000; ++index) {
        manager->AddPendingClickCandidate(BuildPendingSearchClick(index, nowMs));
    }
    EXPECT_TRUE(manager->pendingClickCandidates_.empty());
    ASSERT_EQ(manager->RegisterRuleSet(TEST_PROCESS_ID, BuildRuleJson()), PAGE_SCENE_NO_ERROR);
    manager->AddPendingClickCandidate(BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs()));
    EXPECT_TRUE(manager->pendingClickCandidates_.empty());
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    SearchClickPageContext page { "HomePage", "pages/Home" };
    EXPECT_TRUE(manager->ConsumeSearchClickCandidates(page).empty());
    manager->AddPendingClickCandidate(BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs()));
    EXPECT_EQ(manager->ConsumeSearchClickCandidates(page).size(), 1U);
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickBounded001
 * @tc.desc: A stalled consumer has a bounded backlog; expired clicks are not delivered later.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickBounded001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    const auto nowMs = SearchClickNowMs();
    manager.AddPendingClickCandidate(BuildPendingSearchClick(TEST_TEXT_INPUT_ID, nowMs - 6000));
    EXPECT_TRUE(manager.pendingClickCandidates_.empty());
    auto oldest = BuildPendingSearchClick(TEST_TEXT_INPUT_ID, nowMs - 1000);
    manager.AddPendingClickCandidate(oldest);
    for (int32_t index = 0; index < 300; ++index) {
        manager.AddPendingClickCandidate(BuildPendingSearchClick(index, nowMs));
    }
    EXPECT_EQ(manager.pendingClickCandidates_.size(), 256U);
    EXPECT_EQ(manager.recentClickOperations_.size(), 256U);
    EXPECT_EQ(manager.pendingClickCandidates_.count(oldest.operationId), 0U);
    // Simulate elapsed time without sleeping in a unit test.
    for (auto& [id, candidate] : manager.pendingClickCandidates_) {
        candidate.operationTimestampMs = nowMs - 6000;
    }
    for (auto& [id, timestamp] : manager.recentClickOperations_) {
        timestamp = nowMs - 6000;
    }
    SearchClickPageContext page { "HomePage", "pages/Home" };
    EXPECT_TRUE(manager.ConsumeSearchClickCandidates(page).empty());
    EXPECT_TRUE(manager.pendingClickCandidates_.empty());
    EXPECT_TRUE(manager.recentClickOperations_.empty());
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickDeduplicate001
 * @tc.desc: DOWN and click of the same press report once, including across consumes.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickDeduplicate001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    const auto nowMs = SearchClickNowMs();
    auto down = BuildPendingSearchClick(TEST_TEXT_INPUT_ID, nowMs - 200);
    auto click = down;
    click.operationTimestampMs = nowMs;
    manager.AddPendingClickCandidate(down);
    manager.AddPendingClickCandidate(click);
    EXPECT_EQ(manager.pendingClickCandidates_.size(), 1U);
    SearchClickPageContext page { "HomePage", "pages/Home" };
    EXPECT_EQ(manager.ConsumeSearchClickCandidates(page).size(), 1U);
    manager.AddPendingClickCandidate(click);
    EXPECT_TRUE(manager.ConsumeSearchClickCandidates(page).empty());
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickRapid001
 * @tc.desc: Two independent presses on the same node 200 ms apart must both be reported.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickRapid001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    const auto nowMs = SearchClickNowMs();
    auto first = BuildPendingSearchClick(TEST_TEXT_INPUT_ID, nowMs - 200);
    auto second = BuildPendingSearchClick(TEST_TEXT_INPUT_ID, nowMs);
    ASSERT_NE(first.operationId, second.operationId);
    manager.AddPendingClickCandidate(first);
    manager.AddPendingClickCandidate(second);
    SearchClickPageContext page { "HomePage", "pages/Home" };
    EXPECT_EQ(manager.ConsumeSearchClickCandidates(page).size(), 2U);
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickIdentity001
 * @tc.desc: Operation identity separates devices, pointers, presses and response nodes.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickIdentity001, TestSize.Level1)
{
    auto id = SearchClickObserver::BuildOperationId(1, 2, 3000, 4);
    EXPECT_EQ(id, SearchClickObserver::BuildOperationId(1, 2, 3000, 4));
    EXPECT_NE(id, SearchClickObserver::BuildOperationId(2, 2, 3000, 4));
    EXPECT_NE(id, SearchClickObserver::BuildOperationId(1, 3, 3000, 4));
    EXPECT_NE(id, SearchClickObserver::BuildOperationId(1, 2, 3001, 4));
    EXPECT_NE(id, SearchClickObserver::BuildOperationId(1, 2, 3000, 5));
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickUnregister001
 * @tc.desc: Removing the last rule clears pending clicks and stale callbacks cannot refill the queue.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickUnregister001, TestSize.Level1)
{
    auto manager = std::make_shared<PageSceneRuleManager>();
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_B)),
        PAGE_SCENE_NO_ERROR);
    ASSERT_EQ(manager->RegisterSearchRuleSet(SECOND_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    auto handlerId = manager->RegisterSearchClickObserver(manager);
    ASSERT_NE(handlerId, 0U);
    auto& observer = SearchClickObserver::GetInstance();
    auto copiedCallback = observer.handlers_.at(handlerId);
    auto candidate = BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs());
    observer.Publish(candidate);
    ASSERT_EQ(manager->pendingClickCandidates_.size(), 1U);
    EXPECT_EQ(manager->UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager->searchClickObserverId_, handlerId);
    EXPECT_EQ(manager->pendingClickCandidates_.size(), 1U);
    EXPECT_EQ(manager->UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_B), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager->searchClickObserverId_, handlerId);
    manager->ClearProcess(SECOND_PROCESS_ID);
    EXPECT_EQ(manager->searchClickObserverId_, 0U);
    EXPECT_TRUE(manager->pendingClickCandidates_.empty());
    EXPECT_EQ(observer.handlers_.count(handlerId), 0U);
    copiedCallback(candidate);
    EXPECT_TRUE(manager->pendingClickCandidates_.empty());
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    EXPECT_NE(manager->RegisterSearchClickObserver(manager), 0U);
    observer.Publish(candidate);
    EXPECT_EQ(manager->pendingClickCandidates_.size(), 1U);
    EXPECT_EQ(manager->UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A), PAGE_SCENE_NO_ERROR);
    EXPECT_TRUE(manager->pendingClickCandidates_.empty());
    EXPECT_EQ(manager->searchClickObserverId_, 0U);
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickLifetime001
 * @tc.desc: A callback copied before destruction must not keep or access the destroyed manager.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickLifetime001, TestSize.Level1)
{
    auto manager = std::make_shared<PageSceneRuleManager>();
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    auto handlerId = manager->RegisterSearchClickObserver(manager);
    ASSERT_NE(handlerId, 0U);
    auto& observer = SearchClickObserver::GetInstance();
    auto copiedCallback = observer.handlers_.at(handlerId);
    std::weak_ptr<PageSceneRuleManager> weakManager = manager;
    manager.reset();
    EXPECT_TRUE(weakManager.expired());
    EXPECT_EQ(observer.handlers_.count(handlerId), 0U);
    auto candidate = BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs());
    copiedCallback(candidate);
    observer.Publish(candidate);
}

/**
 * @tc.name: PageSceneRuleManager_SearchClickLegacyClear001
 * @tc.desc: Removing legacy rules preserves the search subscription of the same process.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchClickLegacyClear001, TestSize.Level1)
{
    auto manager = std::make_shared<PageSceneRuleManager>();
    ASSERT_EQ(manager->RegisterRuleSet(TEST_PROCESS_ID, BuildRuleJson()), PAGE_SCENE_NO_ERROR);
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    auto handlerId = manager->RegisterSearchClickObserver(manager);
    ASSERT_NE(handlerId, 0U);
    manager->ClearProcess(TEST_PROCESS_ID, false);
    EXPECT_TRUE(manager->GetActiveRuleJsons().empty());
    EXPECT_TRUE(manager->HasRegisteredSearchRule(TEST_PROCESS_ID));
    EXPECT_EQ(manager->searchClickObserverId_, handlerId);
    SearchClickObserver::GetInstance().Publish(BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs()));
    SearchClickPageContext page { "HomePage", "pages/Home" };
    EXPECT_EQ(manager->ConsumeSearchClickCandidates(page).size(), 1U);
}

namespace {
std::string BuildSearchRuleWithGeneration(const std::string& generation)
{
    auto root = JsonUtil::ParseJsonString(BuildValidSearchRuleJson());
    root->Put("_pageSceneGeneration", generation.c_str());
    return root->ToString();
}
} // namespace

/**
 * @tc.name: PageSceneRuleManager_SearchGeneration001
 * @tc.desc: A newer registration replaces stale cache; old detection/cleanup cannot overwrite or remove it.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchGeneration001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const auto oldRules = BuildSearchRuleWithGeneration("9");
    const auto newRules = BuildSearchRuleWithGeneration("10");
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, oldRules), PAGE_SCENE_NO_ERROR);
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, newRules), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, oldRules), PAGE_SCENE_PARAM_INVALID);
    EXPECT_EQ(manager.UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A, "9"), PAGE_SCENE_PARAM_INVALID);
    auto current = manager.GetSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A);
    ASSERT_TRUE(current.has_value());
    EXPECT_EQ(current->registrationGeneration, "10");
    EXPECT_EQ(current->rawJson, newRules);
    EXPECT_EQ(manager.UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A, "10"), PAGE_SCENE_NO_ERROR);
    EXPECT_FALSE(manager.HasRegisteredSearchRule(TEST_PROCESS_ID));
}

/**
 * @tc.name: PageSceneRuleManager_SearchGeneration002
 * @tc.desc: Re-registering after cleanup is not undone by a delayed duplicate cleanup from the old generation.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchGeneration002, TestSize.Level1)
{
    auto manager = std::make_shared<PageSceneRuleManager>();
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildSearchRuleWithGeneration("1")), PAGE_SCENE_NO_ERROR);
    ASSERT_NE(manager->RegisterSearchClickObserver(manager), 0U);
    ASSERT_EQ(manager->UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A, "1"), PAGE_SCENE_NO_ERROR);
    ASSERT_EQ(manager->RegisterSearchRuleSet(TEST_PROCESS_ID, BuildSearchRuleWithGeneration("2")), PAGE_SCENE_NO_ERROR);
    auto newHandler = manager->RegisterSearchClickObserver(manager);
    ASSERT_NE(newHandler, 0U);
    EXPECT_EQ(manager->UnregisterSearchRuleSet(TEST_PROCESS_ID, SEARCH_RULE_SET_ID_A, "1"), PAGE_SCENE_PARAM_INVALID);
    manager->ClearProcess(TEST_PROCESS_ID, false);
    EXPECT_TRUE(manager->HasRegisteredSearchRule(TEST_PROCESS_ID));
    EXPECT_EQ(manager->searchClickObserverId_, newHandler);
}

// =========================================================================
// Regression tests for subwindow presence, source naming, stability gating,
// empty-pageName handling, visible-only subtree text, and custom component names.
// =========================================================================

namespace {
constexpr int32_t MAIN_WINDOW_NODE_ID = 100;
constexpr int32_t SUBWINDOW_NODE_ID = 200;

// Build a rule with +Dialog subwindow pageName
SearchRuleSet BuildSubwindowRuleSet(const std::string& pageName = "HomePage+Dialog")
{
    SearchRuleSet ruleSet;
    ruleSet.ruleSetId = "subwindow-set";
    SearchSceneRule rule;
    rule.ruleId = "sub-rule-1";
    rule.pageType = "Navigation";
    rule.pageName = pageName;
    rule.regions.emplace_back(PageSceneRectInfo { 0.0f, 0.0f, 360.0f, 1280.0f });
    rule.keywords.emplace_back("搜索");
    ruleSet.rules.emplace_back(rule);
    return ruleSet;
}

// Build a snapshot with both main-window and subwindow nodes
SearchSceneSnapshot BuildSubwindowSnapshot()
{
    SearchSceneSnapshot snapshot;
    snapshot.navDestinationName = "HomePage";
    snapshot.pageUrl = "pages/Home";
    // Main-window node
    SearchSceneNodeFeature mainNode;
    mainNode.nodeId = MAIN_WINDOW_NODE_ID;
    mainNode.nodeType = "Text";
    mainNode.visibleRect = RectF(10.0f, 20.0f, 200.0f, 40.0f);
    mainNode.keywordSources.emplace_back(SearchKeywordSource { "text", "搜索框" });
    mainNode.isSubwindow = false;
    snapshot.nodes.emplace_back(mainNode);
    // Subwindow node
    SearchSceneNodeFeature subNode;
    subNode.nodeId = SUBWINDOW_NODE_ID;
    subNode.nodeType = "Text";
    subNode.visibleRect = RectF(10.0f, 600.0f, 200.0f, 40.0f);
    subNode.keywordSources.emplace_back(SearchKeywordSource { "text", "搜索历史" });
    subNode.isSubwindow = true;
    snapshot.nodes.emplace_back(subNode);
    return snapshot;
}
} // namespace

/**
 * @tc.name: PageSceneRuleManager_SearchSubwindowPageMatch001
 * @tc.desc: Rule with +Dialog pageName matches when snapshot.navDestinationName
 *           is the base name (AC-4.5: subwindow page matching).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubwindowPageMatch001, TestSize.Level1)
{
    SearchSnapshotMatcher matcher;
    auto ruleSet = BuildSubwindowRuleSet("HomePage+Dialog");
    auto snapshot = BuildSubwindowSnapshot();
    // Page match: rule "HomePage+Dialog" should match base "HomePage"
    auto hits = matcher.Match(snapshot, ruleSet);
    // Only the subwindow node should match (isSubwindow filtering)
    ASSERT_EQ(hits.size(), 1U);
    EXPECT_EQ(hits[0].nodeId, 200);
    EXPECT_TRUE(hits[0].isSubwindow);
    EXPECT_EQ(hits[0].subwindowSuffix, "+Dialog");
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubwindowPageMatch002
 * @tc.desc: Rule with +Sheet pageName also matches base navDestinationName.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubwindowPageMatch002, TestSize.Level1)
{
    SearchSnapshotMatcher matcher;
    auto ruleSet = BuildSubwindowRuleSet("HomePage+Sheet");
    auto snapshot = BuildSubwindowSnapshot();
    auto hits = matcher.Match(snapshot, ruleSet);
    ASSERT_EQ(hits.size(), 1U);
    EXPECT_EQ(hits[0].nodeId, 200);
    EXPECT_TRUE(hits[0].isSubwindow);
    EXPECT_EQ(hits[0].subwindowSuffix, "+Sheet");
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubwindowNodeFilter001
 * @tc.desc: Subwindow rule (+Dialog) only matches isSubwindow nodes; main-window
 *           rule only matches non-subwindow nodes (AC-4.5 node filtering).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubwindowNodeFilter001, TestSize.Level1)
{
    SearchSnapshotMatcher matcher;
    auto snapshot = BuildSubwindowSnapshot();
    // Main-window rule should only match the main node
    auto mainRuleSet = BuildDirectSearchRuleSet(); // pageName="HomePage"
    auto mainHits = matcher.Match(snapshot, mainRuleSet);
    ASSERT_EQ(mainHits.size(), 1U);
    EXPECT_EQ(mainHits[0].nodeId, 100);
    EXPECT_FALSE(mainHits[0].isSubwindow);
    // Subwindow rule should only match the subwindow node
    auto subRuleSet = BuildSubwindowRuleSet("HomePage+Dialog");
    auto subHits = matcher.Match(snapshot, subRuleSet);
    ASSERT_EQ(subHits.size(), 1U);
    EXPECT_EQ(subHits[0].nodeId, 200);
    EXPECT_TRUE(subHits[0].isSubwindow);
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubwindowPresence001
 * @tc.desc: UpdateSearchPresence splits main/subwindow hits into independent
 *           presence states; main key=pid:pageName, sub key=pid:pageName+Dialog.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubwindowPresence001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID,
        BuildSubwindowRuleSet().ruleSetId.empty() ? BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_B) :
            BuildValidSearchRuleJson(SEARCH_RULE_SET_ID_B)),
        PAGE_SCENE_NO_ERROR);

    auto snapshot = BuildSubwindowSnapshot();
    // Manually build hits for both main and subwindow
    SearchMatchEvidence mainHit;
    mainHit.ruleSetId = SEARCH_RULE_SET_ID_A;
    mainHit.ruleId = SEARCH_RULE_ID;
    mainHit.nodeId = 100;
    mainHit.isSubwindow = false;
    SearchMatchEvidence subHit;
    subHit.ruleSetId = SEARCH_RULE_SET_ID_B;
    subHit.ruleId = "sub-rule-1";
    subHit.nodeId = 200;
    subHit.isSubwindow = true;
    std::vector<SearchMatchEvidence> hits { mainHit, subHit };

    auto events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, hits);
    // Two events: main (isTrue, pageName="HomePage") + subwindow (isTrue, pageName="HomePage+Dialog")
    ASSERT_EQ(events.size(), 2U);
    EXPECT_TRUE(events[0].isTrue);
    EXPECT_EQ(events[0].pageName, "HomePage");
    EXPECT_FALSE(events[0].anySubwindow);
    EXPECT_TRUE(events[1].isTrue);
    EXPECT_EQ(events[1].pageName, "HomePage+Dialog");
    EXPECT_TRUE(events[1].anySubwindow);
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubwindowPresence002
 * @tc.desc: Only main-window hits produce a single main presence event (no subwindow event).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubwindowPresence002, TestSize.Level1)
{
    PageSceneRuleManager manager;
    SearchSceneSnapshot snapshot;
    snapshot.navDestinationName = "HomePage";
    snapshot.pageUrl = "pages/Home";
    SearchMatchEvidence mainHit;
    mainHit.ruleSetId = "s1";
    mainHit.ruleId = "r1";
    mainHit.isSubwindow = false;
    std::vector<SearchMatchEvidence> hits { mainHit };

    auto events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, hits);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0].pageName, "HomePage");
    EXPECT_FALSE(events[0].anySubwindow);
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubwindowPresence003
 * @tc.desc: Only subwindow hits produce a single subwindow presence event with +Dialog.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubwindowPresence003, TestSize.Level1)
{
    PageSceneRuleManager manager;
    SearchSceneSnapshot snapshot;
    snapshot.navDestinationName = "HomePage";
    snapshot.pageUrl = "pages/Home";
    SearchMatchEvidence subHit;
    subHit.ruleSetId = "s1";
    subHit.ruleId = "r1";
    subHit.isSubwindow = true;
    std::vector<SearchMatchEvidence> hits { subHit };

    auto events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, hits);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0].pageName, "HomePage+Dialog");
    EXPECT_TRUE(events[0].anySubwindow);
}

/**
 * @tc.name: PageSceneRuleManager_SearchExitOtherEmptyPageName001
 * @tc.desc: ExitOtherSearchPresence with empty currentPageName returns no events
 *           and does not clear PRESENT states (AC-5.1 guard).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchExitOtherEmptyPageName001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    SearchSceneSnapshot snapshot;
    snapshot.navDestinationName = "HomePage";
    SearchMatchEvidence hit;
    hit.ruleSetId = "s1";
    hit.ruleId = "r1";
    hit.isSubwindow = false;
    // Establish PRESENT state
    auto events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { hit });
    ASSERT_EQ(events.size(), 1U);
    EXPECT_TRUE(events[0].isTrue);

    // Empty pageName: no exits, PRESENT state preserved
    auto exitEvents = manager.ExitOtherSearchPresence(TEST_PROCESS_ID, "");
    EXPECT_TRUE(exitEvents.empty());
    // Verify the page is still PRESENT via Exit (should emit false)
    auto exitEvt = manager.ExitSearchPresence(TEST_PROCESS_ID, "HomePage");
    ASSERT_TRUE(exitEvt.has_value());
    EXPECT_FALSE(exitEvt->isTrue);
}

/**
 * @tc.name: PageSceneRuleManager_SearchExitOtherEmptyPageName002
 * @tc.desc: ExitOtherSearchPresence with non-empty currentPageName still exits
 *           other PRESENT pages normally (regression check for AC-6.4).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchExitOtherEmptyPageName002, TestSize.Level1)
{
    PageSceneRuleManager manager;
    SearchSceneSnapshot snapshotA;
    snapshotA.navDestinationName = "PageA";
    SearchMatchEvidence hitA;
    hitA.ruleSetId = "s1";
    hitA.ruleId = "r1";
    hitA.isSubwindow = false;
    manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshotA, { hitA });

    // Navigate to PageB: should exit PageA
    auto exitEvents = manager.ExitOtherSearchPresence(TEST_PROCESS_ID, "PageB");
    ASSERT_EQ(exitEvents.size(), 1U);
    EXPECT_FALSE(exitEvents[0].isTrue);
    EXPECT_EQ(exitEvents[0].pageName, "PageA");
}

/**
 * @tc.name: PageSceneRuleManager_SearchFlushPendingClicksStable001
 * @tc.desc: FlushPendingSearchClicksIfStable with no stability gate set always
 *           consumes candidates (gate function not injected).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchFlushPendingClicksStable001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    SearchClickPageContext page { "HomePage", "pages/Home" };
    // No candidates -> empty result
    EXPECT_TRUE(manager.FlushPendingSearchClicksIfStable(page).empty());
    // Add a candidate and consume
    manager.AddPendingClickCandidate(BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs()));
    EXPECT_EQ(manager.FlushPendingSearchClicksIfStable(page).size(), 1U);
}

/**
 * @tc.name: PageSceneRuleManager_SearchFlushPendingClicksStable002
 * @tc.desc: FlushPendingSearchClicksIfStable with a gate returning false (not stable)
 *           skips consumption and keeps candidates.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchFlushPendingClicksStable002, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    // Inject a gate that always returns false (not stable)
    manager.SetIsContentStableFunction([]() -> bool { return false; });
    SearchClickPageContext page { "HomePage", "pages/Home" };
    manager.AddPendingClickCandidate(BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs()));
    // Not stable: no consumption
    EXPECT_TRUE(manager.FlushPendingSearchClicksIfStable(page).empty());
    EXPECT_EQ(manager.pendingClickCandidates_.size(), 1U);
}

/**
 * @tc.name: PageSceneRuleManager_SearchFlushPendingClicksStable003
 * @tc.desc: FlushPendingSearchClicksIfStable with a gate returning true (stable)
 *           consumes candidates normally.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchFlushPendingClicksStable003, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    // Inject a gate that always returns true (stable)
    manager.SetIsContentStableFunction([]() -> bool { return true; });
    SearchClickPageContext page { "HomePage", "pages/Home" };
    manager.AddPendingClickCandidate(BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs()));
    // Stable: consumption proceeds
    EXPECT_EQ(manager.FlushPendingSearchClicksIfStable(page).size(), 1U);
    EXPECT_TRUE(manager.pendingClickCandidates_.empty());
}

/**
 * @tc.name: PageSceneRuleManager_SearchSourceNaming001
 * @tc.desc: Placeholder text and input values use the "placeholderValue" source name.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSourceNaming001, TestSize.Level1)
{
    // Build a text field node with placeholder
    auto pageRoot = CreatePageRoot();
    auto textField = CreateTextFieldNode(V2::TEXTINPUT_ETS_TAG, TEST_TEXT_INPUT_ID,
        RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT), "search text", true,
        u"搜索占位符");
    AddChild(pageRoot, textField);

    // Build snapshot directly via the snapshot builder
    SearchSceneSnapshotBuilder builder;
    SearchWindowContext ctx;
    ctx.root = pageRoot;
    ctx.isSubwindow = false;
    auto snapshot = builder.Build("pages/Index", "", "pages/Index", { ctx });
    ASSERT_FALSE(snapshot.nodes.empty());

    // Find the text field node
    const SearchSceneNodeFeature* tfNode = nullptr;
    for (const auto& node : snapshot.nodes) {
        if (node.nodeId == TEST_TEXT_INPUT_ID) {
            tfNode = &node;
            break;
        }
    }
    ASSERT_NE(tfNode, nullptr);

    // Check that placeholder is reported as "placeholderValue" source
    bool foundPlaceholderValue = false;
    bool foundLegacyPlaceholder = false;
    for (const auto& src : tfNode->keywordSources) {
        if (src.name == "placeholderValue") {
            foundPlaceholderValue = true;
        }
        if (src.name == "placeholder" || src.name == "value") {
            foundLegacyPlaceholder = true;
        }
    }
    EXPECT_TRUE(foundPlaceholderValue);
    EXPECT_FALSE(foundLegacyPlaceholder);
}

/**
 * @tc.name: PageSceneRuleManager_SearchSourceNaming002
 * @tc.desc: Accessibility text uses the "accessibility" source name without a legacy alias.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSourceNaming002, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    auto textNode = CreateTestNode(V2::TEXT_ETS_TAG, TEST_TEXT_INPUT_ID,
        RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    ASSERT_NE(textNode, nullptr);
    // Set accessibility text
    auto a11yProp = textNode->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(a11yProp, nullptr);
    a11yProp->SetAccessibilityText("搜索入口");
    AddChild(pageRoot, textNode);

    SearchSceneSnapshotBuilder builder;
    SearchWindowContext ctx;
    ctx.root = pageRoot;
    ctx.isSubwindow = false;
    auto snapshot = builder.Build("pages/Index", "", "pages/Index", { ctx });

    const SearchSceneNodeFeature* targetNode = nullptr;
    for (const auto& node : snapshot.nodes) {
        if (node.nodeId == TEST_TEXT_INPUT_ID) {
            targetNode = &node;
            break;
        }
    }
    ASSERT_NE(targetNode, nullptr);

    bool foundAccessibility = false;
    bool foundLegacyA11y = false;
    for (const auto& src : targetNode->keywordSources) {
        if (src.name == "accessibility") {
            foundAccessibility = true;
        }
        if (src.name == "a11y") {
            foundLegacyA11y = true;
        }
    }
    EXPECT_TRUE(foundAccessibility);
    EXPECT_FALSE(foundLegacyA11y);
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubtreeTextVisibleOnly001
 * @tc.desc: Subtree text is only collected from nodes with non-empty visible
 *           rect; invisible/clipped nodes are excluded (AC-2.8).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubtreeTextVisibleOnly001, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    // Create a container with accessibilityGroup(true)
    auto container = CreateTestNode(V2::COLUMN_ETS_TAG, TEST_DIALOG_ID,
        RectF(0.0f, 0.0f, 360.0f, 200.0f));
    ASSERT_NE(container, nullptr);
    auto a11yProp = container->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(a11yProp, nullptr);
    a11yProp->SetAccessibilityGroup(true);

    // Visible child with text
    auto visibleChild = CreateTestNode(V2::TEXT_ETS_TAG, TEST_DIALOG_INPUT_ID,
        RectF(10.0f, 10.0f, 100.0f, 40.0f));
    ASSERT_NE(visibleChild, nullptr);
    auto childA11y = visibleChild->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(childA11y, nullptr);
    childA11y->SetText("visible text");

    // Invisible child (zero height)
    auto invisibleChild = CreateTestNode(V2::TEXT_ETS_TAG, TEST_OVERLAY_INPUT_ID,
        RectF(10.0f, 10.0f, 100.0f, 0.0f));
    ASSERT_NE(invisibleChild, nullptr);
    auto invisA11y = invisibleChild->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(invisA11y, nullptr);
    invisA11y->SetText("invisible text");

    AddChild(container, visibleChild);
    AddChild(container, invisibleChild);
    AddChild(pageRoot, container);

    SearchSceneSnapshotBuilder builder;
    SearchWindowContext ctx;
    ctx.root = pageRoot;
    ctx.isSubwindow = false;
    auto snapshot = builder.Build("pages/Index", "", "pages/Index", { ctx });

    // Find the container node and check its subtreeText source
    const SearchSceneNodeFeature* containerNode = nullptr;
    for (const auto& node : snapshot.nodes) {
        if (node.nodeId == TEST_DIALOG_ID) {
            containerNode = &node;
            break;
        }
    }
    ASSERT_NE(containerNode, nullptr);

    bool foundSubtreeText = false;
    std::string subtreeValue;
    for (const auto& src : containerNode->keywordSources) {
        if (src.name == "subtreeText") {
            foundSubtreeText = true;
            subtreeValue = src.value;
        }
    }
    EXPECT_TRUE(foundSubtreeText);
    // Visible text should be included; invisible text should be excluded
    EXPECT_NE(subtreeValue.find("visible text"), std::string::npos);
    EXPECT_EQ(subtreeValue.find("invisible text"), std::string::npos);
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubtreeTextNoGroup001
 * @tc.desc: Default accessibilityGroup(false) retains visible subtree keyword matching.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubtreeTextNoGroup001, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    // Container without accessibilityGroup
    auto container = CreateTestNode(V2::COLUMN_ETS_TAG, TEST_DIALOG_ID,
        RectF(0.0f, 0.0f, 360.0f, 200.0f));
    ASSERT_NE(container, nullptr);
    auto a11yProp = container->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(a11yProp, nullptr);
    // Ensure accessibilityGroup is NOT set (default false)
    a11yProp->SetAccessibilityGroup(false);

    auto child = CreateTestNode(V2::TEXT_ETS_TAG, TEST_DIALOG_INPUT_ID,
        RectF(10.0f, 10.0f, 100.0f, 40.0f));
    ASSERT_NE(child, nullptr);
    auto childA11y = child->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(childA11y, nullptr);
    childA11y->SetText("child text");

    AddChild(container, child);
    AddChild(pageRoot, container);

    SearchSceneSnapshotBuilder builder;
    SearchWindowContext ctx;
    ctx.root = pageRoot;
    ctx.isSubwindow = false;
    auto snapshot = builder.Build("pages/Index", "", "pages/Index", { ctx });

    const SearchSceneNodeFeature* containerNode = nullptr;
    for (const auto& node : snapshot.nodes) {
        if (node.nodeId == TEST_DIALOG_ID) {
            containerNode = &node;
            break;
        }
    }
    ASSERT_NE(containerNode, nullptr);

    bool foundSubtreeText = false;
    for (const auto& src : containerNode->keywordSources) {
        if (src.name == "subtreeText") {
            foundSubtreeText = true;
        }
    }
    EXPECT_TRUE(foundSubtreeText);
    auto ruleSet = BuildSubwindowRuleSet("pages/Index");
    ruleSet.rules[0].pageType = "Router";
    ruleSet.rules[0].keywords = { "child text" };
    // The region contains the container center, but excludes the text center.
    ruleSet.rules[0].regions = { PageSceneRectInfo { 170.0f, 90.0f, 20.0f, 20.0f } };
    SearchSnapshotMatcher matcher;
    auto hits = matcher.Match(snapshot, ruleSet);
    ASSERT_EQ(hits.size(), 1U);
    EXPECT_EQ(hits[0].nodeId, TEST_DIALOG_ID);
    EXPECT_EQ(hits[0].source, "subtreeText");
}

/**
 * @tc.name: PageSceneRuleManager_SearchAccessibilitySourceNoAlias001
 * @tc.desc: Accessibility text has one source and is not aliased as customName.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchAccessibilitySourceNoAlias001, TestSize.Level1)
{
    auto pageRoot = CreatePageRoot();
    auto textNode = CreateTestNode(V2::TEXT_ETS_TAG, TEST_TEXT_INPUT_ID,
        RectF(FIRST_NODE_X, FIRST_NODE_Y, INPUT_WIDTH, INPUT_HEIGHT));
    ASSERT_NE(textNode, nullptr);
    auto a11yProp = textNode->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(a11yProp, nullptr);
    a11yProp->SetAccessibilityText("MySearchComponent");
    AddChild(pageRoot, textNode);

    SearchSceneSnapshotBuilder builder;
    SearchWindowContext ctx;
    ctx.root = pageRoot;
    ctx.isSubwindow = false;
    auto snapshot = builder.Build("pages/Index", "", "pages/Index", { ctx });

    const SearchSceneNodeFeature* targetNode = nullptr;
    for (const auto& node : snapshot.nodes) {
        if (node.nodeId == TEST_TEXT_INPUT_ID) {
            targetNode = &node;
            break;
        }
    }
    ASSERT_NE(targetNode, nullptr);

    size_t accessibilityCount = 0;
    for (const auto& source : targetNode->keywordSources) {
        EXPECT_NE(source.name, "customName");
        if (source.name == "accessibility" && source.value == "MySearchComponent") {
            ++accessibilityCount;
        }
    }
    EXPECT_EQ(accessibilityCount, 1U);
}

/**
 * @tc.name: PageSceneRuleManager_SearchPresenceStateExitAllExcept001
 * @tc.desc: ExitAllExcept keeps the keepKey page and exits all other PRESENT pages
 *           matching the prefix; subwindow key (pid:page+Dialog) is also exited
 *           when keepKey is the main page.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchPresenceStateExitAllExcept001, TestSize.Level1)
{
    SearchPresenceStateMachine sm;
    SearchMatchEvidence mainHit;
    mainHit.ruleSetId = "s1";
    mainHit.ruleId = "r1";
    mainHit.isSubwindow = false;
    SearchMatchEvidence subHit;
    subHit.ruleSetId = "s1";
    subHit.ruleId = "r1";
    subHit.isSubwindow = true;

    // Establish PRESENT for both main and subwindow
    sm.Update("1001:HomePage", "HomePage", true, { mainHit });
    sm.Update("1001:HomePage+Dialog", "HomePage+Dialog", true, { subHit });

    // Exit all except HomePage -> subwindow should be exited
    auto events = sm.ExitAllExcept("1001:", "1001:HomePage");
    ASSERT_EQ(events.size(), 1U);
    EXPECT_FALSE(events[0].isTrue);
    EXPECT_EQ(events[0].pageName, "HomePage+Dialog");
    EXPECT_TRUE(events[0].anySubwindow);

    // Main page still PRESENT
    EXPECT_EQ(sm.GetState("1001:HomePage"), SearchPresenceStateMachine::PageState::PRESENT);
    // Subwindow now ABSENT
    EXPECT_EQ(sm.GetState("1001:HomePage+Dialog"), SearchPresenceStateMachine::PageState::ABSENT);
}

/**
 * @tc.name: PageSceneRuleManager_SearchPresenceStateSubwindowExit001
 * @tc.desc: When both main and subwindow are PRESENT, ExitOtherSearchPresence with
 *           a different page exits both (navigation away from the page entirely).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchPresenceStateSubwindowExit001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    SearchSceneSnapshot snapshot;
    snapshot.navDestinationName = "HomePage";
    SearchMatchEvidence mainHit;
    mainHit.ruleSetId = "s1";
    mainHit.ruleId = "r1";
    mainHit.isSubwindow = false;
    SearchMatchEvidence subHit;
    subHit.ruleSetId = "s1";
    subHit.ruleId = "r1";
    subHit.isSubwindow = true;
    manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { mainHit, subHit });

    // Navigate to OtherPage: both HomePage and HomePage+Dialog should be exited.
    // ExitAllExcept iterates an unordered_map, so the order is not guaranteed.
    auto exitEvents = manager.ExitOtherSearchPresence(TEST_PROCESS_ID, "OtherPage");
    ASSERT_EQ(exitEvents.size(), 2U);
    EXPECT_FALSE(exitEvents[0].isTrue);
    EXPECT_FALSE(exitEvents[1].isTrue);
    // Identify main vs subwindow exit by pageName (order-independent)
    bool foundMainExit = false;
    bool foundSubExit = false;
    for (const auto& evt : exitEvents) {
        if (evt.pageName == "HomePage") {
            foundMainExit = true;
            EXPECT_FALSE(evt.anySubwindow);
        } else if (evt.pageName == "HomePage+Dialog") {
            foundSubExit = true;
            EXPECT_TRUE(evt.anySubwindow);
        }
    }
    EXPECT_TRUE(foundMainExit);
    EXPECT_TRUE(foundSubExit);
}

/**
 * @tc.name: PageSceneRuleManager_SearchSubwindowComponentFilter001
 * @tc.desc: Component hits also respect subwindow node filtering: a +Dialog rule
 *           only matches subwindow component nodes.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchSubwindowComponentFilter001, TestSize.Level1)
{
    SearchSnapshotMatcher matcher;
    SearchRuleSet ruleSet;
    ruleSet.ruleSetId = "comp-sub-set";
    SearchSceneRule rule;
    rule.ruleId = "comp-sub-rule";
    rule.pageType = "Navigation";
    rule.pageName = "HomePage+Dialog";
    rule.regions.emplace_back(PageSceneRectInfo { 0.0f, 0.0f, 360.0f, 1280.0f });
    SearchComponentFeature imgFeature;
    imgFeature.type = "Image";
    imgFeature.maxWidth = 48.0f;
    imgFeature.maxHeight = 48.0f;
    rule.components.emplace_back(imgFeature);
    ruleSet.rules.emplace_back(rule);

    SearchSceneSnapshot snapshot;
    snapshot.navDestinationName = "HomePage";
    // Main-window Image node
    SearchSceneNodeFeature mainImg;
    mainImg.nodeId = 10;
    mainImg.nodeType = "Image";
    mainImg.visibleRect = RectF(0.0f, 0.0f, 30.0f, 30.0f);
    mainImg.isSubwindow = false;
    snapshot.nodes.emplace_back(mainImg);
    // Subwindow Image node
    SearchSceneNodeFeature subImg;
    subImg.nodeId = 20;
    subImg.nodeType = "Image";
    subImg.visibleRect = RectF(0.0f, 0.0f, 30.0f, 30.0f);
    subImg.isSubwindow = true;
    snapshot.nodes.emplace_back(subImg);

    auto hits = matcher.Match(snapshot, ruleSet);
    ASSERT_EQ(hits.size(), 1U);
    EXPECT_EQ(hits[0].nodeId, 20);
    EXPECT_TRUE(hits[0].isSubwindow);
    EXPECT_EQ(hits[0].evidenceType, "component");
}

/**
 * @tc.name: PageSceneRuleManager_SearchPageNameByPageType001
 * @tc.desc: UpdateSearchPresence derives pageName by pageType: Navigation uses
 *           navDestinationName, Router uses pageUrl, fallback uses pageName.
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_SearchPageNameByPageType001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    SearchMatchEvidence hit;
    hit.ruleSetId = "s1";
    hit.ruleId = "r1";
    hit.isSubwindow = false;

    // Navigation: uses navDestinationName
    SearchSceneSnapshot navSnapshot;
    navSnapshot.navDestinationName = "NavPage";
    navSnapshot.pageUrl = "pages/nav";
    navSnapshot.pageName = "route,NavPage";
    auto navEvents = manager.UpdateSearchPresence(TEST_PROCESS_ID, navSnapshot, { hit });
    ASSERT_EQ(navEvents.size(), 1U);
    EXPECT_EQ(navEvents[0].pageName, "NavPage");

    // Clean up and test Router: uses pageUrl
    manager.ClearProcess(TEST_PROCESS_ID);
    SearchSceneSnapshot routerSnapshot;
    routerSnapshot.navDestinationName = "";
    routerSnapshot.pageUrl = "pages/router";
    routerSnapshot.pageName = "pages/router";
    auto routerEvents = manager.UpdateSearchPresence(TEST_PROCESS_ID, routerSnapshot, { hit });
    ASSERT_EQ(routerEvents.size(), 1U);
    EXPECT_EQ(routerEvents[0].pageName, "pages/router");
}

// Regression: Dialog and Sheet disappear independently and do not leave stale
// PRESENT states or get forced out by each same-page content scan.
HWTEST_F(PageSceneRuleManagerTestNg, SearchReview_SubwindowLifecycle, TestSize.Level1)
{
    PageSceneRuleManager manager;
    auto snapshot = BuildSubwindowSnapshot();
    auto mainHit = SearchMatchEvidence {};
    mainHit.ruleSetId = "set";
    mainHit.ruleId = "main";
    auto dialogHit = mainHit;
    dialogHit.ruleId = "dialog";
    dialogHit.isSubwindow = true;
    dialogHit.subwindowSuffix = SUBWINDOW_DIALOG_SUFFIX;
    auto sheetHit = dialogHit;
    sheetHit.ruleId = "sheet";
    sheetHit.subwindowSuffix = SUBWINDOW_SHEET_SUFFIX;
    ASSERT_EQ(manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { mainHit, dialogHit, sheetHit }).size(), 3U);
    EXPECT_TRUE(manager.ExitOtherSearchPresence(TEST_PROCESS_ID, "HomePage").empty());
    EXPECT_TRUE(manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { mainHit, dialogHit, sheetHit }).empty());
    auto events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { mainHit, sheetHit });
    ASSERT_EQ(events.size(), 1U);
    EXPECT_FALSE(events[0].isTrue);
    EXPECT_EQ(events[0].pageName, "HomePage+Dialog");
    EXPECT_EQ(events[0].subwindowSuffix, SUBWINDOW_DIALOG_SUFFIX);
    events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { mainHit });
    ASSERT_EQ(events.size(), 1U);
    EXPECT_FALSE(events[0].isTrue);
    EXPECT_EQ(events[0].pageName, "HomePage+Sheet");
    EXPECT_TRUE(manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { mainHit }).empty());
    events = manager.UpdateSearchPresence(TEST_PROCESS_ID, snapshot, { mainHit, dialogHit });
    ASSERT_EQ(events.size(), 1U);
    EXPECT_TRUE(events[0].isTrue);
    EXPECT_EQ(events[0].pageName, "HomePage+Dialog");
    ASSERT_EQ(manager.ExitOtherSearchPresence(TEST_PROCESS_ID, "OtherPage").size(), 2U);
    EXPECT_TRUE(manager.ExitOtherSearchPresence(TEST_PROCESS_ID, "OtherPage").empty());
}

HWTEST_F(PageSceneRuleManagerTestNg, SearchReview_SuffixBoundary, TestSize.Level1)
{
    EXPECT_EQ(BuildSearchSubwindowPageName("HomePage", SUBWINDOW_DIALOG_SUFFIX), "HomePage+Dialog");
    EXPECT_EQ(BuildSearchSubwindowPageName("HomePage+Dialog", SUBWINDOW_DIALOG_SUFFIX), "HomePage+Dialog");
    EXPECT_EQ(BuildSearchSubwindowPageName("HomePage+Sheet", SUBWINDOW_SHEET_SUFFIX), "HomePage+Sheet");
    EXPECT_EQ(BuildSearchSubwindowPageName("Home+DialogDetails", SUBWINDOW_DIALOG_SUFFIX),
        "Home+DialogDetails+Dialog");
    EXPECT_TRUE(BuildSearchSubwindowPageName("", SUBWINDOW_DIALOG_SUFFIX).empty());
    SearchSnapshotMatcher matcher;
    auto snapshot = BuildSubwindowSnapshot();
    // A suffix inside an actual main-page identity must not classify its nodes
    // as subwindow nodes. Cover both an embedded and a trailing suffix.
    for (const auto& name : { "Home+DialogDetails", "Home+Dialog", "Home+Sheet" }) {
        snapshot.navDestinationName = name;
        auto hits = matcher.Match(snapshot, BuildSubwindowRuleSet(name));
        ASSERT_EQ(hits.size(), 1U);
        EXPECT_FALSE(hits[0].isSubwindow);
        EXPECT_TRUE(hits[0].subwindowSuffix.empty());
        EXPECT_EQ(hits[0].nodeId, 100);
    }
}

HWTEST_F(PageSceneRuleManagerTestNg, SearchReview_CustomNamesUnique, TestSize.Level1)
{
    auto root = CreatePageRoot();
    auto outer = CustomNode::CreateCustomNode(2100, "Outer");
    auto inner = CustomNode::CreateCustomNode(2101, "Inner");
    auto distinct = CustomNode::CreateCustomNode(2102, "Distinct");
    outer->SetJSViewName("SearchComponent");
    inner->SetJSViewName("SearchComponent");
    distinct->SetJSViewName("InnerSearchComponent");
    outer->MountToParent(root);
    inner->MountToParent(outer);
    distinct->MountToParent(inner);
    auto text = CreateTestNode(V2::TEXT_ETS_TAG, TEST_TEXT_INPUT_ID, RectF(10, 20, 100, 40));
    text->MountToParent(distinct);
    auto snapshot = SearchSceneSnapshotBuilder().BuildLocal(text, 1);
    ASSERT_EQ(snapshot.nodes.size(), 1U);
    std::vector<std::string> names;
    for (const auto& source : snapshot.nodes[0].keywordSources) {
        if (source.name == "customName") {
            names.emplace_back(source.value);
        }
    }
    ASSERT_EQ(names.size(), 2U);
    EXPECT_EQ(names[0], "InnerSearchComponent");
    EXPECT_EQ(names[1], "SearchComponent");
}

HWTEST_F(PageSceneRuleManagerTestNg, SearchReview_SubtreeVisibilityAndDepth, TestSize.Level1)
{
    auto root = CreatePageRoot();
    auto clip = CreateTestNode(V2::COLUMN_ETS_TAG, TEST_DIALOG_ID, RectF(0, 0, 100, 100));
    clip->GetRenderContext()->UpdateClipEdge(true);
    auto child = CreateTestNode(V2::TEXT_ETS_TAG, TEST_DIALOG_INPUT_ID, RectF(10, 10, 30, 20));
    child->GetAccessibilityProperty<AccessibilityProperty>()->SetText("visible");
    auto clipped = CreateTestNode(V2::TEXT_ETS_TAG, TEST_OVERLAY_INPUT_ID, RectF(200, 200, 30, 20));
    clipped->GetAccessibilityProperty<AccessibilityProperty>()->SetText("clipped-secret");
    auto deep = CreateTestNode(V2::TEXT_ETS_TAG, TEST_TEXT_INPUT_ID, RectF(15, 15, 10, 10));
    deep->GetAccessibilityProperty<AccessibilityProperty>()->SetText("deep-secret");
    auto hidden = CreateTestNode(V2::TEXT_ETS_TAG, TEST_TEXT_AREA_ID, RectF(10, 10, 30, 20));
    hidden->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    hidden->GetAccessibilityProperty<AccessibilityProperty>()->SetText("hidden-secret");
    AddChild(root, clip);
    AddChild(clip, child);
    AddChild(clip, clipped);
    AddChild(clip, hidden);
    AddChild(child, deep);
    auto local = SearchSceneSnapshotBuilder().BuildLocal(clip, 1);
    ASSERT_EQ(local.nodes.size(), 2U);
    bool sawVisible = false;
    for (const auto& feature : local.nodes) {
        for (const auto& source : feature.keywordSources) {
            EXPECT_EQ(source.value.find("clipped-secret"), std::string::npos);
            EXPECT_EQ(source.value.find("hidden-secret"), std::string::npos);
            EXPECT_EQ(source.value.find("deep-secret"), std::string::npos);
            if (source.name == "subtreeText" && source.value == "visible") {
                sawVisible = true;
            }
        }
    }
    EXPECT_TRUE(sawVisible);
    auto full = SearchSceneSnapshotBuilder().Build("HomePage", "HomePage", "", { { root } });
    bool sawDeep = false;
    for (const auto& feature : full.nodes) {
        for (const auto& source : feature.keywordSources) {
            EXPECT_EQ(source.value.find("clipped-secret"), std::string::npos);
            EXPECT_EQ(source.value.find("hidden-secret"), std::string::npos);
            sawDeep = sawDeep || source.value.find("deep-secret") != std::string::npos;
        }
    }
    EXPECT_TRUE(sawDeep);
}

HWTEST_F(PageSceneRuleManagerTestNg, SearchReview_ClickFlushOnceAndRoute, TestSize.Level1)
{
    PageSceneRuleManager manager;
    ASSERT_EQ(manager.RegisterSearchRuleSet(TEST_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    ASSERT_EQ(manager.RegisterSearchRuleSet(SECOND_PROCESS_ID, BuildValidSearchRuleJson()), PAGE_SCENE_NO_ERROR);
    bool stable = false;
    manager.SetIsContentStableFunction([&stable] { return stable; });
    auto first = BuildPendingSearchClick(TEST_TEXT_INPUT_ID, SearchClickNowMs());
    manager.AddPendingClickCandidate(first);
    auto second = first;
    second.operationId += "-second";
    manager.AddPendingClickCandidate(second);
    SearchClickPageContext page { "HomePage", "pages/Home" };
    EXPECT_TRUE(manager.FlushPendingSearchClicksIfStable(page).empty());
    EXPECT_EQ(manager.pendingClickCandidates_.size(), 2U);
    stable = true;
    auto results = manager.FlushPendingSearchClicksIfStable(page);
    ASSERT_EQ(results.size(), 4U);
    std::set<std::pair<int32_t, std::string>> routes;
    for (const auto& result : results) {
        routes.emplace(result.processId, result.operationId);
    }
    EXPECT_EQ(routes.size(), 4U);
    EXPECT_TRUE(manager.FlushPendingSearchClicksIfStable(page).empty());
    EXPECT_TRUE(manager.ConsumeSearchClickCandidates(page).empty());
    manager.AddPendingClickCandidate(first);
    EXPECT_TRUE(manager.FlushPendingSearchClicksIfStable(page).empty());
}


HWTEST_F(PageSceneRuleManagerTestNg, SearchReview_SubtreeUsesOneVisibilityPass, TestSize.Level1)
{
    class CountingRenderContext : public MockRenderContext {
    public:
        RectF GetPaintRectWithTransform() override
        {
            ++reads;
            return MockRenderContext::GetPaintRectWithTransform();
        }
        int reads = 0;
    };
    auto root = CreatePageRoot();
    std::vector<RefPtr<CountingRenderContext>> contexts;
    constexpr int childCount = 64;
    for (int index = 0; index < childCount; ++index) {
        auto child = CreateTestNode(V2::TEXT_ETS_TAG, 2200 + index, RectF(10, 10, 30, 20));
        auto render = AceType::MakeRefPtr<CountingRenderContext>();
        render->SetPaintRectWithTransform(RectF(10, 10, 30, 20));
        child->renderContext_ = render;
        child->GetAccessibilityProperty<AccessibilityProperty>()->SetText("search");
        AddChild(root, child);
        contexts.emplace_back(render);
    }
    auto snapshot = SearchSceneSnapshotBuilder().Build("HomePage", "HomePage", "", { { root } });
    ASSERT_EQ(snapshot.nodes.size(), static_cast<size_t>(childCount + 1));
    for (const auto& context : contexts) {
        EXPECT_EQ(context->reads, 1);
    }
}

} // namespace OHOS::Ace::NG
