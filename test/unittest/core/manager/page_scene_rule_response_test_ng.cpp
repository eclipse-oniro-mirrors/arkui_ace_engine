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
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t PAGE_SCENE_NO_ERROR = 0;
constexpr int32_t PAGE_SCENE_PARAM_INVALID = 4;
constexpr int32_t TEST_PROCESS_ID = 1001;
constexpr int32_t TEST_TEXT_AREA_ID = 2002;
constexpr int32_t TEST_SEARCH_ID = 2003;
constexpr int32_t TEST_RICH_EDITOR_ID = 2005;
constexpr int32_t RESPONSE_ROOT_ID = 3000;
constexpr int32_t RESPONSE_CLIP_PARENT_ID = 3001;
constexpr int32_t RESPONSE_NODE_ID = 3002;
constexpr int32_t RESPONSE_CHILD_ID = 3003;
constexpr int32_t CUSTOM_NODE_ID = 3004;
constexpr int32_t RESPONSE_X_ZERO_NODE_ID = 3008;
constexpr float RESPONSE_VIEW_SIZE = 200.0f;
constexpr float RESPONSE_NODE_X = 20.0f;
constexpr float RESPONSE_NODE_Y = 20.0f;
constexpr float RESPONSE_NODE_SIZE = 100.0f;
constexpr float RESPONSE_CLIP_WIDTH = 60.0f;
constexpr int32_t RESPONSE_REGION_BASE_X = 10;
constexpr int32_t RESPONSE_REGION_STEP_X = 20;
constexpr int32_t RESPONSE_REGION_BASE_Y = 10;
constexpr int32_t RESPONSE_REGION_WIDTH = 5;
constexpr int32_t RESPONSE_REGION_HEIGHT = 5;
const std::string RESPONSE_RULE_SET_ID = "response_scene_rules";
const std::string RESPONSE_RULE_ID = "response_rule_001";
const std::string RESPONSE_INSPECTOR_ID = "respButton";
const std::string RESPONSE_EVENT = "RESPONSE";
const std::string CUSTOM_CONTROL_NAME = "MyControl";
const std::string SECOND_CUSTOM_CONTROL_NAME = "InnerControl";
const std::string TEST_PAGE_NAME = "pages/Index";

std::string BuildResponseRegionJson(int32_t x, int32_t y, int32_t width, int32_t height)
{
    return std::string(R"({"x":)") + std::to_string(x) + R"(,"y":)" + std::to_string(y) +
           R"(,"width":)" + std::to_string(width) + R"(,"height":)" + std::to_string(height) + R"(})";
}

// Builds one RESPONSE scene rule object. pageName empty means no page condition;
// an empty customControls/inspectorIds/regions vector omits the field entirely.
std::string BuildArrayField(const std::string& key, const std::vector<std::string>& values, bool quoteValues)
{
    std::string value = R"(")" + key + R"(":[)";
    for (size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            value += ",";
        }
        if (quoteValues) {
            value += R"(")" + values[index] + R"(")";
        } else {
            value += values[index];
        }
    }
    value += "]";
    return value;
}

// Aggregated fields for BuildResponseRuleJson so the helper stays within the
// parameter-count limit. Field order: pageType, pageName, customControls,
// inspectorIds, regions, includeText, includeAccessibility.
struct ResponseRuleSpec {
    std::string pageType;
    std::string pageName;
    std::vector<std::string> customControls;
    std::vector<std::string> inspectorIds;
    std::vector<std::string> regions;
    bool includeText = false;
    bool includeAccessibility = false;
};

std::string BuildResponseRuleJson(const std::string& ruleId, const ResponseRuleSpec& spec)
{
    std::vector<std::string> fields;
    fields.emplace_back(R"("ruleId":")" + ruleId + R"(")");
    fields.emplace_back(R"("sceneType":"RESPONSE")");
    fields.emplace_back(R"("enabled":true)");
    fields.emplace_back(R"("report":{"includeText":)" + std::string(spec.includeText ? "true" : "false") +
                        R"(,"includeAccessibility":)" + std::string(spec.includeAccessibility ? "true" : "false") +
                        R"(})");
    if (!spec.pageName.empty()) {
        fields.emplace_back(R"("page":{"pageType":")" + spec.pageType + R"(","pageName":")" + spec.pageName +
                            R"("})");
    }
    if (!spec.customControls.empty()) {
        fields.emplace_back(BuildArrayField("customControls", spec.customControls, true));
    }
    if (!spec.inspectorIds.empty()) {
        fields.emplace_back(BuildArrayField("inspectorIds", spec.inspectorIds, true));
    }
    if (!spec.regions.empty()) {
        fields.emplace_back(BuildArrayField("regions", spec.regions, false));
    }
    std::string json = "{";
    for (size_t index = 0; index < fields.size(); ++index) {
        if (index > 0) {
            json += ",";
        }
        json += fields[index];
    }
    json += "}";
    return json;
}

std::string BuildResponseRuleSetJson(const std::string& ruleSetId, const std::vector<std::string>& ruleObjects)
{
    std::string json = R"({"version":1,"ruleSetId":")" + ruleSetId + R"(","rules":[)";
    for (size_t index = 0; index < ruleObjects.size(); ++index) {
        if (index > 0) {
            json += ",";
        }
        json += ruleObjects[index];
    }
    json += "]}";
    return json;
}

