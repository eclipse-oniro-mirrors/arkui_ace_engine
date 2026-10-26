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
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <list>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/components_ng/property/gradient_property.h"
#include "core/components_ng/base/inspector_dump_options.h"
#include "core/components_ng/base/ui_tree_collector.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/flex/flex_layout_pattern.h"
#include "core/components_ng/pattern/linear_indicator/linear_indicator_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/overlay/keyboard_base_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/relative_container/relative_container_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/interfaces/native/node/ui_tree_api.h"
#include "core/pipeline/base/element_register.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t LIMIT = 5 * 1024 * 1024;
constexpr unsigned FILTER_PURE_LAYOUT_MASK = 1U;
constexpr unsigned HAS_RENDER_CONTENT_MASK = 1U << 1;
constexpr unsigned RENDER_FILTER_COMBINATIONS = HAS_RENDER_CONTENT_MASK << 1;
constexpr int32_t MAX_TEST_TREE_NODES = 5000;
constexpr int32_t VISIBILITY_TEST_DEPTH = 1000;
constexpr int32_t OPACITY_UNDERFLOW_DEPTH = 1100;
constexpr double POSITIVE_NODE_OPACITY = 0.5;
constexpr double SMALL_POSITIVE_OPACITY = 1e-20;
constexpr uintptr_t OUTPUT_POINTER_SENTINEL = 1;
constexpr uint32_t OUTPUT_SIZE_SENTINEL = 99;
constexpr int32_t INNER_CALLBACK_ID = 1;
constexpr uint8_t NO_SHORTCUT_MODIFIERS = 0;
constexpr unsigned INTERACTION_INFO_MASK = 1U;
constexpr unsigned ACCESSIBILITY_INFO_MASK = 1U << 1;
constexpr unsigned ADVANCED_VISUAL_MASK = 1U;
constexpr unsigned ADVANCED_FUNCTIONAL_MASK = 1U << 1;
constexpr unsigned ADVANCED_OPTION_COMBINATIONS = ADVANCED_FUNCTIONAL_MASK << 1;
constexpr int32_t PROMOTED_TEXT_ID = 11;
constexpr int32_t PROMOTED_BUTTON_ID = 12;
constexpr int32_t PROMOTED_LEAF_COUNT = 2;
constexpr uint32_t ROOT_NODE_COUNT = 1;
constexpr int32_t CACHED_AND_DISAPPEARING_COUNT = 3;
constexpr int32_t NORMAL_AND_DISAPPEARING_COUNT = 2;
const RectF PROMOTION_VIEWPORT_RECT(0.0f, 0.0f, 100.0f, 100.0f);
const RectF OFFSCREEN_PARENT_RECT(200.0f, 0.0f, 20.0f, 20.0f);
const RectF ONSCREEN_LAYOUT_RECT(-190.0f, 10.0f, 10.0f, 10.0f);
const RectF ONSCREEN_SIBLING_RECT(-180.0f, 20.0f, 5.0f, 5.0f);
const RectF RETAINED_CHILD_RECT(0.0f, 0.0f, 5.0f, 5.0f);
enum class PromotionScenario { EMPTY_LAYOUT, PROMOTED_CHILD, PRECEDING_SIBLING };
struct PromotionNodes {
    RefPtr<FrameNode> root;
    RefPtr<FrameNode> parent;
    RefPtr<FrameNode> layout;
    RefPtr<FrameNode> retained;
};

void SetTestRect(const RefPtr<FrameNode>& node, const RectF& rect)
{
    node->SetActive(true);
    auto render = AceType::DynamicCast<MockRenderContext>(node->GetRenderContext());
    ASSERT_NE(render, nullptr);
    render->SetPaintRectWithTransform(rect);
}
void CheckFlatGroup(const JsonValue& group)
{
    std::set<std::string> keys;
    for (auto item = group.GetChild(); item && item->IsValid(); item = item->GetNext()) {
        auto key = item->GetKey();
        auto lookupKey = key;
        std::transform(
            lookupKey.begin(), lookupKey.end(), lookupKey.begin(), [](unsigned char ch) { return std::tolower(ch); });
        EXPECT_TRUE(keys.insert(lookupKey).second) << key;
        EXPECT_EQ(key.find("Pattern"), std::string::npos) << key;
        EXPECT_EQ(key.find("RenderContext"), std::string::npos) << key;
        EXPECT_EQ(key.find("LayoutInfo"), std::string::npos) << key;
        EXPECT_NE(key, "FrameNode");
        EXPECT_NE(key, "ScrollBar");
        EXPECT_NE(key, "pattern");
        EXPECT_NE(key, "renderContext");
        EXPECT_NE(key, "layoutProperty");
    }
}
class CollectionTestNode : public UINode {
    DECLARE_ACE_TYPE(CollectionTestNode, UINode);

public:
    explicit CollectionTestNode(std::string_view tag = "TreeTest")
        : UINode(tag, ElementRegister::GetInstance()->MakeUniqueId())
    {}
    bool IsAtomicNode() const override
    {
        return false;
    }
    void DumpSimplifyInfoWithParamConfig(std::shared_ptr<JsonValue>& json, ParamConfig config) override {}
    std::list<RefPtr<UINode>> GetChildrenForInspector(bool needCacheNode = false) const override
    {
        return cachedChildren.empty() ? UINode::GetChildrenForInspector(needCacheNode) : cachedChildren;
    }
    std::list<RefPtr<UINode>> cachedChildren;
};
class PayloadCollectionNode : public CollectionTestNode {
    DECLARE_ACE_TYPE(PayloadCollectionNode, CollectionTestNode);

public:
    void DumpSimplifyInfoWithParamConfig(std::shared_ptr<JsonValue>& json, ParamConfig config) override
    {
        ++dumpCalls;
        json->Put("payload", payload.c_str());
    }
    std::string payload;
    uint32_t dumpCalls = 0;
};
class LegacyInspectorPattern : public Pattern {
    DECLARE_ACE_TYPE(LegacyInspectorPattern, Pattern);

public:
    void DumpSimplifyInfo(std::shared_ptr<JsonValue>& json) override
    {
        json->Put("legacyContent", "retained");
    }
};
class VisibilityCountingNode : public CollectionTestNode {
    DECLARE_ACE_TYPE(VisibilityCountingNode, CollectionTestNode);

public:
    bool IsVisibleAndActive() const override
    {
        ++visibilityReads;
        return visible;
    }
    mutable uint32_t visibilityReads = 0;
    bool visible = true;
};
ParamConfig MakeConfig(const ArkUITreeOptions& options, TreeCollectionState& state)
{
    ParamConfig config;
    config.filterPureLayout = options.filterPureLayout;
    config.visualProperties = options.visual;
    config.functionalProperties = options.functional;
    config.collectionState = &state;
    return config;
}
std::shared_ptr<JsonValue> Node(const char* type = "Text")
{
    auto value = JsonUtil::CreateSharedPtrJson();
    value->Put("$type", type);
    return value;
}
int32_t Serialize(const std::shared_ptr<JsonValue>& value, bool pretty, std::string& output)
{
    char* data = reinterpret_cast<char*>(OUTPUT_POINTER_SENTINEL);
    uint32_t size = OUTPUT_SIZE_SENTINEL;
    auto code = SerializeUITree(value, pretty, &data, &size);
    if (code == ARKUI_ERROR_CODE_NO_ERROR) {
        EXPECT_NE(data, nullptr);
        EXPECT_EQ(data[size], '\0');
        EXPECT_EQ(std::strlen(data), size);
        output.assign(data, size);
        std::free(data);
    } else {
        EXPECT_EQ(data, nullptr);
        EXPECT_EQ(size, 0u);
        output.clear();
    }
    return code;
}
} // namespace

