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

import {
  Component,
  Builder,
  Row,
  Text,
  UIPickerComponent,
  ForEach,
  PickerIndicatorType,
  PickerIndicatorStyle,
  PickerComponentTextStyle,
  BorderRadiuses,
  LocalizedBorderRadiuses,
  Callback,
  Font,
  TextOverflow,
  Area
} from '@ohos.arkui.component';
import { State, PropRef, Watch } from '@ohos.arkui.stateManagement';
import { LengthMetrics, LengthUnit } from '@ohos.arkui.node';
import { intl } from '@kit.LocalizationKit';
import i18n from '@ohos.i18n';
import { commonEventManager } from '@kit.BasicServicesKit';

export enum DateMode {
  DATE = 0,
  YEAR_AND_MONTH = 1,
  MONTH_AND_DAY = 2
}

export enum DisplayMode {
  DATE = 0,
  TIME = 1,
  DATE_TIME = 2
}

export enum TimeFormat {
  HOUR_MINUTE = 0,
  HOUR_MINUTE_SECOND = 1
}

export class DatePickerComponentResult {
  public year?: int;
  public month?: int;
  public day?: int;
  public hour?: int;
  public minute?: int;
  public second?: int;
}

const noop = (result: DatePickerComponentResult) => {};

export class CommonOptions {
  public start?: Date;
  public end?: Date;
  public selected?: Date;
  public extendSelected?: Date | DatePickerComponentResult;
  public loop?: boolean;
  public onChange?: Callback<DatePickerComponentResult>;
  public onScrollStop?: Callback<DatePickerComponentResult>;
  public enableHapticFeedback?: boolean;
}

export class DateOptions extends CommonOptions {
  public mode?: DateMode;
  public lunar?: boolean;
  public hideYear?: boolean;
}

export class TimeOptions extends CommonOptions {
  public format?: TimeFormat;
  public useMilitaryTime?: boolean;
}

export class DatePickerComponentOptions {
  public displayMode?: DisplayMode;
  public dateOptions?: DateOptions;
  public timeOptions?: TimeOptions;
  public selectedTextStyle?: PickerComponentTextStyle;
  public itemTextStyle?: PickerComponentTextStyle;
  public selectionIndicator?: PickerIndicatorStyle;
}

class DatePickerConstant {
  public static readonly MIN_YEAR: int = 1;
  public static readonly MAX_YEAR: int = 9999;
  public static readonly DEFAULT_START_YEAR: int = 1970;
  public static readonly DEFAULT_END_YEAR: int = 2100;
  public static readonly MIN_MONTH: int = 0;
  public static readonly MAX_MONTH: int = 11;
  public static readonly MIN_DAY: int = 1;
  public static readonly MIN_HOUR: int = 0;
  public static readonly MAX_HOUR: int = 23;
  public static readonly MIN_MINUTE: int = 0;
  public static readonly MAX_MINUTE: int = 59;
  public static readonly MIN_SECOND: int = 0;
  public static readonly MAX_SECOND: int = 59;
  public static readonly MONTHS_IN_YEAR: int = 12;
  public static readonly DEFAULT_STEP: int = 1;
  public static readonly HIDE_YEAR_SENTINEL: int = -1;
  public static readonly HIDE_YEAR_TEXT: string = '----';
  public static readonly SYSTEM_MIN_DATE: Date = new Date(1970, 0, 1);
  public static readonly SYSTEM_MAX_DATE: Date = new Date(2100, 11, 31);
}

@Component
export struct DatePickerComponent {
  @PropRef @Watch('onOptionsChange') options: DatePickerComponentOptions = new DatePickerComponentOptions();
  @State displayMode: DisplayMode = DisplayMode.DATE;
  @State selectedYear: int = 0;
  @State selectedMonth: int = 0;
  @State selectedIsLeapMonth: boolean = false;
  @State selectedDay: int = 0;
  @State selectedHour: int = 0;
  @State selectedMinute: int = 0;
  @State selectedSecond: int = 0;
  @State startYear: int = DatePickerConstant.DEFAULT_START_YEAR;
  @State endYear: int = DatePickerConstant.DEFAULT_END_YEAR;
  @State startMonth: int = DatePickerConstant.MIN_MONTH;
  @State endMonth: int = DatePickerConstant.MAX_MONTH;
  @State startDay: int = DatePickerConstant.MIN_DAY;
  @State endDay: int = 31;
  @State lunarStartMonth: int = DatePickerConstant.MIN_MONTH;
  @State lunarEndMonth: int = DatePickerConstant.MAX_MONTH;
  @State lunarStartDay: int = DatePickerConstant.MIN_DAY;
  @State lunarEndDay: int = 30;
  @State lunarStartYear: int = 0;
  @State lunarEndYear: int = 0;
  @State startHour: int = DatePickerConstant.MIN_HOUR;
  @State endHour: int = DatePickerConstant.MAX_HOUR;
  @State startMinute: int = DatePickerConstant.MIN_MINUTE;
  @State endMinute: int = DatePickerConstant.MAX_MINUTE;
  @State startSecond: int = DatePickerConstant.MIN_SECOND;
  @State endSecond: int = DatePickerConstant.MAX_SECOND;
  @State dateMode: DateMode = DateMode.DATE;
  @State timeFormat: TimeFormat = TimeFormat.HOUR_MINUTE;
  @State useMilitaryTime: boolean = false;
  @State lunar: boolean = false;
  @State @Watch('onHideYearChange') hideYear: boolean = false;
  @State dateCanLoop: boolean = true;
  @State timeCanLoop: boolean = true;
  @State dateStartEndSet: boolean = false;
  @State timeStartEndSet: boolean = false;
  @State yearColumnCanLoop: boolean = true;
  @State monthColumnCanLoop: boolean = true;
  @State dayColumnCanLoop: boolean = true;
  @State hourColumnCanLoop: boolean = true;
  @State minuteColumnCanLoop: boolean = true;
  @State secondColumnCanLoop: boolean = true;
  @State dateHapticFeedback: boolean = true;
  @State timeHapticFeedback: boolean = true;
  @State yearArray: Array<string> = new Array<string>();
  @State monthArray: Array<string> = new Array<string>();
  @State dayArray: Array<string> = new Array<string>();
  @State hourArray: Array<string> = new Array<string>();
  @State minuteArray: Array<string> = new Array<string>();
  @State secondArray: Array<string> = new Array<string>();
  @State periodArray: Array<string> = new Array<string>();
  @State selectedPeriod: int = 0;
  @State currentLocale: string = '';
  @State firstColumnWidth: number = 0;
  @State lastColumnWidth: number = 0;
  @State selectedTextStyle: PickerComponentTextStyle | undefined = undefined;
  @State itemTextStyle: PickerComponentTextStyle | undefined = undefined;
  @State selectionIndicator: PickerIndicatorStyle | undefined = undefined;
  private dateOnChange: Callback<DatePickerComponentResult> = noop;
  private dateOnScrollStop: Callback<DatePickerComponentResult> = noop;
  private timeOnChange: Callback<DatePickerComponentResult> = noop;
  private timeOnScrollStop: Callback<DatePickerComponentResult> = noop;
  private localeID: string = '';
  private lunarCalendar: i18n.Calendar | null = null;
  private subscriber: commonEventManager.CommonEventSubscriber | null = null;
  private formatter: intl.NumberFormat = new intl.NumberFormat();
  private yearFormatter: intl.NumberFormat = new intl.NumberFormat('', { useGrouping: false });
  private userLunar: boolean = false;  // Save user-set lunar value
  private hourFormatter: intl.DateTimeFormat | null = null;
  private minuteFormatter: intl.DateTimeFormat | null = null;
  private secondFormatter: intl.DateTimeFormat | null = null;
  
  aboutToAppear(): void {
    const locale = i18n.System.getSystemLocaleInstance();
    this.localeID = locale.toString();
    this.currentLocale = i18n.System.getSystemLanguage();
    this.formatter = new intl.NumberFormat();
    this.yearFormatter = new intl.NumberFormat(this.localeID, { useGrouping: false });
    
    // Save user-set lunar value, decide whether to apply based on current language
    this.userLunar = this.lunar;
    const isChinese: boolean = this.isChineseLocale();
    if (!isChinese) {
      // Static scenario: non-Chinese language, force lunar to false
      this.lunar = false;
      this.lunarCalendar = null;
    } else {
      // Chinese language: apply lunar based on user setting
      if (this.lunar) {
        this.lunarCalendar = i18n.getCalendar(this.localeID, 'chinese');
      }
    }
  
    if (this.selectedYear === 0 || this.selectedMonth === 0 || this.selectedDay === 0) {
      const now = new Date();
      this.selectedYear = now.getFullYear().toInt();
      this.selectedMonth = now.getMonth().toInt();
      this.selectedDay = now.getDate().toInt();
      this.selectedHour = now.getHours().toInt();
      this.selectedMinute = now.getMinutes().toInt();
      this.selectedSecond = now.getSeconds().toInt();
    }
  
    if (this.options !== undefined && this.options !== null) {
      this.onOptionsChange('');
    } else {
      this.initArrays();
    }
    
    this.createLocaleChangeSubscriber();
  }
  
  aboutToDisappear(): void {
    this.lunarCalendar = null;
    
    // Unsubscribe from locale change events
    if (this.subscriber !== null) {
      const subscriber: commonEventManager.CommonEventSubscriber = this.subscriber!;
      commonEventManager.unsubscribe(subscriber, (err) => {
        if (err) {
          console.error(`Failed to unsubscribe common event. error code: ${err.code}, message: ${err.message}.`);
        }
      });
      this.subscriber = null;
    }
  }
  
  private createLocaleChangeSubscriber(): void {
    const subscribeInfo: commonEventManager.CommonEventSubscribeInfo = {
      events: [commonEventManager.Support.COMMON_EVENT_LOCALE_CHANGED]
    };
    
    commonEventManager.createSubscriber(subscribeInfo)
      .then((commonEventSubscriber: commonEventManager.CommonEventSubscriber) => {
        this.subscriber = commonEventSubscriber;
        commonEventManager.subscribe(commonEventSubscriber, (err, data) => {
          if (err) {
            console.error(`Failed to subscribe common event. error code: ${err.code}, message: ${err.message}.`);
            return;
          }
          // Update locale when system language changes
          this.currentLocale = i18n.System.getSystemLanguage();
          this.onLocaleChange();
        });
      })
      .catch((err: Error) => {
        console.error(`CreateSubscriber failed, code is ${err.message}`);
      });
  }
  
  private onLocaleChange(): void {
    const locale = i18n.System.getSystemLocaleInstance();
    this.localeID = locale.toString();
    this.formatter = new intl.NumberFormat();
    this.yearFormatter = new intl.NumberFormat(this.localeID, { useGrouping: false });
    
    // Reset cached formatters when locale changes
    this.hourFormatter = null;
    this.minuteFormatter = null;
    this.secondFormatter = null;
    
    const isChinese: boolean = this.isChineseLocale();
    const prevLunar: boolean = this.lunar;
    const prevLunarCalendar: i18n.Calendar | null = this.lunarCalendar;
    
    // Handle lunar calendar based on language
    if (isChinese) {
      // Chinese locale: use user-set lunar value
      this.lunar = this.userLunar;
      if (this.lunar) {
        this.lunarCalendar = i18n.getCalendar(this.localeID, 'chinese');
      } else {
        this.lunarCalendar = null;
      }
    } else {
      // Non-Chinese locale: disable lunar calendar but keep user setting
      this.lunar = false;
      this.lunarCalendar = null;
    }
    
// Convert date when switching between lunar and gregorian modes
    if (prevLunar !== this.lunar) {
      if (!prevLunar && this.lunar && this.lunarCalendar !== null) {
        // Switching from gregorian to lunar: convert gregorian date to lunar date
        const gregorianDate: Date = this.createDateFromParams(this.selectedYear, this.selectedMonth, this.selectedDay);
        this.lunarCalendar!.setTime(gregorianDate);
        this.selectedMonth = this.lunarCalendar!.get('month').toInt();
        this.selectedDay = this.lunarCalendar!.get('date').toInt();
        // Use Gregorian year with lunar year boundary adjustment.
        // lunarCalendar.get('year') returns sexagenary cycle (1-60), not Gregorian year.
        this.selectedYear = gregorianDate.getFullYear().toInt();
        if (this.selectedMonth >= 10 && gregorianDate.getMonth().toInt() <= 1) {
          this.selectedYear -= 1;
        }
        let clampStart: int = this.startYear;
        let clampEnd: int = this.endYear;
        if (this.lunar && this.lunarStartYear > 0) {
          clampStart = this.lunarStartYear;
        }
        if (this.lunar && this.lunarEndYear > 0) {
          clampEnd = this.lunarEndYear;
        }
        if (this.selectedYear < clampStart) {
          this.selectedYear = clampStart;
        }
        if (this.selectedYear > clampEnd) {
          this.selectedYear = clampEnd;
        }
        // Detect if the date falls in a leap month
        this.selectedIsLeapMonth = this.isDateInLeapMonth(gregorianDate, this.selectedYear, this.selectedMonth);
      } else if (prevLunar && !this.lunar && prevLunarCalendar !== null) {
        // Switching from lunar to gregorian: convert lunar date to gregorian date
        // Use prevLunarCalendar for conversion since current lunarCalendar is null
        const gregorianDate: Date | undefined = this.convertLunarToGregorianWithCalendar(
          prevLunarCalendar, this.selectedYear, this.selectedMonth, this.selectedDay, this.selectedIsLeapMonth
        );
        if (gregorianDate !== undefined) {
          this.selectedYear = gregorianDate.getFullYear().toInt();
          this.selectedMonth = gregorianDate.getMonth().toInt();
          this.selectedDay = gregorianDate.getDate().toInt();
        }
        this.selectedIsLeapMonth = false;
      }
    }
    
    this.initArrays();
  }
  
  private initArrays(): void {
    if (this.lunar) {
      this.initLunarArrays();
    } else {
      this.initYearArray();
      this.initMonthArray();
      this.initDayArray();
    }
    this.initTimeArrays();
    this.updateDateColumnLoopState();
    this.updateTimeColumnLoopState();
  }
  
  private initLunarArrays(): void {
    const startYear: int = this.lunar ? this.lunarStartYear : this.startYear;
    const endYear: int = this.lunar ? this.lunarEndYear : this.endYear;
    this.yearArray = [];
    for (let i: int = startYear; i <= endYear; i++) {
      this.yearArray.push(this.formatLunarYear(i));
    }
    if (this.hideYear) {
      this.yearArray.push(DatePickerConstant.HIDE_YEAR_TEXT);
    }
  
    let startLunarMonthIndex: int = DatePickerConstant.MIN_MONTH;
    let endLunarMonthIndex: int = DatePickerConstant.MAX_MONTH;
    
    if (this.selectedYear === startYear) {
      startLunarMonthIndex = this.lunarStartMonth;
    }
    if (this.selectedYear === endYear) {
      endLunarMonthIndex = this.lunarEndMonth;
    }
    
    // Build month array with leap months inserted
    this.monthArray = [];
    const leapMonth: int = this.getLunarLeapMonth(this.selectedYear);
    for (let i: int = startLunarMonthIndex; i <= endLunarMonthIndex; i++) {
      this.monthArray.push(this.formatLunarMonth(i, false));
      if (leapMonth >= 0 && i === leapMonth) {
        this.monthArray.push(this.formatLunarMonth(i, true));
      }
    }
  
    this.initLunarDayArray();
  }
  
