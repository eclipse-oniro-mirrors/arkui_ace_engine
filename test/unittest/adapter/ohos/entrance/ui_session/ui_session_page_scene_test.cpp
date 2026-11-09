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
constexpr int32_t OTHER_PID = 23002;
constexpr size_t TEXT_RULE_COUNT = 2;
constexpr size_t SINGLE_TEXT_RESULT_COUNT = 1;
constexpr size_t INITIAL_SCENE_REQUEST_COUNT = 2;
constexpr size_t CONTENT_CHANGE_REQUEST_COUNT = TEXT_RULE_COUNT + SINGLE_TEXT_RESULT_COUNT;
constexpr size_t SCENE_QUERY_REQUEST_COUNT = 2;
constexpr size_t SINGLE_QUERY_COUNT = 1;
constexpr int32_t FIRST_TEXT_RULE_INDEX = 1;
constexpr int32_t SPLIT_RULE_INDEX = 0;
constexpr int32_t MAX_COMMON_RULES = 10;
const std::string MIXED_ID = "mixed";
const std::string SECOND_ID = "second";
const std::string INPUT_RULE = "input";
const std::string AREA_RULE = "area";
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
const std::string LEGACY_JSON = R"({"version":1,"ruleSetId":"legacy","sourceConfig":{"web":false},
    "rules":[{"ruleId":"input","sceneType":"TEXT_EDITOR","selector":{"nodeTypes":["TextInput"]},
        "condition":{"operator":"COUNT_GTE","threshold":1},"policy":{"reportOnRegister":true}}]})";

std::string MakeMixedRule(const std::string& id = MIXED_ID, bool web = false)
{
    auto root = JsonUtil::ParseJsonString(MIXED_JSON);
    root->Replace("ruleSetId", id.c_str());
    root->GetValue("sourceConfig")->Replace("web", web);
    return root->ToString();
}

std::string MakeSplitOnlyRule(const std::string& id = MIXED_ID, bool web = false)
{
    auto root = JsonUtil::ParseJsonString(MakeMixedRule(id, web));
    auto rules = JsonUtil::CreateArray(true);
    rules->Put(root->GetValue("rules")->GetArrayItem(SPLIT_RULE_INDEX));
    root->Replace("rules", rules);
    return root->ToString();
}

std::string MakeTextOnlyRules(bool web = false)
{
    auto root = JsonUtil::ParseJsonString(MakeMixedRule(SECOND_ID, web));
    auto source = root->GetValue("rules");
    auto rules = JsonUtil::CreateArray(true);
    for (int index = FIRST_TEXT_RULE_INDEX; index < source->GetArraySize(); ++index) {
        auto rule = source->GetArrayItem(index);
        rule->GetValue("policy")->Replace("reportOnRegister", true);
        rules->Put(rule);
    }
    root->Replace("rules", rules);
    return root->ToString();
}

std::string MakeWebOnlyRule(bool emptyNativeArray)
{
    auto root = JsonUtil::ParseJsonString(MakeSplitOnlyRule(MIXED_ID, true));
    root->Delete("rules");
    if (emptyNativeArray) {
        root->Put("rules", JsonUtil::CreateArray(true));
    }
    return root->ToString();
}

std::string MakeSearchRule(const std::string& id)
{
    auto root = JsonUtil::ParseJsonString(R"({"ruleSetId":"search","rules":[{"ruleId":"find",
        "pageType":"Router","pageName":"Home","regions":[{"x":0,"y":0,"width":100,"height":100}],
        "keywords":["search"]}]})");
    root->Replace("ruleSetId", id.c_str());
    return root->ToString();
}

std::string MakeRuleCount(int32_t count, bool includeSplit)
{
    auto root = JsonUtil::ParseJsonString(MIXED_JSON);
    auto source = root->GetValue("rules");
    auto rules = JsonUtil::CreateArray(true);
    for (int32_t index = 0; index < count; ++index) {
        const auto templateIndex = includeSplit && index == 0 ? SPLIT_RULE_INDEX : FIRST_TEXT_RULE_INDEX;
        auto rule = JsonUtil::ParseJsonString(source->GetArrayItem(templateIndex)->ToString());
        rule->Replace("ruleId", ("rule_" + std::to_string(index)).c_str());
        rules->Put(rule);
    }
    root->Replace("rules", rules);
    return root->ToString();
}

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

HWTEST_F(UiSessionPageSceneTest, RegistrationFiltersTextPolicyBeforeDetection, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ASSERT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
    auto text = JsonUtil::ParseJsonString(detections_.front());
    EXPECT_FALSE(text->Contains("_splitBatch"));
    EXPECT_EQ(text->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), AREA_RULE);
    auto split = JsonUtil::ParseJsonString(detections_.back());
    EXPECT_FALSE(split->Contains("_splitTextRules"));
    const auto current = manager_.GetSplitPageSceneRulesForDetection(CALLER_PID);
    ASSERT_EQ(current.size(), SINGLE_TEXT_RESULT_COUNT);
    auto splitRules = JsonUtil::ParseJsonString(current.front());
    auto rules = splitRules->GetValue("rules");
    ASSERT_EQ(rules->GetArraySize(), static_cast<int32_t>(SINGLE_TEXT_RESULT_COUNT));
    EXPECT_EQ(rules->GetArrayItem(0)->GetString("sceneType"), "SPLIT");
}

HWTEST_F(UiSessionPageSceneTest, NodeEventSelectsOnlyAffectedTextRule, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ClearDetections();
    manager_.NotifyPageSceneNodeChanged("TextInput", true);
    manager_.FlushPageSceneNodeChanged();
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    auto text = JsonUtil::ParseJsonString(detections_.front());
    EXPECT_FALSE(text->Contains("_splitBatch"));
    EXPECT_EQ(text->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), INPUT_RULE);
}

HWTEST_F(UiSessionPageSceneTest, IrrelevantNodeDoesNotScheduleMixedDetection, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ClearDetections();
    manager_.NotifyPageSceneNodeChanged("Column", true);
    manager_.NotifyPageSceneNodeStateChanged("Column", UiSessionManager::PageSceneNodeStateChange::FOCUSABILITY);
    manager_.FlushPageSceneNodeChanged();
    EXPECT_TRUE(detections_.empty());
}

HWTEST_F(UiSessionPageSceneTest, TextEventsUseOriginalPendingRuleCoalescing, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ClearDetections();
    manager_.NotifyPageSceneNodeChanged("TextInput", true);
    manager_.NotifyPageSceneNodeChanged("TextInput", true);
    manager_.FlushPageSceneNodeChanged();
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    manager_.NotifyPageSceneNodeChanged("TextInput", true);
    manager_.FlushPageSceneNodeChanged();
    EXPECT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
}

HWTEST_F(UiSessionPageSceneTest, DisabledSplitKeepsTextRegistration, TestSize.Level1)
{
    auto json = JsonUtil::ParseJsonString(MIXED_JSON);
    json->GetValue("rules")->GetArrayItem(SPLIT_RULE_INDEX)->Replace("enabled", false);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, json->ToString()), NO_ERROR);
    ASSERT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
    auto text = JsonUtil::ParseJsonString(detections_.front());
    EXPECT_FALSE(text->Contains("_splitBatch"));
    EXPECT_EQ(text->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), AREA_RULE);
}