class UITreeCollectorTest : public Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        MockPipelineContext::GetCurrent()->SetThemeManager(AceType::MakeRefPtr<NiceMock<MockThemeManager>>());
        MockContainer::SetUp(MockPipelineContext::GetCurrent());
    }
    void TearDown() override
    {
        MockContainer::TearDown();
        MockPipelineContext::TearDown();
    }
    RefPtr<FrameNode> Frame(const char* tag, const RefPtr<Pattern>& pattern = AceType::MakeRefPtr<Pattern>())
    {
        return FrameNode::CreateFrameNode(tag, ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    }
    std::vector<RefPtr<FrameNode>> LayoutFrames()
    {
        return {
            Frame(V2::ROW_ETS_TAG, AceType::MakeRefPtr<LinearLayoutPattern>(false)),
            Frame(V2::COLUMN_ETS_TAG, AceType::MakeRefPtr<LinearLayoutPattern>(true)),
            Frame(V2::STACK_ETS_TAG, AceType::MakeRefPtr<StackPattern>()),
            Frame(V2::FLEX_ETS_TAG, AceType::MakeRefPtr<FlexLayoutPattern>()),
            Frame(V2::RELATIVE_CONTAINER_ETS_TAG, AceType::MakeRefPtr<RelativeContainerPattern>()),
        };
    }
#ifndef CROSS_PLATFORM
    PromotionNodes CreatePromotionNodes(PromotionScenario scenario)
    {
        PromotionNodes nodes { Frame("Root"), Frame("Parent"), Frame("Row"), Frame("Text") };
        nodes.root->AddChild(nodes.parent);
        if (scenario == PromotionScenario::PRECEDING_SIBLING) {
            nodes.parent->AddChild(nodes.retained);
        }
        nodes.parent->AddChild(nodes.layout);
        if (scenario == PromotionScenario::PROMOTED_CHILD) {
            nodes.layout->AddChild(nodes.retained);
        }
        SetTestRect(nodes.root, PROMOTION_VIEWPORT_RECT);
        SetTestRect(nodes.parent, OFFSCREEN_PARENT_RECT);
        SetTestRect(nodes.layout, ONSCREEN_LAYOUT_RECT);
        SetTestRect(nodes.retained,
            scenario == PromotionScenario::PRECEDING_SIBLING ? ONSCREEN_SIBLING_RECT : RETAINED_CHILD_RECT);
        return nodes;
    }

    void CheckVisiblePromotion(PromotionScenario scenario)
    {
        auto [root, parent, layout, retained] = CreatePromotionNodes(scenario);
        ASSERT_FALSE(HasFatalFailure());
        const bool hasRetainedChild = scenario != PromotionScenario::EMPTY_LAYOUT;
        const auto viewport = root->GetTransformRectRelativeToWindow();
        ASSERT_FALSE(parent->GetTransformRectRelativeToWindow().IsInnerIntersectWith(viewport));
        ASSERT_TRUE(layout->GetTransformRectRelativeToWindow().IsInnerIntersectWith(viewport));
        if (hasRetainedChild) {
            ASSERT_TRUE(retained->GetTransformRectRelativeToWindow().IsInnerIntersectWith(viewport));
        }
        ArkUITreeOptions options;
        options.filterPureLayout = true;
        TreeCollectionState state;
        auto config = MakeConfig(options, state);
        config.cacheNodes = false;
        config.rectCulling = true;
        ASSERT_TRUE(IsPureLayoutNode(layout, config));
        auto json = JsonUtil::CreateSharedPtrJson();
        root->DumpSimplifyTreeWithParamConfig(0, json, true, config);
        ASSERT_EQ(state.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(json->GetInt("$ID"), root->GetId());
        auto children = json->GetValue("$children");
        ASSERT_TRUE(children->IsArray());
        ASSERT_EQ(children->GetArraySize(), hasRetainedChild ? 1 : 0);
        if (hasRetainedChild) {
            auto collectedParent = children->GetArrayItem(0);
            EXPECT_EQ(collectedParent->GetInt("$ID"), parent->GetId());
            auto descendants = collectedParent->GetValue("$children");
            ASSERT_EQ(descendants->GetArraySize(), 1);
            EXPECT_EQ(descendants->GetArrayItem(0)->GetInt("$ID"), retained->GetId());
        }
        constexpr uint32_t ROOT_ONLY_COUNT = 1;
        constexpr uint32_t ROOT_PARENT_AND_CHILD_COUNT = 3;
        EXPECT_EQ(state.nodes, hasRetainedChild ? ROOT_PARENT_AND_CHILD_COUNT : ROOT_ONLY_COUNT);
        std::string output;
        ASSERT_EQ(Serialize(json, false, output), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(state.compactBytes, output.size());
    }
#endif
};

HWTEST_F(UITreeCollectorTest, DumpOptionsPreserveLegacyPatternOverrides, TestSize.Level1)
{
    RefPtr<Pattern> pattern = AceType::MakeRefPtr<LegacyInspectorPattern>();
    InspectorDumpOptions options;
    EXPECT_TRUE(options.includeDiagnostic);
    for (bool includeDiagnostic : { false, true }) {
        options.includeDiagnostic = includeDiagnostic;
        auto json = JsonUtil::CreateSharedPtrJson();
        pattern->DumpSimplifyInfo(json, options);
        EXPECT_EQ(json->GetString("legacyContent"), "retained");
    }
}

HWTEST_F(UITreeCollectorTest, DumpOptionsPreserveEmptyDerivedOutput, TestSize.Level1)
{
    std::vector<RefPtr<Pattern>> patterns { AceType::MakeRefPtr<LinearIndicatorPattern>(),
        AceType::MakeRefPtr<KeyboardPattern>(1) };
    for (const auto& pattern : patterns) {
        auto legacy = JsonUtil::CreateSharedPtrJson();
        pattern->DumpSimplifyInfo(legacy);
        EXPECT_EQ(legacy->ToString(), "{}");
        for (bool includeDiagnostic : { false, true }) {
            InspectorDumpOptions options;
            options.includeDiagnostic = includeDiagnostic;
            auto json = JsonUtil::CreateSharedPtrJson();
            pattern->DumpSimplifyInfo(json, options);
            EXPECT_EQ(json->ToString(), legacy->ToString());
        }
    }
}

HWTEST_F(UITreeCollectorTest, LegacyInteractionAccessibilityFieldOrder, TestSize.Level1)
{
    auto node = Frame(V2::TEXT_ETS_TAG);
    node->GetOrCreateFocusHub();
    auto property = node->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(property, nullptr);
    property->SetAccessibilityText("accessible text");
    constexpr unsigned COMBINATIONS = ACCESSIBILITY_INFO_MASK << 1;
    for (unsigned mask = 0; mask < COMBINATIONS; ++mask) {
        SCOPED_TRACE(mask);
        ParamConfig config;
        config.interactionInfo = mask & INTERACTION_INFO_MASK;
        config.accessibilityInfo = mask & ACCESSIBILITY_INFO_MASK;
        auto json = JsonUtil::CreateSharedPtrJson();
        node->DumpSimplifyInfoWithParamConfig(json, config);
        std::vector<std::string> expected;
        if (config.interactionInfo) {
            if (node->GetEventHub<EventHub>()->GetGestureEventHub()) {
                expected = { "clickable", "longClickable" };
            }
            expected.emplace_back("focusable");
        }
        if (config.accessibilityInfo) {
            expected.emplace_back("accessibilityContent");
            EXPECT_EQ(json->GetString("accessibilityContent"), "accessible text");
        }
        if (config.interactionInfo) {
            expected.emplace_back("scrollable");
            expected.emplace_back("editable");
            EXPECT_EQ(json->GetBool("scrollable"), property->IsScrollable());
            EXPECT_EQ(json->GetBool("editable"), property->IsEditable());
        }
        std::vector<std::string> actual;
        const std::set<std::string> fields {
            "clickable", "longClickable", "focusable", "accessibilityContent", "scrollable", "editable" };
        for (auto child = json->GetChild(); child && child->IsValid(); child = child->GetNext()) {
            if (fields.count(child->GetKey())) {
                actual.emplace_back(child->GetKey());
            }
        }
        EXPECT_EQ(actual, expected);
    }
}

HWTEST_F(UITreeCollectorTest, EmptyEnvelopeAndEscaping, TestSize.Level1)
{
    auto empty = JsonUtil::CreateSharedPtrJson();
    std::string output;
    EXPECT_EQ(Serialize(empty, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(output, R"({"schemaVersion":1,"treeInfo":null})");
    EXPECT_EQ(Serialize(empty, true, output), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(output, "{\n\t\"schemaVersion\":\t1,\n\t\"treeInfo\":\tnull\n}");
    auto node = Node();
    node->Put("content", "引号\"\n\t\\");
    ASSERT_EQ(Serialize(node, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    auto parsed = JsonUtil::ParseJsonString(output);
    ASSERT_TRUE(parsed->IsValid());
    EXPECT_EQ(parsed->GetInt("schemaVersion"), 1);
    EXPECT_FALSE(parsed->Contains("nodeCount"));
    EXPECT_EQ(parsed->GetObject("treeInfo")->GetString("content"), "引号\"\n\t\\");
}

HWTEST_F(UITreeCollectorTest, NodeLimit5000And5001, TestSize.Level1)
{
    auto root = Node("Root");
    auto children = JsonUtil::CreateArray();
    for (int i = 0; i < MAX_TEST_TREE_NODES - 1; ++i) {
        children->PutRef(Node());
    }
    root->PutRef("$children", std::move(children));
    std::string output;
    EXPECT_EQ(Serialize(root, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    root->GetValue("$children")->PutRef(Node());
    EXPECT_EQ(Serialize(root, false, output), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
    EXPECT_EQ(Serialize(root, true, output), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
}

HWTEST_F(UITreeCollectorTest, DeepTreeRespectsSerializedByteLimit, TestSize.Level1)
{
    auto root = Node();
    for (int i = 1; i < MAX_TEST_TREE_NODES; ++i) {
        auto parent = Node();
        auto children = JsonUtil::CreateArray();
        children->PutRef(std::move(root));
        parent->PutRef("$children", std::move(children));
        root = std::move(parent);
    }
    std::string output;
    EXPECT_EQ(Serialize(root, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    // Indentation alone crosses the independently enforced pretty byte limit.
    EXPECT_EQ(Serialize(root, true, output), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
}

#ifndef CROSS_PLATFORM
HWTEST_F(UITreeCollectorTest, CollectionStopsBeforeRemainingSiblingsAfterLimit, TestSize.Level1)
{
    auto root = AceType::MakeRefPtr<CollectionTestNode>();
    for (int i = 0; i < MAX_TEST_TREE_NODES + 1; ++i) {
        root->AddChild(AceType::MakeRefPtr<CollectionTestNode>());
    }
    auto unvisited = AceType::MakeRefPtr<CollectionTestNode>();
    root->AddChild(unvisited);
    bool visited = false;
    auto checker = [&visited, &unvisited](const RefPtr<UINode>& node) {
        if (node == unvisited) {
            visited = true;
        }
        return std::make_pair(true, false);
    };
    TreeCollectionState state;
    auto config = MakeConfig(ArkUITreeOptions {}, state);
    auto json = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, json, false, config, checker);
    EXPECT_EQ(state.GetError(), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
    EXPECT_EQ(state.nodes, MAX_TEST_TREE_NODES + 1);
    EXPECT_FALSE(visited);
    unvisited->DumpSimplifyTreeWithParamConfig(0, json, false, config, checker);
    EXPECT_FALSE(visited);
}

HWTEST_F(UITreeCollectorTest, CompactBudgetStopsBeforeRemainingSibling, TestSize.Level1)
{
    auto root = AceType::MakeRefPtr<CollectionTestNode>();
    auto oversized = AceType::MakeRefPtr<PayloadCollectionNode>();
    auto unvisited = AceType::MakeRefPtr<PayloadCollectionNode>();
    oversized->payload.assign(LIMIT, 'x');
    root->AddChild(oversized);
    root->AddChild(unvisited);
    TreeCollectionState state;
    auto config = MakeConfig(ArkUITreeOptions {}, state);
    auto json = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, json, false, config);
    EXPECT_EQ(state.GetError(), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
    EXPECT_EQ(oversized->dumpCalls, 1u);
    EXPECT_EQ(unvisited->dumpCalls, 0u);
    EXPECT_FALSE(json->Contains("$type"));
}

HWTEST_F(UITreeCollectorTest, CompactBudgetAcceptsExactLimitAndDefersPrettyExpansion, TestSize.Level1)
{
    auto root = AceType::MakeRefPtr<PayloadCollectionNode>();
    TreeCollectionState initialState;
    auto config = MakeConfig(ArkUITreeOptions {}, initialState);
    auto initial = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, initial, false, config);
    std::string output;
    ASSERT_EQ(Serialize(initial, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(initialState.compactBytes, output.size());
    root->payload.assign(LIMIT - output.size(), 'x');
    TreeCollectionState exactState;
    config.collectionState = &exactState;
    auto exact = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, exact, false, config);
    EXPECT_EQ(exactState.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(exactState.compactBytes, LIMIT);
    EXPECT_EQ(Serialize(exact, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(output.size(), LIMIT);
    EXPECT_EQ(Serialize(exact, true, output), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
    root->payload.push_back('x');
    TreeCollectionState exceededState;
    config.collectionState = &exceededState;
    auto exceeded = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, exceeded, false, config);
    EXPECT_EQ(exceededState.GetError(), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
}

HWTEST_F(UITreeCollectorTest, CompactBudgetRetainsPureLayoutRoot, TestSize.Level1)
{
    auto root = Frame("Row");
    auto child = Frame("Text");
    root->AddChild(child);
    ArkUITreeOptions options;
    options.filterPureLayout = true;
    TreeCollectionState state;
    auto config = MakeConfig(options, state);
    ASSERT_TRUE(IsPureLayoutNode(root, config));
    auto json = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, json, false, config);
    std::string output;
    ASSERT_EQ(state.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_EQ(Serialize(json, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state.compactBytes, output.size());
    EXPECT_EQ(state.nodes, ROOT_NODE_COUNT + root->GetChildren().size());
    EXPECT_EQ(state.traversalRoot, nullptr);
}

HWTEST_F(UITreeCollectorTest, CollectionAcceptsDeep5000NodesAndRejects5001, TestSize.Level1)
{
    std::vector<RefPtr<CollectionTestNode>> nodes;
    for (int i = 0; i < MAX_TEST_TREE_NODES; ++i) {
        auto node = AceType::MakeRefPtr<CollectionTestNode>();
        if (!nodes.empty()) {
            nodes.back()->AddChild(node);
        }
        nodes.push_back(node);
    }
    TreeCollectionState state;
    auto config = MakeConfig(ArkUITreeOptions {}, state);
    auto json = JsonUtil::CreateSharedPtrJson();
    nodes.front()->DumpSimplifyTreeWithParamConfig(0, json, false, config);
    EXPECT_EQ(state.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(state.nodes, MAX_TEST_TREE_NODES);
    std::string output;
    EXPECT_EQ(Serialize(json, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    auto cursor = json->GetValue("$children");
    for (size_t i = 1; i < nodes.size(); ++i) {
        auto child = cursor->GetArrayItem(0);
        EXPECT_EQ(child->GetInt("$ID"), nodes[i]->GetId());
        cursor = child->GetValue("$children");
    }
    auto extra = AceType::MakeRefPtr<CollectionTestNode>();
    nodes.back()->AddChild(extra);
    nodes.push_back(extra);
    TreeCollectionState overLimit;
    config.collectionState = &overLimit;
    auto rejected = JsonUtil::CreateSharedPtrJson();
    nodes.front()->DumpSimplifyTreeWithParamConfig(0, rejected, false, config);
    EXPECT_EQ(overLimit.GetError(), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
    // Tear down from the leaves so this test isolates collection from UINode destruction.
    for (size_t i = nodes.size() - 1; i > 0; --i) {
        nodes[i - 1]->RemoveChild(nodes[i]);
    }
}

HWTEST_F(UITreeCollectorTest, CollectionPreservesCachedAndDisappearingChildOrder, TestSize.Level1)
{
    auto root = AceType::MakeRefPtr<CollectionTestNode>(V2::JS_LAZY_FOR_EACH_ETS_TAG);
    auto normal = AceType::MakeRefPtr<CollectionTestNode>();
    auto firstCached = AceType::MakeRefPtr<CollectionTestNode>();
    auto secondCached = AceType::MakeRefPtr<CollectionTestNode>();
    auto disappearing = AceType::MakeRefPtr<CollectionTestNode>();
    root->AddChild(normal);
    root->cachedChildren = { firstCached, secondCached };
    root->AddDisappearingChild(disappearing);
    for (bool cacheNodes : { false, true }) {
        TreeCollectionState state;
        ParamConfig config;
        config.cacheNodes = cacheNodes;
        auto legacy = JsonUtil::CreateSharedPtrJson();
        root->DumpSimplifyTreeWithParamConfig(0, legacy, false, config);
        config.collectionState = &state;
        auto collected = JsonUtil::CreateSharedPtrJson();
        root->DumpSimplifyTreeWithParamConfig(0, collected, false, config);
        EXPECT_EQ(collected->ToString(), legacy->ToString());
        auto children = collected->GetValue("$children");
        ASSERT_EQ(children->GetArraySize(), cacheNodes ? CACHED_AND_DISAPPEARING_COUNT : NORMAL_AND_DISAPPEARING_COUNT);
        EXPECT_EQ(children->GetArrayItem(0)->GetInt("$ID"), (cacheNodes ? firstCached : normal)->GetId());
        EXPECT_EQ(children->GetArrayItem(children->GetArraySize() - 1)->GetInt("$ID"), disappearing->GetId());
        EXPECT_EQ(state.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
        std::string output;
        ASSERT_EQ(Serialize(collected, false, output), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(state.compactBytes, output.size());
    }
}
#endif

HWTEST_F(UITreeCollectorTest, ExactCompactAndPrettyByteLimits, TestSize.Level1)
{
    for (bool pretty : { false, true }) {
        auto root = Node();
        root->Put("content", "");
        std::string output;
        ASSERT_EQ(Serialize(root, pretty, output), ARKUI_ERROR_CODE_NO_ERROR);
        auto overhead = output.size();
        std::string text(LIMIT - overhead, 'x');
        root->Replace("content", text.c_str());
        EXPECT_EQ(Serialize(root, pretty, output), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(output.size(), LIMIT);
        text.push_back('x');
        root->Replace("content", text.c_str());
        EXPECT_EQ(Serialize(root, pretty, output), ARKUI_ERROR_CODE_RESULT_TOO_LARGE);
    }
}

HWTEST_F(UITreeCollectorTest, Utf8AndEscapesCountSerializedBytes, TestSize.Level1)
{
    auto root = Node();
    root->Put("content", "中\n");
    std::string output;
    ASSERT_EQ(Serialize(root, false, output), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(output.find("中\\n") != std::string::npos, true);
    auto parsed = JsonUtil::ParseJsonString(output);
    EXPECT_EQ(parsed->GetObject("treeInfo")->GetString("content"), "中\n");
}

HWTEST_F(UITreeCollectorTest, AdvancedGroupsAndNoDiagnosticInheritance, TestSize.Level1)
{
    auto node = Frame("TreeTest");
    for (unsigned mask = 0; mask < ADVANCED_OPTION_COMBINATIONS; ++mask) {
        ArkUITreeOptions options;
        options.visual = mask & ADVANCED_VISUAL_MASK;
        options.functional = mask & ADVANCED_FUNCTIONAL_MASK;
        TreeCollectionState state;
        auto config = MakeConfig(options, state);
        auto json = JsonUtil::CreateSharedPtrJson();
        node->DumpSimplifyInfoWithParamConfig(json, config);
        EXPECT_EQ(json->Contains("advanced"), mask != 0);
        if (!mask) {
            continue;
        }
        auto advanced = json->GetObject("advanced");
        EXPECT_EQ(advanced->Contains("visual"), options.visual);
        EXPECT_EQ(advanced->Contains("functional"), options.functional);
        EXPECT_FALSE(json->Contains("clickable"));
        EXPECT_FALSE(json->Contains("accessibilityContent"));
        EXPECT_EQ(json->ToString().find("indexInList"), std::string::npos);
        EXPECT_EQ(json->ToString().find("isLayouted"), std::string::npos);
        if (options.visual) {
            auto visual = advanced->GetObject("visual");
            EXPECT_TRUE(visual->Contains("FrameRect"));
            CheckFlatGroup(*visual);
        }
        if (options.functional) {
            auto functional = advanced->GetObject("functional");
            EXPECT_EQ(functional->ToString(), "{}");
            CheckFlatGroup(*functional);
        }
    }
}

HWTEST_F(UITreeCollectorTest, SelectedFunctionalGroupWithoutPropertiesIsEmpty, TestSize.Level1)
{
    auto column = Frame("Column", AceType::MakeRefPtr<LinearLayoutPattern>(true));
    for (bool visual : { false, true }) {
        ArkUITreeOptions options;
        options.visual = visual;
        options.functional = true;
        TreeCollectionState state;
        auto json = JsonUtil::CreateSharedPtrJson();
        column->DumpSimplifyInfoWithParamConfig(json, MakeConfig(options, state));
        auto advanced = json->GetObject("advanced");
        EXPECT_EQ(advanced->Contains("visual"), visual);
        EXPECT_EQ(advanced->GetObject("functional")->ToString(), "{}");
    }
}

HWTEST_F(UITreeCollectorTest, PureLayoutPromotionPreservesChildrenAndSemanticNodes, TestSize.Level1)
{
    ArkUITreeOptions options;
    options.filterPureLayout = true;
    TreeCollectionState state;
    auto config = MakeConfig(options, state);
    auto row = Frame("Row");
    ASSERT_TRUE(IsPureLayoutNode(row, config));
    auto child = JsonUtil::CreateSharedPtrJson();
    child->Put("$type", "Row");
    auto children = JsonUtil::CreateArray();
    auto first = Node("Text");
    first->Put("$ID", PROMOTED_TEXT_ID);
    auto second = Node("Button");
    second->Put("$ID", PROMOTED_BUTTON_ID);
    children->PutRef(std::move(first));
    children->PutRef(std::move(second));
    child->PutRef("$children", std::move(children));
    auto array = JsonUtil::CreateArray();
    AppendTreeChild(*array, std::move(child), row, config);
    ASSERT_EQ(array->GetArraySize(), PROMOTED_LEAF_COUNT);
    EXPECT_EQ(array->GetArrayItem(0)->GetInt("$ID"), PROMOTED_TEXT_ID);
    EXPECT_EQ(array->GetArrayItem(1)->GetInt("$ID"), PROMOTED_BUTTON_ID);
    row->UpdateInspectorId("semantic");
    EXPECT_FALSE(IsPureLayoutNode(row, config));
    EXPECT_FALSE(IsPureLayoutNode(Frame("Swiper"), config));
    EXPECT_FALSE(IsPureLayoutNode(Frame("Grid"), config));
}

HWTEST_F(UITreeCollectorTest, VisibilityZeroOpacityAndNoState, TestSize.Level1)
{
    auto node = Frame("Text");
    node->SetActive(true);
    ArkUITreeOptions options;
    ParamConfig emptyConfig;
    EXPECT_FALSE(ShouldPruneNode(node, true, emptyConfig));
    {
        TreeCollectionState state;
        auto config = MakeConfig(options, state);
        node->GetRenderContext()->UpdateOpacity(SMALL_POSITIVE_OPACITY);
        EXPECT_FALSE(ShouldPruneNode(node, true, config));
        // A separate node avoids UpdateOpacity's NearEqual short circuit for 1e-20 -> 0.
        auto zeroOpacityNode = Frame("Text");
        zeroOpacityNode->SetActive(true);
        zeroOpacityNode->GetRenderContext()->UpdateOpacity(0.0);
        ASSERT_DOUBLE_EQ(zeroOpacityNode->GetRenderContext()->GetOpacityValue(1.0), 0.0);
        EXPECT_TRUE(ShouldPruneNode(zeroOpacityNode, true, config));
        EXPECT_FALSE(ShouldPruneNode(zeroOpacityNode, false, config));
        EXPECT_FALSE(ShouldPruneNode(zeroOpacityNode, true, emptyConfig));
    }
    EXPECT_FALSE(ShouldPruneNode(node, true, emptyConfig));
}

HWTEST_F(UITreeCollectorTest, VisibleDeepTraversalReadsEachNodeOnce, TestSize.Level1)
{
    std::vector<RefPtr<VisibilityCountingNode>> nodes;
    for (int i = 0; i < VISIBILITY_TEST_DEPTH; ++i) {
        auto node = AceType::MakeRefPtr<VisibilityCountingNode>();
        if (!nodes.empty()) {
            nodes.back()->AddChild(node);
        }
        nodes.push_back(node);
    }
    TreeCollectionState state;
    auto config = MakeConfig(ArkUITreeOptions {}, state);
    config.cacheNodes = false;
    auto json = JsonUtil::CreateSharedPtrJson();
    nodes.front()->DumpSimplifyTreeWithParamConfig(0, json, true, config);
    EXPECT_EQ(state.nodes, nodes.size());
    for (const auto& node : nodes) {
        EXPECT_EQ(node->visibilityReads, 1u);
    }

    // A FULL request must not perform visibility filtering or build visibility snapshots.
    TreeCollectionState fullState;
    config.collectionState = &fullState;
    auto full = JsonUtil::CreateSharedPtrJson();
    nodes.front()->DumpSimplifyTreeWithParamConfig(0, full, false, config);
    EXPECT_TRUE(fullState.visibility.empty());
    for (const auto& node : nodes) {
        EXPECT_EQ(node->visibilityReads, 1u);
    }
    for (size_t i = nodes.size() - 1; i > 0; --i) {
        nodes[i - 1]->RemoveChild(nodes[i]);
    }
}

HWTEST_F(UITreeCollectorTest, VisibleStartingPointsShareAncestorsAndRefreshPerRequest, TestSize.Level1)
{
    auto root = AceType::MakeRefPtr<VisibilityCountingNode>();
    auto parent = AceType::MakeRefPtr<VisibilityCountingNode>();
    auto first = AceType::MakeRefPtr<VisibilityCountingNode>();
    auto second = AceType::MakeRefPtr<VisibilityCountingNode>();
    root->AddChild(parent);
    parent->AddChild(first);
    parent->AddChild(second);
    TreeCollectionState state;
    auto config = MakeConfig(ArkUITreeOptions {}, state);
    for (const auto& start : { first, second }) {
        auto json = JsonUtil::CreateSharedPtrJson();
        start->DumpSimplifyTreeWithParamConfig(0, json, true, config);
        EXPECT_TRUE(json->Contains("$type"));
    }
    for (const auto& node : { root, parent, first, second }) {
        EXPECT_EQ(node->visibilityReads, 1u);
    }

    // A new request must observe a changed ancestor instead of reusing the previous request's cache.
    root->visible = false;
    TreeCollectionState nextState;
    config.collectionState = &nextState;
    auto rejected = JsonUtil::CreateSharedPtrJson();
    first->DumpSimplifyTreeWithParamConfig(0, rejected, true, config);
    EXPECT_FALSE(rejected->Contains("$type"));
    constexpr uint32_t REQUEST_COUNT = 2;
    EXPECT_EQ(root->visibilityReads, REQUEST_COUNT);
    EXPECT_EQ(first->visibilityReads, 1u);
}

HWTEST_F(UITreeCollectorTest, ZeroOpacityPrunesBeforeVisitingDescendants, TestSize.Level1)
{
    auto root = Frame("Text");
    root->SetActive(true);
    root->GetRenderContext()->UpdateOpacity(0.0);
    std::vector<RefPtr<VisibilityCountingNode>> children;
    for (int i = 0; i < VISIBILITY_TEST_DEPTH; ++i) {
        auto child = AceType::MakeRefPtr<VisibilityCountingNode>();
        root->AddChild(child);
        children.push_back(child);
    }
    TreeCollectionState state;
    auto config = MakeConfig(ArkUITreeOptions {}, state);
    auto json = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, json, true, config);
    EXPECT_FALSE(json->Contains("$type"));
    EXPECT_EQ(state.nodes, 0u);
    for (const auto& child : children) {
        EXPECT_EQ(child->visibilityReads, 0u);
    }
}

HWTEST_F(UITreeCollectorTest, PositiveOpacityDeepChainPreservesLocalOutput, TestSize.Level1)
{
    // Multiplication would underflow; every actual opacity is positive and must be retained.
    std::vector<RefPtr<FrameNode>> nodes;
    for (int i = 0; i < OPACITY_UNDERFLOW_DEPTH; ++i) {
        auto node = Frame("Text");
        node->SetActive(true);
        node->GetRenderContext()->UpdateOpacity(POSITIVE_NODE_OPACITY);
        if (!nodes.empty()) {
            nodes.back()->AddChild(node);
        }
        nodes.push_back(node);
    }
    TreeCollectionState state;
    auto config = MakeConfig(ArkUITreeOptions {}, state);
    config.cacheNodes = false;
    auto json = JsonUtil::CreateSharedPtrJson();
    nodes.front()->DumpSimplifyTreeWithParamConfig(0, json, true, config);
    EXPECT_EQ(state.nodes, nodes.size());
    EXPECT_DOUBLE_EQ(json->GetDouble("opacity"), POSITIVE_NODE_OPACITY);
    auto child = json->GetValue("$children")->GetArrayItem(0);
    EXPECT_DOUBLE_EQ(child->GetDouble("opacity"), POSITIVE_NODE_OPACITY);
    for (size_t i = nodes.size() - 1; i > 0; --i) {
        nodes[i - 1]->RemoveChild(nodes[i]);
    }
}

HWTEST_F(UITreeCollectorTest, EmptyEventHubsDoNotChangeLayoutPurity, TestSize.Level1)
{
    auto row = Frame("Row");
    ArkUITreeOptions options;
    options.filterPureLayout = true;
    TreeCollectionState state;
    auto config = MakeConfig(options, state);
    ASSERT_TRUE(IsPureLayoutNode(row, config));
    row->GetOrCreateGestureEventHub();
    row->GetOrCreateInputEventHub();
    EXPECT_TRUE(IsPureLayoutNode(row, config));
    row->GetEventHub<EventHub>()->SetOnAppear([] {});
    EXPECT_FALSE(IsPureLayoutNode(row, config));
}

HWTEST_F(UITreeCollectorTest, PureLayoutPatternsRespectRenderContentWithDefaultFocusScope, TestSize.Level1)
{
    for (const auto& layout : LayoutFrames()) {
        SCOPED_TRACE(layout->GetTag());
        auto focus = layout->GetFocusHub();
        ASSERT_NE(focus, nullptr);
        ASSERT_EQ(focus->GetFocusType(), FocusType::SCOPE);
        auto root = Frame("Root");
        auto first = Frame("Text");
        auto second = Frame("Button");
        layout->AddChild(first);
        layout->AddChild(second);
        root->AddChild(layout);
        auto render = AceType::DynamicCast<MockRenderContext>(layout->GetRenderContext());
        ASSERT_NE(render, nullptr);
        for (unsigned flags = 0; flags < RENDER_FILTER_COMBINATIONS; ++flags) {
            SCOPED_TRACE(flags);
            bool filter = flags & FILTER_PURE_LAYOUT_MASK;
            bool hasRenderContent = flags & HAS_RENDER_CONTENT_MASK;
            bool promote = filter && !hasRenderContent;
            ON_CALL(*render, HasRenderContent()).WillByDefault(Return(hasRenderContent));
            ArkUITreeOptions options;
            options.filterPureLayout = filter;
            TreeCollectionState state;
            auto config = MakeConfig(options, state);
            EXPECT_EQ(IsPureLayoutNode(layout, config), promote);
            auto json = JsonUtil::CreateSharedPtrJson();
            root->DumpSimplifyTreeWithParamConfig(0, json, false, config);
            EXPECT_EQ(state.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
            auto children = json->GetValue("$children");
            ASSERT_EQ(children->GetArraySize(), promote ? PROMOTED_LEAF_COUNT : 1);
            if (!promote) {
                EXPECT_EQ(children->GetArrayItem(0)->GetInt("$ID"), layout->GetId());
                children = children->GetArrayItem(0)->GetValue("$children");
            }
            ASSERT_EQ(children->GetArraySize(), PROMOTED_LEAF_COUNT);
            EXPECT_EQ(children->GetArrayItem(0)->GetInt("$ID"), first->GetId());
            EXPECT_EQ(children->GetArrayItem(1)->GetInt("$ID"), second->GetId());
            EXPECT_EQ(state.nodes, PROMOTED_LEAF_COUNT + ROOT_NODE_COUNT + static_cast<uint32_t>(!promote));
            EXPECT_EQ(layout->GetFocusHub(), focus);
        }
    }
}

HWTEST_F(UITreeCollectorTest, PureLayoutPatternsRetainExplicitFocusInteraction, TestSize.Level1)
{
    const std::vector<std::pair<const char*, std::function<void(const RefPtr<FocusHub>&)>>> configurations {
        { "focusable", [](const auto& focus) { focus->SetFocusable(true); } },
        { "unfocusable", [](const auto& focus) { focus->SetFocusable(false); } },
        { "onFocus", [](const auto& focus) { focus->SetOnFocusCallback([] {}); } },
        { "onBlur", [](const auto& focus) { focus->SetOnBlurCallback([] {}); } },
        { "onKey", [](const auto& focus) { focus->SetOnKeyCallback([](KeyEventInfo&) { return false; }); } },
        { "tabIndex", [](const auto& focus) { focus->SetTabIndex(1); } },
        { "defaultFocus", [](const auto& focus) { focus->SetIsDefaultFocus(true); } },
        { "focusOnTouch", [](const auto& focus) { focus->SetIsFocusOnTouch(true); } },
        { "tabStop", [](const auto& focus) { focus->SetTabStop(true); } },
        { "focusScope", [](const auto& focus) { focus->SetFocusScopeId(std::to_string(focus->GetFrameId()), true); } },
        { "nextFocus", [](const auto& focus) { focus->SetNextFocus(FocusIntension::TAB, std::string("next")); } },
    };
    for (const auto& [name, configure] : configurations) {
        SCOPED_TRACE(name);
        for (const auto& layout : LayoutFrames()) {
            SCOPED_TRACE(layout->GetTag());
            ArkUITreeOptions options;
            options.filterPureLayout = true;
            TreeCollectionState state;
            auto config = MakeConfig(options, state);
            ASSERT_TRUE(IsPureLayoutNode(layout, config));
            configure(layout->GetFocusHub());
            EXPECT_FALSE(IsPureLayoutNode(layout, config));
        }
    }
}

HWTEST_F(UITreeCollectorTest, ClearedFocusCallbacksRestoreLayoutPurity, TestSize.Level1)
{
    for (const auto& layout : LayoutFrames()) {
        SCOPED_TRACE(layout->GetTag());
        ArkUITreeOptions options;
        options.filterPureLayout = true;
        TreeCollectionState state;
        auto config = MakeConfig(options, state);
        auto focus = layout->GetFocusHub();
        focus->SetOnKeyCallback([](KeyEventInfo&) { return false; });
        EXPECT_FALSE(IsPureLayoutNode(layout, config));
        focus->ClearOnKeyCallback();
        EXPECT_TRUE(IsPureLayoutNode(layout, config));
        focus->SetOnKeyEventInternal([](const KeyEvent&) { return false; });
        EXPECT_FALSE(IsPureLayoutNode(layout, config));
        focus->SetOnKeyEventInternal(nullptr);
        EXPECT_TRUE(IsPureLayoutNode(layout, config));
    }
}

HWTEST_F(UITreeCollectorTest, CallbackSetsAndShortcutsTrackRegisteredCallbacks, TestSize.Level1)
{
    auto row = Frame(V2::ROW_ETS_TAG, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    auto hub = row->GetEventHub<EventHub>();
    ArkUITreeOptions options;
    options.filterPureLayout = true;
    TreeCollectionState state;
    auto config = MakeConfig(options, state);
    auto verify = [row, &config](const std::function<void()>& set, const std::function<void()>& clear) {
        EXPECT_TRUE(IsPureLayoutNode(row, config));
        set();
        EXPECT_FALSE(IsPureLayoutNode(row, config));
        clear();
        EXPECT_TRUE(IsPureLayoutNode(row, config));
    };
    verify([hub] { hub->SetOnAppear([] {}); }, [hub] { hub->ClearUserOnAppear(); });
    verify([hub] { hub->SetOnDisappear([] {}); }, [hub] { hub->ClearUserOnDisAppear(); });
    verify([hub] { hub->SetFrameNodeCommonOnAppear([] {}); }, [hub] { hub->ClearJSFrameNodeOnAppear(); });
    verify([hub] { hub->SetFrameNodeCommonOnDisappear([] {}); }, [hub] { hub->ClearJSFrameNodeOnDisappear(); });
    verify([hub] { hub->SetOnAttach([] {}); }, [hub] { hub->ClearOnAttach(); });
    verify([hub] { hub->SetOnDetach([] {}); }, [hub] { hub->ClearOnDetach(); });
    verify([hub] { hub->SetOnAreaChanged([](const RectF&, const OffsetF&, const RectF&, const OffsetF&) {}); },
        [hub] { hub->ClearUserOnAreaChanged(); });
    verify(
        [hub] {
            hub->AddInnerOnAreaChangedCallback(
                INNER_CALLBACK_ID, [](const RectF&, const OffsetF&, const RectF&, const OffsetF&) {});
        },
        [hub] { hub->ClearOnAreaChangedInnerCallbacks(); });
    verify(
        [hub] { hub->SetOnSizeChanged([](const RectF&, const RectF&) {}); }, [hub] { hub->SetOnSizeChanged(nullptr); });
    verify([hub] { hub->AddInnerOnSizeChanged(INNER_CALLBACK_ID, [](const RectF&, const RectF&) {}); },
        [hub] { hub->ClearInnerOnSizeChanged(); });
    verify([hub] { hub->SetFrameNodeCommonOnSizeChangeCallback([](const RectF&, const RectF&) {}); },
        [hub] { hub->ClearJSFrameNodeOnSizeChange(); });
    verify([hub] { hub->SetNDKDrawCompletedCallback([] {}); }, [hub] { hub->SetNDKDrawCompletedCallback(nullptr); });
    verify([hub] { hub->SetNDKLayoutCallback([] {}); }, [hub] { hub->SetNDKLayoutCallback(nullptr); });
    verify([hub] { hub->SetKeyboardShortcut("a", NO_SHORTCUT_MODIFIERS, [] {}); },
        [hub] { hub->ClearSingleKeyboardShortcutAll(); });
}

#ifndef CROSS_PLATFORM
HWTEST_F(UITreeCollectorTest, VisiblePureLayoutPromotionUsesRetainedDescendants, TestSize.Level1)
{
    for (auto scenario :
        { PromotionScenario::EMPTY_LAYOUT, PromotionScenario::PROMOTED_CHILD, PromotionScenario::PRECEDING_SIBLING }) {
        SCOPED_TRACE(static_cast<int>(scenario));
        ASSERT_NO_FATAL_FAILURE(CheckVisiblePromotion(scenario));
    }
}

HWTEST_F(UITreeCollectorTest, NestedPromotionPreservesSiblingOrder, TestSize.Level1)
{
    constexpr size_t LAYOUT_DEPTH = 3;
    constexpr size_t LEAF_COUNT = 2;
    for (bool hasPrecedingSibling : { false, true }) {
        SCOPED_TRACE(hasPrecedingSibling);
        auto root = Frame("Root");
        auto parent = root;
        std::vector<RefPtr<FrameNode>> expected;
        if (hasPrecedingSibling) {
            expected.push_back(Frame("Text"));
            root->AddChild(expected.back());
        }
        for (size_t level = 0; level < LAYOUT_DEPTH; ++level) {
            auto layout = Frame("Row");
            parent->AddChild(layout);
            parent = layout;
        }
        for (size_t index = 0; index < LEAF_COUNT; ++index) {
            expected.push_back(Frame("Text"));
            parent->AddChild(expected.back());
        }
        expected.push_back(Frame("Button"));
        root->AddChild(expected.back());
        ArkUITreeOptions options;
        options.filterPureLayout = true;
        TreeCollectionState state;
        auto config = MakeConfig(options, state);
        auto json = JsonUtil::CreateSharedPtrJson();
        root->DumpSimplifyTreeWithParamConfig(0, json, false, config);
        ASSERT_EQ(state.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(json->GetInt("$ID"), root->GetId());
        auto children = json->GetValue("$children");
        ASSERT_EQ(children->GetArraySize(), expected.size());
        for (size_t index = 0; index < expected.size(); ++index) {
            EXPECT_EQ(children->GetArrayItem(index)->GetInt("$ID"), expected[index]->GetId());
        }
        EXPECT_EQ(state.nodes, expected.size() + ROOT_NODE_COUNT);
        std::string output;
        ASSERT_EQ(Serialize(json, false, output), ARKUI_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(state.compactBytes, output.size());
    }
}

HWTEST_F(UITreeCollectorTest, SharedTraversalRetainsRootAndPromotesInOrder, TestSize.Level1)
{
    auto root = Frame("Root");
    auto layout = Frame("Row");
    auto first = Frame("Text");
    auto second = Frame("Button");
    auto last = Frame("Text");
    layout->AddChild(first);
    layout->AddChild(second);
    root->AddChild(layout);
    root->AddChild(last);
    ParamConfig legacyConfig;
    auto legacy = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, legacy, false, legacyConfig);
    ASSERT_EQ(legacy->GetValue("$children")->GetArraySize(), root->GetChildren().size());
    EXPECT_FALSE(legacy->Contains("advanced"));
    ArkUITreeOptions options;
    options.filterPureLayout = true;
    TreeCollectionState state;
    auto config = MakeConfig(options, state);
    auto filtered = JsonUtil::CreateSharedPtrJson();
    root->DumpSimplifyTreeWithParamConfig(0, filtered, false, config);
    EXPECT_EQ(filtered->GetInt("$ID"), root->GetId());
    auto children = filtered->GetValue("$children");
    const std::vector<int32_t> expectedIds { first->GetId(), second->GetId(), last->GetId() };
    ASSERT_EQ(children->GetArraySize(), expectedIds.size());
    for (size_t index = 0; index < expectedIds.size(); ++index) {
        EXPECT_EQ(children->GetArrayItem(index)->GetInt("$ID"), expectedIds[index]);
    }
    EXPECT_EQ(state.GetError(), ARKUI_ERROR_CODE_NO_ERROR);
}
#endif

HWTEST_F(UITreeCollectorTest, LegacyDumpCommonFieldsMatchBaselineSnapshot, TestSize.Level1)
{
    auto node = Frame(V2::TEXT_ETS_TAG);
    SetTestRect(node, RectF(1.0f, 2.0f, 3.0f, 4.0f));
    node->UpdateInspectorId("legacy-node");
    auto focusHub = node->GetOrCreateFocusHub();
    focusHub->SetFocusType(FocusType::NODE);
    focusHub->SetFocusable(true);
    auto property = node->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(property, nullptr);
    property->SetAccessibilityText("accessible text");
    ParamConfig config;
    config.interactionInfo = true;
    config.accessibilityInfo = true;
    auto json = JsonUtil::CreateSharedPtrJson();
    node->DumpSimplifyInfoWithParamConfig(json, config);
    // Frozen from fdf7d63988ed9fa338051a11c19fadf1f86653dc; compare serialized key order and types.
    EXPECT_EQ(json->ToString(), R"({"$rect":"[1.00, 2.00],[4.00,6.00]","compid":"legacy-node","$attrs":{},)"
        R"("clickable":false,"longClickable":false,"focusable":true,)"
        R"("accessibilityContent":"accessible text","scrollable":false,"editable":false})");
}

HWTEST_F(UITreeCollectorTest, LegacyDumpTextMissingHostMatchesBaselineSnapshot, TestSize.Level1)
{
    class LegacyTextDumpPattern : public TextPattern {
    public:
        LegacyTextDumpPattern()
        {
            contentRect_ = RectF(1.0f, 2.0f, 3.0f, 4.0f);
        }
    };
    ASSERT_FALSE(SystemProperties::GetDebugEnabled());
    auto pattern = AceType::MakeRefPtr<LegacyTextDumpPattern>();
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json);
    // Frozen from fdf7d63988ed9fa338051a11c19fadf1f86653dc; compare serialized key order and types.
    EXPECT_EQ(json->ToString(),
        R"({"contentRect":"RectT (1.00, 2.00) - [3.00 x 4.00]","BindSelectionMenu":"1"})");
}

HWTEST_F(UITreeCollectorTest, LegacyDumpSheetDuplicateOffsetsMatchBaselineSnapshot, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<SheetPresentationPattern>(77, "target", [](const std::string&) {});
    pattern->SetCenterHeight(100.0f);
    pattern->SetSheetMaxHeight(300.0f);
    pattern->SetPageHeight(400.0f);
    pattern->SetSheetOffsetX(11.0f);
    pattern->SetSheetOffsetY(22.0f);
    pattern->SetSheetMaxWidth(500.0f);
    pattern->SetCurrentOffset(7.0f);
    pattern->SetSheetHeightUp(13.0f);
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json);
    // Frozen from fdf7d63988ed9fa338051a11c19fadf1f86653dc; compare serialized key order and types.
    const std::string expected = R"({"TargetId":77,"TargetTag":"target","SheetPage Pattern":{"SheetType":0,)"
        R"("SheetPage Node Height":100,"Sheet Height [start from the bottom, KeyboardHeight = 0]":0,)"
        R"("SheetMaxHeight [start from the bottom, pageHeight - sheetTopSafeArea]":300,"Page Height":400,)"
        R"("StatusBar Height [current sheetType needed]":0,"PopupSheet OffsetX":11,"PopupSheet OffsetX":22,)"
        R"("SheetMaxWidth":500,"FitContent Height":0,"SheetThemeType":"auto","currentOffset":7},)"
        R"("Height ScrollTo [KeyboardHeight > 0, and is scrolling]":0,"KeyboardHeight":0,)"
        R"("is scrolling":false,"SheetHeightUp[sheet offset to move up when avoiding keyboard]":13})";
    EXPECT_EQ(json->ToString(), expected);
}

namespace {
ParamConfig PublicPropertyConfig(bool visual, bool functional)
{
    ParamConfig config;
    config.dumpMode = TreeDumpMode::PUBLIC_TREE;
    config.visualProperties = visual;
    config.functionalProperties = functional;
    return config;
}

class TreeOpacityListItem : public ListItemPattern {
public:
    TreeOpacityListItem() : ListItemPattern(nullptr) {}
    void SaveOpacity(double opacity) { enableOpacity_ = opacity; }
};
} // namespace

HWTEST_F(UITreeCollectorTest, ConfirmedTextScalarTypesAndLegacyMenuMeaning, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TextPattern>();
    auto host = Frame(V2::TEXT_ETS_TAG, pattern);
    auto publicJson = JsonUtil::Create();
    pattern->DumpAdvanceInfo(publicJson, PublicPropertyConfig(true, true));
    ASSERT_TRUE(publicJson->GetValue("BindSelectionMenu")->IsBool());
    EXPECT_FALSE(publicJson->GetBool("BindSelectionMenu"));
    EXPECT_TRUE(publicJson->GetValue("fontScale")->IsNumber());
    EXPECT_TRUE(publicJson->GetValue("fontWeightScale")->IsNumber());
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_EQ(legacy->GetString("BindSelectionMenu"), "1");
    EXPECT_TRUE(legacy->GetValue("fontScale")->IsString());
}

HWTEST_F(UITreeCollectorTest, ConfirmedScrollPaginationAndBooleanTypes, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ScrollPattern>();
    auto host = Frame(V2::SCROLL_ETS_TAG, pattern);
    pattern->SetSnapPaginations({ Dimension(10, DimensionUnit::VP), Dimension(20, DimensionUnit::PX) });
    pattern->SetEnableSnapToSide({ true, false });
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    auto offsets = json->GetObject("snapPagination")->GetValue("offsets");
    ASSERT_TRUE(offsets->IsArray());
    ASSERT_EQ(offsets->GetArraySize(), 2);
    EXPECT_EQ(offsets->GetArrayItem(0)->GetString(), "10.00vp");
    EXPECT_EQ(offsets->GetArrayItem(1)->GetString(), "20.00px");
    EXPECT_TRUE(json->GetValue("enableSnapToStart")->IsBool());
    EXPECT_TRUE(json->GetBool("enableSnapToStart"));
    EXPECT_FALSE(json->GetBool("enableSnapToEnd"));
    EXPECT_TRUE(json->GetValue("hasOnScrollStart")->IsBool());
    EXPECT_TRUE(json->GetValue("hasOnScrollBegin")->IsBool());
    EXPECT_TRUE(json->GetValue("enablePaging")->IsString());
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_EQ(legacy->GetString("snapPagination"), "[10.00vp 20.00px ]");
    EXPECT_EQ(legacy->GetString("enableSnapToStart"), "true");
    pattern->SetSnapPaginations({});
    pattern->SetIntervalSize(Dimension(50, DimensionUnit::VP));
    json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    EXPECT_EQ(json->GetObject("snapPagination")->GetString("interval"), "50.00vp");
    EXPECT_FALSE(json->GetObject("snapPagination")->Contains("offsets"));
}

HWTEST_F(UITreeCollectorTest, ConfirmedGridOptionsAndScalarTypes, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<GridPattern>();
    auto host = Frame(V2::GRID_ETS_TAG, pattern);
    auto property = host->GetLayoutProperty<GridLayoutProperty>();
    property->UpdateMaxCount(4);
    property->UpdateMinCount(1);
    property->UpdateCachedCount(2);
    property->UpdateEditable(true);
    GridLayoutOptions options;
    options.irregularIndexes = { 1, 3 };
    property->UpdateLayoutOptions(options);
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(true, true));
    EXPECT_TRUE(json->GetValue("MaxCount")->IsNumber());
    EXPECT_EQ(json->GetInt("MaxCount"), 4);
    EXPECT_TRUE(json->GetValue("CachedCount")->IsNumber());
    EXPECT_TRUE(json->GetValue("Editable")->IsBool());
    auto indexes = json->GetObject("GridLayoutOptions")->GetValue("irregularIndexes");
    ASSERT_TRUE(indexes->IsArray());
    ASSERT_EQ(indexes->GetArraySize(), 2);
    EXPECT_EQ(indexes->GetArrayItem(0)->GetInt(), 1);
    EXPECT_EQ(indexes->GetArrayItem(1)->GetInt(), 3);
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_EQ(legacy->GetString("MaxCount"), "4");
    EXPECT_EQ(legacy->GetString("Editable"), "1");
    EXPECT_EQ(legacy->GetString("GridLayoutOptions"), "IrregularIndexes: [1, 3]");
}

HWTEST_F(UITreeCollectorTest, ConfirmedTabRegionArrayAndLegacyText, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TabBarPattern>();
    auto host = Frame(V2::TAB_BAR_ETS_TAG, pattern);
    pattern->SetAnimationDuration(120);
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(true, false));
    EXPECT_TRUE(json->GetValue("animationDuration")->IsNumber());
    auto region = json->GetValue("region");
    ASSERT_TRUE(region->IsArray());
    ASSERT_EQ(region->GetArraySize(), 4);
    for (int i = 0; i < region->GetArraySize(); ++i) {
        EXPECT_TRUE(region->GetArrayItem(i)->IsBool());
        EXPECT_FALSE(region->GetArrayItem(i)->GetBool());
    }
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_EQ(legacy->GetString("region"), "false false false false ");
    EXPECT_EQ(legacy->GetString("animationDuration"), "120");
}

HWTEST_F(UITreeCollectorTest, ConfirmedRestoredOpacityKeepsLegacyValue, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<TreeOpacityListItem>();
    auto host = Frame(V2::LIST_ITEM_ETS_TAG, pattern);
    pattern->SaveOpacity(0.5);
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(true, false));
    EXPECT_FALSE(json->Contains("enableOpacity"));
    EXPECT_TRUE(json->GetValue("restoredOpacity")->IsNumber());
    EXPECT_DOUBLE_EQ(json->GetDouble("restoredOpacity"), 0.5);
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_EQ(legacy->GetString("enableOpacity"), "true:");
    EXPECT_FALSE(legacy->Contains("restoredOpacity"));
}

HWTEST_F(UITreeCollectorTest, ConfirmedGradientColorsUnitsAndTypeParameters, TestSize.Level1)
{
    Gradient gradient;
    gradient.CreateGradientWithType(GradientType::LINEAR);
    gradient.SetRepeat(true);
    gradient.GetLinearGradient()->angle = Dimension(90, DimensionUnit::PX);
    gradient.GetLinearGradient()->linearY = GradientDirection::BOTTOM;
    GradientColor first;
    first.SetColor(Color::RED);
    first.SetDimension(Dimension(50, DimensionUnit::PERCENT));
    gradient.AddColor(first);
    GradientColor second;
    second.SetColor(Color::BLUE);
    second.SetHasValue(false);
    gradient.AddColor(second);
    auto json = GradientJsonUtils::ToTreeJson(gradient);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("type"), "linear");
    EXPECT_TRUE(json->GetValue("angle")->IsNumber());
    EXPECT_DOUBLE_EQ(json->GetDouble("angle"), 90);
    EXPECT_EQ(json->GetString("direction"), "GradientDirection.Bottom");
    EXPECT_TRUE(json->GetValue("repeating")->IsBool());
    EXPECT_TRUE(json->GetBool("repeating"));
    auto colors = json->GetValue("colors");
    ASSERT_EQ(colors->GetArraySize(), 2);
    EXPECT_EQ(colors->GetArrayItem(0)->GetString("color"), "#FFFF0000");
    EXPECT_EQ(colors->GetArrayItem(0)->GetString("offset"), "50.00%");
    EXPECT_FALSE(colors->GetArrayItem(1)->Contains("offset"));
    auto legacy = GradientJsonUtils::LinearGradientToJson(gradient);
    EXPECT_TRUE(legacy->GetValue("angle")->IsString());
    EXPECT_EQ(legacy->GetString("repeating"), "true");
    gradient.CreateGradientWithType(GradientType::SWEEP);
    gradient.GetSweepGradient()->centerX = Dimension(0.5, DimensionUnit::PERCENT);
    gradient.GetSweepGradient()->rotation = Dimension(45);
    json = GradientJsonUtils::ToTreeJson(gradient);
    EXPECT_EQ(json->GetString("centerX"), "50.00%");
    EXPECT_DOUBLE_EQ(json->GetDouble("rotation"), 45);
    EXPECT_FALSE(json->Contains("centerY"));
    gradient.CreateGradientWithType(GradientType::RADIAL);
    gradient.GetRadialGradient()->radialHorizontalSize = Dimension(12, DimensionUnit::VP);
    json = GradientJsonUtils::ToTreeJson(gradient);
    EXPECT_EQ(json->GetString("horizontalSize"), "12.00vp");
    EXPECT_FALSE(json->Contains("verticalSize"));
}

HWTEST_F(UITreeCollectorTest, ConfirmedKeyboardShortcutReadsWithoutInvokingCallbacks, TestSize.Level1)
{
    auto host = Frame("Button");
    int callbacks = 0;
    host->GetEventHub<EventHub>()->SetKeyboardShortcut("K", 3, [&callbacks] { ++callbacks; });
    TreeCollectionState state;
    auto config = PublicPropertyConfig(false, true);
    config.collectionState = &state;
    auto json = JsonUtil::CreateSharedPtrJson();
    host->DumpSimplifyInfoWithParamConfig(json, config);
    auto shortcuts = json->GetObject("advanced")->GetObject("functional")->GetValue("keyboardShortcuts");
    ASSERT_TRUE(shortcuts->IsArray());
    ASSERT_EQ(shortcuts->GetArraySize(), 1);
    EXPECT_EQ(shortcuts->GetArrayItem(0)->GetString("value"), "K");
    EXPECT_EQ(shortcuts->GetArrayItem(0)->GetInt("keys"), 3);
    EXPECT_FALSE(shortcuts->GetArrayItem(0)->Contains("onKeyboardShortcutAction"));
    EXPECT_EQ(callbacks, 0);
    config.functionalProperties = false;
    json = JsonUtil::CreateSharedPtrJson();
    host->DumpSimplifyInfoWithParamConfig(json, config);
    EXPECT_FALSE(json->Contains("advanced"));
}

HWTEST_F(UITreeCollectorTest, ConfirmedWaterFlowSectionsOmitCallbacksAndAbsentValues, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<WaterFlowPattern>();
    auto host = Frame(V2::WATERFLOW_ETS_TAG, pattern);
    int callbacks = 0;
    WaterFlowSections::Section section;
    section.itemsCount = 4;
    section.crossCount = 2;
    section.rowsGap = Dimension(8, DimensionUnit::VP);
    section.onGetItemMainSizeByIndex = [&callbacks](int32_t) { ++callbacks; return 20.0f; };
    pattern->GetOrCreateWaterFlowSections()->ChangeData(0, 0, { section });
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    auto sections = json->GetValue("sections");
    ASSERT_TRUE(sections->IsArray());
    ASSERT_EQ(sections->GetArraySize(), 1);
    auto entry = sections->GetArrayItem(0);
    EXPECT_EQ(entry->GetInt("itemsCount"), 4);
    EXPECT_EQ(entry->GetInt("crossCount"), 2);
    EXPECT_EQ(entry->GetString("rowsGap"), "8.00vp");
    EXPECT_FALSE(entry->Contains("columnsGap"));
    EXPECT_FALSE(entry->Contains("onGetItemMainSizeByIndex"));
    EXPECT_EQ(callbacks, 0);
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_FALSE(legacy->Contains("sections"));
}

HWTEST_F(UITreeCollectorTest, ConfirmedAdditionalNumericReadersPreserveLegacyTypes, TestSize.Level1)
{
    auto list = AceType::MakeRefPtr<ListPattern>();
    auto listHost = Frame(V2::LIST_ETS_TAG, list);
    auto json = JsonUtil::Create();
    list->DumpAdvanceInfo(json, PublicPropertyConfig(true, true));
    EXPECT_TRUE(json->GetValue("lanes")->IsNumber());
    EXPECT_TRUE(json->GetValue("laneGutter")->IsNumber());
    EXPECT_TRUE(json->GetValue("alignListItem")->IsNumber());
    EXPECT_TRUE(json->GetValue("sticky")->IsNumber());
    auto legacy = JsonUtil::Create();
    list->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_TRUE(legacy->GetValue("lanes")->IsString());
    EXPECT_TRUE(legacy->GetValue("sticky")->IsString());
    auto item = AceType::MakeRefPtr<GridItemPattern>(nullptr);
    auto itemHost = Frame(V2::GRID_ITEM_ETS_TAG, item);
    itemHost->GetLayoutProperty<GridItemLayoutProperty>()->UpdateRowStart(0);
    json = JsonUtil::Create();
    item->DumpAdvanceInfo(json, PublicPropertyConfig(true, false));
    EXPECT_TRUE(json->GetValue("RowStart")->IsNumber());
    EXPECT_EQ(json->GetInt("RowStart"), 0);
    auto swiper = AceType::MakeRefPtr<SwiperPattern>();
    auto swiperHost = Frame(V2::SWIPER_ETS_TAG, swiper);
    json = JsonUtil::Create();
    swiper->DumpAdvanceInfo(json, PublicPropertyConfig(true, false));
    EXPECT_TRUE(json->GetValue("currentOffset")->IsNumber());
    EXPECT_TRUE(json->GetValue("fadeOffset")->IsNumber());
}

HWTEST_F(UITreeCollectorTest, ConfirmedGradientHdrAndMissingDirection, TestSize.Level1)
{
    Gradient gradient;
    gradient.CreateGradientWithType(GradientType::LINEAR);
    ColorWithHeadRoom channels;
    channels.red = 0.25f;
    channels.green = 0.5f;
    channels.blue = 0.75f;
    channels.headRoom = 2.0f;
    Color color(channels);
    color.SetColorSpace(ColorSpace::DISPLAY_P3);
    GradientColor stop;
    stop.SetColor(color);
    stop.SetDimension(Dimension(12, DimensionUnit::VP));
    gradient.AddColor(stop);
    auto json = GradientJsonUtils::ToTreeJson(gradient);
    EXPECT_FALSE(json->Contains("direction"));
    EXPECT_FALSE(json->Contains("angle"));
    auto entry = json->GetValue("colors")->GetArrayItem(0);
    EXPECT_EQ(entry->GetInt("colorSpace"), 1);
    EXPECT_EQ(entry->GetString("offset"), "12.00vp");
    EXPECT_DOUBLE_EQ(entry->GetObject("hdr")->GetDouble("red"), 0.25);
    EXPECT_DOUBLE_EQ(entry->GetObject("hdr")->GetDouble("headRoom"), 2.0);
}

HWTEST_F(UITreeCollectorTest, ConfirmedImageSourceReferencesAndOtherSourceTypes, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ImagePattern>();
    auto host = Frame(V2::IMAGE_ETS_TAG, pattern);
    auto property = host->GetLayoutProperty<ImageLayoutProperty>();
    auto json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    EXPECT_FALSE(json->Contains("url"));
    ImageSourceInfo resource;
    resource.SetResourceId(static_cast<InternalResource::ResourceId>(123));
    property->UpdateImageSourceInfo(resource);
    json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    ASSERT_TRUE(json->GetValue("url")->IsObject());
    EXPECT_EQ(json->GetObject("url")->GetInt("id"), 123);
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy, ParamConfig());
    EXPECT_TRUE(legacy->GetValue("url")->IsString());
    const std::string uri = "https://example.test/image.png";
    property->UpdateImageSourceInfo(ImageSourceInfo(uri));
    json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    EXPECT_EQ(json->GetString("url"), uri);
    const auto data = std::string("data:image/png;base64,") + std::string(80, 'A');
    property->UpdateImageSourceInfo(ImageSourceInfo(data));
    json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    ASSERT_TRUE(json->GetValue("url")->IsObject());
    EXPECT_EQ(json->GetObject("url")->GetString("sourceType"), "base64");
    EXPECT_EQ(json->GetObject("url")->ToString(), R"({"sourceType":"base64"})");
    property->UpdateImageSourceInfo(ImageSourceInfo(""));
    json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
    EXPECT_EQ(json->GetObject("url")->GetString("sourceType"), "unsupported");
    json = JsonUtil::Create();
    pattern->DumpAdvanceInfo(json, PublicPropertyConfig(true, false));
    EXPECT_FALSE(json->Contains("url"));
}

HWTEST_F(UITreeCollectorTest, ConfirmedOtherImageSourcesOnlyExposeType, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ImagePattern>();
    auto host = Frame(V2::IMAGE_ETS_TAG, pattern);
    auto property = host->GetLayoutProperty<ImageLayoutProperty>();
    const std::pair<const char*, const char*> cases[] = {
        { "memory://image", "memory" }, { "internal://cached-image", "internal" },
        { "unknown-scheme://image", "unsupported" }
    };
    for (const auto& [source, type] : cases) {
        property->UpdateImageSourceInfo(ImageSourceInfo(source));
        auto json = JsonUtil::Create();
        pattern->DumpAdvanceInfo(json, PublicPropertyConfig(false, true));
        auto description = json->GetObject("url");
        EXPECT_EQ(description->ToString(), std::string("{\"sourceType\":\"") + type + "\"}");
    }
}

} // namespace OHOS::Ace::NG
