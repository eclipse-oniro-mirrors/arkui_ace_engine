/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_DRAG_BAR_PAINT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_DRAG_BAR_PAINT_PROPERTY_H

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/utils/macros.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paint_property.h"

namespace OHOS::Ace::NG {
struct DragBarProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DragBarWidth, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DragBarHeight, Dimension);
};

class ACE_EXPORT SheetDragBarPaintProperty : public PaintProperty {
    DECLARE_ACE_TYPE(SheetDragBarPaintProperty, PaintProperty);

public:
    SheetDragBarPaintProperty() = default;
    ~SheetDragBarPaintProperty() override = default;

    RefPtr<PaintProperty> Clone() const override
    {
        auto paintProperty = MakeRefPtr<SheetDragBarPaintProperty>();
        paintProperty->PaintProperty::UpdatePaintProperty(AceType::DynamicCast<PaintProperty>(this));
        paintProperty->propBarLeftPoint_ = CloneBarLeftPoint();
        paintProperty->propBarCenterPoint_ = CloneBarCenterPoint();
        paintProperty->propBarRightPoint_ = CloneBarRightPoint();
        paintProperty->propDragOffset_ = CloneDragOffset();
        paintProperty->propOpacity_ = CloneOpacity();
        paintProperty->propBarWidth_ = CloneBarWidth();
        return paintProperty;
    }

    void Reset() override
    {
        PaintProperty::Reset();
        ResetBarLeftPoint();
        ResetBarCenterPoint();
        ResetBarRightPoint();
        ResetDragOffset();
        ResetOpacity();
        ResetBarWidth();
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(BarLeftPoint, OffsetT<Dimension>, PROPERTY_UPDATE_RENDER);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(BarCenterPoint, OffsetT<Dimension>, PROPERTY_UPDATE_RENDER);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(BarRightPoint, OffsetT<Dimension>, PROPERTY_UPDATE_RENDER);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(DragOffset, OffsetF, PROPERTY_UPDATE_RENDER);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Opacity, float, PROPERTY_UPDATE_RENDER);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(BarWidth, Dimension, PROPERTY_UPDATE_RENDER);

private:
    ACE_DISALLOW_COPY_AND_MOVE(SheetDragBarPaintProperty);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_DRAG_BAR_PAINT_PROPERTY_H
