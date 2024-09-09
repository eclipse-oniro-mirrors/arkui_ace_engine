/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "arkoala_api_generated.h"

#include "core/components_ng/pattern/grid/grid_item_event_hub.h"
#include "core/components_ng/pattern/grid/grid_item_theme.h"
#include "core/interfaces/arkoala/generated/interface/node_api.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_STYLE_NAME = "style";
    const auto ATTRIBUTE_STYLE_DEFAULT_VALUE = "NONE";
    const auto ATTRIBUTE_ROW_START_NAME = "rowStart";
    const auto ATTRIBUTE_ROW_START_DEFAULT_VALUE = "0";
    const auto ATTRIBUTE_ROW_END_NAME = "rowEnd";
    const auto ATTRIBUTE_ROW_END_DEFAULT_VALUE = "0";
    const auto ATTRIBUTE_COLUMN_START_NAME = "columnStart";
    const auto ATTRIBUTE_COLUMN_START_DEFAULT_VALUE = "0";
    const auto ATTRIBUTE_COLUMN_END_NAME = "columnEnd";
    const auto ATTRIBUTE_COLUMN_END_DEFAULT_VALUE = "0";
    const auto ATTRIBUTE_FORCE_REBUILD_NAME = "forceRebuild";
    const auto ATTRIBUTE_FORCE_REBUILD_DEFAULT_VALUE = "false";
    const auto ATTRIBUTE_SELECTABLE_NAME = "selectable";
    const auto ATTRIBUTE_SELECTABLE_DEFAULT_VALUE = "true";
    const auto ATTRIBUTE_SELECTED_NAME = "selected";
    const auto ATTRIBUTE_SELECTED_DEFAULT_VALUE = "false";

    struct EventsTracker {
        static inline GENERATED_ArkUIGridItemEventsReceiver gridItemEventReceiver {};

        static inline const GENERATED_ArkUIEventsAPI eventsApiImpl = {
            .getGridItemEventsReceiver = [] () -> const GENERATED_ArkUIGridItemEventsReceiver* {
                return &gridItemEventReceiver;
            }
        };
    }; // EventsTracker
} // namespace

class GridItemModifierTest : public ModifierTestBase<GENERATED_ArkUIGridItemModifier,
    &GENERATED_ArkUINodeModifiers::getGridItemModifier, GENERATED_ARKUI_GRID_ITEM> {
public:
    static void SetUpTestCase()
    {
        GeneratedModifier::GetFullAPI()->setArkUIEventsAPI(&EventsTracker::eventsApiImpl);
    }
};

