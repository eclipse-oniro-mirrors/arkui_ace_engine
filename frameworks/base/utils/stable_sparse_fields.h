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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_STABLE_SPARSE_FIELDS_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_STABLE_SPARSE_FIELDS_H

// StableSparseFields — sparse storage for rarely set fields with stable value addresses.
//
// Guarantee: a non-null pointer returned by Get<Id>() stays valid until Reset<Id>() of the same field
// or until the storage is destroyed. Adding and removing OTHER fields, Emplace of the same field (the
// new value is assigned in place) and moving the storage do not move it.
//
// Order of operations (re-entrancy): constructors and destructors of field values may run arbitrary code —
// a RefPtr releasing the last reference to a node, the captures of a callback — and that code may touch the
// same storage. It always finds the storage consistent:
//   * a new value is built before its slot is chosen and published: a field never reads as set while its
//     value is under construction, and the free slot cannot be taken by a nested Emplace meanwhile;
//   * replacing and removing publish first, destroy last, like RefPtr::Reset / RefPtr::operator=,
//     std::function = nullptr and unique_ptr::reset: while the old value is destroyed the field already
//     holds the new value (Emplace, copy, move assignment) or reads as absent (Reset, Clear), so a
//     destructor never sees its own half-destroyed value and cannot destroy it twice.
// Code of a value runs while its field is between these states in three cases, and that code must not use the same
// storage:
//   * a value that lives in its slot is move-constructed there after the slot is chosen and before it is
//     published, and out of it on Reset — its move constructor;
//   * a value of a type without a move constructor that lives in its slot is constructed in the free slot and
//     destroyed in place on Reset — its constructors and destructor;
//   * a set field of a type without a move constructor or without move assignment gets a new value by destroying
//     the old one in place and building the new one there — its destructor and constructors.
// RefPtr, SparseRefPtr, std::optional, std::string and the standard containers are movable and move-assignable,
// and their moves never use the storage.
//
// Interface:
//   bool Has<Id>() const
//   const T* Get<Id>() const / T* Get<Id>()      — nullptr if the field is not set; edit in place
//   T& Emplace<Id>(args...)                       — create (or recreate in place) from arguments
//   void Reset<Id>()                              — remove the field
//
// Template parameters:
//   kSegSlots — slots per block: 8 — one block of 8 slots (128 bytes); 4 — blocks of 4 slots (64 bytes),
//               the second one is allocated when more than four fields are set;
//   kMask     — keep a mask of set fields in the object (8 bytes). Without it "is the field set?" is
//               answered from the slot-number word and the object is 16 bytes. On by default: without
//               it a lookup of an absent field is noticeably slower.
//
// Layout:
//   * in the object: a word of field numbers for 8 slots (one byte per slot, 0xFF — free), a pointer
//     and, with kMask, the mask. "Is the field set?" and "which slot?" are answered from the object's
//     own data — the heap is touched exactly once, for the value itself. No blocks while no field is set;
//   * blocks are slots only, exactly 64 or 128 bytes: no service fields, a round allocator size class.
//     They are allocated once and are never reallocated or moved;
//   * the pointer in the object is tagged in the low bit: 0 — points to the first block (common case);
//     1 — points to an Ext service object with block references and the list of fields beyond 8 slots.
//     Ext appears only with the second block or on overflow; blocks stay in place;
//   * slot search is SWAR "find a byte in a word": a few integer instructions, no popcount;
//   * values up to 16 bytes live in the slot itself, even non-trivial ones: a published value is never
//     moved (it is move-constructed into the slot once, before publication, and out of it on Reset).
//     Larger values live in their own allocation with a pointer in the slot. Fields beyond 8 slots live
//     in their own allocations; their list (number + pointer) may grow — entries move, values do not;
//   * blocks and value allocations come from malloc, so a field type may be aligned no stricter than
//     std::max_align_t (8 bytes on arm32, 16 on arm64); a stricter one does not compile;
//   * Reset frees the slot without compaction; the next new field reuses it. When no fields are left,
//     the blocks are freed;
//   * under ASan free slots are poisoned: access through a pointer after Reset fails immediately.
//
// Schema is a struct with `static constexpr uint32_t kFields` (<= 64 with kMask, otherwise <= 254) and
// `template <uint32_t Id> using Type`. Limitations: no implicit copying — CopyFrom() copies explicitly,
// CopyableStableSparseFields makes it the copy constructor; exception safety only through the order of
// operations. With ACE_SPARSE_FIELDS_STATS every Has/Get/Emplace/Reset is counted (sparse_field_stats.h).

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

