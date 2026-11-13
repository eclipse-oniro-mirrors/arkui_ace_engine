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

@Component
export struct NativeFields {
  caseId: string = 'native_fields';
  @State input: string = 'account-input';
  @State checked: boolean = false;
  @State menuVisible: boolean = false;
  private rich: RichEditorController = new RichEditorController();

  @Builder
  menu() {
    Menu() {
      MenuItem({ content: 'Parent menu', builder: this.submenu })
      MenuItem({ content: 'Selected option' }).selected(this.checked)
    }
  }

  @Builder
  submenu() {
    Menu() { MenuItem({ content: 'Child menu' }) }
  }

  build() {
    Column({ space: 10 }) {
      Text('ordinary-label').id('label')
        .obscured(this.caseId === 'privacy_obscured' ? [ObscuredReasons.PLACEHOLDER] : [])
      Button('Confirm').id('button-one')
      Button('Confirm').id('button-two')
      Search({ value: this.input, placeholder: 'search-hint' }).searchButton('Search now').id('search')
      TextInput({ text: this.input, placeholder: 'input-hint' })
        .type(this.caseId === 'privacy_password' ? InputType.Password : InputType.Normal)
        .obscured(this.caseId === 'privacy_obscured' ? [ObscuredReasons.PLACEHOLDER] : [])
        .accessibilityDescription('account description').accessibilityGroup(false).id('input')
      TextArea({ text: 'line one\nline two', placeholder: 'area-hint' }).id('area')
      RichEditor({ controller: this.rich }).height(90).id('rich')
        .onReady(() => { this.rich.addTextSpan('rich-one'); this.rich.addTextSpan('rich-two'); })
      Marquee({ start: true, step: 5, loop: -1, fromStart: true, src: 'marquee-text' }).width(100).id('marquee')
      TextClock().format('HH:mm:ss').id('clock')
      TextTimer({ isCountDown: false }).format('HH:mm:ss').id('timer')
      Checkbox({ name: 'check' }).select(this.checked).onChange((value: boolean) => { this.checked = value; })
      Radio({ value: 'radio', group: 'group' }).checked(this.checked)
      Toggle({ type: ToggleType.Switch, isOn: this.checked })
      Select([{ value: 'Alpha' }, { value: 'Beta' }]).selected(0).value('Alpha').id('select')
      Button('Menu trigger').bindMenu(this.menuVisible, this.menu)
        .onClick(() => { this.menuVisible = !this.menuVisible; }).id('menu-trigger')
      TextPicker({ range: ['First', 'Second'], selected: 0 }).height(100).id('text-picker')
      DatePicker({ selected: new Date('2026-10-19') }).height(100).id('date-picker')
      TimePicker({ selected: new Date('2026-10-19T12:30:00') }).height(100).id('time-picker')
    }.width('100%')
  }
}
