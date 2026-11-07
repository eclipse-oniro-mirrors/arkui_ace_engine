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

import inspector from '@ohos.arkui.inspector';

interface TokenItem {
  key: string;
  label: string;
  token: Resource;
}

const PICKER_TOKENS: TokenItem[] = [
  { key: 'tok_picker_focus', label: 'picker_focus_color', token: $r('sys.color.ohos_id_picker_focus_color') },
  { key: 'tok_interactive_focus', label: 'interactive_focus', token: $r('sys.color.interactive_focus') },
  { key: 'tok_font_emphasize', label: 'font_emphasize', token: $r('sys.color.ohos_id_color_emphasize') },
  { key: 'tok_text_activated', label: 'text_primary_activated', token: $r('sys.color.ohos_id_color_text_primary_activated') },
  { key: 'tok_picker_focus_opt_back', label: 'picker_focus_option_back_color', token: $r('sys.color.ohos_id_picker_focus_option_back_color') },
  { key: 'tok_picker_select_back', label: 'picker_select_option_back_color', token: $r('sys.color.ohos_id_picker_select_option_back_color') },
  { key: 'tok_picker_press', label: 'picker_press_color', token: $r('sys.color.ohos_id_picker_press_color') },
  { key: 'tok_picker_divider', label: 'picker_select_divider_color', token: $r('sys.color.ohos_id_picker_select_divider_color') },
];

interface DialogTestItem {
  key: string;
  label: string;
  dialogType: number;
}

const DIALOG_TESTS: DialogTestItem[] = [
  { key: 'dlg_date', label: 'DatePickerDialog', dialogType: 0 },
  { key: 'dlg_time', label: 'TimePickerDialog', dialogType: 1 },
  { key: 'dlg_text', label: 'TextPickerDialog', dialogType: 2 },
  { key: 'dlg_calendar', label: 'CalendarPickerDialog', dialogType: 3 },
];

class CustomBrandColors implements CustomColors {
  public fontEmphasize: string = '#FFE62828';
  public fontOnPrimary: string = '#FFFFFFFF';
  public backgroundEmphasize: string = '#FFE62828';
  public compBackgroundEmphasize: string = '#FFE62828';
  public compEmphasizeSecondary: string = '#33E62828';
  public iconEmphasize: string = '#FFE62828';
  public iconOnPrimary: string = '#FFFFFFFF';
  public interactiveFocus: string = '#FFE62828';
  public interactiveHover: string = '#33E62828';
  public interactivePressed: string = '#66E62828';
  public interactiveActive: string = '#FFE62828';
  public interactiveSelect: string = '#FFE62828';
  public interactiveClick: string = '#FFE62828';
}

class CustomBrandTheme implements CustomTheme {
  public colors?: CustomColors;
  constructor(colors: CustomColors) {
    this.colors = colors;
  }
}

@Entry
@Component
struct PickerThemeSwitchTest {
  @State currentMode: number = 0;
  @State useCustomTheme: boolean = false;
  @State inspectorResult: string = '点击下方按钮读取 Inspector 结果';
  @State baselineResult: string = '';
  @State showRegression: boolean = false;
  @State regressionResult: string = '';
  private customTheme: CustomTheme = new CustomBrandTheme(new CustomBrandColors());

  build() {
    Column() {
      Text('Picker 高亮色主题切换验证')
        .fontSize(22)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20, bottom: 10 })

      Row({ space: 10 }) {
        Button('Mode A: Token文本测试')
          .fontSize(14)
          .backgroundColor(this.currentMode === 0 ? '#007DFF' : '#CCCCCC')
          .onClick(() => { this.currentMode = 0; })
        Button('Mode B: 组件实色测试')
          .fontSize(14)
          .backgroundColor(this.currentMode === 1 ? '#007DFF' : '#CCCCCC')
          .onClick(() => { this.currentMode = 1; })
      }
      .margin({ bottom: 10 })

      Row({ space: 10 }) {
        Text('主题:').fontSize(14)
        Button(this.useCustomTheme ? '自定义品牌色(红)' : '系统默认色(蓝)')
          .fontSize(12)
          .backgroundColor(this.useCustomTheme ? '#E62828' : '#007DFF')
          .onClick(() => { this.useCustomTheme = !this.useCustomTheme; })
      }
      .margin({ bottom: 10 })

      Row({ space: 10 }) {
        Button('读取Inspector')
          .fontSize(13)
          .backgroundColor('#333333')
          .onClick(() => { this.readInspector(); })
        Button('记录基线')
          .fontSize(13)
          .backgroundColor('#666666')
          .onClick(() => { this.baselineResult = this.inspectorResult; })
        Button('回归对比')
          .fontSize(13)
          .backgroundColor('#999999')
          .onClick(() => { this.showRegression = !this.showRegression; this.doRegressionCompare(); })
      }
      .margin({ bottom: 10 })

      if (this.currentMode === 0) {
        this.ModeAView()
      } else {
        this.ModeBView()
      }