#include "base/utils/sparse_field_stats.h"

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define SSF_ASAN 1
#endif
#endif
#if defined(__SANITIZE_ADDRESS__) && !defined(SSF_ASAN)
#define SSF_ASAN 1
#endif
#ifdef SSF_ASAN
#include <sanitizer/asan_interface.h>
#endif

namespace OHOS::Ace {
namespace SparseFieldsDetail {
// Under ASan free slots and unused blocks are poisoned; without ASan both calls compile to nothing.
#ifdef SSF_ASAN
inline void Poison(const void* p, size_t n)
{
    ASAN_POISON_MEMORY_REGION(p, n);
}
inline void Unpoison(const void* p, size_t n)
{
    ASAN_UNPOISON_MEMORY_REGION(p, n);
}
#else
inline void Poison(const void*, size_t) {}
inline void Unpoison(const void*, size_t) {}
#endif

// The mask lives in the object only when it is needed: an empty base takes no space.
template<bool kMask>
struct MaskHolder {
    uint64_t mask_ = 0;
};
template<>
struct MaskHolder<false> {};
} // namespace SparseFieldsDetail

template<class Schema, uint32_t kSegSlots = 8, bool kMask = true>
class StableSparseFields : private SparseFieldsDetail::MaskHolder<kMask> {
    static_assert(kSegSlots == 4 || kSegSlots == 8, "a block holds 4 or 8 slots");
    static_assert(!kMask || Schema::kFields <= 64, "the mask is 64-bit");
    static_assert(Schema::kFields < 0xFF, "a field number is a byte, 0xFF marks a free slot");

public:
    static constexpr uint32_t kFields = Schema::kFields;
    static constexpr uint32_t kSlots = 8; // the slot-number word addresses this many slots
    static constexpr uint32_t kSlotBytes = 16;

    template<uint32_t Id>
    using T = typename Schema::template Type<Id>;

    StableSparseFields() = default;
    StableSparseFields(const StableSparseFields&) = delete;
    StableSparseFields& operator=(const StableSparseFields&) = delete;
    StableSparseFields(StableSparseFields&& other) noexcept
    {
        TakeFrom(other);
    }
    StableSparseFields& operator=(StableSparseFields&& other) noexcept
    {
        if (this != &other) {
            StableSparseFields old; // replace first, destroy the old values last
            old.TakeFrom(*this);
            TakeFrom(other);
        }
        return *this;
    }
    ~StableSparseFields()
    {
        Clear();
    }

    template<uint32_t Id>
    bool Has() const
    {
        SparseFieldStats::Count<Schema, Id, SparseFieldStats::OP_HAS>();
        if constexpr (kMask) {
            return (this->mask_ & Bit(Id)) != 0;
        } else {
            return Lookup<Id>() != nullptr;
        }
    }

    template<uint32_t Id>
    const T<Id>* Get() const
    {
        SparseFieldStats::Count<Schema, Id, SparseFieldStats::OP_GET>();
        return Lookup<Id>();
    }

    template<uint32_t Id>
    T<Id>* Get()
    {
        SparseFieldStats::Count<Schema, Id, SparseFieldStats::OP_GET>();
        return Lookup<Id>();
    }

    // Creates the field from arguments, or gives a set field a new value (assigned in place: pointers to it stay
    // valid). The value is always built first — the arguments may refer to the old value, the constructor may use
    // the storage — and only then put into the field.
    template<uint32_t Id, class... Args>
    T<Id>& Emplace(Args&&... args)
    {
        SparseFieldStats::Count<Schema, Id, SparseFieldStats::OP_EMPLACE>();
        if constexpr (!std::is_move_constructible_v<T<Id>>) {
            return EmplaceInPlace<Id>(std::forward<Args>(args)...);
        } else if constexpr (kInline<Id>) {
            T<Id> value(std::forward<Args>(args)...);
            return Put<Id>(std::move(value));
        } else {
            if (Lookup<Id>() != nullptr) {
                T<Id> value(std::forward<Args>(args)...);
                return Put<Id>(std::move(value));
            }
            // A new large value is built right in its own allocation: unreachable until it is published.
            T<Id>* box = ::new (AllocBox<Id>()) T<Id>(std::forward<Args>(args)...);
            if (T<Id>* p = Lookup<Id>()) { // the constructor has set the field meanwhile
                T<Id>& result = Replace<Id>(p, std::move(*box));
                DestroyBox<Id>(box);
                return result;
            }
            PublishBox<Id>(box);
            return *box;
        }
    }

