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

#include <algorithm>
#include <atomic>
#include <future>
#include <thread>

#include "gtest/gtest.h"
#include "interfaces/inner_api/ui_session/ui_content_errors.h"
#include "interfaces/inner_api/ui_session/ui_content_proxy.h"
#include "interfaces/inner_api/ui_session/ui_report_stub.h"

#include "adapter/ohos/entrance/ui_session/include/ui_session_trace.h"
#include "adapter/ohos/entrance/ui_session/ui_session_manager_ohos.h"
#include "base/json/json_util.h"

using namespace testing::ext;

namespace OHOS::Ace {
// Trace output is outside this suite; all session registration and query code is production code.
UiSessionScopedTrace::UiSessionScopedTrace(const char*, ...) {}
UiSessionScopedTrace::~UiSessionScopedTrace() = default;

namespace {
constexpr int32_t CALLER_PID = 23001;
constexpr size_t TEXT_RULE_COUNT = 2;
constexpr size_t SINGLE_TEXT_RESULT_COUNT = 1;
const std::string MIXED_ID = "mixed";
const std::string MIXED_JSON = R"({"version":1,"ruleSetId":"mixed",
    "sourceConfig":{"arkui":true,"web":false},
    "rules":[
        {"ruleId":"split","sceneType":"SPLIT","enabled":true,"navigation":false,
         "custom":{"selectors":[{"compName":"Column"}],"geometry":{
             "heightRatioGt":0.8,"widthRatioLte":0.5,"totalWidthRatioGt":0.8,"nonOverlapping":true}}},
        {"ruleId":"input","sceneType":"TEXT_EDITOR","selector":{"nodeTypes":["TextInput"]},
         "condition":{"operator":"COUNT_GTE","threshold":1},
         "policy":{"reportOnRegister":false,"deduplicate":true,"minReportIntervalMs":0}},
        {"ruleId":"area","sceneType":"TEXT_EDITOR","selector":{"nodeTypes":["TextArea"]},
         "condition":{"operator":"COUNT_GTE","threshold":1},
         "policy":{"reportOnRegister":true,"deduplicate":true,"minReportIntervalMs":0}}],
    "webRules":[{"ruleId":"web_text","sceneType":"TEXT_EDITOR",
        "selector":{"nodeTypes":["input"]},"condition":{"operator":"COUNT_GTE","threshold":1}}]
})";

std::string QueryGeneration(const std::string& json)
{
    return JsonUtil::ParseJsonString(json)->GetString("_splitQueryGeneration");
}

std::string QueryResponse(const std::string& request)
{
    auto response = JsonUtil::Create(true);
    auto events = JsonUtil::CreateArray(true);
    response->Put("events", events);
    response->Put("_splitQueryGeneration", QueryGeneration(request).c_str());
    return response->ToString();
}

// Exercise the real Proxy schema boundary before dispatching to the real manager.
class PageSceneProxyRemote final : public IRemoteObject {
public:
    PageSceneProxyRemote(UiSessionManagerOhos& manager, int32_t pid) : manager_(manager), pid_(pid) {}
    int32_t SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption&) override
    {
        data.ReadInterfaceToken();
        auto request = data.ReadString();
        auto result = code == IUiContentService::REGISTER_PAGE_SCENE_RULES
                          ? manager_.RegisterPageSceneRules(pid_, request)
                          : manager_.GetPageScene(pid_, request);
        reply.WriteInt32(result);
        return 0;
    }

private:
    UiSessionManagerOhos& manager_;
    int32_t pid_;
};

class TestPageSceneReportStub : public UiReportStub {
public:
    bool AddDeathRecipient(const sptr<IRemoteObject::DeathRecipient>& recipient) override
    {
        deathRecipients_.emplace_back(recipient);
        return true;
    }

    void NotifyDeath()
    {
        auto recipients = std::move(deathRecipients_);
        const wptr<IRemoteObject> remote(this);
        for (const auto& recipient : recipients) {
            recipient->OnRemoteDied(remote);
        }
    }

private:
    std::vector<sptr<IRemoteObject::DeathRecipient>> deathRecipients_;
};
} // namespace

