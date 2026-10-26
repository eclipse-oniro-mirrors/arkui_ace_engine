/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_FOREACH_LAZY_FOREACH_COMPONENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_FOREACH_LAZY_FOREACH_COMPONENT_H

#include "base/memory/ace_type.h"
#include "base/utils/macros.h"
#include "core/components/foreach/for_each_component.h"
#include "core/components_v2/common/common_def.h"

namespace OHOS::Ace {
namespace V2 {
class DataChangeListener;
}

// A data-change capability shares its owner's lifetime.
// RefPtr and WeakPtr retain their original storage and public interfaces.
template<>
struct RefPtrTraits<V2::DataChangeListener> {
    static AceType* GetOwner(const V2::DataChangeListener* view);

    template<class T>
    class ListenerAccess final {
    public:
        explicit ListenerAccess(T* view) : guard_(GetOwner(view)), view_(view) {}
        T* operator->() const
        {
            return view_;
        }
        ListenerAccess(const ListenerAccess&) = delete;
        ListenerAccess& operator=(const ListenerAccess&) = delete;

    private:
        LifeCycleCheckable::PtrHolder<AceType> guard_;
        T* view_;
    };

    static ListenerAccess<V2::DataChangeListener> Access(V2::DataChangeListener* view)
    {
        return ListenerAccess<V2::DataChangeListener>(view);
    }

    template<class T>
    static T* DynamicCast(V2::DataChangeListener* view)
    {
        return TypeInfoHelper::DynamicCast<std::remove_const_t<T>>(view);
    }

    template<class T>
    static const T* DynamicCast(const V2::DataChangeListener* view)
    {
        return TypeInfoHelper::DynamicCast<std::remove_const_t<T>>(view);
    }
};

template<>
struct RefPtrTraits<const V2::DataChangeListener> : RefPtrTraits<V2::DataChangeListener> {
    static ListenerAccess<const V2::DataChangeListener> Access(const V2::DataChangeListener* view)
    {
        return ListenerAccess<const V2::DataChangeListener>(view);
    }
};
} // namespace OHOS::Ace

namespace OHOS::Ace::V2 {

typedef struct Operation {
    std::string type;
    int32_t count;
    int32_t index;
    std::pair<int32_t, int32_t> coupleIndex;
    std::string key;
    std::pair<std::string, std::string> coupleKey;
    std::list<std::string> keyList;
    bool reuseImmediately = false;
} Operation;

class DataChangeListener : public virtual TypeInfoBase {
    DECLARE_ACE_TYPE(DataChangeListener, TypeInfoBase);
public:
    ~DataChangeListener() override = default;

    virtual void OnDataReloaded(bool reuseImmediately = false) = 0;
    virtual void OnDataAdded(size_t index) = 0;
    virtual void OnDataBulkAdded(size_t index, size_t count) = 0;
    virtual void OnDataDeleted(size_t index) = 0;
    virtual void OnDataBulkDeleted(size_t index, size_t count) = 0;
    virtual void OnDataChanged(size_t index) = 0;
    // this is exchange impl.
    virtual void OnDataMoved(size_t from, size_t to) = 0;
    virtual void OnDatasetChange(const std::list<Operation>& DataOperations) = 0;
    virtual void OnDataBulkChanged(size_t index, size_t count) {}
    virtual void OnDataMoveToNewPlace(size_t from, size_t to) {}

private:
    friend AceType* RefPtrTraits<DataChangeListener>::GetOwner(const DataChangeListener* view);

    // Stable owner for the entire capability lifetime.
    virtual AceType* GetDataChangeOwner() const = 0;
};

class ACE_FORCE_EXPORT LazyForEachComponent : public V1::ForEachComponent {
    DECLARE_ACE_TYPE(V2::LazyForEachComponent, V1::ForEachComponent);

public:
    explicit LazyForEachComponent(const std::string& id) : V1::ForEachComponent(id, "LazyForEach") {}
    ~LazyForEachComponent() override = default;

    RefPtr<Element> CreateElement() override;

    size_t Count() override
    {
        return TotalCount();
    }

    size_t TotalCount();
    RefPtr<Component> GetChildByIndex(size_t index);

    virtual void ReleaseChildGroupByComposedId(const std::string& composedId) {}
    virtual void RegisterDataChangeListener(const RefPtr<DataChangeListener>& listener) = 0;
    virtual void UnregisterDataChangeListener(DataChangeListener* listener) = 0;

protected:
    virtual size_t OnGetTotalCount() = 0;
    virtual RefPtr<Component> OnGetChildByIndex(size_t index) = 0;

    std::list<RefPtr<Component>>& ExpandChildren() override;

    bool Expanded() const
    {
        return expanded_;
    }

private:
    bool expanded_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(LazyForEachComponent);
};

} // namespace OHOS::Ace::V2

namespace OHOS::Ace {
inline AceType* RefPtrTraits<V2::DataChangeListener>::GetOwner(const V2::DataChangeListener* view)
{
    return view ? view->GetDataChangeOwner() : nullptr;
}
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_FOREACH_LAZY_FOREACH_COMPONENT_H