HWTEST_F(UiSessionPageSceneTest, MixedUnsupportedTextKeepsSupportedRules, TestSize.Level1)
{
    auto json = JsonUtil::ParseJsonString(MIXED_JSON);
    auto condition = json->GetValue("rules")->GetArrayItem(FIRST_TEXT_RULE_INDEX)->GetValue("condition");
    condition->Replace("operator", "WIDTH_GTE");
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, json->ToString()), NO_ERROR);
    ASSERT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
    EXPECT_EQ(JsonUtil::ParseJsonString(detections_.front())->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"),
        AREA_RULE);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
}

HWTEST_F(UiSessionPageSceneTest, WebFailureKeepsSplitAndOtherCallersRules, TestSize.Level1)
{
    manager_.SaveReportProxy(report_, OTHER_PID);
    ASSERT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, LEGACY_JSON), NO_ERROR);
    ClearDetections();
    InstallWebCallback();
    webRegisterResult_ = FAILED;
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule(SECOND_ID, true)), FAILED);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(OTHER_PID).ruleSetId, "legacy");
    EXPECT_EQ(manager_.pageSceneRuleSets_.count(CALLER_PID), 0u);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, SECOND_ID));
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_TRUE(JsonUtil::ParseJsonString(detections_.front())->GetBool("_splitBatch"));
    EXPECT_EQ(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_EQ(webRemovals_, 0);
}

HWTEST_F(UiSessionPageSceneTest, WebRegistrationLimitPreservesFirstOwner, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(MIXED_ID, true)), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(SECOND_ID, true)), PARAM_INVALID);
    EXPECT_EQ(webOwner_, MIXED_ID);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_FALSE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, SECOND_ID));
    EXPECT_EQ(webRemovals_, 0);
}

HWTEST_F(UiSessionPageSceneTest, LegacyWebUnregisterKeepsIndependentSplit, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeTextOnlyRules(true)), NO_ERROR);
    EXPECT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, SECOND_ID), NO_ERROR);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_TRUE(webOwner_.empty());
    EXPECT_EQ(webRemovals_, 1);
    EXPECT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_EQ(webRemovals_, 1);
}

HWTEST_F(UiSessionPageSceneTest, DelayedSplitReportKeepsDetectionSnapshot, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    const std::string event = R"({"events":[{"ruleSetId":"mixed","sceneType":"SPLIT","matched":true}]})";
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule(SECOND_ID)), NO_ERROR);
    std::vector<std::string> reports;
    report_->RegisterPageSceneEventCallback(MIXED_ID,
        [&reports](const std::string& json) { reports.push_back(json); });
    manager_.ReportPageSceneEvent(CALLER_PID, event, false);
    ASSERT_EQ(reports.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_EQ(reports.front(), event);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, SECOND_ID));
}

HWTEST_F(UiSessionPageSceneTest, QueryByIdHasIndependentGenerationAndSingleFlight, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), NO_ERROR);
    ASSERT_EQ(queries_.size(), SCENE_QUERY_REQUEST_COUNT);
    ASSERT_EQ(manager_.pageSceneQueries_.size(), SINGLE_QUERY_COUNT);
    EXPECT_TRUE(manager_.pageSceneQueries_.at(CALLER_PID).legacyPending);
    EXPECT_EQ(manager_.pageSceneQueries_.at(CALLER_PID).splitGeneration, QueryGeneration(queries_.back()));
    auto text = JsonUtil::ParseJsonString(queries_.front());
    EXPECT_EQ(text->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), INPUT_RULE);
    EXPECT_TRUE(QueryGeneration(queries_.front()).empty());
    EXPECT_FALSE(QueryGeneration(queries_.back()).empty());
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), LAST_UNFINISH);
    manager_.CompleteGetPageScene(CALLER_PID);
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, QueryGeneration(queries_.back())));
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
}

HWTEST_F(UiSessionPageSceneTest, QueryJsonDoesNotCreateSubscription, TestSize.Level1)
{
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), NO_ERROR);
    EXPECT_FALSE(manager_.GetPageSceneRulesRegistered());
    EXPECT_FALSE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_TRUE(detections_.empty());
    ASSERT_EQ(queries_.size(), SCENE_QUERY_REQUEST_COUNT);
    auto text = JsonUtil::ParseJsonString(queries_.front());
    auto split = JsonUtil::ParseJsonString(queries_.back());
    EXPECT_EQ(text->GetValue("rules")->GetArraySize(), static_cast<int32_t>(SINGLE_TEXT_RESULT_COUNT));
    EXPECT_EQ(text->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), INPUT_RULE);
    EXPECT_EQ(split->GetValue("rules")->GetArrayItem(0)->GetString("sceneType"), "SPLIT");
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_TRUE(manager_.pageSceneQueries_.count(CALLER_PID));
    manager_.ReportPageSceneEvent(CALLER_PID, R"({"sceneType":"TEXT_EDITOR","matched":true})", true);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
}

HWTEST_F(UiSessionPageSceneTest, WebFirstResultCannotFinishNativeQuery, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule(MIXED_ID, true)), NO_ERROR);
    ASSERT_EQ(webQueries_.size(), 1u);
    ASSERT_EQ(queries_.size(), 1u);
    EXPECT_TRUE(QueryGeneration(webQueries_.front()).empty());
    EXPECT_FALSE(QueryGeneration(queries_.front()).empty());
    manager_.ReportPageSceneEvent(CALLER_PID, "", true);
    ASSERT_EQ(manager_.pageSceneQueries_.size(), SINGLE_QUERY_COUNT);
    EXPECT_FALSE(manager_.pageSceneQueries_.at(CALLER_PID).legacyPending);
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, QueryGeneration(queries_.back())));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), LAST_UNFINISH);
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
}

HWTEST_F(UiSessionPageSceneTest, SplitCompletionKeepsLegacyWebQueryPending, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule(MIXED_ID, true)), NO_ERROR);
    ASSERT_EQ(queries_.size(), 1u);
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    ASSERT_EQ(manager_.pageSceneQueries_.size(), SINGLE_QUERY_COUNT);
    EXPECT_TRUE(manager_.pageSceneQueries_.at(CALLER_PID).legacyPending);
    EXPECT_TRUE(manager_.pageSceneQueries_.at(CALLER_PID).splitGeneration.empty());
    EXPECT_FALSE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, QueryGeneration(queries_.back())));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), LAST_UNFINISH);
    manager_.ReportPageSceneEvent(CALLER_PID, "", true);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
}

HWTEST_F(UiSessionPageSceneTest, WebQueryFailureDoesNotCancelSplitResult, TestSize.Level1)
{
    InstallWebCallback();
    webQueryResult_ = FAILED;
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule(MIXED_ID, true)), FAILED);
    ASSERT_EQ(queries_.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_TRUE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), LAST_UNFINISH);
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, StaleQueryCancellationAndResponsePreserveNewRequest, TestSize.Level1)
{
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    auto oldRequest = queries_.back();
    manager_.CancelSplitPageSceneQuery(CALLER_PID, QueryGeneration(oldRequest));
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    auto current = QueryGeneration(queries_.back());
    EXPECT_NE(current, QueryGeneration(oldRequest));
    manager_.CancelSplitPageSceneQuery(CALLER_PID, QueryGeneration(oldRequest));
    ReportSplitQueryResult(oldRequest);
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, current));
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
}

