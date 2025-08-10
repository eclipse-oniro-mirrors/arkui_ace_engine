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
import { BrandColors, ArkThemeResource } from 'arkui/handwritten/theme';
import { Resource } from 'global.resource';
import { ArkUIAniModule } from 'arkui.ani';
import { Colors, CustomColors } from '@ohos/arkui/theme';
import { _r } from 'arkui/component/resources';

export class ArkColorsImpl implements Colors {
    brand: ResourceColor = _r(125830976, 10001, '', '');
    warning: ResourceColor = _r(125830979, 10001, '', '');
    alert: ResourceColor = _r(125830980, 10001, '', '');
    confirm: ResourceColor = _r(125830981, 10001, '', '');

    fontPrimary: ResourceColor = _r(125830982, 10001, '', '');
    fontSecondary: ResourceColor = _r(125830983, 10001, '', '');
    fontTertiary: ResourceColor = _r(125830984, 10001, '', '');
    fontFourth: ResourceColor = _r(125830985, 10001, '', '');
    fontEmphasize: ResourceColor = _r(125830986, 10001, '', '');

    fontOnPrimary: ResourceColor = _r(125830987, 10001, '', '');
    fontOnSecondary: ResourceColor = _r(125830988, 10001, '', '');
    fontOnTertiary: ResourceColor = _r(125830989, 10001, '', '');
    fontOnFourth: ResourceColor = _r(125830990, 10001, '', '');

    iconPrimary: ResourceColor = _r(125830991, 10001, '', '');
    iconSecondary: ResourceColor = _r(125830992, 10001, '', '');
    iconTertiary: ResourceColor = _r(125830993, 10001, '', '');
    iconFourth: ResourceColor = _r(125830994, 10001, '', '');
    iconEmphasize: ResourceColor = _r(125830995, 10001, '', '');
    iconSubEmphasize: ResourceColor = _r(125830996, 10001, '', '');

    iconOnPrimary: ResourceColor = _r(125831057, 10001, '', '');
    iconOnSecondary: ResourceColor = _r(125831058, 10001, '', '');
    iconOnTertiary: ResourceColor = _r(125831059, 10001, '', '');
    iconOnFourth: ResourceColor = _r(125831060, 10001, '', '');

    backgroundPrimary: ResourceColor = _r(125831061, 10001, '', '');
    backgroundSecondary: ResourceColor = _r(125831062, 10001, '', '');
    backgroundTertiary: ResourceColor = _r(125831063, 10001, '', '');
    backgroundFourth: ResourceColor = _r(125831064, 10001, '', '');
    backgroundEmphasize: ResourceColor = _r(125831065, 10001, '', '');

    compForegroundPrimary: ResourceColor = _r(125831003, 10001, '', '');
    compBackgroundPrimary: ResourceColor = _r(125831004, 10001, '', '');
    compBackgroundPrimaryTran: ResourceColor = _r('', '', 'sys.color.comp_background_primary_tran'); // TODO: 添加实际资源ID
    compBackgroundPrimaryContrary: ResourceColor = _r(125831005, 10001, '', '');
    compBackgroundGray: ResourceColor = _r(125831006, 10001, '', '');
    compBackgroundSecondary: ResourceColor = _r(125831007, 10001, '', '');
    compBackgroundTertiary: ResourceColor = _r(125831008, 10001, '', '');
    compBackgroundEmphasize: ResourceColor = _r(125831009, 10001, '', '');
    compBackgroundNeutral: ResourceColor = _r(125831066, 10001, '', '');
    compEmphasizeSecondary: ResourceColor = _r(125831011, 10001, '', '');
    compEmphasizeTertiary: ResourceColor = _r(125831012, 10001, '', '');
    compDivider: ResourceColor = _r(125831013, 10001, '', '');
    compCommonContrary: ResourceColor = _r(125831014, 10001, '', '');
    compBackgroundFocus: ResourceColor = _r(125831015, 10001, '', '');
    compFocusedPrimary: ResourceColor = _r(125831016, 10001, '', '');
    compFocusedSecondary: ResourceColor = _r(125831017, 10001, '', '');
    compFocusedTertiary: ResourceColor = _r(125831018, 10001, '', '');

