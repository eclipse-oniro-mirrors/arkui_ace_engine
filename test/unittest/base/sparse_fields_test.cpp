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

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "test/unittest/base/sparse_ref_ptr_late_type.h"

#include "base/utils/sparse_fields.h"
#include "base/utils/sparse_property.h"
#include "base/utils/sparse_ref_ptr.h"
#include "base/utils/stable_sparse_fields.h"
#include "base/utils/utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
int32_t g_trackedAlive = 0;

// Counts live instances: constructed minus destroyed, moved-from objects included.
template<size_t PadBytes>
struct TrackedValue {
    explicit TrackedValue(int32_t v = 0) : value(v)
    {
        ++g_trackedAlive;
    }
    TrackedValue(const TrackedValue& other) : value(other.value)
    {
        ++g_trackedAlive;
    }
    TrackedValue(TrackedValue&& other) noexcept : value(other.value)
    {
        ++g_trackedAlive;
    }
    TrackedValue& operator=(const TrackedValue& other) = default;
    TrackedValue& operator=(TrackedValue&& other) noexcept = default;
    ~TrackedValue()
    {
        --g_trackedAlive;
    }

    int32_t value = 0;
    std::array<char, PadBytes> pad {};
};
// A tracked value with a 4-byte pad (8 bytes in total) lives in the slot; one with a 60-byte pad
// (64 bytes in total) lives in its own allocation.
constexpr size_t TRACKED_PAD_BYTES = 4;
constexpr size_t TRACKED_BIG_PAD_BYTES = 60;
using Tracked = TrackedValue<TRACKED_PAD_BYTES>;
using TrackedBig = TrackedValue<TRACKED_BIG_PAD_BYTES>;

struct Small16 {
    int32_t a = 0;
    int32_t b = 0;
    int32_t c = 0;
    int32_t d = 0;
};

struct Big64 {
    static constexpr size_t kWords = 16; // 64 bytes: above the slot size, lives in its own allocation
    std::array<int32_t, kWords> v {};
};

// 12 fields: more than the 8 slots of the object, the rest goes to the overflow list.
enum : uint32_t {
    INT_A,
    SMALL,
    BIG,
    TEXT,
    TRACKED,
    TRACKED_BIG,
    INT_6,
    INT_7,
    INT_8,
    INT_9,
    INT_10,
    INT_11,
    FIELD_COUNT,
};

using StorageTypes = SparseFieldsDetail::TypeList<int32_t, Small16, Big64, std::string, Tracked, TrackedBig, int32_t,
    int32_t, int32_t, int32_t, int32_t, int32_t>;

struct StorageSchema {
    static constexpr uint32_t kFields = FIELD_COUNT;
    template<uint32_t Id>
    using Type = SparseFieldsDetail::TypeAt<Id, StorageTypes>;
};

constexpr uint32_t DEFAULT_SLOTS = 8; // the default slots-per-block of StableSparseFields
constexpr uint32_t FOUR_SLOTS = 4;    // the compact 4-slot block variant
using Storage = StableSparseFields<StorageSchema>;
using FourSlotStorage = StableSparseFields<StorageSchema, FOUR_SLOTS>;
using NoMaskStorage = StableSparseFields<StorageSchema, DEFAULT_SLOTS, false>;
using CopyableStorage = CopyableStableSparseFields<StorageSchema>;

// Number of heap blocks held by the storage.
template<class S>
uint32_t CountAllocations(const S& storage)
{
    return static_cast<uint32_t>(storage.HeapBytes([](const void* p) -> size_t { return p != nullptr ? 1 : 0; }));
}

// Sets INT_6..INT_11 to their own numbers. After the six fields set before them, INT_6 and INT_7 take the last
// slots and INT_8..INT_11 go beyond them.
template<class S>
void EmplaceIntFields(S& storage)
{
    storage.template Emplace<INT_6>(INT_6);
    storage.template Emplace<INT_7>(INT_7);
    storage.template Emplace<INT_8>(INT_8);
    storage.template Emplace<INT_9>(INT_9);
    storage.template Emplace<INT_10>(INT_10);
    storage.template Emplace<INT_11>(INT_11);
}

// Phase 1: set every field and check the values. Adding fields — into the second block and beyond the slots —
// moves no set value, and Emplace of a set field allocates nothing, beyond the slots too.
template<class S>
void CheckFieldsSet(S& storage)
{
    constexpr int32_t intValueA = 100;
    constexpr int32_t trackedValue = 5;
    constexpr int32_t trackedBigValue = 6;
    constexpr int32_t bothTrackedAlive = 2;
    constexpr int32_t smallFieldD = 4;
    constexpr int32_t bigFieldLast = 15;
    EXPECT_TRUE(storage.Empty());
    Small16* small = &storage.template Emplace<SMALL>(Small16 { 1, 2, 3, smallFieldD });
    Big64* big = &storage.template Emplace<BIG>();
    big->v[Big64::kWords - 1] = bigFieldLast;
    storage.template Emplace<INT_A>(intValueA);
    storage.template Emplace<TEXT>("a text value that is long enough to need its own heap buffer");
    storage.template Emplace<TRACKED>(trackedValue);
    storage.template Emplace<TRACKED_BIG>(trackedBigValue);
    EmplaceIntFields(storage);
    EXPECT_EQ(storage.template Get<SMALL>(), small);
    EXPECT_EQ(storage.template Get<BIG>(), big);
    const uint32_t allocations = CountAllocations(storage);
    const int32_t* overflowValue = storage.template Get<INT_11>();
    EmplaceIntFields(storage);
    EXPECT_EQ(CountAllocations(storage), allocations);
    EXPECT_EQ(storage.template Get<INT_11>(), overflowValue);
    EXPECT_EQ(small->d, smallFieldD);
    EXPECT_EQ(big->v[Big64::kWords - 1], bigFieldLast);
    EXPECT_EQ(*storage.template Get<INT_A>(), intValueA);
    EXPECT_EQ(storage.template Get<TRACKED>()->value, trackedValue);
    EXPECT_EQ(storage.template Get<TRACKED_BIG>()->value, trackedBigValue);
    EXPECT_EQ(*storage.template Get<INT_6>(), static_cast<int32_t>(INT_6));
    EXPECT_EQ(*storage.template Get<INT_11>(), static_cast<int32_t>(INT_11));
    EXPECT_EQ(g_trackedAlive, bothTrackedAlive);
}

