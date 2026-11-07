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

#include "napi_runtime.cpp"
#include "interfaces/napi/kits/observer/render/render_layout_observer_listener.h"

using namespace testing::ext;

namespace OHOS::Ace::Napi {
namespace {
struct CallbackState {
    int calls = 0;
    napi_ref lastArgument = nullptr;
};

napi_value CheckRenderCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr, nullptr };
    void* data = nullptr;
    EXPECT_EQ(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), napi_ok);
    auto* state = static_cast<CallbackState*>(data);
    EXPECT_NE(state, nullptr);
    if (!state) {
        return nullptr;
    }
    ++state->calls;
    EXPECT_EQ(argc, 1u);
    if (argc != 1) {
        return nullptr;
    }
    napi_valuetype type = napi_undefined;
    EXPECT_EQ(napi_typeof(env, argv[0], &type), napi_ok);
    EXPECT_EQ(type, napi_object);
    napi_value keys = nullptr;
    EXPECT_EQ(napi_get_property_names(env, argv[0], &keys), napi_ok);
    uint32_t length = 1;
    EXPECT_EQ(napi_get_array_length(env, keys, &length), napi_ok);
    EXPECT_EQ(length, 0u);
    if (state->lastArgument) {
        napi_value previous = nullptr;
        EXPECT_EQ(napi_get_reference_value(env, state->lastArgument, &previous), napi_ok);
        bool same = true;
        EXPECT_EQ(napi_strict_equals(env, previous, argv[0], &same), napi_ok);
        EXPECT_FALSE(same);
        EXPECT_EQ(napi_delete_reference(env, state->lastArgument), napi_ok);
        state->lastArgument = nullptr;
    }
    EXPECT_EQ(napi_create_reference(env, argv[0], 1, &state->lastArgument), napi_ok);
    return nullptr;
}
} // namespace

class RenderLayoutObserverListenerTest : public testing::Test {};

/**
 * @tc.name: PreservesRenderArguments
 * @tc.desc: Draw/Layout keeps one fresh empty object per callback after listener extraction.
 * @tc.type: FUNC
 */
HWTEST_F(RenderLayoutObserverListenerTest, PreservesRenderArguments, TestSize.Level1)
{
    NativeEngineMock engine;
    CallbackState state;
    napi_value callback = nullptr;
    ASSERT_EQ(
        napi_create_function(engine, "render", NAPI_AUTO_LENGTH, CheckRenderCallback, &state, &callback), napi_ok);
    RenderLayoutObserverListener listener(engine, callback);
    listener.OnDrawOrLayout();
    listener.OnDrawOrLayout();
    EXPECT_EQ(state.calls, 2);
    ASSERT_NE(state.lastArgument, nullptr);
    EXPECT_EQ(napi_delete_reference(engine, state.lastArgument), napi_ok);
}

/**
 * @tc.name: KeepsCallbackPastRegistrationScope
 * @tc.desc: The listener owns its callback after the registration handle scope closes.
 * @tc.type: FUNC
 */
HWTEST_F(RenderLayoutObserverListenerTest, KeepsCallbackPastRegistrationScope, TestSize.Level1)
{
    NativeEngineMock engine;
    CallbackState state;
    napi_handle_scope registrationScope = nullptr;
    ASSERT_EQ(napi_open_handle_scope(engine, &registrationScope), napi_ok);
    napi_value callback = nullptr;
    ASSERT_EQ(
        napi_create_function(engine, "render", NAPI_AUTO_LENGTH, CheckRenderCallback, &state, &callback), napi_ok);
    auto listener = std::make_shared<RenderLayoutObserverListener>(engine, callback);
    EXPECT_EQ(napi_close_handle_scope(engine, registrationScope), napi_ok);
    listener->OnDrawOrLayout();
    EXPECT_EQ(state.calls, 1);
    listener.reset();
    ASSERT_NE(state.lastArgument, nullptr);
    EXPECT_EQ(napi_delete_reference(engine, state.lastArgument), napi_ok);
}

/**
 * @tc.name: UnregisterMatchesAllWrappersOfCallback
 * @tc.desc: Repeated on creates two registrations; off(callback) removes all JS-equal wrappers.
 * @tc.type: FUNC
 */