    interactiveHover: ResourceColor = _r(125831019, 10001, '', '');
    interactivePressed: ResourceColor = _r(125831020, 10001, '', '');
    interactiveFocus: ResourceColor = _r(125831021, 10001, '', '');
    interactiveActive: ResourceColor = _r(125831022, 10001, '', '');
    interactiveSelect: ResourceColor = _r(125831023, 10001, '', '');
    interactiveClick: ResourceColor = _r(125831024, 10001, '', '');

    constructor() {}

    constructor(colors: CustomColors | undefined, baselineColors: Colors) {
        ArkColorsImpl.assign(this, baselineColors);
        if (colors) {
            ArkColorsImpl.assign(this, colors!);
        }
    }

    static assign(dst: Colors, src: Colors): void {
        dst.brand = src.brand;
        dst.warning = src.warning;
        dst.alert = src.alert;
        dst.confirm = src.confirm;

        dst.fontPrimary = src.fontPrimary;
        dst.fontSecondary = src.fontSecondary;
        dst.fontTertiary = src.fontTertiary;
        dst.fontFourth = src.fontFourth;
        dst.fontEmphasize = src.fontEmphasize;

        dst.fontOnPrimary = src.fontOnPrimary;
        dst.fontOnSecondary = src.fontOnSecondary;
        dst.fontOnTertiary = src.fontOnTertiary;
        dst.fontOnFourth = src.fontOnFourth;

        dst.iconPrimary = src.iconPrimary;
        dst.iconSecondary = src.iconSecondary;
        dst.iconTertiary = src.iconTertiary;
        dst.iconFourth = src.iconFourth;
        dst.iconEmphasize = src.iconEmphasize;
        dst.iconSubEmphasize = src.iconSubEmphasize;

        dst.iconOnPrimary = src.iconOnPrimary;
        dst.iconOnSecondary = src.iconOnSecondary;
        dst.iconOnTertiary = src.iconOnTertiary;
        dst.iconOnFourth = src.iconOnFourth;

        dst.backgroundPrimary = src.backgroundPrimary;
        dst.backgroundSecondary = src.backgroundSecondary;
        dst.backgroundTertiary = src.backgroundTertiary;
        dst.backgroundFourth = src.backgroundFourth;
        dst.backgroundEmphasize = src.backgroundEmphasize;

        dst.compForegroundPrimary = src.compForegroundPrimary;
        dst.compBackgroundPrimary = src.compBackgroundPrimary;
        dst.compBackgroundPrimaryTran = src.compBackgroundPrimaryTran;
        dst.compBackgroundPrimaryContrary = src.compBackgroundPrimaryContrary;
        dst.compBackgroundGray = src.compBackgroundGray;
        dst.compBackgroundSecondary = src.compBackgroundSecondary;
        dst.compBackgroundTertiary = src.compBackgroundTertiary;
        dst.compBackgroundEmphasize = src.compBackgroundEmphasize;
        dst.compBackgroundNeutral = src.compBackgroundNeutral;
        dst.compEmphasizeSecondary = src.compEmphasizeSecondary;
        dst.compEmphasizeTertiary = src.compEmphasizeTertiary;
        dst.compDivider = src.compDivider;
        dst.compCommonContrary = src.compCommonContrary;
        dst.compBackgroundFocus = src.compBackgroundFocus;
        dst.compFocusedPrimary = src.compFocusedPrimary;
        dst.compFocusedSecondary = src.compFocusedSecondary;
        dst.compFocusedTertiary = src.compFocusedTertiary;

        dst.interactiveHover = src.interactiveHover;
        dst.interactivePressed = src.interactivePressed;
        dst.interactiveFocus = src.interactiveFocus;
        dst.interactiveActive = src.interactiveActive;
        dst.interactiveSelect = src.interactiveSelect;
        dst.interactiveClick = src.interactiveClick;
    }