// Phase 2: remove the fields part by part, check that unset fields read as absent and nothing is left.
template<class S>
void CheckFieldsRemoved(S& storage)
{
    constexpr int32_t oneTrackedAlive = 1;
    constexpr int32_t noneTrackedAlive = 0;
    const void* small = storage.template Get<SMALL>();
    const void* big = storage.template Get<BIG>();
    storage.template Reset<INT_A>();
    storage.template Reset<TRACKED>();
    storage.template Reset<INT_9>();
    EXPECT_FALSE(storage.template Has<INT_A>());
    EXPECT_FALSE(storage.template Has<TRACKED>());
    EXPECT_FALSE(storage.template Has<INT_9>());
    EXPECT_EQ(storage.template Get<INT_9>(), nullptr);
    EXPECT_EQ(g_trackedAlive, oneTrackedAlive);
    EXPECT_EQ(storage.template Get<SMALL>(), small);
    EXPECT_EQ(storage.template Get<BIG>(), big);
    EXPECT_EQ(*storage.template Get<INT_10>(), static_cast<int32_t>(INT_10));

    storage.template Reset<SMALL>();
    storage.template Reset<BIG>();
    storage.template Reset<TEXT>();
    storage.template Reset<TRACKED_BIG>();
    storage.template Reset<INT_6>();
    storage.template Reset<INT_7>();
    storage.template Reset<INT_8>();
    storage.template Reset<INT_10>();
    EXPECT_FALSE(storage.Empty());
    storage.template Reset<INT_11>();
    EXPECT_TRUE(storage.Empty());
    EXPECT_EQ(CountAllocations(storage), 0u);
    EXPECT_EQ(g_trackedAlive, noneTrackedAlive);
}

// Lifecycle of a storage, split into two phases to keep each helper small. Each INT_N field holds the value
// of its own number; the tracked counters are the number of live tracked values.
template<class S>
void CheckFullLifecycle()
{
    constexpr int32_t trackedReplacedValue = 7;
    constexpr int32_t trackedReusedValue = 8;
    constexpr int32_t noneTrackedAlive = 0;
    {
        S storage;
        CheckFieldsSet(storage);
        CheckFieldsRemoved(storage);
        storage.template Emplace<TRACKED_BIG>(trackedReplacedValue);
        storage.template Emplace<TRACKED>(trackedReusedValue);
    }
    EXPECT_EQ(g_trackedAlive, noneTrackedAlive);
}
} // namespace

class StableSparseFieldsTest : public testing::Test {
protected:
    void SetUp() override
    {
        g_trackedAlive = 0;
    }
};