HWTEST_F(RenderLayoutObserverListenerTest, UnregisterMatchesAllWrappersOfCallback, TestSize.Level1)
{
    NativeEngineMock engine;
    CallbackState state;
    napi_value callback = nullptr;
    napi_value otherCallback = nullptr;
    ASSERT_EQ(
        napi_create_function(engine, "render", NAPI_AUTO_LENGTH, CheckRenderCallback, &state, &callback), napi_ok);
    ASSERT_EQ(
        napi_create_function(engine, "other", NAPI_AUTO_LENGTH, CheckRenderCallback, &state, &otherCallback), napi_ok);
    RenderLayoutStore store;
    auto& listeners = store.GetListeners(UIObserverRenderType::DRAW);
    auto first = std::make_shared<RenderLayoutObserverListener>(engine, callback);
    auto second = std::make_shared<RenderLayoutObserverListener>(engine, callback);
    auto other = std::make_shared<RenderLayoutObserverListener>(engine, otherCallback);
    listeners.Register(first);
    listeners.Register(second);
    listeners.Register(other);
    ASSERT_EQ(listeners.Snapshot().size(), 3u);
    EXPECT_TRUE(first->NapiEqual(callback));
    EXPECT_TRUE(second->NapiEqual(callback));
    EXPECT_FALSE(other->NapiEqual(callback));
    listeners.Unregister([callback](const std::shared_ptr<RenderLayoutListener>& listener) {
        return std::static_pointer_cast<RenderLayoutObserverListener>(listener)->NapiEqual(callback);
    });
    auto remaining = listeners.Snapshot();
    ASSERT_EQ(remaining.size(), 1u);
    EXPECT_EQ(remaining.front(), other);
}

/**
 * @tc.name: ResetPreventsFurtherInvocation
 * @tc.desc: Reset is idempotent and prevents both parameter construction and JS calls.
 * @tc.type: FUNC
 */
HWTEST_F(RenderLayoutObserverListenerTest, ResetPreventsFurtherInvocation, TestSize.Level1)
{
    NativeEngineMock engine;
    CallbackState state;
    napi_value callback = nullptr;
    ASSERT_EQ(
        napi_create_function(engine, "render", NAPI_AUTO_LENGTH, CheckRenderCallback, &state, &callback), napi_ok);
    NapiObserverCallback holder(engine, callback);
    EXPECT_TRUE(holder.Equals(callback));
    holder.Reset();
    holder.Reset();
    EXPECT_FALSE(holder.IsValid());
    EXPECT_FALSE(holder.Equals(callback));
    EXPECT_EQ(holder.GetValue(), nullptr);
    bool built = false;
    auto status = holder.Invoke<1>([&built](napi_env env, std::array<napi_value, 1>& arguments) {
        built = true;
        return napi_create_object(env, &arguments[0]);
    });
    EXPECT_EQ(status, napi_invalid_arg);
    EXPECT_FALSE(built);
    EXPECT_EQ(state.calls, 0);
}

/**
 * @tc.name: ArgumentFailureSkipsCallback
 * @tc.desc: Failed argument construction must not invoke JS and must leave the holder reusable.
 * @tc.type: FUNC
 */
HWTEST_F(RenderLayoutObserverListenerTest, ArgumentFailureSkipsCallback, TestSize.Level1)
{
    NativeEngineMock engine;
    CallbackState state;
    napi_value callback = nullptr;
    ASSERT_EQ(
        napi_create_function(engine, "render", NAPI_AUTO_LENGTH, CheckRenderCallback, &state, &callback), napi_ok);
    NapiObserverCallback holder(engine, callback);
    auto status = holder.Invoke<1>([](napi_env, std::array<napi_value, 1>&) { return napi_generic_failure; });
    EXPECT_EQ(status, napi_generic_failure);
    EXPECT_EQ(state.calls, 0);
    status = holder.Invoke<1>([](napi_env env, std::array<napi_value, 1>& arguments) {
        return napi_create_object(env, &arguments[0]);
    });
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(state.calls, 1);
    ASSERT_NE(state.lastArgument, nullptr);
    EXPECT_EQ(napi_delete_reference(engine, state.lastArgument), napi_ok);
}

/**
 * @tc.name: KeepsPendingCallbackException
 * @tc.desc: The common invocation layer must not clear an exception raised by the callback.
 * @tc.type: FUNC
 */
HWTEST_F(RenderLayoutObserverListenerTest, KeepsPendingCallbackException, TestSize.Level1)
{
    NativeEngineMock engine;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(engine, "throwing", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info) -> napi_value {
            napi_throw_error(env, nullptr, "observer callback error");
            return nullptr;
        }, nullptr, &callback), napi_ok);
    NapiObserverCallback holder(engine, callback);
    auto status = holder.Invoke<0>([](napi_env, std::array<napi_value, 0>&) { return napi_ok; });
    EXPECT_EQ(status, napi_pending_exception);
    bool pending = false;
    EXPECT_EQ(napi_is_exception_pending(engine, &pending), napi_ok);
    EXPECT_TRUE(pending);
    napi_value exception = nullptr;
    EXPECT_EQ(napi_get_and_clear_last_exception(engine, &exception), napi_ok);
}

} // namespace OHOS::Ace::Napi
