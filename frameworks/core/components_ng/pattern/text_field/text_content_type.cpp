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

#include "core/components_ng/pattern/text_field/text_content_type.h"

#include <unordered_map>

#include "base/view_data/ace_auto_fill_type.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::NG {
TextContentType CastToTextContentType(int32_t value)
{
    if (value < static_cast<int32_t>(TextContentType::BEGIN) || value > static_cast<int32_t>(TextContentType::END)) {
        return TextContentType::BEGIN;
    }
    return static_cast<TextContentType>(value);
}

static std::unordered_map<TextContentType, std::pair<AceAutoFillType, std::string>> contentTypeMap_ = {
    {TextContentType::VISIBLE_PASSWORD,
        std::make_pair(AceAutoFillType::ACE_PASSWORD, "TextContentType.VISIBLE_PASSWORD")},
    {TextContentType::USER_NAME, std::make_pair(AceAutoFillType::ACE_USER_NAME, "TextContentType.USER_NAME")},
    {TextContentType::NEW_PASSWORD, std::make_pair(AceAutoFillType::ACE_NEW_PASSWORD, "TextContentType.NEW_PASSWORD")},
    {TextContentType::FULL_STREET_ADDRESS,
        std::make_pair(AceAutoFillType::ACE_FULL_STREET_ADDRESS, "TextContentType.FULL_STREET_ADDRESS")},
    {TextContentType::HOUSE_NUMBER, std::make_pair(AceAutoFillType::ACE_HOUSE_NUMBER, "TextContentType.HOUSE_NUMBER")},
    {TextContentType::DISTRICT_ADDRESS,
        std::make_pair(AceAutoFillType::ACE_DISTRICT_ADDRESS, "TextContentType.DISTRICT_ADDRESS")},
    {TextContentType::CITY_ADDRESS, std::make_pair(AceAutoFillType::ACE_CITY_ADDRESS, "TextContentType.CITY_ADDRESS")},
    {TextContentType::PROVINCE_ADDRESS,
        std::make_pair(AceAutoFillType::ACE_PROVINCE_ADDRESS, "TextContentType.PROVINCE_ADDRESS")},
    {TextContentType::COUNTRY_ADDRESS,
        std::make_pair(AceAutoFillType::ACE_COUNTRY_ADDRESS, "TextContentType.COUNTRY_ADDRESS")},
    {TextContentType::PERSON_FULL_NAME,
        std::make_pair(AceAutoFillType::ACE_PERSON_FULL_NAME, "TextContentType.PERSON_FULL_NAME")},
    {TextContentType::PERSON_LAST_NAME,
        std::make_pair(AceAutoFillType::ACE_PERSON_LAST_NAME, "TextContentType.PERSON_LAST_NAME")},
    {TextContentType::PERSON_FIRST_NAME,
        std::make_pair(AceAutoFillType::ACE_PERSON_FIRST_NAME, "TextContentType.PERSON_FIRST_NAME")},
    {TextContentType::PHONE_NUMBER, std::make_pair(AceAutoFillType::ACE_PHONE_NUMBER, "TextContentType.PHONE_NUMBER")},
    {TextContentType::PHONE_COUNTRY_CODE,
        std::make_pair(AceAutoFillType::ACE_PHONE_COUNTRY_CODE, "TextContentType.PHONE_COUNTRY_CODE")},
    {TextContentType::FULL_PHONE_NUMBER,
        std::make_pair(AceAutoFillType::ACE_FULL_PHONE_NUMBER, "TextContentType.FULL_PHONE_NUMBER")},
    {TextContentType::EMAIL_ADDRESS,
        std::make_pair(AceAutoFillType::ACE_EMAIL_ADDRESS, "TextContentType.EMAIL_ADDRESS")},
    {TextContentType::BANK_CARD_NUMBER,
        std::make_pair(AceAutoFillType::ACE_BANK_CARD_NUMBER, "TextContentType.BANK_CARD_NUMBER")},
    {TextContentType::ID_CARD_NUMBER,
        std::make_pair(AceAutoFillType::ACE_ID_CARD_NUMBER, "TextContentType.ID_CARD_NUMBER")},
    {TextContentType::PRECISE_TIME, std::make_pair(AceAutoFillType::ACE_PRECISE_TIME, "TextContentType.PRECISE_TIME")},
    {TextContentType::HOUR_AND_MINUTE,
        std::make_pair(AceAutoFillType::ACE_HOUR_AND_MINUTE, "TextContentType.HOUR_AND_MINUTE")},
    {TextContentType::DATE, std::make_pair(AceAutoFillType::ACE_DATE, "TextContentType.DATE")},
    {TextContentType::MONTH, std::make_pair(AceAutoFillType::ACE_MONTH, "TextContentType.MONTH")},
    {TextContentType::YEAR, std::make_pair(AceAutoFillType::ACE_YEAR, "TextContentType.YEAR")},
    {TextContentType::NICKNAME, std::make_pair(AceAutoFillType::ACE_NICKNAME, "TextContentType.NICKNAME")},
    {TextContentType::DETAIL_INFO_WITHOUT_STREET,
        std::make_pair(AceAutoFillType::ACE_DETAIL_INFO_WITHOUT_STREET, "TextContentType.DETAIL_INFO_WITHOUT_STREET")},
    {TextContentType::FORMAT_ADDRESS,
        std::make_pair(AceAutoFillType::ACE_FORMAT_ADDRESS, "TextContentType.FORMAT_ADDRESS")},
    {TextContentType::PASSPORT_NUMBER,
        std::make_pair(AceAutoFillType::ACE_PASSPORT_NUMBER, "TextContentType.PASSPORT_NUMBER")},
    {TextContentType::VALIDITY,
        std::make_pair(AceAutoFillType::ACE_VALIDITY, "TextContentType.VALIDITY")},
    {TextContentType::ISSUE_AT,
        std::make_pair(AceAutoFillType::ACE_ISSUE_AT, "TextContentType.ISSUE_AT")},
    {TextContentType::ORGANIZATION,
        std::make_pair(AceAutoFillType::ACE_ORGANIZATION, "TextContentType.ORGANIZATION")},
    {TextContentType::TAX_ID,
        std::make_pair(AceAutoFillType::ACE_TAX_ID, "TextContentType.TAX_ID")},
    {TextContentType::ADDRESS_CITY_AND_STATE,
        std::make_pair(AceAutoFillType::ACE_ADDRESS_CITY_AND_STATE, "TextContentType.ADDRESS_CITY_AND_STATE")},
    {TextContentType::FLIGHT_NUMBER,
        std::make_pair(AceAutoFillType::ACE_FLIGHT_NUMBER, "TextContentType.FLIGHT_NUMBER")},
    {TextContentType::LICENSE_NUMBER,
        std::make_pair(AceAutoFillType::ACE_LICENSE_NUMBER, "TextContentType.LICENSE_NUMBER")},
    {TextContentType::LICENSE_FILE_NUMBER,
        std::make_pair(AceAutoFillType::ACE_LICENSE_FILE_NUMBER, "TextContentType.LICENSE_FILE_NUMBER")},
    {TextContentType::LICENSE_PLATE,
        std::make_pair(AceAutoFillType::ACE_LICENSE_PLATE, "TextContentType.LICENSE_PLATE")},
    {TextContentType::ENGINE_NUMBER,
        std::make_pair(AceAutoFillType::ACE_ENGINE_NUMBER, "TextContentType.ENGINE_NUMBER")},
    {TextContentType::LICENSE_CHASSIS_NUMBER,
        std::make_pair(AceAutoFillType::ACE_LICENSE_CHASSIS_NUMBER, "TextContentType.LICENSE_CHASSIS_NUMBER")},
    {TextContentType::UNSPECIFIED, std::make_pair(AceAutoFillType::ACE_UNSPECIFIED, "TextContentType.UNSPECIFIED")}};

AceAutoFillType TextContentTypeToAceAutoFillType(const TextContentType& type)
{
    if (contentTypeMap_.find(type) != contentTypeMap_.end()) {
        return contentTypeMap_[type].first;
    }
    return contentTypeMap_[TextContentType::UNSPECIFIED].first;
}

AceAutoFillType ConvertToAceAutoFillType(TextInputType type)
{
    static std::unordered_map<TextInputType, AceAutoFillType> convertMap = {
        { TextInputType::VISIBLE_PASSWORD, AceAutoFillType::ACE_PASSWORD },
        { TextInputType::USER_NAME, AceAutoFillType::ACE_USER_NAME },
        { TextInputType::NEW_PASSWORD, AceAutoFillType::ACE_NEW_PASSWORD },
        { TextInputType::NUMBER_PASSWORD, AceAutoFillType::ACE_PASSWORD } };
    auto it = convertMap.find(type);
    if (it != convertMap.end()) {
        return it->second;
    }
    return AceAutoFillType::ACE_UNSPECIFIED;
}
} // namespace OHOS::Ace::NG