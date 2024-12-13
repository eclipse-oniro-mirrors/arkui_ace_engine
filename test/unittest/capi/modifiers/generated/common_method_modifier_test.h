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

#ifndef GENERATED_FOUNDATION_ACE_CAPI_TEST_COMMON_METHOD_H
#define GENERATED_FOUNDATION_ACE_CAPI_TEST_COMMON_METHOD_H
#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "test_fixtures.h"
#include "type_helpers.h"

#include "core/interfaces/native/utility/reverse_converter.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;
using namespace Converter;
using namespace TypeHelper;
namespace TestConst::CommonMethod {
const auto ATTRIBUTE_MOTION_BLUR_NAME = "motionBlur";
const auto ATTRIBUTE_MOTION_BLUR_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ROTATE_NAME = "rotate";
const auto ATTRIBUTE_MARK_ANCHOR_NAME = "markAnchor";
const auto ATTRIBUTE_USE_SIZE_TYPE_NAME = "useSizeType";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_XS_NAME = "xs";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_SM_NAME = "sm";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_MD_NAME = "md";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_LG_NAME = "lg";
const auto ATTRIBUTE_ALIGN_RULES_NAME = "alignRules";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_NAME = "left";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_NAME = "right";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_NAME = "middle";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_NAME = "top";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_NAME = "bottom";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_NAME = "center";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_NAME = "bias";
const auto ATTRIBUTE_ALIGN_RULES_I_START_NAME = "start";
const auto ATTRIBUTE_ALIGN_RULES_I_END_NAME = "end";
const auto ATTRIBUTE_DRAG_PREVIEW_NAME = "dragPreview";
const auto ATTRIBUTE_TAB_INDEX_NAME = "tabIndex";
const auto ATTRIBUTE_TAB_INDEX_DEFAULT_VALUE = "0";
const auto ATTRIBUTE_DEFAULT_FOCUS_NAME = "defaultFocus";
const auto ATTRIBUTE_DEFAULT_FOCUS_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_GROUP_DEFAULT_FOCUS_NAME = "groupDefaultFocus";
const auto ATTRIBUTE_GROUP_DEFAULT_FOCUS_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_FOCUS_ON_TOUCH_NAME = "focusOnTouch";
const auto ATTRIBUTE_FOCUS_ON_TOUCH_DEFAULT_VALUE = "false";
const auto ATTRIBUTE_MOTION_BLUR_I_RADIUS_NAME = "radius";
const auto ATTRIBUTE_MOTION_BLUR_I_RADIUS_DEFAULT_VALUE = "";
const auto ATTRIBUTE_MOTION_BLUR_I_ANCHOR_I_X_NAME = "x";
const auto ATTRIBUTE_MOTION_BLUR_I_ANCHOR_I_X_DEFAULT_VALUE = "";
const auto ATTRIBUTE_MOTION_BLUR_I_ANCHOR_I_Y_NAME = "y";
const auto ATTRIBUTE_MOTION_BLUR_I_ANCHOR_I_Y_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ROTATE_I_X_NAME = "x";
const auto ATTRIBUTE_ROTATE_I_X_DEFAULT_VALUE = "0.000000";
const auto ATTRIBUTE_ROTATE_I_Y_NAME = "y";
const auto ATTRIBUTE_ROTATE_I_Y_DEFAULT_VALUE = "0.000000";
const auto ATTRIBUTE_ROTATE_I_Z_NAME = "z";
const auto ATTRIBUTE_ROTATE_I_Z_DEFAULT_VALUE = "1.000000";
const auto ATTRIBUTE_ROTATE_I_CENTER_X_NAME = "centerX";
const auto ATTRIBUTE_ROTATE_I_CENTER_X_DEFAULT_VALUE = "5000.00%";
const auto ATTRIBUTE_ROTATE_I_CENTER_Y_NAME = "centerY";
const auto ATTRIBUTE_ROTATE_I_CENTER_Y_DEFAULT_VALUE = "5000.00%";
const auto ATTRIBUTE_ROTATE_I_CENTER_Z_NAME = "centerZ";
const auto ATTRIBUTE_ROTATE_I_CENTER_Z_DEFAULT_VALUE = "0.00px";
const auto ATTRIBUTE_ROTATE_I_PERSPECTIVE_NAME = "perspective";
const auto ATTRIBUTE_ROTATE_I_PERSPECTIVE_DEFAULT_VALUE = "0.000000";
const auto ATTRIBUTE_ROTATE_I_ANGLE_NAME = "angle";
const auto ATTRIBUTE_ROTATE_I_ANGLE_DEFAULT_VALUE = "0.000000";
const auto ATTRIBUTE_FLEX_GROW_NAME = "flexGrow";
const auto ATTRIBUTE_FLEX_GROW_DEFAULT_VALUE = "0";
const auto ATTRIBUTE_FLEX_SHRINK_NAME = "flexShrink";
const auto ATTRIBUTE_FLEX_SHRINK_DEFAULT_VALUE = "1";
const auto ATTRIBUTE_FLEX_BASIS_NAME = "flexBasis";
const auto ATTRIBUTE_FLEX_BASIS_DEFAULT_VALUE = "auto";
const auto ATTRIBUTE_MARK_ANCHOR_I_X_NAME = "x";
const auto ATTRIBUTE_MARK_ANCHOR_I_X_DEFAULT_VALUE = "";
const auto ATTRIBUTE_MARK_ANCHOR_I_Y_NAME = "y";
const auto ATTRIBUTE_MARK_ANCHOR_I_Y_DEFAULT_VALUE = "";
const auto ATTRIBUTE_MARK_ANCHOR_I_START_NAME = "start";
const auto ATTRIBUTE_MARK_ANCHOR_I_START_DEFAULT_VALUE = "";
const auto ATTRIBUTE_MARK_ANCHOR_I_TOP_NAME = "top";
const auto ATTRIBUTE_MARK_ANCHOR_I_TOP_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_XS_I_SPAN_NAME = "span";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_XS_I_SPAN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_XS_I_OFFSET_NAME = "offset";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_XS_I_OFFSET_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_SM_I_SPAN_NAME = "span";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_SM_I_SPAN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_SM_I_OFFSET_NAME = "offset";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_SM_I_OFFSET_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_MD_I_SPAN_NAME = "span";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_MD_I_SPAN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_MD_I_OFFSET_NAME = "offset";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_MD_I_OFFSET_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_LG_I_SPAN_NAME = "span";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_LG_I_SPAN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_LG_I_OFFSET_NAME = "offset";
const auto ATTRIBUTE_USE_SIZE_TYPE_I_LG_I_OFFSET_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_LEFT_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_RIGHT_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_MIDDLE_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_TOP_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_BOTTOM_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_CENTER_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_HORIZONTAL_NAME = "horizontal";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_HORIZONTAL_DEFAULT_VALUE = "0.5";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_VERTICAL_NAME = "vertical";
const auto ATTRIBUTE_ALIGN_RULES_I_BIAS_I_VERTICAL_DEFAULT_VALUE = "0.5";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_START_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ANCHOR_NAME = "anchor";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ANCHOR_DEFAULT_VALUE = "";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ALIGN_NAME = "align";
const auto ATTRIBUTE_ALIGN_RULES_I_END_I_ALIGN_DEFAULT_VALUE = "";
const auto ATTRIBUTE_DRAG_PREVIEW_I_EXTRA_INFO_NAME = "extraInfo";
const auto ATTRIBUTE_DRAG_PREVIEW_I_EXTRA_INFO_DEFAULT_VALUE = "!NOT-DEFINED!";
const auto ATTRIBUTE_RENDER_FIT_NAME = "renderFit";
const auto ATTRIBUTE_RENDER_FIT_DEFAULT_VALUE = "RenderFit.TOP_LEFT";
} // namespace TestConst::CommonMethod

class CommonMethodModifierTest
    : public ModifierTestBase<GENERATED_ArkUICommonMethodModifier,
          &GENERATED_ArkUINodeModifiers::getCommonMethodModifier, GENERATED_ARKUI_COMMON_METHOD>,
      public testing::WithParamInterface<GENERATED_Ark_NodeType> {
public:
    virtual Ark_NodeHandle CreateNode(GENERATED_Ark_NodeType)
    {
        return ModifierTestBase::CreateNode(GetParam());
    }

    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        for (auto& [id, strid, res] : Fixtures::resourceInitTable) {
            AddResource(id, res);
            AddResource(strid, res);
        }
    }
};

} // namespace OHOS::Ace::NG

#endif // GENERATED_FOUNDATION_ACE_CAPI_TEST_COMMON_METHOD_H