    static assign(dst: Colors, src: CustomColors): void {
        if (src.brand !== undefined) dst.brand = src.brand!;
        if (src.warning !== undefined) dst.warning = src.warning!;
        if (src.alert !== undefined) dst.alert = src.alert!;
        if (src.confirm !== undefined) dst.confirm = src.confirm!;

        if (src.fontPrimary !== undefined) dst.fontPrimary = src.fontPrimary!;
        if (src.fontSecondary !== undefined) dst.fontSecondary = src.fontSecondary!;
        if (src.fontTertiary !== undefined) dst.fontTertiary = src.fontTertiary!;
        if (src.fontFourth !== undefined) dst.fontFourth = src.fontFourth!;
        if (src.fontEmphasize !== undefined) dst.fontEmphasize = src.fontEmphasize!;

        if (src.fontOnPrimary !== undefined) dst.fontOnPrimary = src.fontOnPrimary!;
        if (src.fontOnSecondary !== undefined) dst.fontOnSecondary = src.fontOnSecondary!;
        if (src.fontOnTertiary !== undefined) dst.fontOnTertiary = src.fontOnTertiary!;
        if (src.fontOnFourth !== undefined) dst.fontOnFourth = src.fontOnFourth!;

        if (src.iconPrimary !== undefined) dst.iconPrimary = src.iconPrimary!;
        if (src.iconSecondary !== undefined) dst.iconSecondary = src.iconSecondary!;
        if (src.iconTertiary !== undefined) dst.iconTertiary = src.iconTertiary!;
        if (src.iconFourth !== undefined) dst.iconFourth = src.iconFourth!;
        if (src.iconEmphasize !== undefined) dst.iconEmphasize = src.iconEmphasize!;
        if (src.iconSubEmphasize !== undefined) dst.iconSubEmphasize = src.iconSubEmphasize!;

        if (src.iconOnPrimary !== undefined) dst.iconOnPrimary = src.iconOnPrimary!;
        if (src.iconOnSecondary !== undefined) dst.iconOnSecondary = src.iconOnSecondary!;
        if (src.iconOnTertiary !== undefined) dst.iconOnTertiary = src.iconOnTertiary!;
        if (src.iconOnFourth !== undefined) dst.iconOnFourth = src.iconOnFourth!;

        if (src.backgroundPrimary !== undefined) dst.backgroundPrimary = src.backgroundPrimary!;
        if (src.backgroundSecondary !== undefined) dst.backgroundSecondary = src.backgroundSecondary!;
        if (src.backgroundTertiary !== undefined) dst.backgroundTertiary = src.backgroundTertiary!;
        if (src.backgroundFourth !== undefined) dst.backgroundFourth = src.backgroundFourth!;
        if (src.backgroundEmphasize !== undefined) dst.backgroundEmphasize = src.backgroundEmphasize!;

        if (src.compForegroundPrimary !== undefined) dst.compForegroundPrimary = src.compForegroundPrimary!;
        if (src.compBackgroundPrimary !== undefined) dst.compBackgroundPrimary = src.compBackgroundPrimary!;
        if (src.compBackgroundPrimaryTran !== undefined) dst.compBackgroundPrimaryTran = src.compBackgroundPrimaryTran!;
        if (src.compBackgroundPrimaryContrary !== undefined)
            dst.compBackgroundPrimaryContrary = src.compBackgroundPrimaryContrary!;
        if (src.compBackgroundGray !== undefined) dst.compBackgroundGray = src.compBackgroundGray!;
        if (src.compBackgroundSecondary !== undefined) dst.compBackgroundSecondary = src.compBackgroundSecondary!;
        if (src.compBackgroundTertiary !== undefined) dst.compBackgroundTertiary = src.compBackgroundTertiary!;
        if (src.compBackgroundEmphasize !== undefined) dst.compBackgroundEmphasize = src.compBackgroundEmphasize!;
        if (src.compBackgroundNeutral !== undefined) dst.compBackgroundNeutral = src.compBackgroundNeutral!;
        if (src.compEmphasizeSecondary !== undefined) dst.compEmphasizeSecondary = src.compEmphasizeSecondary!;
        if (src.compEmphasizeTertiary !== undefined) dst.compEmphasizeTertiary = src.compEmphasizeTertiary!;
        if (src.compDivider !== undefined) dst.compDivider = src.compDivider!;
        if (src.compCommonContrary !== undefined) dst.compCommonContrary = src.compCommonContrary!;
        if (src.compBackgroundFocus !== undefined) dst.compBackgroundFocus = src.compBackgroundFocus!;
        if (src.compFocusedPrimary !== undefined) dst.compFocusedPrimary = src.compFocusedPrimary!;
        if (src.compFocusedSecondary !== undefined) dst.compFocusedSecondary = src.compFocusedSecondary!;
        if (src.compFocusedTertiary !== undefined) dst.compFocusedTertiary = src.compFocusedTertiary!;

        if (src.interactiveHover !== undefined) dst.interactiveHover = src.interactiveHover!;
        if (src.interactivePressed !== undefined) dst.interactivePressed = src.interactivePressed!;
        if (src.interactiveFocus !== undefined) dst.interactiveFocus = src.interactiveFocus!;
        if (src.interactiveActive !== undefined) dst.interactiveActive = src.interactiveActive!;
        if (src.interactiveSelect !== undefined) dst.interactiveSelect = src.interactiveSelect!;
        if (src.interactiveClick !== undefined) dst.interactiveClick = src.interactiveClick!;
    }

