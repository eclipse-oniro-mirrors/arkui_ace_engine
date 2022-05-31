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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_PROPERTY_H

#include <cstdint>
#include <optional>

#include "base/memory/ace_type.h"
#include "base/utils/macros.h"

namespace OHOS::Ace::NG {

using PropertyChangeFlag = uint32_t;

inline constexpr PropertyChangeFlag PROPERTY_UPDATE_NORMAL = 0;
// Affects self size.
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_MEASURE = 1;
// Affects the child position.
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_LAYOUT = 1 << 1;
// Affects self position.
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_POSITION = 1 << 2;
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_NODE_TREE = 1 << 3;
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_CHILD_REQUEST = 1 << 4;
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_RENDER = 1 << 5;
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_RENDER_PROPERTY = 1 << 6;
inline constexpr PropertyChangeFlag PROPERTY_UPDATE_EVENT = 1 << 7;

bool CheckNeedRender(PropertyChangeFlag propertyChangeFlag);

bool CheckNeedLayoutSelf(PropertyChangeFlag propertyChangeFlag);

bool CheckNeedParentLayout(PropertyChangeFlag propertyChangeFlag);

bool CheckMeasureFlag(PropertyChangeFlag propertyChangeFlag);

bool CheckLayoutFlag(PropertyChangeFlag propertyChangeFlag);

bool CheckPositionFlag(PropertyChangeFlag propertyChangeFlag);

bool CheckTreeChangedFlag(PropertyChangeFlag propertyChangeFlag);

bool CheckNoChanged(PropertyChangeFlag propertyChangeFlag);

bool CheckNodeTreeFlag(PropertyChangeFlag propertyChangeFlag);

#define ACE_DEFINE_CLASS_PROPERTY_GROUP(group, type)    \
public:                                                 \
    const std::unique_ptr<type>& GetOrCreate##group()   \
    {                                                   \
        if (!prop##group##_) {                          \
            prop##group##_ = std::make_unique<type>();  \
        }                                               \
        return prop##group##_;                          \
    }                                                   \
    const std::unique_ptr<type>& Get##group() const     \
    {                                                   \
        return prop##group##_;                          \
    }                                                   \
    std::unique_ptr<type> Clone##group() const          \
    {                                                   \
        return std::make_unique<type>(*prop##group##_); \
    }                                                   \
    void Reset##group()                                 \
    {                                                   \
        return prop##group##_.reset();                  \
    }                                                   \
                                                        \
private:                                                \
    std::unique_ptr<type> prop##group##_;

#define ACE_DEFINE_CLASS_PROPERTY(group, name, type, changeFlag) \
public:                                                          \
    void Update##name(const type& value)                         \
    {                                                            \
        auto& groupProperty = GetOrCreate##group();              \
        if (groupProperty->Check##name(value)) {                 \
            LOGD("the %{public}s is same, just ignore", #name);  \
            return;                                              \
        }                                                        \
        groupProperty->Update##name(value);                      \
        UpdatePropertyChangeFlag(changeFlag);                    \
    }

#define ACE_DEFINE_STRUCT_PROPERTY(name, type)                \
    std::optional<type> prop##name;                           \
                                                              \
    const std::optional<type>& Get##name() const              \
    {                                                         \
        return prop##name;                                    \
    }                                                         \
    bool Update##name(const type& value##name)                \
    {                                                         \
        if (prop##name == value##name) {                      \
            return false;                                     \
        }                                                     \
        prop##name = value##name;                             \
        return true;                                          \
    }                                                         \
    bool Update##name(const std::optional<type>& value##name) \
    {                                                         \
        if (prop##name == value##name) {                      \
            return false;                                     \
        }                                                     \
        if (!value##name.has_value()) {                       \
            return false;                                     \
        }                                                     \
        prop##name = value##name;                             \
        return true;                                          \
    }                                                         \
    bool Has##name() const                                    \
    {                                                         \
        return prop##name.has_value();                        \
    }                                                         \
    const type& Get##name##Value() const                      \
    {                                                         \
        return prop##name.value();                            \
    }                                                         \
    bool Check##name(const type& value) const                 \
    {                                                         \
        return prop##name == value;                           \
    }

class ACE_EXPORT Property : public virtual AceType {
    DECLARE_ACE_TYPE(Property, AceType);

public:
    Property() = default;

    ~Property() override = default;

    uint32_t GetPropertyChangeFlag() const
    {
        return propertyChangeFlag_;
    }

    void CleanDirty()
    {
        propertyChangeFlag_ = PROPERTY_UPDATE_NORMAL;
    }

    void UpdatePropertyChangeFlag(PropertyChangeFlag propertyChangeFlag)
    {
        propertyChangeFlag_ = propertyChangeFlag_ | propertyChangeFlag;
    }

    virtual void AjdustPropertyChangeFlagByChild(PropertyChangeFlag propertyChangeFlag) {}

protected:
    PropertyChangeFlag propertyChangeFlag_ = PROPERTY_UPDATE_NORMAL;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_PROPERTY_H