HWTEST_F(UiSessionPageSceneTest, SplitQueryBlocksAllOriginalQueries, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule(SECOND_ID)), NO_ERROR);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MakeSearchRule(SECOND_ID)), PARAM_INVALID);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    const auto split = queries_.back();
    for (const auto& request : { LEGACY_JSON, SECOND_ID, MakeWebOnlyRule(false) }) {
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, request), LAST_UNFINISH);
    }
    manager_.CompleteGetPageScene(CALLER_PID);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, QueryGeneration(split)));
    ReportSplitQueryResult(split);
    for (const auto& request : { LEGACY_JSON, SECOND_ID, MakeWebOnlyRule(false) }) {
        ASSERT_EQ(manager_.GetPageScene(CALLER_PID, request), NO_ERROR);
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, request), LAST_UNFINISH);
        manager_.ReportPageSceneEvent(CALLER_PID, "", true);
    }
}

HWTEST_F(UiSessionPageSceneTest, SplitCancellationKeepsMixedOriginalBranchPending, TestSize.Level1)
{
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), NO_ERROR);
    const auto split = queries_.back();
    manager_.CancelSplitPageSceneQuery(CALLER_PID, QueryGeneration(split));
    ReportSplitQueryResult(split);
    EXPECT_TRUE(manager_.pageSceneQueries_.count(CALLER_PID));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), LAST_UNFINISH);
    manager_.CompleteGetPageScene(CALLER_PID);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, OriginalUnregisterDoesNotReleasePendingSplitGate, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), NO_ERROR);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), NO_ERROR);
    const auto split = queries_.back();
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, "legacy"), NO_ERROR);
    ASSERT_EQ(manager_.pageSceneQueries_.size(), SINGLE_QUERY_COUNT);
    EXPECT_FALSE(manager_.pageSceneQueries_.at(CALLER_PID).legacyPending);
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, QueryGeneration(split)));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
    ReportSplitQueryResult(split);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), NO_ERROR);
    manager_.CompleteGetPageScene(CALLER_PID);
}

HWTEST_F(UiSessionPageSceneTest, OriginalQueriesBlockSplitAndMixedQueries, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule(SECOND_ID)), NO_ERROR);
    for (const auto& request : { LEGACY_JSON, SECOND_ID, MakeWebOnlyRule(false) }) {
        ASSERT_EQ(manager_.GetPageScene(CALLER_PID, request), NO_ERROR);
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), LAST_UNFINISH);
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), LAST_UNFINISH);
        EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
        manager_.CompleteGetPageScene(CALLER_PID);
        ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
        manager_.CancelSplitPageSceneQuery(CALLER_PID, QueryGeneration(queries_.back()));
    }
}

HWTEST_F(UiSessionPageSceneTest, UnifiedQueryStateRejectsEmptySplitOwnership, TestSize.Level1)
{
    manager_.CompleteGetPageScene(CALLER_PID);
    EXPECT_TRUE(manager_.pageSceneQueries_.empty());
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), NO_ERROR);
    EXPECT_FALSE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, ""));
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    manager_.CancelSplitPageSceneQuery(CALLER_PID, "");
    manager_.ReportSplitPageSceneQueryResult(CALLER_PID, "", "");
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), LAST_UNFINISH);
    manager_.CompleteGetPageScene(CALLER_PID);
    manager_.CompleteGetPageScene(CALLER_PID);
    EXPECT_TRUE(manager_.pageSceneQueries_.empty());
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, RepeatedConnectPreservesRegistration, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    sptr<UiReportStub> replacement = std::make_shared<UiReportStub>();
    manager_.SaveReportProxy(replacement, CALLER_PID);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, ReplacedConnectionDeathKeepsPidCleanupAndRemovesSplit, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(MIXED_ID, true)), NO_ERROR);
    sptr<UiReportStub> replacement = std::make_shared<UiReportStub>();
    manager_.SaveReportProxy(replacement, CALLER_PID);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), NO_ERROR);
    report_->NotifyDeath();
    EXPECT_TRUE(manager_.pageSceneQueries_.empty());
    EXPECT_FALSE(manager_.reportObjectMap_.count(CALLER_PID) != 0);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_FALSE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_EQ(manager_.pageSceneRuleSets_.count(CALLER_PID), 0u);
    EXPECT_TRUE(webOwner_.empty());
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NOT_CONNECTED);
}

HWTEST_F(UiSessionPageSceneTest, DeathCleanupRemovesRulesAndQueryForOnlyOneCaller, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(MIXED_ID, true)), NO_ERROR);
    sptr<TestPageSceneReportStub> otherReport = std::make_shared<TestPageSceneReportStub>();
    manager_.SaveReportProxy(otherReport, OTHER_PID);
    ASSERT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, LEGACY_JSON), NO_ERROR);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), NO_ERROR);
    report_->NotifyDeath();
    EXPECT_FALSE(manager_.reportObjectMap_.count(CALLER_PID) != 0);
    EXPECT_TRUE(manager_.reportObjectMap_.count(OTHER_PID) != 0);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_FALSE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(OTHER_PID).ruleSetId, "legacy");
    EXPECT_TRUE(webOwner_.empty());
    EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeSplitOnlyRule()), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, PureTextRegistrationRetainsLegacyPayload, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), NO_ERROR);
    ASSERT_EQ(detections_.size(), 1u);
    auto request = JsonUtil::ParseJsonString(detections_.front());
    EXPECT_EQ(request->GetString("ruleSetId"), "legacy");
    EXPECT_FALSE(request->Contains("_splitBatch"));
    EXPECT_EQ(request->GetValue("rules")->GetArraySize(), 1);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, "legacy"), NO_ERROR);
    EXPECT_TRUE(QueryGeneration(queries_.back()).empty());
}

HWTEST_F(UiSessionPageSceneTest, EmptyLegacyWebResultReleasesQuery, TestSize.Level1)
{
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), NO_ERROR);
    manager_.ReportPageSceneEvent(CALLER_PID, "", true);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), NO_ERROR);
}
HWTEST_F(UiSessionPageSceneTest, NativeRulesArePublishedBeforeWebRegistration, TestSize.Level1)
{
    manager_.SaveWebPageSceneFunction([this](UiSessionManager::WebPageSceneOp, int32_t, const std::string&, bool) {
        EXPECT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
        EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
        manager_.NotifyPageSceneNodeChanged("TextInput", true);
        manager_.FlushPageSceneNodeChanged();
        EXPECT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT + SINGLE_TEXT_RESULT_COUNT);
        return NO_ERROR;
    });
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(MIXED_ID, true)), NO_ERROR);
    EXPECT_EQ(JsonUtil::ParseJsonString(detections_.back())->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"),
        INPUT_RULE);
}

HWTEST_F(UiSessionPageSceneTest, RejectedRegistrationPreservesPendingTextRequest, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    const auto previous = manager_.GetSplitPageSceneRulesForDetection(CALLER_PID);
    ASSERT_EQ(previous.size(), SINGLE_TEXT_RESULT_COUNT);
    const auto detections = detections_.size();
    manager_.NotifyPageSceneNodeChanged("TextInput", true);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(SECOND_ID)), PARAM_INVALID);
    EXPECT_EQ(detections_.size(), detections);
    EXPECT_EQ(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID), previous);
    ClearDetections();
    manager_.FlushPageSceneNodeChanged();
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    auto text = JsonUtil::ParseJsonString(detections_.front());
    EXPECT_EQ(text->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), INPUT_RULE);
}

