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

#include "gtest/gtest.h"

#include "frameworks/bridge/declarative_frontend/engine/bindings_defines.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_bindings.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

class JsiConstructorBindingTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: JsiClassConstructorBindingDefault001
 * @tc.desc: Test JsiClassConstructorBinding default initialization (all fields nullptr)
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, JsiClassConstructorBindingDefault001, TestSize.Level1)
{
    JsiClassConstructorBinding binding;
    EXPECT_EQ(binding.jsConstructor, nullptr);
    EXPECT_EQ(binding.jsDestructorErased, nullptr);
    EXPECT_EQ(binding.jsGcMarkErased, nullptr);
}

/**
 * @tc.name: JsiClassConstructorBindingAssign002
 * @tc.desc: Test field assignment round-trip (exact callback kept, reset restores default)
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, JsiClassConstructorBindingAssign002, TestSize.Level1)
{
    JsiClassConstructorBinding binding;

    JSFunctionCallback ctor = [](const JSCallbackInfo& info) {};
    binding.jsConstructor = ctor;
    // Exact-pointer assertion: distinguishes which callback is stored, unlike a
    // tautological EXPECT_NE(nullptr) on a just-assigned function pointer.
    EXPECT_EQ(binding.jsConstructor, ctor);
    binding.jsConstructor = nullptr;
    EXPECT_EQ(binding.jsConstructor, nullptr);
}

/**
 * @tc.name: JsiClassConstructorBindingTypeErase003
 * @tc.desc: Test type-safe destroy adapter round-trip (store, invoke via erased signature, verify)
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, JsiClassConstructorBindingTypeErase003, TestSize.Level1)
{
    JsiClassConstructorBinding binding;

    // The local class destructor below can only reference static storage;
    // reset it here to keep the test repeatable under --gtest_repeat.
    static int destroyedValue = 0;
    destroyedValue = 0;
    struct TestType {
        int value;
        ~TestType()
        {
            destroyedValue = value;
        }
    };

    // Same registration sequence as JsiClass<C>::Bind: keep the typed callback in
    // per-type storage and register the adapter whose signature matches the erased
    // type exactly, so no function pointer reinterpret_cast is involved.
    JSDestructorCallback<TestType> typedDtor = +[](TestType* ptr) { delete ptr; };
    jsiTypedDestructor<TestType> = typedDtor;
    binding.jsDestructorErased = &JsiDestroyCallbackAdapter<TestType>;
    // Exact registration: guards against reverting to reinterpret_cast of the typed callback
    EXPECT_EQ(binding.jsDestructorErased, &JsiDestroyCallbackAdapter<TestType>);

    // Round-trip: allocate, invoke erased destructor, verify
    constexpr int testValue = 42;
    auto* obj = new TestType { testValue };
    binding.jsDestructorErased(obj);
    EXPECT_EQ(destroyedValue, testValue);

    // Clear per-type storage so other test cases observe a clean state
    jsiTypedDestructor<TestType> = nullptr;
}

/**
 * @tc.name: JsiClassConstructorBindingTypeErase004
 * @tc.desc: Test type-safe GC mark adapter assignment and invocation
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, JsiClassConstructorBindingTypeErase004, TestSize.Level1)
{
    JsiClassConstructorBinding binding;

    // The callback must be a captureless lambda to convert to a function pointer,
    // so it can only touch static storage; reset it here for repeatability.
    static int markedCount = 0;
    markedCount = 0;
    struct TestType {
        int value;
    };
    JSGCMarkCallback<TestType> typedGcMark = +[](TestType* instance, const JSGCMarkCallbackInfo& info) {
        markedCount++;
    };
    jsiTypedGcMark<TestType> = typedGcMark;
    binding.jsGcMarkErased = &JsiGcMarkCallbackAdapter<TestType>;
    // Exact registration (not a tautological non-null check on a just-taken address)
    EXPECT_EQ(binding.jsGcMarkErased, &JsiGcMarkCallbackAdapter<TestType>);
    EXPECT_EQ(binding.jsConstructor, nullptr); // fresh binding in this test case, never assigned

    // Invoke the GC mark callback through the erased signature and verify it took effect
    JSGCMarkCallbackInfo info;
    binding.jsGcMarkErased(nullptr, info);
    EXPECT_EQ(markedCount, 1);

    // Clear per-type storage so other test cases observe a clean state
    jsiTypedGcMark<TestType> = nullptr;
}

/**
 * @tc.name: JsiClassConstructorBindingTypeErase005
 * @tc.desc: Test adapters no-op safely when the per-type typed callback is null
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, JsiClassConstructorBindingTypeErase005, TestSize.Level1)
{
    JsiClassConstructorBinding binding;

    // Real scenario for the null branch: the adapter runs on a thread whose
    // thread_local typed storage was never registered. It must call nothing.
    static int destroyCalls = 0;
    static int markCalls = 0;
    destroyCalls = 0;
    markCalls = 0;
    struct TestType {
        int value;
    };
    JSDestructorCallback<TestType> typedDtor = +[](TestType* ptr) {
        destroyCalls++;
        delete ptr;
    };
    JSGCMarkCallback<TestType> typedGcMark = +[](TestType* instance, const JSGCMarkCallbackInfo& info) {
        markCalls++;
    };

    // Null typed storage: both adapters must be no-ops
    jsiTypedDestructor<TestType> = nullptr;
    jsiTypedGcMark<TestType> = nullptr;
    binding.jsDestructorErased = &JsiDestroyCallbackAdapter<TestType>;
    binding.jsGcMarkErased = &JsiGcMarkCallbackAdapter<TestType>;
    JSGCMarkCallbackInfo info;
    binding.jsGcMarkErased(nullptr, info);
    binding.jsDestructorErased(nullptr);
    EXPECT_EQ(destroyCalls, 0);
    EXPECT_EQ(markCalls, 0);

    // Contrast with registered storage: the same erased entry points do dispatch,
    // proving the no-op above comes from the null guard and not a broken call chain
    jsiTypedDestructor<TestType> = typedDtor;
    jsiTypedGcMark<TestType> = typedGcMark;
    auto* obj = new TestType { 1 };
    binding.jsDestructorErased(obj);
    binding.jsGcMarkErased(nullptr, info);
    EXPECT_EQ(destroyCalls, 1);
    EXPECT_EQ(markCalls, 1);

    // Clear per-type storage so other test cases observe a clean state
    jsiTypedDestructor<TestType> = nullptr;
    jsiTypedGcMark<TestType> = nullptr;
}

/**
 * @tc.name: JsiClassConstructorBindingDestructorInterceptor001
 * @tc.desc: Test JsiJSNativePointerDestructorInterceptor branches (null data, null erased, full chain)
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, JsiClassConstructorBindingDestructorInterceptor001, TestSize.Level1)
{
    static int destroyedValue = 0;
    destroyedValue = 0;
    struct TestType {
        int value;
        ~TestType()
        {
            destroyedValue = value;
        }
    };

    // Branch 1: null binding data -> no-op
    JsiJSNativePointerDestructorInterceptor(nullptr, nullptr, nullptr);

    // Branch 2: binding present but no erased destructor -> object must stay untouched
    JsiClassConstructorBinding emptyBinding;
    auto* untouched = new TestType { 7 };
    JsiJSNativePointerDestructorInterceptor(nullptr, untouched, &emptyBinding);
    EXPECT_EQ(destroyedValue, 0);
    delete untouched; // manual cleanup proves the interceptor did not destroy it
    EXPECT_EQ(destroyedValue, 7);

    // Branch 3: full chain interceptor -> adapter -> typed destructor
    JsiClassConstructorBinding binding;
    JSDestructorCallback<TestType> typedDtor = +[](TestType* ptr) { delete ptr; };
    jsiTypedDestructor<TestType> = typedDtor;
    binding.jsDestructorErased = &JsiDestroyCallbackAdapter<TestType>;
    constexpr int testValue = 42;
    auto* obj = new TestType { testValue };
    JsiJSNativePointerDestructorInterceptor(nullptr, obj, &binding);
    EXPECT_EQ(destroyedValue, testValue);

    // Clear per-type storage so other test cases observe a clean state
    jsiTypedDestructor<TestType> = nullptr;
}

