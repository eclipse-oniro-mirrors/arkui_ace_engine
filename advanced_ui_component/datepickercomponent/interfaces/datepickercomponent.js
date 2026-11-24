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
if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
const LengthMetrics = requireNapi('arkui.node').LengthMetrics;
const LengthUnit = requireNapi('arkui.node').LengthUnit;
const intl = requireNapi('intl');
const i18n = requireNapi('i18n');
const commonEventManager = requireNapi('commonEventManager');
export var DateMode;
(function (DateMode) {
    DateMode[DateMode["DATE"] = 0] = "DATE";
    DateMode[DateMode["YEAR_AND_MONTH"] = 1] = "YEAR_AND_MONTH";
    DateMode[DateMode["MONTH_AND_DAY"] = 2] = "MONTH_AND_DAY";
})(DateMode || (DateMode = {}));
export var DisplayMode;
(function (DisplayMode) {
    DisplayMode[DisplayMode["DATE"] = 0] = "DATE";
    DisplayMode[DisplayMode["TIME"] = 1] = "TIME";
    DisplayMode[DisplayMode["DATE_TIME"] = 2] = "DATE_TIME";
})(DisplayMode || (DisplayMode = {}));
export var TimeFormat;
(function (TimeFormat) {
    TimeFormat[TimeFormat["HOUR_MINUTE"] = 0] = "HOUR_MINUTE";
    TimeFormat[TimeFormat["HOUR_MINUTE_SECOND"] = 1] = "HOUR_MINUTE_SECOND";
})(TimeFormat || (TimeFormat = {}));
export class DatePickerComponentResult {
}
export class CommonOptions {
}
export class DateOptions extends CommonOptions {
}
export class TimeOptions extends CommonOptions {
}
export class DatePickerComponentOptions {
}
class DatePickerConstant {
}
DatePickerConstant.MIN_YEAR = 1;
DatePickerConstant.MAX_YEAR = 9999;
DatePickerConstant.DEFAULT_START_YEAR = 1970;
DatePickerConstant.DEFAULT_END_YEAR = 2100;
DatePickerConstant.MIN_MONTH = 0;
DatePickerConstant.MAX_MONTH = 11;
DatePickerConstant.MIN_DAY = 1;
DatePickerConstant.MIN_HOUR = 0;
DatePickerConstant.MAX_HOUR = 23;
DatePickerConstant.MIN_MINUTE = 0;
DatePickerConstant.MAX_MINUTE = 59;
DatePickerConstant.MIN_SECOND = 0;
DatePickerConstant.MAX_SECOND = 59;
DatePickerConstant.MONTHS_IN_YEAR = 12;
DatePickerConstant.DEFAULT_STEP = 1;
DatePickerConstant.HIDE_YEAR_SENTINEL = -1;
DatePickerConstant.HIDE_YEAR_TEXT = '----';
DatePickerConstant.SYSTEM_MIN_DATE = new Date(1970, 0, 1);
DatePickerConstant.SYSTEM_MAX_DATE = new Date(2100, 11, 31);
export class DatePickerComponent extends ViewPU {
    constructor(parent, params, __localStorage, elmtId = -1, paramsLambda = undefined, extraInfo) {
        super(parent, __localStorage, elmtId, extraInfo);
        if (typeof paramsLambda === "function") {
            this.paramsGenerator_ = paramsLambda;
        }
        this.__options = new SynchedPropertyObjectOneWayPU(params.options, this, "options");
        this.__selectionIndicator = new SynchedPropertyObjectOneWayPU(params.selectionIndicator, this, "selectionIndicator");
        this.__selectedTextStyle = new SynchedPropertyObjectOneWayPU(params.selectedTextStyle, this, "selectedTextStyle");
        this.__itemTextStyle = new SynchedPropertyObjectOneWayPU(params.itemTextStyle, this, "itemTextStyle");
        this.__displayMode = new ObservedPropertySimplePU(DisplayMode.DATE, this, "displayMode");
        this.__selectedYear = new ObservedPropertySimplePU(0, this, "selectedYear");
        this.__selectedMonth = new ObservedPropertySimplePU(0, this, "selectedMonth");
        this.__selectedIsLeapMonth = new ObservedPropertySimplePU(false, this, "selectedIsLeapMonth");
        this.__selectedDay = new ObservedPropertySimplePU(0, this, "selectedDay");
        this.__selectedHour = new ObservedPropertySimplePU(0, this, "selectedHour");
        this.__selectedMinute = new ObservedPropertySimplePU(0, this, "selectedMinute");
        this.__selectedSecond = new ObservedPropertySimplePU(0, this, "selectedSecond");
        this.__startYear = new ObservedPropertySimplePU(DatePickerConstant.DEFAULT_START_YEAR, this, "startYear");
        this.__endYear = new ObservedPropertySimplePU(DatePickerConstant.DEFAULT_END_YEAR, this, "endYear");
        this.__startMonth = new ObservedPropertySimplePU(DatePickerConstant.MIN_MONTH, this, "startMonth");
        this.__endMonth = new ObservedPropertySimplePU(DatePickerConstant.MAX_MONTH, this, "endMonth");
        this.__startDay = new ObservedPropertySimplePU(DatePickerConstant.MIN_DAY, this, "startDay");
        this.__endDay = new ObservedPropertySimplePU(31, this, "endDay");
        this.__lunarStartMonth = new ObservedPropertySimplePU(DatePickerConstant.MIN_MONTH, this, "lunarStartMonth");
        this.__lunarEndMonth = new ObservedPropertySimplePU(DatePickerConstant.MAX_MONTH, this, "lunarEndMonth");
        this.__lunarStartDay = new ObservedPropertySimplePU(DatePickerConstant.MIN_DAY, this, "lunarStartDay");
        this.__lunarEndDay = new ObservedPropertySimplePU(30, this, "lunarEndDay");
        this.__lunarStartYear = new ObservedPropertySimplePU(DatePickerConstant.DEFAULT_START_YEAR, this, "lunarStartYear");
        this.__lunarEndYear = new ObservedPropertySimplePU(DatePickerConstant.DEFAULT_END_YEAR, this, "lunarEndYear");
        this.__startHour = new ObservedPropertySimplePU(DatePickerConstant.MIN_HOUR, this, "startHour");
        this.__endHour = new ObservedPropertySimplePU(DatePickerConstant.MAX_HOUR, this, "endHour");
        this.__startMinute = new ObservedPropertySimplePU(DatePickerConstant.MIN_MINUTE, this, "startMinute");
        this.__endMinute = new ObservedPropertySimplePU(DatePickerConstant.MAX_MINUTE, this, "endMinute");
        this.__startSecond = new ObservedPropertySimplePU(DatePickerConstant.MIN_SECOND, this, "startSecond");
        this.__endSecond = new ObservedPropertySimplePU(DatePickerConstant.MAX_SECOND, this, "endSecond");
        this.__dateMode = new ObservedPropertySimplePU(DateMode.DATE, this, "dateMode");
        this.__timeFormat = new ObservedPropertySimplePU(TimeFormat.HOUR_MINUTE, this, "timeFormat");
        this.__useMilitaryTime = new ObservedPropertySimplePU(false, this, "useMilitaryTime");
        this.__lunar = new ObservedPropertySimplePU(false, this, "lunar");
        this.__hideYear = new ObservedPropertySimplePU(false, this, "hideYear");
        this.__dateCanLoop = new ObservedPropertySimplePU(true, this, "dateCanLoop");
        this.__timeCanLoop = new ObservedPropertySimplePU(true, this, "timeCanLoop");
        this.__dateStartEndSet = new ObservedPropertySimplePU(false, this, "dateStartEndSet");
        this.__timeStartEndSet = new ObservedPropertySimplePU(false, this, "timeStartEndSet");
        this.__yearColumnCanLoop = new ObservedPropertySimplePU(true, this, "yearColumnCanLoop");
        this.__monthColumnCanLoop = new ObservedPropertySimplePU(true, this, "monthColumnCanLoop");
        this.__dayColumnCanLoop = new ObservedPropertySimplePU(true, this, "dayColumnCanLoop");
        this.__hourColumnCanLoop = new ObservedPropertySimplePU(true, this, "hourColumnCanLoop");
        this.__minuteColumnCanLoop = new ObservedPropertySimplePU(true, this, "minuteColumnCanLoop");
        this.__secondColumnCanLoop = new ObservedPropertySimplePU(true, this, "secondColumnCanLoop");
        this.__dateHapticFeedback = new ObservedPropertySimplePU(true, this, "dateHapticFeedback");
        this.__timeHapticFeedback = new ObservedPropertySimplePU(true, this, "timeHapticFeedback");
        this.__yearArray = new ObservedPropertyObjectPU([], this, "yearArray");
        this.__monthArray = new ObservedPropertyObjectPU([], this, "monthArray");
        this.__dayArray = new ObservedPropertyObjectPU([], this, "dayArray");
        this.__hourArray = new ObservedPropertyObjectPU([], this, "hourArray");
        this.__minuteArray = new ObservedPropertyObjectPU([], this, "minuteArray");
        this.__secondArray = new ObservedPropertyObjectPU([], this, "secondArray");
        this.__periodArray = new ObservedPropertyObjectPU([], this, "periodArray");
        this.__selectedPeriod = new ObservedPropertySimplePU(0, this, "selectedPeriod");
        this.dateOnChange = undefined;
        this.dateOnScrollStop = undefined;
        this.timeOnChange = undefined;
        this.timeOnScrollStop = undefined;
        this.__currentLocale = new ObservedPropertySimplePU('', this, "currentLocale");
        this.locale = new intl.Locale();
        this.initFlag = true;
        this.lunarCalendar = null;
        this.subscriber = null;
        this.isComponentActive = false;
        this.formatter = null;
        this.yearFormatter = null;
        this.userLunar = false;
        this.__firstColumnWidth = new ObservedPropertySimplePU(0, this, "firstColumnWidth");
        this.__lastColumnWidth = new ObservedPropertySimplePU(0, this, "lastColumnWidth");
        this.hourFormatter = null;
        this.minuteFormatter = null;
        this.secondFormatter = null;
        this.lunarLeapMonthCacheKey = -2;
        this.lunarLeapMonthCacheValue = -1;
        this.lunarDaysCacheKey = '';
        this.lunarDaysCacheValue = 30;
        this.convertResultCacheKey = '';
        this.convertResultCacheValue = null;
        this.setInitiallyProvidedValue(params);
        this.declareWatch("options", this.onOptionsChange);
        this.declareWatch("hideYear", this.onHideYearChange);
        this.declareWatch("currentLocale", this.onLocaleChange);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(params) {
        if (params.displayMode !== undefined) {
            this.displayMode = params.displayMode;
        }
        if (params.selectedYear !== undefined) {
            this.selectedYear = params.selectedYear;
        }
        if (params.selectedMonth !== undefined) {
            this.selectedMonth = params.selectedMonth;
        }
        if (params.selectedIsLeapMonth !== undefined) {
            this.selectedIsLeapMonth = params.selectedIsLeapMonth;
        }
        if (params.selectedDay !== undefined) {
            this.selectedDay = params.selectedDay;
        }
        if (params.selectedHour !== undefined) {
            this.selectedHour = params.selectedHour;
        }
        if (params.selectedMinute !== undefined) {
            this.selectedMinute = params.selectedMinute;
        }
        if (params.selectedSecond !== undefined) {
            this.selectedSecond = params.selectedSecond;
        }
        if (params.startYear !== undefined) {
            this.startYear = params.startYear;
        }
        if (params.endYear !== undefined) {
            this.endYear = params.endYear;
        }
        if (params.startMonth !== undefined) {
            this.startMonth = params.startMonth;
        }
        if (params.endMonth !== undefined) {
            this.endMonth = params.endMonth;
        }
        if (params.startDay !== undefined) {
            this.startDay = params.startDay;
        }
        if (params.endDay !== undefined) {
            this.endDay = params.endDay;
        }
        if (params.lunarStartMonth !== undefined) {
            this.lunarStartMonth = params.lunarStartMonth;
        }
        if (params.lunarEndMonth !== undefined) {
            this.lunarEndMonth = params.lunarEndMonth;
        }
        if (params.lunarStartDay !== undefined) {
            this.lunarStartDay = params.lunarStartDay;
        }
        if (params.lunarEndDay !== undefined) {
            this.lunarEndDay = params.lunarEndDay;
        }
        if (params.lunarStartYear !== undefined) {
            this.lunarStartYear = params.lunarStartYear;
        }
        if (params.lunarEndYear !== undefined) {
            this.lunarEndYear = params.lunarEndYear;
        }
        if (params.startHour !== undefined) {
            this.startHour = params.startHour;
        }
        if (params.endHour !== undefined) {
            this.endHour = params.endHour;
        }
        if (params.startMinute !== undefined) {
            this.startMinute = params.startMinute;
        }
        if (params.endMinute !== undefined) {
            this.endMinute = params.endMinute;
        }
        if (params.startSecond !== undefined) {
            this.startSecond = params.startSecond;
        }
        if (params.endSecond !== undefined) {
            this.endSecond = params.endSecond;
        }
        if (params.dateMode !== undefined) {
            this.dateMode = params.dateMode;
        }
        if (params.timeFormat !== undefined) {
            this.timeFormat = params.timeFormat;
        }
        if (params.useMilitaryTime !== undefined) {
            this.useMilitaryTime = params.useMilitaryTime;
        }
        if (params.lunar !== undefined) {
            this.lunar = params.lunar;
        }
        if (params.hideYear !== undefined) {
            this.hideYear = params.hideYear;
        }
        if (params.dateCanLoop !== undefined) {
            this.dateCanLoop = params.dateCanLoop;
        }
        if (params.timeCanLoop !== undefined) {
            this.timeCanLoop = params.timeCanLoop;
        }
        if (params.dateStartEndSet !== undefined) {
            this.dateStartEndSet = params.dateStartEndSet;
        }
        if (params.timeStartEndSet !== undefined) {
            this.timeStartEndSet = params.timeStartEndSet;
        }
        if (params.yearColumnCanLoop !== undefined) {
            this.yearColumnCanLoop = params.yearColumnCanLoop;
        }
        if (params.monthColumnCanLoop !== undefined) {
            this.monthColumnCanLoop = params.monthColumnCanLoop;
        }
        if (params.dayColumnCanLoop !== undefined) {
            this.dayColumnCanLoop = params.dayColumnCanLoop;
        }
        if (params.hourColumnCanLoop !== undefined) {
            this.hourColumnCanLoop = params.hourColumnCanLoop;
        }
        if (params.minuteColumnCanLoop !== undefined) {
            this.minuteColumnCanLoop = params.minuteColumnCanLoop;
        }
        if (params.secondColumnCanLoop !== undefined) {
            this.secondColumnCanLoop = params.secondColumnCanLoop;
        }
        if (params.dateHapticFeedback !== undefined) {
            this.dateHapticFeedback = params.dateHapticFeedback;
        }
        if (params.timeHapticFeedback !== undefined) {
            this.timeHapticFeedback = params.timeHapticFeedback;
        }
        if (params.yearArray !== undefined) {
            this.yearArray = params.yearArray;
        }
        if (params.monthArray !== undefined) {
            this.monthArray = params.monthArray;
        }
        if (params.dayArray !== undefined) {
            this.dayArray = params.dayArray;
        }
        if (params.hourArray !== undefined) {
            this.hourArray = params.hourArray;
        }
        if (params.minuteArray !== undefined) {
            this.minuteArray = params.minuteArray;
        }
        if (params.secondArray !== undefined) {
            this.secondArray = params.secondArray;
        }
        if (params.periodArray !== undefined) {
            this.periodArray = params.periodArray;
        }
        if (params.selectedPeriod !== undefined) {
            this.selectedPeriod = params.selectedPeriod;
        }
        if (params.dateOnChange !== undefined) {
            this.dateOnChange = params.dateOnChange;
        }
        if (params.dateOnScrollStop !== undefined) {
            this.dateOnScrollStop = params.dateOnScrollStop;
        }
        if (params.timeOnChange !== undefined) {
            this.timeOnChange = params.timeOnChange;
        }
        if (params.timeOnScrollStop !== undefined) {
            this.timeOnScrollStop = params.timeOnScrollStop;
        }
        if (params.currentLocale !== undefined) {
            this.currentLocale = params.currentLocale;
        }
        if (params.locale !== undefined) {
            this.locale = params.locale;
        }
        if (params.initFlag !== undefined) {
            this.initFlag = params.initFlag;
        }
        if (params.lunarCalendar !== undefined) {
            this.lunarCalendar = params.lunarCalendar;
        }
        if (params.subscriber !== undefined) {
            this.subscriber = params.subscriber;
        }
        if (params.isComponentActive !== undefined) {
            this.isComponentActive = params.isComponentActive;
        }
        if (params.formatter !== undefined) {
            this.formatter = params.formatter;
        }
        if (params.yearFormatter !== undefined) {
            this.yearFormatter = params.yearFormatter;
        }
        if (params.userLunar !== undefined) {
            this.userLunar = params.userLunar;
        }
        if (params.firstColumnWidth !== undefined) {
            this.firstColumnWidth = params.firstColumnWidth;
        }
        if (params.lastColumnWidth !== undefined) {
            this.lastColumnWidth = params.lastColumnWidth;
        }
        if (params.hourFormatter !== undefined) {
            this.hourFormatter = params.hourFormatter;
        }
        if (params.minuteFormatter !== undefined) {
            this.minuteFormatter = params.minuteFormatter;
        }
        if (params.secondFormatter !== undefined) {
            this.secondFormatter = params.secondFormatter;
        }
        if (params.lunarLeapMonthCacheKey !== undefined) {
            this.lunarLeapMonthCacheKey = params.lunarLeapMonthCacheKey;
        }
        if (params.lunarLeapMonthCacheValue !== undefined) {
            this.lunarLeapMonthCacheValue = params.lunarLeapMonthCacheValue;
        }
        if (params.lunarDaysCacheKey !== undefined) {
            this.lunarDaysCacheKey = params.lunarDaysCacheKey;
        }
        if (params.lunarDaysCacheValue !== undefined) {
            this.lunarDaysCacheValue = params.lunarDaysCacheValue;
        }
        if (params.convertResultCacheKey !== undefined) {
            this.convertResultCacheKey = params.convertResultCacheKey;
        }
        if (params.convertResultCacheValue !== undefined) {
            this.convertResultCacheValue = params.convertResultCacheValue;
        }
    }
    updateStateVars(params) {
        this.__options.reset(params.options);
        this.__selectionIndicator.reset(params.selectionIndicator);
        this.__selectedTextStyle.reset(params.selectedTextStyle);
        this.__itemTextStyle.reset(params.itemTextStyle);
    }
    purgeVariableDependenciesOnElmtId(rmElmtId) {
        this.__options.purgeDependencyOnElmtId(rmElmtId);
        this.__selectionIndicator.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedTextStyle.purgeDependencyOnElmtId(rmElmtId);
        this.__itemTextStyle.purgeDependencyOnElmtId(rmElmtId);
        this.__displayMode.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedYear.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedMonth.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedIsLeapMonth.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedDay.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedHour.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedMinute.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedSecond.purgeDependencyOnElmtId(rmElmtId);
        this.__startYear.purgeDependencyOnElmtId(rmElmtId);
        this.__endYear.purgeDependencyOnElmtId(rmElmtId);
        this.__startMonth.purgeDependencyOnElmtId(rmElmtId);
        this.__endMonth.purgeDependencyOnElmtId(rmElmtId);
        this.__startDay.purgeDependencyOnElmtId(rmElmtId);
        this.__endDay.purgeDependencyOnElmtId(rmElmtId);
        this.__lunarStartMonth.purgeDependencyOnElmtId(rmElmtId);
        this.__lunarEndMonth.purgeDependencyOnElmtId(rmElmtId);
        this.__lunarStartDay.purgeDependencyOnElmtId(rmElmtId);
        this.__lunarEndDay.purgeDependencyOnElmtId(rmElmtId);
        this.__lunarStartYear.purgeDependencyOnElmtId(rmElmtId);
        this.__lunarEndYear.purgeDependencyOnElmtId(rmElmtId);
        this.__startHour.purgeDependencyOnElmtId(rmElmtId);
        this.__endHour.purgeDependencyOnElmtId(rmElmtId);
        this.__startMinute.purgeDependencyOnElmtId(rmElmtId);
        this.__endMinute.purgeDependencyOnElmtId(rmElmtId);
        this.__startSecond.purgeDependencyOnElmtId(rmElmtId);
        this.__endSecond.purgeDependencyOnElmtId(rmElmtId);
        this.__dateMode.purgeDependencyOnElmtId(rmElmtId);
        this.__timeFormat.purgeDependencyOnElmtId(rmElmtId);
        this.__useMilitaryTime.purgeDependencyOnElmtId(rmElmtId);
        this.__lunar.purgeDependencyOnElmtId(rmElmtId);
        this.__hideYear.purgeDependencyOnElmtId(rmElmtId);
        this.__dateCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__timeCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__dateStartEndSet.purgeDependencyOnElmtId(rmElmtId);
        this.__timeStartEndSet.purgeDependencyOnElmtId(rmElmtId);
        this.__yearColumnCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__monthColumnCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__dayColumnCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__hourColumnCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__minuteColumnCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__secondColumnCanLoop.purgeDependencyOnElmtId(rmElmtId);
        this.__dateHapticFeedback.purgeDependencyOnElmtId(rmElmtId);
        this.__timeHapticFeedback.purgeDependencyOnElmtId(rmElmtId);
        this.__yearArray.purgeDependencyOnElmtId(rmElmtId);
        this.__monthArray.purgeDependencyOnElmtId(rmElmtId);
        this.__dayArray.purgeDependencyOnElmtId(rmElmtId);
        this.__hourArray.purgeDependencyOnElmtId(rmElmtId);
        this.__minuteArray.purgeDependencyOnElmtId(rmElmtId);
        this.__secondArray.purgeDependencyOnElmtId(rmElmtId);
        this.__periodArray.purgeDependencyOnElmtId(rmElmtId);
        this.__selectedPeriod.purgeDependencyOnElmtId(rmElmtId);
        this.__currentLocale.purgeDependencyOnElmtId(rmElmtId);
        this.__firstColumnWidth.purgeDependencyOnElmtId(rmElmtId);
        this.__lastColumnWidth.purgeDependencyOnElmtId(rmElmtId);
    }
    aboutToBeDeleted() {
        this.__options.aboutToBeDeleted();
        this.__selectionIndicator.aboutToBeDeleted();
        this.__selectedTextStyle.aboutToBeDeleted();
        this.__itemTextStyle.aboutToBeDeleted();
        this.__displayMode.aboutToBeDeleted();
        this.__selectedYear.aboutToBeDeleted();
        this.__selectedMonth.aboutToBeDeleted();
        this.__selectedIsLeapMonth.aboutToBeDeleted();
        this.__selectedDay.aboutToBeDeleted();
        this.__selectedHour.aboutToBeDeleted();
        this.__selectedMinute.aboutToBeDeleted();
        this.__selectedSecond.aboutToBeDeleted();
        this.__startYear.aboutToBeDeleted();
        this.__endYear.aboutToBeDeleted();
        this.__startMonth.aboutToBeDeleted();
        this.__endMonth.aboutToBeDeleted();
        this.__startDay.aboutToBeDeleted();
        this.__endDay.aboutToBeDeleted();
        this.__lunarStartMonth.aboutToBeDeleted();
        this.__lunarEndMonth.aboutToBeDeleted();
        this.__lunarStartDay.aboutToBeDeleted();
        this.__lunarEndDay.aboutToBeDeleted();
        this.__lunarStartYear.aboutToBeDeleted();
        this.__lunarEndYear.aboutToBeDeleted();
        this.__startHour.aboutToBeDeleted();
        this.__endHour.aboutToBeDeleted();
        this.__startMinute.aboutToBeDeleted();
        this.__endMinute.aboutToBeDeleted();
        this.__startSecond.aboutToBeDeleted();
        this.__endSecond.aboutToBeDeleted();
        this.__dateMode.aboutToBeDeleted();
        this.__timeFormat.aboutToBeDeleted();
        this.__useMilitaryTime.aboutToBeDeleted();
        this.__lunar.aboutToBeDeleted();
        this.__hideYear.aboutToBeDeleted();
        this.__dateCanLoop.aboutToBeDeleted();
        this.__timeCanLoop.aboutToBeDeleted();
        this.__dateStartEndSet.aboutToBeDeleted();
        this.__timeStartEndSet.aboutToBeDeleted();
        this.__yearColumnCanLoop.aboutToBeDeleted();
        this.__monthColumnCanLoop.aboutToBeDeleted();
        this.__dayColumnCanLoop.aboutToBeDeleted();
        this.__hourColumnCanLoop.aboutToBeDeleted();
        this.__minuteColumnCanLoop.aboutToBeDeleted();
        this.__secondColumnCanLoop.aboutToBeDeleted();
        this.__dateHapticFeedback.aboutToBeDeleted();
        this.__timeHapticFeedback.aboutToBeDeleted();
        this.__yearArray.aboutToBeDeleted();
        this.__monthArray.aboutToBeDeleted();
        this.__dayArray.aboutToBeDeleted();
        this.__hourArray.aboutToBeDeleted();
        this.__minuteArray.aboutToBeDeleted();
        this.__secondArray.aboutToBeDeleted();
        this.__periodArray.aboutToBeDeleted();
        this.__selectedPeriod.aboutToBeDeleted();
        this.__currentLocale.aboutToBeDeleted();
        this.__firstColumnWidth.aboutToBeDeleted();
        this.__lastColumnWidth.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    get options() {
        return this.__options.get();
    }
    set options(newValue) {
        this.__options.set(newValue);
    }
    get selectionIndicator() {
        return this.__selectionIndicator.get();
    }
    set selectionIndicator(newValue) {
        this.__selectionIndicator.set(newValue);
    }
    get selectedTextStyle() {
        return this.__selectedTextStyle.get();
    }
    set selectedTextStyle(newValue) {
        this.__selectedTextStyle.set(newValue);
    }
    get itemTextStyle() {
        return this.__itemTextStyle.get();
    }
    set itemTextStyle(newValue) {
        this.__itemTextStyle.set(newValue);
    }
    get displayMode() {
        return this.__displayMode.get();
    }
    set displayMode(newValue) {
        this.__displayMode.set(newValue);
    }
    get selectedYear() {
        return this.__selectedYear.get();
    }
    set selectedYear(newValue) {
        this.__selectedYear.set(newValue);
    }
    get selectedMonth() {
        return this.__selectedMonth.get();
    }
    set selectedMonth(newValue) {
        this.__selectedMonth.set(newValue);
    }
    get selectedIsLeapMonth() {
        return this.__selectedIsLeapMonth.get();
    }
    set selectedIsLeapMonth(newValue) {
        this.__selectedIsLeapMonth.set(newValue);
    }
    get selectedDay() {
        return this.__selectedDay.get();
    }
    set selectedDay(newValue) {
        this.__selectedDay.set(newValue);
    }
    get selectedHour() {
        return this.__selectedHour.get();
    }
    set selectedHour(newValue) {
        this.__selectedHour.set(newValue);
    }
    get selectedMinute() {
        return this.__selectedMinute.get();
    }
    set selectedMinute(newValue) {
        this.__selectedMinute.set(newValue);
    }
    get selectedSecond() {
        return this.__selectedSecond.get();
    }
    set selectedSecond(newValue) {
        this.__selectedSecond.set(newValue);
    }
    get startYear() {
        return this.__startYear.get();
    }
    set startYear(newValue) {
        this.__startYear.set(newValue);
    }
    get endYear() {
        return this.__endYear.get();
    }
    set endYear(newValue) {
        this.__endYear.set(newValue);
    }
    get startMonth() {
        return this.__startMonth.get();
    }
    set startMonth(newValue) {
        this.__startMonth.set(newValue);
    }
    get endMonth() {
        return this.__endMonth.get();
    }
    set endMonth(newValue) {
        this.__endMonth.set(newValue);
    }
    get startDay() {
        return this.__startDay.get();
    }
    set startDay(newValue) {
        this.__startDay.set(newValue);
    }
    get endDay() {
        return this.__endDay.get();
    }
    set endDay(newValue) {
        this.__endDay.set(newValue);
    }
    get lunarStartMonth() {
        return this.__lunarStartMonth.get();
    }
    set lunarStartMonth(newValue) {
        this.__lunarStartMonth.set(newValue);
    }
    get lunarEndMonth() {
        return this.__lunarEndMonth.get();
    }
    set lunarEndMonth(newValue) {
        this.__lunarEndMonth.set(newValue);
    }
    get lunarStartDay() {
        return this.__lunarStartDay.get();
    }
    set lunarStartDay(newValue) {
        this.__lunarStartDay.set(newValue);
    }
    get lunarEndDay() {
        return this.__lunarEndDay.get();
    }
    set lunarEndDay(newValue) {
        this.__lunarEndDay.set(newValue);
    }
    get lunarStartYear() {
        return this.__lunarStartYear.get();
    }
    set lunarStartYear(newValue) {
        this.__lunarStartYear.set(newValue);
    }
    get lunarEndYear() {
        return this.__lunarEndYear.get();
    }
    set lunarEndYear(newValue) {
        this.__lunarEndYear.set(newValue);
    }
    get startHour() {
        return this.__startHour.get();
    }
    set startHour(newValue) {
        this.__startHour.set(newValue);
    }
    get endHour() {
        return this.__endHour.get();
    }
    set endHour(newValue) {
        this.__endHour.set(newValue);
    }
    get startMinute() {
        return this.__startMinute.get();
    }
    set startMinute(newValue) {
        this.__startMinute.set(newValue);
    }
    get endMinute() {
        return this.__endMinute.get();
    }
    set endMinute(newValue) {
        this.__endMinute.set(newValue);
    }
    get startSecond() {
        return this.__startSecond.get();
    }
    set startSecond(newValue) {
        this.__startSecond.set(newValue);
    }
    get endSecond() {
        return this.__endSecond.get();
    }
    set endSecond(newValue) {
        this.__endSecond.set(newValue);
    }
    get dateMode() {
        return this.__dateMode.get();
    }
    set dateMode(newValue) {
        this.__dateMode.set(newValue);
    }
    get timeFormat() {
        return this.__timeFormat.get();
    }
    set timeFormat(newValue) {
        this.__timeFormat.set(newValue);
    }
    get useMilitaryTime() {
        return this.__useMilitaryTime.get();
    }
    set useMilitaryTime(newValue) {
        this.__useMilitaryTime.set(newValue);
    }
    get lunar() {
        return this.__lunar.get();
    }
    set lunar(newValue) {
        this.__lunar.set(newValue);
    }
    get hideYear() {
        return this.__hideYear.get();
    }
    set hideYear(newValue) {
        this.__hideYear.set(newValue);
    }
    get dateCanLoop() {
        return this.__dateCanLoop.get();
    }
    set dateCanLoop(newValue) {
        this.__dateCanLoop.set(newValue);
    }
    get timeCanLoop() {
        return this.__timeCanLoop.get();
    }
    set timeCanLoop(newValue) {
        this.__timeCanLoop.set(newValue);
    }
    get dateStartEndSet() {
        return this.__dateStartEndSet.get();
    }
    set dateStartEndSet(newValue) {
        this.__dateStartEndSet.set(newValue);
    }
    get timeStartEndSet() {
        return this.__timeStartEndSet.get();
    }
    set timeStartEndSet(newValue) {
        this.__timeStartEndSet.set(newValue);
    }
    get yearColumnCanLoop() {
        return this.__yearColumnCanLoop.get();
    }
    set yearColumnCanLoop(newValue) {
        this.__yearColumnCanLoop.set(newValue);
    }
    get monthColumnCanLoop() {
        return this.__monthColumnCanLoop.get();
    }
    set monthColumnCanLoop(newValue) {
        this.__monthColumnCanLoop.set(newValue);
    }
    get dayColumnCanLoop() {
        return this.__dayColumnCanLoop.get();
    }
    set dayColumnCanLoop(newValue) {
        this.__dayColumnCanLoop.set(newValue);
    }
    get hourColumnCanLoop() {
        return this.__hourColumnCanLoop.get();
    }
    set hourColumnCanLoop(newValue) {
        this.__hourColumnCanLoop.set(newValue);
    }
    get minuteColumnCanLoop() {
        return this.__minuteColumnCanLoop.get();
    }
    set minuteColumnCanLoop(newValue) {
        this.__minuteColumnCanLoop.set(newValue);
    }
    get secondColumnCanLoop() {
        return this.__secondColumnCanLoop.get();
    }
    set secondColumnCanLoop(newValue) {
        this.__secondColumnCanLoop.set(newValue);
    }
    get dateHapticFeedback() {
        return this.__dateHapticFeedback.get();
    }
    set dateHapticFeedback(newValue) {
        this.__dateHapticFeedback.set(newValue);
    }
    get timeHapticFeedback() {
        return this.__timeHapticFeedback.get();
    }
    set timeHapticFeedback(newValue) {
        this.__timeHapticFeedback.set(newValue);
    }
    get yearArray() {
        return this.__yearArray.get();
    }
    set yearArray(newValue) {
        this.__yearArray.set(newValue);
    }
    get monthArray() {
        return this.__monthArray.get();
    }
    set monthArray(newValue) {
        this.__monthArray.set(newValue);
    }
    get dayArray() {
        return this.__dayArray.get();
    }
    set dayArray(newValue) {
        this.__dayArray.set(newValue);
    }
    get hourArray() {
        return this.__hourArray.get();
    }
    set hourArray(newValue) {
        this.__hourArray.set(newValue);
    }
    get minuteArray() {
        return this.__minuteArray.get();
    }
    set minuteArray(newValue) {
        this.__minuteArray.set(newValue);
    }
    get secondArray() {
        return this.__secondArray.get();
    }
    set secondArray(newValue) {
        this.__secondArray.set(newValue);
    }
    get periodArray() {
        return this.__periodArray.get();
    }
    set periodArray(newValue) {
        this.__periodArray.set(newValue);
    }
    get selectedPeriod() {
        return this.__selectedPeriod.get();
    }
    set selectedPeriod(newValue) {
        this.__selectedPeriod.set(newValue);
    }
    get currentLocale() {
        return this.__currentLocale.get();
    }
    set currentLocale(newValue) {
        this.__currentLocale.set(newValue);
    }
    get firstColumnWidth() {
        return this.__firstColumnWidth.get();
    }
    set firstColumnWidth(newValue) {
        this.__firstColumnWidth.set(newValue);
    }
    get lastColumnWidth() {
        return this.__lastColumnWidth.get();
    }
    set lastColumnWidth(newValue) {
        this.__lastColumnWidth.set(newValue);
    }
    aboutToAppear() {
        // Try to get system language, fallback to zh-CN for previewer environment
        this.currentLocale = i18n.System.getSystemLanguage();
        if (!this.currentLocale || this.currentLocale.length === 0) {
            this.currentLocale = 'zh-CN'; // Fallback for previewer
        }
        this.locale = new intl.Locale(this.currentLocale);
        this.formatter = new intl.NumberFormat(this.locale.toString());
        this.yearFormatter = new intl.NumberFormat(this.locale.toString(), { useGrouping: false });
        // Save user-set lunar value, decide whether to apply based on current language
        this.userLunar = this.lunar;
        const isChinese = this.isChineseLocale();
        if (!isChinese) {
            // Static scenario: non-Chinese language, force lunar to false
            this.lunar = false;
            this.lunarCalendar = null;
        }
        else {
            // Chinese language: apply lunar based on user setting
            if (this.lunar) {
                this.lunarCalendar = i18n.getCalendar(this.locale.toString(), 'chinese');
            }
        }
        if (this.selectedYear === 0 || this.selectedMonth === 0 || this.selectedDay === 0) {
            const now = new Date();
            this.selectedYear = now.getFullYear();
            this.selectedMonth = now.getMonth();
            this.selectedDay = now.getDate();
            this.selectedHour = now.getHours();
            this.selectedMinute = now.getMinutes();
            this.selectedSecond = now.getSeconds();
        }
        if (this.options !== undefined && this.options !== null) {
            this.onOptionsChange();
        }
        else {
            this.initArrays();
        }
        this.isComponentActive = true;
        this.createLocaleChangeSubscriber();
    }
    aboutToDisappear() {
        this.isComponentActive = false;
        this.lunarCalendar = null;
        // Unsubscribe from locale change events
        if (this.subscriber !== null) {
            commonEventManager.unsubscribe(this.subscriber, (err) => {
                if (err) {
                    console.error(`Failed to unsubscribe common event. error code: ${err.code}, message: ${err.message}.`);
                }
            });
            this.subscriber = null;
        }
    }
    createLocaleChangeSubscriber() {
        const subscribeInfo = {
            events: [commonEventManager.Support.COMMON_EVENT_LOCALE_CHANGED]
        };
        commonEventManager.createSubscriber(subscribeInfo)
            .then((commonEventSubscriber) => {
            if (!this.isComponentActive) {
                commonEventManager.unsubscribe(commonEventSubscriber, (err) => {
                    if (err) {
                        console.error(`Component destroyed, unsubscribe failed. code: ${err.code}, message: ${err.message}.`);
                    }
                });
                return;
            }
            this.subscriber = commonEventSubscriber;
            commonEventManager.subscribe(this.subscriber, (err, data) => {
                if (err) {
                    console.error(`Failed to subscribe common event. error code: ${err.code}, message: ${err.message}.`);
                    return;
                }
                // Update locale when system language changes
                this.currentLocale = i18n.System.getSystemLanguage();
            });
        })
            .catch((err) => {
            console.error(`CreateSubscriber failed, code is ${err.message}`);
        });
    }
    onLocaleChange() {
        // Ensure currentLocale is valid, fallback to zh-CN for previewer environment
        if (!this.currentLocale || this.currentLocale.length === 0) {
            this.currentLocale = 'zh-CN';
        }
        this.locale = new intl.Locale(this.currentLocale);
        this.formatter = new intl.NumberFormat(this.locale.toString());
        this.yearFormatter = new intl.NumberFormat(this.locale.toString(), { useGrouping: false });
        // Reset cached formatters when locale changes
        this.hourFormatter = null;
        this.minuteFormatter = null;
        this.secondFormatter = null;
        const isChinese = this.isChineseLocale();
        const prevLunar = this.lunar;
        const prevLunarCalendar = this.lunarCalendar;
        // Handle lunar calendar based on language
        if (isChinese) {
            // Chinese locale: use user-set lunar value
            this.lunar = this.userLunar;
            if (this.lunar) {
                this.lunarCalendar = i18n.getCalendar(this.locale.toString(), 'chinese');
            }
            else {
                this.lunarCalendar = null;
            }
        }
        else {
            // Non-Chinese locale: disable lunar calendar but keep user setting
            this.lunar = false;
            this.lunarCalendar = null;
        }
        // Convert date when switching between lunar and gregorian modes
        if (prevLunar !== this.lunar) {
            if (!prevLunar && this.lunar && this.lunarCalendar !== null) {
                // Switching from gregorian to lunar: convert gregorian date to lunar date
                const gregorianDate = this.createDateFromParams(this.selectedYear, this.selectedMonth, this.selectedDay);
                this.lunarCalendar.setTime(gregorianDate);
                this.selectedMonth = this.lunarCalendar.get('month');
                this.selectedDay = this.lunarCalendar.get('date');
                // Use Gregorian year (not lunarCalendar.get('year') which returns
                // sexagenary cycle 1-60). Adjust for lunar year boundary:
                // if lunar month is 腊月(11) and Gregorian month is Jan/Feb,
                // the lunar year is the previous Gregorian year.
                this.selectedYear = gregorianDate.getFullYear();
                if (this.selectedMonth >= 11 && gregorianDate.getMonth() <= 1) {
                    this.selectedYear -= 1;
                }
                if (this.selectedYear < this.startYear) {
                    this.selectedYear = this.startYear;
                }
                if (this.selectedYear > this.endYear) {
                    this.selectedYear = this.endYear;
                }
                // Detect if the date falls in a leap month
                this.selectedIsLeapMonth = this.isDateInLeapMonth(gregorianDate, this.selectedYear, this.selectedMonth);
            }
            else if (prevLunar && !this.lunar && prevLunarCalendar !== null) {
                // Switching from lunar to gregorian: convert lunar date to gregorian date
                // Use prevLunarCalendar for conversion since current lunarCalendar is null
                const gregorianDate = this.convertLunarToGregorianWithCalendar(prevLunarCalendar, this.selectedYear, this.selectedMonth, this.selectedDay, this.selectedIsLeapMonth);
                if (gregorianDate !== null) {
                    this.selectedYear = gregorianDate.getFullYear();
                    this.selectedMonth = gregorianDate.getMonth();
                    this.selectedDay = gregorianDate.getDate();
                }
                this.selectedIsLeapMonth = false;
            }
        }
        this.initArrays();
    }
    initArrays() {
        if (this.lunar) {
            this.initLunarArrays();
        }
        else {
            this.initYearArray();
            this.initMonthArray();
            this.initDayArray();
        }
        this.initTimeArrays();
        this.updateDateColumnLoopState();
        this.updateTimeColumnLoopState();
    }
    initLunarArrays() {
        const startYear = this.lunar ? this.lunarStartYear : this.startYear;
        const endYear = this.lunar ? this.lunarEndYear : this.endYear;
        this.yearArray = [];
        for (let i = startYear; i <= endYear; i++) {
            this.yearArray.push(this.formatLunarYear(i));
        }
        if (this.hideYear) {
            this.yearArray.push(DatePickerConstant.HIDE_YEAR_TEXT);
        }
        let startLunarMonthIndex = DatePickerConstant.MIN_MONTH;
        let endLunarMonthIndex = DatePickerConstant.MAX_MONTH;
        if (this.selectedYear === startYear) {
            startLunarMonthIndex = this.lunarStartMonth;
        }
        if (this.selectedYear === endYear) {
            endLunarMonthIndex = this.lunarEndMonth;
        }
        // Build month array with leap months inserted
        this.monthArray = [];
        const leapMonth = this.getLunarLeapMonth(this.selectedYear);
        for (let i = startLunarMonthIndex; i <= endLunarMonthIndex; i++) {
            this.monthArray.push(this.formatLunarMonth(i, false));
            if (leapMonth >= 0 && i === leapMonth) {
                this.monthArray.push(this.formatLunarMonth(i, true));
            }
        }
        this.initLunarDayArray();
    }
    initLunarDayArray() {
        this.dayArray = [];
        const lunarDays = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
        let startLunarDayIndex = DatePickerConstant.MIN_DAY;
        let endLunarDayIndex = lunarDays;
        if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
            !this.selectedIsLeapMonth) {
            startLunarDayIndex = this.lunarStartDay;
        }
        if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
            !this.selectedIsLeapMonth) {
            endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays);
        }
        for (let i = startLunarDayIndex; i <= endLunarDayIndex; i++) {
            this.dayArray.push(this.formatLunarDay(i));
        }
    }
    getLunarLeapMonth(gregorianYear) {
        if (gregorianYear === DatePickerConstant.HIDE_YEAR_SENTINEL || this.lunarCalendar === null) {
            return -1;
        }
        if (gregorianYear === this.lunarLeapMonthCacheKey) {
            return this.lunarLeapMonthCacheValue;
        }
        try {
            const refDate = this.createDateFromParams(gregorianYear, 6, 1);
            this.lunarCalendar.setTime(refDate);
            const expectedLunarYear = this.lunarCalendar.get('year');
            const monthStartCounts = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
            let leapMonth = -1;
            const startDate = this.createDateFromParams(gregorianYear, 0, 1);
            for (let dayOffset = 0; dayOffset < 470; dayOffset++) {
                const testDate = new Date(startDate.getTime() + dayOffset * 86400000);
                this.lunarCalendar.setTime(testDate);
                if (this.lunarCalendar.get('year') !== expectedLunarYear) {
                    if (leapMonth >= 0) {
                        break;
                    }
                    continue;
                }
                if (this.lunarCalendar.get('date') === 1) {
                    const m = this.lunarCalendar.get('month');
                    monthStartCounts[m]++;
                    if (monthStartCounts[m] === 2) {
                        leapMonth = m;
                    }
                }
            }
            this.lunarLeapMonthCacheKey = gregorianYear;
            this.lunarLeapMonthCacheValue = leapMonth;
            return leapMonth;
        }
        catch (e) {
            return -1;
        }
    }
    /**
     * Determines whether a Gregorian date falls in the leap month (2nd occurrence)
     * of the given lunar month. Returns false if the month has no leap month or
     * the date is in the regular (1st) occurrence.
     */
    isDateInLeapMonth(date, gregorianYear, lunarMonth) {
        const leapMonth = this.getLunarLeapMonth(gregorianYear);
        if (leapMonth < 0 || leapMonth !== lunarMonth || this.lunarCalendar === null) {
            return false;
        }
        // Find the start of the regular month (1st occurrence) and leap month (2nd occurrence)
        const regularStart = this.findLunarMonthStart(gregorianYear, lunarMonth, 1);
        if (regularStart === null) {
            return false;
        }
        // If the date is before the end of the regular month, it's not in the leap month.
        // The regular month typically has 29 or 30 days; we check if the date is past
        // the regular month by comparing with the leap month start.
        const leapStart = this.findLunarMonthStart(gregorianYear, lunarMonth, 2);
        if (leapStart === null) {
            return false;
        }
        return date.getTime() >= leapStart.getTime();
    }
    /**
     * Finds the Gregorian date for the start (lunar day==1) of the Nth occurrence
     * (1=regular, 2=leap) of the given lunar month in the given lunar year.
     */
    findLunarMonthStart(gregorianYear, lunarMonth, occurrence) {
        if (this.lunarCalendar === null) {
            return null;
        }
        try {
            const refDate = this.createDateFromParams(gregorianYear, 6, 1);
            this.lunarCalendar.setTime(refDate);
            const expectedLunarYear = this.lunarCalendar.get('year');
            let count = 0;
            const startDate = this.createDateFromParams(gregorianYear, 0, 1);
            for (let dayOffset = 0; dayOffset < 470; dayOffset++) {
                const testDate = new Date(startDate.getTime() + dayOffset * 86400000);
                this.lunarCalendar.setTime(testDate);
                if (this.lunarCalendar.get('year') !== expectedLunarYear) {
                    continue;
                }
                if (this.lunarCalendar.get('month') === lunarMonth && this.lunarCalendar.get('date') === 1) {
                    count++;
                    if (count === occurrence) {
                        return testDate;
                    }
                }
            }
        }
        catch (e) {
            // fall through
        }
        return null;
    }
    getLunarDaysInMonth(gregorianYear, lunarMonth, isLeap) {
        if (gregorianYear === DatePickerConstant.HIDE_YEAR_SENTINEL || this.lunarCalendar === null) {
            return 30;
        }
        const cacheKey = `${gregorianYear}_${lunarMonth}_${isLeap}`;
        if (cacheKey === this.lunarDaysCacheKey) {
            return this.lunarDaysCacheValue;
        }
        try {
            // Determine the lunar year from a mid-year reference date
            const refDate = this.createDateFromParams(gregorianYear, 6, 1);
            this.lunarCalendar.setTime(refDate);
            const expectedLunarYear = this.lunarCalendar.get('year');
            // For isLeap=true, we need the 2nd occurrence of (lunarMonth, date=1);
            // for isLeap=false, we need the 1st occurrence.
            const targetOccurrence = isLeap ? 2 : 1;
            let occurrenceCount = 0;
            let monthStart = null;
            let nextMonthStart = null;
            // Scan through ~400 days to find the target month start and the next month start.
            // The expectedLunarYear check only applies when searching for monthStart;
            // nextMonthStart can be in the next lunar year (e.g., 腊月 → 正月 of next year).
            const startDate = this.createDateFromParams(gregorianYear, 0, 1);
            for (let dayOffset = 0; dayOffset < 470 && nextMonthStart === null; dayOffset++) {
                const testDate = new Date(startDate.getTime() + dayOffset * 86400000);
                this.lunarCalendar.setTime(testDate);
                const currentLunarDay = this.lunarCalendar.get('date');
                if (currentLunarDay === 1) {
                    if (monthStart === null) {
                        // Haven't found the target month start yet — must be in the expected lunar year
                        if (this.lunarCalendar.get('year') === expectedLunarYear) {
                            const currentLunarMonth = this.lunarCalendar.get('month');
                            if (currentLunarMonth === lunarMonth) {
                                occurrenceCount++;
                                if (occurrenceCount === targetOccurrence) {
                                    monthStart = testDate;
                                }
                            }
                        }
                    }
                    else {
                        // Found the target month start, now look for the next month start (any lunar year)
                        nextMonthStart = testDate;
                    }
                }
            }
            let result = 30;
            if (monthStart !== null && nextMonthStart !== null) {
                result = Math.round((nextMonthStart.getTime() - monthStart.getTime()) / 86400000);
            }
            this.lunarDaysCacheKey = cacheKey;
            this.lunarDaysCacheValue = result;
            return result;
        }
        catch (e) {
            return 30;
        }
    }
    convertLunarToGregorian(gregorianYear, lunarMonth, lunarDay, isLeap) {
        return this.convertLunarToGregorianWithCalendar(this.lunarCalendar, gregorianYear, lunarMonth, lunarDay, isLeap);
    }
    convertLunarToGregorianWithCalendar(calendar, gregorianYear, lunarMonth, lunarDay, isLeap) {
        if (calendar === null) {
            return null;
        }
        const cacheKey = `${gregorianYear}_${lunarMonth}_${lunarDay}_${isLeap}`;
        if (cacheKey === this.convertResultCacheKey) {
            return this.convertResultCacheValue;
        }
        try {
            // Determine the lunar year from a mid-year reference date
            const refDate = this.createDateFromParams(gregorianYear, 6, 1);
            calendar.setTime(refDate);
            const expectedLunarYear = calendar.get('year');
            // Find the start of the target month occurrence (1st = regular, 2nd = leap)
            const targetOccurrence = isLeap ? 2 : 1;
            let occurrenceCount = 0;
            let monthStart = null;
            const startDate = this.createDateFromParams(gregorianYear, 0, 1);
            for (let dayOffset = 0; dayOffset < 470; dayOffset++) {
                const testDate = new Date(startDate.getTime() + dayOffset * 86400000);
                calendar.setTime(testDate);
                if (calendar.get('year') !== expectedLunarYear) {
                    continue;
                }
                if (calendar.get('month') === lunarMonth && calendar.get('date') === 1) {
                    occurrenceCount++;
                    if (occurrenceCount === targetOccurrence) {
                        monthStart = testDate;
                        break;
                    }
                }
            }
            let resultDate = null;
            if (monthStart !== null) {
                const candidate = new Date(monthStart.getTime() + (lunarDay - 1) * 86400000);
                calendar.setTime(candidate);
                if (calendar.get('month') === lunarMonth && calendar.get('date') === lunarDay) {
                    resultDate = candidate;
                }
            }
            this.convertResultCacheKey = cacheKey;
            this.convertResultCacheValue = resultDate;
            return resultDate;
        }
        catch (e) {
            return null;
        }
    }
    formatLunarYear(gregorianYear) {
        // Try formatter for localization, fallback to plain string
        if (this.yearFormatter) {
            try {
                return `${this.yearFormatter.format(gregorianYear)}年`;
            }
            catch (error) {
                return `${gregorianYear}年`;
            }
        }
        return `${gregorianYear}年`;
    }
    formatLunarMonth(month, isLeap) {
        const lunarMonthNames = ['正月', '二月', '三月', '四月', '五月', '六月',
            '七月', '八月', '九月', '十月', '冬月', '腊月'];
        if (isLeap) {
            return `闰${lunarMonthNames[month]}`;
        }
        return lunarMonthNames[month];
    }
    formatLunarDay(day) {
        const lunarDayNames = [
            '初一', '初二', '初三', '初四', '初五',
            '初六', '初七', '初八', '初九', '初十',
            '十一', '十二', '十三', '十四', '十五',
            '十六', '十七', '十八', '十九', '二十',
            '廿一', '廿二', '廿三', '廿四', '廿五',
            '廿六', '廿七', '廿八', '廿九', '三十'
        ];
        if (day >= 1 && day <= 30) {
            return lunarDayNames[day - 1];
        }
        return `${day}日`;
    }
    initYearArray() {
        this.yearArray = [];
        for (let i = this.startYear; i <= this.endYear; i++) {
            this.yearArray.push(this.formatYear(i));
        }
        if (this.hideYear) {
            this.yearArray.push(DatePickerConstant.HIDE_YEAR_TEXT);
        }
    }
    initMonthArray() {
        this.monthArray = [];
        let startMonthIndex = DatePickerConstant.MIN_MONTH;
        let endMonthIndex = DatePickerConstant.MAX_MONTH;
        if (this.selectedYear === this.startYear) {
            startMonthIndex = this.startMonth;
        }
        if (this.selectedYear === this.endYear) {
            endMonthIndex = this.endMonth;
        }
        for (let i = startMonthIndex; i <= endMonthIndex; i++) {
            this.monthArray.push(this.formatMonth(i));
        }
    }
    initDayArray() {
        this.dayArray = [];
        const daysInMonth = this.getDaysInMonth(this.selectedYear, this.selectedMonth);
        let startDayIndex = DatePickerConstant.MIN_DAY;
        let endDayIndex = daysInMonth;
        if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
            startDayIndex = this.startDay;
        }
        if (this.selectedYear === this.endYear && this.selectedMonth === this.endMonth) {
            endDayIndex = Math.min(this.endDay, daysInMonth);
        }
        for (let i = startDayIndex; i <= endDayIndex; i++) {
            this.dayArray.push(this.formatDay(i));
        }
    }
    initTimeArrays() {
        this.periodArray = [];
        if (!this.useMilitaryTime) {
            this.selectedPeriod = this.selectedHour < 12 ? 0 : 1;
            this.updatePeriodArray();
            this.updateHourArrayFor12Hour();
        }
        else {
            this.hourArray = [];
            for (let i = this.startHour; i <= this.endHour; i++) {
                this.hourArray.push(this.formatHour(i));
            }
        }
        this.minuteArray = [];
        let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
        let endMinuteIndex = DatePickerConstant.MAX_MINUTE;
        if (this.selectedHour === this.startHour) {
            startMinuteIndex = this.startMinute;
        }
        if (this.selectedHour === this.endHour) {
            endMinuteIndex = this.endMinute;
        }
        for (let i = startMinuteIndex; i <= endMinuteIndex; i++) {
            this.minuteArray.push(this.formatMinute(i));
        }
        if (this.selectedMinute < startMinuteIndex) {
            this.selectedMinute = startMinuteIndex;
        }
        if (this.selectedMinute > endMinuteIndex) {
            this.selectedMinute = endMinuteIndex;
        }
        this.secondArray = [];
        let startSecondIndex = DatePickerConstant.MIN_SECOND;
        let endSecondIndex = DatePickerConstant.MAX_SECOND;
        if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
            startSecondIndex = this.startSecond;
        }
        if (this.selectedHour === this.endHour && this.selectedMinute === this.endMinute) {
            endSecondIndex = this.endSecond;
        }
        for (let i = startSecondIndex; i <= endSecondIndex; i++) {
            this.secondArray.push(this.formatSecond(i));
        }
        if (this.selectedSecond < startSecondIndex) {
            this.selectedSecond = startSecondIndex;
        }
        if (this.selectedSecond > endSecondIndex) {
            this.selectedSecond = endSecondIndex;
        }
    }
    updateHourArrayFor12Hour() {
        this.hourArray = [];
        const isAM = this.selectedPeriod === 0;
        // For 12-hour format, build hour array based on current period and time range
        // AM period: 12 (midnight 0:00) and 1-11 (1:00-11:00)
        // PM period: 12 (noon 12:00) and 1-11 (13:00-23:00)
        if (isAM) {
            // Check if 12 AM (hour24=0) is within range
            const hour24ForAm12 = 0;
            if (hour24ForAm12 >= this.startHour && hour24ForAm12 <= this.endHour) {
                this.hourArray.push(this.formatHour12(12));
            }
            // Check if 1-11 AM (hour24=1-11) are within range
            for (let displayHour = 1; displayHour <= 11; displayHour++) {
                const hour24 = displayHour;
                if (hour24 >= this.startHour && hour24 <= this.endHour) {
                    this.hourArray.push(this.formatHour12(displayHour));
                }
            }
        }
        else {
            // Check if 12 PM (hour24=12) is within range
            const hour24ForPm12 = 12;
            if (hour24ForPm12 >= this.startHour && hour24ForPm12 <= this.endHour) {
                this.hourArray.push(this.formatHour12(12));
            }
            // Check if 1-11 PM (hour24=13-23) are within range
            for (let displayHour = 1; displayHour <= 11; displayHour++) {
                const hour24 = displayHour + 12;
                if (hour24 >= this.startHour && hour24 <= this.endHour) {
                    this.hourArray.push(this.formatHour12(displayHour));
                }
            }
        }
        // Fallback: if no hours available for current period, show all 12 hours
        if (this.hourArray.length === 0) {
            for (let i = 1; i <= 12; i++) {
                this.hourArray.push(this.formatHour12(i));
            }
        }
    }
    updatePeriodArray() {
        this.periodArray = [];
        // Check if AM period (hour24=0-11) has any available hours
        let canSelectAM = false;
        for (let hour = 0; hour <= 11; hour++) {
            if (hour >= this.startHour && hour <= this.endHour) {
                canSelectAM = true;
                break;
            }
        }
        // Check if PM period (hour24=12-23) has any available hours
        let canSelectPM = false;
        for (let hour = 12; hour <= 23; hour++) {
            if (hour >= this.startHour && hour <= this.endHour) {
                canSelectPM = true;
                break;
            }
        }
        if (canSelectAM && canSelectPM) {
            this.periodArray.push(this.formatPeriod(true));
            this.periodArray.push(this.formatPeriod(false));
        }
        else if (canSelectAM && !canSelectPM) {
            this.periodArray.push(this.formatPeriod(true));
            if (this.selectedPeriod === 1) {
                this.selectedPeriod = 0;
                // Adjust selectedHour to AM range when forced to switch to AM
                if (this.selectedHour >= 12) {
                    const minAMHour = Math.max(0, this.startHour);
                    const maxAMHour = Math.min(11, this.endHour);
                    if (this.selectedHour === 12) {
                        this.selectedHour = 0; // 12 PM -> 12 AM (0:00)
                    }
                    else {
                        this.selectedHour = Math.max(minAMHour, Math.min(maxAMHour, this.selectedHour - 12));
                    }
                }
            }
        }
        else if (!canSelectAM && canSelectPM) {
            this.periodArray.push(this.formatPeriod(false));
            if (this.selectedPeriod === 0) {
                this.selectedPeriod = 1;
                // Adjust selectedHour to PM range when forced to switch to PM
                if (this.selectedHour < 12) {
                    const minPMHour = Math.max(12, this.startHour);
                    const maxPMHour = Math.min(23, this.endHour);
                    // Calculate target hour (24-hour format): 12 AM->12, 1-11 AM->13-23
                    let targetHour24 = this.selectedHour === 0 ? 12 : this.selectedHour + 12;
                    // Ensure target hour is within valid range
                    this.selectedHour = Math.max(minPMHour, Math.min(maxPMHour, targetHour24));
                }
            }
        }
        else {
            // Fallback: show both periods if no hours available (shouldn't happen with valid ranges)
            this.periodArray.push(this.formatPeriod(true));
            this.periodArray.push(this.formatPeriod(false));
        }
    }
    isChineseLocale() {
        const localeStr = this.locale.toString();
        return localeStr.startsWith('zh') || localeStr.includes('zh-CN') || localeStr.includes('zh-TW') || localeStr.includes('zh-Hans') || localeStr.includes('zh-Hant');
    }
    formatYear(year) {
        // Previewer or invalid locale: default to Chinese format
        const localeStr = this.locale?.toString();
        if (!localeStr || localeStr.length === 0) {
            return `${year}年`;
        }
        // Try to use formatter for localization on real device, fallback to plain string for previewer
        if (this.isChineseLocale()) {
            // Try formatter first, fallback to plain string if fails
            if (this.yearFormatter) {
                try {
                    return `${this.yearFormatter.format(year)}年`;
                }
                catch (error) {
                    return `${year}年`;
                }
            }
            return `${year}年`;
        }
        // Non-Chinese: try formatter for localized numbers, fallback to plain
        if (this.yearFormatter) {
            try {
                return this.yearFormatter.format(year);
            }
            catch (error) {
                return year.toString();
            }
        }
        return year.toString();
    }
    formatMonth(month) {
        // Previewer or invalid locale: default to Chinese format
        const localeStr = this.locale?.toString();
        if (!localeStr || localeStr.length === 0) {
            return `${month + 1}月`;
        }
        if (this.isChineseLocale()) {
            return `${month + 1}月`;
        }
        const monthNames = [
            'January', 'February', 'March', 'April', 'May', 'June',
            'July', 'August', 'September', 'October', 'November', 'December'
        ];
        const shortMonthNames = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
            'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
        try {
            const monthFormat = this.displayMode === DisplayMode.DATE_TIME ? 'short' : 'long';
            const dateFormat = new Intl.DateTimeFormat(localeStr, {
                month: monthFormat
            });
            const safeMonth = Math.max(0, Math.min(11, month));
            const date = new Date(2026, safeMonth, 15);
            return dateFormat.format(date);
        }
        catch (error) {
            return this.displayMode === DisplayMode.DATE_TIME ? shortMonthNames[month] : monthNames[month];
        }
    }
    formatDay(day) {
        // Previewer or invalid locale: default to Chinese format
        const localeStr = this.locale?.toString();
        if (!localeStr || localeStr.length === 0) {
            return `${day}日`;
        }
        // Try to use formatter for localization on real device, fallback to plain string for previewer
        if (this.isChineseLocale()) {
            if (this.formatter) {
                try {
                    return `${this.formatter.format(day)}日`;
                }
                catch (error) {
                    return `${day}日`;
                }
            }
            return `${day}日`;
        }
        // Non-Chinese: use DateTimeFormat for localized date format
        try {
            const localeStr = this.locale?.toString();
            if (!localeStr || localeStr.length === 0) {
                return `${day}`;
            }
            const dateFormat = new Intl.DateTimeFormat(localeStr, {
                day: 'numeric'
            });
            const safeDay = Math.max(1, Math.min(31, day));
            const date = new Date(2026, 0, safeDay);
            return dateFormat.format(date);
        }
        catch (error) {
            return `${day}`;
        }
    }
    formatHour(hour24) {
        // Try to use formatter for localization on real device, fallback to plain string for previewer
        // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
        try {
            const localeStr = this.locale?.toString();
            // 1. Previewer or invalid locale: simple padStart
            if (!localeStr || localeStr.length === 0) {
                return hour24.toString().padStart(2, '0');
            }
            // 2. Chinese/English: simple padStart sufficient (no digit localization needed)
            const lang = localeStr.split('-')[0];
            if (['zh', 'en'].includes(lang)) {
                return hour24.toString().padStart(2, '0');
            }
            // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
            if (this.hourFormatter === null) {
                this.hourFormatter = new Intl.DateTimeFormat(localeStr, {
                    hour: '2-digit',
                    hour12: false
                });
            }
            const date = new Date(2026, 0, 1, hour24, 0, 0);
            return this.hourFormatter.format(date);
        }
        catch (error) {
            return hour24.toString().padStart(2, '0');
        }
    }
    formatHour12(displayHour) {
        // Try to use formatter for localization on real device, fallback to plain string for previewer
        // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
        try {
            const localeStr = this.locale?.toString();
            // 1. Previewer or invalid locale: simple padStart
            if (!localeStr || localeStr.length === 0) {
                return displayHour.toString().padStart(2, '0');
            }
            // 2. Chinese/English: simple padStart sufficient (no digit localization needed)
            const lang = localeStr.split('-')[0];
            if (['zh', 'en'].includes(lang)) {
                return displayHour.toString().padStart(2, '0');
            }
            // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
            if (this.hourFormatter === null) {
                this.hourFormatter = new Intl.DateTimeFormat(localeStr, {
                    hour: '2-digit',
                    hour12: true
                });
            }
            const date = new Date(2026, 0, 1, displayHour, 0, 0);
            const formatted = this.hourFormatter.format(date);
            // Remove any non-digit characters (like AM/PM, spaces, etc.)
            // This gives us just the hour number in locale format
            // Include Unicode ranges for all localized digits:
            // Arabic-Indic, Persian, Devanagari, Bengali, Thai, Lao, Myanmar, Khmer
            const pattern = '[\\d\\u0660-\\u0669\\u06F0-\\u06F9]' +
                '[\\u0966-\\u096F\\u09E6-\\u09EF]' +
                '[\\u0E50-\\u0E59\\u0ED0-\\u0ED9]' +
                '[\\u1040-\\u1049\\u17E0-\\u17E9]+';
            const hourMatch = formatted.match(new RegExp(pattern));
            if (hourMatch && hourMatch.length > 0) {
                return hourMatch[0];
            }
            return displayHour.toString().padStart(2, '0');
        }
        catch (error) {
            return displayHour.toString().padStart(2, '0');
        }
    }
    formatMinute(minute) {
        // Try to use formatter for localization on real device, fallback to plain string for previewer
        // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
        try {
            const localeStr = this.locale?.toString();
            // 1. Previewer or invalid locale: simple padStart
            if (!localeStr || localeStr.length === 0) {
                return minute.toString().padStart(2, '0');
            }
            // 2. Chinese/English: simple padStart sufficient (no digit localization needed)
            const lang = localeStr.split('-')[0];
            if (['zh', 'en'].includes(lang)) {
                return minute.toString().padStart(2, '0');
            }
            // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
            if (this.minuteFormatter === null) {
                this.minuteFormatter = new Intl.DateTimeFormat(localeStr, {
                    minute: '2-digit'
                });
            }
            const date = new Date(2026, 0, 1, 0, minute, 0);
            return this.minuteFormatter.format(date);
        }
        catch (error) {
            return minute.toString().padStart(2, '0');
        }
    }
    formatSecond(second) {
        // Try to use formatter for localization on real device, fallback to plain string for previewer
        // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
        try {
            const localeStr = this.locale?.toString();
            // 1. Previewer or invalid locale: simple padStart
            if (!localeStr || localeStr.length === 0) {
                return second.toString().padStart(2, '0');
            }
            // 2. Chinese/English: simple padStart sufficient (no digit localization needed)
            const lang = localeStr.split('-')[0];
            if (['zh', 'en'].includes(lang)) {
                return second.toString().padStart(2, '0');
            }
            // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
            if (this.secondFormatter === null) {
                this.secondFormatter = new Intl.DateTimeFormat(localeStr, {
                    second: '2-digit'
                });
            }
            const date = new Date(2026, 0, 1, 0, 0, second);
            return this.secondFormatter.format(date);
        }
        catch (error) {
            return second.toString().padStart(2, '0');
        }
    }
    formatPeriod(isAM) {
        // Direct translation map for period markers in different languages
        // Only include languages with non-AM/PM period markers
        const periodEntries = [
            ['zh', { am: '上午', pm: '下午' }],
            ['ja', { am: '午前', pm: '午後' }],
            ['ko', { am: '오전', pm: '오후' }],
            ['ar', { am: 'ص', pm: 'م' }],
            ['my', { am: 'နံနက်', pm: 'ညနေ' }],
            ['th', { am: 'น.', pm: 'ทุ่ม' }],
            ['fa', { am: 'ق.ظ', pm: 'ب.ظ' }],
            ['ur', { am: 'صبح', pm: 'شام' }],
            ['hi', { am: 'पूर्वाह्न', pm: 'अपराह्न' }],
            ['bn', { am: 'পূর্বাহ্ণ', pm: 'অপরাহ্ণ' }],
            ['vi', { am: 'SA', pm: 'CH' }],
            ['ms', { am: 'PG', pm: 'PTG' }],
            ['tr', { am: 'ÖÖ', pm: 'AS' }]
        ];
        const periodMap = new Map(periodEntries);
        try {
            const localeStr = this.locale?.toString();
            // Previewer or invalid locale: use Chinese period markers as default
            if (!localeStr || localeStr.length === 0) {
                return isAM ? '上午' : '下午';
            }
            // Check language code prefix (e.g., 'zh-CN' -> 'zh')
            const langCode = localeStr.split('-')[0];
            const langMatch = periodMap.get(langCode);
            if (langMatch) {
                return isAM ? langMatch.am : langMatch.pm;
            }
            // Fallback to AM/PM for all other languages
            return isAM ? 'AM' : 'PM';
        }
        catch (error) {
            return isAM ? 'AM' : 'PM';
        }
    }
    getDaysInMonth(year, month) {
        if (year === DatePickerConstant.HIDE_YEAR_SENTINEL) {
            if (month === 1) {
                return 29;
            }
            const monthsWith31Days = [0, 2, 4, 6, 7, 9, 11];
            const monthsWith30Days = [3, 5, 8, 10];
            if (monthsWith31Days.includes(month)) {
                return 31;
            }
            if (monthsWith30Days.includes(month)) {
                return 30;
            }
            return 30;
        }
        const monthsWith31Days = [0, 2, 4, 6, 7, 9, 11];
        const monthsWith30Days = [3, 5, 8, 10];
        if (month === 1) {
            const isLeapYear = (year % 4 === 0 && year % 100 !== 0) || (year % 400 === 0);
            return isLeapYear ? 29 : 28;
        }
        else if (monthsWith31Days.includes(month)) {
            return 31;
        }
        else if (monthsWith30Days.includes(month)) {
            return 30;
        }
        return 30;
    }
    validateDate(date) {
        try {
            const jsYear = date.getFullYear();
            const month = date.getMonth();
            const day = date.getDate();
            // Check for Invalid Date (NaN values)
            if (isNaN(jsYear) || isNaN(month) || isNaN(day)) {
                return undefined;
            }
            let actualYear = jsYear;
            // Check if year, month, day are in valid ranges
            if (actualYear < DatePickerConstant.MIN_YEAR || actualYear > DatePickerConstant.MAX_YEAR) {
                return undefined;
            }
            if (month < DatePickerConstant.MIN_MONTH || month > DatePickerConstant.MAX_MONTH) {
                return undefined;
            }
            if (day < DatePickerConstant.MIN_DAY || day > 31) {
                return undefined;
            }
            const daysInMonth = this.getDaysInMonth(actualYear, month);
            if (day > daysInMonth) {
                return undefined;
            }
            // Create corrected date using setFullYear to preserve exact year
            const correctedDate = new Date();
            correctedDate.setFullYear(actualYear, month, day);
            // Verify the date matches what was input (no overflow correction)
            const originalTime = date.getTime();
            const correctedTime = correctedDate.getTime();
            // If time difference > 365 days, indicates year jump due to abnormal month/day
            const timeDiff = Math.abs(originalTime - correctedTime);
            const oneYearInMs = 365 * 24 * 60 * 60 * 1000;
            if (timeDiff > oneYearInMs) {
                return undefined;
            }
            return correctedDate;
        }
        catch (e) {
            return undefined;
        }
    }
    validateTime(date) {
        try {
            const hour = date.getHours();
            const minute = date.getMinutes();
            const second = date.getSeconds();
            // Check for Invalid Date (NaN values)
            if (isNaN(hour) || isNaN(minute) || isNaN(second)) {
                return undefined;
            }
            if (hour < DatePickerConstant.MIN_HOUR || hour > DatePickerConstant.MAX_HOUR) {
                return undefined;
            }
            if (minute < DatePickerConstant.MIN_MINUTE || minute > DatePickerConstant.MAX_MINUTE) {
                return undefined;
            }
            if (second < DatePickerConstant.MIN_SECOND || second > DatePickerConstant.MAX_SECOND) {
                return undefined;
            }
            // Detect JavaScript auto-correction by checking time jump
            // Use original year/month/day to construct comparison time
            const year = date.getFullYear();
            const month = date.getMonth();
            const day = date.getDate();
            const originalTime = date.getTime();
            const correctedTime = this.createDateWithTime(year, month, day, hour, minute, second).getTime();
            // If time difference > 1 hour, indicates abnormal minute/second overflow
            const timeDiff = Math.abs(originalTime - correctedTime);
            const oneHourInMs = 60 * 60 * 1000;
            if (timeDiff > oneHourInMs) {
                return undefined;
            }
            // Return Date with fixed base date (2026-01-01) + validated time
            // This ensures timeOptions only considers time part, ignoring date part
            return new Date(2026, 0, 1, hour, minute, second);
        }
        catch (e) {
            return undefined;
        }
    }
    createDateFromParams(year, month, day) {
        const date = new Date();
        date.setFullYear(year, month, day);
        return date;
    }
    createDateWithTime(year, month, day, hour, minute, second) {
        const date = new Date();
        date.setFullYear(year, month, day);
        date.setHours(hour, minute, second);
        return date;
    }
    onOptionsChange() {
        // Save range snapshot before processing
        const oldDisplayMode = this.displayMode;
        const oldDateMode = this.dateMode;
        const oldLunar = this.lunar;
        const oldStartYear = this.startYear;
        const oldEndYear = this.endYear;
        const oldStartMonth = this.startMonth;
        const oldEndMonth = this.endMonth;
        const oldStartDay = this.startDay;
        const oldEndDay = this.endDay;
        const oldTimeFormat = this.timeFormat;
        const oldUseMilitaryTime = this.useMilitaryTime;
        const oldStartHour = this.startHour;
        const oldEndHour = this.endHour;
        const oldStartMinute = this.startMinute;
        const oldEndMinute = this.endMinute;
        const oldStartSecond = this.startSecond;
        const oldEndSecond = this.endSecond;
        // Restore defaults for undefined/null values
        if (this.options.displayMode !== undefined && this.options.displayMode !== null) {
            this.displayMode = this.options.displayMode;
        }
        else {
            this.displayMode = DisplayMode.DATE;
        }
        if (this.options.dateOptions !== undefined && this.options.dateOptions !== null) {
            this.updateDateOptions(this.options.dateOptions);
        }
        else {
            // Restore defaults when dateOptions is undefined
            this.dateMode = DateMode.DATE;
            this.lunar = false;
            this.userLunar = false;
            this.lunarCalendar = null;
            this.dateCanLoop = true;
            this.dateStartEndSet = false;
            this.hideYear = false;
            this.dateHapticFeedback = true;
            this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
            this.endYear = DatePickerConstant.DEFAULT_END_YEAR;
            this.startMonth = DatePickerConstant.MIN_MONTH;
            this.endMonth = DatePickerConstant.MAX_MONTH;
            this.startDay = DatePickerConstant.MIN_DAY;
            this.endDay = 31;
        }
        if (this.options.timeOptions !== undefined && this.options.timeOptions !== null) {
            this.updateTimeOptions(this.options.timeOptions);
        }
        else {
            // Restore defaults when timeOptions is undefined
            this.timeFormat = TimeFormat.HOUR_MINUTE;
            this.useMilitaryTime = false;
            this.timeCanLoop = true;
            this.timeStartEndSet = false;
            this.timeHapticFeedback = true;
            this.startHour = DatePickerConstant.MIN_HOUR;
            this.endHour = DatePickerConstant.MAX_HOUR;
            this.startMinute = DatePickerConstant.MIN_MINUTE;
            this.endMinute = DatePickerConstant.MAX_MINUTE;
            this.startSecond = DatePickerConstant.MIN_SECOND;
            this.endSecond = DatePickerConstant.MAX_SECOND;
            this.selectedPeriod = this.selectedHour < 12 ? 0 : 1;
        }
        const newSelectedTextStyle = this.options.selectedTextStyle;
        const newItemTextStyle = this.options.itemTextStyle;
        const newSelectionIndicator = this.options.selectionIndicator;
        if (!this.textStyleEqual(newSelectedTextStyle, this.selectedTextStyle)) {
            this.selectedTextStyle = newSelectedTextStyle;
        }
        if (!this.textStyleEqual(newItemTextStyle, this.itemTextStyle)) {
            this.itemTextStyle = newItemTextStyle;
        }
        if (!this.indicatorStyleEqual(newSelectionIndicator, this.selectionIndicator)) {
            this.selectionIndicator = newSelectionIndicator;
        }
        // Only rebuild arrays when range/mode actually changed (not just selected)
        const rangeChanged = oldDisplayMode !== this.displayMode ||
            oldDateMode !== this.dateMode ||
            oldLunar !== this.lunar ||
            oldStartYear !== this.startYear ||
            oldEndYear !== this.endYear ||
            oldStartMonth !== this.startMonth ||
            oldEndMonth !== this.endMonth ||
            oldStartDay !== this.startDay ||
            oldEndDay !== this.endDay ||
            oldTimeFormat !== this.timeFormat ||
            oldUseMilitaryTime !== this.useMilitaryTime ||
            oldStartHour !== this.startHour ||
            oldEndHour !== this.endHour ||
            oldStartMinute !== this.startMinute ||
            oldEndMinute !== this.endMinute ||
            oldStartSecond !== this.startSecond ||
            oldEndSecond !== this.endSecond;
        if (rangeChanged) {
            this.initArrays();
        }
    }
    updateDateOptions(dateOptions) {
        // Restore defaults for undefined/null values
        if (dateOptions.mode !== undefined && dateOptions.mode !== null) {
            this.dateMode = dateOptions.mode;
        }
        else {
            this.dateMode = DateMode.DATE;
        }
        if (dateOptions.lunar !== undefined && dateOptions.lunar !== null) {
            // Save user-set value first, then decide based on current language
            this.userLunar = dateOptions.lunar;
            const isChinese = this.isChineseLocale();
            if (isChinese) {
                this.lunar = dateOptions.lunar;
                if (this.lunar) {
                    this.lunarCalendar = i18n.getCalendar(this.locale.toString(), 'chinese');
                }
                else {
                    this.lunarCalendar = null;
                }
            }
            else {
                // Non-Chinese language: force lunar to false, do not apply
                this.lunar = false;
                this.lunarCalendar = null;
            }
        }
        else {
            this.lunar = false;
            this.userLunar = false;
            this.lunarCalendar = null;
        }
        if (dateOptions.loop !== undefined && dateOptions.loop !== null) {
            this.dateCanLoop = dateOptions.loop;
        }
        else {
            this.dateCanLoop = true;
        }
        if (dateOptions.enableHapticFeedback !== undefined && dateOptions.enableHapticFeedback !== null) {
            this.dateHapticFeedback = dateOptions.enableHapticFeedback;
        }
        else {
            this.dateHapticFeedback = true;
        }
        if (dateOptions.hideYear !== undefined && dateOptions.hideYear !== null) {
            this.hideYear = dateOptions.hideYear;
        }
        else {
            this.hideYear = false;
        }
        let startValid = false;
        let endValid = false;
        let startDate = undefined;
        let endDate = undefined;
        if (dateOptions.start !== undefined) {
            startDate = this.validateDate(dateOptions.start);
            if (startDate !== undefined) {
                startValid = true;
            }
        }
        if (dateOptions.end !== undefined) {
            endDate = this.validateDate(dateOptions.end);
            if (endDate !== undefined) {
                endValid = true;
            }
        }
        if (startValid && endValid) {
            if (startDate > endDate) {
                this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
                this.endYear = DatePickerConstant.DEFAULT_END_YEAR;
                this.startMonth = DatePickerConstant.MIN_MONTH;
                this.endMonth = DatePickerConstant.MAX_MONTH;
                this.startDay = DatePickerConstant.MIN_DAY;
                this.endDay = 31;
            }
            else {
                this.startYear = startDate.getFullYear();
                this.startMonth = startDate.getMonth();
                this.startDay = startDate.getDate();
                this.endYear = endDate.getFullYear();
                this.endMonth = endDate.getMonth();
                this.endDay = endDate.getDate();
            }
        }
        else if (startValid && !endValid) {
            this.startYear = startDate.getFullYear();
            this.startMonth = startDate.getMonth();
            this.startDay = startDate.getDate();
            if (this.startYear < DatePickerConstant.MIN_YEAR) {
                this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
                this.startMonth = DatePickerConstant.MIN_MONTH;
                this.startDay = DatePickerConstant.MIN_DAY;
            }
            if (this.startYear > DatePickerConstant.MAX_YEAR) {
                this.startYear = DatePickerConstant.DEFAULT_END_YEAR;
                this.startMonth = DatePickerConstant.MAX_MONTH;
                this.startDay = 31;
            }
            this.endYear = DatePickerConstant.DEFAULT_END_YEAR;
            this.endMonth = DatePickerConstant.MAX_MONTH;
            this.endDay = 31;
        }
        else if (!startValid && endValid) {
            this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
            this.startMonth = DatePickerConstant.MIN_MONTH;
            this.startDay = DatePickerConstant.MIN_DAY;
            this.endYear = endDate.getFullYear();
            this.endMonth = endDate.getMonth();
            this.endDay = endDate.getDate();
            if (this.endYear < DatePickerConstant.MIN_YEAR) {
                this.endYear = DatePickerConstant.DEFAULT_START_YEAR;
                this.endMonth = DatePickerConstant.MIN_MONTH;
                this.endDay = DatePickerConstant.MIN_DAY;
            }
            if (this.endYear > DatePickerConstant.MAX_YEAR) {
                this.endYear = DatePickerConstant.DEFAULT_END_YEAR;
                this.endMonth = DatePickerConstant.MAX_MONTH;
                this.endDay = 31;
            }
        }
        else {
            this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
            this.endYear = DatePickerConstant.DEFAULT_END_YEAR;
            this.startMonth = DatePickerConstant.MIN_MONTH;
            this.endMonth = DatePickerConstant.MAX_MONTH;
            this.startDay = DatePickerConstant.MIN_DAY;
            this.endDay = 31;
        }
        if (this.startYear > this.endYear) {
            const tempYear = this.startYear;
            const tempMonth = this.startMonth;
            const tempDay = this.startDay;
            this.startYear = this.endYear;
            this.startMonth = this.endMonth;
            this.startDay = this.endDay;
            this.endYear = tempYear;
            this.endMonth = tempMonth;
            this.endDay = tempDay;
        }
        this.dateStartEndSet = startValid || endValid;
        if (this.startYear < DatePickerConstant.MIN_YEAR) {
            this.startYear = DatePickerConstant.MIN_YEAR;
            this.startMonth = DatePickerConstant.MIN_MONTH;
            this.startDay = DatePickerConstant.MIN_DAY;
        }
        if (this.endYear > DatePickerConstant.MAX_YEAR) {
            this.endYear = DatePickerConstant.MAX_YEAR;
            this.endMonth = DatePickerConstant.MAX_MONTH;
            this.endDay = 31;
        }
        if (this.startYear > DatePickerConstant.MAX_YEAR && this.endYear > DatePickerConstant.MAX_YEAR) {
            this.startYear = DatePickerConstant.MAX_YEAR;
            this.startMonth = DatePickerConstant.MIN_MONTH;
            this.startDay = DatePickerConstant.MIN_DAY;
            this.endYear = DatePickerConstant.MAX_YEAR;
            this.endMonth = DatePickerConstant.MAX_MONTH;
            this.endDay = 31;
        }
        if (this.startYear < DatePickerConstant.MIN_YEAR && this.endYear < DatePickerConstant.MIN_YEAR) {
            this.startYear = DatePickerConstant.MIN_YEAR;
            this.startMonth = DatePickerConstant.MIN_MONTH;
            this.startDay = DatePickerConstant.MIN_DAY;
            this.endYear = DatePickerConstant.MIN_YEAR;
            this.endMonth = DatePickerConstant.MAX_MONTH;
            this.endDay = 31;
        }
        // Calculate lunar date range when in lunar mode
        if (this.lunar && this.lunarCalendar !== null) {
            const startGregorian = this.createDateFromParams(this.startYear, this.startMonth, this.startDay);
            this.lunarCalendar.setTime(startGregorian);
            this.lunarStartMonth = this.lunarCalendar.get('month');
            this.lunarStartDay = this.lunarCalendar.get('date');
            this.lunarStartYear = this.startYear;
            if (this.lunarStartMonth >= 11 && startGregorian.getMonth() <= 1) { this.lunarStartYear -= 1; }
            const endGregorian = this.createDateFromParams(this.endYear, this.endMonth, this.endDay);
            this.lunarCalendar.setTime(endGregorian);
            this.lunarEndMonth = this.lunarCalendar.get('month');
            this.lunarEndDay = this.lunarCalendar.get('date');
            this.lunarEndYear = this.endYear;
            if (this.lunarEndMonth >= 11 && endGregorian.getMonth() <= 1) { this.lunarEndYear -= 1; }
        }
        else {
            this.lunarStartMonth = DatePickerConstant.MIN_MONTH;
            this.lunarEndMonth = DatePickerConstant.MAX_MONTH;
            this.lunarStartDay = DatePickerConstant.MIN_DAY;
            this.lunarEndDay = 30;
            this.lunarStartYear = this.startYear;
            this.lunarEndYear = this.endYear;
        }
        const now = new Date();
        let selectedDate = undefined;
        let selectedValid = false;
        let selectedIsResult = false;
        // extendSelected takes priority over selected; when both are set, selected does not take effect.
        const effectiveSelected = (dateOptions.extendSelected !== undefined && dateOptions.extendSelected !== null)
            ? dateOptions.extendSelected
            : dateOptions.selected;
        if (effectiveSelected !== undefined && effectiveSelected !== null) {
            if (effectiveSelected instanceof Date) {
                selectedDate = this.validateDate(effectiveSelected);
                if (selectedDate !== undefined) {
                    selectedValid = true;
                }
            }
            else {
                // effectiveSelected is a DatePickerComponentResult: restore a prior selection result.
                const result = effectiveSelected;
                // Each set field takes effect; each omitted field uses the corresponding
                // part of the system date.
                const m = result.month ?? now.getMonth();
                const d = result.day ?? now.getDate();
                if (this.hideYear && result.year === undefined) {
                    // hideYear is enabled and year is omitted: select the hidden year column
                    // ("----") only when (month, day) form a valid sentinel date; otherwise
                    // (e.g. Feb 30) fall back to the system default date.
                    // In lunar mode, validate against lunar days; in gregorian mode, gregorian days.
                    const maxDay = this.lunar
                        ? this.getLunarDaysInMonth(DatePickerConstant.HIDE_YEAR_SENTINEL, m, false)
                        : this.getDaysInMonth(DatePickerConstant.HIDE_YEAR_SENTINEL, m);
                    if (m >= DatePickerConstant.MIN_MONTH && m <= DatePickerConstant.MAX_MONTH &&
                        d >= DatePickerConstant.MIN_DAY && d <= maxDay) {
                        selectedIsResult = true;
                        if (this.selectedYear < this.startYear || this.selectedYear > this.endYear) {
                            this.selectedYear = DatePickerConstant.HIDE_YEAR_SENTINEL;
                        }
                        this.selectedMonth = m;
                        this.selectedDay = d;
                        this.selectedIsLeapMonth = false;
                    }
                }
                else {
                    // Real year: use the provided year, or the system year when omitted
                    // (and hideYear is off). Rebuild a Date and reuse the normal flow
                    // (range clamping + lunar conversion); invalid (e.g. Feb 30) falls back
                    // to the system default date.
                    const y = result.year ?? now.getFullYear();
                    selectedDate = this.validateDate(this.createDateFromParams(y, m, d));
                    if (selectedDate !== undefined) {
                        selectedValid = true;
                    }
                }
            }
        }
        // Create Date objects for start and end
        const startDateObj = this.createDateFromParams(this.startYear, this.startMonth, this.startDay);
        const endDateObj = this.createDateFromParams(this.endYear, this.endMonth, this.endDay);
        // Determine the initial selected date
        let targetDate;
        if (selectedValid) {
            targetDate = selectedDate;
        }
        else {
            targetDate = now;
        }
        if (!selectedIsResult) {
            // Adjust selected date to be within range
            if (targetDate < startDateObj) {
                targetDate = startDateObj;
            }
            else if (targetDate > endDateObj) {
                targetDate = endDateObj;
            }
            // Extract year, month, day from adjusted date
            if (this.lunar && this.lunarCalendar !== null) {
                this.lunarCalendar.setTime(targetDate);
                this.selectedMonth = this.lunarCalendar.get('month');
                this.selectedDay = this.lunarCalendar.get('date');
                // Use Gregorian year (not lunarCalendar.get('year') which returns
                // sexagenary cycle 1-60). Adjust for lunar year boundary:
                // if lunar month is 腊月(11) and Gregorian month is Jan/Feb,
                // the lunar year is the previous Gregorian year.
                this.selectedYear = targetDate.getFullYear();
                if (this.selectedMonth >= 11 && targetDate.getMonth() <= 1) {
                    this.selectedYear -= 1;
                }
                if (this.selectedYear < this.startYear) {
                    this.selectedYear = this.startYear;
                }
                if (this.selectedYear > this.endYear) {
                    this.selectedYear = this.endYear;
                }
                // Detect if the target date falls in a leap month
                this.selectedIsLeapMonth = this.isDateInLeapMonth(targetDate, this.selectedYear, this.selectedMonth);
                // Ensure selected date is within lunar range based on gregorian year
                if (this.selectedYear === this.lunarStartYear) {
                    if (this.selectedMonth < this.lunarStartMonth) {
                        this.selectedMonth = this.lunarStartMonth;
                        this.selectedIsLeapMonth = false;
                        this.selectedDay = this.lunarStartDay;
                    }
                    else if (this.selectedMonth === this.lunarStartMonth && this.selectedDay < this.lunarStartDay &&
                        !this.selectedIsLeapMonth) {
                        this.selectedDay = this.lunarStartDay;
                    }
                }
                if (this.selectedYear === this.lunarEndYear) {
                    if (this.selectedMonth > this.lunarEndMonth) {
                        this.selectedMonth = this.lunarEndMonth;
                        this.selectedIsLeapMonth = false;
                        this.selectedDay = this.lunarEndDay;
                    }
                    else if (this.selectedMonth === this.lunarEndMonth && this.selectedDay > this.lunarEndDay &&
                        !this.selectedIsLeapMonth) {
                        this.selectedDay = this.lunarEndDay;
                    }
                }
            }
            else {
                this.selectedYear = targetDate.getFullYear();
                this.selectedMonth = targetDate.getMonth();
                this.selectedDay = targetDate.getDate();
                // Ensure day doesn't exceed days in month (gregorian)
                const daysInMonth = this.getDaysInMonth(this.selectedYear, this.selectedMonth);
                if (this.selectedDay > daysInMonth) {
                    this.selectedDay = daysInMonth;
                }
            }
        }
        if (dateOptions.onChange !== undefined) {
            this.dateOnChange = dateOptions.onChange;
        }
        if (dateOptions.onScrollStop !== undefined) {
            this.dateOnScrollStop = dateOptions.onScrollStop;
        }
    }
    updateTimeOptions(timeOptions) {
        // Restore defaults for undefined/null values
        if (timeOptions.format !== undefined && timeOptions.format !== null) {
            this.timeFormat = timeOptions.format;
        }
        else {
            this.timeFormat = TimeFormat.HOUR_MINUTE;
        }
        if (timeOptions.useMilitaryTime !== undefined && timeOptions.useMilitaryTime !== null) {
            this.useMilitaryTime = timeOptions.useMilitaryTime;
        }
        else {
            this.useMilitaryTime = false;
        }
        if (timeOptions.loop !== undefined && timeOptions.loop !== null) {
            this.timeCanLoop = timeOptions.loop;
        }
        else {
            this.timeCanLoop = true;
        }
        if (timeOptions.enableHapticFeedback !== undefined && timeOptions.enableHapticFeedback !== null) {
            this.timeHapticFeedback = timeOptions.enableHapticFeedback;
        }
        else {
            this.timeHapticFeedback = true;
        }
        let startValid = false;
        let endValid = false;
        let startTime = undefined;
        let endTime = undefined;
        if (timeOptions.start !== undefined) {
            startTime = this.validateTime(timeOptions.start);
            if (startTime !== undefined) {
                startValid = true;
            }
        }
        if (timeOptions.end !== undefined) {
            endTime = this.validateTime(timeOptions.end);
            if (endTime !== undefined) {
                endValid = true;
            }
        }
        if (startValid && endValid) {
            if (startTime.getHours() > endTime.getHours() ||
                (startTime.getHours() === endTime.getHours() && startTime.getMinutes() > endTime.getMinutes()) ||
                (startTime.getHours() === endTime.getHours() && startTime.getMinutes() === endTime.getMinutes() && startTime.getSeconds() > endTime.getSeconds())) {
                this.startHour = DatePickerConstant.MIN_HOUR;
                this.endHour = DatePickerConstant.MAX_HOUR;
                this.startMinute = DatePickerConstant.MIN_MINUTE;
                this.endMinute = DatePickerConstant.MAX_MINUTE;
                this.startSecond = DatePickerConstant.MIN_SECOND;
                this.endSecond = DatePickerConstant.MAX_SECOND;
            }
            else {
                this.startHour = startTime.getHours();
                this.startMinute = startTime.getMinutes();
                this.startSecond = startTime.getSeconds();
                this.endHour = endTime.getHours();
                this.endMinute = endTime.getMinutes();
                this.endSecond = endTime.getSeconds();
            }
        }
        else if (startValid && !endValid) {
            this.startHour = startTime.getHours();
            this.startMinute = startTime.getMinutes();
            this.startSecond = startTime.getSeconds();
            this.endHour = DatePickerConstant.MAX_HOUR;
            this.endMinute = DatePickerConstant.MAX_MINUTE;
            this.endSecond = DatePickerConstant.MAX_SECOND;
        }
        else if (!startValid && endValid) {
            this.startHour = DatePickerConstant.MIN_HOUR;
            this.startMinute = DatePickerConstant.MIN_MINUTE;
            this.startSecond = DatePickerConstant.MIN_SECOND;
            this.endHour = endTime.getHours();
            this.endMinute = endTime.getMinutes();
            this.endSecond = endTime.getSeconds();
        }
        else {
            this.startHour = DatePickerConstant.MIN_HOUR;
            this.endHour = DatePickerConstant.MAX_HOUR;
            this.startMinute = DatePickerConstant.MIN_MINUTE;
            this.endMinute = DatePickerConstant.MAX_MINUTE;
            this.startSecond = DatePickerConstant.MIN_SECOND;
            this.endSecond = DatePickerConstant.MAX_SECOND;
        }
        this.startHour = Math.max(DatePickerConstant.MIN_HOUR, Math.min(DatePickerConstant.MAX_HOUR, this.startHour));
        this.endHour = Math.max(DatePickerConstant.MIN_HOUR, Math.min(DatePickerConstant.MAX_HOUR, this.endHour));
        this.startMinute = Math.max(DatePickerConstant.MIN_MINUTE, Math.min(DatePickerConstant.MAX_MINUTE, this.startMinute));
        this.endMinute = Math.max(DatePickerConstant.MIN_MINUTE, Math.min(DatePickerConstant.MAX_MINUTE, this.endMinute));
        this.startSecond = Math.max(DatePickerConstant.MIN_SECOND, Math.min(DatePickerConstant.MAX_SECOND, this.startSecond));
        this.endSecond = Math.max(DatePickerConstant.MIN_SECOND, Math.min(DatePickerConstant.MAX_SECOND, this.endSecond));
        this.timeStartEndSet = startValid || endValid;
        const now = new Date();
        let selectedTime = undefined;
        let selectedValid = false;
        // extendSelected takes priority over selected; when both are set, selected does not take effect.
        const effectiveSelected = (timeOptions.extendSelected !== undefined && timeOptions.extendSelected !== null)
            ? timeOptions.extendSelected
            : timeOptions.selected;
        if (effectiveSelected !== undefined && effectiveSelected !== null) {
            if (effectiveSelected instanceof Date) {
                selectedTime = this.validateTime(effectiveSelected);
                if (selectedTime !== undefined) {
                    selectedValid = true;
                }
            }
            else {
                // effectiveSelected is a DatePickerComponentResult: restore a prior selection result.
                const result = effectiveSelected;
                // Each set field takes effect; each omitted field uses the corresponding
                // part of the system time.
                const h = result.hour ?? now.getHours();
                const min = result.minute ?? now.getMinutes();
                const sec = result.second ?? now.getSeconds();
                selectedTime = this.validateTime(new Date(2026, 0, 1, h, min, sec));
                if (selectedTime !== undefined) {
                    selectedValid = true;
                }
            }
        }
        // Determine the initial selected time
        let targetTime;
        if (selectedValid) {
            targetTime = selectedTime;
        }
        else {
            targetTime = now;
        }
        // Create Date objects for start and end time on same day
        const startTimeObj = new Date(2026, 0, 1, this.startHour, this.startMinute, this.startSecond);
        const endTimeObj = new Date(2026, 0, 1, this.endHour, this.endMinute, this.endSecond);
        const targetTimeObj = new Date(2026, 0, 1, targetTime.getHours(), targetTime.getMinutes(), targetTime.getSeconds());
        // Adjust selected time to be within range
        if (targetTimeObj < startTimeObj) {
            targetTimeObj.setTime(startTimeObj.getTime());
        }
        else if (targetTimeObj > endTimeObj) {
            targetTimeObj.setTime(endTimeObj.getTime());
        }
        // Extract hour, minute, second from adjusted time
        this.selectedHour = targetTimeObj.getHours();
        this.selectedMinute = targetTimeObj.getMinutes();
        this.selectedSecond = targetTimeObj.getSeconds();
        // Set selectedPeriod based on selectedHour for 12-hour format
        if (!this.useMilitaryTime) {
            this.selectedPeriod = this.selectedHour < 12 ? 0 : 1;
        }
        if (timeOptions.onChange !== undefined) {
            this.timeOnChange = timeOptions.onChange;
        }
        if (timeOptions.onScrollStop !== undefined) {
            this.timeOnScrollStop = timeOptions.onScrollStop;
        }
    }
    getResult() {
        const result = new DatePickerComponentResult();
        if (this.selectedYear === DatePickerConstant.HIDE_YEAR_SENTINEL) {
            result.year = undefined;
            result.month = this.selectedMonth;
            result.day = this.selectedDay;
        }
        else if (this.lunar && this.lunarCalendar !== null) {
            const gregorianDate = this.convertLunarToGregorian(this.selectedYear, this.selectedMonth, this.selectedDay, this.selectedIsLeapMonth);
            if (gregorianDate !== null) {
                result.year = gregorianDate.getFullYear();
                result.month = gregorianDate.getMonth();
                result.day = gregorianDate.getDate();
            }
            else {
                result.year = undefined;
                result.month = this.selectedMonth;
                result.day = this.selectedDay;
            }
        }
        else {
            result.year = this.selectedYear;
            result.month = this.selectedMonth;
            result.day = this.selectedDay;
        }
        result.hour = this.selectedHour;
        result.minute = this.selectedMinute;
        result.second = this.selectedSecond;
        return result;
    }
    onYearChange(selectedIndex) {
        const newYear = this.hideYear && selectedIndex === this.yearArray.length - 1
            ? DatePickerConstant.HIDE_YEAR_SENTINEL
            : this.startYear + selectedIndex;
        // Guard: if the year hasn't actually changed, do nothing.
        // This prevents reverse linkage when ArkUI re-fires onChange during re-renders.
        if (newYear === this.selectedYear) {
            return;
        }
        this.selectedYear = newYear;
        if (this.lunar) {
            // Calculate valid month range first
            let startLunarMonthIndex = DatePickerConstant.MIN_MONTH;
            let endLunarMonthIndex = DatePickerConstant.MAX_MONTH;
            if (this.selectedYear === this.lunarStartYear) {
                startLunarMonthIndex = this.lunarStartMonth;
            }
            if (this.selectedYear === this.lunarEndYear) {
                endLunarMonthIndex = this.lunarEndMonth;
            }
            // Adjust selectedMonth BEFORE changing monthArray
            if (this.selectedMonth < startLunarMonthIndex) {
                this.selectedMonth = startLunarMonthIndex;
                this.selectedIsLeapMonth = false;
            }
            if (this.selectedMonth > endLunarMonthIndex) {
                this.selectedMonth = endLunarMonthIndex;
                this.selectedIsLeapMonth = false;
            }
            // The new year may not have a leap month for the currently selected month.
            // Reset leap flag if the selected month doesn't match the new year's leap month.
            const newLeapMonth = this.getLunarLeapMonth(this.selectedYear);
            if (this.selectedIsLeapMonth && (newLeapMonth < 0 || this.selectedMonth !== newLeapMonth)) {
                this.selectedIsLeapMonth = false;
            }
            // Build month array with leap months
            this.monthArray = [];
            for (let i = startLunarMonthIndex; i <= endLunarMonthIndex; i++) {
                this.monthArray.push(this.formatLunarMonth(i, false));
                if (newLeapMonth >= 0 && i === newLeapMonth) {
                    this.monthArray.push(this.formatLunarMonth(i, true));
                }
            }
            // Adjust selectedDay BEFORE generating day array
            const lunarDays = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
            let startLunarDayIndex = DatePickerConstant.MIN_DAY;
            let endLunarDayIndex = lunarDays;
            if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
                !this.selectedIsLeapMonth) {
                startLunarDayIndex = this.lunarStartDay;
            }
            if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
                !this.selectedIsLeapMonth) {
                endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays);
            }
            if (this.selectedDay < startLunarDayIndex) {
                this.selectedDay = startLunarDayIndex;
            }
            if (this.selectedDay > endLunarDayIndex) {
                this.selectedDay = endLunarDayIndex;
            }
            // Generate day array
            this.dayArray = [];
            for (let i = startLunarDayIndex; i <= endLunarDayIndex; i++) {
                this.dayArray.push(this.formatLunarDay(i));
            }
        }
        else {
            // Calculate valid month range first
            let startMonthIndex = DatePickerConstant.MIN_MONTH;
            let endMonthIndex = DatePickerConstant.MAX_MONTH;
            if (this.selectedYear === this.startYear) {
                startMonthIndex = this.startMonth;
            }
            if (this.selectedYear === this.endYear) {
                endMonthIndex = this.endMonth;
            }
            // Adjust selectedMonth first
            if (this.selectedMonth < startMonthIndex) {
                this.selectedMonth = startMonthIndex;
            }
            if (this.selectedMonth > endMonthIndex) {
                this.selectedMonth = endMonthIndex;
            }
            // Then update month and day arrays
            this.initMonthArray();
            this.updateDaysArray();
        }
        this.updateDateColumnLoopState();
        if (this.lunar) {
            setTimeout(() => { this.dateOnChange?.(this.getResult()); }, 0);
        }
        else {
            this.dateOnChange?.(this.getResult());
        }
    }
    onMonthChange(selectedIndex) {
        if (this.lunar) {
            let startLunarMonthIndex = DatePickerConstant.MIN_MONTH;
            if (this.selectedYear === this.lunarStartYear) {
                startLunarMonthIndex = this.lunarStartMonth;
            }
            // Decode selectedIndex into (month, isLeap) accounting for leap month insertion
            const leapMonth = this.getLunarLeapMonth(this.selectedYear);
            let newMonth;
            let newIsLeap = false;
            if (leapMonth >= 0 && leapMonth >= startLunarMonthIndex) {
                const leapArrayIndex = leapMonth - startLunarMonthIndex; // index of regular leap month
                if (selectedIndex === leapArrayIndex + 1) {
                    // The entry right after the regular leap month is the leap month
                    newMonth = leapMonth;
                    newIsLeap = true;
                }
                else if (selectedIndex <= leapArrayIndex) {
                    newMonth = startLunarMonthIndex + selectedIndex;
                }
                else {
                    // After the leap month entry, subtract 1 from the index
                    newMonth = startLunarMonthIndex + selectedIndex - 1;
                }
            }
            else {
                newMonth = startLunarMonthIndex + selectedIndex;
            }
            // Guard: if the month hasn't actually changed, do nothing.
            // This prevents reverse linkage when ArkUI re-fires onChange during re-renders.
            if (newMonth === this.selectedMonth && newIsLeap === this.selectedIsLeapMonth) {
                return;
            }
            this.selectedMonth = newMonth;
            this.selectedIsLeapMonth = newIsLeap;
            // Adjust selectedDay BEFORE generating day array
            const lunarDays = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
            let startLunarDayIndex = DatePickerConstant.MIN_DAY;
            let endLunarDayIndex = lunarDays;
            if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
                !this.selectedIsLeapMonth) {
                startLunarDayIndex = this.lunarStartDay;
            }
            if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
                !this.selectedIsLeapMonth) {
                endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays);
            }
            if (this.selectedDay < startLunarDayIndex) {
                this.selectedDay = startLunarDayIndex;
            }
            if (this.selectedDay > endLunarDayIndex) {
                this.selectedDay = endLunarDayIndex;
            }
            // Generate day array without adjusting selectedDay
            this.dayArray = [];
            for (let i = startLunarDayIndex; i <= endLunarDayIndex; i++) {
                this.dayArray.push(this.formatLunarDay(i));
            }
        }
        else {
            let startMonthIndex = DatePickerConstant.MIN_MONTH;
            if (this.selectedYear === this.startYear) {
                startMonthIndex = this.startMonth;
            }
            this.selectedMonth = startMonthIndex + selectedIndex;
            this.updateDaysArray();
        }
        this.updateDateColumnLoopState();
        if (this.lunar) {
            setTimeout(() => { this.dateOnChange?.(this.getResult()); }, 0);
        }
        else {
            this.dateOnChange?.(this.getResult());
        }
    }
    onDayChange(selectedIndex) {
        if (this.lunar) {
            let startLunarDayIndex = DatePickerConstant.MIN_DAY;
            if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
                !this.selectedIsLeapMonth) {
                startLunarDayIndex = this.lunarStartDay;
            }
            const newDay = startLunarDayIndex + selectedIndex;
            // Guard: if the day hasn't actually changed, do nothing.
            if (newDay === this.selectedDay) {
                return;
            }
            this.selectedDay = newDay;
            // Ensure selectedDay doesn't exceed lunar days in month
            const lunarDays = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
            if (this.selectedDay > lunarDays) {
                this.selectedDay = lunarDays;
            }
        }
        else {
            let startDayIndex = DatePickerConstant.MIN_DAY;
            if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
                startDayIndex = this.startDay;
            }
            this.selectedDay = startDayIndex + selectedIndex;
        }
        if (this.lunar) {
            setTimeout(() => { this.dateOnChange?.(this.getResult()); }, 0);
        }
        else {
            this.dateOnChange?.(this.getResult());
        }
    }
    onHourChange(selectedIndex) {
        if (this.useMilitaryTime) {
            this.selectedHour = this.startHour + selectedIndex;
        }
        else {
            const oldDisplayHour = this.selectedHour === 0 || this.selectedHour === 12 ? 12 :
                (this.selectedHour > 12 ? this.selectedHour - 12 : this.selectedHour);
            // Calculate the actual display hour from selectedIndex using the SAME logic as getHourSelectedIndex()
            // This ensures bidirectional consistency
            const isAM = this.selectedPeriod === 0;
            let newDisplayHour = 12; // default fallback
            // Find displayHour by counting valid hours (same as getHourSelectedIndex logic)
            let validIndex = 0;
            if (isAM) {
                // AM period: check 12 AM (hour24=0) and 1-11 AM (hour24=1-11)
                if (0 >= this.startHour && 0 <= this.endHour) {
                    if (validIndex === selectedIndex) {
                        newDisplayHour = 12;
                    }
                    validIndex++;
                }
                for (let h = 1; h <= 11; h++) {
                    if (h >= this.startHour && h <= this.endHour) {
                        if (validIndex === selectedIndex) {
                            newDisplayHour = h;
                        }
                        validIndex++;
                    }
                }
            }
            else {
                // PM period: check 12 PM (hour24=12) and 1-11 PM (hour24=13-23)
                if (12 >= this.startHour && 12 <= this.endHour) {
                    if (validIndex === selectedIndex) {
                        newDisplayHour = 12;
                    }
                    validIndex++;
                }
                for (let h = 13; h <= 23; h++) {
                    if (h >= this.startHour && h <= this.endHour) {
                        if (validIndex === selectedIndex) {
                            newDisplayHour = h === 12 ? 12 : h - 12;
                        }
                        validIndex++;
                    }
                }
            }
            // Crossing 11↔12 always triggers period toggle
            const crossingBoundary = (oldDisplayHour === 11 && newDisplayHour === 12) ||
                (oldDisplayHour === 12 && newDisplayHour === 11);
            if (crossingBoundary) {
                // Toggle period
                this.selectedPeriod = this.selectedPeriod === 0 ? 1 : 0;
            }
            // Convert display hour to 24-hour format based on CURRENT period (after toggle)
            if (newDisplayHour === 12) {
                if (this.selectedPeriod === 0) {
                    this.selectedHour = 0; // 12 AM = 0:00 (24-hour)
                }
                else {
                    this.selectedHour = 12; // 12 PM = 12:00 (24-hour)
                }
            }
            else {
                if (this.selectedPeriod === 0) {
                    this.selectedHour = newDisplayHour; // 1-11 AM
                }
                else {
                    this.selectedHour = newDisplayHour + 12; // 1-11 PM = 13-23
                }
            }
        }
        this.updateTimeArrays();
        this.updateTimeColumnLoopState();
        this.timeOnChange?.(this.getResult());
    }
    onPeriodChange(selectedIndex) {
        // Determine selectedPeriod based on periodArray content, not directly using selectedIndex
        // This ensures correct mapping even if periodArray has only one option
        const selectedPeriodStr = this.periodArray[selectedIndex];
        this.selectedPeriod = selectedPeriodStr === this.formatPeriod(true) ? 0 : 1;
        if (!this.useMilitaryTime) {
            // Update hourArray
            this.updateHourArrayFor12Hour();
            // Adjust selectedHour based on new period
            const isAM = this.selectedPeriod === 0;
            if (isAM) {
                // Switch to AM period
                const minHour = Math.max(0, this.startHour);
                const maxHour = Math.min(11, this.endHour < 12 ? this.endHour : 11);
                // If current hour is not in AM range, adjust to within range
                if (this.selectedHour >= 12 || this.selectedHour < minHour) {
                    this.selectedHour = minHour;
                }
                if (this.selectedHour > maxHour) {
                    this.selectedHour = maxHour;
                }
            }
            else {
                // Switch to PM period
                const minHour = Math.max(12, this.startHour >= 12 ? this.startHour : 12);
                const maxHour = Math.min(23, this.endHour);
                // If current hour is not in PM range, adjust to within range
                if (this.selectedHour < 12 || this.selectedHour < minHour) {
                    this.selectedHour = minHour;
                }
                if (this.selectedHour > maxHour) {
                    this.selectedHour = maxHour;
                }
            }
            // Update minute and second arrays
            this.updateTimeArrays();
        }
        this.updateTimeColumnLoopState();
        this.timeOnChange?.(this.getResult());
    }
    onMinuteChange(selectedIndex) {
        let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
        if (this.selectedHour === this.startHour) {
            startMinuteIndex = this.startMinute;
        }
        this.selectedMinute = startMinuteIndex + selectedIndex;
        this.updateSecondArray();
        this.updateTimeColumnLoopState();
        this.timeOnChange?.(this.getResult());
    }
    onSecondChange(selectedIndex) {
        let startSecondIndex = DatePickerConstant.MIN_SECOND;
        if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
            startSecondIndex = this.startSecond;
        }
        this.selectedSecond = startSecondIndex + selectedIndex;
        this.timeOnChange?.(this.getResult());
    }
    updateTimeArrays() {
        // Optimize: only update arrays when range actually changes
        const oldMinuteStart = this.minuteArray.length > 0 ? 0 : -1;
        this.minuteArray = [];
        let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
        let endMinuteIndex = DatePickerConstant.MAX_MINUTE;
        if (this.selectedHour === this.startHour) {
            startMinuteIndex = this.startMinute;
        }
        if (this.selectedHour === this.endHour) {
            endMinuteIndex = this.endMinute;
        }
        // Use pre-allocated array with direct assignment for better performance
        const minuteCount = endMinuteIndex - startMinuteIndex + 1;
        for (let i = 0; i < minuteCount; i++) {
            this.minuteArray.push(this.formatMinute(startMinuteIndex + i));
        }
        if (this.selectedMinute < startMinuteIndex) {
            this.selectedMinute = startMinuteIndex;
        }
        if (this.selectedMinute > endMinuteIndex) {
            this.selectedMinute = endMinuteIndex;
        }
        this.updateSecondArray();
    }
    updateSecondArray() {
        this.secondArray = [];
        let startSecondIndex = DatePickerConstant.MIN_SECOND;
        let endSecondIndex = DatePickerConstant.MAX_SECOND;
        if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
            startSecondIndex = this.startSecond;
        }
        if (this.selectedHour === this.endHour && this.selectedMinute === this.endMinute) {
            endSecondIndex = this.endSecond;
        }
        // Use pre-allocated array with direct assignment for better performance
        const secondCount = endSecondIndex - startSecondIndex + 1;
        for (let i = 0; i < secondCount; i++) {
            this.secondArray.push(this.formatSecond(startSecondIndex + i));
        }
        if (this.selectedSecond < startSecondIndex) {
            this.selectedSecond = startSecondIndex;
        }
        if (this.selectedSecond > endSecondIndex) {
            this.selectedSecond = endSecondIndex;
        }
    }
    updateDaysArray() {
        if (this.lunar) {
            const lunarDays = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
            let startLunarDayIndex = DatePickerConstant.MIN_DAY;
            let endLunarDayIndex = lunarDays;
            if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
                !this.selectedIsLeapMonth) {
                startLunarDayIndex = this.lunarStartDay;
            }
            if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
                !this.selectedIsLeapMonth) {
                endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays);
            }
            if (this.selectedDay < startLunarDayIndex) {
                this.selectedDay = startLunarDayIndex;
            }
            if (this.selectedDay > endLunarDayIndex) {
                this.selectedDay = endLunarDayIndex;
            }
            this.initLunarDayArray();
        }
        else {
            const daysInMonth = this.getDaysInMonth(this.selectedYear, this.selectedMonth);
            // Determine the valid day range based on selected year and month
            let startDayIndex = DatePickerConstant.MIN_DAY;
            let endDayIndex = daysInMonth;
            if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
                startDayIndex = this.startDay;
            }
            if (this.selectedYear === this.endYear && this.selectedMonth === this.endMonth) {
                endDayIndex = Math.min(this.endDay, daysInMonth);
            }
            // Adjust selectedDay to be within valid range
            if (this.selectedDay < startDayIndex) {
                this.selectedDay = startDayIndex;
            }
            if (this.selectedDay > endDayIndex) {
                this.selectedDay = endDayIndex;
            }
            this.initDayArray();
        }
    }
    getYearSelectedIndex() {
        if (this.selectedYear === DatePickerConstant.HIDE_YEAR_SENTINEL) {
            return this.yearArray.length - 1;
        }
        return this.selectedYear - this.startYear;
    }
    getMonthSelectedIndex() {
        if (this.lunar) {
            let startLunarMonthIndex = DatePickerConstant.MIN_MONTH;
            if (this.selectedYear === this.lunarStartYear) {
                startLunarMonthIndex = this.lunarStartMonth;
            }
            // Encode (month, isLeap) to array index, accounting for leap month insertion
            const leapMonth = this.getLunarLeapMonth(this.selectedYear);
            let index = this.selectedMonth - startLunarMonthIndex;
            if (leapMonth >= 0 && leapMonth >= startLunarMonthIndex) {
                if (this.selectedIsLeapMonth && this.selectedMonth === leapMonth) {
                    // Leap month is right after the regular month
                    index = (leapMonth - startLunarMonthIndex) + 1;
                }
                else if (this.selectedMonth > leapMonth) {
                    // After the leap month insertion, add 1 to skip the extra entry
                    index += 1;
                }
            }
            return index;
        }
        else {
            let startMonthIndex = DatePickerConstant.MIN_MONTH;
            if (this.selectedYear === this.startYear) {
                startMonthIndex = this.startMonth;
            }
            return this.selectedMonth - startMonthIndex;
        }
    }
    getDaySelectedIndex() {
        if (this.lunar) {
            let startLunarDayIndex = DatePickerConstant.MIN_DAY;
            if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
                !this.selectedIsLeapMonth) {
                startLunarDayIndex = this.lunarStartDay;
            }
            return this.selectedDay - startLunarDayIndex;
        }
        else {
            let startDayIndex = DatePickerConstant.MIN_DAY;
            if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
                startDayIndex = this.startDay;
            }
            return this.selectedDay - startDayIndex;
        }
    }
    getHourSelectedIndex() {
        if (this.useMilitaryTime) {
            return this.selectedHour - this.startHour;
        }
        else {
            // For 12-hour format, calculate index directly based on hourArray generation logic
            // hourArray structure:
            // - AM period: [12, 1, 2, ..., 11] (if in range)
            // - PM period: [12, 1, 2, ..., 11] (if in range)
            const isAM = this.selectedPeriod === 0;
            const displayHour = this.selectedHour === 0 || this.selectedHour === 12 ? 12 :
                (this.selectedHour > 12 ? this.selectedHour - 12 : this.selectedHour);
            // Calculate index by matching display hour to hourArray structure
            // hourArray[0] = 12, hourArray[1] = 1, hourArray[2] = 2, ..., hourArray[11] = 11
            let targetIndex = 0;
            if (displayHour === 12) {
                targetIndex = 0;
            }
            else {
                targetIndex = displayHour;
            }
            // Ensure the targetIndex is within valid hourArray bounds
            // Need to verify this index exists in hourArray based on actual time range
            const actualHour24 = isAM ? (displayHour === 12 ? 0 : displayHour) :
                (displayHour === 12 ? 12 : displayHour + 12);
            // Check if this hour is actually in hourArray
            if (actualHour24 >= this.startHour && actualHour24 <= this.endHour) {
                // Find the actual index in hourArray by counting valid hours
                let validIndex = 0;
                if (isAM) {
                    // AM period: check 12 AM (hour24=0) and 1-11 AM (hour24=1-11)
                    if (0 >= this.startHour && 0 <= this.endHour) {
                        if (displayHour === 12) {
                            return validIndex;
                        }
                        validIndex++;
                    }
                    for (let h = 1; h <= 11; h++) {
                        if (h >= this.startHour && h <= this.endHour) {
                            if (h === displayHour) {
                                return validIndex;
                            }
                            validIndex++;
                        }
                    }
                }
                else {
                    // PM period: check 12 PM (hour24=12) and 1-11 PM (hour24=13-23)
                    if (12 >= this.startHour && 12 <= this.endHour) {
                        if (displayHour === 12) {
                            return validIndex;
                        }
                        validIndex++;
                    }
                    for (let h = 13; h <= 23; h++) {
                        if (h >= this.startHour && h <= this.endHour) {
                            const hDisplay = h === 12 ? 12 : h - 12;
                            if (hDisplay === displayHour) {
                                return validIndex;
                            }
                            validIndex++;
                        }
                    }
                }
            }
            // Fallback: return first element index
            return 0;
        }
    }
    getMinuteSelectedIndex() {
        let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
        if (this.selectedHour === this.startHour) {
            startMinuteIndex = this.startMinute;
        }
        return this.selectedMinute - startMinuteIndex;
    }
    getSecondSelectedIndex() {
        let startSecondIndex = DatePickerConstant.MIN_SECOND;
        if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
            startSecondIndex = this.startSecond;
        }
        return this.selectedSecond - startSecondIndex;
    }
    onHideYearChange() {
        if (this.lunar) {
            this.initLunarArrays();
        }
        else {
            this.initYearArray();
            this.initMonthArray();
            this.updateDaysArray();
        }
        this.updateDateColumnLoopState();
    }
    updateDateColumnLoopState() {
        if (!this.dateStartEndSet || !this.dateCanLoop) {
            const loop = this.dateCanLoop;
            this.yearColumnCanLoop = loop;
            this.monthColumnCanLoop = loop;
            this.dayColumnCanLoop = loop;
            return;
        }
        this.yearColumnCanLoop = false;
        const atStartYear = this.lunar ?
            (this.selectedYear === this.lunarStartYear) : (this.selectedYear === this.startYear);
        const atEndYear = this.lunar ?
            (this.selectedYear === this.lunarEndYear) : (this.selectedYear === this.endYear);
        this.monthColumnCanLoop = !atStartYear && !atEndYear;
        if (this.lunar) {
            const atStartMonth = atStartYear && this.selectedMonth === this.lunarStartMonth && !this.selectedIsLeapMonth;
            const atEndMonth = atEndYear && this.selectedMonth === this.lunarEndMonth && !this.selectedIsLeapMonth;
            this.dayColumnCanLoop = !atStartMonth && !atEndMonth;
        }
        else {
            const atStartMonth = atStartYear && this.selectedMonth === this.startMonth;
            const atEndMonth = atEndYear && this.selectedMonth === this.endMonth;
            this.dayColumnCanLoop = !atStartMonth && !atEndMonth;
        }
    }
    updateTimeColumnLoopState() {
        if (!this.timeStartEndSet || !this.timeCanLoop) {
            const loop = this.timeCanLoop;
            this.hourColumnCanLoop = loop;
            this.minuteColumnCanLoop = loop;
            this.secondColumnCanLoop = loop;
            return;
        }
        this.hourColumnCanLoop = false;
        const atStartHour = this.selectedHour === this.startHour;
        const atEndHour = this.selectedHour === this.endHour;
        this.minuteColumnCanLoop = !atStartHour && !atEndHour;
        const atStartMinute = atStartHour && this.selectedMinute === this.startMinute;
        const atEndMinute = atEndHour && this.selectedMinute === this.endMinute;
        this.secondColumnCanLoop = !atStartMinute && !atEndMinute;
    }
    isRTL() {
        return i18n.isRTL(this.locale.toString()) ?? false;
    }
    getDateColumnCount() {
        if (this.dateMode === DateMode.DATE) {
            return 3;
        }
        else {
            return 2;
        }
    }
    getTimeColumnCount() {
        if (this.timeFormat === TimeFormat.HOUR_MINUTE) {
            return this.useMilitaryTime ? 2 : 3;
        }
        else {
            return this.useMilitaryTime ? 3 : 4;
        }
    }
    getColumnCount() {
        let columnCount = 0;
        if (this.displayMode === DisplayMode.DATE) {
            columnCount = this.getDateColumnCount();
        }
        else if (this.displayMode === DisplayMode.TIME) {
            columnCount = this.getTimeColumnCount();
        }
        else {
            columnCount = this.getDateColumnCount() + this.getTimeColumnCount();
        }
        return columnCount;
    }
    getFontSize() {
        const columnCount = this.getColumnCount();
        return columnCount >= 6 ? '14vp' : '16vp';
    }
    getDatePartRatio() {
        const timeColumns = this.getTimeColumnCount();
        const dateColumns = this.getDateColumnCount();
        if (timeColumns === 4) {
            if (dateColumns === 3) {
                return 1;
            }
            else {
                return 2;
            }
        }
        else if (timeColumns === 2) {
            if (dateColumns === 3) {
                return 3;
            }
            else {
                return 1;
            }
        }
        else {
            return 1;
        }
    }
    getTimePartRatio() {
        const timeColumns = this.getTimeColumnCount();
        const dateColumns = this.getDateColumnCount();
        if (timeColumns === 4) {
            if (dateColumns === 3) {
                return 1;
            }
            else {
                return 3;
            }
        }
        else if (timeColumns === 2) {
            if (dateColumns === 3) {
                return 2;
            }
            else {
                return 1;
            }
        }
        else {
            return 1;
        }
    }
    getDateColumnWidth(columnIndex) {
        const inDateTimeMode = this.displayMode === DisplayMode.DATE_TIME;
        let parts = [];
        if (this.dateMode === DateMode.DATE) {
            parts = [3, 2, 2];
        }
        else if (this.dateMode === DateMode.YEAR_AND_MONTH) {
            parts = [3, 2];
        }
        else {
            parts = [1, 1];
        }
        const totalParts = parts.reduce((a, b) => a + b, 0);
        const columnPart = parts[columnIndex];
        if (inDateTimeMode) {
            const dateRatio = this.getDatePartRatio();
            const timeRatio = this.getTimePartRatio();
            const totalRatio = dateRatio + timeRatio;
            const percent = (dateRatio / totalRatio) * (columnPart / totalParts) * 100;
            return `${percent.toFixed(2)}%`;
        }
        else {
            const percent = (columnPart / totalParts) * 100;
            return `${percent.toFixed(2)}%`;
        }
    }
    getTimeColumnWidth(columnIndex) {
        const inDateTimeMode = this.displayMode === DisplayMode.DATE_TIME;
        const totalColumns = this.getTimeColumnCount();
        if (inDateTimeMode) {
            const dateRatio = this.getDatePartRatio();
            const timeRatio = this.getTimePartRatio();
            const totalRatio = dateRatio + timeRatio;
            const percent = (timeRatio / totalRatio) * (1 / totalColumns) * 100;
            return `${percent.toFixed(2)}%`;
        }
        else {
            const percent = (1 / totalColumns) * 100;
            return `${percent.toFixed(2)}%`;
        }
    }
    getFirstPickerIndicator(radius = 12) {
        if (this.isRTL()) {
            return {
                type: PickerIndicatorType.BACKGROUND,
                borderRadius: {
                    topLeft: 0,
                    topRight: radius,
                    bottomLeft: 0,
                    bottomRight: radius
                }
            };
        }
        return {
            type: PickerIndicatorType.BACKGROUND,
            borderRadius: {
                topLeft: radius,
                topRight: 0,
                bottomLeft: radius,
                bottomRight: 0
            }
        };
    }
    getMiddlePickerIndicator() {
        return {
            type: PickerIndicatorType.BACKGROUND,
            borderRadius: {
                topLeft: 0,
                topRight: 0,
                bottomLeft: 0,
                bottomRight: 0
            }
        };
    }
    getLastPickerIndicator(radius = 12) {
        if (this.isRTL()) {
            return {
                type: PickerIndicatorType.BACKGROUND,
                borderRadius: {
                    topLeft: radius,
                    topRight: 0,
                    bottomLeft: radius,
                    bottomRight: 0
                }
            };
        }
        return {
            type: PickerIndicatorType.BACKGROUND,
            borderRadius: {
                topLeft: 0,
                topRight: radius,
                bottomLeft: 0,
                bottomRight: radius
            }
        };
    }
    getAdjustedRadius() {
        const minColumnWidth = Math.min(this.firstColumnWidth, this.lastColumnWidth);
        if (minColumnWidth <= 0) {
            return 12;
        }
        const calculatedRadius = Math.min(minColumnWidth / 2, 12);
        return Math.max(calculatedRadius, 0);
    }
    textStyleEqual(a, b) {
        if (a === b) {
            return true;
        }
        if (!a || !b) {
            return false;
        }
        if (a.color !== b.color) {
            return false;
        }
        if (a.overflow !== b.overflow) {
            return false;
        }
        if (a.minFontSize !== b.minFontSize) {
            return false;
        }
        if (a.maxFontSize !== b.maxFontSize) {
            return false;
        }
        if (a.font === b.font) {
            return true;
        }
        if (!a.font || !b.font) {
            return false;
        }
        if (a.font.size !== b.font.size) {
            return false;
        }
        if (a.font.weight !== b.font.weight) {
            return false;
        }
        if (a.font.family !== b.font.family) {
            return false;
        }
        if (a.font.style !== b.font.style) {
            return false;
        }
        return true;
    }
    lengthMetricsEqual(a, b) {
        if (a === b) {
            return true;
        }
        if (!a || !b) {
            return false;
        }
        return a.value === b.value && a.unit === b.unit;
    }
    borderRadiusEqual(a, b) {
        if (a === b) {
            return true;
        }
        if (!a || !b) {
            return false;
        }
        const lmA = a;
        const lmB = b;
        if (lmA.value !== undefined || lmB.value !== undefined) {
            if (lmA.value === undefined || lmB.value === undefined) {
                return false;
            }
            return lmA.value === lmB.value && lmA.unit === lmB.unit;
        }
        const lrA = a;
        const lrB = b;
        if (lrA.topStart !== undefined || lrA.topEnd !== undefined ||
            lrA.bottomStart !== undefined || lrA.bottomEnd !== undefined ||
            lrB.topStart !== undefined || lrB.topEnd !== undefined ||
            lrB.bottomStart !== undefined || lrB.bottomEnd !== undefined) {
            return this.lengthMetricsEqual(lrA.topStart, lrB.topStart) &&
                this.lengthMetricsEqual(lrA.topEnd, lrB.topEnd) &&
                this.lengthMetricsEqual(lrA.bottomStart, lrB.bottomStart) &&
                this.lengthMetricsEqual(lrA.bottomEnd, lrB.bottomEnd);
        }
        const rbA = a;
        const rbB = b;
        return rbA.topLeft === rbB.topLeft &&
            rbA.topRight === rbB.topRight &&
            rbA.bottomLeft === rbB.bottomLeft &&
            rbA.bottomRight === rbB.bottomRight;
    }
    indicatorStyleEqual(a, b) {
        if (a === b) {
            return true;
        }
        if (!a || !b) {
            return false;
        }
        if (a.type !== b.type) {
            return false;
        }
        if (a.dividerColor !== b.dividerColor) {
            return false;
        }
        if (a.backgroundColor !== b.backgroundColor) {
            return false;
        }
        if (!this.lengthMetricsEqual(a.startMargin, b.startMargin)) {
            return false;
        }
        if (!this.lengthMetricsEqual(a.endMargin, b.endMargin)) {
            return false;
        }
        if (!this.lengthMetricsEqual(a.strokeWidth, b.strokeWidth)) {
            return false;
        }
        if (!this.borderRadiusEqual(a.borderRadius, b.borderRadius)) {
            return false;
        }
        return true;
    }
    lengthMetricsToPx(lm) {
        if (lm.unit === LengthUnit.PX) {
            return lm.value;
        }
        else if (lm.unit === LengthUnit.VP) {
            return vp2px(lm.value);
        }
        else if (lm.unit === LengthUnit.FP) {
            return fp2px(lm.value);
        }
        else if (lm.unit === LengthUnit.LPX) {
            return lpx2px(lm.value);
        }
        return lm.value;
    }
    getColumnWidthPercent(overallIndex) {
        const dateCount = this.displayMode === DisplayMode.TIME ? 0 : this.getDateColumnCount();
        if (this.displayMode === DisplayMode.DATE) {
            return parseFloat(this.getDateColumnWidth(overallIndex));
        }
        else if (this.displayMode === DisplayMode.TIME) {
            return parseFloat(this.getTimeColumnWidth(overallIndex));
        }
        else {
            if (overallIndex < dateCount) {
                return parseFloat(this.getDateColumnWidth(overallIndex));
            }
            else {
                return parseFloat(this.getTimeColumnWidth(overallIndex - dateCount));
            }
        }
    }
    getTotalWidthPx() {
        if (this.firstColumnWidth <= 0) {
            return 0;
        }
        const firstPercent = this.getColumnWidthPercent(0);
        if (firstPercent <= 0) {
            return 0;
        }
        return vp2px(this.firstColumnWidth * 100 / firstPercent);
    }
    getColumnWidthPx(columnIndex) {
        const totalWidth = this.getTotalWidthPx();
        if (totalWidth <= 0) {
            return 0;
        }
        return totalWidth * this.getColumnWidthPercent(columnIndex) / 100;
    }
    getPrecedingWidthPx(columnIndex) {
        let preceding = 0;
        for (let i = 0; i < columnIndex; i++) {
            preceding += this.getColumnWidthPx(i);
        }
        return preceding;
    }
    getFollowingWidthPx(columnIndex) {
        const totalColumns = this.getColumnCount();
        let following = 0;
        for (let i = columnIndex + 1; i < totalColumns; i++) {
            following += this.getColumnWidthPx(i);
        }
        return following;
    }
    getTimeStartIndex() {
        return this.displayMode === DisplayMode.DATE_TIME ? this.getDateColumnCount() : 0;
    }
    adjustSelectionIndicatorForPosition(columnIndex) {
        const totalColumns = this.getColumnCount();
        const position = columnIndex === 0 ? 'first' :
            columnIndex === totalColumns - 1 ? 'last' : 'middle';
        if (!this.selectionIndicator) {
            if (position === 'first') {
                return this.getFirstPickerIndicator(this.getAdjustedRadius());
            }
            else if (position === 'last') {
                return this.getLastPickerIndicator(this.getAdjustedRadius());
            }
            return this.getMiddlePickerIndicator();
        }
        const indicator = this.selectionIndicator;
        const validType = (indicator.type === PickerIndicatorType.BACKGROUND ||
            indicator.type === PickerIndicatorType.DIVIDER)
            ? indicator.type : PickerIndicatorType.BACKGROUND;
        const userBR = indicator.borderRadius;
        const defaultRadius = this.getAdjustedRadius();
        let tl = defaultRadius;
        let tr = defaultRadius;
        let bl = defaultRadius;
        let br = defaultRadius;
        if (userBR) {
            const lm = userBR;
            const lr = userBR;
            const rb = userBR;
            if (lm.value !== undefined) {
                tl = tr = bl = br = lm.value;
            }
            else if (lr.topStart !== undefined) {
                if (this.isRTL()) {
                    tl = lr.topEnd !== undefined ? lr.topEnd.value : defaultRadius;
                    tr = lr.topStart.value;
                    bl = lr.bottomEnd !== undefined ? lr.bottomEnd.value : defaultRadius;
                    br = lr.bottomStart !== undefined ? lr.bottomStart.value : defaultRadius;
                }
                else {
                    tl = lr.topStart.value;
                    tr = lr.topEnd !== undefined ? lr.topEnd.value : defaultRadius;
                    bl = lr.bottomStart !== undefined ? lr.bottomStart.value : defaultRadius;
                    br = lr.bottomEnd !== undefined ? lr.bottomEnd.value : defaultRadius;
                }
            }
            else if (rb.topLeft !== undefined) {
                tl = rb.topLeft;
                tr = rb.topRight !== undefined ? rb.topRight : defaultRadius;
                bl = rb.bottomLeft !== undefined ? rb.bottomLeft : defaultRadius;
                br = rb.bottomRight !== undefined ? rb.bottomRight : defaultRadius;
            }
        }
        if (position === 'middle') {
            tl = tr = bl = br = 0;
        }
        else if (position === 'first') {
            if (this.isRTL()) {
                tl = 0;
                bl = 0;
            }
            else {
                tr = 0;
                br = 0;
            }
        }
        else {
            if (this.isRTL()) {
                tr = 0;
                br = 0;
            }
            else {
                tl = 0;
                bl = 0;
            }
        }
        let adjustedStartMargin = indicator.startMargin;
        let adjustedEndMargin = indicator.endMargin;
        let adjustedStrokeWidth = indicator.strokeWidth;
        const totalWidthPx = this.getTotalWidthPx();
        const currentColumnWidthPx = totalWidthPx > 0 ? this.getColumnWidthPx(columnIndex) : 0;
        if (totalWidthPx > 0 && currentColumnWidthPx > 0) {
            let rawStartPx = 0;
            let rawEndPx = 0;
            if (indicator.startMargin) {
                rawStartPx = Math.max(0, this.lengthMetricsToPx(indicator.startMargin));
            }
            if (indicator.endMargin) {
                rawEndPx = Math.max(0, this.lengthMetricsToPx(indicator.endMargin));
            }
            if (rawStartPx + rawEndPx > totalWidthPx) {
                adjustedStartMargin = undefined;
                adjustedEndMargin = undefined;
            }
            else {
                const precedingWidthPx = this.getPrecedingWidthPx(columnIndex);
                const followingWidthPx = this.getFollowingWidthPx(columnIndex);
                const remainingStart = Math.max(0, rawStartPx - precedingWidthPx);
                const remainingEnd = Math.max(0, rawEndPx - followingWidthPx);
                const safetyMargin = 2;
                const safeColumnWidth = Math.max(0, currentColumnWidthPx - safetyMargin);
                const cappedStart = Math.min(remainingStart, safeColumnWidth);
                adjustedStartMargin = cappedStart > 0 ? LengthMetrics.px(cappedStart) : undefined;
                const cappedEnd = Math.min(remainingEnd, safeColumnWidth);
                adjustedEndMargin = cappedEnd > 0 ? LengthMetrics.px(cappedEnd) : undefined;
            }
        }
        else {
            adjustedStartMargin = undefined;
            adjustedEndMargin = undefined;
        }
        return {
            type: validType,
            borderRadius: { topLeft: tl, topRight: tr, bottomLeft: bl, bottomRight: br },
            strokeWidth: adjustedStrokeWidth,
            dividerColor: indicator.dividerColor,
            startMargin: adjustedStartMargin,
            endMargin: adjustedEndMargin,
            backgroundColor: indicator.backgroundColor
        };
    }
    DateModeDateColumns(parent = null) {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIPickerComponent.create({ selectedIndex: this.getYearSelectedIndex() });
            UIPickerComponent.width(this.getDateColumnWidth(0));
            UIPickerComponent.height(200);
            UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
            UIPickerComponent.canLoop(this.yearColumnCanLoop);
            UIPickerComponent.enableHapticFeedback(this.dateHapticFeedback);
            UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(0));
            UIPickerComponent.onChange((selectedIndex) => {
                this.onYearChange(selectedIndex);
            });
            UIPickerComponent.onScrollStop((selectedIndex) => {
                this.dateOnScrollStop?.(this.getResult());
            });
            UIPickerComponent.onAreaChange((oldValue, newValue) => {
                const newWidth = newValue.width;
                if (this.firstColumnWidth !== newWidth) {
                    this.firstColumnWidth = newWidth;
                }
            });
        }, UIPickerComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = _item => {
                const item = _item;
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item);
                    Text.maxLines(1);
                    Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                    Text.fontColor(this.itemTextStyle?.color);
                    Text.fontWeight(this.itemTextStyle?.font?.weight);
                    Text.minFontSize(this.itemTextStyle?.minFontSize);
                    Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                    Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                }, Text);
                Text.pop();
            };
            this.forEachUpdateFunction(elmtId, this.yearArray, forEachItemGenFunction);
        }, ForEach);
        ForEach.pop();
        UIPickerComponent.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIPickerComponent.create({ selectedIndex: this.getMonthSelectedIndex() });
            UIPickerComponent.width(this.getDateColumnWidth(1));
            UIPickerComponent.height(200);
            UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
            UIPickerComponent.canLoop(this.monthColumnCanLoop);
            UIPickerComponent.enableHapticFeedback(this.dateHapticFeedback);
            UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(1));
            UIPickerComponent.onChange((selectedIndex) => {
                this.onMonthChange(selectedIndex);
            });
            UIPickerComponent.onScrollStop((selectedIndex) => {
                this.dateOnScrollStop?.(this.getResult());
            });
        }, UIPickerComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = _item => {
                const item = _item;
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item);
                    Text.maxLines(1);
                    Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                    Text.fontColor(this.itemTextStyle?.color);
                    Text.fontWeight(this.itemTextStyle?.font?.weight);
                    Text.minFontSize(this.itemTextStyle?.minFontSize);
                    Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                    Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                }, Text);
                Text.pop();
            };
            this.forEachUpdateFunction(elmtId, this.monthArray, forEachItemGenFunction);
        }, ForEach);
        ForEach.pop();
        UIPickerComponent.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIPickerComponent.create({ selectedIndex: this.getDaySelectedIndex() });
            UIPickerComponent.width(this.getDateColumnWidth(2));
            UIPickerComponent.height(200);
            UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
            UIPickerComponent.canLoop(this.dayColumnCanLoop);
            UIPickerComponent.enableHapticFeedback(this.dateHapticFeedback);
            UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(2));
            UIPickerComponent.onChange((selectedIndex) => {
                this.onDayChange(selectedIndex);
            });
            UIPickerComponent.onScrollStop((selectedIndex) => {
                this.dateOnScrollStop?.(this.getResult());
            });
            UIPickerComponent.onAreaChange((oldValue, newValue) => {
                if (this.displayMode !== DisplayMode.DATE_TIME) {
                    const newWidth = newValue.width;
                    if (this.lastColumnWidth !== newWidth) {
                        this.lastColumnWidth = newWidth;
                    }
                }
            });
        }, UIPickerComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = _item => {
                const item = _item;
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item);
                    Text.maxLines(1);
                    Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                    Text.fontColor(this.itemTextStyle?.color);
                    Text.fontWeight(this.itemTextStyle?.font?.weight);
                    Text.minFontSize(this.itemTextStyle?.minFontSize);
                    Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                    Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                }, Text);
                Text.pop();
            };
            this.forEachUpdateFunction(elmtId, this.dayArray, forEachItemGenFunction);
        }, ForEach);
        ForEach.pop();
        UIPickerComponent.pop();
    }
    DateModeYearMonthColumns(parent = null) {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIPickerComponent.create({ selectedIndex: this.getYearSelectedIndex() });
            UIPickerComponent.width(this.getDateColumnWidth(0));
            UIPickerComponent.height(200);
            UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
            UIPickerComponent.canLoop(this.yearColumnCanLoop);
            UIPickerComponent.enableHapticFeedback(this.dateHapticFeedback);
            UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(0));
            UIPickerComponent.onChange((selectedIndex) => {
                this.onYearChange(selectedIndex);
            });
            UIPickerComponent.onScrollStop((selectedIndex) => {
                this.dateOnScrollStop?.(this.getResult());
            });
            UIPickerComponent.onAreaChange((oldValue, newValue) => {
                const newWidth = newValue.width;
                if (this.firstColumnWidth !== newWidth) {
                    this.firstColumnWidth = newWidth;
                }
            });
        }, UIPickerComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = _item => {
                const item = _item;
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item);
                    Text.maxLines(1);
                    Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                    Text.fontColor(this.itemTextStyle?.color);
                    Text.fontWeight(this.itemTextStyle?.font?.weight);
                    Text.minFontSize(this.itemTextStyle?.minFontSize);
                    Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                    Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                }, Text);
                Text.pop();
            };
            this.forEachUpdateFunction(elmtId, this.yearArray, forEachItemGenFunction);
        }, ForEach);
        ForEach.pop();
        UIPickerComponent.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIPickerComponent.create({ selectedIndex: this.getMonthSelectedIndex() });
            UIPickerComponent.width(this.getDateColumnWidth(1));
            UIPickerComponent.height(200);
            UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
            UIPickerComponent.canLoop(this.monthColumnCanLoop);
            UIPickerComponent.enableHapticFeedback(this.dateHapticFeedback);
            UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(1));
            UIPickerComponent.onChange((selectedIndex) => {
                this.onMonthChange(selectedIndex);
            });
            UIPickerComponent.onScrollStop((selectedIndex) => {
                this.dateOnScrollStop?.(this.getResult());
            });
            UIPickerComponent.onAreaChange((oldValue, newValue) => {
                if (this.displayMode !== DisplayMode.DATE_TIME) {
                    const newWidth = newValue.width;
                    if (this.lastColumnWidth !== newWidth) {
                        this.lastColumnWidth = newWidth;
                    }
                }
            });
        }, UIPickerComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = _item => {
                const item = _item;
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item);
                    Text.maxLines(1);
                    Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                    Text.fontColor(this.itemTextStyle?.color);
                    Text.fontWeight(this.itemTextStyle?.font?.weight);
                    Text.minFontSize(this.itemTextStyle?.minFontSize);
                    Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                    Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                }, Text);
                Text.pop();
            };
            this.forEachUpdateFunction(elmtId, this.monthArray, forEachItemGenFunction);
        }, ForEach);
        ForEach.pop();
        UIPickerComponent.pop();
    }
    DateModeMonthDayColumns(parent = null) {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIPickerComponent.create({ selectedIndex: this.getMonthSelectedIndex() });
            UIPickerComponent.width(this.getDateColumnWidth(0));
            UIPickerComponent.height(200);
            UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
            UIPickerComponent.canLoop(this.monthColumnCanLoop);
            UIPickerComponent.enableHapticFeedback(this.dateHapticFeedback);
            UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(0));
            UIPickerComponent.onChange((selectedIndex) => {
                this.onMonthChange(selectedIndex);
            });
            UIPickerComponent.onScrollStop((selectedIndex) => {
                this.dateOnScrollStop?.(this.getResult());
            });
            UIPickerComponent.onAreaChange((oldValue, newValue) => {
                const newWidth = newValue.width;
                if (this.firstColumnWidth !== newWidth) {
                    this.firstColumnWidth = newWidth;
                }
            });
        }, UIPickerComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = _item => {
                const item = _item;
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item);
                    Text.maxLines(1);
                    Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                    Text.fontColor(this.itemTextStyle?.color);
                    Text.fontWeight(this.itemTextStyle?.font?.weight);
                    Text.minFontSize(this.itemTextStyle?.minFontSize);
                    Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                    Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                }, Text);
                Text.pop();
            };
            this.forEachUpdateFunction(elmtId, this.monthArray, forEachItemGenFunction);
        }, ForEach);
        ForEach.pop();
        UIPickerComponent.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            UIPickerComponent.create({ selectedIndex: this.getDaySelectedIndex() });
            UIPickerComponent.width(this.getDateColumnWidth(1));
            UIPickerComponent.height(200);
            UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
            UIPickerComponent.canLoop(this.dayColumnCanLoop);
            UIPickerComponent.enableHapticFeedback(this.dateHapticFeedback);
            UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(1));
            UIPickerComponent.onChange((selectedIndex) => {
                this.onDayChange(selectedIndex);
            });
            UIPickerComponent.onScrollStop((selectedIndex) => {
                this.dateOnScrollStop?.(this.getResult());
            });
            UIPickerComponent.onAreaChange((oldValue, newValue) => {
                if (this.displayMode !== DisplayMode.DATE_TIME) {
                    const newWidth = newValue.width;
                    if (this.lastColumnWidth !== newWidth) {
                        this.lastColumnWidth = newWidth;
                    }
                }
            });
        }, UIPickerComponent);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = _item => {
                const item = _item;
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item);
                    Text.maxLines(1);
                    Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                    Text.fontColor(this.itemTextStyle?.color);
                    Text.fontWeight(this.itemTextStyle?.font?.weight);
                    Text.minFontSize(this.itemTextStyle?.minFontSize);
                    Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                    Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                }, Text);
                Text.pop();
            };
            this.forEachUpdateFunction(elmtId, this.dayArray, forEachItemGenFunction);
        }, ForEach);
        ForEach.pop();
        UIPickerComponent.pop();
    }
    initialRender() {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create();
            Row.width('100%');
        }, Row);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (this.displayMode === DisplayMode.DATE || this.displayMode === DisplayMode.DATE_TIME) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                        If.create();
                        if (this.dateMode === DateMode.DATE) {
                            this.ifElseBranchUpdateFunction(0, () => {
                                this.observeComponentCreation2((elmtId, isInitialRender) => {
                                    If.create();
                                    if (this.lunar) {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.DateModeDateColumns.bind(this)();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.DateModeDateColumns.bind(this)();
                                        });
                                    }
                                }, If);
                                If.pop();
                            });
                        }
                        else if (this.dateMode === DateMode.YEAR_AND_MONTH) {
                            this.ifElseBranchUpdateFunction(1, () => {
                                this.observeComponentCreation2((elmtId, isInitialRender) => {
                                    If.create();
                                    if (this.lunar) {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.DateModeYearMonthColumns.bind(this)();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.DateModeYearMonthColumns.bind(this)();
                                        });
                                    }
                                }, If);
                                If.pop();
                            });
                        }
                        else if (this.dateMode === DateMode.MONTH_AND_DAY) {
                            this.ifElseBranchUpdateFunction(2, () => {
                                this.observeComponentCreation2((elmtId, isInitialRender) => {
                                    If.create();
                                    if (this.lunar) {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.DateModeMonthDayColumns.bind(this)();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.DateModeMonthDayColumns.bind(this)();
                                        });
                                    }
                                }, If);
                                If.pop();
                            });
                        }
                        else {
                            this.ifElseBranchUpdateFunction(3, () => {
                            });
                        }
                    }, If);
                    If.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
        }, If);
        If.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (this.displayMode === DisplayMode.TIME || this.displayMode === DisplayMode.DATE_TIME) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                        If.create();
                        if (this.timeFormat === TimeFormat.HOUR_MINUTE) {
                            this.ifElseBranchUpdateFunction(0, () => {
                                this.observeComponentCreation2((elmtId, isInitialRender) => {
                                    If.create();
                                    if (this.useMilitaryTime) {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getHourSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(0));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.hourColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onHourChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    this.selectedHour = this.startHour + selectedIndex;
                                                    this.updateTimeArrays();
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    if (this.displayMode === DisplayMode.TIME) {
                                                        const newWidth = newValue.width;
                                                        if (this.firstColumnWidth !== newWidth) {
                                                            this.firstColumnWidth = newWidth;
                                                        }
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.hourArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getMinuteSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(1));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.minuteColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onMinuteChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
                                                    if (this.selectedHour === this.startHour) {
                                                        startMinuteIndex = this.startMinute;
                                                    }
                                                    this.selectedMinute = startMinuteIndex + selectedIndex;
                                                    this.updateSecondArray();
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    const newWidth = newValue.width;
                                                    if (this.lastColumnWidth !== newWidth) {
                                                        this.lastColumnWidth = newWidth;
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.minuteArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.selectedPeriod });
                                                UIPickerComponent.width(this.getTimeColumnWidth(0));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(false);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onPeriodChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    this.selectedPeriod = selectedIndex;
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    if (this.displayMode === DisplayMode.TIME) {
                                                        const newWidth = newValue.width;
                                                        if (this.firstColumnWidth !== newWidth) {
                                                            this.firstColumnWidth = newWidth;
                                                        }
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.periodArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getHourSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(1));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.hourColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onHourChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    this.onHourChange(selectedIndex);
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.hourArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getMinuteSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(2));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.minuteColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 2));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onMinuteChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
                                                    if (this.selectedHour === this.startHour) {
                                                        startMinuteIndex = this.startMinute;
                                                    }
                                                    this.selectedMinute = startMinuteIndex + selectedIndex;
                                                    this.updateSecondArray();
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    const newWidth = newValue.width;
                                                    if (this.lastColumnWidth !== newWidth) {
                                                        this.lastColumnWidth = newWidth;
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.minuteArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                        });
                                    }
                                }, If);
                                If.pop();
                            });
                        }
                        else if (this.timeFormat === TimeFormat.HOUR_MINUTE_SECOND) {
                            this.ifElseBranchUpdateFunction(1, () => {
                                this.observeComponentCreation2((elmtId, isInitialRender) => {
                                    If.create();
                                    if (this.useMilitaryTime) {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getHourSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(0));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.hourColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onHourChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    this.selectedHour = this.startHour + selectedIndex;
                                                    this.updateTimeArrays();
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    if (this.displayMode === DisplayMode.TIME) {
                                                        const newWidth = newValue.width;
                                                        if (this.firstColumnWidth !== newWidth) {
                                                            this.firstColumnWidth = newWidth;
                                                        }
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.hourArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getMinuteSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(1));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.minuteColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onMinuteChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
                                                    if (this.selectedHour === this.startHour) {
                                                        startMinuteIndex = this.startMinute;
                                                    }
                                                    this.selectedMinute = startMinuteIndex + selectedIndex;
                                                    this.updateSecondArray();
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.minuteArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getSecondSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(2));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.secondColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 2));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onSecondChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    let startSecondIndex = DatePickerConstant.MIN_SECOND;
                                                    if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
                                                        startSecondIndex = this.startSecond;
                                                    }
                                                    this.selectedSecond = startSecondIndex + selectedIndex;
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    const newWidth = newValue.width;
                                                    if (this.lastColumnWidth !== newWidth) {
                                                        this.lastColumnWidth = newWidth;
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.secondArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.selectedPeriod });
                                                UIPickerComponent.width(this.getTimeColumnWidth(0));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(false);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onPeriodChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    this.selectedPeriod = selectedIndex;
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    if (this.displayMode === DisplayMode.TIME) {
                                                        const newWidth = newValue.width;
                                                        if (this.firstColumnWidth !== newWidth) {
                                                            this.firstColumnWidth = newWidth;
                                                        }
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.periodArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getHourSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(1));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.hourColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onHourChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    this.onHourChange(selectedIndex);
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.hourArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getMinuteSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(2));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.minuteColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 2));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onMinuteChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    let startMinuteIndex = DatePickerConstant.MIN_MINUTE;
                                                    if (this.selectedHour === this.startHour) {
                                                        startMinuteIndex = this.startMinute;
                                                    }
                                                    this.selectedMinute = startMinuteIndex + selectedIndex;
                                                    this.updateSecondArray();
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.minuteArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                UIPickerComponent.create({ selectedIndex: this.getSecondSelectedIndex() });
                                                UIPickerComponent.width(this.getTimeColumnWidth(3));
                                                UIPickerComponent.height(200);
                                                UIPickerComponent.selectedTextStyle(ObservedObject.GetRawObject(this.selectedTextStyle));
                                                UIPickerComponent.canLoop(this.secondColumnCanLoop);
                                                UIPickerComponent.enableHapticFeedback(this.timeHapticFeedback);
                                                UIPickerComponent.selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 3));
                                                UIPickerComponent.onChange((selectedIndex) => {
                                                    this.onSecondChange(selectedIndex);
                                                });
                                                UIPickerComponent.onScrollStop((selectedIndex) => {
                                                    let startSecondIndex = DatePickerConstant.MIN_SECOND;
                                                    if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
                                                        startSecondIndex = this.startSecond;
                                                    }
                                                    this.selectedSecond = startSecondIndex + selectedIndex;
                                                    this.timeOnScrollStop?.(this.getResult());
                                                });
                                                UIPickerComponent.onAreaChange((oldValue, newValue) => {
                                                    const newWidth = newValue.width;
                                                    if (this.lastColumnWidth !== newWidth) {
                                                        this.lastColumnWidth = newWidth;
                                                    }
                                                });
                                            }, UIPickerComponent);
                                            this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                ForEach.create();
                                                const forEachItemGenFunction = _item => {
                                                    const item = _item;
                                                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                                                        Text.create(item);
                                                        Text.maxLines(1);
                                                        Text.fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize());
                                                        Text.fontColor(this.itemTextStyle?.color);
                                                        Text.fontWeight(this.itemTextStyle?.font?.weight);
                                                        Text.minFontSize(this.itemTextStyle?.minFontSize);
                                                        Text.maxFontSize(this.itemTextStyle?.maxFontSize);
                                                        Text.textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip });
                                                    }, Text);
                                                    Text.pop();
                                                };
                                                this.forEachUpdateFunction(elmtId, this.secondArray, forEachItemGenFunction);
                                            }, ForEach);
                                            ForEach.pop();
                                            UIPickerComponent.pop();
                                        });
                                    }
                                }, If);
                                If.pop();
                            });
                        }
                        else {
                            this.ifElseBranchUpdateFunction(2, () => {
                            });
                        }
                    }, If);
                    If.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
        }, If);
        If.pop();
        Row.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    __resetStateVarsOnReuse__Internal(params) {
    }
}

export default {
  DatePickerComponent,
  DateMode,
  DisplayMode,
  TimeFormat,
  DatePickerComponentResult,
  CommonOptions,
  DateOptions,
  TimeOptions,
  DatePickerComponentOptions,
};
