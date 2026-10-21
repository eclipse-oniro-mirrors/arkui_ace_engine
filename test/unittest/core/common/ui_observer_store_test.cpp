/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <functional>

#include "gtest/gtest.h"

#include "core/common/ui_observer/render_layout_store.h"
#include "core/common/ui_observer/ui_observer_store.h"

using namespace testing::ext;

namespace OHOS::Ace {
class UIObserverStoreTest : public testing::Test {};

namespace {
class TestRenderLayoutListener final : public RenderLayoutListener {
public:
    explicit TestRenderLayoutListener(std::function<void()> callback = {}) : callback_(std::move(callback)) {}

    void OnDrawOrLayout() override
    {
        if (callback_) {
            callback_();
        }
    }

private:
    std::function<void()> callback_;
};
} // namespace

/**
 * @tc.name: InstanceAndEventIsolation
 * @tc.desc: A listener removed from one instance or event must not affect another.
 * @tc.type: FUNC
 */
HWTEST_F(UIObserverStoreTest, InstanceAndEventIsolation, TestSize.Level1)
{
    UIObserverStore first;
    UIObserverStore second;
    auto listener = std::make_shared<TestRenderLayoutListener>();
    auto& firstDraw = first.GetRenderStore().GetListeners(UIObserverRenderType::DRAW);
    auto& firstLayout = first.GetRenderStore().GetListeners(UIObserverRenderType::LAYOUT);
    auto& secondDraw = second.GetRenderStore().GetListeners(UIObserverRenderType::DRAW);
    firstDraw.Register(listener);
    firstLayout.Register(listener);
    secondDraw.Register(listener);
    firstDraw.ClearListeners();
    EXPECT_TRUE(firstDraw.Snapshot().empty());
    EXPECT_EQ(firstLayout.Snapshot().size(), 1u);
    EXPECT_EQ(secondDraw.Snapshot().size(), 1u);
    first.ClearListeners();
    EXPECT_TRUE(firstLayout.Snapshot().empty());
    EXPECT_EQ(secondDraw.Snapshot().size(), 1u);
}

/**
 * @tc.name: SnapshotSurvivesReentrantRegistration
 * @tc.desc: on/off during a dispatch changes the next batch, preserving current order.
 * @tc.type: FUNC
 */
HWTEST_F(UIObserverStoreTest, SnapshotSurvivesReentrantRegistration, TestSize.Level1)
{
    UIObserverStore store;
    auto& listeners = store.GetRenderStore().GetListeners(UIObserverRenderType::DRAW);
    std::vector<int> calls;
    auto later = std::make_shared<TestRenderLayoutListener>([&calls] { calls.push_back(3); });
    auto first = std::make_shared<TestRenderLayoutListener>([&] {
        calls.push_back(1);
        listeners.ClearListeners();
        listeners.Register(later);
    });
    auto second = std::make_shared<TestRenderLayoutListener>([&calls] { calls.push_back(2); });
    listeners.Register(first);
    listeners.Register(second);
    for (const auto& listener : listeners.Snapshot()) {
        listener->OnDrawOrLayout();
    }
    EXPECT_EQ(calls, (std::vector<int> { 1, 2 }));
    for (const auto& listener : listeners.Snapshot()) {
        listener->OnDrawOrLayout();
    }
    EXPECT_EQ(calls, (std::vector<int> { 1, 2, 3 }));
}

/**
 * @tc.name: UnregisterOnlyMatchingListener
 * @tc.desc: Removing one callback preserves other registrations and ignores null/duplicates.
 * @tc.type: FUNC
 */
HWTEST_F(UIObserverStoreTest, UnregisterOnlyMatchingListener, TestSize.Level1)
{
    UIObserverStore store;
    auto& listeners = store.GetRenderStore().GetListeners(UIObserverRenderType::LAYOUT);
    auto first = std::make_shared<TestRenderLayoutListener>();
    auto second = std::make_shared<TestRenderLayoutListener>();
    listeners.Register(nullptr);
    listeners.Register(first);
    listeners.Register(first);
    listeners.Register(second);
    listeners.Unregister([&first](const std::shared_ptr<RenderLayoutListener>& listener) { return listener == first; });
    auto snapshot = listeners.Snapshot();
    ASSERT_EQ(snapshot.size(), 1u);
    EXPECT_EQ(snapshot.front(), second);
}

/**
 * @tc.name: ClearReleasesAllBusinessListeners
 * @tc.desc: The root cleanup entry releases both Render event buckets and is idempotent.
 * @tc.type: FUNC
 */
HWTEST_F(UIObserverStoreTest, ClearReleasesAllBusinessListeners, TestSize.Level1)
{
    UIObserverStore store;
    auto draw = std::make_shared<TestRenderLayoutListener>();
    auto layout = std::make_shared<TestRenderLayoutListener>();
    std::weak_ptr<TestRenderLayoutListener> weakDraw = draw;
    std::weak_ptr<TestRenderLayoutListener> weakLayout = layout;
    store.GetRenderStore().GetListeners(UIObserverRenderType::DRAW).Register(draw);
    store.GetRenderStore().GetListeners(UIObserverRenderType::LAYOUT).Register(layout);
    draw.reset();
    layout.reset();
    ASSERT_FALSE(weakDraw.expired());
    ASSERT_FALSE(weakLayout.expired());
    store.ClearListeners();
    store.ClearListeners();
    EXPECT_TRUE(weakDraw.expired());
    EXPECT_TRUE(weakLayout.expired());
}

/**
 * @tc.name: CloseInvalidatesPendingDispatch
 * @tc.desc: A retained dispatch snapshot can observe closure after container teardown.
 * @tc.type: FUNC
 */
HWTEST_F(UIObserverStoreTest, CloseInvalidatesPendingDispatch, TestSize.Level1)
{
    auto store = std::make_shared<UIObserverStore>();
    store->GetRenderStore().GetListeners(UIObserverRenderType::DRAW).Register(
        std::make_shared<TestRenderLayoutListener>());
    auto dispatchStore = store;
    auto snapshot = dispatchStore->GetRenderStore().GetListeners(UIObserverRenderType::DRAW).Snapshot();
    store->Close();
    store.reset();
    EXPECT_TRUE(dispatchStore->IsClosed());
    EXPECT_TRUE(dispatchStore->GetRenderStore().GetListeners(UIObserverRenderType::DRAW).Snapshot().empty());
    EXPECT_EQ(snapshot.size(), 1u);
}
/**
 * @tc.name: SnapshotOwnsListenerUntilDispatchEnds
 * @tc.desc: off releases the bucket but keeps callbacks alive through the current batch.
 * @tc.type: FUNC
 */
HWTEST_F(UIObserverStoreTest, SnapshotOwnsListenerUntilDispatchEnds, TestSize.Level1)
{
    UIObserverStore store;
    auto listener = std::make_shared<TestRenderLayoutListener>();
    std::weak_ptr<RenderLayoutListener> weak = listener;
    auto& listeners = store.GetRenderStore().GetListeners(UIObserverRenderType::DRAW);
    listeners.Register(listener);
    auto snapshot = listeners.Snapshot();
    listener.reset();
    store.Close();
    EXPECT_FALSE(weak.expired());
    snapshot.clear();
    EXPECT_TRUE(weak.expired());
}

} // namespace OHOS::Ace
