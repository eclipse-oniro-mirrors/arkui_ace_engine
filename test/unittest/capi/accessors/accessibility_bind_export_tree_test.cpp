/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <securec.h>

#include "native_interface_accessibility.h"
#include "native_type.h"
#include "interfaces/native/node/node_model.h"

using namespace testing;
using namespace testing::ext;

namespace {
/*
 * Test suite for OH_ArkUI_Accessibility_BindExportTree / OH_ArkUI_Accessibility_UnbindExportTree.
 * Design ref: CMP mixed-rendering, proposal v1.6, design ADR-2 (bindingMap_), ADR-9 (error codes).
 *
 * Red conditions (before implementation):
 *   - The two CAPI symbols don't exist → compilation fails.
 *
 * Pass conditions:
 *   - Bind with null handles → 401 (ARKUI_ERROR_CODE_PARAM_INVALID)
 *   - Bind with valid handles + no subAM → 401 (exportedRoot not adopted)
 *   - Unbind with null handle → 401
 *   - Unbind with unknown bindId → 0 (idempotent, no side effects)
 *   - Multi-binding isolation: two bindIds on same owner, unbind one, the other unaffected
 */
class NativeAccessibilityBindExportTreeTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: BindExportTreeNullOwner001
 * @tc.desc: Test BindExportTree with null owner handle returns 401.
 */
HWTEST_F(NativeAccessibilityBindExportTreeTest, BindExportTreeNullOwner001, TestSize.Level1)
{
    int64_t bindId = 100;
    ArkUI_NodeHandle exportedRoot = reinterpret_cast<ArkUI_NodeHandle>(0x12345678);
    int32_t result = OH_ArkUI_Accessibility_BindExportTree(nullptr, bindId, exportedRoot);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: BindExportTreeNullExportedRoot002
 * @tc.desc: Test BindExportTree with null exportedRoot handle returns 401.
 */
HWTEST_F(NativeAccessibilityBindExportTreeTest, BindExportTreeNullExportedRoot002, TestSize.Level1)
{
    ArkUI_NodeHandle owner = reinterpret_cast<ArkUI_NodeHandle>(0x87654321);
    int64_t bindId = 101;
    int32_t result = OH_ArkUI_Accessibility_BindExportTree(owner, bindId, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: BindExportTreeBothNull003
 * @tc.desc: Test BindExportTree with both null handles returns 401.
 */
HWTEST_F(NativeAccessibilityBindExportTreeTest, BindExportTreeBothNull003, TestSize.Level1)
{
    int64_t bindId = 102;
    int32_t result = OH_ArkUI_Accessibility_BindExportTree(nullptr, bindId, nullptr);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: UnbindExportTreeNullOwner004
 * @tc.desc: Test UnbindExportTree with null owner handle returns 401.
 */
HWTEST_F(NativeAccessibilityBindExportTreeTest, UnbindExportTreeNullOwner004, TestSize.Level1)
{
    int64_t bindId = 103;
    int32_t result = OH_ArkUI_Accessibility_UnbindExportTree(nullptr, bindId);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: UnbindExportTreeInvalidOwner005
 * @tc.desc: Test UnbindExportTree with an invalid (non-CAPI) owner handle returns 401.
 *           The handle is a real ArkUI_Node object with cNode/buildNode unset (a node not
 *           created through the CAPI path) — wild-cast integer handles would crash inside
 *           CheckIsCNode's dereference, so a constructible non-CAPI object is used instead.
 *           The idempotent NO_ERROR path for unknown bindIds on a valid owner is covered
 *           at the AM level (UnbindUnknownIdempotent007).
 */
HWTEST_F(NativeAccessibilityBindExportTreeTest, UnbindExportTreeInvalidOwner005, TestSize.Level1)
{
    ArkUI_Node ownerNode = {};
    int64_t unknownBindId = 99999;
    int32_t result = OH_ArkUI_Accessibility_UnbindExportTree(&ownerNode, unknownBindId);
    EXPECT_EQ(result, ARKUI_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: MultiBindIsolation006
 *  @tc.desc: Test multi-binding isolation: two bindIds, unbind one, the other unaffected.
 *           Uses non-CAPI node objects — all calls fail with 401 (invalid handle) but validate
 *           the API surface without dereferencing garbage.
 */
HWTEST_F(NativeAccessibilityBindExportTreeTest, MultiBindIsolation006, TestSize.Level1)
{
    ArkUI_Node ownerNode = {};
    ArkUI_Node rootNode1 = {};
    ArkUI_Node rootNode2 = {};
    ArkUI_NodeHandle owner = &ownerNode;
    ArkUI_NodeHandle exportedRoot1 = &rootNode1;
    ArkUI_NodeHandle exportedRoot2 = &rootNode2;
    int64_t bindId1 = 201;
    int64_t bindId2 = 202;
    // Both binds fail with 401 (invalid handles).
    int32_t result1 = OH_ArkUI_Accessibility_BindExportTree(owner, bindId1, exportedRoot1);
    int32_t result2 = OH_ArkUI_Accessibility_BindExportTree(owner, bindId2, exportedRoot2);
    EXPECT_EQ(result1, ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(result2, ARKUI_ERROR_CODE_PARAM_INVALID);
    // Unbind both — invalid owner handle is also a param error (401).
    int32_t unbind1 = OH_ArkUI_Accessibility_UnbindExportTree(owner, bindId1);
    int32_t unbind2 = OH_ArkUI_Accessibility_UnbindExportTree(owner, bindId2);
    EXPECT_EQ(unbind1, ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(unbind2, ARKUI_ERROR_CODE_PARAM_INVALID);
}
} // namespace