/**
 * @tc.name: EmptyStorage001
 * @tc.desc: An empty storage takes 24 bytes with the mask and 16 without it, has no fields and no heap blocks
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, EmptyStorage001, TestSize.Level1)
{
    constexpr size_t maskedSize = 24;   // slot ids + block pointer + mask
    constexpr size_t unmaskedSize = 16; // slot ids + block pointer
    EXPECT_EQ(sizeof(Storage), maskedSize);
    EXPECT_EQ(sizeof(NoMaskStorage), unmaskedSize);

    const Storage storage;
    EXPECT_TRUE(storage.Empty());
    EXPECT_FALSE(storage.Has<INT_A>());
    EXPECT_FALSE(storage.Has<BIG>());
    EXPECT_EQ(storage.Get<INT_A>(), nullptr);
    EXPECT_EQ(storage.Get<BIG>(), nullptr);
    EXPECT_EQ(CountAllocations(storage), 0u);
}

/**
 * @tc.name: EmplaceGet001
 * @tc.desc: Emplace sets a field; a small value lives in the block, a big value in its own allocation
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, EmplaceGet001, TestSize.Level1)
{
    Storage storage;
    int32_t& a = storage.Emplace<INT_A>(42);
    EXPECT_EQ(a, 42);
    EXPECT_TRUE(storage.Has<INT_A>());
    EXPECT_EQ(storage.Get<INT_A>(), &a);
    EXPECT_FALSE(storage.Empty());
    constexpr uint32_t oneBlock = 1;
    EXPECT_EQ(CountAllocations(storage), oneBlock);

    storage.Emplace<SMALL>(Small16 { 1, 2, 3, 4 });
    EXPECT_EQ(storage.Get<SMALL>()->c, 3);
    EXPECT_EQ(CountAllocations(storage), oneBlock);

    Big64 big;
    big.v[0] = 7;
    storage.Emplace<BIG>(big);
    EXPECT_EQ(storage.Get<BIG>()->v[0], 7);
    constexpr uint32_t twoBlocks = 2;
    EXPECT_EQ(CountAllocations(storage), twoBlocks);

    storage.Get<BIG>()->v[0] = 8;
    const Storage& constStorage = storage;
    EXPECT_EQ(constStorage.Get<BIG>()->v[0], 8);
    EXPECT_FALSE(constStorage.Has<TEXT>());
}

/**
 * @tc.name: EmplaceSameField001
 * @tc.desc: Emplace of a set field assigns the new value in place: the address does not change
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, EmplaceSameField001, TestSize.Level1)
{
    Storage storage;
    Small16* small = &storage.Emplace<SMALL>(Small16 { 1, 1, 1, 1 });
    Big64* big = &storage.Emplace<BIG>();
    storage.Emplace<TEXT>("first");

    EXPECT_EQ(&storage.Emplace<SMALL>(Small16 { 2, 2, 2, 2 }), small);
    EXPECT_EQ(small->a, 2);
    Big64 other;
    other.v[3] = 3;
    EXPECT_EQ(&storage.Emplace<BIG>(other), big);
    EXPECT_EQ(big->v[3], 3);

    // The arguments may refer to the old value.
    storage.Emplace<TEXT>(*storage.Get<TEXT>() + " second");
    EXPECT_EQ(*storage.Get<TEXT>(), "first second");
    storage.Emplace<BIG>(*storage.Get<BIG>());
    EXPECT_EQ(storage.Get<BIG>()->v[3], 3);
}

/**
 * @tc.name: AddressStability001
 * @tc.desc: Adding and removing other fields, including overflow beyond 8 slots, does not move a set value
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, AddressStability001, TestSize.Level1)
{
    Storage storage;
    Small16* small = &storage.Emplace<SMALL>(Small16 { 1, 2, 3, 4 });
    Big64* big = &storage.Emplace<BIG>();
    big->v[1] = 1;

    storage.Emplace<INT_A>(0);
    storage.Emplace<TEXT>("text");
    storage.Emplace<TRACKED>(1);
    storage.Emplace<TRACKED_BIG>(2);
    storage.Emplace<INT_6>(6);
    storage.Emplace<INT_7>(7);
    storage.Emplace<INT_8>(8);
    storage.Emplace<INT_9>(9);
    storage.Emplace<INT_10>(10);
    storage.Emplace<INT_11>(11);
    EXPECT_EQ(storage.Get<SMALL>(), small);
    EXPECT_EQ(storage.Get<BIG>(), big);

    storage.Reset<INT_A>();
    storage.Reset<INT_9>();
    storage.Reset<TRACKED_BIG>();
    storage.Emplace<INT_9>(90);
    EXPECT_EQ(storage.Get<SMALL>(), small);
    EXPECT_EQ(storage.Get<BIG>(), big);
    EXPECT_EQ(small->d, 4);
    EXPECT_EQ(big->v[1], 1);
    EXPECT_EQ(*storage.Get<INT_9>(), 90);
    EXPECT_EQ(*storage.Get<INT_11>(), 11);
}

/**
 * @tc.name: ResetReusesSlot001
 * @tc.desc: Reset frees the slot and the next new field takes it; a field beyond 8 slots goes to the overflow
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, ResetReusesSlot001, TestSize.Level1)
{
    Storage storage;
    storage.Emplace<INT_A>(0);
    storage.Emplace<SMALL>();
    storage.Emplace<TRACKED>(1);
    storage.Emplace<INT_6>(6);
    storage.Emplace<INT_7>(7);
    storage.Emplace<INT_8>(8);
    storage.Emplace<INT_9>(9);
    storage.Emplace<INT_10>(10);
    EXPECT_EQ(CountAllocations(storage), 1u); // 8 small values in one block

    storage.Reset<INT_6>();
    storage.Emplace<INT_11>(11);
    EXPECT_EQ(CountAllocations(storage), 1u); // the freed slot is reused
    EXPECT_FALSE(storage.Has<INT_6>());
    EXPECT_EQ(*storage.Get<INT_11>(), 11);

    storage.Emplace<BIG>().v[0] = 5; // the 9th field: service object + entry list + block + the value itself
    EXPECT_EQ(CountAllocations(storage), 4u);
    EXPECT_EQ(storage.Get<BIG>()->v[0], 5);
    EXPECT_EQ(*storage.Get<INT_10>(), 10);

    storage.Reset<BIG>();
    EXPECT_FALSE(storage.Has<BIG>());
    EXPECT_EQ(*storage.Get<INT_10>(), 10);
}

/**
 * @tc.name: ResetAll001
 * @tc.desc: Reset of an absent field does nothing; when the last field is removed all blocks are freed
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, ResetAll001, TestSize.Level1)
{
    Storage storage;
    storage.Reset<INT_A>();
    storage.Reset<BIG>();
    EXPECT_TRUE(storage.Empty());
    EXPECT_EQ(CountAllocations(storage), 0u);

    storage.Emplace<INT_A>(1);
    storage.Emplace<BIG>();
    storage.Reset<TEXT>();
    EXPECT_TRUE(storage.Has<INT_A>());
    storage.Reset<INT_A>();
    EXPECT_FALSE(storage.Empty());
    storage.Reset<BIG>();
    EXPECT_TRUE(storage.Empty());
    EXPECT_EQ(CountAllocations(storage), 0u);
}

/**
 * @tc.name: Destruction001
 * @tc.desc: Every value is destroyed exactly once: on replace, on Reset and with the storage
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, Destruction001, TestSize.Level1)
{
    {
        Storage storage;
        storage.Emplace<TRACKED>(1);
        storage.Emplace<TRACKED_BIG>(2);
        EXPECT_EQ(g_trackedAlive, 2);
        storage.Emplace<TRACKED>(3);
        storage.Emplace<TRACKED_BIG>(4);
        EXPECT_EQ(g_trackedAlive, 2);
        EXPECT_EQ(storage.Get<TRACKED>()->value, 3);
        EXPECT_EQ(storage.Get<TRACKED_BIG>()->value, 4);
        storage.Reset<TRACKED>();
        EXPECT_EQ(g_trackedAlive, 1);
        storage.Emplace<TRACKED>(5);
        EXPECT_EQ(g_trackedAlive, 2);
    }
    EXPECT_EQ(g_trackedAlive, 0);
}

/**
 * @tc.name: Move001
 * @tc.desc: Moving the storage takes its blocks: value addresses are kept, the source becomes empty
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, Move001, TestSize.Level1)
{
    Storage source;
    Small16* small = &source.Emplace<SMALL>(Small16 { 1, 2, 3, 4 });
    Big64* big = &source.Emplace<BIG>();

    Storage moved(std::move(source));
    EXPECT_TRUE(source.Empty());
    EXPECT_EQ(CountAllocations(source), 0u);
    EXPECT_EQ(moved.Get<SMALL>(), small);
    EXPECT_EQ(moved.Get<BIG>(), big);

    Storage target;
    target.Emplace<TRACKED>(1);
    EXPECT_EQ(g_trackedAlive, 1);
    target = std::move(moved);
    EXPECT_EQ(g_trackedAlive, 0); // the old contents of the target are destroyed
    EXPECT_FALSE(target.Has<TRACKED>());
    EXPECT_EQ(target.Get<SMALL>(), small);
    EXPECT_EQ(target.Get<BIG>(), big);
    EXPECT_TRUE(moved.Empty());
}

/**
 * @tc.name: Copy001
 * @tc.desc: CopyableStableSparseFields copies every set field into new addresses; the source is untouched
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, Copy001, TestSize.Level1)
{
    {
        CopyableStorage source;
        source.Emplace<TEXT>("text");
        source.Emplace<BIG>().v[2] = 2;
        source.Emplace<TRACKED>(7);

        CopyableStorage copy(source);
        EXPECT_NE(copy.Get<TEXT>(), source.Get<TEXT>());
        EXPECT_NE(copy.Get<BIG>(), source.Get<BIG>());
        EXPECT_EQ(*copy.Get<TEXT>(), "text");
        EXPECT_EQ(copy.Get<BIG>()->v[2], 2);
        EXPECT_EQ(copy.Get<TRACKED>()->value, 7);
        EXPECT_EQ(g_trackedAlive, 2);

        copy.Get<TEXT>()->append("!");
        EXPECT_EQ(*source.Get<TEXT>(), "text");

        CopyableStorage target;
        target.Emplace<INT_A>(1);
        target = source;
        EXPECT_FALSE(target.Has<INT_A>()); // fields absent in the source are removed
        EXPECT_EQ(*target.Get<TEXT>(), "text");
        EXPECT_EQ(g_trackedAlive, 3);

        const CopyableStorage& self = target;
        target = self; // self-assignment keeps the contents
        EXPECT_EQ(*target.Get<TEXT>(), "text");
        EXPECT_EQ(g_trackedAlive, 3);
    }
    EXPECT_EQ(g_trackedAlive, 0);
}

/**
 * @tc.name: FourSlotBlocks001
 * @tc.desc: With blocks of 4 slots the second block is allocated only for the fifth field
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, FourSlotBlocks001, TestSize.Level1)
{
    FourSlotStorage storage;
    storage.Emplace<INT_A>(0);
    storage.Emplace<INT_6>(6);
    storage.Emplace<INT_7>(7);
    storage.Emplace<INT_8>(8);
    EXPECT_EQ(CountAllocations(storage), 1u);

    storage.Emplace<INT_9>(9); // service object + two blocks
    EXPECT_EQ(CountAllocations(storage), 3u);
    EXPECT_EQ(*storage.Get<INT_A>(), 0);
    EXPECT_EQ(*storage.Get<INT_9>(), 9);

    CheckFullLifecycle<FourSlotStorage>();
}

/**
 * @tc.name: Lifecycle001
 * @tc.desc: All fields set, partly removed and removed again work the same with and without the mask
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, Lifecycle001, TestSize.Level1)
{
    CheckFullLifecycle<Storage>();
    CheckFullLifecycle<NoMaskStorage>();
}

namespace {
struct Pinned {
    explicit Pinned(int32_t v) : value(v) {}
    Pinned(const Pinned&) = delete;
    Pinned& operator=(const Pinned&) = delete;
    ~Pinned() = default;

    int32_t value = 0;
};

struct PinnedBig {
    explicit PinnedBig(int32_t v)
    {
        values[0] = v;
    }
    PinnedBig(const PinnedBig&) = delete;
    PinnedBig& operator=(const PinnedBig&) = delete;
    ~PinnedBig() = default;

    // 64 bytes: above the slot size, lives in its own allocation
    std::array<int32_t, Big64::kWords> values {};
};

// A large type without copy and move constructors whose constructor and destructor call back: the callbacks may use
// the storage.
struct PinnedBuildProbe {
    PinnedBuildProbe(int32_t v, const std::function<void()>& onBuild, std::function<void()> callback = nullptr)
        : onDestroy(std::move(callback))
    {
        values[0] = v;
        onBuild();
    }
    PinnedBuildProbe(const PinnedBuildProbe&) = delete;
    PinnedBuildProbe& operator=(const PinnedBuildProbe&) = delete;
    ~PinnedBuildProbe()
    {
        if (onDestroy) {
            onDestroy();
        }
    }

    std::array<int32_t, Big64::kWords> values {};
    std::function<void()> onDestroy;
};

enum : uint32_t {
    PINNED,
    PINNED_BIG,
    PINNED_BUILD,
    PINNED_FIELD_COUNT,
};

struct PinnedSchema {
    static constexpr uint32_t kFields = PINNED_FIELD_COUNT;
    template<uint32_t Id>
    using Type = SparseFieldsDetail::TypeAt<Id, SparseFieldsDetail::TypeList<Pinned, PinnedBig, PinnedBuildProbe>>;
};
} // namespace

/**
 * @tc.name: NonMovableType001
 * @tc.desc: A type without copy and move constructors is built and rebuilt in place
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, NonMovableType001, TestSize.Level1)
{
    StableSparseFields<PinnedSchema> storage;
    Pinned* pinned = &storage.Emplace<PINNED>(1);
    PinnedBig* pinnedBig = &storage.Emplace<PINNED_BIG>(2);
    EXPECT_EQ(&storage.Emplace<PINNED>(3), pinned);
    EXPECT_EQ(&storage.Emplace<PINNED_BIG>(4), pinnedBig);
    EXPECT_EQ(storage.Get<PINNED>()->value, 3);
    EXPECT_EQ(storage.Get<PINNED_BIG>()->values[0], 4);

    storage.Reset<PINNED>();
    EXPECT_FALSE(storage.Has<PINNED>());
    storage.Reset<PINNED_BIG>();
    EXPECT_TRUE(storage.Empty());
}

/**
 * @tc.name: NonMovableReentrantBuild001
 * @tc.desc: The constructor of a large type without a move constructor may set fields of the same storage: another
 *           field keeps its own slot, the same field is replaced by the value under construction
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, NonMovableReentrantBuild001, TestSize.Level1)
{
    StableSparseFields<PinnedSchema> storage;
    storage.Emplace<PINNED_BUILD>(1, [&storage]() { storage.Emplace<PINNED>(5); });
    ASSERT_NE(storage.Get<PINNED>(), nullptr);
    EXPECT_EQ(storage.Get<PINNED>()->value, 5);
    ASSERT_NE(storage.Get<PINNED_BUILD>(), nullptr);
    EXPECT_EQ(storage.Get<PINNED_BUILD>()->values[0], 1);
    storage.Reset<PINNED>();
    storage.Reset<PINNED_BUILD>();
    EXPECT_TRUE(storage.Empty());

    storage.Emplace<PINNED_BUILD>(2, [&storage]() { storage.Emplace<PINNED_BUILD>(3, []() {}); });
    ASSERT_NE(storage.Get<PINNED_BUILD>(), nullptr);
    EXPECT_EQ(storage.Get<PINNED_BUILD>()->values[0], 2);
    storage.Reset<PINNED_BUILD>();
    EXPECT_TRUE(storage.Empty());
    EXPECT_EQ(CountAllocations(storage), 0u);
}

/**
 * @tc.name: NonMovableReentrantBuild002
 * @tc.desc: The constructor of a large type without a move constructor sets the same field to a value whose
 *           destructor sets it once more: the value under construction still wins and the field keeps one slot
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, NonMovableReentrantBuild002, TestSize.Level1)
{
    StableSparseFields<PinnedSchema> storage;
    auto noop = []() {};
    auto setAgainOnDestroy = [&storage, &noop]() { storage.Emplace<PINNED_BUILD>(4, noop); };
    storage.Emplace<PINNED_BUILD>(
        2, [&storage, &noop, &setAgainOnDestroy]() { storage.Emplace<PINNED_BUILD>(3, noop, setAgainOnDestroy); });
    ASSERT_NE(storage.Get<PINNED_BUILD>(), nullptr);
    EXPECT_EQ(storage.Get<PINNED_BUILD>()->values[0], 2);
    storage.Reset<PINNED_BUILD>();
    EXPECT_FALSE(storage.Has<PINNED_BUILD>());
    EXPECT_TRUE(storage.Empty());
    EXPECT_EQ(CountAllocations(storage), 0u);
}

namespace {
// Calls the callback from its destructor, once; a moved-from probe calls nothing.
struct DestroyProbe {
    explicit DestroyProbe(std::function<void()> callback) : onDestroy(std::move(callback)) {}
    DestroyProbe(DestroyProbe&& other) noexcept : onDestroy(std::move(other.onDestroy))
    {
        other.onDestroy = nullptr;
    }
    DestroyProbe& operator=(DestroyProbe&& other) noexcept
    {
        if (this != &other) {
            onDestroy = std::move(other.onDestroy);
            other.onDestroy = nullptr;
        }
        return *this;
    }
    DestroyProbe(const DestroyProbe&) = delete;
    DestroyProbe& operator=(const DestroyProbe&) = delete;
    ~DestroyProbe()
    {
        if (onDestroy) {
            auto callback = std::move(onDestroy);
            onDestroy = nullptr;
            callback();
        }
    }

    std::function<void()> onDestroy;
};

enum : uint32_t {
    PROBE,
    PROBE_PEER,
    PROBE_FIELD_COUNT,
};

struct ProbeSchema {
    static constexpr uint32_t kFields = PROBE_FIELD_COUNT;
    template<uint32_t Id>
    using Type = SparseFieldsDetail::TypeAt<Id, SparseFieldsDetail::TypeList<DestroyProbe, int32_t>>;
};
using ProbeStorage = StableSparseFields<ProbeSchema>;
} // namespace

/**
 * @tc.name: ReentrantDestructor001
 * @tc.desc: A value destructor that uses the same storage finds its field absent and the storage consistent
 * @tc.type: FUNC
 */