/*
 * @tc.name: setGridItemOptionsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setGridItemOptionsTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_STYLE_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_STYLE_DEFAULT_VALUE);
}

/*
 * @tc.name: setGridItemOptionsTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setGridItemOptionsTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_GridItemOptions options;
    Opt_GridItemOptions opt_options;

    // check PLAIN style
    options.style = Converter::ArkValue<Opt_GridItemStyle>(GridItemStyle::PLAIN);
    opt_options = Converter::ArkValue<Opt_GridItemOptions>(options);
    modifier_->setGridItemOptions(node_, &opt_options);
    strResult = GetStringAttribute(node_, ATTRIBUTE_STYLE_NAME);
    EXPECT_EQ(strResult, "PLAIN");

    // check NONE style
    options.style = Converter::ArkValue<Opt_GridItemStyle>(GridItemStyle::NONE);
    opt_options = Converter::ArkValue<Opt_GridItemOptions>(options);
    modifier_->setGridItemOptions(node_, &opt_options);
    strResult = GetStringAttribute(node_, ATTRIBUTE_STYLE_NAME);
    EXPECT_EQ(strResult, "NONE");
}

/*
 * @tc.name: setGridItemOptionsTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setGridItemOptionsTestInvalidValues, TestSize.Level1)
{
    std::string strResult;
    Opt_GridItemOptions inputValue = Converter::ArkValue<Opt_GridItemOptions>(Ark_Empty());

    // check that style is not changed
    modifier_->setGridItemOptions(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_STYLE_NAME);
    EXPECT_EQ(strResult, "NONE");
}

/*
 * @tc.name: setRowStartTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setRowStartTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_START_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_ROW_START_DEFAULT_VALUE);
}

/*
 * @tc.name: setRowStartTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setRowStartTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_Number inputValue;

    // check positive value
    inputValue = Converter::ArkValue<Ark_Number>(1);
    modifier_->setRowStart(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_START_NAME);
    EXPECT_EQ(strResult, "1");

    // check another positive value
    inputValue = Converter::ArkValue<Ark_Number>(5);
    modifier_->setRowStart(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_START_NAME);
    EXPECT_EQ(strResult, "5");

    // check negative value
    inputValue = Converter::ArkValue<Ark_Number>(-10);
    modifier_->setRowStart(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_START_NAME);
    EXPECT_EQ(strResult, "-10");
}

/*
 * @tc.name: setRowStartTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setRowStartTestInvalidValues, TestSize.Level1)
{
    std::string strResult;
    modifier_->setRowStart(node_, nullptr);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_START_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_ROW_START_DEFAULT_VALUE);
}

/*
 * @tc.name: setRowEndTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setRowEndTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_END_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_ROW_END_DEFAULT_VALUE);
}

/*
 * @tc.name: setRowEndTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setRowEndTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_Number inputValue;

    // check positive value
    inputValue = Converter::ArkValue<Ark_Number>(1);
    modifier_->setRowEnd(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_END_NAME);
    EXPECT_EQ(strResult, "1");

    // check another positive value
    inputValue = Converter::ArkValue<Ark_Number>(5);
    modifier_->setRowEnd(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_END_NAME);
    EXPECT_EQ(strResult, "5");

    // check negative value
    inputValue = Converter::ArkValue<Ark_Number>(-10);
    modifier_->setRowEnd(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_END_NAME);
    EXPECT_EQ(strResult, "-10");
}

/*
 * @tc.name: setRowEndTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setRowEndTestInvalidValues, TestSize.Level1)
{
    std::string strResult;
    modifier_->setRowEnd(node_, nullptr);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ROW_END_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_ROW_END_DEFAULT_VALUE);
}

/*
 * @tc.name: setColumnStartTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setColumnStartTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_START_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_COLUMN_START_DEFAULT_VALUE);
}

/*
 * @tc.name: setColumnStartTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setColumnStartTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_Number inputValue;

    // check positive value
    inputValue = Converter::ArkValue<Ark_Number>(1);
    modifier_->setColumnStart(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_START_NAME);
    EXPECT_EQ(strResult, "1");

    // check another positive value
    inputValue = Converter::ArkValue<Ark_Number>(5);
    modifier_->setColumnStart(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_START_NAME);
    EXPECT_EQ(strResult, "5");

    // check negative value
    inputValue = Converter::ArkValue<Ark_Number>(-10);
    modifier_->setColumnStart(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_START_NAME);
    EXPECT_EQ(strResult, "-10");
}

/*
 * @tc.name: setColumnStartTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setColumnStartTestInvalidValues, TestSize.Level1)
{
    std::string strResult;
    modifier_->setColumnStart(node_, nullptr);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_START_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_COLUMN_START_DEFAULT_VALUE);
}

/*
 * @tc.name: setColumnEndTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setColumnEndTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_END_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_COLUMN_END_DEFAULT_VALUE);
}

/*
 * @tc.name: setColumnEndTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setColumnEndTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_Number inputValue;

    // check positive value
    inputValue = Converter::ArkValue<Ark_Number>(1);
    modifier_->setColumnEnd(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_END_NAME);
    EXPECT_EQ(strResult, "1");

    // check another positive value
    inputValue = Converter::ArkValue<Ark_Number>(5);
    modifier_->setColumnEnd(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_END_NAME);
    EXPECT_EQ(strResult, "5");

    // check negative value
    inputValue = Converter::ArkValue<Ark_Number>(-10);
    modifier_->setColumnEnd(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_END_NAME);
    EXPECT_EQ(strResult, "-10");
}

/*
 * @tc.name: setColumnEndTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setColumnEndTestInvalidValues, TestSize.Level1)
{
    std::string strResult;
    modifier_->setColumnEnd(node_, nullptr);
    strResult = GetStringAttribute(node_, ATTRIBUTE_COLUMN_END_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_COLUMN_END_DEFAULT_VALUE);
}

/*
 * @tc.name: setForceRebuildTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setForceRebuildTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_FORCE_REBUILD_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_FORCE_REBUILD_DEFAULT_VALUE);
}

/*
 * @tc.name: setForceRebuildTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setForceRebuildTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_Boolean inputValue;

    // check true value
    inputValue = Converter::ArkValue<Ark_Boolean>(true);
    modifier_->setForceRebuild(node_, inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_FORCE_REBUILD_NAME);
    EXPECT_EQ(strResult, "true");

    // check false value
    inputValue = Converter::ArkValue<Ark_Boolean>(false);
    modifier_->setForceRebuild(node_, inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_FORCE_REBUILD_NAME);
    EXPECT_EQ(strResult, "false");
}

/*
 * @tc.name: setSelectableTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setSelectableTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_SELECTABLE_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_SELECTABLE_DEFAULT_VALUE);
}

/*
 * @tc.name: setSelectableTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setSelectableTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_Boolean inputValue;

    // check false value
    inputValue = Converter::ArkValue<Ark_Boolean>(false);
    modifier_->setSelectable(node_, inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_SELECTABLE_NAME);
    EXPECT_EQ(strResult, "false");

    // check true value
    inputValue = Converter::ArkValue<Ark_Boolean>(true);
    modifier_->setSelectable(node_, inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_SELECTABLE_NAME);
    EXPECT_EQ(strResult, "true");
}

/*
 * @tc.name: setSelectedTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setSelectedTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_SELECTED_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_SELECTED_DEFAULT_VALUE);
}

/*
 * @tc.name: setSelectedTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setSelectedTestValidValues, TestSize.Level1)
{
    std::string strResult;
    Ark_Boolean inputValue;

    // check true value
    inputValue = Converter::ArkValue<Ark_Boolean>(true);
    modifier_->setSelected(node_, inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_SELECTED_NAME);
    EXPECT_EQ(strResult, "true");

    // check false value
    inputValue = Converter::ArkValue<Ark_Boolean>(false);
    modifier_->setSelected(node_, inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_SELECTED_NAME);
    EXPECT_EQ(strResult, "false");
}

/*
 * @tc.name: setOnSelectTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridItemModifierTest, setOnSelectTest, TestSize.Level1)
{
    Ark_Function func = {};
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto eventHub = frameNode->GetEventHub<GridItemEventHub>();

    struct CheckEvent {
        int32_t nodeId;
        bool isSelected;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    EventsTracker::gridItemEventReceiver.onSelect = [](Ark_Int32 nodeId, const Ark_Boolean isSelected)
    {
        checkEvent = {
            .nodeId = nodeId,
            .isSelected = Converter::ArkValue<Ark_Boolean>(isSelected)
        };
    };

    modifier_->setOnSelect(node_, func);

    // check true value
    EXPECT_EQ(checkEvent.has_value(), false);
    eventHub->FireSelectChangeEvent(true);
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->isSelected, true);

    // check false value
    eventHub->FireSelectChangeEvent(false);
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, frameNode->GetId());
    EXPECT_EQ(checkEvent->isSelected, false);
}
} // namespace OHOS::Ace::NG
