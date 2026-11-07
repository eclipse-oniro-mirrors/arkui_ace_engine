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

#include <atomic>
#include <memory>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "ui/properties/ui_material.h"
#include "ui/properties/ui_material_structs.h"

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/ui_material.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/interfaces/arkoala/arkoala_api.h"
#include "core/pipeline_ng/pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {

static int32_t g_setMaterialCallCount = 0;
static void* g_lastSetMaterialNode = nullptr;
static const void* g_lastSetMaterialMaterial = nullptr;
static bool MockSetMaterial(void* node, const void* material)
{
    g_setMaterialCallCount++;
    g_lastSetMaterialNode = node;
    g_lastSetMaterialMaterial = material;
    return true;
}

constexpr int32_t MOCK_MATERIAL_ID = 42;

static int32_t g_getMaterialIdCallCount = 0;
static int32_t MockGetMaterialId(const void* material)
{
    g_getMaterialIdCallCount++;
    return MOCK_MATERIAL_ID;
}

static int32_t g_getEnableColorInvertCallCount = 0;
static int32_t MockGetEnableColorInvert(int32_t style, int32_t transparency)
{
    g_getEnableColorInvertCallCount++;
    return 1;
}

ArkUIMaterialFuncRegistry g_funcRegistry = {
    .setMaterial = MockSetMaterial,
    .getMaterialId = MockGetMaterialId,
};

ArkUIMaterialFuncRegistry g_nullFuncRegistry = {
    .setMaterial = nullptr,
    .getMaterialId = nullptr,
};

ArkUIMaterialFilterFuncRegistry g_filterRegistry = {};
ArkUIMaterialFilterFuncRegistry g_nullFilterRegistry = {};

static void InitFilterRegistry()
{
    g_filterRegistry.getEnableColorInvert = MockGetEnableColorInvert;
}

static void ResetCallCounters()
{
    g_setMaterialCallCount = 0;
    g_getMaterialIdCallCount = 0;
    g_getEnableColorInvertCallCount = 0;
    g_lastSetMaterialNode = nullptr;
    g_lastSetMaterialMaterial = nullptr;
}

static void ResetRegistries()
{
    MaterialUtils::materialFuncRegistry_.store(nullptr, std::memory_order_release);
    MaterialUtils::materialFilterFuncRegistry_.store(nullptr, std::memory_order_release);
}
} // namespace

class MaterialRegistryTestNg : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    bool savedMaterialSupported_ = true;
};

void MaterialRegistryTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> { return nullptr; });
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    InitFilterRegistry();
}

void MaterialRegistryTestNg::TearDownTestCase()
{
    ResetRegistries();
    MockPipelineContext::GetCurrent()->themeManager_ = nullptr;
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

void MaterialRegistryTestNg::SetUp()
{
    ResetCallCounters();
    ResetRegistries();
    savedMaterialSupported_ = g_isDeviceSystemMaterialSupported;
    g_isDeviceSystemMaterialSupported = true;
}

void MaterialRegistryTestNg::TearDown()
{
    g_isDeviceSystemMaterialSupported = savedMaterialSupported_;
    ResetCallCounters();
    ResetRegistries();
}

/**
 * @tc.name: RegisterMaterialFunc001
 * @tc.desc: Test RegisterMaterialFunc full lifecycle:
 *           null param no-op, valid registration sets atomic,
 *           second registration silently ignored (compare_exchange_strong)
 * @tc.type: FUNC
 */
HWTEST_F(MaterialRegistryTestNg, RegisterMaterialFunc001, TestSize.Level1)
{
    // 1. null param should be no-op
    MaterialUtils::RegisterMaterialFunc(nullptr);
    EXPECT_EQ(MaterialUtils::materialFuncRegistry_.load(std::memory_order_acquire), nullptr);

    // 2. valid registration should set atomic pointer
    MaterialUtils::RegisterMaterialFunc(&g_funcRegistry);
    auto* loaded = MaterialUtils::materialFuncRegistry_.load(std::memory_order_acquire);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded, &g_funcRegistry);
    EXPECT_EQ(loaded->setMaterial, MockSetMaterial);
    EXPECT_EQ(loaded->getMaterialId, MockGetMaterialId);

    // 3. second registration should be silently ignored (compare_exchange_strong)
    MaterialUtils::RegisterMaterialFunc(&g_nullFuncRegistry);
    auto* loadedAfterSecond = MaterialUtils::materialFuncRegistry_.load(std::memory_order_acquire);
    EXPECT_EQ(loadedAfterSecond, &g_funcRegistry);
    EXPECT_NE(loadedAfterSecond, &g_nullFuncRegistry);
    EXPECT_EQ(loadedAfterSecond->setMaterial, MockSetMaterial);
}