  private initLunarDayArray(): void {
    this.dayArray = [];
    const lunarDays: int = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
    
    let startLunarDayIndex: int = DatePickerConstant.MIN_DAY;
    let endLunarDayIndex: int = lunarDays;
    
    if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
        !this.selectedIsLeapMonth) {
      startLunarDayIndex = this.lunarStartDay;
    }
    if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
        !this.selectedIsLeapMonth) {
      endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays).toInt();
    }
    
    for (let i: int = startLunarDayIndex; i <= endLunarDayIndex; i++) {
      this.dayArray.push(this.formatLunarDay(i));
    }
  }
  
  /**
   * Detects which lunar month (0-11) has a leap month in the given lunar year.
   * A leap month shares the same month index as its preceding regular month;
   * it's detected by finding a lunar month whose start (date==1) appears twice.
   * Returns the leap month index or -1 if no leap month exists.
   */
  private lunarLeapMonthCacheKey: int = -2;
  private lunarLeapMonthCacheValue: int = -1;

  private getLunarLeapMonth(gregorianYear: int): int {
    if (gregorianYear === DatePickerConstant.HIDE_YEAR_SENTINEL || this.lunarCalendar === null) {
      return -1;
    }
    if (gregorianYear === this.lunarLeapMonthCacheKey) {
      return this.lunarLeapMonthCacheValue;
    }
    try {
      // Use mid-year reference to determine the lunar year
      const refDate: Date = this.createDateFromParams(gregorianYear, 6, 1);
      this.lunarCalendar!.setTime(refDate);
      const expectedLunarYear: int = this.lunarCalendar!.get('year').toInt();

      // Track how many times each lunar month starts (date == 1)
      const monthStartCounts: int[] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
      let leapMonth: int = -1;

      // Scan through the Gregorian year (~384 days covers a full lunar year)
      const startDate: Date = this.createDateFromParams(gregorianYear, 0, 1);
      for (let dayOffset: number = 0; dayOffset < 470; dayOffset++) {
        const testDate: Date = new Date();
        testDate.setTime((startDate.getTime() + dayOffset * 86400000).toLong());
        this.lunarCalendar!.setTime(testDate);
        if (this.lunarCalendar!.get('year').toInt() !== expectedLunarYear) {
          if (leapMonth >= 0) {
            break;
          }
          continue;
        }
        if (this.lunarCalendar!.get('date').toInt() === 1) {
          const m: int = this.lunarCalendar!.get('month').toInt();
          monthStartCounts[m] = monthStartCounts[m] + 1;
          if (monthStartCounts[m] === 2) {
            leapMonth = m;
          }
        }
      }
      this.lunarLeapMonthCacheKey = gregorianYear;
      this.lunarLeapMonthCacheValue = leapMonth;
      return leapMonth;
    } catch (e) {
      return -1;
    }
  }

  /**
   * Determines whether a Gregorian date falls in the leap month (2nd occurrence)
   * of the given lunar month. Returns false if the month has no leap month or
   * the date is in the regular (1st) occurrence.
   */
  private isDateInLeapMonth(date: Date, gregorianYear: int, lunarMonth: int): boolean {
    const leapMonth: int = this.getLunarLeapMonth(gregorianYear);
    if (leapMonth < 0 || leapMonth !== lunarMonth || this.lunarCalendar === null) {
      return false;
    }
    // Find the start of the regular month (1st occurrence) and leap month (2nd occurrence)
    const regularStart: Date | null = this.findLunarMonthStart(gregorianYear, lunarMonth, 1);
    if (regularStart === null) {
      return false;
    }
    // If the date is before the end of the regular month, it's not in the leap month.
    // The regular month typically has 29 or 30 days; we check if the date is past
    // the regular month by comparing with the leap month start.
    const leapStart: Date | null = this.findLunarMonthStart(gregorianYear, lunarMonth, 2);
    if (leapStart === null) {
      return false;
    }
    return date.getTime() >= leapStart.getTime();
  }

  /**
   * Finds the Gregorian date for the start (lunar day==1) of the Nth occurrence
   * (1=regular, 2=leap) of the given lunar month in the given lunar year.
   */
  private findLunarMonthStart(gregorianYear: int, lunarMonth: int, occurrence: int): Date | null {
    if (this.lunarCalendar === null) {
      return null;
    }
    try {
      const refDate: Date = this.createDateFromParams(gregorianYear, 6, 1);
      this.lunarCalendar!.setTime(refDate);
      const expectedLunarYear: int = this.lunarCalendar!.get('year').toInt();

      let count: int = 0;
      const startDate: Date = this.createDateFromParams(gregorianYear, 0, 1);
      for (let dayOffset: number = 0; dayOffset < 470; dayOffset++) {
        const testDate: Date = new Date();
        testDate.setTime((startDate.getTime() + dayOffset * 86400000).toLong());
        this.lunarCalendar!.setTime(testDate);
        if (this.lunarCalendar!.get('year').toInt() !== expectedLunarYear) {
          continue;
        }
        if (this.lunarCalendar!.get('month').toInt() === lunarMonth && this.lunarCalendar!.get('date').toInt() === 1) {
          count++;
          if (count === occurrence) {
            return testDate;
          }
        }
      }
    } catch (e) {
      // fall through
    }
    return null;
  }

  private lunarDaysCacheKey: string = '';
  private lunarDaysCacheValue: int = 30;

  private getLunarDaysInMonth(gregorianYear: int, lunarMonth: int, isLeap: boolean): int {
    if (gregorianYear === DatePickerConstant.HIDE_YEAR_SENTINEL || this.lunarCalendar === null) {
      return 30;
    }
    const cacheKey: string = `${gregorianYear}_${lunarMonth}_${isLeap}`;
    if (cacheKey === this.lunarDaysCacheKey) {
      return this.lunarDaysCacheValue;
    }

    try {
      // Determine the lunar year from a mid-year reference date
      const refDate: Date = this.createDateFromParams(gregorianYear, 6, 1);
      this.lunarCalendar!.setTime(refDate);
      const expectedLunarYear: int = this.lunarCalendar!.get('year').toInt();

      // For isLeap=true, we need the 2nd occurrence of (lunarMonth, date=1);
      // for isLeap=false, we need the 1st occurrence.
      const targetOccurrence: int = isLeap ? 2 : 1;
      let occurrenceCount: int = 0;
      let monthStart: Date | null = null;
      let nextMonthStart: Date | null = null;

      // Scan through ~400 days to find the target month start and the next month start.
      // The expectedLunarYear check only applies when searching for monthStart;
      // nextMonthStart can be in the next lunar year (e.g., 腊月 → 正月 of next year).
      const startDate: Date = this.createDateFromParams(gregorianYear, 0, 1);
      for (let dayOffset: number = 0; dayOffset < 470 && nextMonthStart === null; dayOffset++) {
        const testDate: Date = new Date();
        testDate.setTime((startDate.getTime() + dayOffset * 86400000).toLong());
        this.lunarCalendar!.setTime(testDate);
        const currentLunarDay: int = this.lunarCalendar!.get('date').toInt();

        if (currentLunarDay === 1) {
          if (monthStart === null) {
            // Haven't found the target month start yet — must be in the expected lunar year
            if (this.lunarCalendar!.get('year').toInt() === expectedLunarYear) {
              const currentLunarMonth: int = this.lunarCalendar!.get('month').toInt();
              if (currentLunarMonth === lunarMonth) {
                occurrenceCount++;
                if (occurrenceCount === targetOccurrence) {
                  monthStart = testDate;
                }
              }
            }
          } else {
            // Found the target month start, now look for the next month start (any lunar year)
            nextMonthStart = testDate;
          }
        }
      }
      
      let result: int = 30;
      if (monthStart !== null && nextMonthStart !== null) {
        const diff: number = (nextMonthStart.getTime() - monthStart.getTime()) / 86400000;
        result = Math.round(diff).toInt();
      }
      this.lunarDaysCacheKey = cacheKey;
      this.lunarDaysCacheValue = result;
      return result;
    } catch (e) {
      return 30;
    }
  }
  
  private convertLunarToGregorian(
    gregorianYear: int, lunarMonth: int, lunarDay: int, isLeap: boolean
  ): Date | undefined {
    return this.convertLunarToGregorianWithCalendar(this.lunarCalendar, gregorianYear, lunarMonth, lunarDay, isLeap);
  }
  
  private convertResultCacheKey: string = '';
  private convertResultCacheValue: Date | undefined = undefined;

  private convertLunarToGregorianWithCalendar(
    calendar: i18n.Calendar | null, gregorianYear: int, lunarMonth: int, lunarDay: int, isLeap: boolean
  ): Date | undefined {
    if (calendar === null) {
      return undefined;
    }
    const cacheKey: string = `${gregorianYear}_${lunarMonth}_${lunarDay}_${isLeap}`;
    if (cacheKey === this.convertResultCacheKey) {
      return this.convertResultCacheValue;
    }

    try {
      // Determine the lunar year from a mid-year reference date
      const refDate: Date = this.createDateFromParams(gregorianYear, 6, 1);
      calendar.setTime(refDate);
      const expectedLunarYear: int = calendar.get('year').toInt();

      // Find the start of the target month occurrence (1st = regular, 2nd = leap)
      const targetOccurrence: int = isLeap ? 2 : 1;
      let occurrenceCount: int = 0;
      let monthStart: Date | null = null;

      const startDate: Date = this.createDateFromParams(gregorianYear, 0, 1);
      for (let dayOffset: number = 0; dayOffset < 470; dayOffset++) {
        const testDate: Date = new Date();
        testDate.setTime((startDate.getTime() + dayOffset * 86400000).toLong());
        calendar.setTime(testDate);
        if (calendar.get('year').toInt() !== expectedLunarYear) {
          continue;
        }
        if (calendar.get('month').toInt() === lunarMonth && calendar.get('date').toInt() === 1) {
          occurrenceCount++;
          if (occurrenceCount === targetOccurrence) {
            monthStart = testDate;
            break;
          }
        }
      }

      let resultDate: Date | undefined = undefined;
      if (monthStart !== null) {
        // Add (lunarDay - 1) days to reach the target date
        const lunarDayNum: number = lunarDay;
        const candidate: Date = new Date();
        candidate.setTime((monthStart.getTime() + (lunarDayNum - 1) * 86400000).toLong());
        // Verify the result is still in the correct lunar month
        calendar.setTime(candidate);
        if (calendar.get('month').toInt() === lunarMonth && calendar.get('date').toInt() === lunarDay) {
          resultDate = candidate;
        }
      }
      this.convertResultCacheKey = cacheKey;
      this.convertResultCacheValue = resultDate;
      return resultDate;
    } catch (e) {
      return undefined;
    }
  }
  
  private formatLunarYear(gregorianYear: int): string {
    // Try formatter for localization, fallback to plain string
    if (this.yearFormatter) {
      try {
        return this.yearFormatter.format(gregorianYear) + '年';
      } catch (error) {
        return String(gregorianYear) + '年';
      }
    }
    return String(gregorianYear) + '年';
  }
  
  private formatLunarMonth(month: int, isLeap: boolean): string {
    const lunarMonthNames: string[] = ['正月', '二月', '三月', '四月', '五月', '六月',
                     '七月', '八月', '九月', '十月', '冬月', '腊月'];
  
    if (isLeap) {
      return '闰' + lunarMonthNames[month];
    }
    return lunarMonthNames[month];
  }
  
  private formatLunarDay(day: int): string {
    const lunarDayNames: string[] = [
      '初一', '初二', '初三', '初四', '初五',
      '初六', '初七', '初八', '初九', '初十',
      '十一', '十二', '十三', '十四', '十五',
      '十六', '十七', '十八', '十九', '二十',
      '廿一', '廿二', '廿三', '廿四', '廿五',
      '廿六', '廿七', '廿八', '廿九', '三十'
    ];
  
    if (day >= 1 && day <= 30) {
      const idx: int = (day - 1).toInt();
      return lunarDayNames[idx];
    }
    return day + '日';
  }
  
  private initYearArray(): void {
    this.yearArray = [];
    for (let i: int = this.startYear; i <= this.endYear; i++) {
      this.yearArray.push(this.formatYear(i));
    }
    if (this.hideYear) {
      this.yearArray.push(DatePickerConstant.HIDE_YEAR_TEXT);
    }
  }
  
  private initMonthArray(): void {
    this.monthArray = [];
    let startMonthIndex: int = DatePickerConstant.MIN_MONTH;
    let endMonthIndex: int = DatePickerConstant.MAX_MONTH;
    
    if (this.selectedYear === this.startYear) {
      startMonthIndex = this.startMonth;
    }
    if (this.selectedYear === this.endYear) {
      endMonthIndex = this.endMonth;
    }
    
    for (let i: int = startMonthIndex; i <= endMonthIndex; i++) {
      this.monthArray.push(this.formatMonth(i));
    }
  }
  
  private initDayArray(): void {
    this.dayArray = [];
    const daysInMonth: int = this.getDaysInMonth(this.selectedYear, this.selectedMonth);
    let startDayIndex: int = DatePickerConstant.MIN_DAY;
    let endDayIndex: int = daysInMonth;
    
    if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
      startDayIndex = this.startDay;
    }
    if (this.selectedYear === this.endYear && this.selectedMonth === this.endMonth) {
      endDayIndex = Math.min(this.endDay, daysInMonth).toInt();
    }
    
    for (let i: int = startDayIndex; i <= endDayIndex; i++) {
      this.dayArray.push(this.formatDay(i));
    }
  }
  
  private initTimeArrays(): void {
    this.periodArray = [];
    if (!this.useMilitaryTime) {
      this.selectedPeriod = this.selectedHour < 12 ? 0 : 1;
      this.updatePeriodArray();
      this.updateHourArrayFor12Hour();
    } else {
      this.hourArray = [];
      for (let i: int = this.startHour; i <= this.endHour; i++) {
        this.hourArray.push(this.formatHour(i));
      }
    }
  
    this.minuteArray = [];
    let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
    let endMinuteIndex: int = DatePickerConstant.MAX_MINUTE;
    
    if (this.selectedHour === this.startHour) {
      startMinuteIndex = this.startMinute;
    }
    if (this.selectedHour === this.endHour) {
      endMinuteIndex = this.endMinute;
    }
    
    for (let i: int = startMinuteIndex; i <= endMinuteIndex; i++) {
      this.minuteArray.push(this.formatMinute(i));
    }
    
    if (this.selectedMinute < startMinuteIndex) {
      this.selectedMinute = startMinuteIndex;
    }
    if (this.selectedMinute > endMinuteIndex) {
      this.selectedMinute = endMinuteIndex;
    }
  
    this.secondArray = [];
    let startSecondIndex: int = DatePickerConstant.MIN_SECOND;
    let endSecondIndex: int = DatePickerConstant.MAX_SECOND;
    
    if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
      startSecondIndex = this.startSecond;
    }
    if (this.selectedHour === this.endHour && this.selectedMinute === this.endMinute) {
      endSecondIndex = this.endSecond;
    }
    
    for (let i: int = startSecondIndex; i <= endSecondIndex; i++) {
      this.secondArray.push(this.formatSecond(i));
    }
    
    if (this.selectedSecond < startSecondIndex) {
      this.selectedSecond = startSecondIndex;
    }
    if (this.selectedSecond > endSecondIndex) {
      this.selectedSecond = endSecondIndex;
    }
  }
  
  private updateHourArrayFor12Hour(): void {
    this.hourArray = [];
    
    const isAM: boolean = this.selectedPeriod === 0;
    
    // For 12-hour format, build hour array based on current period and time range
    // AM period: 12 (midnight 0:00) and 1-11 (1:00-11:00)
    // PM period: 12 (noon 12:00) and 1-11 (13:00-23:00)
    
    if (isAM) {
      // Check if 12 AM (hour24=0) is within range
      const hour24ForAm12: int = 0;
      if (hour24ForAm12 >= this.startHour && hour24ForAm12 <= this.endHour) {
        this.hourArray.push(this.formatHour12(12));
      }
      
      // Check if 1-11 AM (hour24=1-11) are within range
      for (let displayHour: int = 1; displayHour <= 11; displayHour++) {
        const hour24: int = displayHour;
        if (hour24 >= this.startHour && hour24 <= this.endHour) {
          this.hourArray.push(this.formatHour12(displayHour));
        }
      }
    } else {
      // Check if 12 PM (hour24=12) is within range
      const hour24ForPm12: int = 12;
      if (hour24ForPm12 >= this.startHour && hour24ForPm12 <= this.endHour) {
        this.hourArray.push(this.formatHour12(12));
      }
      
      // Check if 1-11 PM (hour24=13-23) are within range
      for (let displayHour: int = 1; displayHour <= 11; displayHour++) {
        const hour24: int = displayHour + 12;
        if (hour24 >= this.startHour && hour24 <= this.endHour) {
          this.hourArray.push(this.formatHour12(displayHour));
        }
      }
    }
    
    // Fallback: if no hours available for current period, show all 12 hours
    if (this.hourArray.length === 0) {
      for (let i: int = 1; i <= 12; i++) {
        this.hourArray.push(this.formatHour12(i));
      }
    }
  }
  
  private updatePeriodArray(): void {
    this.periodArray = [];
    
    // Check if AM period (hour24=0-11) has any available hours
    let canSelectAM: boolean = false;
    for (let hour: int = 0; hour <= 11; hour++) {
      if (hour >= this.startHour && hour <= this.endHour) {
        canSelectAM = true;
        break;
      }
    }
    
    // Check if PM period (hour24=12-23) has any available hours
    let canSelectPM: boolean = false;
    for (let hour: int = 12; hour <= 23; hour++) {
      if (hour >= this.startHour && hour <= this.endHour) {
        canSelectPM = true;
        break;
      }
    }
    
    if (canSelectAM && canSelectPM) {
      this.periodArray.push(this.formatPeriod(true));
      this.periodArray.push(this.formatPeriod(false));
    } else if (canSelectAM && !canSelectPM) {
      this.periodArray.push(this.formatPeriod(true));
      if (this.selectedPeriod === 1) {
        this.selectedPeriod = 0;
        // Adjust selectedHour to AM range when forced to switch to AM
        if (this.selectedHour >= 12) {
          const minAMHour: int = Math.max(0, this.startHour).toInt();
          const maxAMHour: int = Math.min(11, this.endHour).toInt();
          if (this.selectedHour === 12) {
            this.selectedHour = 0; // 12 PM -> 12 AM (0:00)
          } else {
            this.selectedHour = Math.max(minAMHour, Math.min(maxAMHour, this.selectedHour - 12)).toInt();
          }
        }
      }
    } else if (!canSelectAM && canSelectPM) {
      this.periodArray.push(this.formatPeriod(false));
      if (this.selectedPeriod === 0) {
        this.selectedPeriod = 1;
        // Adjust selectedHour to PM range when forced to switch to PM
        if (this.selectedHour < 12) {
          const minPMHour = Math.max(12, this.startHour).toInt();
          const maxPMHour = Math.min(23, this.endHour).toInt();
          // Calculate target hour (24-hour format): 12 AM->12, 1-11 AM->13-23
          let targetHour24 = this.selectedHour === 0 ? 12 : this.selectedHour + 12;
          // Ensure target hour is within valid range
          this.selectedHour = Math.max(minPMHour, Math.min(maxPMHour, targetHour24)).toInt();
        }
      }
    } else {
      // Fallback: show both periods if no hours available (shouldn't happen with valid ranges)
      this.periodArray.push(this.formatPeriod(true));
      this.periodArray.push(this.formatPeriod(false));
    }
  }
  
  private formatNumber(num: int): string {
    if (num < 10) {
      return '0' + String(num);
    }
    return String(num);
  }
  
  private isChineseLocale(): boolean {
    const localeStr: string = this.localeID;
    return localeStr.startsWith('zh') || localeStr.includes('zh-CN') || localeStr.includes('zh-TW') || localeStr.includes('zh-Hans') || localeStr.includes('zh-Hant');
  }
  
  private formatYear(year: int): string {
    // Previewer or invalid locale: default to Chinese format
    const localeStr: string = this.localeID;
    if (!localeStr || localeStr.length === 0) {
      return year + '年';
    }
    
    // Try to use formatter for localization on real device, fallback to plain string for previewer
    if (this.isChineseLocale()) {
      if (this.yearFormatter) {
        try {
          return this.yearFormatter.format(year) + '年';
        } catch (error) {
          return year + '年';
        }
      }
      return year + '年';
    }
    
    // Non-Chinese: try formatter for localized numbers, fallback to plain
    if (this.yearFormatter) {
      try {
        return this.yearFormatter.format(year);
      } catch (error) {
        return String(year);
      }
    }
    return String(year);
  }
  
  private formatMonth(month: int): string {
    // Previewer or invalid locale: default to Chinese format
    const localeStr: string = this.localeID;
    if (!localeStr || localeStr.length === 0) {
      return (month + 1) + '月';
    }
    
    if (this.isChineseLocale()) {
      return (month + 1) + '月';
    }
  
    const monthNames: string[] = [
      'January', 'February', 'March', 'April', 'May', 'June',
      'July', 'August', 'September', 'October', 'November', 'December'
    ];
  
    const shortMonthNames: string[] = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
                                       'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
  
    try {
      const monthFormat: string = this.displayMode === DisplayMode.DATE_TIME ? 'short' : 'long';
      const dateFormat = new intl.DateTimeFormat(localeStr, {
        month: monthFormat
      });
      const safeMonth: int = Math.max(0, Math.min(11, month)).toInt();
      const date = new Date(2026, safeMonth, 15);
      return dateFormat.format(date);
    } catch (error) {
      return this.displayMode === DisplayMode.DATE_TIME ? shortMonthNames[month] : monthNames[month];
    }
  }
  
  private formatDay(day: int): string {
    // Previewer or invalid locale: default to Chinese format
    const localeStr: string = this.localeID;
    if (!localeStr || localeStr.length === 0) {
      return day + '日';
    }
    
    // Try to use formatter for localization on real device, fallback to plain string for previewer
    if (this.isChineseLocale()) {
      if (this.formatter) {
        try {
          return this.formatter.format(day) + '日';
        } catch (error) {
          return day + '日';
        }
      }
      return day + '日';
    }
    
    // Non-Chinese: use DateTimeFormat for localized date format
    try {
      const dateFormat = new intl.DateTimeFormat(localeStr, {
        day: 'numeric'
      });
      const safeDay: int = Math.max(1, Math.min(31, day)).toInt();
      const date = new Date(2026, 0, safeDay);
      return dateFormat.format(date);
    } catch (error) {
      return String(day);
    }
  }
  
  private formatHour(hour24: int): string {
    // Try to use formatter for localization on real device, fallback to plain string for previewer
    // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
    try {
      const localeStr: string = this.localeID;
      
      // 1. Previewer or invalid locale: simple formatNumber
      if (!localeStr || localeStr.length === 0) {
        return this.formatNumber(hour24);
      }
      
      // 2. Chinese/English: simple formatNumber sufficient (no digit localization needed)
      const lang: string = localeStr.split('-')[0];
      if (lang === 'zh' || lang === 'en') {
        return this.formatNumber(hour24);
      }
      
      // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
      if (this.hourFormatter === null) {
        this.hourFormatter = new intl.DateTimeFormat(localeStr, {
          hour: '2-digit',
          hour12: false
        });
      }
      
      const date = new Date(2026, 0, 1, hour24, 0, 0);
      return this.hourFormatter!.format(date);
    } catch (error) {
      return this.formatNumber(hour24);
    }
  }
  
  private formatHour12(displayHour: int): string {
    // Try to use formatter for localization on real device, fallback to plain string for previewer
    // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
    try {
      const localeStr: string = this.localeID;
      
      // 1. Previewer or invalid locale: simple formatNumber
      if (!localeStr || localeStr.length === 0) {
        return this.formatNumber(displayHour);
      }
      
      // 2. Chinese/English: simple formatNumber sufficient (no digit localization needed)
      const lang: string = localeStr.split('-')[0];
      if (lang === 'zh' || lang === 'en') {
        return this.formatNumber(displayHour);
      }
      
      // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
      if (this.hourFormatter === null) {
        this.hourFormatter = new intl.DateTimeFormat(localeStr, {
          hour: '2-digit',
          hour12: true
        });
      }
      
      const date = new Date(2026, 0, 1, displayHour, 0, 0);
      const formatted: string = this.hourFormatter!.format(date);
      
      // Remove any non-digit characters (like AM/PM, spaces, etc.)
      // This gives us just the hour number in locale format
      // Include Unicode ranges for all localized digits:
      // Arabic-Indic, Persian, Devanagari, Bengali, Thai, Lao, Myanmar, Khmer
      const pattern =
        '[\\d\\u0660-\\u0669\\u06F0-\\u06F9]' +
        '[\\u0966-\\u096F\\u09E6-\\u09EF]' +
        '[\\u0E50-\\u0E59\\u0ED0-\\u0ED9]' +
        '[\\u1040-\\u1049\\u17E0-\\u17E9]+';
      const digitPattern = new RegExp(pattern);
      const hourMatch: RegExpMatchArray | null = formatted.match(digitPattern);
      if (hourMatch !== null && hourMatch.length > 0) {
        return hourMatch[0]!;
      }
      
      return this.formatNumber(displayHour);
    } catch (error) {
      return this.formatNumber(displayHour);
    }
  }
  
  private formatMinute(minute: int): string {
    // Try to use formatter for localization on real device, fallback to plain string for previewer
    // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
    try {
      const localeStr: string = this.localeID;
      
      // 1. Previewer or invalid locale: simple formatNumber
      if (!localeStr || localeStr.length === 0) {
        return this.formatNumber(minute);
      }
      
      // 2. Chinese/English: simple formatNumber sufficient (no digit localization needed)
      const lang: string = localeStr.split('-')[0];
      if (lang === 'zh' || lang === 'en') {
        return this.formatNumber(minute);
      }
      
      // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
      if (this.minuteFormatter === null) {
        this.minuteFormatter = new intl.DateTimeFormat(localeStr, {
          minute: '2-digit'
        });
      }
      
      const date = new Date(2026, 0, 1, 0, minute, 0);
      return this.minuteFormatter!.format(date);
    } catch (error) {
      return this.formatNumber(minute);
    }
  }
  
  private formatSecond(second: int): string {
    // Try to use formatter for localization on real device, fallback to plain string for previewer
    // Use DateTimeFormat with '2-digit' to ensure leading zeros and localized digits
    try {
      const localeStr: string = this.localeID;
      
      // 1. Previewer or invalid locale: simple formatNumber
      if (!localeStr || localeStr.length === 0) {
        return this.formatNumber(second);
      }
      
      // 2. Chinese/English: simple formatNumber sufficient (no digit localization needed)
      const lang: string = localeStr.split('-')[0];
      if (lang === 'zh' || lang === 'en') {
        return this.formatNumber(second);
      }
      
      // 3. Arabic, Hindi, Thai etc: use DateTimeFormat for localized digits with leading zero
      if (this.secondFormatter === null) {
        this.secondFormatter = new intl.DateTimeFormat(localeStr, {
          second: '2-digit'
        });
      }
      
      const date = new Date(2026, 0, 1, 0, 0, second);
      return this.secondFormatter!.format(date);
    } catch (error) {
      return this.formatNumber(second);
    }
  }
  
  private formatPeriod(isAM: boolean): string {
    // Direct translation map for period markers in different languages
    // Only include languages with non-AM/PM period markers
    
    try {
      const localeStr: string = this.localeID;
      
      // Previewer or invalid locale: use Chinese period markers as default
      if (!localeStr || localeStr.length === 0) {
        return isAM ? '上午' : '下午';
      }
      
      // Check language code prefix (e.g., 'zh-CN' -> 'zh')
      const langCode: string = localeStr.split('-')[0];
      
      // Languages with special period markers
      if (langCode === 'zh') {
        return isAM ? '上午' : '下午';
      } else if (langCode === 'ja') {
        return isAM ? '午前' : '午後';
      } else if (langCode === 'ko') {
        return isAM ? '오전' : '오후';
      } else if (langCode === 'ar') {
        return isAM ? 'ص' : 'م';
      } else if (langCode === 'my') {
        return isAM ? 'နံနက်' : 'ညနေ';
      } else if (langCode === 'th') {
        return isAM ? 'น.' : 'ทุ่ม';
      } else if (langCode === 'fa') {
        return isAM ? 'ق.ظ' : 'ب.ظ';
      } else if (langCode === 'ur') {
        return isAM ? 'صبح' : 'شام';
      } else if (langCode === 'hi') {
        return isAM ? 'पूर्वाह्न' : 'अपराह्न';
      } else if (langCode === 'bn') {
        return isAM ? 'পূর্বাহ্ণ' : 'অপরাহ্ণ';
      } else if (langCode === 'vi') {
        return isAM ? 'SA' : 'CH';
      } else if (langCode === 'ms') {
        return isAM ? 'PG' : 'PTG';
      } else if (langCode === 'tr') {
        return isAM ? 'ÖÖ' : 'AS';
      }
      
      // Fallback to AM/PM for all other languages
      return isAM ? 'AM' : 'PM';
    } catch (error) {
      return isAM ? 'AM' : 'PM';
    }
  }
  
  private getDaysInMonth(year: int, month: int): int {
    if (year === DatePickerConstant.HIDE_YEAR_SENTINEL) {
      if (month === 1) {
        return 29;
      }
      const monthsWith31Days: int[] = [0, 2, 4, 6, 7, 9, 11];
      const monthsWith30Days: int[] = [3, 5, 8, 10];
      for (let i: int = 0; i < monthsWith31Days.length; i++) {
        if (monthsWith31Days[i] === month) {
          return 31;
        }
      }
      for (let i: int = 0; i < monthsWith30Days.length; i++) {
        if (monthsWith30Days[i] === month) {
          return 30;
        }
      }
      return 30;
    }
    const monthsWith31Days: int[] = [0, 2, 4, 6, 7, 9, 11];
    const monthsWith30Days: int[] = [3, 5, 8, 10];
  
    if (month === 1) {
      const isLeapYear: boolean = (year % 4 === 0 && year % 100 !== 0) || (year % 400 === 0);
      return isLeapYear ? 29 : 28;
    }
  
    for (let i: int = 0; i < monthsWith31Days.length; i++) {
      if (monthsWith31Days[i] === month) {
        return 31;
      }
    }
  
    for (let i: int = 0; i < monthsWith30Days.length; i++) {
      if (monthsWith30Days[i] === month) {
        return 30;
      }
    }
  
    return 30;
  }
  
  private validateDate(date: Date): Date | undefined {
    try {
      const jsYear: int = date.getFullYear().toInt();
      const month: int = date.getMonth().toInt();
      const day: int = date.getDate().toInt();
    
      // Check for Invalid Date (NaN values)
      if (isNaN(jsYear) || isNaN(month) || isNaN(day)) {
        return undefined;
      }
    
      let actualYear: int = jsYear;
    
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
    
      const daysInMonth: int = this.getDaysInMonth(actualYear, month);
      if (day > daysInMonth) {
        return undefined;
      }
    
      // Create corrected date using setFullYear to preserve exact year
      const correctedDate: Date = new Date();
      correctedDate.setFullYear(actualYear, month, day);
    
      // Verify the date matches what was input (no overflow correction)
      const originalTime: number = date.getTime();
      const correctedTime: number = correctedDate.getTime();
    
      // If time difference > 365 days, indicates year jump due to abnormal month/day
      const timeDiff: number = Math.abs(originalTime - correctedTime);
      const oneYearInMs: number = 365 * 24 * 60 * 60 * 1000;
      if (timeDiff > oneYearInMs) {
        return undefined;
      }
    
      return correctedDate;
    } catch (e) {
      return undefined;
    }
  }
  
  private validateTime(date: Date): Date | undefined {
    try {
      const hour: int = date.getHours().toInt();
      const minute: int = date.getMinutes().toInt();
      const second: int = date.getSeconds().toInt();
    
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
      const year: int = date.getFullYear().toInt();
      const month: int = date.getMonth().toInt();
      const day: int = date.getDate().toInt();
    
      const originalTime: number = date.getTime();
      const correctedTime: number = this.createDateWithTime(year, month, day, hour, minute, second).getTime();
    
      // If time difference > 1 hour, indicates abnormal minute/second overflow
      const timeDiff: number = Math.abs(originalTime - correctedTime);
      const oneHourInMs: number = 60 * 60 * 1000;
      if (timeDiff > oneHourInMs) {
        return undefined;
      }
    
      // Return Date with fixed base date (2026-01-01) + validated time
      // This ensures timeOptions only considers time part, ignoring date part
      return new Date(2026, 0, 1, hour, minute, second);
    } catch (e) {
      return undefined;
    }
  }
  
  private createDateFromParams(year: int, month: int, day: int): Date {
    const date: Date = new Date();
    date.setFullYear(year, month, day);
    return date;
  }
  
  private createDateWithTime(year: int, month: int, day: int, hour: int, minute: int, second: int): Date {
    const date: Date = new Date();
    date.setFullYear(year, month, day);
    date.setHours(hour, minute, second);
    return date;
  }
  
  private onOptionsChange(str: string): void {
    // Restore defaults for undefined/null values
    if (this.options.displayMode !== undefined && this.options.displayMode !== null) {
      this.displayMode = this.options.displayMode!;
    } else {
      this.displayMode = DisplayMode.DATE;
    }
  
    if (this.options.dateOptions !== undefined && this.options.dateOptions !== null) {
      this.updateDateOptions(this.options.dateOptions!);
    } else {
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
      this.updateTimeOptions(this.options.timeOptions!);
    } else {
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
  
    const newSelectedTextStyle: PickerComponentTextStyle | undefined = this.options.selectedTextStyle;
    const newItemTextStyle: PickerComponentTextStyle | undefined = this.options.itemTextStyle;
    const newSelectionIndicator: PickerIndicatorStyle | undefined = this.options.selectionIndicator;
    if (!this.textStyleEqual(newSelectedTextStyle, this.selectedTextStyle)) {
      this.selectedTextStyle = newSelectedTextStyle;
    }
    if (!this.textStyleEqual(newItemTextStyle, this.itemTextStyle)) {
      this.itemTextStyle = newItemTextStyle;
    }
    if (!this.indicatorStyleEqual(newSelectionIndicator, this.selectionIndicator)) {
      this.selectionIndicator = newSelectionIndicator;
    }
  
    this.initArrays();
  }
  
  private updateDateOptions(dateOptions: DateOptions): void {
    // Restore defaults for undefined/null values
    if (dateOptions.mode !== undefined && dateOptions.mode !== null) {
      this.dateMode = dateOptions.mode!;
    } else {
      this.dateMode = DateMode.DATE;
    }
  
    if (dateOptions.lunar !== undefined && dateOptions.lunar !== null) {
      // Save user-set value first, then decide based on current language
      this.userLunar = dateOptions.lunar!;
      const isChinese: boolean = this.isChineseLocale();
      if (isChinese) {
        this.lunar = dateOptions.lunar!;
        if (this.lunar) {
          this.lunarCalendar = i18n.getCalendar(this.localeID, 'chinese');
        } else {
          this.lunarCalendar = null;
        }
      } else {
        // Non-Chinese language: force lunar to false, do not apply
        this.lunar = false;
        this.lunarCalendar = null;
      }
    } else {
      this.lunar = false;
      this.userLunar = false;
      this.lunarCalendar = null;
    }
  
    if (dateOptions.loop !== undefined && dateOptions.loop !== null) {
      this.dateCanLoop = dateOptions.loop!;
    } else {
      this.dateCanLoop = true;
    }
  
    if (dateOptions.enableHapticFeedback !== undefined && dateOptions.enableHapticFeedback !== null) {
      this.dateHapticFeedback = dateOptions.enableHapticFeedback!;
    } else {
      this.dateHapticFeedback = true;
    }

    if (dateOptions.hideYear !== undefined && dateOptions.hideYear !== null) {
      this.hideYear = dateOptions.hideYear!;
    } else {
      this.hideYear = false;
    }
  
    let startValid: boolean = false;
    let endValid: boolean = false;
    let startDate: Date | undefined = undefined;
    let endDate: Date | undefined = undefined;
  
    if (dateOptions.start !== undefined && dateOptions.start !== null) {
      startDate = this.validateDate(dateOptions.start!);
      if (startDate !== undefined) {
        startValid = true;
      }
    }
  
    if (dateOptions.end !== undefined && dateOptions.end !== null) {
      endDate = this.validateDate(dateOptions.end!);
      if (endDate !== undefined) {
        endValid = true;
      }
    }
  
    if (startValid && endValid) {
      if (startDate!.getTime() > endDate!.getTime()) {
        this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
        this.endYear = DatePickerConstant.DEFAULT_END_YEAR;
        this.startMonth = DatePickerConstant.MIN_MONTH;
        this.endMonth = DatePickerConstant.MAX_MONTH;
        this.startDay = DatePickerConstant.MIN_DAY;
        this.endDay = 31;
      } else {
        this.startYear = startDate!.getFullYear().toInt();
        this.startMonth = startDate!.getMonth().toInt();
        this.startDay = startDate!.getDate().toInt();
        this.endYear = endDate!.getFullYear().toInt();
        this.endMonth = endDate!.getMonth().toInt();
        this.endDay = endDate!.getDate().toInt();
      }
    } else if (startValid && !endValid) {
      this.startYear = startDate!.getFullYear().toInt();
      this.startMonth = startDate!.getMonth().toInt();
      this.startDay = startDate!.getDate().toInt();
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
    } else if (!startValid && endValid) {
      this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
      this.startMonth = DatePickerConstant.MIN_MONTH;
      this.startDay = DatePickerConstant.MIN_DAY;
      this.endYear = endDate!.getFullYear().toInt();
      this.endMonth = endDate!.getMonth().toInt();
      this.endDay = endDate!.getDate().toInt();
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
    } else {
      this.startYear = DatePickerConstant.DEFAULT_START_YEAR;
      this.endYear = DatePickerConstant.DEFAULT_END_YEAR;
      this.startMonth = DatePickerConstant.MIN_MONTH;
      this.endMonth = DatePickerConstant.MAX_MONTH;
      this.startDay = DatePickerConstant.MIN_DAY;
      this.endDay = 31;
    }
  
    if (this.startYear > this.endYear) {
      const tempYear: int = this.startYear;
      const tempMonth: int = this.startMonth;
      const tempDay: int = this.startDay;
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
      const startGregorian: Date = this.createDateFromParams(this.startYear, this.startMonth, this.startDay);
      this.lunarCalendar!.setTime(startGregorian);
      this.lunarStartMonth = this.lunarCalendar!.get('month').toInt();
      this.lunarStartDay = this.lunarCalendar!.get('date').toInt();
      this.lunarStartYear = this.startYear;
      if (this.lunarStartMonth >= 10 && startGregorian.getMonth().toInt() <= 1) { this.lunarStartYear -= 1; }
      
      const endGregorian: Date = this.createDateFromParams(this.endYear, this.endMonth, this.endDay);
      this.lunarCalendar!.setTime(endGregorian);
      this.lunarEndMonth = this.lunarCalendar!.get('month').toInt();
      this.lunarEndDay = this.lunarCalendar!.get('date').toInt();
      this.lunarEndYear = this.endYear;
      if (this.lunarEndMonth >= 10 && endGregorian.getMonth().toInt() <= 1) { this.lunarEndYear -= 1; }
    } else {
      this.lunarStartMonth = DatePickerConstant.MIN_MONTH;
      this.lunarEndMonth = DatePickerConstant.MAX_MONTH;
      this.lunarStartDay = DatePickerConstant.MIN_DAY;
      this.lunarEndDay = 30;
      this.lunarStartYear = this.startYear;
      this.lunarEndYear = this.endYear;
    }
  
    const now = new Date();
    let selectedDate: Date | undefined = undefined;
    let selectedValid: boolean = false;
    let selectedIsResult: boolean = false;

    // extendSelected takes priority over selected; when both are set, selected does not take effect.
    const effectiveSelected: Date | DatePickerComponentResult | undefined =
      (dateOptions.extendSelected !== undefined && dateOptions.extendSelected !== null)
        ? dateOptions.extendSelected
        : dateOptions.selected;

    if (effectiveSelected !== undefined && effectiveSelected !== null) {
      if (effectiveSelected instanceof Date) {
        // effectiveSelected is a Date: validate and reuse the normal flow.
        selectedDate = this.validateDate(effectiveSelected as Date);
        if (selectedDate !== undefined) {
          selectedValid = true;
        }
      } else {
        // effectiveSelected is a DatePickerComponentResult: restore a prior selection result.
        const result: DatePickerComponentResult = effectiveSelected as DatePickerComponentResult;
        // Each set field takes effect; each omitted field uses the corresponding
        // part of the system date.
        const m: int = result.month ?? now.getMonth().toInt();
        const d: int = result.day ?? now.getDate().toInt();
        if (this.hideYear && result.year === undefined) {
          // hideYear is enabled and year is omitted: select the hidden year column
          // ("----") only when (month, day) form a valid sentinel date; otherwise
          // (e.g. Feb 30) fall back to the system default date.
          // In lunar mode, validate against lunar days; in gregorian mode, gregorian days.
          const maxDay: int = this.lunar
            ? this.getLunarDaysInMonth(DatePickerConstant.HIDE_YEAR_SENTINEL, m, false)
            : this.getDaysInMonth(DatePickerConstant.HIDE_YEAR_SENTINEL, m);
          if (m >= DatePickerConstant.MIN_MONTH && m <= DatePickerConstant.MAX_MONTH &&
              d >= DatePickerConstant.MIN_DAY && d <= maxDay) {
            selectedIsResult = true;
            this.selectedYear = DatePickerConstant.HIDE_YEAR_SENTINEL;
            this.selectedMonth = m;
            this.selectedDay = d;
            this.selectedIsLeapMonth = false;
          }
        } else {
          // Real year: use the provided year, or the system year when omitted
          // (and hideYear is off). Rebuild a Date and reuse the normal flow
          // (range clamping + lunar conversion); invalid (e.g. Feb 30) falls back
          // to the system default date.
          const y: int = result.year ?? now.getFullYear().toInt();
          selectedDate = this.validateDate(this.createDateFromParams(y, m, d));
          if (selectedDate !== undefined) {
            selectedValid = true;
          }
        }
      }
    }

    // Create Date objects for start and end
    const startDateObj: Date = this.createDateFromParams(this.startYear, this.startMonth, this.startDay);
    const endDateObj: Date = this.createDateFromParams(this.endYear, this.endMonth, this.endDay);

    // Determine the initial selected date
    let targetDate: Date;
    if (selectedValid) {
      targetDate = selectedDate!;
    } else {
      targetDate = now;
    }

    if (!selectedIsResult) {
      // Adjust selected date to be within range
      if (targetDate.getTime() < startDateObj.getTime()) {
        targetDate = startDateObj;
      } else if (targetDate.getTime() > endDateObj.getTime()) {
        targetDate = endDateObj;
      }

      // Extract year, month, day from adjusted date
      if (this.lunar && this.lunarCalendar !== null) {
        this.lunarCalendar!.setTime(targetDate);
        this.selectedMonth = this.lunarCalendar!.get('month').toInt();
        this.selectedDay = this.lunarCalendar!.get('date').toInt();
        // Use Gregorian year with lunar year boundary adjustment.
        // lunarCalendar.get('year') returns sexagenary cycle (1-60), not Gregorian year.
        this.selectedYear = targetDate.getFullYear().toInt();
        if (this.selectedMonth >= 10 && targetDate.getMonth().toInt() <= 1) {
          this.selectedYear -= 1;
        }
        if (this.selectedYear < this.lunarStartYear) {
          this.selectedYear = this.lunarStartYear;
        }
        if (this.selectedYear > this.lunarEndYear) {
          this.selectedYear = this.lunarEndYear;
        }
        // Detect if the target date falls in a leap month
        this.selectedIsLeapMonth = this.isDateInLeapMonth(targetDate, this.selectedYear, this.selectedMonth);

        // Ensure selected date is within lunar range based on gregorian year
        if (this.selectedYear === this.lunarStartYear) {
          if (this.selectedMonth < this.lunarStartMonth) {
            this.selectedMonth = this.lunarStartMonth;
            this.selectedIsLeapMonth = false;
            this.selectedDay = this.lunarStartDay;
          } else if (this.selectedMonth === this.lunarStartMonth && this.selectedDay < this.lunarStartDay &&
                     !this.selectedIsLeapMonth) {
            this.selectedDay = this.lunarStartDay;
          }
        }
        if (this.selectedYear === this.lunarEndYear) {
          if (this.selectedMonth > this.lunarEndMonth) {
            this.selectedMonth = this.lunarEndMonth;
            this.selectedIsLeapMonth = false;
            this.selectedDay = this.lunarEndDay;
          } else if (this.selectedMonth === this.lunarEndMonth && this.selectedDay > this.lunarEndDay &&
                     !this.selectedIsLeapMonth) {
            this.selectedDay = this.lunarEndDay;
          }
        }
      } else {
        this.selectedYear = targetDate.getFullYear().toInt();
        this.selectedMonth = targetDate.getMonth().toInt();
        this.selectedDay = targetDate.getDate().toInt();

        // Ensure day doesn't exceed days in month (gregorian)
        const daysInMonth: int = this.getDaysInMonth(this.selectedYear, this.selectedMonth);
        if (this.selectedDay > daysInMonth) {
          this.selectedDay = daysInMonth;
        }
      }
    }
  
    if (dateOptions.onChange !== undefined) {
      this.dateOnChange = dateOptions.onChange ?? noop;
    }
  
    if (dateOptions.onScrollStop !== undefined) {
      this.dateOnScrollStop = dateOptions.onScrollStop ?? noop;
    }
  }
  
  private updateTimeOptions(timeOptions: TimeOptions): void {
    // Restore defaults for undefined/null values
    if (timeOptions.format !== undefined && timeOptions.format !== null) {
      this.timeFormat = timeOptions.format!;
    } else {
      this.timeFormat = TimeFormat.HOUR_MINUTE;
    }
  
    if (timeOptions.useMilitaryTime !== undefined && timeOptions.useMilitaryTime !== null) {
      this.useMilitaryTime = timeOptions.useMilitaryTime!;
    } else {
      this.useMilitaryTime = false;
    }
  
    if (timeOptions.loop !== undefined && timeOptions.loop !== null) {
      this.timeCanLoop = timeOptions.loop!;
    } else {
      this.timeCanLoop = true;
    }
  
    if (timeOptions.enableHapticFeedback !== undefined && timeOptions.enableHapticFeedback !== null) {
      this.timeHapticFeedback = timeOptions.enableHapticFeedback!;
    } else {
      this.timeHapticFeedback = true;
    }
  
    let startValid: boolean = false;
    let endValid: boolean = false;
    let startTime: Date | undefined = undefined;
    let endTime: Date | undefined = undefined;
  
    if (timeOptions.start !== undefined && timeOptions.start !== null) {
      startTime = this.validateTime(timeOptions.start!);
      if (startTime !== undefined) {
        startValid = true;
      }
    }
  
    if (timeOptions.end !== undefined && timeOptions.end !== null) {
      endTime = this.validateTime(timeOptions.end!);
      if (endTime !== undefined) {
        endValid = true;
      }
    }
  
    if (startValid && endValid) {
      if (startTime!.getHours() > endTime!.getHours() ||
          (startTime!.getHours() === endTime!.getHours() && startTime!.getMinutes() > endTime!.getMinutes()) ||
          (startTime!.getHours() === endTime!.getHours() && startTime!.getMinutes() === endTime!.getMinutes() && startTime!.getSeconds() > endTime!.getSeconds())) {
        this.startHour = DatePickerConstant.MIN_HOUR;
        this.endHour = DatePickerConstant.MAX_HOUR;
        this.startMinute = DatePickerConstant.MIN_MINUTE;
        this.endMinute = DatePickerConstant.MAX_MINUTE;
        this.startSecond = DatePickerConstant.MIN_SECOND;
        this.endSecond = DatePickerConstant.MAX_SECOND;
      } else {
        this.startHour = startTime!.getHours().toInt();
        this.startMinute = startTime!.getMinutes().toInt();
        this.startSecond = startTime!.getSeconds().toInt();
        this.endHour = endTime!.getHours().toInt();
        this.endMinute = endTime!.getMinutes().toInt();
        this.endSecond = endTime!.getSeconds().toInt();
      }
    } else if (startValid && !endValid) {
      this.startHour = startTime!.getHours().toInt();
      this.startMinute = startTime!.getMinutes().toInt();
      this.startSecond = startTime!.getSeconds().toInt();
      this.endHour = DatePickerConstant.MAX_HOUR;
      this.endMinute = DatePickerConstant.MAX_MINUTE;
      this.endSecond = DatePickerConstant.MAX_SECOND;
    } else if (!startValid && endValid) {
      this.startHour = DatePickerConstant.MIN_HOUR;
      this.startMinute = DatePickerConstant.MIN_MINUTE;
      this.startSecond = DatePickerConstant.MIN_SECOND;
      this.endHour = endTime!.getHours().toInt();
      this.endMinute = endTime!.getMinutes().toInt();
      this.endSecond = endTime!.getSeconds().toInt();
    } else {
      this.startHour = DatePickerConstant.MIN_HOUR;
      this.endHour = DatePickerConstant.MAX_HOUR;
      this.startMinute = DatePickerConstant.MIN_MINUTE;
      this.endMinute = DatePickerConstant.MAX_MINUTE;
      this.startSecond = DatePickerConstant.MIN_SECOND;
      this.endSecond = DatePickerConstant.MAX_SECOND;
    }
  
    this.startHour = Math.max(DatePickerConstant.MIN_HOUR, Math.min(DatePickerConstant.MAX_HOUR, this.startHour)).toInt();
    this.endHour = Math.max(DatePickerConstant.MIN_HOUR, Math.min(DatePickerConstant.MAX_HOUR, this.endHour)).toInt();
    this.startMinute = Math.max(DatePickerConstant.MIN_MINUTE, Math.min(DatePickerConstant.MAX_MINUTE, this.startMinute)).toInt();
    this.endMinute = Math.max(DatePickerConstant.MIN_MINUTE, Math.min(DatePickerConstant.MAX_MINUTE, this.endMinute)).toInt();
    this.startSecond = Math.max(DatePickerConstant.MIN_SECOND, Math.min(DatePickerConstant.MAX_SECOND, this.startSecond)).toInt();
    this.endSecond = Math.max(DatePickerConstant.MIN_SECOND, Math.min(DatePickerConstant.MAX_SECOND, this.endSecond)).toInt();
    
    this.timeStartEndSet = startValid || endValid;
   
    const now = new Date();
    let selectedTime: Date | undefined = undefined;
    let selectedValid: boolean = false;

    // extendSelected takes priority over selected; when both are set, selected does not take effect.
    const effectiveSelected: Date | DatePickerComponentResult | undefined =
      (timeOptions.extendSelected !== undefined && timeOptions.extendSelected !== null)
        ? timeOptions.extendSelected
        : timeOptions.selected;

    if (effectiveSelected !== undefined && effectiveSelected !== null) {
      if (effectiveSelected instanceof Date) {
        // effectiveSelected is a Date: validate and reuse the normal flow.
        selectedTime = this.validateTime(effectiveSelected as Date);
        if (selectedTime !== undefined) {
          selectedValid = true;
        }
      } else {
        // effectiveSelected is a DatePickerComponentResult: restore a prior selection result.
        const result: DatePickerComponentResult = effectiveSelected as DatePickerComponentResult;
        // Each set field takes effect; each omitted field uses the corresponding
        // part of the system time.
        const h: int = result.hour ?? now.getHours().toInt();
        const min: int = result.minute ?? now.getMinutes().toInt();
        const sec: int = result.second ?? now.getSeconds().toInt();
        selectedTime = this.validateTime(new Date(2026, 0, 1, h, min, sec));
        if (selectedTime !== undefined) {
          selectedValid = true;
        }
      }
    }
  
    // Determine the initial selected time
    let targetTime: Date;
    if (selectedValid) {
      targetTime = selectedTime!;
    } else {
      targetTime = now;
    }
  
    // Create Date objects for start and end time on same day
    const startTimeObj: Date = new Date(2026, 0, 1, this.startHour, this.startMinute, this.startSecond);
    const endTimeObj: Date = new Date(2026, 0, 1, this.endHour, this.endMinute, this.endSecond);
    const targetTimeObj: Date = new Date(2026, 0, 1, targetTime.getHours(), targetTime.getMinutes(), targetTime.getSeconds());
  
    // Adjust selected time to be within range
    if (targetTimeObj.getTime() < startTimeObj.getTime()) {
      targetTimeObj.setTime(startTimeObj.getTime());
    } else if (targetTimeObj.getTime() > endTimeObj.getTime()) {
      targetTimeObj.setTime(endTimeObj.getTime());
    }
  
    // Extract hour, minute, second from adjusted time
    this.selectedHour = targetTimeObj.getHours().toInt();
    this.selectedMinute = targetTimeObj.getMinutes().toInt();
    this.selectedSecond = targetTimeObj.getSeconds().toInt();
  
    // Set selectedPeriod based on selectedHour for 12-hour format
    if (!this.useMilitaryTime) {
      this.selectedPeriod = this.selectedHour < 12 ? 0 : 1;
    }
  
    if (timeOptions.onChange !== undefined) {
      this.timeOnChange = timeOptions.onChange ?? noop;
    }
  
    if (timeOptions.onScrollStop !== undefined) {
      this.timeOnScrollStop = timeOptions.onScrollStop ?? noop;
    }
  }
  
  private getResult(): DatePickerComponentResult {
    const result = new DatePickerComponentResult();
  
    if (this.selectedYear === DatePickerConstant.HIDE_YEAR_SENTINEL) {
      result.month = this.selectedMonth;
      result.day = this.selectedDay;
    } else if (this.lunar && this.lunarCalendar !== null) {
      const gregorianDate: Date | undefined = this.convertLunarToGregorian(
        this.selectedYear, this.selectedMonth, this.selectedDay, this.selectedIsLeapMonth
      );
      if (gregorianDate !== undefined) {
        result.year = gregorianDate.getFullYear().toInt();
        result.month = gregorianDate.getMonth().toInt();
        result.day = gregorianDate.getDate().toInt();
      } else {
        result.month = this.selectedMonth;
        result.day = this.selectedDay;
      }
    } else {
      result.year = this.selectedYear;
      result.month = this.selectedMonth;
      result.day = this.selectedDay;
    }
  
    result.hour = this.selectedHour;
  
    result.minute = this.selectedMinute;
    result.second = this.selectedSecond;
    return result;
  }
  
private onYearChange(selectedIndex: int): void {
    const newYear: int = this.hideYear && selectedIndex === this.yearArray.length - 1
      ? DatePickerConstant.HIDE_YEAR_SENTINEL
      : (this.lunar ? this.lunarStartYear : this.startYear) + selectedIndex;
    if (newYear === this.selectedYear) {
      return;
    }
    this.selectedYear = newYear;
    
    if (this.lunar) {
      // Calculate valid month range first
      let startLunarMonthIndex: int = DatePickerConstant.MIN_MONTH;
      let endLunarMonthIndex: int = DatePickerConstant.MAX_MONTH;
      
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
      const newLeapMonth: int = this.getLunarLeapMonth(this.selectedYear);
      if (this.selectedIsLeapMonth && (newLeapMonth < 0 || this.selectedMonth !== newLeapMonth)) {
        this.selectedIsLeapMonth = false;
      }
      
      // Build month array with leap months
      this.monthArray = [];
      for (let i: int = startLunarMonthIndex; i <= endLunarMonthIndex; i++) {
        this.monthArray.push(this.formatLunarMonth(i, false));
        if (newLeapMonth >= 0 && i === newLeapMonth) {
          this.monthArray.push(this.formatLunarMonth(i, true));
        }
      }
      
      // Adjust selectedDay BEFORE generating day array
      const lunarDays: int = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
      let startLunarDayIndex: int = DatePickerConstant.MIN_DAY;
      let endLunarDayIndex: int = lunarDays;
      
      if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
          !this.selectedIsLeapMonth) {
        startLunarDayIndex = this.lunarStartDay;
      }
      if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
          !this.selectedIsLeapMonth) {
        endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays).toInt();
      }
      
      if (this.selectedDay < startLunarDayIndex) {
        this.selectedDay = startLunarDayIndex;
      }
      if (this.selectedDay > endLunarDayIndex) {
        this.selectedDay = endLunarDayIndex;
      }
      
      // Generate day array
      this.dayArray = [];
      for (let i: int = startLunarDayIndex; i <= endLunarDayIndex; i++) {
        this.dayArray.push(this.formatLunarDay(i));
      }
    } else {
      // Calculate valid month range first
      let startMonthIndex: int = DatePickerConstant.MIN_MONTH;
      let endMonthIndex: int = DatePickerConstant.MAX_MONTH;
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
      setTimeout(() => { this.dateOnChange(this.getResult()); }, 0);
    } else {
      this.dateOnChange(this.getResult());
    }
  }
  
  private applyArrayIndexToLunarMonth(selectedIndex: int, startLunarMonthIndex: int): void {
    // Decode selectedIndex into (month, isLeap) accounting for leap month insertion.
    // Sets this.selectedMonth and this.selectedIsLeapMonth directly (no object return).
    const leapMonth: int = this.getLunarLeapMonth(this.selectedYear);
    this.selectedIsLeapMonth = false;
    if (leapMonth >= 0 && leapMonth >= startLunarMonthIndex) {
      const leapArrayIndex: int = leapMonth - startLunarMonthIndex; // index of regular leap month
      if (selectedIndex === leapArrayIndex + 1) {
        // The entry right after the regular leap month is the leap month
        this.selectedMonth = leapMonth;
        this.selectedIsLeapMonth = true;
      } else if (selectedIndex <= leapArrayIndex) {
        this.selectedMonth = startLunarMonthIndex + selectedIndex;
      } else {
        // After the leap month entry, subtract 1 from the index
        this.selectedMonth = startLunarMonthIndex + selectedIndex - 1;
      }
    } else {
      this.selectedMonth = startLunarMonthIndex + selectedIndex;
    }
  }

  private onMonthChange(selectedIndex: int): void {
    if (this.lunar) {
      let startLunarMonthIndex: int = DatePickerConstant.MIN_MONTH;
      if (this.selectedYear === this.lunarStartYear) {
        startLunarMonthIndex = this.lunarStartMonth;
      }
      // Save old values to detect if anything actually changed
      const oldMonth: int = this.selectedMonth;
      const oldIsLeap: boolean = this.selectedIsLeapMonth;
      // Decode selectedIndex into (month, isLeap) accounting for leap month insertion
      this.applyArrayIndexToLunarMonth(selectedIndex, startLunarMonthIndex);
      // Guard: if the month hasn't actually changed, do nothing.
      if (this.selectedMonth === oldMonth && this.selectedIsLeapMonth === oldIsLeap) {
        return;
      }
      
      // Adjust selectedDay BEFORE generating day array
      const lunarDays: int = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
      let startLunarDayIndex: int = DatePickerConstant.MIN_DAY;
      let endLunarDayIndex: int = lunarDays;
      
      if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
          !this.selectedIsLeapMonth) {
        startLunarDayIndex = this.lunarStartDay;
      }
      if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
          !this.selectedIsLeapMonth) {
        endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays).toInt();
      }
      
      if (this.selectedDay < startLunarDayIndex) {
        this.selectedDay = startLunarDayIndex;
      }
      if (this.selectedDay > endLunarDayIndex) {
        this.selectedDay = endLunarDayIndex;
      }
      
      // Generate day array without adjusting selectedDay
      this.dayArray = [];
      for (let i: int = startLunarDayIndex; i <= endLunarDayIndex; i++) {
        this.dayArray.push(this.formatLunarDay(i));
      }
    } else {
      let startMonthIndex: int = DatePickerConstant.MIN_MONTH;
      if (this.selectedYear === this.startYear) {
        startMonthIndex = this.startMonth;
      }
      this.selectedMonth = startMonthIndex + selectedIndex;
      this.updateDaysArray();
      }
     this.updateDateColumnLoopState();
    if (this.lunar) {
      setTimeout(() => { this.dateOnChange(this.getResult()); }, 0);
    } else {
      this.dateOnChange(this.getResult());
    }
   }
   
   private onDayChange(selectedIndex: int): void {
    if (this.lunar) {
      let startLunarDayIndex: int = DatePickerConstant.MIN_DAY;
      if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
          !this.selectedIsLeapMonth) {
        startLunarDayIndex = this.lunarStartDay;
      }
      const newDay: int = startLunarDayIndex + selectedIndex;
      if (newDay === this.selectedDay) {
        return;
      }
      this.selectedDay = newDay;
      
      // Ensure selectedDay doesn't exceed lunar days in month
      const lunarDays: int = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
      if (this.selectedDay > lunarDays) {
        this.selectedDay = lunarDays;
      }
    } else {
      let startDayIndex: int = DatePickerConstant.MIN_DAY;
      if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
        startDayIndex = this.startDay;
      }
      this.selectedDay = startDayIndex + selectedIndex;
    }
    if (this.lunar) {
      setTimeout(() => { this.dateOnChange(this.getResult()); }, 0);
    } else {
      this.dateOnChange(this.getResult());
    }
  }

    private onHourChange(selectedIndex: int): void {
      if (this.useMilitaryTime) {
        this.selectedHour = this.startHour + selectedIndex;
      } else {
        const oldDisplayHour: int = this.selectedHour === 0 || this.selectedHour === 12 ? 12 : 
                                  (this.selectedHour > 12 ? this.selectedHour - 12 : this.selectedHour);
        
        // Calculate the actual display hour from selectedIndex using the SAME logic as getHourSelectedIndex()
        // This ensures bidirectional consistency
        const isAM: boolean = this.selectedPeriod === 0;
        let newDisplayHour: int = 12; // default fallback
        
        // Find displayHour by counting valid hours (same as getHourSelectedIndex logic)
        let validIndex: int = 0;
        if (isAM) {
          // AM period: check 12 AM (hour24=0) and 1-11 AM (hour24=1-11)
          if (0 >= this.startHour && 0 <= this.endHour) {
            if (validIndex === selectedIndex) {
              newDisplayHour = 12;
            }
            validIndex++;
          }
          for (let h: int = 1; h <= 11; h++) {
            if (h >= this.startHour && h <= this.endHour) {
              if (validIndex === selectedIndex) {
                newDisplayHour = h;
              }
              validIndex++;
            }
          }
        } else {
          // PM period: check 12 PM (hour24=12) and 1-11 PM (hour24=13-23)
          if (12 >= this.startHour && 12 <= this.endHour) {
            if (validIndex === selectedIndex) {
              newDisplayHour = 12;
            }
            validIndex++;
          }
          for (let h: int = 13; h <= 23; h++) {
            if (h >= this.startHour && h <= this.endHour) {
              if (validIndex === selectedIndex) {
                newDisplayHour = h === 12 ? 12 : h - 12;
              }
              validIndex++;
            }
          }
        }
        
        // Crossing 11↔12 always triggers period toggle
        const crossingBoundary: boolean = (oldDisplayHour === 11 && newDisplayHour === 12) || 
                                         (oldDisplayHour === 12 && newDisplayHour === 11);
        
        if (crossingBoundary) {
          // Toggle period
          this.selectedPeriod = this.selectedPeriod === 0 ? 1 : 0;
        }
        
        // Convert display hour to 24-hour format based on CURRENT period (after toggle)
        if (newDisplayHour === 12) {
          if (this.selectedPeriod === 0) {
            this.selectedHour = 0; // 12 AM = 0:00 (24-hour)
          } else {
            this.selectedHour = 12; // 12 PM = 12:00 (24-hour)
          }
        } else {
          if (this.selectedPeriod === 0) {
            this.selectedHour = newDisplayHour; // 1-11 AM
          } else {
            this.selectedHour = newDisplayHour + 12; // 1-11 PM = 13-23
          }
        }
      }
      this.updateTimeArrays();
      this.updateTimeColumnLoopState();
      this.timeOnChange(this.getResult());
    }
  
  private onPeriodChange(selectedIndex: int): void {
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
        const minHour = Math.max(0, this.startHour).toInt();
        const maxHour = Math.min(11, this.endHour < 12 ? this.endHour : 11).toInt();
        
        // If current hour is not in AM range, adjust to within range
        if (this.selectedHour >= 12 || this.selectedHour < minHour) {
          this.selectedHour = minHour;
        }
        if (this.selectedHour > maxHour) {
          this.selectedHour = maxHour;
        }
      } else {
        // Switch to PM period
        const minHour = Math.max(12, this.startHour >= 12 ? this.startHour : 12).toInt();
        const maxHour = Math.min(23, this.endHour).toInt();
        
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
    this.timeOnChange(this.getResult());
  }
  
  private onMinuteChange(selectedIndex: int): void {
    let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
    if (this.selectedHour === this.startHour) {
      startMinuteIndex = this.startMinute;
    }
    this.selectedMinute = startMinuteIndex + selectedIndex;
    this.updateSecondArray();
    this.updateTimeColumnLoopState();
    this.timeOnChange(this.getResult());
  }
  
  private onSecondChange(selectedIndex: int): void {
    let startSecondIndex: int = DatePickerConstant.MIN_SECOND;
    if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
      startSecondIndex = this.startSecond;
    }
    this.selectedSecond = startSecondIndex + selectedIndex;
    this.timeOnChange(this.getResult());
  }
  
  private updateTimeArrays(): void {
    // Optimize: only update arrays when range actually changes
    this.minuteArray = [];
    let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
    let endMinuteIndex: int = DatePickerConstant.MAX_MINUTE;
    
    if (this.selectedHour === this.startHour) {
      startMinuteIndex = this.startMinute;
    }
    if (this.selectedHour === this.endHour) {
      endMinuteIndex = this.endMinute;
    }
    
    // Use pre-allocated array with direct assignment for better performance
    const minuteCount: int = endMinuteIndex - startMinuteIndex + 1;
    for (let i: int = 0; i < minuteCount; i++) {
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
  
  private updateSecondArray(): void {
    this.secondArray = [];
    let startSecondIndex: int = DatePickerConstant.MIN_SECOND;
    let endSecondIndex: int = DatePickerConstant.MAX_SECOND;
    
    if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
      startSecondIndex = this.startSecond;
    }
    if (this.selectedHour === this.endHour && this.selectedMinute === this.endMinute) {
      endSecondIndex = this.endSecond;
    }
    
    // Use pre-allocated array with direct assignment for better performance
    const secondCount: int = endSecondIndex - startSecondIndex + 1;
    for (let i: int = 0; i < secondCount; i++) {
      this.secondArray.push(this.formatSecond(startSecondIndex + i));
    }
    
    if (this.selectedSecond < startSecondIndex) {
      this.selectedSecond = startSecondIndex;
    }
    if (this.selectedSecond > endSecondIndex) {
      this.selectedSecond = endSecondIndex;
    }
  }
  
  private updateDaysArray(): void {
    if (this.lunar) {
      const lunarDays: int = this.getLunarDaysInMonth(this.selectedYear, this.selectedMonth, this.selectedIsLeapMonth);
      
      let startLunarDayIndex: int = DatePickerConstant.MIN_DAY;
      let endLunarDayIndex: int = lunarDays;
      
      if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
          !this.selectedIsLeapMonth) {
        startLunarDayIndex = this.lunarStartDay;
      }
      if (this.selectedYear === this.lunarEndYear && this.selectedMonth === this.lunarEndMonth &&
          !this.selectedIsLeapMonth) {
        endLunarDayIndex = Math.min(this.lunarEndDay, lunarDays).toInt();
      }
      
      if (this.selectedDay < startLunarDayIndex) {
        this.selectedDay = startLunarDayIndex;
      }
      if (this.selectedDay > endLunarDayIndex) {
        this.selectedDay = endLunarDayIndex;
      }
      
      this.initLunarDayArray();
    } else {
      const daysInMonth: int = this.getDaysInMonth(this.selectedYear, this.selectedMonth);
      
      // Determine the valid day range based on selected year and month
      let startDayIndex: int = DatePickerConstant.MIN_DAY;
      let endDayIndex: int = daysInMonth;
      
      if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
        startDayIndex = this.startDay;
      }
      if (this.selectedYear === this.endYear && this.selectedMonth === this.endMonth) {
        endDayIndex = Math.min(this.endDay, daysInMonth).toInt();
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
  
  private getYearSelectedIndex(): int {
    if (this.selectedYear === DatePickerConstant.HIDE_YEAR_SENTINEL) {
      return this.yearArray.length - 1;
    }
    const startYear: int = this.lunar ? this.lunarStartYear : this.startYear;
    return this.selectedYear - startYear;
  }
  
  private getMonthSelectedIndex(): int {
    if (this.lunar) {
      let startLunarMonthIndex: int = DatePickerConstant.MIN_MONTH;
      if (this.selectedYear === this.lunarStartYear) {
        startLunarMonthIndex = this.lunarStartMonth;
      }
      // Encode (month, isLeap) to array index, accounting for leap month insertion
      const leapMonth: int = this.getLunarLeapMonth(this.selectedYear);
      let index: int = this.selectedMonth - startLunarMonthIndex;
      if (leapMonth >= 0 && leapMonth >= startLunarMonthIndex) {
        if (this.selectedIsLeapMonth && this.selectedMonth === leapMonth) {
          // Leap month is right after the regular month
          index = (leapMonth - startLunarMonthIndex) + 1;
        } else if (this.selectedMonth > leapMonth) {
          // After the leap month insertion, add 1 to skip the extra entry
          index += 1;
        }
      }
      return index;
    } else {
      let startMonthIndex: int = DatePickerConstant.MIN_MONTH;
      if (this.selectedYear === this.startYear) {
        startMonthIndex = this.startMonth;
      }
      return this.selectedMonth - startMonthIndex;
    }
  }
  
  private getDaySelectedIndex(): int {
    if (this.lunar) {
      let startLunarDayIndex: int = DatePickerConstant.MIN_DAY;
      if (this.selectedYear === this.lunarStartYear && this.selectedMonth === this.lunarStartMonth &&
          !this.selectedIsLeapMonth) {
        startLunarDayIndex = this.lunarStartDay;
      }
      return this.selectedDay - startLunarDayIndex;
    } else {
      let startDayIndex: int = DatePickerConstant.MIN_DAY;
      if (this.selectedYear === this.startYear && this.selectedMonth === this.startMonth) {
        startDayIndex = this.startDay;
      }
      return this.selectedDay - startDayIndex;
    }
  }
  
  private getHourSelectedIndex(): int {
    if (this.useMilitaryTime) {
      return this.selectedHour - this.startHour;
    } else {
      // For 12-hour format, calculate index directly based on hourArray generation logic
      // hourArray structure:
      // - AM period: [12, 1, 2, ..., 11] (if in range)
      // - PM period: [12, 1, 2, ..., 11] (if in range)
      
      const isAM: boolean = this.selectedPeriod === 0;
      const displayHour: int = this.selectedHour === 0 || this.selectedHour === 12 ? 12 : 
                              (this.selectedHour > 12 ? this.selectedHour - 12 : this.selectedHour);
      
      // Calculate index by matching display hour to hourArray structure
      // hourArray[0] = 12, hourArray[1] = 1, hourArray[2] = 2, ..., hourArray[11] = 11
      let targetIndex: int = 0;
      
      if (displayHour === 12) {
        targetIndex = 0;
      } else {
        targetIndex = displayHour;
      }
      
      // Ensure the targetIndex is within valid hourArray bounds
      // Need to verify this index exists in hourArray based on actual time range
      const actualHour24: int = isAM ? (displayHour === 12 ? 0 : displayHour) : 
                                    (displayHour === 12 ? 12 : displayHour + 12);
      
      // Check if this hour is actually in hourArray
      if (actualHour24 >= this.startHour && actualHour24 <= this.endHour) {
        // Find the actual index in hourArray by counting valid hours
        let validIndex: int = 0;
        if (isAM) {
          // AM period: check 12 AM (hour24=0) and 1-11 AM (hour24=1-11)
          if (0 >= this.startHour && 0 <= this.endHour) {
            if (displayHour === 12) {
              return validIndex;
            }
            validIndex++;
          }
          for (let h: int = 1; h <= 11; h++) {
            if (h >= this.startHour && h <= this.endHour) {
              if (h === displayHour) {
                return validIndex;
              }
              validIndex++;
            }
          }
        } else {
          // PM period: check 12 PM (hour24=12) and 1-11 PM (hour24=13-23)
          if (12 >= this.startHour && 12 <= this.endHour) {
            if (displayHour === 12) {
              return validIndex;
            }
            validIndex++;
          }
          for (let h: int = 13; h <= 23; h++) {
            if (h >= this.startHour && h <= this.endHour) {
              const hDisplay: int = h === 12 ? 12 : h - 12;
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
  
  private getMinuteSelectedIndex(): int {
    let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
    if (this.selectedHour === this.startHour) {
      startMinuteIndex = this.startMinute;
    }
    return this.selectedMinute - startMinuteIndex;
  }
  
  private getSecondSelectedIndex(): int {
    let startSecondIndex: int = DatePickerConstant.MIN_SECOND;
    if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
      startSecondIndex = this.startSecond;
    }
    return this.selectedSecond - startSecondIndex;
  }

  private onHideYearChange(propName?: string): void {
    if (this.lunar) {
      this.initLunarArrays();
    } else {
      this.initYearArray();
      this.initMonthArray();
      this.updateDaysArray();
    }
    this.updateDateColumnLoopState();
  }

  private updateDateColumnLoopState(): void {
    if (!this.dateStartEndSet || !this.dateCanLoop) {
      const loop = this.dateCanLoop;
      this.yearColumnCanLoop = loop;
      this.monthColumnCanLoop = loop;
      this.dayColumnCanLoop = loop;
      return;
    }

    this.yearColumnCanLoop = false;
    const atStartYear: boolean = this.lunar ?
        (this.selectedYear === this.lunarStartYear) : (this.selectedYear === this.startYear);
    const atEndYear: boolean = this.lunar ?
        (this.selectedYear === this.lunarEndYear) : (this.selectedYear === this.endYear);
    this.monthColumnCanLoop = !atStartYear && !atEndYear;
    if (this.lunar) {
      const atStartMonth: boolean = atStartYear && this.selectedMonth === this.lunarStartMonth &&
          !this.selectedIsLeapMonth;
      const atEndMonth: boolean = atEndYear && this.selectedMonth === this.lunarEndMonth &&
          !this.selectedIsLeapMonth;
      this.dayColumnCanLoop = !atStartMonth && !atEndMonth;
    } else {
      const atStartMonth: boolean = atStartYear && this.selectedMonth === this.startMonth;
      const atEndMonth: boolean = atEndYear && this.selectedMonth === this.endMonth;
      this.dayColumnCanLoop = !atStartMonth && !atEndMonth;
    }
  }

  private updateTimeColumnLoopState(): void {
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

  private isRTL(): boolean {
    return i18n.isRTL(this.localeID) ?? false;
  }
  
  private getDateColumnCount(): number {
    if (this.dateMode === DateMode.DATE) {
      return 3;
    } else {
      return 2;
    }
  }
  
  private getTimeColumnCount(): number {
    if (this.timeFormat === TimeFormat.HOUR_MINUTE) {
      return this.useMilitaryTime ? 2 : 3;
    } else {
      return this.useMilitaryTime ? 3 : 4;
    }
  }
  
  private getColumnCount(): number {
    let columnCount: number = 0;
    
    if (this.displayMode === DisplayMode.DATE) {
      columnCount = this.getDateColumnCount();
    } else if (this.displayMode === DisplayMode.TIME) {
      columnCount = this.getTimeColumnCount();
    } else {
      columnCount = this.getDateColumnCount() + this.getTimeColumnCount();
    }
    
    return columnCount;
  }
  
  private getFontSize(): string {
    const columnCount: number = this.getColumnCount();
    return columnCount >= 6 ? '14vp' : '16vp';
  }
  
  private getDatePartRatio(): number {
    const timeColumns: number = this.getTimeColumnCount();
    const dateColumns: number = this.getDateColumnCount();
    if (timeColumns === 4) {
      if (dateColumns === 3) {
        return 1;
      } else {
        return 2;
      }
    } else if (timeColumns === 2) {
      if (dateColumns === 3) {
        return 3;
      } else {
        return 1;
      }
    } else {
      return 1;
    }
  }
  
  private getTimePartRatio(): number {
    const timeColumns: number = this.getTimeColumnCount();
    const dateColumns: number = this.getDateColumnCount();
    if (timeColumns === 4) {
      if (dateColumns === 3) {
        return 1;
      } else {
        return 3;
      }
    } else if (timeColumns === 2) {
      if (dateColumns === 3) {
        return 2;
      } else {
        return 1;
      }
    } else {
      return 1;
    }
  }
  
  private getDateColumnWidth(columnIndex: number): string {
    const inDateTimeMode: boolean = this.displayMode === DisplayMode.DATE_TIME;
    let parts: number[] = [];
    
    if (this.dateMode === DateMode.DATE) {
      parts = [3, 2, 2];
    } else if (this.dateMode === DateMode.YEAR_AND_MONTH) {
      parts = [3, 2];
    } else {
      parts = [1, 1];
    }
    
    let totalParts: number = 0;
    for (let i: int = 0; i < parts.length; i++) {
      totalParts += parts[i];
    }
    const columnPart: number = parts[columnIndex.toInt()];
    
    if (inDateTimeMode) {
      const dateRatio: number = this.getDatePartRatio();
      const timeRatio: number = this.getTimePartRatio();
      const totalRatio: number = dateRatio + timeRatio;
      const percent: number = (dateRatio / totalRatio) * (columnPart / totalParts) * 100;
      return percent.toFixed(2) + '%';
    } else {
      const percent: number = (columnPart / totalParts) * 100;
      return percent.toFixed(2) + '%';
    }
  }
  
  private getTimeColumnWidth(columnIndex: number): string {
    const inDateTimeMode: boolean = this.displayMode === DisplayMode.DATE_TIME;
    const totalColumns: number = this.getTimeColumnCount();
    
    if (inDateTimeMode) {
      const dateRatio: number = this.getDatePartRatio();
      const timeRatio: number = this.getTimePartRatio();
      const totalRatio: number = dateRatio + timeRatio;
      const percent: number = (timeRatio / totalRatio) * (1 / totalColumns) * 100;
      return percent.toFixed(2) + '%';
    } else {
      const percent: number = (1 / totalColumns) * 100;
      return percent.toFixed(2) + '%';
    }
  }
  
  private getFirstPickerIndicator(radius: int = 20): PickerIndicatorStyle {
    if (this.isRTL()) {
      return {
        type: PickerIndicatorType.BACKGROUND,
        borderRadius: {
          topLeft: 0,
          topRight: radius,
          bottomLeft: 0,
          bottomRight: radius
        } as BorderRadiuses
      };
    }
    return {
      type: PickerIndicatorType.BACKGROUND,
      borderRadius: {
        topLeft: radius,
        topRight: 0,
        bottomLeft: radius,
        bottomRight: 0
      } as BorderRadiuses
    };
  }
  
  private getMiddlePickerIndicator(): PickerIndicatorStyle {
    return {
      type: PickerIndicatorType.BACKGROUND,
      borderRadius: {
        topLeft: 0,
        topRight: 0,
        bottomLeft: 0,
        bottomRight: 0
      } as BorderRadiuses
    };
  }
  
  private getLastPickerIndicator(radius: int = 20): PickerIndicatorStyle {
    if (this.isRTL()) {
      return {
        type: PickerIndicatorType.BACKGROUND,
        borderRadius: {
          topLeft: radius,
          topRight: 0,
          bottomLeft: radius,
          bottomRight: 0
        } as BorderRadiuses
      };
    }
    return {
      type: PickerIndicatorType.BACKGROUND,
      borderRadius: {
        topLeft: 0,
        topRight: radius,
        bottomLeft: 0,
        bottomRight: radius
      } as BorderRadiuses
    };
  }
  
  private getAdjustedRadius(): int {
    const minColumnWidth: number = Math.min(this.firstColumnWidth, this.lastColumnWidth);
    if (minColumnWidth <= 0) {
      return 20;
    }
    const calculatedRadius: number = Math.min(minColumnWidth / 2, 20);
    return Math.max(Math.floor(calculatedRadius), 0).toInt();
  }

  private textStyleEqual(a: PickerComponentTextStyle | undefined, b: PickerComponentTextStyle | undefined): boolean {
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
    const aFont: Font | undefined = a.font;
    const bFont: Font | undefined = b.font;
    if (!aFont || !bFont) {
      return false;
    }
    if (aFont.size !== bFont.size) {
      return false;
    }
    if (aFont.weight !== bFont.weight) {
      return false;
    }
    if (aFont.family !== bFont.family) {
      return false;
    }
    if (aFont.style !== bFont.style) {
      return false;
    }
    return true;
  }

  private lengthMetricsEqual(a: LengthMetrics | undefined, b: LengthMetrics | undefined): boolean {
    if (a === b) {
      return true;
    }
    if (!a || !b) {
      return false;
    }
    return a.value === b.value && a.unit === b.unit;
  }

  private borderRadiusEqual(a: LengthMetrics | BorderRadiuses | LocalizedBorderRadiuses | undefined,
                            b: LengthMetrics | BorderRadiuses | LocalizedBorderRadiuses | undefined): boolean {
    if (a === b) {
      return true;
    }
    if (!a || !b) {
      return false;
    }
    const lmA: LengthMetrics = a as LengthMetrics;
    const lmB: LengthMetrics = b as LengthMetrics;
    if (lmA.value !== undefined || lmB.value !== undefined) {
      if (lmA.value === undefined || lmB.value === undefined) {
        return false;
      }
      return lmA.value === lmB.value && lmA.unit === lmB.unit;
    }
    const lrA: LocalizedBorderRadiuses = a as LocalizedBorderRadiuses;
    const lrB: LocalizedBorderRadiuses = b as LocalizedBorderRadiuses;
    if (lrA.topStart !== undefined || lrA.topEnd !== undefined ||
        lrA.bottomStart !== undefined || lrA.bottomEnd !== undefined ||
        lrB.topStart !== undefined || lrB.topEnd !== undefined ||
        lrB.bottomStart !== undefined || lrB.bottomEnd !== undefined) {
      return this.lengthMetricsEqual(lrA.topStart, lrB.topStart) &&
             this.lengthMetricsEqual(lrA.topEnd, lrB.topEnd) &&
             this.lengthMetricsEqual(lrA.bottomStart, lrB.bottomStart) &&
             this.lengthMetricsEqual(lrA.bottomEnd, lrB.bottomEnd);
    }
    const rbA: BorderRadiuses = a as BorderRadiuses;
    const rbB: BorderRadiuses = b as BorderRadiuses;
    return rbA.topLeft === rbB.topLeft &&
           rbA.topRight === rbB.topRight &&
           rbA.bottomLeft === rbB.bottomLeft &&
           rbA.bottomRight === rbB.bottomRight;
  }

  private indicatorStyleEqual(a: PickerIndicatorStyle | undefined, b: PickerIndicatorStyle | undefined): boolean {
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

  private lengthMetricsToPx(lm: LengthMetrics): number {
    if (lm.unit === LengthUnit.PX) {
      return lm.value;
    } else if (lm.unit === LengthUnit.VP) {
      return this.getUIContext().vp2px(lm.value);
    } else if (lm.unit === LengthUnit.FP) {
      return this.getUIContext().fp2px(lm.value);
    } else if (lm.unit === LengthUnit.LPX) {
      return this.getUIContext().lpx2px(lm.value);
    }
    return lm.value;
  }

  private getColumnWidthPercent(overallIndex: number): number {
    const dateCount: number = this.displayMode === DisplayMode.TIME ? 0 : this.getDateColumnCount();
    if (this.displayMode === DisplayMode.DATE) {
      return parseFloat(this.getDateColumnWidth(overallIndex));
    } else if (this.displayMode === DisplayMode.TIME) {
      return parseFloat(this.getTimeColumnWidth(overallIndex));
    } else {
      if (overallIndex < dateCount) {
        return parseFloat(this.getDateColumnWidth(overallIndex));
      } else {
        return parseFloat(this.getTimeColumnWidth(overallIndex - dateCount));
      }
    }
  }

  private getTotalWidthPx(): number {
    if (this.firstColumnWidth <= 0) {
      return 0;
    }
    const firstPercent: number = this.getColumnWidthPercent(0);
    if (firstPercent <= 0) {
      return 0;
    }
    return this.getUIContext().vp2px(this.firstColumnWidth * 100 / firstPercent);
  }

  private getColumnWidthPx(columnIndex: number): number {
    const totalWidth: number = this.getTotalWidthPx();
    if (totalWidth <= 0) {
      return 0;
    }
    return totalWidth * this.getColumnWidthPercent(columnIndex) / 100;
  }

  private getPrecedingWidthPx(columnIndex: number): number {
    let preceding: number = 0;
    for (let i: int = 0; i < columnIndex; i++) {
      preceding += this.getColumnWidthPx(i);
    }
    return preceding;
  }

  private getFollowingWidthPx(columnIndex: number): number {
    const totalColumns: number = this.getColumnCount();
    let following: number = 0;
    for (let i: number = columnIndex + 1; i < totalColumns; i++) {
      following += this.getColumnWidthPx(i);
    }
    return following;
  }

  private getTimeStartIndex(): number {
    return this.displayMode === DisplayMode.DATE_TIME ? this.getDateColumnCount() : 0 as number;
  }

  private adjustSelectionIndicatorForPosition(columnIndex: number): PickerIndicatorStyle {
    const totalColumns: number = this.getColumnCount();
    const isFirst: boolean = columnIndex === 0;
    const isLast: boolean = columnIndex === totalColumns - 1;
    if (!this.selectionIndicator) {
      if (isFirst) {
        return this.getFirstPickerIndicator(this.getAdjustedRadius());
      } else if (isLast) {
        return this.getLastPickerIndicator(this.getAdjustedRadius());
      }
      return this.getMiddlePickerIndicator();
    }
    const indicator: PickerIndicatorStyle = this.selectionIndicator!;
    const validType: PickerIndicatorType = (indicator.type === PickerIndicatorType.BACKGROUND ||
                       indicator.type === PickerIndicatorType.DIVIDER)
                      ? indicator.type : PickerIndicatorType.BACKGROUND;
    const userBR: LengthMetrics | BorderRadiuses | LocalizedBorderRadiuses | undefined = indicator.borderRadius;
    const defaultRadius: number = this.getAdjustedRadius();
    let tl: number = defaultRadius;
    let tr: number = defaultRadius;
    let bl: number = defaultRadius;
    let br: number = defaultRadius;
    if (userBR) {
      if (userBR instanceof LengthMetrics) {
        tl = tr = bl = br = userBR.value;
      } else {
        const brStr: string = JSON.stringify(userBR);
        if (brStr.indexOf('"topStart"') >= 0) {
          const lr: LocalizedBorderRadiuses = userBR as LocalizedBorderRadiuses;
          const lrTopStart: LengthMetrics | undefined = lr.topStart;
          const lrTopEnd: LengthMetrics | undefined = lr.topEnd;
          const lrBottomStart: LengthMetrics | undefined = lr.bottomStart;
          const lrBottomEnd: LengthMetrics | undefined = lr.bottomEnd;
          if (this.isRTL()) {
            tl = lrTopEnd ? lrTopEnd.value : defaultRadius;
            tr = lrTopStart ? lrTopStart.value : defaultRadius;
            bl = lrBottomEnd ? lrBottomEnd.value : defaultRadius;
            br = lrBottomStart ? lrBottomStart.value : defaultRadius;
          } else {
            tl = lrTopStart ? lrTopStart.value : defaultRadius;
            tr = lrTopEnd ? lrTopEnd.value : defaultRadius;
            bl = lrBottomStart ? lrBottomStart.value : defaultRadius;
            br = lrBottomEnd ? lrBottomEnd.value : defaultRadius;
          }
        } else if (brStr.indexOf('"topLeft"') >= 0) {
          const rb: BorderRadiuses = userBR as BorderRadiuses;
          tl = rb.topLeft !== undefined ? rb.topLeft as number : defaultRadius;
          tr = rb.topRight !== undefined ? rb.topRight as number : defaultRadius;
          bl = rb.bottomLeft !== undefined ? rb.bottomLeft as number : defaultRadius;
          br = rb.bottomRight !== undefined ? rb.bottomRight as number : defaultRadius;
        }
      }
    }
    if (!isFirst && !isLast) {
      tl = tr = bl = br = 0;
    } else if (isFirst) {
      if (this.isRTL()) { tl = 0; bl = 0; } else { tr = 0; br = 0; }
    } else {
      if (this.isRTL()) { tr = 0; br = 0; } else { tl = 0; bl = 0; }
    }
    let adjustedStartMargin: LengthMetrics | undefined = indicator.startMargin;
    let adjustedEndMargin: LengthMetrics | undefined = indicator.endMargin;
    let adjustedStrokeWidth: LengthMetrics | undefined = indicator.strokeWidth;
    const totalWidthPx: number = this.getTotalWidthPx();
    const currentColumnWidthPx: number = totalWidthPx > 0 ? this.getColumnWidthPx(columnIndex) : 0;
    if (totalWidthPx > 0 && currentColumnWidthPx > 0) {
      let rawStartPx: number = 0;
      let rawEndPx: number = 0;
      const startMargin: LengthMetrics | undefined = indicator.startMargin;
      if (startMargin) {
        rawStartPx = Math.max(0, this.lengthMetricsToPx(startMargin));
      }
      const endMargin: LengthMetrics | undefined = indicator.endMargin;
      if (endMargin) {
        rawEndPx = Math.max(0, this.lengthMetricsToPx(endMargin));
      }
      if (rawStartPx + rawEndPx > totalWidthPx) {
        adjustedStartMargin = undefined;
        adjustedEndMargin = undefined;
      } else {
        const precedingWidthPx: number = this.getPrecedingWidthPx(columnIndex);
        const followingWidthPx: number = this.getFollowingWidthPx(columnIndex);
        const remainingStart: number = Math.max(0, rawStartPx - precedingWidthPx);
        const remainingEnd: number = Math.max(0, rawEndPx - followingWidthPx);
        const cappedStart: number = Math.min(remainingStart, currentColumnWidthPx);
        adjustedStartMargin = cappedStart > 0 ? LengthMetrics.px(cappedStart) : undefined;
        const cappedEnd: number = Math.min(remainingEnd, currentColumnWidthPx);
        adjustedEndMargin = cappedEnd > 0 ? LengthMetrics.px(cappedEnd) : undefined;
      }
    } else {
      adjustedStartMargin = undefined;
      adjustedEndMargin = undefined;
    }
    return {
      type: validType,
      borderRadius: {
        topStart: LengthMetrics.vp(tl),
        topEnd: LengthMetrics.vp(tr),
        bottomStart: LengthMetrics.vp(bl),
        bottomEnd: LengthMetrics.vp(br)
      } as LocalizedBorderRadiuses,
      strokeWidth: adjustedStrokeWidth,
      dividerColor: indicator.dividerColor,
      startMargin: adjustedStartMargin,
      endMargin: adjustedEndMargin,
      backgroundColor: indicator.backgroundColor
    } as PickerIndicatorStyle;
  }

  @Builder
  DateModeDateColumns() {
    UIPickerComponent({ selectedIndex: this.getYearSelectedIndex() }) {
      ForEach(this.yearArray, (item: string) => {
        Text(item)
          .maxLines(1)
          .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
          .fontColor(this.itemTextStyle?.color)
          .fontWeight(this.itemTextStyle?.font?.weight)
          .minFontSize(this.itemTextStyle?.minFontSize)
          .maxFontSize(this.itemTextStyle?.maxFontSize)
          .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
      })
    }
    .width(this.getDateColumnWidth(0))
    .height(200)
    .selectedTextStyle(this.selectedTextStyle)
    .canLoop(this.yearColumnCanLoop)
    .enableHapticFeedback(this.dateHapticFeedback)
    .selectionIndicator(this.adjustSelectionIndicatorForPosition(0))
    .onChange((selectedIndex: int) => {
      this.onYearChange(selectedIndex);
    })
    .onScrollStop((selectedIndex: int) => {
      this.dateOnScrollStop(this.getResult());
    })
    .onAreaChange((oldValue: Area, newValue: Area) => {
      const newWidth: number = newValue.width as number;
      if (this.firstColumnWidth !== newWidth) {
        this.firstColumnWidth = newWidth;
      }
    })

    UIPickerComponent({ selectedIndex: this.getMonthSelectedIndex() }) {
      ForEach(this.monthArray, (item: string) => {
        Text(item)
          .maxLines(1)
          .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
          .fontColor(this.itemTextStyle?.color)
          .fontWeight(this.itemTextStyle?.font?.weight)
          .minFontSize(this.itemTextStyle?.minFontSize)
          .maxFontSize(this.itemTextStyle?.maxFontSize)
          .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
      })
    }
    .width(this.getDateColumnWidth(1))
    .height(200)
    .selectedTextStyle(this.selectedTextStyle)
    .canLoop(this.monthColumnCanLoop)
    .enableHapticFeedback(this.dateHapticFeedback)
    .selectionIndicator(this.adjustSelectionIndicatorForPosition(1))
    .onChange((selectedIndex: int) => {
      this.onMonthChange(selectedIndex);
    })
    .onScrollStop((selectedIndex: int) => {
      this.dateOnScrollStop(this.getResult());
    })

    UIPickerComponent({ selectedIndex: this.getDaySelectedIndex() }) {
      ForEach(this.dayArray, (item: string) => {
        Text(item)
          .maxLines(1)
          .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
          .fontColor(this.itemTextStyle?.color)
          .fontWeight(this.itemTextStyle?.font?.weight)
          .minFontSize(this.itemTextStyle?.minFontSize)
          .maxFontSize(this.itemTextStyle?.maxFontSize)
          .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
      })
    }
    .width(this.getDateColumnWidth(2))
    .height(200)
    .selectedTextStyle(this.selectedTextStyle)
    .canLoop(this.dayColumnCanLoop)
    .enableHapticFeedback(this.dateHapticFeedback)
    .selectionIndicator(this.adjustSelectionIndicatorForPosition(2))
    .onChange((selectedIndex: int) => {
      this.onDayChange(selectedIndex);
    })
    .onScrollStop((selectedIndex: int) => {
      this.dateOnScrollStop(this.getResult());
    })
    .onAreaChange((oldValue: Area, newValue: Area) => {
      if (this.displayMode !== DisplayMode.DATE_TIME) {
        const newWidth: number = newValue.width as number;
        if (this.lastColumnWidth !== newWidth) {
          this.lastColumnWidth = newWidth;
        }
      }
    })
  }

  @Builder
  DateModeYearMonthColumns() {
    UIPickerComponent({ selectedIndex: this.getYearSelectedIndex() }) {
      ForEach(this.yearArray, (item: string) => {
        Text(item)
          .maxLines(1)
          .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
          .fontColor(this.itemTextStyle?.color)
          .fontWeight(this.itemTextStyle?.font?.weight)
          .minFontSize(this.itemTextStyle?.minFontSize)
          .maxFontSize(this.itemTextStyle?.maxFontSize)
          .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
      })
    }
    .width(this.getDateColumnWidth(0))
    .height(200)
    .selectedTextStyle(this.selectedTextStyle)
    .canLoop(this.yearColumnCanLoop)
    .enableHapticFeedback(this.dateHapticFeedback)
    .selectionIndicator(this.adjustSelectionIndicatorForPosition(0))
    .onChange((selectedIndex: int) => {
      this.onYearChange(selectedIndex);
    })
    .onScrollStop((selectedIndex: int) => {
      this.dateOnScrollStop(this.getResult());
    })
    .onAreaChange((oldValue: Area, newValue: Area) => {
      const newWidth: number = newValue.width as number;
      if (this.firstColumnWidth !== newWidth) {
        this.firstColumnWidth = newWidth;
      }
    })

    UIPickerComponent({ selectedIndex: this.getMonthSelectedIndex() }) {
      ForEach(this.monthArray, (item: string) => {
        Text(item)
          .maxLines(1)
          .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
          .fontColor(this.itemTextStyle?.color)
          .fontWeight(this.itemTextStyle?.font?.weight)
          .minFontSize(this.itemTextStyle?.minFontSize)
          .maxFontSize(this.itemTextStyle?.maxFontSize)
          .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
      })
    }
    .width(this.getDateColumnWidth(1))
    .height(200)
    .selectedTextStyle(this.selectedTextStyle)
    .canLoop(this.monthColumnCanLoop)
    .enableHapticFeedback(this.dateHapticFeedback)
    .selectionIndicator(this.adjustSelectionIndicatorForPosition(1))
    .onChange((selectedIndex: int) => {
      this.onMonthChange(selectedIndex);
    })
    .onScrollStop((selectedIndex: int) => {
      this.dateOnScrollStop(this.getResult());
    })
    .onAreaChange((oldValue: Area, newValue: Area) => {
      if (this.displayMode !== DisplayMode.DATE_TIME) {
        const newWidth: number = newValue.width as number;
        if (this.lastColumnWidth !== newWidth) {
          this.lastColumnWidth = newWidth;
        }
      }
    })
  }

  @Builder
  DateModeMonthDayColumns() {
    UIPickerComponent({ selectedIndex: this.getMonthSelectedIndex() }) {
      ForEach(this.monthArray, (item: string) => {
        Text(item)
          .maxLines(1)
          .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
          .fontColor(this.itemTextStyle?.color)
          .fontWeight(this.itemTextStyle?.font?.weight)
          .minFontSize(this.itemTextStyle?.minFontSize)
          .maxFontSize(this.itemTextStyle?.maxFontSize)
          .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
      })
    }
    .width(this.getDateColumnWidth(0))
    .height(200)
    .selectedTextStyle(this.selectedTextStyle)
    .canLoop(this.monthColumnCanLoop)
    .enableHapticFeedback(this.dateHapticFeedback)
    .selectionIndicator(this.adjustSelectionIndicatorForPosition(0))
    .onChange((selectedIndex: int) => {
      this.onMonthChange(selectedIndex);
    })
    .onScrollStop((selectedIndex: int) => {
      this.dateOnScrollStop(this.getResult());
    })
    .onAreaChange((oldValue: Area, newValue: Area) => {
      const newWidth: number = newValue.width as number;
      if (this.firstColumnWidth !== newWidth) {
        this.firstColumnWidth = newWidth;
      }
    })

    UIPickerComponent({ selectedIndex: this.getDaySelectedIndex() }) {
      ForEach(this.dayArray, (item: string) => {
        Text(item)
          .maxLines(1)
          .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
          .fontColor(this.itemTextStyle?.color)
          .fontWeight(this.itemTextStyle?.font?.weight)
          .minFontSize(this.itemTextStyle?.minFontSize)
          .maxFontSize(this.itemTextStyle?.maxFontSize)
          .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
      })
    }
    .width(this.getDateColumnWidth(1))
    .height(200)
    .selectedTextStyle(this.selectedTextStyle)
    .canLoop(this.dayColumnCanLoop)
    .enableHapticFeedback(this.dateHapticFeedback)
    .selectionIndicator(this.adjustSelectionIndicatorForPosition(1))
    .onChange((selectedIndex: int) => {
      this.onDayChange(selectedIndex);
    })
    .onScrollStop((selectedIndex: int) => {
      this.dateOnScrollStop(this.getResult());
    })
    .onAreaChange((oldValue: Area, newValue: Area) => {
      if (this.displayMode !== DisplayMode.DATE_TIME) {
        const newWidth: number = newValue.width as number;
        if (this.lastColumnWidth !== newWidth) {
          this.lastColumnWidth = newWidth;
        }
      }
    })
  }

  build() {
    Row() {
      if (this.displayMode === DisplayMode.DATE || this.displayMode === DisplayMode.DATE_TIME) {
        if (this.dateMode === DateMode.DATE) {
          if (this.lunar) {
            this.DateModeDateColumns()
          } else {
            this.DateModeDateColumns()
          }
        } else if (this.dateMode === DateMode.YEAR_AND_MONTH) {
          if (this.lunar) {
            this.DateModeYearMonthColumns()
          } else {
            this.DateModeYearMonthColumns()
          }
        } else if (this.dateMode === DateMode.MONTH_AND_DAY) {
          if (this.lunar) {
            this.DateModeMonthDayColumns()
          } else {
            this.DateModeMonthDayColumns()
          }
        }
      }
      
      if (this.displayMode === DisplayMode.TIME || this.displayMode === DisplayMode.DATE_TIME) {
        if (this.timeFormat === TimeFormat.HOUR_MINUTE) {
          if (this.useMilitaryTime) {
            UIPickerComponent({ selectedIndex: this.getHourSelectedIndex() }) {
              ForEach(this.hourArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(0))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.hourColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()))
            .onChange((selectedIndex: int) => {
              this.onHourChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              this.selectedHour = this.startHour + selectedIndex;
              this.updateTimeArrays();
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              if (this.displayMode === DisplayMode.TIME) {
                const newWidth: number = newValue.width as number;
                if (this.firstColumnWidth !== newWidth) {
                  this.firstColumnWidth = newWidth;
                }
              }
            })
            
            UIPickerComponent({ selectedIndex: this.getMinuteSelectedIndex() }) {
              ForEach(this.minuteArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(1))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.minuteColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1))
            .onChange((selectedIndex: int) => {
              this.onMinuteChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
              if (this.selectedHour === this.startHour) {
                startMinuteIndex = this.startMinute;
              }
              this.selectedMinute = startMinuteIndex + selectedIndex;
              this.updateSecondArray();
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              const newWidth: number = newValue.width as number;
              if (this.lastColumnWidth !== newWidth) {
                this.lastColumnWidth = newWidth;
              }
            })
            
          } else {
            UIPickerComponent({ selectedIndex: this.selectedPeriod }) {
              ForEach(this.periodArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(0))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(false)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()))
            .onChange((selectedIndex: int) => {
              this.onPeriodChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              this.selectedPeriod = selectedIndex;
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              if (this.displayMode === DisplayMode.TIME) {
                const newWidth: number = newValue.width as number;
                if (this.firstColumnWidth !== newWidth) {
                  this.firstColumnWidth = newWidth;
                }
              }
            })
            UIPickerComponent({ selectedIndex: this.getHourSelectedIndex() }) {
              ForEach(this.hourArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(1))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.hourColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1))
            .onChange((selectedIndex: int) => {
              this.onHourChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              this.onHourChange(selectedIndex);
              this.timeOnScrollStop(this.getResult());
            })
            
            UIPickerComponent({ selectedIndex: this.getMinuteSelectedIndex() }) {
              ForEach(this.minuteArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(2))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.minuteColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 2))
            .onChange((selectedIndex: int) => {
              this.onMinuteChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
              if (this.selectedHour === this.startHour) {
                startMinuteIndex = this.startMinute;
              }
              this.selectedMinute = startMinuteIndex + selectedIndex;
              this.updateSecondArray();
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              const newWidth: number = newValue.width as number;
              if (this.lastColumnWidth !== newWidth) {
                this.lastColumnWidth = newWidth;
              }
            })
          }
        } else if (this.timeFormat === TimeFormat.HOUR_MINUTE_SECOND) {
          if (this.useMilitaryTime) {
            UIPickerComponent({ selectedIndex: this.getHourSelectedIndex() }) {
              ForEach(this.hourArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(0))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.hourColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()))
            .onChange((selectedIndex: int) => {
              this.onHourChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              this.selectedHour = this.startHour + selectedIndex;
              this.updateTimeArrays();
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              if (this.displayMode === DisplayMode.TIME) {
                const newWidth: number = newValue.width as number;
                if (this.firstColumnWidth !== newWidth) {
                  this.firstColumnWidth = newWidth;
                }
              }
            })
            
            UIPickerComponent({ selectedIndex: this.getMinuteSelectedIndex() }) {
              ForEach(this.minuteArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(1))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.minuteColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1))
            .onChange((selectedIndex: int) => {
              this.onMinuteChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
              if (this.selectedHour === this.startHour) {
                startMinuteIndex = this.startMinute;
              }
              this.selectedMinute = startMinuteIndex + selectedIndex;
              this.updateSecondArray();
              this.timeOnScrollStop(this.getResult());
            })
            
            UIPickerComponent({ selectedIndex: this.getSecondSelectedIndex() }) {
              ForEach(this.secondArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(2))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.secondColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 2))
            .onChange((selectedIndex: int) => {
              this.onSecondChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              let startSecondIndex: int = DatePickerConstant.MIN_SECOND;
              if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
                startSecondIndex = this.startSecond;
              }
              this.selectedSecond = startSecondIndex + selectedIndex;
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              const newWidth: number = newValue.width as number;
              if (this.lastColumnWidth !== newWidth) {
                this.lastColumnWidth = newWidth;
              }
            })
          } else {
            UIPickerComponent({ selectedIndex: this.selectedPeriod }) {
              ForEach(this.periodArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(0))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(false)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex()))
            .onChange((selectedIndex: int) => {
              this.onPeriodChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              this.selectedPeriod = selectedIndex;
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              if (this.displayMode === DisplayMode.TIME) {
                const newWidth: number = newValue.width as number;
                if (this.firstColumnWidth !== newWidth) {
                  this.firstColumnWidth = newWidth;
                }
              }
            })
            
            UIPickerComponent({ selectedIndex: this.getHourSelectedIndex() }) {
              ForEach(this.hourArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(1))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.hourColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 1))
            .onChange((selectedIndex: int) => {
              this.onHourChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              this.onHourChange(selectedIndex);
              this.timeOnScrollStop(this.getResult());
            })
            
            UIPickerComponent({ selectedIndex: this.getMinuteSelectedIndex() }) {
              ForEach(this.minuteArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(2))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.minuteColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 2))
            .onChange((selectedIndex: int) => {
              this.onMinuteChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              let startMinuteIndex: int = DatePickerConstant.MIN_MINUTE;
              if (this.selectedHour === this.startHour) {
                startMinuteIndex = this.startMinute;
              }
              this.selectedMinute = startMinuteIndex + selectedIndex;
              this.updateSecondArray();
              this.timeOnScrollStop(this.getResult());
            })
            
            UIPickerComponent({ selectedIndex: this.getSecondSelectedIndex() }) {
              ForEach(this.secondArray, (item: string) => {
                Text(item)
                  .maxLines(1)
                  .fontSize(this.itemTextStyle?.font?.size ?? this.getFontSize())
                  .fontColor(this.itemTextStyle?.color)
                  .fontWeight(this.itemTextStyle?.font?.weight)
                  .minFontSize(this.itemTextStyle?.minFontSize)
                  .maxFontSize(this.itemTextStyle?.maxFontSize)
                  .textOverflow({ overflow: this.itemTextStyle?.overflow ?? TextOverflow.Clip })
              })
            }
            .width(this.getTimeColumnWidth(3))
            .height(200)
            .selectedTextStyle(this.selectedTextStyle)
            .canLoop(this.secondColumnCanLoop)
            .enableHapticFeedback(this.timeHapticFeedback)
            .selectionIndicator(this.adjustSelectionIndicatorForPosition(this.getTimeStartIndex() + 3))
            .onChange((selectedIndex: int) => {
              this.onSecondChange(selectedIndex);
            })
            .onScrollStop((selectedIndex: int) => {
              let startSecondIndex: int = DatePickerConstant.MIN_SECOND;
              if (this.selectedHour === this.startHour && this.selectedMinute === this.startMinute) {
                startSecondIndex = this.startSecond;
              }
              this.selectedSecond = startSecondIndex + selectedIndex;
              this.timeOnScrollStop(this.getResult());
            })
            .onAreaChange((oldValue: Area, newValue: Area) => {
              const newWidth: number = newValue.width as number;
              if (this.lastColumnWidth !== newWidth) {
                this.lastColumnWidth = newWidth;
              }
            })
          }
        }
      }
    }
    .width('100%')
  }
}