    // Removes the field. The field is unpublished first and the old value is destroyed last, when the storage is
    // consistent again: its destructor finds the field absent (as with RefPtr::Reset). The one exception — a type
    // without a move constructor that lives in its slot — is in ResetSlot().
    template<uint32_t Id>
    void Reset()
    {
        SparseFieldStats::Count<Schema, Id, SparseFieldStats::OP_RESET>();
        if constexpr (kMask) {
            if (!(this->mask_ & Bit(Id))) {
                return;
            }
        }
        const uint32_t s = FindSlot(ids_, Id);
        if (s < kSlots) {
            ResetSlot<Id>(s);
            return;
        }
        if (!HasExt()) {
            return; // the field is not set (never reached with the mask)
        }
        void* box = RemoveOverflow(GetExt(), Id);
        if (box == nullptr) {
            return; // the field is not set
        }
        if constexpr (kMask) {
            this->mask_ &= ~Bit(Id);
        }
        ReleaseIfEmpty();
        DestroyBox<Id>(box);
    }

    // Replaces the contents with copies of every field set in `other`. Compiled only where it is called,
    // and only if all field types are copyable. Addresses of the copies are new; `other` is untouched.
    // The copies are complete before anything here changes; the old values are destroyed last.
    void CopyFrom(const StableSparseFields& other)
    {
        if (this == &other) {
            return;
        }
        StableSparseFields copy;
        copy.CopyFields(other, std::make_index_sequence<kFields> {});
        StableSparseFields old;
        old.TakeFrom(*this);
        TakeFrom(copy);
    }

    bool Empty() const
    {
        if constexpr (kMask) {
            return this->mask_ == 0;
        } else {
            return ids_ == kAllFree && (!HasExt() || GetExt()->count == 0);
        }
    }

    // All heap bytes held by the storage, measured by `usable` (e.g. malloc_usable_size).
    template<class UsableFn>
    size_t HeapBytes(UsableFn usable) const
    {
        if (ptr_ == 0) {
            return 0;
        }
        size_t total = 0;
        if (HasExt()) {
            const Ext* e = GetExt();
            total += usable(e) + usable(e->entries);
            for (uint32_t g = 0; g < kSegs; ++g) {
                total += usable(e->seg[g]);
            }
            for (uint32_t i = 0; i < e->count; ++i) {
                total += usable(e->entries[i].box);
            }
        } else {
            total += usable(reinterpret_cast<const Seg*>(ptr_));
        }
        for (uint32_t s = 0; s < kSlots; ++s) {
            const uint32_t id = IdAt(s);
            if (id != kFree && !kInlineTable[id]) {
                void* box = LoadPtr(SlotAddr(s));
                total += usable(box);
            }
        }
        return total;
    }

private:
    static constexpr uint32_t kIdBits = 8;     // a field number in ids_ takes one byte per slot
    static constexpr uint32_t kIdMask = 0xFF;  // the bits of one field number
    static constexpr uint32_t kFree = kIdMask; // the field number of a free slot
    static constexpr uint64_t kAllFree = ~uint64_t(0);
    static constexpr uint32_t kSegs = kSlots / kSegSlots; // 1 or 2 blocks
    static constexpr uintptr_t kExtTag = 1;

    // A block comes from malloc, aligned to std::max_align_t, and a slot offset is a multiple of 16 bytes: a slot
    // is aligned as its block. A stricter type goes to the AllocBox path, which rejects it at compile time.
    template<uint32_t Id>
    static constexpr bool kInline = sizeof(T<Id>) <= kSlotBytes && alignof(T<Id>) <= alignof(std::max_align_t);

    struct alignas(std::max_align_t) Seg {
        unsigned char slots[kSegSlots][kSlotBytes];
    };
    static_assert(sizeof(Seg) == kSegSlots * kSlotBytes, "a block is exactly its slots, no service fields");

