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

#include <string>

#include "gtest/gtest.h"

#define protected public
#define private public
#include "base_shape_pattern_test_ng.h"
#include "test/mock/core/rosen/mock_canvas.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/shape/path_model_ng.h"
#include "core/components_ng/pattern/shape/path_paint_property.h"
#include "core/components_ng/pattern/shape/path_pattern.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {

const std::string PATH_CMD = "M150 0 L300 300 L0 300 Z";

} // namespace

class PathPatternTestNg : public BaseShapePatternTestNg {
public:
    RefPtr<FrameNode> CreadFrameNode() override
    {
        PathModelNG().Create();
        return AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    }

    void Draw(RefPtr<FrameNode> frameNode) override
    {
        EXPECT_EQ(frameNode == nullptr, false);
        auto pattern = frameNode->GetPattern<PathPattern>();
        EXPECT_EQ(pattern == nullptr, false);
        auto layoutProperty = frameNode->GetLayoutProperty();
        EXPECT_EQ(layoutProperty == nullptr, false);
        auto layoutAlgorithm = AceType::DynamicCast<ShapeLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
        EXPECT_EQ(layoutAlgorithm == nullptr, false);
        LayoutConstraintF layoutConstraint;
        layoutConstraint.percentReference.SetWidth(WIDTH);
        layoutConstraint.percentReference.SetHeight(HEIGHT);
        layoutConstraint.selfIdealSize.SetWidth(WIDTH);
        layoutConstraint.selfIdealSize.SetHeight(HEIGHT);
        layoutProperty->UpdateLayoutConstraint(layoutConstraint);
        layoutProperty->UpdateContentConstraint();
        auto size = layoutAlgorithm->MeasureContent(layoutProperty->CreateContentConstraint(), nullptr);
        EXPECT_EQ(size.has_value(), true);
        auto paintMethod = pattern->CreateNodePaintMethod();
        EXPECT_EQ(paintMethod == nullptr, false);
        frameNode->GetGeometryNode()->SetContentSize(size.value());
        auto paintWrapper = AceType::MakeRefPtr<PaintWrapper>(frameNode->GetRenderContext(),
            frameNode->GetGeometryNode()->Clone(), frameNode->GetPaintProperty<ShapePaintProperty>());
        EXPECT_EQ(paintWrapper == nullptr, false);
        auto contentDraw = paintMethod->GetContentDrawFunction(AceType::RawPtr(paintWrapper));
        EXPECT_EQ(contentDraw == nullptr, false);
        std::shared_ptr<SkCanvas> canvas = std::make_shared<SkCanvas>();
        Testing::MockCanvas rsCavas(&canvas);
        contentDraw(rsCavas);
    }

    void CheckCommands(bool hasValue)
    {
        auto pathModelNG = PathModelNG();
        pathModelNG.Create();
        auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
        EXPECT_EQ(frameNode == nullptr, false);
        if (hasValue) {
            pathModelNG.SetCommands(PATH_CMD);
        }
        auto shapeAbstactModel = ShapeAbstractModelNG();
        SetSize(shapeAbstactModel);
        ViewStackProcessor::GetInstance()->Pop();
        auto pathPaintProperty = frameNode->GetPaintProperty<PathPaintProperty>();
        if (hasValue) {
            EXPECT_EQ(pathPaintProperty->HasCommands(), true);
            EXPECT_STREQ(pathPaintProperty->GetCommandsValue().c_str(), PATH_CMD.c_str());
        } else {
            EXPECT_EQ(pathPaintProperty->HasCommands(), false);
        }
        Draw(frameNode);
    }
};

/**
 * @tc.name: COMMONDS001
 * @tc.desc: create path with cmd
 * @tc.type: FUNC
 */

HWTEST_F(PathPatternTestNg, COMMONDS001, TestSize.Level1)
{
    CheckCommands(true);
}

/**
 * @tc.name: COMMONDS002
 * @tc.desc: create path with no cmd
 * @tc.type: FUNC
 */

HWTEST_F(PathPatternTestNg, COMMONDS002, TestSize.Level1)
{
    CheckCommands(false);
}

/**
 * @tc.name: MeasureContent001
 * @tc.desc: PathLayoutAlgorithm::MeasureContent
 * @tc.type: FUNC
 */

HWTEST_F(PathPatternTestNg, MeasureContent001, TestSize.Level1)
{
    /**
     * @tc.steps1: initialize parameters.
     * @tc.expected: All pointer is non-null.
     */
    PathModelNG().Create();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_TRUE(frameNode);
    auto pattern = frameNode->GetPattern<PathPattern>();
    ASSERT_TRUE(pattern);
    auto layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_TRUE(layoutProperty);
    auto layoutAlgorithm = AceType::DynamicCast<PathLayoutAlgorithm>(pattern->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutConstraintF contentConstraint;
    auto layoutWrapper = LayoutWrapperNode(frameNode, geometryNode, layoutProperty);

    /**
     * @tc.steps2: contentConstraint.selfIdealSize is (100, 200)
     * @tc.expected: the return value of MeasureContent is (100, 200)
     */
    contentConstraint.selfIdealSize = OptionalSizeF(100, 200);
    auto size = layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(size.value(), SizeF(100, 200));
    contentConstraint.selfIdealSize.Reset();

    /**
     * @tc.steps3: Width is Zero and Height is Zero
     * @tc.expected: the return value of MeasureContent is (0, 0)
     */
    auto paintProperty = frameNode->GetPaintProperty<PathPaintProperty>();
    ASSERT_TRUE(paintProperty);
    paintProperty->UpdateCommands("M0 0 L0 0");
    size = layoutAlgorithm->MeasureContent(contentConstraint, &layoutWrapper);
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(size.value(), SizeF(0, 0));
}
} // namespace OHOS::Ace::NG