    static expandByBrandColor(colors: CustomColors) {
        console.warn(`FZY expandByBrandColor ${Object.keys(colors)}`);
        console.warn(`FZY expandByBrandColor ${Object.values(colors)}`);
        if (colors!.brand) {
            const brandColors: BrandColors = ArkColorsImpl.makeBrandColors(colors.brand);
            colors.fontEmphasize = colors.fontEmphasize ?? brandColors.primary!;

            colors.iconEmphasize = colors.iconEmphasize ?? brandColors.primary!;
            colors.iconSubEmphasize = colors.iconSubEmphasize ?? brandColors.tertiary!;

            colors.backgroundEmphasize = colors.backgroundEmphasize ?? brandColors.primary!;

            colors.compBackgroundEmphasize = colors.compBackgroundEmphasize ?? brandColors.primary!;

            colors.compEmphasizeSecondary = colors.compEmphasizeSecondary ?? brandColors.fourth!;
            colors.compEmphasizeTertiary = colors.compEmphasizeTertiary ?? brandColors.fifth!;

            colors.interactiveFocus = colors.interactiveFocus ?? brandColors.primary!;
            colors.interactiveActive = colors.interactiveActive ?? brandColors.primary!;
        }
    }

    static makeBrandColors(brandColor: ResourceColor | undefined): BrandColors {
        const result: BrandColors = {
            primary: undefined,
            secondary: undefined,
            tertiary: undefined,
            fourth: undefined,
            fifth: undefined,
            sixth: undefined,
        };
        if (brandColor) {
            if (brandColor instanceof Color) {
                result.primary = brandColor as string;
                result.secondary = brandColor as string;
                result.tertiary = brandColor as string;
                result.fourth = brandColor as string;
                result.fifth = brandColor as string;
                result.sixth = brandColor as string;
            } else if (brandColor instanceof Resource) {
                result.primary = brandColor as Resource;
                result.secondary = ArkColorsImpl.makeResourceWithOpacity(brandColor as Resource, 0.6) as Resource;
                result.tertiary = ArkColorsImpl.makeResourceWithOpacity(brandColor as Resource, 0.4) as Resource;
                result.fourth = ArkColorsImpl.makeResourceWithOpacity(brandColor as Resource, 0.2) as Resource;
                result.fifth = ArkColorsImpl.makeResourceWithOpacity(brandColor as Resource, 0.1) as Resource;
                result.sixth = ArkColorsImpl.makeResourceWithOpacity(brandColor as Resource, 0.05) as Resource;
            } else {
                const brandColorValue = brandColor as number | string;
                const argbColor = ArkUIAniModule._GetColorValue(brandColorValue);
                result.primary = argbColor;
                result.secondary = ArkColorsImpl.blendOpacity(argbColor, 0.6);
                result.tertiary = ArkColorsImpl.blendOpacity(argbColor, 0.4);
                result.fourth = ArkColorsImpl.blendOpacity(argbColor, 0.2);
                result.fifth = ArkColorsImpl.blendOpacity(argbColor, 0.1);
                result.sixth = ArkColorsImpl.blendOpacity(argbColor, 0.05);
            }
        }
        return result;
    }

    static makeResourceWithOpacity(resourceColor: Resource, opacityRatio: number): ArkThemeResource {
        return {
            id: resourceColor.id,
            type: resourceColor.type,
            params: [...resourceColor.params!],
            bundleName: resourceColor.bundleName,
            moduleName: resourceColor.moduleName,
            opacityRatio: opacityRatio,
        } as ArkThemeResource;
    }

    static blendOpacity(argbColor: number, opacityRatio: number): number {
        if (opacityRatio < 0 || opacityRatio > 1.0) {
            return argbColor;
        }
        const alpha = (argbColor >> 24) & 0xff;
        const outAlpha = (alpha * opacityRatio) & 0xff;
        return ((argbColor & 0x00ffffff) | ((outAlpha & 0xff) << 24)) >>> 0;
    }

    static checkIsColor(value: ResourceColor): boolean {
        return value instanceof Color;
    }
}