// Distinct non-overlapping key regions for capacity tests.
std::vector<std::string> BuildResponseRegions(int32_t count)
{
    std::vector<std::string> regions;
    for (int32_t index = 0; index < count; ++index) {
        regions.emplace_back(BuildResponseRegionJson(
            RESPONSE_REGION_BASE_X + index * RESPONSE_REGION_STEP_X, RESPONSE_REGION_BASE_Y,
            RESPONSE_REGION_WIDTH, RESPONSE_REGION_HEIGHT));
    }
    return regions;
}

std::vector<std::string> BuildResponseRuleObjects(int32_t count, int32_t regionCount = 1)
{
    std::vector<std::string> objects;
    for (int32_t index = 0; index < count; ++index) {
        objects.emplace_back(BuildResponseRuleJson("rule_" + std::to_string(index), { "Router", TEST_PAGE_NAME, {},
            { "id_" + std::to_string(index) }, BuildResponseRegions(regionCount) }));
    }
    return objects;
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

DumpStartNodeSet BuildStartNodes(const RefPtr<FrameNode>& root)
{
    DumpStartNodeSet set;
    set.dumpBeginNode = root;
    if (root) {
        set.pageStartNodes.push_back(root);
    }
    return set;
}

void AddChild(const RefPtr<FrameNode>& parent, const RefPtr<FrameNode>& child)
{
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    child->MountToParent(parent);
}

RefPtr<FrameNode> CreateResponseRoot()
{
    return CreateTestNode(
        V2::COLUMN_ETS_TAG, RESPONSE_ROOT_ID, RectF(0.0f, 0.0f, RESPONSE_VIEW_SIZE, RESPONSE_VIEW_SIZE));
}

RefPtr<FrameNode> CreateResponseNode(int32_t nodeId, float x, float y, const std::string& inspectorId)
{
    auto node = CreateTestNode(
        V2::BUTTON_ETS_TAG, nodeId, RectF(x, y, RESPONSE_NODE_SIZE, RESPONSE_NODE_SIZE));
    if (!inspectorId.empty()) {
        node->UpdateInspectorId(inspectorId);
    }
    return node;
}

// root [0,0,200,200] -> clipParent [0,0,60,200] (clip enabled) -> responseNode [20,20,100,100].
// The effective visible rect is [20,20,40,100], matching the clipping example in AC-5.4.
struct ClippedResponseScene {
    RefPtr<FrameNode> root;
    RefPtr<FrameNode> clipParent;
    RefPtr<FrameNode> responseNode;
    DumpStartNodeSet startNodes;
};

ClippedResponseScene CreateClippedResponseScene(const std::string& inspectorId)
{
    ClippedResponseScene scene;
    scene.root = CreateTestNode(
        V2::COLUMN_ETS_TAG, RESPONSE_ROOT_ID, RectF(0.0f, 0.0f, RESPONSE_VIEW_SIZE, RESPONSE_VIEW_SIZE));
    scene.clipParent = CreateTestNode(V2::COLUMN_ETS_TAG, RESPONSE_CLIP_PARENT_ID,
        RectF(0.0f, 0.0f, RESPONSE_CLIP_WIDTH, RESPONSE_VIEW_SIZE));
    scene.clipParent->GetRenderContext()->UpdateClipEdge(true);
    scene.responseNode = CreateResponseNode(RESPONSE_NODE_ID, RESPONSE_NODE_X, RESPONSE_NODE_Y, inspectorId);
    AddChild(scene.clipParent, scene.responseNode);
    AddChild(scene.root, scene.clipParent);
    scene.startNodes = BuildStartNodes(scene.root);
    return scene;
}

RefPtr<FrameNode> CreateResponseTextNode(int32_t nodeId, float x, float y, const std::string& text,
    const std::string& accessibilityText)
{
    auto node = CreateTestNode(V2::TEXT_ETS_TAG, nodeId, RectF(x, y, 30.0f, 20.0f));
    auto accessibilityProperty = node->GetAccessibilityProperty<AccessibilityProperty>();
    if (!text.empty()) {
        accessibilityProperty->SetText(text);
    }
    if (!accessibilityText.empty()) {
        accessibilityProperty->SetAccessibilityText(accessibilityText);
    }
    return node;
}
} // namespace

class PageSceneRuleManagerTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

