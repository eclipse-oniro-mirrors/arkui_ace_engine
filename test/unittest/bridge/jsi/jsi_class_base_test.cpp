/**
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

#include "gtest/gtest.h"

#include <string>
#include <unordered_map>

#include "frameworks/bridge/declarative_frontend/engine/bindings_defines.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_class_base.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

class JsiClassBaseTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override
    {
        // Clean up the global registry so a failed/skipped test cannot leak
        // registered callbacks into subsequent tests.
        JsiClassBase::UnDeclareAll();
    }
};

/**
 * @tc.name: JsiClassBaseRegisterUnDeclare001
 * @tc.desc: Test RegisterUnDeclare with valid function pointer
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, JsiClassBaseRegisterUnDeclare001, TestSize.Level1)
{
    // UnDeclareFunc is a plain function pointer, so the callback lambda must be
    // captureless and can only touch static storage; reset it here to keep the
    // test repeatable under --gtest_repeat.
    static bool called = false;
    called = false;
    JsiClassBase::UnDeclareFunc fn = []() { called = true; };
    JsiClassBase::RegisterUnDeclare(fn);
    // RegisterUnDeclare should succeed without crash
    // Clean up
    JsiClassBase::UnDeclareAll();
    EXPECT_TRUE(called);
}

/**
 * @tc.name: JsiClassBaseRegisterUnDeclare002
 * @tc.desc: Test RegisterUnDeclare with nullptr (safe no-op, registry unaffected)
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, JsiClassBaseRegisterUnDeclare002, TestSize.Level1)
{
    // nullptr is rejected without registering (JsiClassBase::RegisterUnDeclare).
    // The registry has no externally observable state, so the no-op is verified
    // by confirming that registration and dispatch still work afterwards.
    static bool called = false;
    called = false;
    JsiClassBase::RegisterUnDeclare(nullptr); // should be a safe no-op
    JsiClassBase::UnDeclareFunc fn = []() { called = true; };
    JsiClassBase::RegisterUnDeclare(fn);
    JsiClassBase::UnDeclareAll();
    EXPECT_TRUE(called);
}

/**
 * @tc.name: JsiClassBaseRegisterUnDeclare003
 * @tc.desc: Test RegisterUnDeclare with duplicate function (registry is a set)
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, JsiClassBaseRegisterUnDeclare003, TestSize.Level1)
{
    static int callCount = 0;
    callCount = 0;
    auto func = []() { callCount++; };
    JsiClassBase::RegisterUnDeclare(func);
    JsiClassBase::RegisterUnDeclare(func); // duplicate
    JsiClassBase::UnDeclareAll();
    // Set semantics: only called once even if registered twice
    EXPECT_EQ(callCount, 1);
}

/**
 * @tc.name: JsiClassBaseUnDeclareAll001
 * @tc.desc: Test UnDeclareAll invokes all registered functions and clears registry
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, JsiClassBaseUnDeclareAll001, TestSize.Level1)
{
    static int counter = 0;
    counter = 0;
    constexpr int expectedTotal = 111; // 1 + 10 + 100
    auto fn1 = []() { counter += 1; };
    auto fn2 = []() { counter += 10; };
    auto fn3 = []() { counter += 100; };
    JsiClassBase::RegisterUnDeclare(fn1);
    JsiClassBase::RegisterUnDeclare(fn2);
    JsiClassBase::RegisterUnDeclare(fn3);
    JsiClassBase::UnDeclareAll();
    EXPECT_EQ(counter, expectedTotal);
    // After UnDeclareAll, registry should be empty; calling again should not re-invoke
    JsiClassBase::UnDeclareAll();
    EXPECT_EQ(counter, expectedTotal);
}

/**
 * @tc.name: JsiClassBaseUnDeclareAll002
 * @tc.desc: Test UnDeclareAll with empty registry (should be safe no-op)
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, JsiClassBaseUnDeclareAll002, TestSize.Level1)
{
    JsiClassBase::UnDeclareAll();
    SUCCEED();
}

/**
 * @tc.name: JsiClassBaseDeclareImpl001
 * @tc.desc: Test DeclareImpl resets className and clears all function maps
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, JsiClassBaseDeclareImpl001, TestSize.Level1)
{
    JsiFunctionMap staticFns;
    JsiFunctionMap customFns;
    JsiFunctionMap customGetFns;
    JsiFunctionMap customSetFns;
    panda::Global<panda::FunctionRef> classFn;
    std::string className = "oldName";

    // Insert dummy entries (empty Global handles are safe to clear)
    staticFns.emplace("staticMethod", panda::Global<panda::FunctionRef>());
    customFns.emplace("customMethod", panda::Global<panda::FunctionRef>());
    customGetFns.emplace("getter", panda::Global<panda::FunctionRef>());
    customSetFns.emplace("setter", panda::Global<panda::FunctionRef>());

    JsiClassBase::DeclareImpl("testClass", className, staticFns, customFns, customGetFns, customSetFns, classFn);

    EXPECT_EQ(className, "testClass");
    EXPECT_TRUE(staticFns.empty());
    EXPECT_TRUE(customFns.empty());
    EXPECT_TRUE(customGetFns.empty());
    EXPECT_TRUE(customSetFns.empty());
}

/**
 * @tc.name: JsiClassBaseDeclareImpl002
 * @tc.desc: Test DeclareImpl with already-empty maps (should be safe)
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, JsiClassBaseDeclareImpl002, TestSize.Level1)
{
    JsiFunctionMap staticFns;
    JsiFunctionMap customFns;
    JsiFunctionMap customGetFns;
    JsiFunctionMap customSetFns;
    panda::Global<panda::FunctionRef> classFn;
    std::string className;

    JsiClassBase::DeclareImpl("freshClass", className, staticFns, customFns, customGetFns, customSetFns, classFn);

    EXPECT_EQ(className, "freshClass");
    EXPECT_TRUE(staticFns.empty());
}

/**
 * @tc.name: BindImplMethods_001
 * @tc.desc: A real VM observes static methods, prototype methods and paired accessors after BindImpl.
 * @tc.type: FUNC
 */