HWTEST_F(UiSessionPageSceneTest, SplitRegistrationIsUniqueAcrossCallersAndIds, TestSize.Level1)
{
    manager_.SaveReportProxy(report_, OTHER_PID);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    const auto original = manager_.splitPageSceneRegistration_->ruleSet;
    const auto textPending = manager_.pendingPageSceneDetectRules_;
    const auto batches = detections_.size();
    for (auto pid : { CALLER_PID, OTHER_PID }) {
        EXPECT_EQ(manager_.RegisterPageSceneRules(pid, MIXED_JSON), PARAM_INVALID);
        auto changed = JsonUtil::ParseJsonString(MakeMixedRule(SECOND_ID));
        changed->GetValue("rules")->GetArrayItem(0)->Replace("ruleId", "another_split");
        EXPECT_EQ(manager_.RegisterPageSceneRules(pid, changed->ToString()), PARAM_INVALID);
    }
    EXPECT_EQ(detections_.size(), batches);
    ASSERT_TRUE(manager_.splitPageSceneRegistration_.has_value());
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ownerPid, CALLER_PID);
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ruleSet.ruleJson, original.ruleJson);
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ruleSet.ruleJson, original.ruleJson);
    EXPECT_EQ(manager_.pendingPageSceneDetectRules_, textPending);
}

HWTEST_F(UiSessionPageSceneTest, MultipleSplitEntriesRejectWholeRegistrationButRemainQueryable, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(MIXED_JSON);
    auto rules = root->GetValue("rules");
    auto extra = JsonUtil::ParseJsonString(rules->GetArrayItem(0)->ToString());
    extra->Replace("ruleId", "second_split");
    extra->Replace("enabled", false);
    rules->Put(extra);
    const auto json = root->ToString();
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, json), PARAM_INVALID);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_.has_value());
    EXPECT_TRUE(detections_.empty());
    EXPECT_FALSE(manager_.GetPageSceneRulesRegistered());
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, json), NO_ERROR);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_.has_value());
    ASSERT_EQ(RequestThroughProxy(CALLER_PID, MIXED_JSON), NO_ERROR);
    ReportSplitQueryResult(queries_.back());
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
}

HWTEST_F(UiSessionPageSceneTest, DisabledSplitAndDisabledSourceStillReserveProcessSlot, TestSize.Level1)
{
    manager_.SaveReportProxy(report_, OTHER_PID);
    for (bool disableRule : { true, false }) {
        auto json = JsonUtil::ParseJsonString(MIXED_JSON);
        if (disableRule) {
            json->GetValue("rules")->GetArrayItem(0)->Replace("enabled", false);
        } else {
            json->GetValue("sourceConfig")->Replace("arkui", false);
        }
        ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, json->ToString()), NO_ERROR);
        EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeMixedRule(SECOND_ID)), PARAM_INVALID);
        ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    }
    EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeMixedRule(SECOND_ID)), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, NonOwnerCleanupCannotReleaseSplitSlot, TestSize.Level1)
{
    sptr<TestPageSceneReportStub> otherReport = std::make_shared<TestPageSceneReportStub>();
    manager_.SaveReportProxy(otherReport, OTHER_PID);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    const auto original = manager_.splitPageSceneRegistration_->ruleSet;
    EXPECT_EQ(manager_.UnregisterPageSceneRules(OTHER_PID, MIXED_ID), PARAM_INVALID);
    EXPECT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, SECOND_ID), PARAM_INVALID);
    otherReport->NotifyDeath();
    EXPECT_FALSE(manager_.reportObjectMap_.count(OTHER_PID) != 0);
    manager_.SaveReportProxy(otherReport, OTHER_PID);
    EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeMixedRule(SECOND_ID)), PARAM_INVALID);
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ruleSet.ruleJson, original.ruleJson);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MIXED_JSON), NO_ERROR);
    const auto replacement = manager_.splitPageSceneRegistration_->ruleSet.ruleJson;
    report_->NotifyDeath();
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ownerPid, OTHER_PID);
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ruleSet.ruleJson, replacement);
}

HWTEST_F(UiSessionPageSceneTest, PendingWebRegistrationAndFailureKeepSplitSlot, TestSize.Level1)
{
    manager_.SaveReportProxy(report_, OTHER_PID);
    int calls = 0;
    manager_.SaveWebPageSceneFunction(
        [this, &calls](UiSessionManager::WebPageSceneOp op, int32_t, const std::string&, bool) {
            ++calls;
            EXPECT_EQ(op, UiSessionManager::WebPageSceneOp::RegisterRules);
            EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
            EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeMixedRule(SECOND_ID)), PARAM_INVALID);
            return FAILED;
        });
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(MIXED_ID, true)), FAILED);
    EXPECT_EQ(calls, 1);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeMixedRule(SECOND_ID)), PARAM_INVALID);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeMixedRule(SECOND_ID)), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, ConcurrentCallersCanCommitOnlyOneSplitRegistration, TestSize.Level1)
{
    manager_.SaveReportProxy(report_, OTHER_PID);
    std::atomic<int> batches { 0 };
    manager_.SavePageSceneDetectFunction([&batches](int32_t, const std::string&, bool) { ++batches; });
    std::promise<void> start;
    auto ready = start.get_future().share();
    int32_t first = FAILED;
    int32_t second = FAILED;
    const auto other = MakeMixedRule(SECOND_ID);
    std::thread a([&ready, &first, this]() {
        ready.wait();
        first = manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON);
    });
    std::thread b([&ready, &second, &other, this]() {
        ready.wait();
        second = manager_.RegisterPageSceneRules(OTHER_PID, other);
    });
    start.set_value();
    a.join();
    b.join();
    EXPECT_EQ(std::min(first, second), NO_ERROR);
    EXPECT_EQ(std::max(first, second), PARAM_INVALID);
    EXPECT_EQ(static_cast<size_t>(batches.load()), INITIAL_SCENE_REQUEST_COUNT);
    ASSERT_TRUE(manager_.splitPageSceneRegistration_.has_value());
    const auto owner = first == NO_ERROR ? CALLER_PID : OTHER_PID;
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ownerPid, owner);
}

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

HWTEST_F(UiSessionPageSceneTest, ExistingTextRejectsMixedBeforeReservingSplit, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), NO_ERROR);
    const auto original = manager_.pageSceneRuleSets_.at(CALLER_PID).ruleJson;
    detections_.clear();
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), LAST_UNFINISH);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleJson, original);
    EXPECT_TRUE(detections_.empty());
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, MixedTextSlotRejectsLegacyUntilWholeSetIsRemoved, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    const auto generation = manager_.pageSceneRuleSets_.at(CALLER_PID).ruleJson;
    const auto calls = detections_.size();
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
    EXPECT_EQ(detections_.size(), calls);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleJson, generation);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_EQ(manager_.pageSceneRuleSets_.count(CALLER_PID), 0u);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, PureSplitDoesNotOccupyTextSlot, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    EXPECT_EQ(manager_.pageSceneRuleSets_.count(CALLER_PID), 0u);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), NO_ERROR);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleSetId, "legacy");
    EXPECT_TRUE(manager_.GetPageSceneRulesRegistered());
}