/**
 * @tc.name: PageSceneRuleManager_RegisterResponseRuleSet001
 * @tc.desc: Test RESPONSE ruleSet registration validation rejects: duplicate ruleSetId,
 *           empty/non-string IDs, ruleId uniqueness, region overlap and capacity limits
 *           (AC-1).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_RegisterResponseRuleSet001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);
    auto singleRule = [&region](const std::string& ruleSetId, const std::string& ruleId,
                          const std::string& pageName, const std::vector<std::string>& inspectorIds,
                          const std::vector<std::string>& regions) {
        return BuildResponseRuleSetJson(ruleSetId, { BuildResponseRuleJson(ruleId,
            { "Router", pageName, {}, inspectorIds, regions }) });
    };

    // AC-1.1: a valid RESPONSE ruleSet with one rule and one key region.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_1", "rule_1", TEST_PAGE_NAME, {"payButton"}, {region})),
        PAGE_SCENE_NO_ERROR);

    // AC-1.4: reusing the same ruleSetId fails the whole registration.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_1", "rule_1", TEST_PAGE_NAME, {"payButton"}, {region})),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.2: an empty string element inside inspectorIds.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_2", "rule_1", TEST_PAGE_NAME, {""}, {region})),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.9: a numeric element inside inspectorIds.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        std::string(R"({"version":1,"ruleSetId":"response_ruleset_3","rules":[{)") +
        R"("ruleId":"rule_1","sceneType":"RESPONSE","enabled":true,)" +
        R"("page":{"pageType":"Router","pageName":")" + TEST_PAGE_NAME + R"("},)" +
        R"("inspectorIds":["abc",123],)" + R"("regions":[)" + region + R"(]}]})"),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.11: empty ruleSetId.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("", "rule_1", TEST_PAGE_NAME, {"payButton"}, {region})),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.12: empty ruleId.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_4", "", TEST_PAGE_NAME, {"payButton"}, {region})),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.5: duplicate ruleId within one ruleSet.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID, BuildResponseRuleSetJson("response_ruleset_5", {
        BuildResponseRuleJson("rule_1", { "Router", TEST_PAGE_NAME, {}, { "payButton" }, { region } }),
        BuildResponseRuleJson("rule_1", { "Router", TEST_PAGE_NAME, {}, { "otherButton" }, { region } }) })),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.6: two regions with a positive-area intersection in one rule.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID, singleRule("response_ruleset_6", "rule_1",
        TEST_PAGE_NAME,
        {"payButton"}, { BuildResponseRegionJson(0, 0, 10, 10), BuildResponseRegionJson(5, 5, 10, 10) })),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.7: six regions in one rule exceed the per-rule limit.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_7", "rule_1", TEST_PAGE_NAME, {"payButton"}, BuildResponseRegions(6))),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.3: eleven rules exceed the per-ruleSet limit.
    EXPECT_EQ(manager.RegisterResponseRuleSet(
        TEST_PROCESS_ID, BuildResponseRuleSetJson("response_ruleset_8", BuildResponseRuleObjects(11))),
        PAGE_SCENE_PARAM_INVALID);
}

/**
 * @tc.name: PageSceneRuleManager_RegisterResponseRuleSet002
 * @tc.desc: Test cross-ruleSet capacity and isolation: a third ruleSet is rejected, same
 *           ruleId and overlapping regions across ruleSets are allowed, unregister of a
 *           never-registered ruleSet fails and two ruleSets at their limits both succeed
 *           (AC-1.8 / AC-1.10 / AC-10).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_RegisterResponseRuleSet002, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);
    auto singleRule = [&region](const std::string& ruleSetId, const std::string& ruleId,
                          const std::string& pageName, const std::vector<std::string>& inspectorIds,
                          const std::vector<std::string>& regions) {
        return BuildResponseRuleSetJson(ruleSetId, { BuildResponseRuleJson(ruleId,
            { "Router", pageName, {}, inspectorIds, regions }) });
    };

    // AC-10.3: regions of two different rules in the same ruleSet may overlap.
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID, BuildResponseRuleSetJson("response_ruleset_9", {
        BuildResponseRuleJson("rule_1", { "Router", TEST_PAGE_NAME, {}, { "payButton" },
            { BuildResponseRegionJson(0, 0, 10, 10) } }),
        BuildResponseRuleJson("rule_2", { "Router", TEST_PAGE_NAME, {}, { "otherButton" },
            { BuildResponseRegionJson(5, 5, 10, 10) } }) })),
        PAGE_SCENE_NO_ERROR);

    // AC-1.8: the same ruleId in two different ruleSets is allowed (fills the capacity of two).
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_9b", "rule_1", TEST_PAGE_NAME, {"payButton"}, {region})),
        PAGE_SCENE_NO_ERROR);

    // AC-10.1: a third ruleSet for the same process is rejected (capacity is two).
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_10", "rule_1", TEST_PAGE_NAME, {"payButton"}, {region})),
        PAGE_SCENE_PARAM_INVALID);

    // AC-1.8: the same ruleId in two different ruleSets is allowed.
    manager.ClearProcess(TEST_PROCESS_ID);
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_a", "shared_rule", TEST_PAGE_NAME, {"aButton"}, {region})),
        PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID,
        singleRule("response_ruleset_b", "shared_rule", TEST_PAGE_NAME, {"bButton"}, {region})),
        PAGE_SCENE_NO_ERROR);

    // AC-10.4: unregistering a ruleSetId that was never registered.
    EXPECT_EQ(manager.UnregisterResponseRuleSet(TEST_PROCESS_ID, "never_registered"), PAGE_SCENE_PARAM_INVALID);

    // AC-1.10: two ruleSets each at the 10-rule / 5-region limits both succeed.
    manager.ClearProcess(TEST_PROCESS_ID);
    auto tenRuleObjects = BuildResponseRuleObjects(10, 5);
    EXPECT_EQ(manager.RegisterResponseRuleSet(
        TEST_PROCESS_ID, BuildResponseRuleSetJson("response_ruleset_10a", tenRuleObjects)), PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterResponseRuleSet(
        TEST_PROCESS_ID, BuildResponseRuleSetJson("response_ruleset_10b", tenRuleObjects)), PAGE_SCENE_NO_ERROR);

    // AC-10.5: overlapping regions across two different ruleSets are allowed.
    manager.ClearProcess(TEST_PROCESS_ID);
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID, singleRule(
        "response_ruleset_c", "rule_1", TEST_PAGE_NAME, {"payButton"}, { BuildResponseRegionJson(0, 0, 10, 10) })),
        PAGE_SCENE_NO_ERROR);
    EXPECT_EQ(manager.RegisterResponseRuleSet(TEST_PROCESS_ID, singleRule(
        "response_ruleset_d", "rule_1", TEST_PAGE_NAME, {"payButton"}, { BuildResponseRegionJson(5, 5, 10, 10) })),
        PAGE_SCENE_NO_ERROR);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse001
 * @tc.desc: Test RESPONSE page condition matching: a page match produces a hit and a page
 *           mismatch produces no hit when no custom control is present (AC-2.1 / AC-2.4).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse001, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);

    // AC-2.1: page condition matches, custom control absent.
    auto pageRoot = CreateResponseRoot();
    auto pageButton = CreateResponseNode(RESPONSE_NODE_ID, RESPONSE_NODE_X, RESPONSE_NODE_Y, RESPONSE_INSPECTOR_ID);
    AddChild(pageRoot, pageButton);
    auto pageRule = BuildResponseRuleSetJson(RESPONSE_RULE_SET_ID, {
        BuildResponseRuleJson(RESPONSE_RULE_ID,
            { "Router", TEST_PAGE_NAME, {}, { RESPONSE_INSPECTOR_ID }, { region } }) });
    auto pageResult = manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, pageRule, pageButton, BuildStartNodes(pageRoot), TEST_PAGE_NAME, "");
    ASSERT_TRUE(pageResult.has_value());
    EXPECT_TRUE(pageResult->matched);
    ASSERT_EQ(pageResult->hits.size(), 1U);
    EXPECT_EQ(pageResult->hits[0].ruleId, RESPONSE_RULE_ID);
    EXPECT_EQ(pageResult->hits[0].pageName, TEST_PAGE_NAME);
    EXPECT_EQ(pageResult->hits[0].inspectorId, RESPONSE_INSPECTOR_ID);

    // AC-2.4: neither page nor custom control matches.
    auto noMatchResult = manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, pageRule, pageButton, BuildStartNodes(pageRoot), "pages/Other", "");
    EXPECT_FALSE(noMatchResult.has_value());
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse006
 * @tc.desc: Test RESPONSE custom-control matching: a custom control matches, and two custom
 *           controls in one rule both match in walk-up order (AC-2.2 / AC-2.5).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse006, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);

    // AC-2.2: custom control matches, page condition absent.
    auto customRoot = CreateResponseRoot();
    auto customNode = CustomNode::CreateCustomNode(CUSTOM_NODE_ID, CUSTOM_CONTROL_NAME);
    customNode->SetJSViewName(std::string(CUSTOM_CONTROL_NAME));
    auto customButton = CreateResponseNode(RESPONSE_NODE_ID, RESPONSE_NODE_X, RESPONSE_NODE_Y, RESPONSE_INSPECTOR_ID);
    customButton->MountToParent(customNode);
    customNode->MountToParent(customRoot);
    auto customRule = BuildResponseRuleSetJson(RESPONSE_RULE_SET_ID, {
        BuildResponseRuleJson(RESPONSE_RULE_ID, { "", "", { CUSTOM_CONTROL_NAME }, { RESPONSE_INSPECTOR_ID },
            { region } }) });
    auto customResult = manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, customRule, customButton, BuildStartNodes(customRoot), "pages/Other", "");
    ASSERT_TRUE(customResult.has_value());
    ASSERT_EQ(customResult->hits.size(), 1U);
    EXPECT_EQ(customResult->hits[0].customControls, (std::vector<std::string> { CUSTOM_CONTROL_NAME }));

    // AC-2.5: two custom controls in one rule both match (walk-up order from the node).
    auto multiRoot = CreateResponseRoot();
    auto outerCustom = CustomNode::CreateCustomNode(CUSTOM_NODE_ID, CUSTOM_CONTROL_NAME);
    outerCustom->SetJSViewName(std::string(CUSTOM_CONTROL_NAME));
    auto innerCustom = CustomNode::CreateCustomNode(TEST_RICH_EDITOR_ID, SECOND_CUSTOM_CONTROL_NAME);
    innerCustom->SetJSViewName(std::string(SECOND_CUSTOM_CONTROL_NAME));
    auto multiButton = CreateResponseNode(RESPONSE_NODE_ID, RESPONSE_NODE_X, RESPONSE_NODE_Y, RESPONSE_INSPECTOR_ID);
    multiButton->MountToParent(innerCustom);
    innerCustom->MountToParent(outerCustom);
    outerCustom->MountToParent(multiRoot);
    auto multiCustomRule = BuildResponseRuleSetJson(RESPONSE_RULE_SET_ID, {
        BuildResponseRuleJson(RESPONSE_RULE_ID,
            { "", "", { CUSTOM_CONTROL_NAME, SECOND_CUSTOM_CONTROL_NAME }, { RESPONSE_INSPECTOR_ID },
                { region } }) });
    auto multiResult = manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, multiCustomRule, multiButton, BuildStartNodes(multiRoot), "pages/Other", "");
    ASSERT_TRUE(multiResult.has_value());
    ASSERT_EQ(multiResult->hits.size(), 1U);
    EXPECT_EQ(multiResult->hits[0].customControls,
        (std::vector<std::string> { SECOND_CUSTOM_CONTROL_NAME, CUSTOM_CONTROL_NAME }));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse009
 * @tc.desc: Test page and custom-control conditions both matching in one RESPONSE rule
 *           (AC-2.3).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse009, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);

    // AC-2.3: page and custom control both match in one rule.
    auto bothRoot = CreateResponseRoot();
    auto bothCustom = CustomNode::CreateCustomNode(CUSTOM_NODE_ID, CUSTOM_CONTROL_NAME);
    bothCustom->SetJSViewName(std::string(CUSTOM_CONTROL_NAME));
    auto bothButton = CreateResponseNode(RESPONSE_NODE_ID, RESPONSE_NODE_X, RESPONSE_NODE_Y, RESPONSE_INSPECTOR_ID);
    bothButton->MountToParent(bothCustom);
    bothCustom->MountToParent(bothRoot);
    auto bothRule = BuildResponseRuleSetJson(RESPONSE_RULE_SET_ID, {
        BuildResponseRuleJson(RESPONSE_RULE_ID,
            { "Router", TEST_PAGE_NAME, { CUSTOM_CONTROL_NAME }, { RESPONSE_INSPECTOR_ID }, { region } }) });
    auto bothResult = manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, bothRule, bothButton, BuildStartNodes(bothRoot), TEST_PAGE_NAME, "");
    ASSERT_TRUE(bothResult.has_value());
    ASSERT_EQ(bothResult->hits.size(), 1U);
    EXPECT_EQ(bothResult->hits[0].pageName, TEST_PAGE_NAME);
    EXPECT_EQ(bothResult->hits[0].customControls, (std::vector<std::string> { CUSTOM_CONTROL_NAME }));
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse002
 * @tc.desc: Test ID-priority matching and region fallback under ancestor clipping: an exact
 *           Inspector-ID hit skips the region intersection, region matching uses the clipped
 *           visible rect [20,20,40,100] (AC-5.1 / AC-5.2 / AC-5.4).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse002, TestSize.Level1)
{
    PageSceneRuleManager manager;
    auto scene = CreateClippedResponseScene(RESPONSE_INSPECTOR_ID);

    auto matchWith = [&manager, &scene](const std::string& ruleSetId, const std::string& ruleId,
                          const std::vector<std::string>& inspectorIds,
                          const std::vector<std::string>& regions) {
        auto ruleJson = BuildResponseRuleSetJson(ruleSetId, {
            BuildResponseRuleJson(ruleId, { "Router", TEST_PAGE_NAME, {}, inspectorIds, regions }) });
        return manager.MatchPageSceneResponse(
            TEST_PROCESS_ID, ruleJson, scene.responseNode, scene.startNodes, TEST_PAGE_NAME, "");
    };

    // AC-5.1: an exact ID hit skips the region intersection and reports the effective rect.
    auto idHit = matchWith("id_hit", "id_rule", { RESPONSE_INSPECTOR_ID },
        { BuildResponseRegionJson(80, 30, 10, 10) });
    ASSERT_TRUE(idHit.has_value());
    ASSERT_EQ(idHit->hits.size(), 1U);
    EXPECT_EQ(idHit->hits[0].inspectorId, RESPONSE_INSPECTOR_ID);
    EXPECT_TRUE(idHit->hits[0].hasValidRect);
    EXPECT_FLOAT_EQ(idHit->hits[0].rect.x, RESPONSE_NODE_X);
    EXPECT_FLOAT_EQ(idHit->hits[0].rect.y, RESPONSE_NODE_Y);
    EXPECT_FLOAT_EQ(idHit->hits[0].rect.width, 40.0f);
    EXPECT_FLOAT_EQ(idHit->hits[0].rect.height, RESPONSE_NODE_SIZE);

    // AC-5.2 / AC-5.4: without inspectorIds, a region intersecting the clipped visible
    // rect [20,20,40,100] matches and the clipped rect is reported.
    auto regionHit = matchWith("region_hit", "region_rule", {}, { BuildResponseRegionJson(30, 30, 10, 10) });
    ASSERT_TRUE(regionHit.has_value());
    ASSERT_EQ(regionHit->hits.size(), 1U);
    EXPECT_FLOAT_EQ(regionHit->hits[0].rect.x, RESPONSE_NODE_X);
    EXPECT_FLOAT_EQ(regionHit->hits[0].rect.y, RESPONSE_NODE_Y);
    EXPECT_FLOAT_EQ(regionHit->hits[0].rect.width, 40.0f);
    EXPECT_FLOAT_EQ(regionHit->hits[0].rect.height, RESPONSE_NODE_SIZE);

    auto regionJson = JsonUtil::ParseJsonString(regionHit->sceneJson);
    ASSERT_TRUE(regionJson);
    auto hitJson = regionJson->GetValue("hits")->GetArrayItem(0);
    ASSERT_TRUE(hitJson);
    auto rectJson = hitJson->GetValue("rect");
    ASSERT_TRUE(rectJson);
    EXPECT_EQ(rectJson->GetDouble("x"), RESPONSE_NODE_X);
    EXPECT_EQ(rectJson->GetDouble("y"), RESPONSE_NODE_Y);
    EXPECT_EQ(rectJson->GetDouble("width"), 40.0);
    EXPECT_EQ(rectJson->GetDouble("height"), RESPONSE_NODE_SIZE);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse003
 * @tc.desc: Test region-only negatives under ancestor clipping: a region that only intersects
 *           the clipped-away part, case/prefix/substring ID mismatches, and IDs on parent or
 *           child nodes do not match (AC-5.3 / AC-5.5 / AC-5.7 / AC-5.8 / AC-5.9 / AC-5.10 / AC-5.11).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse003, TestSize.Level1)
{
    PageSceneRuleManager manager;
    auto scene = CreateClippedResponseScene(RESPONSE_INSPECTOR_ID);

    auto matchWith = [&manager, &scene](const std::string& ruleSetId, const std::string& ruleId,
                          const std::vector<std::string>& inspectorIds,
                          const std::vector<std::string>& regions) {
        auto ruleJson = BuildResponseRuleSetJson(ruleSetId, {
            BuildResponseRuleJson(ruleId, { "Router", TEST_PAGE_NAME, {}, inspectorIds, regions }) });
        return manager.MatchPageSceneResponse(
            TEST_PROCESS_ID, ruleJson, scene.responseNode, scene.startNodes, TEST_PAGE_NAME, "");
    };

    // AC-5.5: [80,30,10,10] only intersects the clipped-away part of the component.
    EXPECT_FALSE(matchWith("no_region", "r", {}, { BuildResponseRegionJson(80, 30, 10, 10) }).has_value());

    // AC-5.3: ID matching is case sensitive.
    EXPECT_FALSE(matchWith("case_id", "r", { "PayButton" }, { BuildResponseRegionJson(80, 30, 10, 10) }).has_value());

    // AC-5.8: a prefix-only ID does not match.
    EXPECT_FALSE(matchWith("prefix_id", "r", { "resp" }, { BuildResponseRegionJson(80, 30, 10, 10) }).has_value());

    // AC-5.9: a substring-only ID does not match.
    EXPECT_FALSE(
        matchWith("substring_id", "r", { "Button" }, { BuildResponseRegionJson(80, 30, 10, 10) }).has_value());

    // AC-5.7: a non-matching ID still falls back to a matching region.
    EXPECT_TRUE(matchWith("region_fallback", "r", { "payButton" }, { BuildResponseRegionJson(30, 30, 10, 10) })
                    .has_value());

    // AC-5.10: the ID lives on a parent node, not the response node.
    auto parentIdScene = CreateClippedResponseScene("otherButton");
    parentIdScene.clipParent->UpdateInspectorId("payButton");
    auto parentIdRule = BuildResponseRuleSetJson("parent_id", {
        BuildResponseRuleJson("r", { "Router", TEST_PAGE_NAME, {}, { "payButton" },
            { BuildResponseRegionJson(80, 30, 10, 10) } }) });
    EXPECT_FALSE(manager.MatchPageSceneResponse(TEST_PROCESS_ID, parentIdRule, parentIdScene.responseNode,
        parentIdScene.startNodes, TEST_PAGE_NAME, "").has_value());

    // AC-5.11: the ID lives on a child node, not the response node.
    auto childIdScene = CreateClippedResponseScene("otherButton");
    AddChild(childIdScene.responseNode, CreateResponseNode(RESPONSE_CHILD_ID, 0.0f, 0.0f, "payButton"));
    auto childIdRule = BuildResponseRuleSetJson("child_id", {
        BuildResponseRuleJson("r", { "Router", TEST_PAGE_NAME, {}, { "payButton" },
            { BuildResponseRegionJson(80, 30, 10, 10) } }) });
    EXPECT_FALSE(manager.MatchPageSceneResponse(TEST_PROCESS_ID, childIdRule, childIdScene.responseNode,
        childIdScene.startNodes, TEST_PAGE_NAME, "").has_value());
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse004
 * @tc.desc: Test text-tree collection flags, empty-field omission, subtree-only
 *           scoping, no event/touch leakage in the payload, and an x=0 rect preservation (AC-7).
 * @tc.type: FUNC
 */