    struct OverflowEntry {
        uint32_t id;
        void* box;
    };
    // Service object: appears with the second block or on overflow. Never reallocated.
    struct Ext {
        Seg* seg[kSegs];
        OverflowEntry* entries; // fields beyond 8 slots; the array may grow — entries move, values do not
        uint32_t count;
        uint32_t cap;
    };

    static constexpr uint64_t Bit(uint32_t id)
    {
        return uint64_t(1) << id;
    }

    // Slot holding byte `id`, or kSlots. SWAR: zero bytes of (ids ^ id * 0x0101..) are found exactly, and every
    // field number occurs at most once.
    static uint32_t FindSlot(uint64_t ids, uint32_t id)
    {
        constexpr uint64_t kOnes = 0x0101010101010101ull;
        constexpr uint64_t kLow7 = 0x7F7F7F7F7F7F7F7Full;
        const uint64_t x = ids ^ (kOnes * id);
        // 0x80 in every zero byte of x, 0 elsewhere. No carries between bytes, so no unsigned wraparound:
        // OHOS builds ace with integer_overflow sanitizer, which traps on it.
        const uint64_t z = ~(((x & kLow7) + kLow7) | x | kLow7);
        if (z == 0) {
            return kSlots;
        }
        return static_cast<uint32_t>(__builtin_ctzll(z)) / kIdBits;
    }

    uint32_t IdAt(uint32_t s) const
    {
        return static_cast<uint32_t>((ids_ >> (kIdBits * s)) & kIdMask);
    }

    void SetId(uint32_t s, uint32_t id)
    {
        const uint32_t shift = kIdBits * s;
        ids_ = (ids_ & ~(uint64_t(kIdMask) << shift)) | (uint64_t(id) << shift);
    }

    bool HasExt() const
    {
        return (ptr_ & kExtTag) != 0;
    }
    Ext* GetExt() const
    {
        return reinterpret_cast<Ext*>(ptr_ & ~kExtTag);
    }

    // Address of slot s. Common case: ptr_ points directly to the first block.
    unsigned char* SlotAddr(uint32_t s) const
    {
        return SlotAddrOf(ptr_, s);
    }
    static unsigned char* SlotAddrOf(uintptr_t ptr, uint32_t s)
    {
        if (__builtin_expect(!(ptr & kExtTag), 1)) {
            return reinterpret_cast<Seg*>(ptr)->slots[s]; // without Ext only the first block is used
        }
        Seg* seg = reinterpret_cast<Ext*>(ptr & ~kExtTag)->seg[s / kSegSlots];
        return seg->slots[s % kSegSlots];
    }

    // The same for writing into a free slot: allocates the second block if the slot is in it.
    unsigned char* SlotForWrite(uint32_t s)
    {
        if constexpr (kSegs == 2) {
            if (s >= kSegSlots) {
                Ext* e = EnsureExt();
                if (e->seg[1] == nullptr) {
                    e->seg[1] = NewSeg();
                }
            }
        }
        return SlotAddr(s);
    }

    // A box pointer lives in its slot as an object of its own, built there by placement new like a small value.
    using BoxPtr = void*;
    static void* LoadPtr(const unsigned char* slot)
    {
        return *std::launder(reinterpret_cast<const BoxPtr*>(slot));
    }
    static void StorePtr(unsigned char* slot, void* box)
    {
        ::new (static_cast<void*>(slot)) BoxPtr(box);
    }

    Ext* EnsureExt()
    {
        if (HasExt()) {
            return GetExt();
        }
        void* raw = std::malloc(sizeof(Ext));
        if (raw == nullptr) {
            std::abort();
        }
        Ext* e = static_cast<Ext*>(raw);
        e->seg[0] = reinterpret_cast<Seg*>(ptr_); // the first block stays in place
        for (uint32_t g = 1; g < kSegs; ++g) {
            e->seg[g] = nullptr;
        }
        e->entries = nullptr;
        e->count = 0;
        e->cap = 0;
        ptr_ = reinterpret_cast<uintptr_t>(e) | kExtTag;
        return e;
    }

