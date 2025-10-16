/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "flex_testtwo_ng.h"

namespace OHOS::Ace::NG {
/**
 * @tc.name: FlexRowLayoutTest016
 * @tc.desc: Set former 3 texts with 40% of container size in Row direction, container has no padding, the latter 3
 * texts with 20% width, and 4,5 has 1 layoutweight, the 6 has 2 displayPriority and 1 layoutweight.
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexRowLayoutTest016, TestSize.Level0)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(rowFrameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    ASSERT_NE(rowLayoutPattern, nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(rowLayoutProperty, nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW_REVERSE);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    ASSERT_NE(rowLayoutAlgorithm, nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);

    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.RowReverse, alignItems: ItemAlign.Center}) {
        Text('1').width('40%').height(40).backgroundColor(0xFFFFFF)
        Text('2').width('40%').height(40).backgroundColor(0xEEEEEE)
        Text('3').width('40%').height(40).backgroundColor(0xDDDDDD)
        Text('4').width('20%').height(40).backgroundColor(0xCCCCCC).layoutWeight(1)
        Text('5').width('20%').height(40).backgroundColor(0xBBBBBB).layoutWeight(1)
        Text('6').width('20%').height(40).backgroundColor(0xAAAAAA).displayPriority(2).layoutWeight(1)
      }
      .height(80)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    int32_t index = 1;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapperNode> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapperNode>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(2 * TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        ASSERT_NE(boxLayoutAlgorithm, nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE - 1; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapperNode> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapperNode>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        ASSERT_NE(boxLayoutAlgorithm, nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    auto sixthFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> sixthGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    sixthGeometryNode->Reset();
    RefPtr<LayoutWrapperNode> sixthLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(sixthFrameNode, sixthGeometryNode, sixthFrameNode->GetLayoutProperty());
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    sixthLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
    auto boxLayoutAlgorithm = sixthFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(boxLayoutAlgorithm, nullptr);
    sixthLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(sixthFrameNode);
    layoutWrapper->AppendChild(sixthLayoutWrapper);

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    index = 0;
    for (int32_t i = START_INDEX; i < 2 * THREE_ITEM_SIZE - 1; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(0.0f, 0.0f));
        EXPECT_EQ(childOffset, OffsetF(0.0f, 0.0f));
        index++;
    }
    auto verticalRemaning = ROW_HEIGHT - SMALL_ITEM_HEIGHT;
    auto sixthChildWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    auto sixthChildSize = sixthChildWrapper->GetGeometryNode()->GetFrameSize();
    auto sixthChildOffset = sixthChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(sixthChildSize, SizeF(RK356_WIDTH / THREE_ITEM_SIZE, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(sixthChildOffset, OffsetF(RK356_WIDTH - RK356_WIDTH / 3, verticalRemaning / 2));
}

/**
 * @tc.name: FlexRowLayoutTest017
 * @tc.desc: Set three texts with different flexBasis, flexGrows, flexShrink.
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexRowLayoutTest017, TestSize.Level0)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start}) {
            Text('1').height(40).backgroundColor(0xDDDDDD).flexBasis(96)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF).flexGrow(2)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3).flexShrink(1)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    auto firstItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstItemGeometryNode->Reset();
    RefPtr<LayoutWrapperNode> firstItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        firstItemFrameNode, firstItemGeometryNode, firstItemFrameNode->GetLayoutProperty());
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(), CalcLength(SMALL_ITEM_HEIGHT)));
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateFlexBasis(Dimension(TWENTY_PERCENT_WIDTH));
    firstItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto boxLayoutAlgorithm = firstItemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(firstItemFrameNode);
    layoutWrapper->AppendChild(firstItemLayoutWrapper);

    auto secondItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> secondItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    secondItemGeometryNode->Reset();
    RefPtr<LayoutWrapperNode> secondItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        secondItemFrameNode, secondItemGeometryNode, secondItemFrameNode->GetLayoutProperty());
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateFlexGrow(2);
    secondItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    secondItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(secondItemFrameNode);
    layoutWrapper->AppendChild(secondItemLayoutWrapper);

    auto thirdItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 3, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> thirdItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    thirdItemGeometryNode->Reset();
    RefPtr<LayoutWrapperNode> thirdItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        thirdItemFrameNode, thirdItemGeometryNode, thirdItemFrameNode->GetLayoutProperty());
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateFlexShrink(1);
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    thirdItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(thirdItemFrameNode);
    layoutWrapper->AppendChild(thirdItemLayoutWrapper);

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OFFSET_TOP_LEFT);
    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(SIXTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(TWENTY_PERCENT_WIDTH, 0.0f));
    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(SIXTY_PERCENT_WIDTH + TWENTY_PERCENT_WIDTH, 0.0f));
}

/**
 * @tc.name: FlexLayoutTest001
 * @tc.desc: Set Flex Row Layout Test direction_ is HORIZONTAL
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest001, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);

    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::HORIZONTAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);

    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->GetMainAxisRemainSpace(LAYOUT_WEIGHT_ONE);
}

/**
 * @tc.name: FlexLayoutTest002
 * @tc.desc: Set Flex Row Layout Test direction_ is VERTICAL
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest002, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);

    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);

    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->GetCrossAxisRemainSpace(LAYOUT_WEIGHT_ONE);
}

/**
 * @tc.name: FlexLayoutTest003
 * @tc.desc: Set Flex Row Layout Test direction_ is VERTICAL
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest003, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);

    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);

    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->GetMainAxisOffset(OFFSET_TOP_MIDDLE);
}

/**
 * @tc.name: FlexLayoutTest004
 * @tc.desc: Set Flex Row Layout Test direction_ is VERTICAL
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest004, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);

    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);

    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->GetCrossAxisOffset(OFFSET_TOP_MIDDLE);
}

/**
 * @tc.name: FlexLayoutTest005
 * @tc.desc: Set Flex Row Layout Test direction_ is VERTICAL
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest005, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);

    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);

    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->GetItemMainOffset(LAYOUT_WEIGHT_ONE);
}

/**
 * @tc.name: FlexLayoutTest006
 * @tc.desc: Test layout of different flex wrap settings.
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest006, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL_REVERSE);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::SPACE_AROUND);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::SPACE_AROUND);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::STRETCH);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Column, wrap: FlexWrap.WrapReverse, justifyContent: FlexAlign.SpaceAround,
       alignItems: ItemAlign.Stretch, alignContent: FlexAlign.SpaceAround }) {
            Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapperNode> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapperNode>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    wrapLayoutAlgorithm->GetMainAxisRemainSpace(LAYOUT_WEIGHT_ONE);
    wrapLayoutAlgorithm->GetCrossAxisRemainSpace(LAYOUT_WEIGHT_ONE);
    wrapLayoutAlgorithm->GetMainAxisOffset(OFFSET_TOP_MIDDLE);
    wrapLayoutAlgorithm->GetCrossAxisOffset(OFFSET_TOP_MIDDLE);
    wrapLayoutAlgorithm->GetItemMainOffset(LAYOUT_WEIGHT_ONE);
}

/**
 * @tc.name: FlexLayoutTest007
 * @tc.desc: Set Flex Row Layout Test WrapAlignment is change
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest007, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);

    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);

    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    OffsetF startPosition = OffsetF(ZERO, ZERO);
    OffsetF spaceBetweenContentsOnCrossAxis = OffsetF(RK356_WIDTH / 2.0f, ZERO);
    wrapLayoutAlgorithm->UpdateStartPositionByAlign(startPosition, 2.0f, spaceBetweenContentsOnCrossAxis, 2);
}

/**
 * @tc.name: FlexLayoutTest008
 * @tc.desc: Set Flex Row Layout Test WrapAlignment is change
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexLayoutTest008, TestSize.Level0)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::HORIZONTAL_REVERSE);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::BASELINE);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::BASELINE);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::BASELINE);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.WrapReverse, justifyContent: FlexAlign.Center, alignItems:
       ItemAlign.Center, alignContent: FlexAlign.Center }) { Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapperNode> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapperNode>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);
    OffsetF startPosition = OffsetF(ZERO, ZERO);
    OffsetF spaceBetweenContentsOnCrossAxis = OffsetF(RK356_WIDTH / 2.0f, ZERO);
    wrapLayoutAlgorithm->UpdateStartPositionByAlign(startPosition, 2.0f, spaceBetweenContentsOnCrossAxis, 2);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::END);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::END);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::END);
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::SPACE_AROUND);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::SPACE_AROUND);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::SPACE_AROUND);
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
}

/**
 * @tc.name: FlexDirectionWrapTest001
 * @tc.desc: Test layout of different flex which direction is RTL and FlexWrap is Wrap.
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexDirectionWrapTest001, TestSize.Level0)
{
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Start, alignItems:
       ItemAlign.Start, alignContent: FlexAlign.Start }) { Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
        .direction(Direction.Rtl)
    */
    RefPtr<LayoutWrapperNode> layoutWrapper = CreateRTLWrapper();

    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OffsetF(HALF_PERCENT_WIDTH, 0.0f));

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(0.0f, 0.0f));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
}

/**
 * @tc.name: FlexDirectionWrapTest002
 * @tc.desc: Test layout of different flex which direction is RTL and FlexWrap is WrapReverse.
 * @tc.type: FUNC
 */
HWTEST_F(FlexTestTwoNg, FlexDirectionWrapTest002, TestSize.Level0)
{
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.WrapReverse, justifyContent: FlexAlign.Center, alignItems:
       ItemAlign.Center, alignContent: FlexAlign.Center }) { Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
        .direction(Direction.Rtl)
    */
    RefPtr<LayoutWrapperNode> layoutWrapper = CreateRTLReverseWrapper();
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto verticalRemaining = COLUMN_HEIGHT - 2 * SMALL_ITEM_HEIGHT;
    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OffsetF(0.0f, verticalRemaining / 2));

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(HALF_PERCENT_WIDTH, verticalRemaining / 2));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(HALF_PERCENT_WIDTH / 2, verticalRemaining / 2 + SMALL_ITEM_HEIGHT));
}
} // namespace OHOS::Ace::NG