HWTEST_F(StableSparseFieldsTest, ReentrantDestructor001, TestSize.Level1)
{
    int32_t calls = 0;
    bool sawSelf = true;
    int32_t peer = 0;
    ProbeStorage storage;
    storage.Emplace<PROBE_PEER>(1);
    storage.Emplace<PROBE>([&storage, &calls, &sawSelf, &peer]() {
        ++calls;
        sawSelf = storage.Has<PROBE>();
        peer = storage.Has<PROBE_PEER>() ? *storage.Get<PROBE_PEER>() : 0;
        storage.Reset<PROBE>(); // must not destroy the value a second time
    });
    EXPECT_EQ(calls, 0);
    storage.Reset<PROBE>();
    EXPECT_EQ(calls, 1);
    EXPECT_FALSE(sawSelf);
    EXPECT_EQ(peer, 1);
    EXPECT_EQ(*storage.Get<PROBE_PEER>(), 1);

    bool sawEmpty = false;
    {
        ProbeStorage dying;
        dying.Emplace<PROBE_PEER>(2);
        dying.Emplace<PROBE>([&dying, &calls, &sawEmpty]() {
            ++calls;
            sawEmpty = dying.Empty();
            dying.Reset<PROBE>();
        });
    }
    EXPECT_EQ(calls, 2);
    EXPECT_TRUE(sawEmpty);
}