    template<uint32_t Id>
    T<Id>* Lookup() const
    {
        if constexpr (kMask) {
            if (!(this->mask_ & Bit(Id))) {
                return nullptr;
            }
        }
        const uint32_t s = FindSlot(ids_, Id); // from the object's own data, blocks are not read
        if (__builtin_expect(s < kSlots, 1)) {
            unsigned char* slot = SlotAddr(s);
            if constexpr (kInline<Id>) {
                return std::launder(reinterpret_cast<T<Id>*>(slot));
            } else {
                return static_cast<T<Id>*>(LoadPtr(slot));
            }
        }
        if (!HasExt()) {
            return nullptr; // the field is not set (never reached with the mask)
        }
        return static_cast<T<Id>*>(FindOverflow(GetExt(), Id));
    }

    static Seg* NewSeg()
    {
        void* raw = std::malloc(sizeof(Seg));
        if (raw == nullptr) {
            std::abort();
        }
        SparseFieldsDetail::Poison(raw, sizeof(Seg));
        return static_cast<Seg*>(raw);
    }

    static void FreeSeg(Seg* seg)
    {
        if (seg != nullptr) {
            SparseFieldsDetail::Unpoison(seg, sizeof(Seg));
            std::free(seg);
        }
    }

    template<uint32_t Id>
    static void* AllocBox()
    {
        static_assert(alignof(T<Id>) <= alignof(std::max_align_t), "over-aligned types are not supported");
        void* p = std::malloc(sizeof(T<Id>));
        if (p == nullptr) {
            std::abort();
        }
        return p;
    }

    template<uint32_t Id>
    static void DestroyBox(void* box)
    {
        static_cast<T<Id>*>(box)->~T<Id>();
        std::free(box);
    }

    template<uint32_t Id>
    static void DestroySlot(unsigned char* slot)
    {
        if constexpr (kInline<Id>) {
            std::launder(reinterpret_cast<T<Id>*>(slot))->~T<Id>();
        } else {
            DestroyBox<Id>(LoadPtr(slot));
        }
    }

    // Puts a built value into the field: assigned to the field if it is set, otherwise moved into a free slot (or
    // its own allocation) that is published only after that. The move constructor of a small value runs in the
    // chosen free slot, so it must not use the same storage (see the header).
    template<uint32_t Id>
    T<Id>& Put(T<Id>&& value)
    {
        if (T<Id>* p = Lookup<Id>()) {
            return Replace<Id>(p, std::move(value));
        }
        if constexpr (kInline<Id>) {
            if (ptr_ == 0) {
                ptr_ = reinterpret_cast<uintptr_t>(NewSeg());
            }
            const uint32_t s = FindSlot(ids_, kFree);
            if (s < kSlots) {
                unsigned char* slot = SlotForWrite(s);
                SparseFieldsDetail::Unpoison(slot, kSlotBytes);
                T<Id>* obj = ::new (static_cast<void*>(slot)) T<Id>(std::move(value));
                SetId(s, Id);
                if constexpr (kMask) {
                    this->mask_ |= Bit(Id);
                }
                return *obj;
            }
        }
        T<Id>* box = ::new (AllocBox<Id>()) T<Id>(std::move(value));
        PublishBox<Id>(box);
        return *box;
    }

    // A set field gets a new value through the type's own assignment (RefPtr swaps: the old reference is
    // released after the field holds the new one). Types without move assignment are rebuilt in place.
    template<uint32_t Id>
    static T<Id>& Replace(T<Id>* p, T<Id>&& value)
    {
        if constexpr (std::is_move_assignable_v<T<Id>>) {
            *p = std::move(value);
            return *p;
        } else {
            p->~T<Id>();
            return *::new (static_cast<void*>(p)) T<Id>(std::move(value));
        }
    }

    // Publishes a value built in its own allocation: a pointer in a free slot, or an overflow entry.
    template<uint32_t Id>
    void PublishBox(T<Id>* box)
    {
        if (ptr_ == 0) {
            ptr_ = reinterpret_cast<uintptr_t>(NewSeg());
        }
        const uint32_t s = FindSlot(ids_, kFree);
        if (s < kSlots) {
            unsigned char* slot = SlotForWrite(s);
            SparseFieldsDetail::Unpoison(slot, kSlotBytes);
            StorePtr(slot, box);
            SetId(s, Id);
        } else {
            AppendOverflow(EnsureExt(), Id, box);
        }
        if constexpr (kMask) {
            this->mask_ |= Bit(Id);
        }
    }

