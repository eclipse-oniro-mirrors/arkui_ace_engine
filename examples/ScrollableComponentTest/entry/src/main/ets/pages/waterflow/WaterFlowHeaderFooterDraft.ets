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

class WaterFlowHeightModifier implements AttributeModifier<WaterFlowAttribute> {
  private height: Length | undefined;

  constructor(height: Length | undefined) {
    this.height = height;
  }

  applyNormalAttribute(instance: WaterFlowAttribute): void {
    if (this.height !== undefined) {
      instance.height(this.height);
    }
  }
}

@Entry
@Component
struct WaterFlowHeaderFooterDraft {
  @State parent: number = 0;
  @State stickyMode: StickyStyle = StickyStyle.BOTH;
  @State showHeader: boolean = true;
  @State showFooter: boolean = true;
  @State layoutMode: WaterFlowLayoutMode = WaterFlowLayoutMode.ALWAYS_TOP_DOWN;
  @State clicks: number = 0;
  private items: number[] = Array.from({ length: 40 }, (_: number, index: number) => index);

  @Builder
  Header() {
    Text(`Header 点击 ${this.clicks}`)
      .width('100%').height(60).backgroundColor('#c6e6ff')
      .onClick(() => { this.clicks++; })
  }

  @Builder
  Footer() {
    Text(`Footer 点击 ${this.clicks}`)
      .width('100%').height(50).backgroundColor('#ffe0b2')
      .onClick(() => { this.clicks++; })
  }

  @Builder
  Target(height: Length | undefined) {
    WaterFlow({ layoutMode: this.layoutMode }) {
      ForEach(this.items, (index: number) => {
        FlowItem() {
          Text(`FlowItem ${index}`).width('100%').height(90 + index % 3 * 30)
            .backgroundColor(index % 2 === 0 ? '#e8f5e9' : '#f3e5f5')
        }
      }, (index: number) => `${index}`)
    }
    .width('100%').attributeModifier(new WaterFlowHeightModifier(height))
    .columnsTemplate('1fr 1fr').rowsGap(8).columnsGap(8)
    .header(this.showHeader ? this.Header : undefined)
    .footer(this.showFooter ? this.Footer : undefined)
    .sticky(this.stickyMode)
  }

  build() {
    Column({ space: 8 }) {
      Text('WaterFlow header / footer / sticky draftCode').fontSize(18)
      Row({ space: 4 }) {
        ForEach(['Scroll', 'List', 'Grid', 'WaterFlow', '独立'], (label: string, index: number) => {
          Button(label).fontSize(12).onClick(() => { this.parent = index; })
        })
      }
      Row({ space: 4 }) {
        Button('切换吸附').onClick(() => { this.stickyMode = (this.stickyMode + 1) % 4; })
        Button('头部').onClick(() => { this.showHeader = !this.showHeader; })
        Button('尾部').onClick(() => { this.showFooter = !this.showFooter; })
        Button('布局').onClick(() => {
          this.layoutMode = this.layoutMode === WaterFlowLayoutMode.ALWAYS_TOP_DOWN
            ? WaterFlowLayoutMode.SLIDING_WINDOW : WaterFlowLayoutMode.ALWAYS_TOP_DOWN;
        })
      }
      Text(`sticky=${this.stickyMode}; layoutMode=${this.layoutMode}; 点击=${this.clicks}`)
      if (this.parent === 0) {
        Scroll() {
          this.Target(undefined)
        }.layoutWeight(1).width('100%')
      } else if (this.parent === 1) {
        List() {
          ListItem() { Text('向下滚动查看目标').height(180) }
          ListItem() { this.Target(1200) }
          ListItem() { Text('目标已离开').height(500) }
        }.layoutWeight(1).width('100%')
      } else if (this.parent === 2) {
        Grid() {
          GridItem() { this.Target(1200) }
          GridItem() { Text('另一列').height(1500).backgroundColor('#eeeeee') }
        }.columnsTemplate('1fr 1fr').layoutWeight(1).width('100%')
      } else if (this.parent === 3) {
        WaterFlow() {
          FlowItem() { Text('外层 WaterFlow').height(180) }
          FlowItem() { this.Target(1200) }
          FlowItem() { Text('目标已离开').height(500) }
        }.columnsTemplate('1fr').layoutWeight(1).width('100%')
      } else {
        Column() { this.Target('100%') }.layoutWeight(1).width('100%')
      }
    }.width('100%').height('100%').padding(8)
  }
}