namespace {
#define SPARSE_NODE_SPARSE_FIELDS(F) \
    F(Offset, Small16)               \
    F(Matrix, Big64)                 \
    F(Names, std::map<int32_t, std::string>)

class SparseNode {
public:
    int32_t id = 0;
    SPARSE_FIELDS(SPARSE_NODE_SPARSE_FIELDS);
};

#define SPARSE_NODE_EXT_SPARSE_FIELDS(F) F(Tag, std::string)

class SparseNodeExt : public SparseNode {
    SPARSE_FIELDS(SPARSE_NODE_EXT_SPARSE_FIELDS);
};

#define FOUR_SLOT_NODE_SPARSE_FIELDS(F) F(A, int32_t) F(B, int32_t) F(C, int32_t) F(D, int32_t) F(E, std::string)

class FourSlotNode {
    SPARSE_FIELDS_WITH_SLOTS(FOUR_SLOT_NODE_SPARSE_FIELDS, 4);
};
} // namespace

class SparseFieldsMacroTest : public testing::Test {};

/**
 * @tc.name: Accessors001
 * @tc.desc: F(Name, Type) gives Has/Get/Emplace/GetOrEmplace/Reset; a type with commas is taken whole
 * @tc.type: FUNC
 */
HWTEST_F(SparseFieldsMacroTest, Accessors001, TestSize.Level1)
{
    EXPECT_EQ(sizeof(SparseNode), 32u); // 4 bytes of id + padding + 24 bytes of storage
    EXPECT_EQ(SparseNode::SparseFieldIds::Offset, 0u);
    EXPECT_EQ(SparseNode::SparseFieldIds::Names, 2u);

    SparseNode node;
    EXPECT_FALSE(node.HasOffset());
    EXPECT_EQ(node.GetMatrix(), nullptr);

    node.EmplaceOffset(Small16 { 1, 2, 3, 4 });
    EXPECT_TRUE(node.HasOffset());
    EXPECT_EQ(node.GetOffset()->b, 2);
    node.GetOffset()->b = 20;
    const SparseNode& constNode = node;
    EXPECT_EQ(constNode.GetOffset()->b, 20);

    std::map<int32_t, std::string>& names = node.GetOrEmplaceNames();
    names[1] = "one";
    EXPECT_EQ(&node.GetOrEmplaceNames(), &names); // the existing value is returned, not recreated
    EXPECT_EQ(node.GetNames()->at(1), "one");

    Small16& offset = node.GetOrEmplaceOffset(Small16 { 9, 9, 9, 9 });
    EXPECT_EQ(offset.b, 20);

    node.ResetOffset();
    node.ResetNames();
    EXPECT_FALSE(node.HasOffset());
    EXPECT_EQ(node.GetNames(), nullptr);
}