    // Emplace for types without a move constructor: the old value is destroyed and the new one is built in its place.
    // A new large value is built in its own allocation before it is published, as in Emplace(); a new small one can
    // only be built in a free slot, so its constructor must not use the same storage.
    template<uint32_t Id, class... Args>
    T<Id>& EmplaceInPlace(Args&&... args)
    {
        if (T<Id>* p = Lookup<Id>()) {
            p->~T<Id>();
            return *::new (static_cast<void*>(p)) T<Id>(std::forward<Args>(args)...);
        }
        if constexpr (!kInline<Id>) {
            T<Id>* box = ::new (AllocBox<Id>()) T<Id>(std::forward<Args>(args)...);
            // The constructor has set the field meanwhile, or the destructor of that value sets it again: the new
            // value wins, as in Emplace()
            while (Lookup<Id>() != nullptr) {
                Reset<Id>();
            }
            PublishBox<Id>(box);
            return *box;
        } else {
            if (ptr_ == 0) {
                ptr_ = reinterpret_cast<uintptr_t>(NewSeg());
            }
            const uint32_t s = FindSlot(ids_, kFree);
            T<Id>* obj;
            if (s < kSlots) {
                unsigned char* slot = SlotForWrite(s);
                SparseFieldsDetail::Unpoison(slot, kSlotBytes);
                obj = ::new (static_cast<void*>(slot)) T<Id>(std::forward<Args>(args)...);
                SetId(s, Id);
            } else {
                obj = ::new (AllocBox<Id>()) T<Id>(std::forward<Args>(args)...);
                AppendOverflow(EnsureExt(), Id, obj);
            }
            if constexpr (kMask) {
                this->mask_ |= Bit(Id);
            }
            return *obj;
        }
    }

    void Unpublish(uint32_t s, uint32_t id)
    {
        SetId(s, kFree);
        if constexpr (kMask) {
            this->mask_ &= ~Bit(id);
        }
    }

    void ReleaseIfEmpty()
    {
        if (Empty()) {
            Clear();
        }
    }

    // Removes a field that lives in slot s. A large value and a movable small one are destroyed after the slot
    // is released, so their destructor finds the field absent; a small type without a move constructor can only
    // be destroyed in place, before that.
    template<uint32_t Id>
    void ResetSlot(uint32_t s)
    {
        unsigned char* slot = SlotAddr(s);
        if constexpr (!kInline<Id>) {
            void* box = LoadPtr(slot);
            ReleaseSlot(s, Id, slot);
            DestroyBox<Id>(box);
        } else if constexpr (std::is_trivially_destructible_v<T<Id>>) {
            ReleaseSlot(s, Id, slot); // nothing to destroy
        } else if constexpr (std::is_move_constructible_v<T<Id>>) {
            T<Id>* p = std::launder(reinterpret_cast<T<Id>*>(slot));
            [[maybe_unused]] T<Id> old(std::move(*p)); // the slot keeps a moved-from value: empty RefPtr...
            p->~T<Id>();
            ReleaseSlot(s, Id, slot);
            // `old` is destroyed on return
        } else {
            DestroySlot<Id>(slot);
            ReleaseSlot(s, Id, slot);
        }
    }

    // Marks slot s free, poisons it and frees the blocks if no field is left.
    void ReleaseSlot(uint32_t s, uint32_t id, unsigned char* slot)
    {
        Unpublish(s, id);
        SparseFieldsDetail::Poison(slot, kSlotBytes);
        ReleaseIfEmpty();
    }

    static void AppendOverflow(Ext* e, uint32_t id, void* box)
    {
        if (e->count == e->cap) {
            const uint32_t cap = e->cap ? e->cap * 2 : 2;
            auto* n = static_cast<OverflowEntry*>(std::realloc(e->entries, cap * sizeof(OverflowEntry)));
            if (n == nullptr) {
                std::abort();
            }
            e->entries = n;
            e->cap = cap;
        }
        e->entries[e->count++] = OverflowEntry { id, box };
    }

    static void* FindOverflow(const Ext* e, uint32_t id)
    {
        for (uint32_t i = 0; i < e->count; ++i) {
            if (e->entries[i].id == id) {
                return e->entries[i].box;
            }
        }
        return nullptr;
    }

    static void* RemoveOverflow(Ext* e, uint32_t id)
    {
        for (uint32_t i = 0; i < e->count; ++i) {
            if (e->entries[i].id == id) {
                void* box = e->entries[i].box;
                e->entries[i] = e->entries[--e->count]; // entry order does not matter
                return box;
            }
        }
        return nullptr;
    }

