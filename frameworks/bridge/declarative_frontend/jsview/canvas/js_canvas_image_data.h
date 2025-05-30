/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_CANVAS_IMAGE_DATA_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_CANVAS_IMAGE_DATA_H

#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/engine/bindings_defines.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"

namespace OHOS::Ace::Framework {

class JSCanvasImageData : public Referenced {
public:
    JSCanvasImageData() = default;
    ~JSCanvasImageData() override = default;

    static void JSBind(BindingTarget globalObj);
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSCanvasImageData* controller);

    void JsGetWidth(const JSCallbackInfo& info);
    void JsGetHeight(const JSCallbackInfo& info);
    void JsGetData(const JSCallbackInfo& info);
    void JsSetWidth(const JSCallbackInfo& info);
    void JsSetHeight(const JSCallbackInfo& info);
    void JsSetData(const JSCallbackInfo& info);
    bool GetImageDataSize(const JSCallbackInfo& args, int32_t& finalWidth, int32_t& finalHeight);

    int32_t width_ = 0;
    int32_t height_ = 0;
    JSRef<JSUint8ClampedArray> colorArray_;

    void setX(int32_t x)
    {
        x_ = x;
    }
    int32_t getX() const
    {
        return x_;
    }
    void setY(int32_t y)
    {
        y_ = y;
    }
    int32_t getY() const
    {
        return y_;
    }
    void setDirtyX(int32_t dirtyX)
    {
        dirtyX_ = dirtyX;
    }
    int32_t getDirtyX() const
    {
        return dirtyX_;
    }
    void setDirtyY(int32_t dirtyY)
    {
        dirtyY_ = dirtyY;
    }
    int32_t getDirtyY() const
    {
        return dirtyY_;
    }
    void setDirtyWidth(int32_t dirtyWidth)
    {
        dirtyWidth_ = dirtyWidth;
    }
    int32_t getDirtyWidth() const
    {
        return dirtyWidth_;
    }
    void setDirtyHeight(int32_t dirtyHeight)
    {
        dirtyHeight_ = dirtyHeight;
    }
    int32_t getDirtyHeight() const
    {
        return dirtyHeight_;
    }
    void setData(const std::vector<Color>& data)
    {
        data_ = data;
    }

    void SetUnit(CanvasUnit unit)
    {
        unit_ = unit;
    }

    CanvasUnit GetUnit()
    {
        return unit_;
    }

    double GetDensity()
    {
        double density = PipelineBase::GetCurrentDensity();
        return ((GetUnit() == CanvasUnit::DEFAULT) && !NearZero(density)) ? density : 1.0;
    }

private:
    int32_t x_ = 0;
    int32_t y_ = 0;
    int32_t dirtyX_ = 0;
    int32_t dirtyY_ = 0;
    int32_t dirtyWidth_ = 0;
    int32_t dirtyHeight_ = 0;
    std::vector<Color> data_;
    CanvasUnit unit_ = CanvasUnit::DEFAULT;
    ACE_DISALLOW_COPY_AND_MOVE(JSCanvasImageData);
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_CANVAS_IMAGE_DATA_H