/**
 * @tc.name: DerivedList001
 * @tc.desc: A derived class with its own list has its own storage; the base fields keep working
 * @tc.type: FUNC
 */
HWTEST_F(SparseFieldsMacroTest, DerivedList001, TestSize.Level1)
{
    SparseNodeExt node;
    node.EmplaceTag("tag");
    node.EmplaceMatrix().v[0] = 1;
    EXPECT_EQ(*node.GetTag(), "tag");
    EXPECT_EQ(node.GetMatrix()->v[0], 1);
    node.ResetMatrix();
    EXPECT_TRUE(node.HasTag());
    EXPECT_FALSE(node.HasMatrix());
}

/**
 * @tc.name: FourSlots001
 * @tc.desc: SPARSE_FIELDS_WITH_SLOTS(LIST, 4) keeps all five fields
 * @tc.type: FUNC
 */
HWTEST_F(SparseFieldsMacroTest, FourSlots001, TestSize.Level1)
{
    FourSlotNode node;
    node.EmplaceA(1);
    node.EmplaceB(2);
    node.EmplaceC(3);
    node.EmplaceD(4);
    node.EmplaceE("fifth");
    EXPECT_EQ(*node.GetA(), 1);
    EXPECT_EQ(*node.GetD(), 4);
    EXPECT_EQ(*node.GetE(), "fifth");
    node.ResetA();
    node.ResetE();
    EXPECT_FALSE(node.HasA());
    EXPECT_FALSE(node.HasE());
    EXPECT_EQ(*node.GetC(), 3);
}

namespace {
constexpr uint32_t FLAG_MEASURE = 1;
constexpr uint32_t FLAG_RENDER = 2;

#define SPARSE_GROUP_SPARSE_FIELDS(F, P) \
    P(Radius, float)                     \
    P(Name, std::string)

struct SparseGroup {
    SPARSE_FIELDS_GROUP(SPARSE_GROUP_SPARSE_FIELDS);
    ACE_DEFINE_SPARSE_PROPERTY_GROUP_ITEM(Radius, float);
    ACE_DEFINE_SPARSE_PROPERTY_GROUP_ITEM(Name, std::string);
};

#define SPARSE_PROP_OWNER_SPARSE_FIELDS(F, P) \
    F(Note, std::string)                      \
    P(Width, float)                           \
    P(Label, std::string)                     \
    P(Order, int32_t)                         \
    P(Weight, int32_t)                        \
    P(Group, SparseGroup)

class SparsePropOwner {
    SPARSE_FIELDS_WITH_PROPS(SPARSE_PROP_OWNER_SPARSE_FIELDS);

public:
    ACE_DEFINE_SPARSE_PROPERTY_ITEM(Width, float, FLAG_MEASURE);
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_FUNC(Label, std::string);
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_AND_USING_CALLBACK(Order, int32_t, FLAG_RENDER);
    ACE_DEFINE_SPARSE_PROPERTY_GROUP_ITEM_WITH_CALLBACK(Weight, int32_t);
    ACE_DEFINE_SPARSE_PROPERTY_GROUP(Group, SparseGroup);

public:
    void UpdatePropertyChangeFlag(uint32_t flag)
    {
        changeFlag |= flag;
    }
    void OnLabelUpdate(const std::string& label)
    {
        labelUpdates.push_back(label);
    }
    void OnOrderUpdate(int32_t order)
    {
        orderInCallback = GetOrderValue(-1);
        flagBeforeOrderCallback = (changeFlag & FLAG_RENDER) != 0;
    }
    void OnWeightUpdate(int32_t weight)
    {
        weightInCallback = GetWeightValue(-1);
    }

    uint32_t changeFlag = 0;
    std::vector<std::string> labelUpdates;
    int32_t orderInCallback = 0;
    bool flagBeforeOrderCallback = false;
    int32_t weightInCallback = 0;
};
} // namespace

class SparsePropertyTest : public testing::Test {};

/**
 * @tc.name: PropertyItem001
 * @tc.desc: Update writes a new value and sets the change flag; a NearEqual value changes nothing
 * @tc.type: FUNC
 */