HWTEST_F(UiSessionPageSceneTest, DeferredSplitDispatchCannotRestoreUnregisteredRules, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    UiSessionManagerOhos::PageSceneRegistrationPlan accepted;
    accepted.splitRules = manager_.splitPageSceneRegistration_->ruleSet;
    const auto batch = detections_.back();
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    ClearDetections();
    EXPECT_EQ(manager_.CompletePageSceneRegistration(CALLER_PID, accepted), NO_ERROR);
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_EQ(detections_.back(), batch);
    EXPECT_FALSE(JsonUtil::ParseJsonString(batch)->Contains("_splitGeneration"));
    EXPECT_FALSE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_TRUE(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).empty());
}

HWTEST_F(UiSessionPageSceneTest, SplitNotificationDoesNotCopyRulePayload, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    const auto notification = detections_.back();
    manager_.NotifyPageSceneContentChanged();
    manager_.FlushPageSceneNodeChanged();
    EXPECT_EQ(detections_.back(), notification);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_EQ(detections_.back(), notification);

    auto largeRule = JsonUtil::ParseJsonString(MakeSplitOnlyRule(SECOND_ID));
    constexpr size_t DESCRIPTION_SIZE = 4096;
    largeRule->Put("description", std::string(DESCRIPTION_SIZE, 'x').c_str());
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, largeRule->ToString()), NO_ERROR);
    const auto current = manager_.GetSplitPageSceneRulesForDetection(CALLER_PID);
    ASSERT_EQ(current.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_GT(current.front().size(), DESCRIPTION_SIZE);
    EXPECT_EQ(detections_.back(), notification);
}

HWTEST_F(UiSessionPageSceneTest, SplitDetectionUsesCurrentRegistrationAfterReregister, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    const auto original = manager_.GetSplitPageSceneRulesForDetection(CALLER_PID);
    ASSERT_EQ(original.size(), SINGLE_TEXT_RESULT_COUNT);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_TRUE(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).empty());

    auto next = JsonUtil::ParseJsonString(MakeSplitOnlyRule(SECOND_ID));
    next->GetValue("rules")->GetArrayItem(SPLIT_RULE_INDEX)->GetValue("custom")
        ->GetValue("selectors")->GetArrayItem(0)->Replace("compName", "Row");
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, next->ToString()), NO_ERROR);
    const auto current = manager_.GetSplitPageSceneRulesForDetection(CALLER_PID);
    ASSERT_EQ(current.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_NE(current, original);
    auto rules = JsonUtil::ParseJsonString(current.front());
    EXPECT_EQ(rules->GetString("ruleSetId"), SECOND_ID);
    EXPECT_EQ(rules->GetValue("rules")->GetArrayItem(SPLIT_RULE_INDEX)->GetValue("custom")
        ->GetValue("selectors")->GetArrayItem(0)->GetString("compName"), "Row");
    EXPECT_TRUE(manager_.GetSplitPageSceneRulesForDetection(OTHER_PID).empty());
}

HWTEST_F(UiSessionPageSceneTest, SplitDetectionDoesNotRetainDeadOwnersRules, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    ASSERT_FALSE(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).empty());
    report_->NotifyDeath();
    EXPECT_TRUE(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).empty());

    sptr<TestPageSceneReportStub> otherReport = std::make_shared<TestPageSceneReportStub>();
    manager_.SaveReportProxy(otherReport, OTHER_PID);
    ASSERT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeSplitOnlyRule(SECOND_ID)), NO_ERROR);
    EXPECT_TRUE(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).empty());
    const auto current = manager_.GetSplitPageSceneRulesForDetection(OTHER_PID);
    ASSERT_EQ(current.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_EQ(JsonUtil::ParseJsonString(current.front())->GetString("ruleSetId"), SECOND_ID);
}

HWTEST_F(UiSessionPageSceneTest, LegacyMultipleRulesKeepIndependentInitialRequests, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeTextOnlyRules()), NO_ERROR);
    ASSERT_EQ(detections_.size(), TEXT_RULE_COUNT);
    auto first = JsonUtil::ParseJsonString(detections_.front());
    auto second = JsonUtil::ParseJsonString(detections_.back());
    EXPECT_FALSE(first->Contains("_splitBatch"));
    EXPECT_FALSE(second->Contains("_splitBatch"));
    EXPECT_EQ(first->GetValue("rules")->GetArraySize(), 1);
    EXPECT_EQ(first->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), INPUT_RULE);
    EXPECT_EQ(second->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), AREA_RULE);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
}

HWTEST_F(UiSessionPageSceneTest, LegacyUnsupportedTextKeepsSupportedRuleDetection, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(MakeTextOnlyRules());
    root->GetValue("rules")->GetArrayItem(0)->Delete("condition");
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, root->ToString()), NO_ERROR);
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    auto detected = JsonUtil::ParseJsonString(detections_.front());
    EXPECT_EQ(detected->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"), AREA_RULE);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    detections_.clear();
    manager_.NotifyPageSceneNodeChanged("TextInput", true);
    manager_.FlushPageSceneNodeChanged();
    EXPECT_TRUE(detections_.empty());
    manager_.NotifyPageSceneNodeChanged("TextArea", true);
    manager_.FlushPageSceneNodeChanged();
    EXPECT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
}

HWTEST_F(UiSessionPageSceneTest, LegacyDisabledUnsupportedTextKeepsRegistrationSlot, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(LEGACY_JSON);
    auto rule = root->GetValue("rules")->GetArrayItem(0);
    rule->Put("enabled", false);
    rule->Delete("condition");
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, root->ToString()), NO_ERROR);
    EXPECT_TRUE(detections_.empty());
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
}

HWTEST_F(UiSessionPageSceneTest, LegacyWebBypassAllowsUnsupportedNativeText, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(MakeTextOnlyRules(true));
    root->GetValue("sourceConfig")->Replace("arkui", false);
    root->GetValue("rules")->GetArrayItem(0)->Delete("condition");
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, root->ToString()), NO_ERROR);
    EXPECT_EQ(webOwner_, SECOND_ID);
    EXPECT_TRUE(detections_.empty());
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
}

HWTEST_F(UiSessionPageSceneTest, LegacyWebReceivesOriginalUnfilteredRequest, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(MakeTextOnlyRules(true));
    auto unknown = JsonUtil::Create(true);
    unknown->Put("ruleId", "future");
    unknown->Put("sceneType", "FUTURE_SCENE");
    root->GetValue("rules")->Put(unknown);
    const auto json = std::string(" \n") + root->ToString() + "\n";
    std::string forwarded;
    manager_.SaveWebPageSceneFunction(
        [&](UiSessionManager::WebPageSceneOp op, int32_t, const std::string& request, bool) {
            EXPECT_EQ(op, UiSessionManager::WebPageSceneOp::RegisterRules);
            EXPECT_EQ(detections_.size(), TEXT_RULE_COUNT);
            forwarded = request;
            return NO_ERROR;
        });
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, json), NO_ERROR);
    EXPECT_EQ(forwarded, json);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleJson, json);
}

