/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "text_based_modifier_module.h"

#include "core/pipeline/pipeline_base.h"
#include "load.h"
#include "log/log.h"
#include "utils/ani_utils.h"

namespace OHOS::Ace::Ani {

class TextBasedComponentCallbackAni {
public:
    TextBasedComponentCallbackAni(ani_env* env, ani_ref func)
    {
        CHECK_NULL_VOID(env && func);
        env_ = env;
        env->GlobalReference_Create(func, &func_);
    }
    ~TextBasedComponentCallbackAni()
    {
        CHECK_NULL_VOID(func_ && env_);
        env_->GlobalReference_Delete(func_);
    }
    void Call(ani_env* env, ani_size argc, ani_ref* argv, ani_ref* result)
    {
        CHECK_NULL_VOID(env);
        env->FunctionalObject_Call(static_cast<ani_fn_object>(func_), argc, argv, result);
    }
private:
    ani_env* env_ = nullptr;
    ani_ref func_ = nullptr;
};

ani_object ExtractorsFromSymbolGlyphModifierPtr(ani_env* env, [[maybe_unused]]ani_object aniClass, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, nullptr);

    auto symbolModifierAni = modifier->getTextBasedAniModifier()->fromSymbolModifierPeer(
        reinterpret_cast<void*>(pointer));
    return reinterpret_cast<ani_object>(symbolModifierAni);
}

ani_long ExtractorsToSymbolGlyphModifierPtr(ani_env* env, [[maybe_unused]] ani_object obj, ani_fn_object fnObj,
    ani_object symbolModifierAni)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier || fnObj, 0);

    auto callbackAni = std::make_shared<TextBasedComponentCallbackAni>(env, fnObj);
    std::function<void(WeakPtr<NG::FrameNode>)> symbolApply =
        [env, callbackAni](WeakPtr<NG::FrameNode> frameNode) -> void {
            auto node = reinterpret_cast<void*>(frameNode.Upgrade().GetRawPtr());
            ani_object pointerObject = AniUtils::CreateLong(env, reinterpret_cast<ani_long>(node));
            CHECK_NULL_VOID(pointerObject);
            std::vector<ani_ref> args = { pointerObject };
            ani_ref ret = nullptr;
            callbackAni->Call(env, args.size(), args.data(), &ret);
        };

    auto symbolModifierPeer = modifier->getTextBasedAniModifier()->toSymbolModifierPeer(symbolApply,
        reinterpret_cast<void*>(symbolModifierAni));
    return reinterpret_cast<ani_long>(symbolModifierPeer);
}

ani_object ExtractorsFromTextModifierPtr(ani_env* env, [[maybe_unused]]ani_object aniClass, ani_long pointer)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier, nullptr);
    auto textModifierAni = modifier->getTextBasedAniModifier()->fromTextModifierPeer(
        reinterpret_cast<void*>(pointer));
    return reinterpret_cast<ani_object>(textModifierAni);
}

ani_long ExtractorsToTextModifierPtr(ani_env* env, [[maybe_unused]] ani_object obj, ani_fn_object fnObj,
    ani_object textModifierAni)
{
    const auto* modifier = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifier || fnObj, 0);

    auto callbackAni = std::make_shared<TextBasedComponentCallbackAni>(env, fnObj);
    std::function<void(WeakPtr<NG::FrameNode>)> textApply =
        [env, callbackAni](WeakPtr<NG::FrameNode> frameNode) -> void {
            auto node = reinterpret_cast<void*>(frameNode.Upgrade().GetRawPtr());
            ani_object pointerObject = AniUtils::CreateLong(env, reinterpret_cast<ani_long>(node));
            CHECK_NULL_VOID(pointerObject);
            std::vector<ani_ref> args = { pointerObject };
            ani_ref ret = nullptr;
            callbackAni->Call(env, args.size(), args.data(), &ret);
        };

    auto textModifierPeer = modifier->getTextBasedAniModifier()->toTextModifierPeer(textApply,
        reinterpret_cast<void*>(textModifierAni));
    return reinterpret_cast<ani_long>(textModifierPeer);
}
} // namespace OHOS::Ace::Ani