HWTEST_F(SparsePropertyTest, PropertyItem001, TestSize.Level1)
{
    SparsePropOwner owner;
    EXPECT_FALSE(owner.HasWidth());
    EXPECT_TRUE(owner.GetWidth() == std::nullopt);
    EXPECT_FLOAT_EQ(owner.GetWidthValue(5.0f), 5.0f);
    EXPECT_FALSE(owner.CloneWidth().has_value());

    owner.UpdateWidth(10.0f);
    EXPECT_EQ(owner.changeFlag, FLAG_MEASURE);
    EXPECT_TRUE(owner.HasWidth());
    EXPECT_FLOAT_EQ(owner.GetWidthValue(), 10.0f);
    EXPECT_FLOAT_EQ(owner.GetWidthValue(5.0f), 10.0f);

    owner.changeFlag = 0;
    owner.UpdateWidth(10.0005f);
    EXPECT_EQ(owner.changeFlag, 0u);
    EXPECT_FLOAT_EQ(owner.GetWidthValue(), 10.0f);

    owner.UpdateWidth(20.0f);
    EXPECT_EQ(owner.changeFlag, FLAG_MEASURE);
    EXPECT_FLOAT_EQ(owner.CloneWidth().value_or(0.0f), 20.0f);

    owner.ResetWidth();
    EXPECT_FALSE(owner.HasWidth());
}

/**
 * @tc.name: PropertyView001
 * @tc.desc: Get##name() is a live view with the interface of const std::optional<T>&; ToOptional is a snapshot
 * @tc.type: FUNC
 */
HWTEST_F(SparsePropertyTest, PropertyView001, TestSize.Level1)
{
    SparsePropOwner owner;
    const auto& view = owner.GetWidth();
    EXPECT_FALSE(view.has_value());
    EXPECT_FLOAT_EQ(view.value_or(7.0f), 7.0f);

    owner.UpdateWidth(1.0f);
    EXPECT_TRUE(view.has_value());
    EXPECT_FLOAT_EQ(view.value(), 1.0f);
    std::optional<float> snapshot = owner.GetWidth().ToOptional();

    owner.UpdateWidth(2.0f);
    EXPECT_FLOAT_EQ(*view, 2.0f);
    EXPECT_FLOAT_EQ(snapshot.value_or(0.0f), 1.0f);
    EXPECT_TRUE(view == 2.0f);
    EXPECT_TRUE(view != 1.0f);
    EXPECT_TRUE(view != std::nullopt);
    EXPECT_TRUE(view == std::optional<float>(2.0f));
    EXPECT_TRUE(view == owner.GetWidth());

    owner.ResetWidth();
    EXPECT_FALSE(view.has_value());
    EXPECT_TRUE(view == std::nullopt);
}

/**
 * @tc.name: PropertyCallbacks001
 * @tc.desc: ITEM_FUNC calls On##name##Update after the write; Assign writes without NearEqual and callbacks
 * @tc.type: FUNC
 */
HWTEST_F(SparsePropertyTest, PropertyCallbacks001, TestSize.Level1)
{
    SparsePropOwner owner;
    owner.UpdateLabel("a");
    owner.UpdateLabel("a");
    ASSERT_EQ(owner.labelUpdates.size(), 1u);
    EXPECT_EQ(owner.labelUpdates[0], "a");
    EXPECT_EQ(owner.changeFlag, 0u);

    owner.AssignLabel(std::string("b"));
    EXPECT_EQ(owner.labelUpdates.size(), 1u);
    EXPECT_EQ(owner.GetLabelValue(), "b");
    owner.AssignLabel(std::optional<std::string>());
    EXPECT_FALSE(owner.HasLabel());
    owner.AssignLabel(std::optional<std::string>("c"));
    EXPECT_EQ(owner.GetLabelValue(), "c");

    // ITEM_AND_USING_CALLBACK: write, change flag, then the callback sees the new value.
    owner.UpdateOrder(3);
    EXPECT_EQ(owner.orderInCallback, 3);
    EXPECT_TRUE(owner.flagBeforeOrderCallback);
    EXPECT_EQ(owner.changeFlag, FLAG_RENDER);
}

/**
 * @tc.name: PropertyCallbackBeforeWrite001
 * @tc.desc: GROUP_ITEM_WITH_CALLBACK calls the callback before the write and reports whether the value changed
 * @tc.type: FUNC
 */
HWTEST_F(SparsePropertyTest, PropertyCallbackBeforeWrite001, TestSize.Level1)
{
    SparsePropOwner owner;
    EXPECT_TRUE(owner.UpdateWeight(1));
    EXPECT_EQ(owner.weightInCallback, -1);
    EXPECT_TRUE(owner.UpdateWeight(2));
    EXPECT_EQ(owner.weightInCallback, 1);
    owner.weightInCallback = 0;
    EXPECT_FALSE(owner.UpdateWeight(2));
    EXPECT_EQ(owner.weightInCallback, 0);
    EXPECT_EQ(owner.GetWeightValue(), 2);
}

/**
 * @tc.name: PropertyGroup001
 * @tc.desc: A property group lives in the owner storage: GetOrCreate, Clone, Assign and Reset
 * @tc.type: FUNC
 */
HWTEST_F(SparsePropertyTest, PropertyGroup001, TestSize.Level1)
{
    SparsePropOwner owner;
    EXPECT_TRUE(owner.GetGroup() == nullptr);
    EXPECT_TRUE(owner.CloneGroup() == nullptr);

    EXPECT_TRUE(owner.GetOrCreateGroup()->UpdateRadius(3.0f));
    EXPECT_FALSE(owner.GetOrCreateGroup()->UpdateRadius(3.0f));
    ASSERT_TRUE(owner.GetGroup() != nullptr);
    EXPECT_TRUE(owner.GetGroup()->CheckRadius(3.0f));
    EXPECT_FLOAT_EQ(owner.GetGroup()->GetRadiusValue(), 3.0f);
    EXPECT_FALSE(owner.GetGroup()->HasName());

    std::unique_ptr<SparseGroup> clone = owner.CloneGroup();
    ASSERT_TRUE(clone != nullptr);
    clone->UpdateRadius(4.0f);
    EXPECT_FLOAT_EQ(owner.GetGroup()->GetRadiusValue(), 3.0f);

    owner.AssignGroup(std::move(clone));
    EXPECT_FLOAT_EQ(owner.GetGroup()->GetRadiusValue(), 4.0f);
    owner.AssignGroup(nullptr);
    EXPECT_TRUE(owner.GetGroup() == nullptr);

    owner.GetOrCreateGroup()->UpdateName(std::string("name"));
    owner.ResetGroup();
    EXPECT_TRUE(owner.GetGroup() == nullptr);
}