/**
 * @tc.name: RegisterMaterialFilterFunc001
 * @tc.desc: Test RegisterMaterialFilterFunc full lifecycle:
 *           null param no-op, valid registration sets atomic,
 *           second registration silently ignored
 * @tc.type: FUNC
 */
HWTEST_F(MaterialRegistryTestNg, RegisterMaterialFilterFunc001, TestSize.Level1)
{
    // 1. null param should be no-op
    MaterialUtils::RegisterMaterialFilterFunc(nullptr);
    EXPECT_EQ(MaterialUtils::materialFilterFuncRegistry_.load(std::memory_order_acquire), nullptr);

    // 2. valid registration should set atomic pointer
    MaterialUtils::RegisterMaterialFilterFunc(&g_filterRegistry);
    auto* loaded = MaterialUtils::materialFilterFuncRegistry_.load(std::memory_order_acquire);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded, &g_filterRegistry);
    EXPECT_EQ(loaded->getEnableColorInvert, MockGetEnableColorInvert);

    // 3. second registration should be silently ignored
    MaterialUtils::RegisterMaterialFilterFunc(&g_nullFilterRegistry);
    auto* loadedAfterSecond = MaterialUtils::materialFilterFuncRegistry_.load(std::memory_order_acquire);
    EXPECT_EQ(loadedAfterSecond, &g_filterRegistry);
    EXPECT_EQ(loadedAfterSecond->getEnableColorInvert, MockGetEnableColorInvert);
}

/**
 * @tc.name: CallSetMaterialDelayedRegistration001
 * @tc.desc: Test CallSetMaterial picks up registration even if the first call happened
 *           before registration (so is not preloaded). Proves static caching is wrong.
 * @tc.type: FUNC
 */
HWTEST_F(MaterialRegistryTestNg, CallSetMaterialDelayedRegistration001, TestSize.Level1)
{
    auto frameNode = FrameNode::CreateFrameNode("test", 1, AceType::MakeRefPtr<Pattern>());
    auto material = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR);
    ASSERT_NE(frameNode, nullptr);
    ASSERT_NE(material, nullptr);

    // 1. first call before registration -> false, mock not called
    EXPECT_FALSE(MaterialUtils::CallSetMaterial(AceType::RawPtr(frameNode), AceType::RawPtr(material)));
    EXPECT_EQ(g_setMaterialCallCount, 0);

    // 2. register after first call
    MaterialUtils::RegisterMaterialFunc(&g_funcRegistry);

    // 3. second call after registration -> true, mock called
    EXPECT_TRUE(MaterialUtils::CallSetMaterial(AceType::RawPtr(frameNode), AceType::RawPtr(material)));
    EXPECT_EQ(g_setMaterialCallCount, 1);
    EXPECT_EQ(g_lastSetMaterialNode, AceType::RawPtr(frameNode));
    EXPECT_EQ(g_lastSetMaterialMaterial, AceType::RawPtr(material));
}

/**
 * @tc.name: CallGetMaterialId001
 * @tc.desc: Test CallGetMaterialId: null material returns -1,
 *           no registration returns -1, with registration returns mock value
 * @tc.type: FUNC
 */
HWTEST_F(MaterialRegistryTestNg, CallGetMaterialId001, TestSize.Level1)
{
    // 1. null material -> -1, no crash
    EXPECT_EQ(MaterialUtils::CallGetMaterialId(nullptr), -1);

    // 2. no registration -> -1, mock not called
    auto material = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR);
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(MaterialUtils::CallGetMaterialId(AceType::RawPtr(material)), -1);
    EXPECT_EQ(g_getMaterialIdCallCount, 0);

    // 3. with registration -> returns mock value, mock called once
    MaterialUtils::RegisterMaterialFunc(&g_funcRegistry);
    EXPECT_EQ(MaterialUtils::CallGetMaterialId(AceType::RawPtr(material)), MOCK_MATERIAL_ID);
    EXPECT_EQ(g_getMaterialIdCallCount, 1);
}

/**
 * @tc.name: ValidColorInvertNoRegistration001
 * @tc.desc: Test ValidColorInvert with EXQUISITE level + colorInvert=true but no registration.
 *           Should use fallback path without crash (old code used dlopen, new code reads registry).
 * @tc.type: FUNC
 */
HWTEST_F(MaterialRegistryTestNg, ValidColorInvertNoRegistration001, TestSize.Level1)
{
    auto options = std::make_shared<ImmersiveOptions>();
    options->colorInvert = true;
    options->style = UiMaterialStyle::ULTRA_THIN;
    MaterialUtils::ValidColorInvert(options, UiMaterialLevel::EXQUISITE, UiMaterialTransparency::THIN);
    EXPECT_EQ(g_getEnableColorInvertCallCount, 0);
}

} // namespace OHOS::Ace::NG
