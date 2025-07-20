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

import { ResourceColor, Color } from 'arkui/component';
import { ArkThemeScopeManager } from 'arkui/handwritten/theme/ArkThemeScopeManager';
import { ArkResourcesHelper } from 'arkui/handwritten/theme/ArkResourcesHelper';
import { ArkSystemShapes } from 'arkui/handwritten/theme/system/ArkSystemShapes';
import { ArkSystemTypography } from 'arkui/handwritten/theme/system/ArkSystemTypography';
import { Shapes, Typography, CustomTypography, CustomShapes, BrandColors, ArkThemeResource } from 'arkui/handwritten/theme';
import { Resource } from 'global.resource';
import { ArkUIAniModule } from 'arkui.ani';

export interface Colors {
    brand: ResourceColor;
    warning: ResourceColor;
    alert: ResourceColor;
    confirm: ResourceColor;
    fontPrimary: ResourceColor;
    fontSecondary: ResourceColor;
    fontTertiary: ResourceColor;
    fontFourth: ResourceColor;
    fontEmphasize: ResourceColor;
    fontOnPrimary: ResourceColor;
    fontOnSecondary: ResourceColor;
    fontOnTertiary: ResourceColor;
    fontOnFourth: ResourceColor;
    iconPrimary: ResourceColor;
    iconSecondary: ResourceColor;
    iconTertiary: ResourceColor;
    iconFourth: ResourceColor;
    iconEmphasize: ResourceColor;
    iconSubEmphasize: ResourceColor;
    iconOnPrimary: ResourceColor;
    iconOnSecondary: ResourceColor;
    iconOnTertiary: ResourceColor;
    iconOnFourth: ResourceColor;
    backgroundPrimary: ResourceColor;
    backgroundSecondary: ResourceColor;
    backgroundTertiary: ResourceColor;
    backgroundFourth: ResourceColor;
    backgroundEmphasize: ResourceColor;
    compForegroundPrimary: ResourceColor;
    compBackgroundPrimary: ResourceColor;
    compBackgroundPrimaryTran: ResourceColor;
    compBackgroundPrimaryContrary: ResourceColor;
    compBackgroundGray: ResourceColor;
    compBackgroundSecondary: ResourceColor;
    compBackgroundTertiary: ResourceColor;
    compBackgroundEmphasize: ResourceColor;
    compBackgroundNeutral: ResourceColor;
    compEmphasizeSecondary: ResourceColor;
    compEmphasizeTertiary: ResourceColor;
    compDivider: ResourceColor;
    compCommonContrary: ResourceColor;
    compBackgroundFocus: ResourceColor;
    compFocusedPrimary: ResourceColor;
    compFocusedSecondary: ResourceColor;
    compFocusedTertiary: ResourceColor;
    interactiveHover: ResourceColor;
    interactivePressed: ResourceColor;
    interactiveFocus: ResourceColor;
    interactiveActive: ResourceColor;
    interactiveSelect: ResourceColor;
    interactiveClick: ResourceColor;
}

export type CustomColors = Partial<Colors>;

export interface Theme {
    colors: Colors;
}

export interface CustomTheme {
    colors?: CustomColors;
}

export class ThemeControl {
    static setDefaultTheme(customTheme: CustomTheme) {
        ArkThemeScopeManager.getInstance().setDefaultTheme(customTheme);
    }
}