std::optional<PageSceneResponseMatch> MatchResponseWithTrees(PageSceneRuleManager& manager,
    const std::string& region, bool includeText, bool includeAccessibility)
{
    auto root = CreateResponseRoot();
    auto responseNode = CreateResponseNode(RESPONSE_NODE_ID, RESPONSE_NODE_X, RESPONSE_NODE_Y, RESPONSE_INSPECTOR_ID);
    auto responseA11y = responseNode->GetAccessibilityProperty<AccessibilityProperty>();
    responseA11y->SetText("buttonText");
    responseA11y->SetAccessibilityText("buttonDesc");
    auto childA = CreateResponseTextNode(RESPONSE_CHILD_ID, 0.0f, 0.0f, "childText", "childDesc");
    childA->UpdateInspectorId("childA");
    auto childB = CreateResponseTextNode(TEST_TEXT_AREA_ID, 40.0f, 0.0f, "", "");
    childB->UpdateInspectorId("childB");
    AddChild(responseNode, childA);
    AddChild(responseNode, childB);
    AddChild(root, responseNode);
    auto sibling = CreateResponseTextNode(TEST_SEARCH_ID, 150.0f, 150.0f, "siblingText", "");
    sibling->UpdateInspectorId("sibling");
    AddChild(root, sibling);
    auto ruleJson = BuildResponseRuleSetJson(RESPONSE_RULE_SET_ID, {
        BuildResponseRuleJson(RESPONSE_RULE_ID, { "Router", TEST_PAGE_NAME, {}, { RESPONSE_INSPECTOR_ID },
            { region }, includeText, includeAccessibility }) });
    return manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, ruleJson, responseNode, BuildStartNodes(root), TEST_PAGE_NAME, "");
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse004
 * @tc.desc: Test the RESPONSE JSON payload shape without text trees: only hit
 *           metadata is reported and the payload never leaks event fields (AC-7.1 / AC-7.3 /
 *           AC-7.6 / AC-7.12).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse004, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);

    // AC-7.1 / AC-7.3 / AC-7.6: includeText and includeAccessibility default to false.
    auto plainResult = MatchResponseWithTrees(manager, region, false, false);
    ASSERT_TRUE(plainResult.has_value());
    auto plainJson = JsonUtil::ParseJsonString(plainResult->sceneJson);
    ASSERT_TRUE(plainJson);
    auto plainHit = plainJson->GetValue("hits")->GetArrayItem(0);
    ASSERT_TRUE(plainHit);
    EXPECT_EQ(plainHit->GetString("inspectorId"), RESPONSE_INSPECTOR_ID);
    EXPECT_EQ(plainHit->GetString("type"), V2::BUTTON_ETS_TAG);
    EXPECT_TRUE(plainHit->GetValue("rect")->IsObject());
    EXPECT_TRUE(plainHit->GetValue("textTree")->IsNull());
    EXPECT_TRUE(plainHit->GetValue("accessibilityTree")->IsNull());
    // AC-7.12: the payload never leaks event type or touch stage fields.
    EXPECT_TRUE(plainJson->GetValue("eventType")->IsNull());
    EXPECT_TRUE(plainJson->GetValue("touchStage")->IsNull());
    EXPECT_EQ(plainJson->GetString("sceneType"), RESPONSE_EVENT);
    EXPECT_EQ(plainJson->GetString("eventName"), RESPONSE_EVENT);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse007
 * @tc.desc: Test includeText/includeAccessibility tree collection: non-empty texts and
 *           descriptions are reported, empty ones are kept, and only the response subtree
 *           is collected (AC-7.2 / AC-7.5 / AC-7.8 / AC-7.9 / AC-7.11).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse007, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);

    // AC-7.2 / AC-7.8: includeText=true reports non-empty texts and keeps nodes
    // whose text is empty.
    auto textResult = MatchResponseWithTrees(manager, region, true, false);
    ASSERT_TRUE(textResult.has_value());
    auto textJson = JsonUtil::ParseJsonString(textResult->sceneJson);
    ASSERT_TRUE(textJson);
    auto textHit = textJson->GetValue("hits")->GetArrayItem(0);
    ASSERT_TRUE(textHit);
    auto textTree = JsonUtil::ParseJsonString(textHit->GetString("textTree"));
    ASSERT_TRUE(textTree);
    EXPECT_EQ(textTree->GetString("id"), RESPONSE_INSPECTOR_ID);
    EXPECT_EQ(textTree->GetString("type"), V2::BUTTON_ETS_TAG);
    EXPECT_EQ(textTree->GetString("text"), "buttonText");
    auto textChildren = textTree->GetValue("children");
    ASSERT_TRUE(textChildren);
    ASSERT_EQ(textChildren->GetArraySize(), 2);
    auto textChildA = textChildren->GetArrayItem(0);
    ASSERT_TRUE(textChildA);
    EXPECT_EQ(textChildA->GetString("id"), "childA");
    EXPECT_EQ(textChildA->GetString("type"), V2::TEXT_ETS_TAG);
    EXPECT_EQ(textChildA->GetString("text"), "childText");
    auto textChildB = textChildren->GetArrayItem(1);
    ASSERT_TRUE(textChildB);
    EXPECT_EQ(textChildB->GetString("id"), "childB");
    EXPECT_TRUE(textChildB->GetValue("text")->IsNull());
    EXPECT_TRUE(textHit->GetValue("accessibilityTree")->IsNull());
    // AC-7.11: only the response subtree is collected, not parent or sibling content.
    EXPECT_TRUE(textTree->ToString().find("siblingText") == std::string::npos);

    // AC-7.5 / AC-7.9: includeAccessibility=true reports non-empty descriptions and keeps
    // nodes whose description is empty.
    auto accessibilityResult = MatchResponseWithTrees(manager, region, false, true);
    ASSERT_TRUE(accessibilityResult.has_value());
    auto accessibilityJson = JsonUtil::ParseJsonString(accessibilityResult->sceneJson);
    ASSERT_TRUE(accessibilityJson);
    auto accessibilityHit = accessibilityJson->GetValue("hits")->GetArrayItem(0);
    ASSERT_TRUE(accessibilityHit);
    auto accessibilityTree = JsonUtil::ParseJsonString(accessibilityHit->GetString("accessibilityTree"));
    ASSERT_TRUE(accessibilityTree);
    EXPECT_EQ(accessibilityTree->GetString("accessibility"), "buttonDesc");
    auto accessibilityChildren = accessibilityTree->GetValue("children");
    ASSERT_TRUE(accessibilityChildren);
    ASSERT_EQ(accessibilityChildren->GetArraySize(), 2);
    EXPECT_EQ(accessibilityChildren->GetArrayItem(0)->GetString("accessibility"), "childDesc");
    EXPECT_TRUE(accessibilityChildren->GetArrayItem(1)->GetValue("accessibility")->IsNull());
    EXPECT_TRUE(accessibilityHit->GetValue("textTree")->IsNull());
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse008
 * @tc.desc: Test an x=0 key region rect is preserved in the payload instead of being omitted
 *           (AC-7.10).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse008, TestSize.Level1)
{
    PageSceneRuleManager manager;
    const std::string region = BuildResponseRegionJson(20, 20, 40, 100);

    // AC-7.10: an x=0 rect is preserved instead of being omitted.
    auto xZeroRoot = CreateResponseRoot();
    auto xZeroNode = CreateResponseNode(RESPONSE_X_ZERO_NODE_ID, 0.0f, RESPONSE_NODE_Y, RESPONSE_INSPECTOR_ID);
    AddChild(xZeroRoot, xZeroNode);
    auto xZeroRule = BuildResponseRuleSetJson("x_zero_rules", {
        BuildResponseRuleJson("x_zero_rule",
            { "Router", TEST_PAGE_NAME, {}, { RESPONSE_INSPECTOR_ID }, { region } }) });
    auto xZeroResult = manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, xZeroRule, xZeroNode, BuildStartNodes(xZeroRoot), TEST_PAGE_NAME, "");
    ASSERT_TRUE(xZeroResult.has_value());
    EXPECT_FLOAT_EQ(xZeroResult->hits[0].rect.x, 0.0f);
    auto xZeroJson = JsonUtil::ParseJsonString(xZeroResult->sceneJson);
    ASSERT_TRUE(xZeroJson);
    auto xZeroRect = xZeroJson->GetValue("hits")->GetArrayItem(0)->GetValue("rect");
    ASSERT_TRUE(xZeroRect);
    EXPECT_EQ(xZeroRect->GetDouble("x"), 0.0);
}

