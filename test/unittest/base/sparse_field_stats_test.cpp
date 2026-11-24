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

// The sparse storage in the diagnostic build: BUILD.gn compiles this file with ACE_SPARSE_FIELDS_STATS, so the
// macros of sparse_fields.h and sparse_property.h are checked in that configuration too. The libraries linked
// into the binary are built without it: the schemas here stay local to this file, and no product header with
// sparse fields may be included, or one class would get two definitions.

#include <cstdint>
#include <map>
#include <string>

#include "gtest/gtest.h"

#include "base/utils/sparse_field_stats.h"
#include "base/utils/sparse_fields.h"
#include "base/utils/sparse_property.h"
#include "base/utils/utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
// Hits of every counter since the previous flush, by key "Owner::field:op".
std::map<std::string, uint32_t> FlushCounters()
{
    std::map<std::string, uint32_t> hits;
    SparseFieldStats::FlushFrame(
        [&hits](const SparseFieldStats::Counter& counter, uint32_t n) { hits[counter.Key()] += n; });
    return hits;
}

// Hits of the counter whose key ends with "Class::field:op": the owner is taken from __PRETTY_FUNCTION__ with
// its namespaces.
uint32_t HitsOf(const std::map<std::string, uint32_t>& hits, const std::string& suffix)
{
    for (const auto& [key, n] : hits) {
        if (key.size() >= suffix.size() && key.compare(key.size() - suffix.size(), suffix.size(), suffix) == 0) {
            return n;
        }
    }
    return 0;
}

#define STATS_NODE_SPARSE_FIELDS(F) F(Tag, std::string) F(Order, int32_t)

class StatsNode {
public:
    SPARSE_FIELDS(STATS_NODE_SPARSE_FIELDS);
};

#define STATS_GROUP_SPARSE_FIELDS(F, P) P(Radius, float)

struct StatsGroup {
    SPARSE_FIELDS_GROUP(STATS_GROUP_SPARSE_FIELDS);
    ACE_DEFINE_SPARSE_PROPERTY_GROUP_ITEM(Radius, float);
};

#define STATS_OWNER_SPARSE_FIELDS(F, P) \
    F(Note, std::string)                \
    P(Width, float)                     \
    P(Group, StatsGroup)

class StatsOwner {
    SPARSE_FIELDS_WITH_PROPS(STATS_OWNER_SPARSE_FIELDS);

public:
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_FUNC(Width, float);
    ACE_DEFINE_SPARSE_PROPERTY_GROUP(Group, StatsGroup);

public:
    void OnWidthUpdate(float width)
    {
        lastWidth = width;
    }

    float lastWidth = 0.0f;
};
} // namespace

class SparseFieldStatsTest : public testing::Test {
protected:
    void SetUp() override
    {
        FlushCounters(); // hits left by other tests
    }
};

/**
 * @tc.name: CountsEveryAccess001
 * @tc.desc: Has/Get/Emplace/Reset of a field are counted per field and operation; a flush zeroes the counters
 * @tc.type: FUNC
 */
HWTEST_F(SparseFieldStatsTest, CountsEveryAccess001, TestSize.Level1)
{
    StatsNode node;
    EXPECT_FALSE(node.HasTag());
    node.EmplaceTag("tag");
    EXPECT_EQ(*node.GetTag(), "tag");
    const StatsNode& constNode = node;
    EXPECT_EQ(*constNode.GetTag(), "tag");
    node.ResetTag();

    const std::map<std::string, uint32_t> hits = FlushCounters();
    EXPECT_EQ(HitsOf(hits, "StatsNode::Tag:has"), 1u);
    EXPECT_EQ(HitsOf(hits, "StatsNode::Tag:emplace"), 1u);
    EXPECT_EQ(HitsOf(hits, "StatsNode::Tag:get"), 2u);
    EXPECT_EQ(HitsOf(hits, "StatsNode::Tag:reset"), 1u);
    EXPECT_TRUE(FlushCounters().empty());
}

/**
 * @tc.name: PropertiesAndGroups001
 * @tc.desc: Properties and groups count their accesses under their own names, in the owner's and in the
 *           group's storage
 * @tc.type: FUNC
 */
HWTEST_F(SparseFieldStatsTest, PropertiesAndGroups001, TestSize.Level1)
{
    StatsOwner owner;
    owner.UpdateWidth(10.0f);
    EXPECT_FLOAT_EQ(owner.lastWidth, 10.0f);
    owner.GetOrCreateGroup()->UpdateRadius(2.0f);
    EXPECT_FLOAT_EQ(owner.GetGroup()->GetRadiusValue(), 2.0f);

    const std::map<std::string, uint32_t> hits = FlushCounters();
    EXPECT_EQ(HitsOf(hits, "StatsOwner::Width:get"), 1u); // the NearEqual lookup of Update
    EXPECT_EQ(HitsOf(hits, "StatsOwner::Width:emplace"), 1u);
    EXPECT_EQ(HitsOf(hits, "StatsOwner::Group:emplace"), 1u);
    EXPECT_EQ(HitsOf(hits, "StatsGroup::Radius:emplace"), 1u);
}
} // namespace OHOS::Ace