HWTEST_F(JsiClassBaseTest, BindImplMethods_001, TestSize.Level1)
{
    panda::RuntimeOption option;
    auto* vm = panda::JSNApi::CreateJSVM(option);
    ASSERT_NE(vm, nullptr);
    {
        panda::LocalScope scope(vm);
        auto global = panda::JSNApi::GetGlobalObject(vm);
        auto callback = [](panda::JsiRuntimeCallInfo* info) -> panda::Local<panda::JSValueRef> {
            return panda::NumberRef::New(info->GetVM(), 42);
        };
        auto getter = [](panda::JsiRuntimeCallInfo* info) -> panda::Local<panda::JSValueRef> {
            auto object = panda::Local<panda::ObjectRef>(info->GetThisRef());
            return object->Get(info->GetVM(), panda::StringRef::NewFromUtf8(info->GetVM(), "stored"));
        };
        auto setter = [](panda::JsiRuntimeCallInfo* info) -> panda::Local<panda::JSValueRef> {
            auto vm = info->GetVM();
            auto object = panda::Local<panda::ObjectRef>(info->GetThisRef());
            object->Set(vm, panda::StringRef::NewFromUtf8(vm, "stored"), info->GetCallArgRef(0));
            return panda::JSValueRef::Undefined(vm);
        };
        JsiFunctionMap staticFns;
        JsiFunctionMap customFns;
        JsiFunctionMap getters;
        JsiFunctionMap setters;
        staticFns.emplace("staticMethod", panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, callback)));
        customFns.emplace("method", panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, callback)));
        getters.emplace("value", panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, getter)));
        setters.emplace("value", panda::Global<panda::FunctionRef>(vm, panda::FunctionRef::New(vm, setter)));
        panda::Global<panda::FunctionRef> classFn;
        std::string name = "BoundClass";
        JsiClassBase::BindImpl(
            vm, global, name.c_str(), name, [](panda::JsiRuntimeCallInfo* info) { return info->GetThisRef(); },
            staticFns, customFns, getters, setters, classFn);
        auto exported = global->Get(vm, panda::StringRef::NewFromUtf8(vm, name.c_str()));
        EXPECT_TRUE(exported->IsFunction(vm));
        auto staticMethod =
            panda::Local<panda::FunctionRef>(classFn->Get(vm, panda::StringRef::NewFromUtf8(vm, "staticMethod")));
        EXPECT_EQ(staticMethod->Call(vm, exported, nullptr, 0)->Int32Value(vm), 42);
        auto instance = panda::Local<panda::ObjectRef>(classFn->Constructor(vm, nullptr, 0));
        auto method = panda::Local<panda::FunctionRef>(instance->Get(vm, panda::StringRef::NewFromUtf8(vm, "method")));
        EXPECT_EQ(method->Call(vm, instance, nullptr, 0)->Int32Value(vm), 42);
        instance->Set(vm, panda::StringRef::NewFromUtf8(vm, "value"), panda::NumberRef::New(vm, 7));
        EXPECT_EQ(instance->Get(vm, panda::StringRef::NewFromUtf8(vm, "value"))->Int32Value(vm), 7);
        EXPECT_FALSE(panda::JSNApi::HasPendingException(vm));
        // Clear live Global handles before VM destruction (the earlier cases only used empty handles).
        JsiClassBase::DeclareImpl("reset", name, staticFns, customFns, getters, setters, classFn);
        EXPECT_TRUE(staticFns.empty());
        EXPECT_TRUE(customFns.empty());
        EXPECT_TRUE(getters.empty());
        EXPECT_TRUE(setters.empty());
        EXPECT_TRUE(classFn.IsEmpty());
    }
    panda::JSNApi::DestroyJSVM(vm);
}