/**
 * @tc.name: PageSceneRuleManager_MatchPageSceneResponse005
 * @tc.desc: Test two rules in one ruleSet both hitting the same response node are merged into
 *           a single JSON payload with complete hit entries (AC-8.1).
 * @tc.type: FUNC
 */
HWTEST_F(PageSceneRuleManagerTestNg, PageSceneRuleManager_MatchPageSceneResponse005, TestSize.Level1)
{
    PageSceneRuleManager manager;
    auto root = CreateResponseRoot();
    auto responseNode = CreateResponseNode(RESPONSE_NODE_ID, RESPONSE_NODE_X, RESPONSE_NODE_Y, RESPONSE_INSPECTOR_ID);
    AddChild(root, responseNode);

    auto ruleJson = BuildResponseRuleSetJson(RESPONSE_RULE_SET_ID, {
        BuildResponseRuleJson("rule_a", { "Router", TEST_PAGE_NAME, {}, { RESPONSE_INSPECTOR_ID },
            { BuildResponseRegionJson(80, 30, 10, 10) } }),
        BuildResponseRuleJson("rule_b", { "Router", TEST_PAGE_NAME, {}, {},
            { BuildResponseRegionJson(30, 30, 10, 10) } }) });
    auto result = manager.MatchPageSceneResponse(
        TEST_PROCESS_ID, ruleJson, responseNode, BuildStartNodes(root), TEST_PAGE_NAME, "");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->matched);
    ASSERT_EQ(result->hits.size(), 2U);
    EXPECT_EQ(result->hits[0].ruleId, "rule_a");
    EXPECT_EQ(result->hits[1].ruleId, "rule_b");

    auto sceneJson = JsonUtil::ParseJsonString(result->sceneJson);
    ASSERT_TRUE(sceneJson);
    EXPECT_EQ(sceneJson->GetString("ruleSetId"), RESPONSE_RULE_SET_ID);
    EXPECT_EQ(sceneJson->GetString("sceneType"), RESPONSE_EVENT);
    auto hitsJson = sceneJson->GetValue("hits");
    ASSERT_TRUE(hitsJson);
    ASSERT_TRUE(hitsJson->IsArray());
    ASSERT_EQ(hitsJson->GetArraySize(), 2);
    EXPECT_EQ(hitsJson->GetArrayItem(0)->GetString("ruleId"), "rule_a");
    EXPECT_EQ(hitsJson->GetArrayItem(1)->GetString("ruleId"), "rule_b");
    EXPECT_TRUE(manager.ShouldReport(TEST_PROCESS_ID, result.value()));
}
} // namespace OHOS::Ace::NG