class UiSessionPageSceneTest : public testing::Test {
protected:
    void SetUp() override
    {
        report_ = std::make_shared<TestPageSceneReportStub>();
        manager_.SaveReportProxy(report_, CALLER_PID);
        manager_.SavePageSceneDetectFunction(
            [this](int32_t, const std::string& json, bool query) { (query ? queries_ : detections_).push_back(json); });
    }

    int32_t RequestThroughProxy(int32_t pid, const std::string& request, bool query = false)
    {
        MessageParcel::SetDefaultPayloadStorage(true);
        sptr<PageSceneProxyRemote> remote = std::make_shared<PageSceneProxyRemote>(manager_, pid);
        UIContentServiceProxy proxy(remote);
        proxy.report_ = std::make_shared<UiReportStub>();
        auto callback = [](const std::string&) {};
        const auto result =
            query ? proxy.GetPageScene(request, callback) : proxy.RegisterPageSceneRules(request, callback);
        MessageParcel::SetDefaultPayloadStorage(false);
        return result;
    }

    void InstallWebCallback()
    {
        manager_.SaveWebPageSceneFunction([this](UiSessionManager::WebPageSceneOp op, int32_t, const std::string& json,
                                              bool) { return ApplyWebOperation(op, json); });
    }

    int32_t ApplyWebOperation(UiSessionManager::WebPageSceneOp op, const std::string& json)
    {
        if (op == UiSessionManager::WebPageSceneOp::GetPageScene) {
            webQueries_.push_back(json);
            return webQueryResult_;
        }
        if (op == UiSessionManager::WebPageSceneOp::UnregisterRules) {
            ++webRemovals_;
            webOwner_.clear();
            return NO_ERROR;
        }
        return RegisterWeb(json);
    }

    int32_t RegisterWeb(const std::string& json)
    {
        if (!webOwner_.empty()) {
            return LAST_UNFINISH;
        }
        if (webRegisterResult_ != NO_ERROR) {
            return webRegisterResult_;
        }
        webOwner_ = JsonUtil::ParseJsonString(json)->GetString("ruleSetId");
        return NO_ERROR;
    }

    void ReportSplitQueryResult(const std::string& request)
    {
        manager_.ReportSplitPageSceneQueryResult(CALLER_PID, QueryResponse(request), QueryGeneration(request));
    }

    void ClearDetections()
    {
        detections_.clear();
    }

    UiSessionManagerOhos manager_;
    sptr<TestPageSceneReportStub> report_;
    std::vector<std::string> detections_;
    std::vector<std::string> queries_;
    std::vector<std::string> webQueries_;
    std::string webOwner_;
    int32_t webRegisterResult_ = NO_ERROR;
    int32_t webQueryResult_ = NO_ERROR;
    int webRemovals_ = 0;
};

HWTEST_F(UiSessionPageSceneTest, MixedRegistrationUsesOriginalTextSlot, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ASSERT_EQ(manager_.pageSceneRuleSets_.count(CALLER_PID), 1u);
    const auto& text = manager_.pageSceneRuleSets_.at(CALLER_PID);
    EXPECT_EQ(text.rules.size(), TEXT_RULE_COUNT);
    EXPECT_TRUE(std::all_of(
        text.rules.begin(), text.rules.end(), [](const auto& rule) { return rule.sceneType == "TEXT_EDITOR"; }));
    auto textJson = JsonUtil::ParseJsonString(text.ruleJson);
    EXPECT_EQ(textJson->GetValue("rules")->GetArraySize(), static_cast<int32_t>(TEXT_RULE_COUNT));
    const auto& split = manager_.splitPageSceneRegistration_->ruleSet;
    auto splitJson = JsonUtil::ParseJsonString(split.ruleJson);
    auto splitRules = splitJson->GetValue("rules");
    ASSERT_EQ(splitRules->GetArraySize(), 1);
    EXPECT_EQ(splitRules->GetArrayItem(0)->GetString("sceneType"), "SPLIT");
    auto query = JsonUtil::ParseJsonString(manager_.FindRegisteredSplitQueryLocked(CALLER_PID, MIXED_ID));
    EXPECT_EQ(query->GetValue("rules")->GetArraySize(), static_cast<int32_t>(SINGLE_TEXT_RESULT_COUNT));
}


} // namespace OHOS::Ace
