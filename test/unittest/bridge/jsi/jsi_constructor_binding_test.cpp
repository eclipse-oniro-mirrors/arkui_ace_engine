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

#include <atomic>
#include <thread>

#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_bindings.h"

using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace {
constexpr int SECOND_NATIVE_VALUE = 2;

struct TestNative {
    int value = 0;
};
struct TestDerived : TestNative {};
std::atomic<int> g_createdFirst { 0 };
std::atomic<int> g_destroyedFirst { 0 };
std::atomic<int> g_createdSecond { 0 };
std::atomic<int> g_destroyedSecond { 0 };
std::atomic<int> g_marked { 0 };

void ConstructFirst(const JSCallbackInfo& info)
{
    ++g_createdFirst;
    info.SetReturnValue(new TestNative { 1 });
}

void ConstructSecond(const JSCallbackInfo& info)
{
    ++g_createdSecond;
    info.SetReturnValue(new TestNative { SECOND_NATIVE_VALUE });
}

void DestroyFirst(TestNative* instance)
{
    EXPECT_EQ(instance->value, 1);
    ++g_destroyedFirst;
    delete instance;
}

void DestroySecond(TestNative* instance)
{
    EXPECT_EQ(instance->value, SECOND_NATIVE_VALUE);
    ++g_destroyedSecond;
    delete instance;
}

void Mark(TestNative* instance, const JSGCMarkCallbackInfo& info)
{
    EXPECT_EQ(instance->value, 1);
    ++g_marked;
}
} // namespace

class JsiConstructorBindingTest : public testing::Test {
public:
    void SetUp() override
    {
        g_createdFirst = 0;
        g_destroyedFirst = 0;
        g_createdSecond = 0;
        g_destroyedSecond = 0;
        g_marked = 0;
        panda::RuntimeOption option;
        vm_ = panda::JSNApi::CreateJSVM(option);
        ASSERT_NE(vm_, nullptr);
    }