HWTEST_F(UiSessionPageSceneTest, LegacyWebTimingStillFollowsInitialNativeRequests, TestSize.Level1)
{
    int webCalls = 0;
    manager_.SaveWebPageSceneFunction([&](UiSessionManager::WebPageSceneOp op, int32_t, const std::string&, bool) {
        ++webCalls;
        EXPECT_EQ(op, UiSessionManager::WebPageSceneOp::RegisterRules);
        EXPECT_EQ(detections_.size(), TEXT_RULE_COUNT);
        return NO_ERROR;
    });
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeTextOnlyRules(true)), NO_ERROR);
    EXPECT_EQ(webCalls, 1);
}

HWTEST_F(UiSessionPageSceneTest, MixedWebFailureKeepsCommittedSplit, TestSize.Level1)
{
    manager_.SaveWebPageSceneFunction([&](UiSessionManager::WebPageSceneOp, int32_t, const std::string&, bool) {
        EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleSetId, MIXED_ID);
        EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
        EXPECT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
        return FAILED;
    });
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(MIXED_ID, true)), FAILED);
    EXPECT_EQ(manager_.pageSceneRuleSets_.count(CALLER_PID), 0u);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_TRUE(manager_.GetPageSceneRulesRegistered());
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), PARAM_INVALID);
}

HWTEST_F(UiSessionPageSceneTest, MixedMissingWebCallbackKeepsLegacyAndSplit, TestSize.Level1)
{
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeMixedRule(MIXED_ID, true)), FAILED);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleSetId, MIXED_ID);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
    EXPECT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, FailedMixedValidationCannotReserveOriginalTextSlot, TestSize.Level1)
{
    auto json = JsonUtil::ParseJsonString(MIXED_JSON);
    json->GetValue("rules")->GetArrayItem(SPLIT_RULE_INDEX)->GetValue("custom")->Delete("geometry");
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, json->ToString()), PARAM_INVALID);
    EXPECT_EQ(manager_.pageSceneRuleSets_.count(CALLER_PID), 0u);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_TRUE(detections_.empty());
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, LEGACY_JSON), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, MixedMissingTextConditionUsesOriginalFiltering, TestSize.Level1)
{
    auto json = JsonUtil::ParseJsonString(MIXED_JSON);
    json->GetValue("rules")->GetArrayItem(FIRST_TEXT_RULE_INDEX)->Delete("condition");
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, json->ToString()), NO_ERROR);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    ASSERT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
    EXPECT_EQ(JsonUtil::ParseJsonString(detections_.front())->GetValue("rules")->GetArrayItem(0)->GetString("ruleId"),
        AREA_RULE);
}

HWTEST_F(UiSessionPageSceneTest, MixedContentChangeDoesNotDuplicateLegacyDispatch, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ClearDetections();
    manager_.NotifyPageSceneContentChanged();
    manager_.FlushPageSceneNodeChanged();
    ASSERT_EQ(detections_.size(), CONTENT_CHANGE_REQUEST_COUNT);
    size_t textCount = 0;
    for (const auto& request : detections_) {
        auto root = JsonUtil::ParseJsonString(request);
        if (!root->GetBool("_splitBatch")) {
            ++textCount;
            EXPECT_EQ(root->GetValue("rules")->GetArrayItem(0)->GetString("sceneType"), "TEXT_EDITOR");
        }
    }
    EXPECT_EQ(textCount, TEXT_RULE_COUNT);
    EXPECT_TRUE(JsonUtil::ParseJsonString(detections_.back())->GetBool("_splitBatch"));
}

HWTEST_F(UiSessionPageSceneTest, MixedStateChangeDoesNotDuplicateLegacyDispatch, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ClearDetections();
    manager_.NotifyPageSceneNodeStateChanged("Column", UiSessionManager::PageSceneNodeStateChange::VISIBILITY);
    manager_.FlushPageSceneNodeChanged();
    ASSERT_EQ(detections_.size(), TEXT_RULE_COUNT);
    for (const auto& request : detections_) {
        EXPECT_FALSE(JsonUtil::ParseJsonString(request)->GetBool("_splitBatch"));
    }
}

HWTEST_F(UiSessionPageSceneTest, LegacyMissingWebCallbackPreservesBaselineRegistration, TestSize.Level1)
{
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeTextOnlyRules(true)), FAILED);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleSetId, SECOND_ID);
    EXPECT_EQ(detections_.size(), TEXT_RULE_COUNT);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
}

HWTEST_F(UiSessionPageSceneTest, LegacyConnectionErrorStillPrecedesInvalidInput, TestSize.Level1)
{
    EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, "invalid"), NOT_CONNECTED);
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
    EXPECT_TRUE(detections_.empty());
}

HWTEST_F(UiSessionPageSceneTest, RegistrationPlanKeepsScenesIndependent, TestSize.Level1)
{
    UiSessionManagerOhos::PageSceneRegistrationPlan splitPlan;
    manager_.PrepareConnectedRegistration(MakeSplitOnlyRule(), splitPlan);
    EXPECT_EQ(splitPlan.splitRules.ruleSetId, MIXED_ID);
    EXPECT_FALSE(splitPlan.legacyRules.has_value());
    UiSessionManagerOhos::PageSceneRegistrationPlan mixedPlan;
    manager_.PrepareConnectedRegistration(MIXED_JSON, mixedPlan);
    EXPECT_EQ(mixedPlan.splitRules.ruleSetId, MIXED_ID);
    ASSERT_TRUE(mixedPlan.legacyRules.has_value());
    EXPECT_EQ(mixedPlan.legacyRules->rules.size(), TEXT_RULE_COUNT);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
}

HWTEST_F(UiSessionPageSceneTest, TextTriggersDoNotMutateSplitRegistration, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ClearDetections();
    const auto split = manager_.splitPageSceneRegistration_->ruleSet;
    manager_.NotifyPageSceneNodeChanged("TextInput", true);
    manager_.FlushPageSceneNodeChanged();
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_FALSE(JsonUtil::ParseJsonString(detections_.front())->GetBool("_splitBatch"));
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ruleSet.ruleJson, split.ruleJson);
    EXPECT_EQ(manager_.splitPageSceneRegistration_->ruleSet.ruleJson, split.ruleJson);
}

HWTEST_F(UiSessionPageSceneTest, MixedQueryReportsTextAndSplitThroughIndependentPayloads, TestSize.Level1)
{
    std::vector<std::string> reports;
    report_->RegisterGetPageSceneCallback([&reports](const std::string& json) { reports.push_back(json); });
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_JSON), NO_ERROR);
    ASSERT_EQ(queries_.size(), SCENE_QUERY_REQUEST_COUNT);
    const auto split = queries_.back();
    const std::string text = R"({"ruleSetId":"mixed","ruleId":"input","sceneType":"TEXT_EDITOR","matched":true})";
    manager_.ReportPageSceneEvent(CALLER_PID, text, true);
    ASSERT_EQ(reports.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_EQ(reports.front(), text);
    EXPECT_TRUE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    ASSERT_EQ(manager_.pageSceneQueries_.size(), SINGLE_QUERY_COUNT);
    EXPECT_FALSE(manager_.pageSceneQueries_.at(CALLER_PID).legacyPending);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), LAST_UNFINISH);
    ReportSplitQueryResult(split);
    ASSERT_EQ(reports.size(), SCENE_QUERY_REQUEST_COUNT);
    EXPECT_TRUE(JsonUtil::ParseJsonString(reports.back())->Contains("events"));
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_TRUE(manager_.pageSceneQueries_.empty());
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, LEGACY_JSON), NO_ERROR);
    manager_.CompleteGetPageScene(CALLER_PID);
}

