/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef GENERATED_FOUNDATION_ACE_CAPI_TEST_FIXTURES_H
#define GENERATED_FOUNDATION_ACE_CAPI_TEST_FIXTURES_H

#include <string>
#include <tuple>
#include <vector>

#include "arkoala_api.h"
#include "test_fixtures_enums.h"

#include "core/interfaces/arkoala/utility/reverse_converter.h"

namespace OHOS::Ace::NG::Fixtures {
enum ResID {
    COLORS_RES_0_ID,
};

inline constexpr auto COLORS_RES_0_STR = "COLORS_RES_0_STR";

extern std::vector<std::tuple<ResID, std::string, ResRawValue>> resourceInitTable;

// Fixture 'Boolean' for type 'Ark_Boolean'
extern std::vector<std::tuple<std::string, Ark_Boolean, std::string>> testFixtureBooleanValidValues;

// Fixture 'String' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureStringValidValues;

// Fixture 'ColorsStr' for type 'Ark_String'
extern std::vector<std::tuple<std::string, Ark_String, std::string>> testFixtureColorsStrValidValues;
extern std::vector<std::tuple<std::string, Ark_String>> testFixtureColorsStrInvalidValues;

// Fixture 'ColorsNum' for type 'Ark_Number'
extern std::vector<std::tuple<std::string, Ark_Number, std::string>> testFixtureColorsNumValidValues;

// Fixture 'ColorsRes' for type 'Ark_Resource'
extern std::vector<std::tuple<std::string, Ark_Resource, std::string>> testFixtureColorsResValidValues;

// Fixture 'ColorsEnum' for type 'Ark_Color'
extern std::vector<std::tuple<std::string, Ark_Color, std::string>> testFixtureColorsEnumValidValues;
extern std::vector<std::tuple<std::string, Ark_Color>> testFixtureColorsEnumInvalidValues;
} // namespace OHOS::Ace::NG::Fixtures
#endif // GENERATED_FOUNDATION_ACE_CAPI_TEST_FIXTURES_H