    // Tables by field number — for operations where the number is known only at run time.
    using DestroySlotFn = void (*)(unsigned char*);
    using DestroyBoxFn = void (*)(void*);
    template<size_t... I>
    static constexpr std::array<DestroySlotFn, kFields> MakeDestroySlot(std::index_sequence<I...>)
    {
        return { { &DestroySlot<I>... } };
    }
    template<size_t... I>
    static constexpr std::array<DestroyBoxFn, kFields> MakeDestroyBox(std::index_sequence<I...>)
    {
        return { { &DestroyBox<I>... } };
    }
    template<size_t... I>
    static constexpr std::array<bool, kFields> MakeInline(std::index_sequence<I...>)
    {
        return { { kInline<I>... } };
    }
    static constexpr auto kDestroySlotTable = MakeDestroySlot(std::make_index_sequence<kFields> {});
    static constexpr auto kDestroyBoxTable = MakeDestroyBox(std::make_index_sequence<kFields> {});
    static constexpr auto kInlineTable = MakeInline(std::make_index_sequence<kFields> {});

    template<size_t... I>
    void CopyFields(const StableSparseFields& other, std::index_sequence<I...>)
    {
        (CopyField<I>(other), ...);
    }

    template<uint32_t Id>
    void CopyField(const StableSparseFields& other)
    {
        if (const T<Id>* p = other.template Lookup<Id>()) {
            Emplace<Id>(*p);
        }
    }

    void TakeFrom(StableSparseFields& other)
    {
        if constexpr (kMask) {
            this->mask_ = other.mask_;
            other.mask_ = 0;
        }
        ids_ = other.ids_;
        ptr_ = other.ptr_;
        other.ids_ = kAllFree;
        other.ptr_ = 0;
    }

    // Detaches the contents first and destroys them after: while the values are destroyed the storage already
    // reads as empty, and a destructor that resets its own field cannot destroy the value twice.
    void Clear()
    {
        if (ptr_ == 0) {
            return;
        }
        const uint64_t ids = ids_;
        const uintptr_t ptr = ptr_;
        ptr_ = 0;
        ids_ = kAllFree;
        if constexpr (kMask) {
            this->mask_ = 0;
        }
        for (uint32_t s = 0; s < kSlots; ++s) {
            const uint32_t id = static_cast<uint32_t>((ids >> (kIdBits * s)) & kIdMask);
            if (id != kFree) {
                kDestroySlotTable[id](SlotAddrOf(ptr, s));
            }
        }
        if (ptr & kExtTag) {
            Ext* e = reinterpret_cast<Ext*>(ptr & ~kExtTag);
            for (uint32_t i = 0; i < e->count; ++i) {
                kDestroyBoxTable[e->entries[i].id](e->entries[i].box);
            }
            std::free(e->entries);
            for (uint32_t g = 0; g < kSegs; ++g) {
                FreeSeg(e->seg[g]);
            }
            std::free(e);
        } else {
            FreeSeg(reinterpret_cast<Seg*>(ptr));
        }
    }

    uint64_t ids_ = kAllFree; // field number in each of 8 slots, 0xFF — the slot is free
    uintptr_t ptr_ = 0;       // 0 — no blocks; tag 0 — first block; tag 1 — Ext
};

// The same storage, copyable through CopyFrom — for value structs (property groups) that are copied as a
// whole: the struct keeps its implicit copy constructor and copy assignment for all members.
template<class Schema, uint32_t kSegSlots = 8, bool kMask = true>
class CopyableStableSparseFields : public StableSparseFields<Schema, kSegSlots, kMask> {
    using Base = StableSparseFields<Schema, kSegSlots, kMask>;

public:
    CopyableStableSparseFields() = default;
    CopyableStableSparseFields(const CopyableStableSparseFields& other) : Base()
    {
        this->CopyFrom(other);
    }
    CopyableStableSparseFields& operator=(const CopyableStableSparseFields& other)
    {
        this->CopyFrom(other);
        return *this;
    }
    CopyableStableSparseFields(CopyableStableSparseFields&& other) noexcept = default;
    CopyableStableSparseFields& operator=(CopyableStableSparseFields&& other) noexcept = default;
    ~CopyableStableSparseFields() = default;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_STABLE_SPARSE_FIELDS_H
