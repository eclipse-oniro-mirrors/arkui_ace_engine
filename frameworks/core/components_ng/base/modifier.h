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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MODIFIER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MODIFIER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/modifier_adapter.h"

namespace OHOS::Ace::NG {

class Modifier : public virtual AceType {
    DECLARE_ACE_TYPE(Modifier, AceType);

public:
    Modifier()
    {
        static std::atomic<int32_t> genId = 0;
        id_ = genId.fetch_add(1, std::memory_order_relaxed);
    }
    ~Modifier() override
    {
        ModifierAdapter::RemoveModifier(id_);
    }

    int32_t GetId() const
    {
        return id_;
    }

private:
    int32_t id_ = 0;
    ACE_DISALLOW_COPY_AND_MOVE(Modifier);
};

class AnimatablePropertyBase : public virtual AceType {
    DECLARE_ACE_TYPE(AnimatablePropertyBase, AceType);

public:
    AnimatablePropertyBase() = default;
    ~AnimatablePropertyBase() override = default;

private:
    ACE_DISALLOW_COPY_AND_MOVE(AnimatablePropertyBase);
};

struct DrawingContext {
    RSCanvas& canvas;
    float width = 0;
    float height = 0;
};

template<typename T>
class AnimatableProperty : public AnimatablePropertyBase {
    DECLARE_ACE_TYPE(AnimatableProperty, AnimatablePropertyBase);

public:
    AnimatableProperty(const T& value) : value_(value) {}
    ~AnimatableProperty() override = default;

    void SetUpCallbacks(std::function<T()>&& getFunc, std::function<void(const T&)>&& setFunc)
    {
        getFunc_ = getFunc;
        setFunc_ = setFunc;
    }

    T Get()
    {
        if (getFunc_) {
            return getFunc_();
        } else {
            return value_;
        }
    }

    void Set(const T& value)
    {
        if (setFunc_) {
            setFunc_(value);
        } else {
            value_ = value;
        }
    }

private:
    T value_;
    std::function<T()> getFunc_;
    std::function<void(const T&)> setFunc_;
    ACE_DISALLOW_COPY_AND_MOVE(AnimatableProperty);
};

class ContentModifier : public Modifier {
    DECLARE_ACE_TYPE(ContentModifier, Modifier);

public:
    ContentModifier() = default;
    ~ContentModifier() override = default;
    virtual void onDraw(DrawingContext& Context) = 0;

    void AttachProperty(const RefPtr<AnimatablePropertyBase>& prop)
    {
        attachedProperties_.push_back(prop);
    }

    const std::vector<RefPtr<AnimatablePropertyBase>>& GetAttachedProperties()
    {
        return attachedProperties_;
    }

private:
    std::vector<RefPtr<AnimatablePropertyBase>> attachedProperties_;
    ACE_DISALLOW_COPY_AND_MOVE(ContentModifier);
};

class OverlayModifier : public Modifier {
    DECLARE_ACE_TYPE(OverlayModifier, Modifier);

public:
    OverlayModifier() = default;
    ~OverlayModifier() override = default;
    virtual void onDraw(DrawingContext& Context) = 0;

    void AttachProperty(const RefPtr<AnimatablePropertyBase>& prop)
    {
        attachedProperties_.push_back(prop);
    }

    const std::vector<RefPtr<AnimatablePropertyBase>>& GetAttachedProperties()
    {
        return attachedProperties_;
    }

    const RectF& GetBoundsRect()
    {
        return rect_;
    }

    void SetBoundsRect(const RectF& rect)
    {
        rect_ = rect;
    }

private:
    std::vector<RefPtr<AnimatablePropertyBase>> attachedProperties_;
    RectF rect_;
    ACE_DISALLOW_COPY_AND_MOVE(OverlayModifier);
};

#define DECLARE_PROP_TYPED_CLASS(classname, template_class, type) \
    class classname : public template_class<type> {               \
        DECLARE_ACE_TYPE(classname, template_class);              \
                                                                  \
    public:                                                       \
        explicit classname(type value) : template_class(value) {} \
        ~classname() override = default;                          \
        ACE_DISALLOW_COPY_AND_MOVE(classname);                    \
    };

DECLARE_PROP_TYPED_CLASS(AnimatablePropertyBool, AnimatableProperty, bool);
DECLARE_PROP_TYPED_CLASS(AnimatablePropertyFloat, AnimatableProperty, float);
DECLARE_PROP_TYPED_CLASS(AnimatablePropertyColor, AnimatableProperty, LinearColor);
DECLARE_PROP_TYPED_CLASS(AnimatablePropertySizeF, AnimatableProperty, SizeF);
DECLARE_PROP_TYPED_CLASS(AnimatablePropertyOffsetF, AnimatableProperty, OffsetF);

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MODIFIER_H