HWTEST_F(UiSessionPageSceneTest, MissingDetectorReleasesEachQueryPath, TestSize.Level1)
{
    manager_.SavePageSceneDetectFunction({});
    for (const auto& request : { LEGACY_JSON, MakeSplitOnlyRule(), MIXED_JSON }) {
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, request), NO_ERROR);
        EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
        EXPECT_TRUE(manager_.pageSceneQueries_.empty());
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, request), NO_ERROR);
    }
    EXPECT_TRUE(queries_.empty());
}

HWTEST_F(UiSessionPageSceneTest, MixedUnregisterCleansEachDetectionPath, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    ClearDetections();
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    ASSERT_EQ(detections_.size(), INITIAL_SCENE_REQUEST_COUNT);
    auto text = JsonUtil::ParseJsonString(detections_.front());
    EXPECT_FALSE(text->GetBool("_splitBatch"));
    EXPECT_EQ(text->GetValue("rules")->GetArraySize(), 0);
    auto split = JsonUtil::ParseJsonString(detections_.back());
    EXPECT_TRUE(split->GetBool("_splitBatch"));
    EXPECT_TRUE(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).empty());
}


HWTEST_F(UiSessionPageSceneTest, WebOnlyWithoutNativeRulesKeepsBaselineLifecycle, TestSize.Level1)
{
    InstallWebCallback();
    for (bool emptyNativeArray : { false, true }) {
        const auto json = MakeWebOnlyRule(emptyNativeArray);
        ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, json), NO_ERROR);
        EXPECT_EQ(webOwner_, MIXED_ID);
        EXPECT_TRUE(manager_.pageSceneRuleSets_.at(CALLER_PID).rules.empty());
        EXPECT_FALSE(manager_.splitPageSceneRegistration_);
        EXPECT_TRUE(detections_.empty());
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), NO_ERROR);
        EXPECT_EQ(webQueries_.back(), MIXED_ID);
        EXPECT_TRUE(queries_.empty());
        manager_.CompleteGetPageScene(CALLER_PID);
        EXPECT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
        EXPECT_TRUE(webOwner_.empty());
        ClearDetections();
        EXPECT_EQ(manager_.GetPageScene(CALLER_PID, json), NO_ERROR);
        EXPECT_EQ(webQueries_.back(), json);
        manager_.CompleteGetPageScene(CALLER_PID);
    }
}

HWTEST_F(UiSessionPageSceneTest, SplitAndWebWithoutTextKeepsBothLifecycles, TestSize.Level1)
{
    InstallWebCallback();
    const auto json = MakeSplitOnlyRule(MIXED_ID, true);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, json), NO_ERROR);
    EXPECT_EQ(webOwner_, MIXED_ID);
    const auto& legacy = manager_.pageSceneRuleSets_.at(CALLER_PID);
    EXPECT_TRUE(legacy.rules.empty());
    EXPECT_TRUE(legacy.webEnabled);
    auto root = JsonUtil::ParseJsonString(legacy.ruleJson);
    EXPECT_EQ(root->GetValue("rules")->GetArraySize(), 0);
    EXPECT_EQ(root->GetValue("webRules")->ToString(),
        JsonUtil::ParseJsonString(json)->GetValue("webRules")->ToString());
    ASSERT_EQ(detections_.size(), SINGLE_TEXT_RESULT_COUNT);
    for (const auto& request : { MIXED_ID, json }) {
        ASSERT_EQ(manager_.GetPageScene(CALLER_PID, request), NO_ERROR);
        EXPECT_EQ(webQueries_.back(), request);
        ReportSplitQueryResult(queries_.back());
        EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
        EXPECT_TRUE(manager_.pageSceneQueries_.count(CALLER_PID));
        manager_.CompleteGetPageScene(CALLER_PID);
        EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    }
    EXPECT_EQ(queries_.size(), SCENE_QUERY_REQUEST_COUNT);
    EXPECT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_TRUE(webOwner_.empty());
    EXPECT_EQ(webRemovals_, 1);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
}

HWTEST_F(UiSessionPageSceneTest, WebOnlyAndSplitWebInvalidPayloadUseOriginalWebValidator, TestSize.Level1)
{
    InstallWebCallback();
    webRegisterResult_ = PARAM_INVALID;
    auto root = JsonUtil::ParseJsonString(MakeSplitOnlyRule(MIXED_ID, true));
    root->Delete("webRules");
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, root->ToString()), PARAM_INVALID);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
    EXPECT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeWebOnlyRule(false)), PARAM_INVALID);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
}

HWTEST_F(UiSessionPageSceneTest, SplitCommonCapacityCountsAllExistingRuleSets, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule(SECOND_ID)), NO_ERROR);
    ClearDetections();
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), PARAM_INVALID);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_TRUE(detections_.empty());
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, SECOND_ID), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleSetId, "legacy");
}

HWTEST_F(UiSessionPageSceneTest, SplitCapacityRejectsTwoSearchSetsButDoesNotCountOtherCaller, TestSize.Level1)
{
    manager_.SaveReportProxy(report_, OTHER_PID);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule("first")), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule(SECOND_ID)), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), PARAM_INVALID);
    EXPECT_EQ(manager_.RegisterPageSceneRules(OTHER_PID, MakeSplitOnlyRule()), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, SplitCapacityCountsLegacyAndSearchIdentityOnlyOnce, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, LEGACY_JSON), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule("legacy")), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
}

HWTEST_F(UiSessionPageSceneTest, LegacySearchCapacityIsNotTightenedBySplit, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule("first")), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule(SECOND_ID)), NO_ERROR);
    EXPECT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule("third")), PARAM_INVALID);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
}

HWTEST_F(UiSessionPageSceneTest, SplitRuleCountRejectsElevenButLegacyCountIsUnchanged, TestSize.Level1)
{
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, MakeRuleCount(MAX_COMMON_RULES + 1, true)), PARAM_INVALID);
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    ASSERT_EQ(RequestThroughProxy(CALLER_PID, MakeRuleCount(MAX_COMMON_RULES, true)), NO_ERROR);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, MakeRuleCount(MAX_COMMON_RULES + 1, false)), NO_ERROR);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).rules.size(), static_cast<size_t>(MAX_COMMON_RULES + 1));
}

HWTEST_F(UiSessionPageSceneTest, SplitRejectsDuplicateIdsAcrossScenesWithoutChangingLegacyValidation, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(MIXED_JSON);
    root->GetValue("rules")->GetArrayItem(FIRST_TEXT_RULE_INDEX)->Replace("ruleId", "split");
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, root->ToString()), PARAM_INVALID);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
    root = JsonUtil::ParseJsonString(MakeTextOnlyRules());
    root->GetValue("rules")->GetArrayItem(0)->Replace("ruleId", AREA_RULE.c_str());
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, root->ToString()), NO_ERROR);
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).rules.size(), TEXT_RULE_COUNT);
}