    void TearDown() override
    {
        if (vm_ != nullptr) {
            JsiClassBase::UnDeclareAll();
            panda::JSNApi::DestroyJSVM(vm_);
            vm_ = nullptr;
        }
    }

protected:
    void Collect()
    {
        panda::JSNApi::TriggerGC(vm_, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    }

    panda::ecmascript::EcmaVM* vm_ = nullptr;
};

/**
 * @tc.name: BindingSnapshot_001
 * @tc.desc: Typed callbacks remain paired with their context after another registration.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, BindingSnapshot_001, TestSize.Level1)
{
    auto first = MakeJsiClassConstructorBinding<TestNative>(ConstructFirst, DestroyFirst, Mark);
    auto second = MakeJsiClassConstructorBinding<TestNative>(ConstructSecond, DestroySecond, nullptr);
    EXPECT_EQ(first->jsConstructor, ConstructFirst);
    EXPECT_EQ(second->jsConstructor, ConstructSecond);
    JSGCMarkCallbackInfo info;
    auto* instance = new TestNative { 1 };
    first->jsGcMarkErased(instance, info);
    first->jsDestructorErased(instance);
    second->jsDestructorErased(new TestNative { SECOND_NATIVE_VALUE });
    EXPECT_EQ(g_marked, 1);
    EXPECT_EQ(g_destroyedFirst, 1);
    EXPECT_EQ(g_destroyedSecond, 1);
}

/**
 * @tc.name: BindingThreadExit_001
 * @tc.desc: A native finalizer owns its context after the registering thread exits.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, BindingThreadExit_001, TestSize.Level1)
{
    JsiClassConstructorBindingRef* owner = nullptr;
    std::weak_ptr<const JsiClassConstructorBinding> weak;
    std::thread registeringThread([&owner, &weak]() {
        auto binding = MakeJsiClassConstructorBinding<TestNative>(ConstructFirst, DestroyFirst, nullptr);
        weak = binding;
        owner = new JsiClassConstructorBindingRef(binding);
    });
    registeringThread.join();
    ASSERT_NE(owner, nullptr);
    EXPECT_FALSE(weak.expired());
    // Runs on a different thread with no TLS callback registration.
    JsiJSNativePointerDestructorInterceptor(nullptr, new TestNative { 1 }, owner);
    EXPECT_EQ(g_destroyedFirst, 1);
    EXPECT_TRUE(weak.expired());
}

/**
 * @tc.name: BindingOwners_001
 * @tc.desc: Constructor finalization cannot release a context still owned by an instance.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, BindingOwners_001, TestSize.Level1)
{
    auto binding = MakeJsiClassConstructorBinding<TestNative>(ConstructFirst, DestroyFirst, nullptr);
    std::weak_ptr<const JsiClassConstructorBinding> weak = binding;
    auto* constructorOwner = new JsiClassConstructorBindingRef(binding);
    auto* instanceOwner = new JsiClassConstructorBindingRef(binding);
    binding.reset();
    JsiReleaseConstructorBinding(nullptr, nullptr, constructorOwner);
    EXPECT_FALSE(weak.expired());
    JsiJSNativePointerDestructorInterceptor(nullptr, new TestNative { 1 }, instanceOwner);
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(g_destroyedFirst, 1);
}

/**
 * @tc.name: BindingNullCallbacks_001
 * @tc.desc: Empty callbacks still release their owner and null owner data is accepted.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, BindingNullCallbacks_001, TestSize.Level1)
{
    auto binding = MakeJsiClassConstructorBinding<TestNative>(nullptr, nullptr, nullptr);
    EXPECT_EQ(binding->jsConstructor, nullptr);
    EXPECT_FALSE(binding->jsDestructorErased);
    EXPECT_FALSE(binding->jsGcMarkErased);
    std::weak_ptr<const JsiClassConstructorBinding> weak = binding;
    auto* owner = new JsiClassConstructorBindingRef(binding);
    binding.reset();
    JsiJSNativePointerDestructorInterceptor(nullptr, nullptr, owner);
    EXPECT_TRUE(weak.expired());
    JsiJSNativePointerDestructorInterceptor(nullptr, nullptr, nullptr);
    JsiReleaseConstructorBinding(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: BindConstructCollect_001
 * @tc.desc: Real Bind, VM constructor execution and GC invoke the matching native destructor once.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, BindConstructCollect_001, TestSize.Level1)
{
    {
        panda::LocalScope scope(vm_);
        auto global = panda::JSNApi::GetGlobalObject(vm_);
        JSClass<TestNative>::Declare("BindingNative");
        JsiClass<TestNative>::Bind(vm_, global, ConstructFirst, DestroyFirst);
        auto value = global->Get(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingNative"));
        ASSERT_TRUE(value->IsFunction(vm_));
        auto instance = panda::Local<panda::FunctionRef>(value)->Constructor(vm_, nullptr, 0);
        ASSERT_FALSE(panda::JSNApi::HasPendingException(vm_));
        ASSERT_TRUE(instance->IsObject(vm_));
        EXPECT_EQ(g_createdFirst, 1);
        EXPECT_EQ(g_destroyedFirst, 0);
        JsiClass<TestNative>::UnDeclare();
        global->Delete(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingNative"));
    }
    Collect();
    EXPECT_EQ(g_destroyedFirst, 1);
    Collect();
    EXPECT_EQ(g_destroyedFirst, 1);
}

/**
 * @tc.name: RebindConstructCollect_001
 * @tc.desc: Old constructors and instances keep their callbacks across Declare/Bind/UnDeclare.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, RebindConstructCollect_001, TestSize.Level1)
{
    {
        panda::LocalScope scope(vm_);
        auto global = panda::JSNApi::GetGlobalObject(vm_);
        JSClass<TestNative>::Declare("BindingNative");
        JsiClass<TestNative>::Bind(vm_, global, ConstructFirst, DestroyFirst);
        auto key = panda::StringRef::NewFromUtf8(vm_, "BindingNative");
        auto first = panda::Local<panda::FunctionRef>(global->Get(vm_, key));
        ASSERT_TRUE(first->Constructor(vm_, nullptr, 0)->IsObject(vm_));
        JSClass<TestNative>::Declare("BindingNative");
        JsiClass<TestNative>::Bind(vm_, global, ConstructSecond, DestroySecond);
        auto second = panda::Local<panda::FunctionRef>(global->Get(vm_, key));
        ASSERT_TRUE(second->Constructor(vm_, nullptr, 0)->IsObject(vm_));
        // This must still call ConstructFirst even though the same C was rebound.
        ASSERT_TRUE(first->Constructor(vm_, nullptr, 0)->IsObject(vm_));
        EXPECT_EQ(g_createdFirst, 2);
        EXPECT_EQ(g_createdSecond, 1);
        JsiClass<TestNative>::UnDeclare();
        global->Delete(vm_, key);
    }
    Collect();
    EXPECT_EQ(g_destroyedFirst, 2);
    EXPECT_EQ(g_destroyedSecond, 1);
}

/**
 * @tc.name: InheritBindConstructCollect_001
 * @tc.desc: The inheritance binding entry also transfers an owned context to native instances.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, InheritBindConstructCollect_001, TestSize.Level1)
{
    {
        panda::LocalScope scope(vm_);
        auto global = panda::JSNApi::GetGlobalObject(vm_);
        JSClass<TestNative>::Declare("BindingBase");
        JsiClass<TestNative>::Bind(vm_, global, ConstructFirst, DestroyFirst);
        JSClass<TestDerived>::Declare("BindingDerived");
        JsiClass<TestDerived>::InheritAndBind<TestNative>(
            vm_, global, [](const JSCallbackInfo& info) { info.SetReturnValue(new TestDerived {}); },
            [](TestDerived* ptr) {
                ++g_destroyedSecond;
                delete ptr;
            });
        auto value = global->Get(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingDerived"));
        ASSERT_TRUE(value->IsFunction(vm_));
        auto instance = panda::Local<panda::FunctionRef>(value)->Constructor(vm_, nullptr, 0);
        ASSERT_FALSE(panda::JSNApi::HasPendingException(vm_));
        ASSERT_TRUE(instance->IsObject(vm_));
        JsiClass<TestDerived>::UnDeclare();
        JsiClass<TestNative>::UnDeclare();
        global->Delete(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingDerived"));
        global->Delete(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingBase"));
    }
    Collect();
    EXPECT_EQ(g_destroyedSecond, 1);
    EXPECT_EQ(g_destroyedFirst, 0);
}

/**
 * @tc.name: ConstructException_001
 * @tc.desc: A throwing native constructor releases its returned instance before handing ownership to the VM.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, ConstructException_001, TestSize.Level1)
{
    {
        panda::LocalScope scope(vm_);
        auto global = panda::JSNApi::GetGlobalObject(vm_);
        JSClass<TestNative>::Declare("BindingNative");
        JsiClass<TestNative>::Bind(
            vm_, global,
            [](const JSCallbackInfo& info) {
                info.SetReturnValue(new TestNative { 1 });
                auto vm = info.GetVm();
                panda::JSNApi::ThrowException(
                    vm, panda::Exception::TypeError(vm, panda::StringRef::NewFromUtf8(vm, "constructor failed")));
            },
            DestroyFirst);
        auto value = global->Get(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingNative"));
        ASSERT_TRUE(value->IsFunction(vm_));
        panda::Local<panda::FunctionRef>(value)->Constructor(vm_, nullptr, 0);
        EXPECT_TRUE(panda::JSNApi::HasPendingException(vm_));
        panda::JSNApi::GetAndClearUncaughtException(vm_);
        EXPECT_EQ(g_destroyedFirst, 1);
        JsiClass<TestNative>::UnDeclare();
        global->Delete(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingNative"));
    }
    Collect();
    EXPECT_EQ(g_destroyedFirst, 1);
}

/**
 * @tc.name: ConstructExceptionWithoutInstance_001
 * @tc.desc: A constructor that throws before returning an instance must not invoke its destructor.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, ConstructExceptionWithoutInstance_001, TestSize.Level1)
{
    {
        panda::LocalScope scope(vm_);
        auto global = panda::JSNApi::GetGlobalObject(vm_);
        JSClass<TestNative>::Declare("BindingNative");
        JsiClass<TestNative>::Bind(
            vm_, global,
            [](const JSCallbackInfo& info) {
                auto vm = info.GetVm();
                panda::JSNApi::ThrowException(
                    vm, panda::Exception::TypeError(vm, panda::StringRef::NewFromUtf8(vm, "constructor failed")));
            },
            DestroyFirst);
        auto value = global->Get(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingNative"));
        ASSERT_TRUE(value->IsFunction(vm_));
        panda::Local<panda::FunctionRef>(value)->Constructor(vm_, nullptr, 0);
        EXPECT_TRUE(panda::JSNApi::HasPendingException(vm_));
        panda::JSNApi::GetAndClearUncaughtException(vm_);
        EXPECT_EQ(g_destroyedFirst, 0);
        JsiClass<TestNative>::UnDeclare();
        global->Delete(vm_, panda::StringRef::NewFromUtf8(vm_, "BindingNative"));
    }
    Collect();
    EXPECT_EQ(g_destroyedFirst, 0);
}

/**
 * @tc.name: MissingBinding_001
 * @tc.desc: Missing callback data produces a JS exception without dereferencing a null owner.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, MissingBinding_001, TestSize.Level1)
{
    panda::LocalScope scope(vm_);
    auto constructor = panda::FunctionRef::NewClassFunction(vm_, JsiJSConstructorInterceptor, nullptr, nullptr);
    constructor->Constructor(vm_, nullptr, 0);
    EXPECT_TRUE(panda::JSNApi::HasPendingException(vm_));
    panda::JSNApi::GetAndClearUncaughtException(vm_);
    EXPECT_EQ(g_createdFirst, 0);
    EXPECT_EQ(g_destroyedFirst, 0);
}

/**
 * @tc.name: NewClassFunctionPendingException_001
 * @tc.desc: With a pending exception JsiNewClassFunction returns non-empty undefined and keeps RAII cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, NewClassFunctionPendingException_001, TestSize.Level1)
{
    auto binding = MakeJsiClassConstructorBinding<TestNative>(ConstructFirst, DestroyFirst, nullptr);
    std::weak_ptr<const JsiClassConstructorBinding> weak = binding;
    {
        panda::LocalScope scope(vm_);
        panda::JSNApi::ThrowException(
            vm_, panda::Exception::TypeError(vm_, panda::StringRef::NewFromUtf8(vm_, "probe")));
        auto function = JsiNewClassFunction(vm_, binding);
        // The pending exception is preserved for the caller; the failure result
        // is a non-empty undefined ref, not a function.
        EXPECT_TRUE(panda::JSNApi::HasPendingException(vm_));
        EXPECT_FALSE(function.IsEmpty());
        EXPECT_FALSE(function->IsFunction(vm_));
        panda::JSNApi::GetAndClearUncaughtException(vm_);
        EXPECT_FALSE(panda::JSNApi::HasPendingException(vm_));
        // Release the caller-side binding and the LocalScope; the VM never took
        // ownership of the holder, so RAII must have freed it.
        binding.reset();
    }
    Collect();
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(g_createdFirst, 0);
    EXPECT_EQ(g_destroyedFirst, 0);
}

/**
 * @tc.name: NewClassFunctionNormal_001
 * @tc.desc: A successfully created constructor is released by the VM finalizer exactly once.
 * @tc.type: FUNC
 */
HWTEST_F(JsiConstructorBindingTest, NewClassFunctionNormal_001, TestSize.Level1)
{
    auto binding = MakeJsiClassConstructorBinding<TestNative>(ConstructFirst, DestroyFirst, nullptr);
    std::weak_ptr<const JsiClassConstructorBinding> weak = binding;
    {
        panda::LocalScope scope(vm_);
        auto function = JsiNewClassFunction(vm_, binding);
        EXPECT_FALSE(function.IsEmpty());
        EXPECT_TRUE(function->IsFunction(vm_));
        EXPECT_FALSE(weak.expired());
        binding.reset();
        // The constructor is still referenced by the VM after the caller lets go.
        EXPECT_FALSE(weak.expired());
    }
    Collect();
    // The VM finalizer released the holder; exactly one release keeps the count sane.
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(g_createdFirst, 0);
    EXPECT_EQ(g_destroyedFirst, 0);
}
