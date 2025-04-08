/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "gmock/gmock.h"

#include "accessor_test_base.h"
#include "node_api.h"
#include "core/interfaces/native/utility/reverse_converter.h"

#include "core/interfaces/native/implementation/transition_effect_peer_impl.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

namespace {
const std::string g_message = "message";

} // namespace

namespace GeneratedModifier {
    const GENERATED_ArkUITransitionEffectAccessor* GetTransitionEffectAccessor();
}

class TransitionEffectAccessorTest : public AccessorTestCtorBase<GENERATED_ArkUITransitionEffectAccessor,
    &GENERATED_ArkUIAccessors::getTransitionEffectAccessor, TransitionEffectPeer> {
public:
    void *CreatePeerInstance() override
    {
        Ark_String arkType = Converter::ArkValue<Ark_String>(g_message);
        Ark_TransitionEffects* arkEffects = new(Ark_TransitionEffects);
        return accessor_->ctor(&arkType, arkEffects);
    }
};

/**
 * @tc.name: getRotateTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getRotateTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->rotate, nullptr);

    Ark_RotateOptions value;
    value.x = Converter::ArkValue<Opt_Number>(2);
    value.y = Converter::ArkValue<Opt_Number>(3);
    value.z = Converter::ArkValue<Opt_Number>(4);
    value.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(3);
    value.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(3);
    value.centerZ = Converter::ArkValue<Opt_Number>(3);
    value.perspective = Converter::ArkValue<Opt_Number>(6);
    value.angle = Converter::ArkUnion<Ark_Union_Number_String, Ark_Number>(45);

    auto peer = accessor_->rotate(&value);
    ASSERT_NE(peer, nullptr);

    auto effect = AceType::DynamicCast<ChainedRotateEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
    ASSERT_EQ(effect->GetEffect().xDirection, 2);
    ASSERT_EQ(effect->GetEffect().yDirection, 3);
    ASSERT_EQ(effect->GetEffect().zDirection, 4);
    ASSERT_EQ(effect->GetEffect().centerX.ToString(), "3.00vp");
    ASSERT_EQ(effect->GetEffect().centerY.ToString(), "3.00vp");
    ASSERT_EQ(effect->GetEffect().centerZ.ToString(), "3.00vp");
    ASSERT_EQ(effect->GetEffect().perspective, 6);
    ASSERT_EQ(effect->GetEffect().angle, 45);
    GeneratedModifier::GetTransitionEffectAccessor()->destroyPeer(peer);

    peer = accessor_->rotate(nullptr);
    ASSERT_EQ(peer, nullptr);
}

/**
 * @tc.name: getTranslateTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getTranslateTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->translate, nullptr);
    Ark_TranslateOptions value;
    value.x = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(2);
    value.y = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(3);
    value.z = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(4);
    auto peer = accessor_->translate(&value);
    ASSERT_NE(peer, nullptr);

    auto effect = AceType::DynamicCast<ChainedTranslateEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
    ASSERT_EQ(effect->GetEffect().x.ToString(), "2.00vp");
    ASSERT_EQ(effect->GetEffect().y.ToString(), "3.00vp");
    ASSERT_EQ(effect->GetEffect().z.ToString(), "4.00vp");
    GeneratedModifier::GetTransitionEffectAccessor()->destroyPeer(peer);

    peer = accessor_->translate(nullptr);
    ASSERT_EQ(peer, nullptr);
}

/**
 * @tc.name: getScaleTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getScaleTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->scale, nullptr);
    Ark_ScaleOptions value;
    value.x = Converter::ArkValue<Opt_Number>(2);
    value.y = Converter::ArkValue<Opt_Number>(3);
    value.z = Converter::ArkValue<Opt_Number>(4);
    value.centerX = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(3);
    value.centerY = Converter::ArkUnion<Opt_Union_Number_String, Ark_Number>(3);
    auto peer = accessor_->scale(&value);
    ASSERT_NE(peer, nullptr);

    auto effect = AceType::DynamicCast<ChainedScaleEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
    ASSERT_EQ(effect->GetEffect().xScale, 2);
    ASSERT_EQ(effect->GetEffect().yScale, 3);
    ASSERT_EQ(effect->GetEffect().zScale, 4);
    ASSERT_EQ(effect->GetEffect().centerX.ToString(), "3.00vp");
    ASSERT_EQ(effect->GetEffect().centerY.ToString(), "3.00vp");
    GeneratedModifier::GetTransitionEffectAccessor()->destroyPeer(peer);

    peer = accessor_->scale(nullptr);
    ASSERT_EQ(peer, nullptr);
}

/**
 * @tc.name: getOpacityTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getOpacityTest, TestSize.Level1)
{
    const int testValue = 330;
    ASSERT_NE(accessor_->opacity, nullptr);
    Ark_Number value = Converter::ArkValue<Ark_Number>(testValue);

    auto peer = accessor_->opacity(&value);
    ASSERT_NE(peer, nullptr);

    auto effect = AceType::DynamicCast<ChainedOpacityEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
    ASSERT_EQ(effect->GetEffect(), testValue);
    GeneratedModifier::GetTransitionEffectAccessor()->destroyPeer(peer);

    peer = accessor_->opacity(nullptr);
    ASSERT_EQ(peer, nullptr);
}

/**
 * @tc.name: getMoveTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getMoveTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->move, nullptr);
    Ark_TransitionEdge value = Converter::ArkValue<Ark_TransitionEdge>(NG::TransitionEdge::END);

    auto peer = accessor_->move(value);
    ASSERT_NE(peer, nullptr);

    auto effect = AceType::DynamicCast<ChainedMoveEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
    ASSERT_EQ(effect->GetEffect(), NG::TransitionEdge::END);
    GeneratedModifier::GetTransitionEffectAccessor()->destroyPeer(peer);
}

/**
 * @tc.name: getAsymmetricTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getAsymmetricTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->asymmetric, nullptr);
    TransitionEffectPeer app;
    TransitionEffectPeer disapp;
    auto peer = accessor_->asymmetric(&app, &disapp);
    ASSERT_NE(peer, nullptr);
}

/**
 * @tc.name: getAnimationTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getAnimationTest, TestSize.Level1)
{
    const int testValue = 333;
    ASSERT_NE(accessor_->animation, nullptr);
    Ark_AnimateParam animParam;
    animParam.duration = Converter::ArkValue<Opt_Number>(testValue);
    animParam.tempo =  Converter::ArkValue<Opt_Number>(0.5);
    TransitionEffectPeer peer;
    peer.handler = AceType::MakeRefPtr<ChainedOpacityEffect>(33);

    auto peer2 = accessor_->animation(reinterpret_cast<TransitionEffectPeer*>(&peer), &animParam);
    ASSERT_NE(peer2, nullptr);
    ASSERT_EQ(peer2->handler->GetAnimationOption()->GetDuration(), testValue);
}

/**
 * @tc.name: getCombineTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getCombineTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->combine, nullptr);
    auto opacity1 = 444;
    auto opacity2 = 555;

    TransitionEffectPeer peer1;
    peer1.handler = AceType::MakeRefPtr<ChainedOpacityEffect>(opacity1);

    TransitionEffectPeer peer2;
    peer2.handler = AceType::MakeRefPtr<ChainedOpacityEffect>(opacity2);

    auto ret_peer1 = accessor_->combine(reinterpret_cast<TransitionEffectPeer*>(&peer1), &peer2);
    ASSERT_NE(ret_peer1, nullptr);
    auto effect1 = AceType::DynamicCast<ChainedOpacityEffect>(ret_peer1->handler);
    ASSERT_NE(effect1, nullptr);
    ASSERT_EQ(effect1->GetEffect(), opacity1);

    auto effect2 = AceType::DynamicCast<ChainedOpacityEffect>(ret_peer1->handler->GetNext());
    ASSERT_NE(effect2, nullptr);
    ASSERT_EQ(effect2->GetEffect(), opacity2);
}

/**
 * @tc.name: getIdentityTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getIdentityTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getIDENTITY, nullptr);
    auto peer = accessor_->getIDENTITY();
    ASSERT_NE(peer, nullptr);
    auto effect = AceType::DynamicCast<ChainedIdentityEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
}

/**
 * @tc.name: getSlideSwitchTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getSlideSwitchTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getSLIDE_SWITCH, nullptr);
    auto peer = accessor_->getSLIDE_SWITCH();
    ASSERT_NE(peer, nullptr);
    auto effect = AceType::DynamicCast<ChainedSlideSwitchEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
}

/**
 * @tc.name: getZeroOpacityTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getZeroOpacityTest, TestSize.Level1)
{
    const int testValue = 0;
    ASSERT_NE(accessor_->getOPACITY, nullptr);
    auto peer = accessor_->getOPACITY();
    ASSERT_NE(peer, nullptr);

    auto effect = AceType::DynamicCast<ChainedOpacityEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
    ASSERT_EQ(effect->GetEffect(), testValue);
    GeneratedModifier::GetTransitionEffectAccessor()->destroyPeer(peer);

    peer = accessor_->opacity(nullptr);
    ASSERT_EQ(peer, nullptr);
}

/**
 * @tc.name: getSlideTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TransitionEffectAccessorTest, getSlideTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getSLIDE, nullptr);
    auto peer = accessor_->getSLIDE();
    ASSERT_NE(peer, nullptr);

    auto effect = AceType::DynamicCast<ChainedAsymmetricEffect>(peer->handler);
    ASSERT_NE(effect, nullptr);
    GeneratedModifier::GetTransitionEffectAccessor()->destroyPeer(peer);

    auto appearEffect = AceType::DynamicCast<ChainedMoveEffect>(effect->GetAppearEffect());
    ASSERT_NE(appearEffect, nullptr);
    auto disappearEffect = AceType::DynamicCast<ChainedMoveEffect>(effect->GetDisappearEffect());
    ASSERT_NE(disappearEffect, nullptr);

    auto appearType = appearEffect->GetEffect();
    auto disappearType = disappearEffect->GetEffect();
    ASSERT_EQ(appearType, TransitionEdge::START);
    ASSERT_EQ(disappearType, TransitionEdge::END);
}

} // namespace OHOS::Ace::NG
