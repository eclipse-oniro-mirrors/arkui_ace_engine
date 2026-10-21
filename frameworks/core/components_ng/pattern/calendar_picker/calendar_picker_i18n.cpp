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

#include "core/components_ng/pattern/calendar_picker/calendar_picker_i18n.h"

#include <algorithm>

#include "unicode/calendar.h"
#include "unicode/dtptngen.h"
#include "unicode/numberformatter.h"

namespace OHOS::Ace::NG {

namespace {
icu::Locale CreateIcuLocale(const std::string& locale)
{
    if (locale.empty()) {
        return icu::Locale::getDefault();
    }
    std::string name = locale;
    std::replace(name.begin(), name.end(), '-', '_');
    return icu::Locale(name.c_str());
}
} // namespace

int32_t CalendarPickerI18n::GetFirstDayOfWeek(const std::string& locale)
{
    UErrorCode status = U_ZERO_ERROR;
    auto cal = icu::Calendar::createInstance(CreateIcuLocale(locale), status);
    if (status > U_ZERO_ERROR || cal == nullptr) {
        delete cal;
        return 0;
    }
    int32_t firstDay = cal->getFirstDayOfWeek();
    delete cal;
    return firstDay - 1;
}

std::string CalendarPickerI18n::LocalizeDateNumber(int32_t value, int32_t minDigits, const std::string& locale)
{
    auto fallback = [value, minDigits]() {
        std::string str = std::to_string(value);
        if (static_cast<int32_t>(str.length()) < minDigits) {
            str = std::string(minDigits - str.length(), '0') + str;
        }
        return str;
    };
    UErrorCode status = U_ZERO_ERROR;
    icu::number::LocalizedNumberFormatter formatter =
        icu::number::NumberFormatter::withLocale(CreateIcuLocale(locale));
    formatter = formatter.grouping(UNumberGroupingStrategy::UNUM_GROUPING_OFF);
    formatter = formatter.integerWidth(icu::number::IntegerWidth::zeroFillTo(minDigits));
    auto result = formatter.formatInt(value, status);
    if (U_FAILURE(status)) {
        return fallback();
    }
    icu::UnicodeString us = result.toString(status);
    if (U_FAILURE(status)) {
        return fallback();
    }
    std::string s;
    us.toUTF8String(s);
    return s;
}

std::vector<std::string> CalendarPickerI18n::GetDateSeparators(const std::string& locale)
{
    UErrorCode status = U_ZERO_ERROR;
    auto* gen = icu::DateTimePatternGenerator::createInstance(CreateIcuLocale(locale), status);
    if (U_FAILURE(status) || gen == nullptr) {
        return {"/", "/"};
    }
    icu::UnicodeString pattern = gen->getBestPattern(icu::UnicodeString("yMMdd"), status);
    delete gen;
    if (U_FAILURE(status)) {
        return {"/", "/"};
    }
    std::string patternStr;
    pattern.toUTF8String(patternStr);

    std::vector<std::string> separators;
    std::string currentSep;
    bool inField = false;
    bool seenField = false;
    bool inQuote = false;
    for (size_t i = 0; i < patternStr.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(patternStr[i]);
        if (ch == '\'') {
            inQuote = !inQuote;
            continue;
        }
        bool isField = !inQuote &&
            (ch == 'y' || ch == 'Y' || ch == 'M' || ch == 'L' || ch == 'd' || ch == 'D');
        if (isField) {
            if (seenField && !inField && !currentSep.empty()) {
                separators.push_back(currentSep);
                currentSep.clear();
            }
            inField = true;
            seenField = true;
        } else {
            if (inField) {
                currentSep.clear();
            }
            currentSep += static_cast<char>(ch);
            inField = false;
        }
    }
    static constexpr size_t MIN_SEPARATOR_COUNT = 2;
    if (separators.size() < MIN_SEPARATOR_COUNT) {
        return {"/", "/"};
    }
    return {separators[0], separators[1]};
}

} // namespace OHOS::Ace::NG