/**
 * @tc.name: PropertyGroupCopy001
 * @tc.desc: A group struct is copied as a whole: the copy is independent of the original
 * @tc.type: FUNC
 */
HWTEST_F(SparsePropertyTest, PropertyGroupCopy001, TestSize.Level1)
{
    SparseGroup original;
    original.UpdateRadius(1.0f);
    original.UpdateName(std::string("a"));

    SparseGroup copy = original;
    copy.UpdateName(std::string("b"));
    EXPECT_EQ(original.GetNameValue(), "a");
    EXPECT_EQ(copy.GetNameValue(), "b");
    EXPECT_FLOAT_EQ(copy.GetRadiusValue(), 1.0f);

    SparseGroup assigned;
    assigned.UpdateRadius(9.0f);
    assigned = original;
    EXPECT_FLOAT_EQ(assigned.GetRadiusValue(), 1.0f);
    assigned.ResetRadius();
    EXPECT_FALSE(assigned.HasRadius());
    EXPECT_TRUE(original.HasRadius());

    EXPECT_FALSE(original.UpdateName(std::optional<std::string>()));
    EXPECT_EQ(original.GetNameValue(), "a");
}

/**
 * @tc.name: FieldsAndProperties001
 * @tc.desc: F fields and P properties of one list share the storage without affecting each other
 * @tc.type: FUNC
 */
HWTEST_F(SparsePropertyTest, FieldsAndProperties001, TestSize.Level1)
{
    SparsePropOwner owner;
    owner.EmplaceNote("note");
    owner.UpdateWidth(1.0f);
    owner.GetOrCreateGroup()->UpdateRadius(2.0f);
    owner.ResetWidth();
    EXPECT_EQ(*owner.GetNote(), "note");
    EXPECT_FLOAT_EQ(owner.GetGroup()->GetRadiusValue(), 2.0f);
    owner.ResetNote();
    EXPECT_FALSE(owner.HasNote());
    EXPECT_TRUE(owner.GetGroup() != nullptr);
}
} // namespace OHOS::Ace

// SparseRefPtr: LateType stays incomplete in this file, as in the users of a header (sparse_ref_ptr_late_type.h).
// An F(Late, RefPtr<LateType>) entry would not compile here, and neither would these accessors without the extern
// template declaration or with an inline special member of the holder.
namespace OHOS::Ace {
class SparseRefPtrTestNg : public testing::Test {
public:
    void SetUp() override
    {
        SparseRefPtrTest::ResetLateEvents();
    }
};

/**
 * @tc.name: SparseRefPtr001
 * @tc.desc: A SparseRefPtr field is absent until emplaced, starts empty, holds the object it is given and releases
 *           it on reset.
 * @tc.type: FUNC
 */
HWTEST_F(SparseRefPtrTestNg, SparseRefPtr001, TestSize.Level1)
{
    SparseRefPtrTest::LateHolder holder;
    EXPECT_FALSE(holder.HasLate());
    auto& late = holder.GetOrEmplaceLate();
    EXPECT_EQ(SparseRefPtrTest::GetLateAddress(late), nullptr);
    SparseRefPtrTest::SetNewLate(late, 7, nullptr);
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 1);
    ASSERT_TRUE(holder.HasLate());
    EXPECT_EQ(SparseRefPtrTest::GetLateValue(*holder.GetLate()), 7);
    EXPECT_EQ(&holder.GetOrEmplaceLate(), &late);

    holder.ResetLate();
    EXPECT_FALSE(holder.HasLate());
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 0);
}

/**
 * @tc.name: SparseRefPtr002
 * @tc.desc: Destroying the owner releases its reference: the object dies with the last reference, not earlier.
 * @tc.type: FUNC
 */
HWTEST_F(SparseRefPtrTestNg, SparseRefPtr002, TestSize.Level1)
{
    {
        SparseRefPtrTest::LateHolder holder;
        SparseRefPtrTest::SetNewLate(holder.EmplaceLate(), 1, nullptr);
        SparseRefPtrTest::KeepLate(*holder.GetLate());
    }
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 1);
    SparseRefPtrTest::DropKeptLate();
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 0);
    {
        SparseRefPtrTest::LateHolder holder;
        SparseRefPtrTest::SetNewLate(holder.EmplaceLate(), 2, nullptr);
        EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 1);
    }
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 0);
}

/**
 * @tc.name: SparseRefPtr003
 * @tc.desc: The old object is released after the field changes, as with a RefPtr field: on a new value its destructor
 *           finds the field holding the new object, on reset - absent.
 * @tc.type: FUNC
 */
HWTEST_F(SparseRefPtrTestNg, SparseRefPtr003, TestSize.Level1)
{
    SparseRefPtrTest::LateHolder holder;
    SparseRefPtrTest::SetNewLate(holder.EmplaceLate(), 1, &holder);
    SparseRefPtr<SparseRefPtrTest::LateType> next;
    SparseRefPtrTest::SetNewLate(next, 2, &holder);
    const void* nextAddress = SparseRefPtrTest::GetLateAddress(next);
    holder.EmplaceLate(std::move(next));
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 1);
    EXPECT_TRUE(SparseRefPtrTest::GetLateEvents().seenSet);
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().seenValue, nextAddress);
    ASSERT_TRUE(holder.HasLate());
    EXPECT_EQ(SparseRefPtrTest::GetLateAddress(*holder.GetLate()), nextAddress);

    holder.ResetLate();
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().alive, 0);
    EXPECT_FALSE(SparseRefPtrTest::GetLateEvents().seenSet);
    EXPECT_EQ(SparseRefPtrTest::GetLateEvents().seenValue, nullptr);
}
} // namespace OHOS::Ace