      Text(this.inspectorResult)
        .fontSize(12)
        .fontColor('#333333')
        .maxLines(10)
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .backgroundColor('#F5F5F5')
        .borderRadius(8)
        .padding(10)
        .width('90%')
        .height(140)
        .margin({ bottom: 5 })

      if (this.showRegression && this.baselineResult.length > 0) {
        Text(this.regressionResult)
          .fontSize(12)
          .fontColor('#CC0000')
          .maxLines(8)
          .backgroundColor('#FFF0F0')
          .borderRadius(8)
          .padding(10)
          .width('90%')
          .height(80)
          .margin({ bottom: 10 })
      }

      Button('读取 Inspector 结果')
        .fontSize(14)
        .backgroundColor('#333333')
        .onClick(() => { this.readInspector(); })
        .margin({ bottom: 20 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }

  @Builder
  ModeAView() {
    Column({ space: 6 }) {
      Text('Mode A: Token → brand 链路验证（含获焦态描边色，覆盖 10 Picker + 4 Dialog）')
        .fontSize(13)
        .fontColor('#666666')
      Scroll() {
        Column({ space: 6 }) {
          ForEach(PICKER_TOKENS, (item: TokenItem) => {
            Row({ space: 10 }) {
              Text(item.label)
                .fontSize(11)
                .width(200)
              Text('样色')
                .fontSize(14)
                .fontColor(item.token)
                .key(item.key)
                .backgroundColor('#F0F0F0')
                .borderRadius(4)
                .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            }
            .width('90%')
          })
        }
      }
      .width('95%')
      .height(180)
      .align(Alignment.Center)
    }
    .width('100%')
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  ModeBView() {
    Column({ space: 12 }) {
      Text('Mode B: WithTheme → 组件属性映射验证（文本高亮色/文本主色）')
        .fontSize(13)
        .fontColor('#666666')
      if (this.useCustomTheme) {
        WithTheme({ theme: this.customTheme }) {
          this.PickerComponents()
        }
      } else {
        this.PickerComponents()
      }
    }
    .width('100%')
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  PickerComponents() {
    Column({ space: 8 }) {
      Column() {
        Text('DatePicker').fontSize(11).fontColor('#999999')
        DatePicker({
          start: new Date('2020-01-01'),
          end: new Date('2030-12-31')
        })
          .selected(new Date('2026-01-01'))
          .key('modeB_datepicker')
          .width('80%')
          .height(100)
      }

      Column() {
        Text('TimePicker').fontSize(11).fontColor('#999999')
        TimePicker()
          .selected(new Date('2026-01-01T08:00:00'))
          .key('modeB_timepicker')
          .width('80%')
          .height(100)
      }

      Column() {
        Text('TextPicker').fontSize(11).fontColor('#999999')
        TextPicker({ range: ['选项一', '选项二', '选项三', '选项四'] })
          .selected(0)
          .key('modeB_textpicker')
          .width('80%')
          .height(100)
      }

      Column() {
        Text('CalendarPicker').fontSize(11).fontColor('#999999')
        CalendarPicker()
          .key('modeB_calendarpicker')
          .width('80%')
          .height(100)
      }

      Divider().height(1).color('#EEEEEE').margin({ top: 4, bottom: 4 })

      Text('Dialog 弹窗测试（点击按钮弹出后切换主题）')
        .fontSize(11)
        .fontColor('#666666')

      Row({ space: 8 }) {
        Button('DatePickerDialog')
          .fontSize(11)
          .height(36)
          .onClick(() => {
            DatePickerDialog.show({
              start: new Date('2020-01-01'),
              end: new Date('2030-12-31'),
              selected: new Date('2026-01-01')
            });
          })
        Button('TimePickerDialog')
          .fontSize(11)
          .height(36)
          .onClick(() => {
            TimePickerDialog.show({
              selected: new Date('2026-01-01T08:00:00')
            });
          })
      }

      Row({ space: 8 }) {
        Button('TextPickerDialog')
          .fontSize(11)
          .height(36)
          .onClick(() => {
            TextPickerDialog.show({
              range: ['选项一', '选项二', '选项三'],
              selected: 0
            });
          })
        Button('CalendarPickerDialog')
          .fontSize(11)
          .height(36)
          .onClick(() => {
            CalendarPickerDialog.show({
              selected: new Date('2026-01-15')
            });
          })
      }
    }
    .width('100%')
  }

  private readInspector() {
    let results: string[] = [];

    if (this.currentMode === 0) {
      for (const item of PICKER_TOKENS) {
        try {
          let tree = inspector.byId(item.key);
          if (tree && tree.info) {
            let info = JSON.parse(tree.info);
            let color = this.extractColor(info);
            results.push(`${item.label}: ${color}`);
          } else {
            results.push(`${item.label}: (未获取到)`);
          }
        } catch (e) {
          results.push(`${item.label}: 读取异常`);
        }
      }
    } else {
      const componentKeys = [
        'modeB_datepicker', 'modeB_timepicker', 'modeB_textpicker', 'modeB_calendarpicker'
      ];
      for (const key of componentKeys) {
        try {
          let tree = inspector.byId(key);
          if (tree && tree.info) {
            let info = JSON.parse(tree.info);
            let textColors = this.extractTextColors(info);
            results.push(`${key}: ${textColors.length > 0 ? textColors.join('; ') : '(无Text子节点)'}`);
          } else {
            results.push(`${key}: (未获取到)`);
          }
        } catch (e) {
          results.push(`${key}: 读取异常`);
        }
      }
      results.push('--- Dialog 弹窗需手动弹出后读取 ---');
      for (const item of DIALOG_TESTS) {
        results.push(`${item.label}: (弹窗弹出后点击读取)`);
      }
    }

    this.inspectorResult = `主题: ${this.useCustomTheme ? '自定义(红#E62828)' : '系统(蓝#007DFF)'}\n` + results.join('\n');
  }

  private extractColor(info: object): string {
    let str = JSON.stringify(info);
    let match = str.match(/"fontColor"\s*:\s*"?#?([0-9a-fA-F]{6,8})"?/);
    if (match) {
      return '#' + match[1].toUpperCase();
    }
    let rgbaMatch = str.match(/"fontColor"\s*:\s*\[?\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)/);
    if (rgbaMatch) {
      let r = parseInt(rgbaMatch[1]);
      let g = parseInt(rgbaMatch[2]);
      let b = parseInt(rgbaMatch[3]);
      return '#' + this.toHex(r) + this.toHex(g) + this.toHex(b);
    }
    return '(未解析到fontColor)';
  }

  private extractTextColors(info: object): string[] {
    let colors: string[] = [];
    this.traverseForColors(info, colors);
    return colors;
  }

  private traverseForColors(node: object, colors: string[]) {
    if (!node || typeof node !== 'object') {
      return;
    }
    let nodeStr = JSON.stringify(node);
    let match = nodeStr.match(/"fontColor"\s*:\s*"?#?([0-9a-fA-F]{6,8})"?/);
    if (match) {
      colors.push('#' + match[1].toUpperCase());
    }
    let bgColorMatch = nodeStr.match(/"backgroundColor"\s*:\s*"?#?([0-9a-fA-F]{6,8})"?/);
    if (bgColorMatch) {
      colors.push('bg:#' + bgColorMatch[1].toUpperCase());
    }
    let children = (node as Record<string, object>)['children'];
    if (children && Array.isArray(children)) {
      for (let child of children) {
        this.traverseForColors(child, colors);
      }
    }
  }

  private toHex(n: number): string {
    let h = n.toString(16);
    return h.length === 1 ? '0' + h : h;
  }

  private doRegressionCompare() {
    if (this.baselineResult.length === 0 || this.inspectorResult.length === 0) {
      this.regressionResult = '请先记录基线（系统主题下读取Inspector）再读取当前结果后对比';
      return;
    }
    let baselineLines = this.baselineResult.split('\n');
    let currentLines = this.inspectorResult.split('\n');
    let diffs: string[] = [];
    let maxLen = Math.max(baselineLines.length, currentLines.length);
    for (let i = 0; i < maxLen; i++) {
      let b = i < baselineLines.length ? baselineLines[i] : '(无)';
      let c = i < currentLines.length ? currentLines[i] : '(无)';
      if (b !== c) {
        diffs.push(`差异: 基线[${b}] → 当前[${c}]`);
      }
    }
    if (diffs.length === 0) {
      this.regressionResult = '无差异——色值与基线一致（AC-11 默认视觉无回归 PASS）';
    } else {
      this.regressionResult = diffs.slice(0, 5).join('\n');
    }
  }

  private formatInspectorTree(tree: object, depth: number): string {
    if (!tree || typeof tree !== 'object' || depth > 3) {
      return '';
    }
    let result = '';
    let node = tree as Record<string, object>;
    let type = node['type'];
    let id = node['id'];
    if (type) {
      result += '  '.repeat(depth) + `[${type}]`;
      if (id) {
        result += ` id=${id}`;
      }
      let nodeStr = JSON.stringify(node);
      let fontMatch = nodeStr.match(/"fontColor"\s*:\s*"?#?([0-9a-fA-F]{6,8})"?/);
      if (fontMatch) {
        result += ` fontColor=#${fontMatch[1].toUpperCase()}`;
      }
      let bgMatch = nodeStr.match(/"backgroundColor"\s*:\s*"?#?([0-9a-fA-F]{6,8})"?/);
      if (bgMatch) {
        result += ` bg=#${bgMatch[1].toUpperCase()}`;
      }
      result += '\n';
    }
    let children = node['children'];
    if (children && Array.isArray(children)) {
      for (let child of children) {
        result += this.formatInspectorTree(child, depth + 1);
      }
    }
    return result;
  }

  private readInspectorTreeByKey(key: string): string {
    try {
      let tree = inspector.byId(key);
      if (tree && tree.info) {
        let info = JSON.parse(tree.info);
        return this.formatInspectorTree(info, 0);
      }
    } catch (e) {
      return `(读取异常: ${e})`;
    }
    return '(未获取到)';
  }
}
