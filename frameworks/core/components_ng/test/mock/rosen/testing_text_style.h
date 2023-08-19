/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MOCK_ROSEN_TEST_TESTING_TEXT_STYLE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MOCK_ROSEN_TEST_TESTING_TEXT_STYLE_H

#include <string>
#include <vector>

#include "core/components_ng/test/mock/rosen/testing_color.h"
#include "core/components_ng/test/mock/rosen/testing_pen.h"
#include "core/components_ng/test/mock/rosen/testing_point.h"

namespace OHOS::Ace::Testing {
enum class TestingFontWeight {
    W100, // thin
    W200,
    W300,
    W400,
    W500,
    W600,
    W700,
    W800,
    W900,
};

enum class TestingTextDecoration {
    NONE = 0x0,
    UNDERLINE = 0x1,
    OVERLINE = 0x2,
    LINETHROUGH = 0x4,
};

enum class TestingTextDecorationStyle {
    SOLID,
    DOUBLE,
    DOTTED,
    DASHED,
    WAVY,
};

enum class TestingFontStyle {
    NORMAL,
    ITALIC,
};

enum class TestingTextBaseline {
    ALPHABETIC,
    IDEOGRAPHIC,
};

class TestingTextShadow {
public:
    TestingTextShadow() = default;
    virtual ~TestingTextShadow() = default;

    virtual bool hasShadow() const
    {
        return false;
    }

    TestingColor color_;
    TestingPoint offset_;
    double blurRadius_ = 0.0;
};

class TestingFontFeatures {
public:
    TestingFontFeatures() = default;
    virtual ~TestingFontFeatures() = default;
};

class TestingTextStyle {
public:
    TestingTextStyle() = default;
    ~TestingTextStyle() = default;

    virtual bool equals(const TestingTextStyle& /* rhs */) const
    {
        return false;
    }

    double decorationThicknessMultiplier_ = 1.0;
    double fontSize_ = 14.0;
    double letterSpacing_ = 0.0;
    double wordSpacing_ = 0.0;
    double height_ = 1.0;
    bool hasHeightOverride_ = false;
    bool hasBackground_ = false;
    bool hasForeground_ = false;
    std::u16string ellipsis_;
    std::string locale_;
    std::vector<std::string> fontFamilies_;
    TestingColor color_;
    TestingTextDecoration decoration_ = TestingTextDecoration::NONE;
    TestingColor decorationColor_;
    TestingPen background_;
    TestingPen foreground_;
    TestingTextDecorationStyle decorationStyle_ = TestingTextDecorationStyle::SOLID;
    TestingFontWeight fontWeight_ = TestingFontWeight::W400;
    TestingFontStyle fontStyle_ = TestingFontStyle::NORMAL;
    TestingTextBaseline textBaseline_ = TestingTextBaseline::ALPHABETIC;
    std::vector<TestingTextShadow> textShadows_;
    TestingFontFeatures fontFeatures_;
};
} // namespace OHOS::Ace::Testing
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MOCK_ROSEN_TEST_TESTING_TEXT_STYLE_H
