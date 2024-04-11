/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "core/interfaces/native/node/image_animator_modifier.h"

#include "base/geometry/calc_dimension.h"
#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/declaration/image/image_animator_declaration.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/image_animator/image_animator_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
constexpr int32_t DEFAULT_DURATION = 1000; // ms
constexpr int32_t DEFAULT_ITERATION = 1;
constexpr FillMode DEFAULT_FILL_MODE = FillMode::FORWARDS;
constexpr uint32_t DEFAULT_ITERATIONS = 1;
constexpr int32_t IMAGES_LENGTH = 4;

namespace ImageAnimatorModifier {
void ParseImage(CalcDimension* dimension, int32_t dimensionLength, const ArkUIImagePropertiesStruct* image)
{
    for (int32_t i = 0; i < dimensionLength; i++) {
        if (image->unit[i] == static_cast<int8_t>(DimensionUnit::CALC)) {
            dimension[i].SetUnit(DimensionUnit::CALC);
            dimension[i].SetValue(0.0);
            dimension[i].SetCalcValue(std::string(image->calc[i]));
        } else {
            dimension[i].SetUnit(static_cast<DimensionUnit>(image->unit[i]));
            dimension[i].SetValue(image->number[i]);
        }
    }
}

void SetState(ArkUINodeHandle node, int32_t state)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    int32_t value = state;
    if (value < static_cast<int32_t>(Animator::Status::IDLE) ||
        value > static_cast<int32_t>(Animator::Status::STOPPED)) {
        value = static_cast<int32_t>(Animator::Status::IDLE);
    }

    ImageAnimatorModelNG::SetState(frameNode, value);
}

void ResetState(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageAnimatorModelNG::SetState(frameNode, static_cast<int32_t>(Animator::Status::IDLE));
}

void SetDuration(ArkUINodeHandle node, int32_t duration)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (duration < 0) {
        duration = DEFAULT_DURATION;
    }
    ImageAnimatorModelNG::SetDuration(frameNode, duration);
}

void SetFixedSize(ArkUINodeHandle node, uint32_t fixedSize)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    ImageAnimatorModelNG::SetFixedSize(frameNode, static_cast<bool>(fixedSize));
}

void ResetFixedSize(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    ImageAnimatorModelNG::SetFixedSize(frameNode, true);
}

void SetFillMode(ArkUINodeHandle node, int32_t fillMode)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    int32_t value = fillMode;
    if (value < static_cast<int32_t>(FillMode::NONE) || value > static_cast<int32_t>(FillMode::BOTH)) {
        value = static_cast<int32_t>(DEFAULT_FILL_MODE);
    }

    ImageAnimatorModelNG::SetFillMode(frameNode, value);
}

void ResetFillMode(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageAnimatorModelNG::SetFillMode(frameNode, static_cast<int32_t>(DEFAULT_FILL_MODE));
}

void SetReverse(ArkUINodeHandle node, uint32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageAnimatorModelNG::SetIsReverse(frameNode, static_cast<bool>(value));
}

void ResetReverse(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageAnimatorModelNG::SetIsReverse(frameNode, false);
}

void SetImages(ArkUINodeHandle node, struct ArkUIImagePropertiesStruct* images, int32_t length)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (length == 0) {
        ImageAnimatorModelNG::SetImages(frameNode, std::vector<ImageProperties>());
        return;
    }
    CHECK_NULL_VOID(images);
    if (length <= 0) {
        return;
    }
    std::vector<ImageProperties> imageList;
    for (int32_t i = 0; i < length; i++) {
        ArkUIImagePropertiesStruct* image = images + i;
        CHECK_NULL_VOID(image);
        CalcDimension dimension[IMAGES_LENGTH];
        ParseImage(dimension, IMAGES_LENGTH, image);
        ImageProperties property { std::string(image->src), nullptr, "", "", dimension[0], dimension[1], dimension[2],
            dimension[3], image->duration };
        imageList.emplace_back(property);
    }
    ImageAnimatorModelNG::SetImages(frameNode, imageList);
}

void ResetImages(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageAnimatorModelNG::SetImages(frameNode, std::vector<ImageProperties>());
}

void SetImageAnimatorIteration(ArkUINodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    int32_t iteration = value;
    if (iteration < -1) {
        iteration = DEFAULT_ITERATIONS;
    }

    ImageAnimatorModelNG::SetIteration(frameNode, iteration);
}

void ResetImageAnimatorIteration(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    ImageAnimatorModelNG::SetIteration(frameNode, DEFAULT_ITERATION);
}
} // namespace ImageAnimatorModifier

namespace NodeModifier {
const ArkUIImageAnimatorModifier* GetImageAnimatorModifier()
{
    static const ArkUIImageAnimatorModifier modifier = {
        ImageAnimatorModifier::SetState,
        ImageAnimatorModifier::ResetState,
        ImageAnimatorModifier::SetDuration,
        ImageAnimatorModifier::SetFixedSize,
        ImageAnimatorModifier::ResetFixedSize,
        ImageAnimatorModifier::SetFillMode,
        ImageAnimatorModifier::ResetFillMode,
        ImageAnimatorModifier::SetReverse,
        ImageAnimatorModifier::ResetReverse,
        ImageAnimatorModifier::SetImages,
        ImageAnimatorModifier::ResetImages,
        ImageAnimatorModifier::SetImageAnimatorIteration,
        ImageAnimatorModifier::ResetImageAnimatorIteration
    };

    return &modifier;
}
}
} // namespace OHOS::Ace::NG