HWTEST_F(UiSessionPageSceneTest, SplitShapeValidationStillPrecedesCapacity, TestSize.Level1)
{
    ASSERT_EQ(RequestThroughProxy(CALLER_PID, MakeSearchRule("first")), NO_ERROR);
    ASSERT_EQ(RequestThroughProxy(CALLER_PID, MakeSearchRule(SECOND_ID)), NO_ERROR);
    auto root = JsonUtil::ParseJsonString(MakeSplitOnlyRule());
    root->GetValue("rules")->GetArrayItem(SPLIT_RULE_INDEX)->GetValue("custom")->Delete("geometry");
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, root->ToString()), PARAM_INVALID);
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_EQ(manager_.searchPageSceneRuleSets_.at(CALLER_PID).size(), TEXT_RULE_COUNT);
}


HWTEST_F(UiSessionPageSceneTest, MixedRegistrationFitsRemainingCommonSlot, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSearchRule(SECOND_ID)), NO_ERROR);
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MIXED_JSON), NO_ERROR);
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_EQ(manager_.pageSceneRuleSets_.at(CALLER_PID).ruleSetId, MIXED_ID);
    EXPECT_EQ(manager_.searchPageSceneRuleSets_.at(CALLER_PID).count(SECOND_ID), SINGLE_TEXT_RESULT_COUNT);
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_TRUE(manager_.GetPageSceneRulesRegistered());
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
    EXPECT_EQ(manager_.searchPageSceneRuleSets_.at(CALLER_PID).count(SECOND_ID), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, SECOND_ID), NO_ERROR);
    EXPECT_EQ(JsonUtil::ParseJsonString(queries_.back())->GetString("ruleSetId"), SECOND_ID);
    manager_.CompleteGetPageScene(CALLER_PID);
}

HWTEST_F(UiSessionPageSceneTest, MissingWebCallbackKeepsOriginalLegacyPending, TestSize.Level1)
{
    const auto request = MakeSplitOnlyRule(MIXED_ID, true);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, request), NO_ERROR);
    ASSERT_EQ(queries_.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_TRUE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, request), LAST_UNFINISH);
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, request), LAST_UNFINISH);
    manager_.CompleteGetPageScene(CALLER_PID);
    EXPECT_EQ(manager_.GetPageScene(CALLER_PID, request), NO_ERROR);
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_FALSE(manager_.GetPageSceneRulesRegistered());
}

HWTEST_F(UiSessionPageSceneTest, WebRegistrationFailureKeepsSplitIdQueryable, TestSize.Level1)
{
    InstallWebCallback();
    webRegisterResult_ = FAILED;
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule(MIXED_ID, true)), FAILED);
    ASSERT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), NO_ERROR);
    ASSERT_EQ(queries_.size(), SINGLE_TEXT_RESULT_COUNT);
    EXPECT_TRUE(webQueries_.empty());
    EXPECT_EQ(JsonUtil::ParseJsonString(queries_.back())->GetValue("rules")->GetArrayItem(0)->GetString("sceneType"),
        "SPLIT");
    ReportSplitQueryResult(queries_.back());
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_TRUE(manager_.HasSplitPageSceneRuleSet(CALLER_PID, MIXED_ID));
}

HWTEST_F(UiSessionPageSceneTest, DelayedSubscriptionUsesLiveRecipientWithoutRuleCheck, TestSize.Level1)
{
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    const std::string event = R"({"events":[{"ruleSetId":"mixed","sceneType":"SPLIT","matched":true}]})";
    size_t reports = 0;
    report_->RegisterPageSceneEventCallback(MIXED_ID, [&reports](const std::string&) { ++reports; });
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    manager_.ReportPageSceneEvent(CALLER_PID, event, false);
    EXPECT_EQ(reports, SINGLE_TEXT_RESULT_COUNT);
    report_->NotifyDeath();
    manager_.ReportPageSceneEvent(CALLER_PID, event, false);
    EXPECT_EQ(reports, SINGLE_TEXT_RESULT_COUNT);
}

HWTEST_F(UiSessionPageSceneTest, QueryGenerationsCannotCrossCallerBoundaries, TestSize.Level1)
{
    manager_.SaveReportProxy(report_, OTHER_PID);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
    const auto first = queries_.back();
    ASSERT_EQ(manager_.GetPageScene(OTHER_PID, MakeSplitOnlyRule()), NO_ERROR);
    const auto second = queries_.back();
    const auto firstGeneration = QueryGeneration(first);
    const auto secondGeneration = QueryGeneration(second);
    ASSERT_NE(firstGeneration, secondGeneration);
    manager_.ReportSplitPageSceneQueryResult(OTHER_PID, QueryResponse(first), firstGeneration);
    manager_.ReportSplitPageSceneQueryResult(CALLER_PID, QueryResponse(second), secondGeneration);
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, firstGeneration));
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(OTHER_PID, secondGeneration));
    ReportSplitQueryResult(first);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_TRUE(manager_.HasPendingSplitPageSceneQuery(OTHER_PID));
    manager_.ReportSplitPageSceneQueryResult(OTHER_PID, QueryResponse(second), secondGeneration);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(OTHER_PID));
}

HWTEST_F(UiSessionPageSceneTest, UnregisterSubscriptionDoesNotCancelAcceptedQuery, TestSize.Level1)
{
    InstallWebCallback();
    ASSERT_EQ(manager_.RegisterPageSceneRules(CALLER_PID, MakeSplitOnlyRule(MIXED_ID, true)), NO_ERROR);
    ASSERT_EQ(manager_.GetPageScene(CALLER_PID, MIXED_ID), NO_ERROR);
    const auto request = queries_.back();
    ASSERT_EQ(manager_.UnregisterPageSceneRules(CALLER_PID, MIXED_ID), NO_ERROR);
    EXPECT_FALSE(manager_.GetPageSceneRulesRegistered());
    EXPECT_TRUE(manager_.GetSplitPageSceneRulesForDetection(CALLER_PID).empty());
    EXPECT_TRUE(manager_.IsSplitPageSceneQueryCurrent(CALLER_PID, QueryGeneration(request)));
    ReportSplitQueryResult(request);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_TRUE(manager_.pageSceneQueries_.empty());
    manager_.CompleteGetPageScene(CALLER_PID);
    EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    EXPECT_TRUE(manager_.pageSceneRuleSets_.empty());
    EXPECT_FALSE(manager_.splitPageSceneRegistration_);
}

HWTEST_F(UiSessionPageSceneTest, InvalidSelectorTagsRejectRegistrationAndQueryWithoutState, TestSize.Level1)
{
    for (const auto* tag : { "column", "width", "MyPane", "ForEach", "Span", "Column " }) {
        auto root = JsonUtil::ParseJsonString(MakeSplitOnlyRule());
        auto rule = root->GetValue("rules")->GetArrayItem(SPLIT_RULE_INDEX);
        rule->GetValue("custom")->GetValue("selectors")->GetArrayItem(0)->Replace("compName", tag);
        EXPECT_EQ(RequestThroughProxy(CALLER_PID, root->ToString()), PARAM_INVALID) << tag;
        EXPECT_EQ(RequestThroughProxy(CALLER_PID, root->ToString(), true), PARAM_INVALID) << tag;
        EXPECT_FALSE(manager_.splitPageSceneRegistration_);
        EXPECT_FALSE(manager_.HasPendingSplitPageSceneQuery(CALLER_PID));
    }
    EXPECT_TRUE(detections_.empty());
    EXPECT_TRUE(queries_.empty());
    EXPECT_EQ(RequestThroughProxy(CALLER_PID, MakeSplitOnlyRule()), NO_ERROR);
}

} // namespace OHOS::Ace
