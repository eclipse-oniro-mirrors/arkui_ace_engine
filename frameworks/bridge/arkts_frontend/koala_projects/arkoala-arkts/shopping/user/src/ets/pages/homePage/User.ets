/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import router from '@ohos.router';
import { RecordModel, OrderModel, DiscountModel, ServiceModel } from '../../model/homeModel'
import { recordData, orderData, discountData, serviceData } from '../../data/homeData'
import { Core2 } from './core2'
import { SearchParam } from '../../model/routeModel'

@Component
export struct User {
  @State signName: string = '点这里可以添加个性签名'
  @State cardName: string = '用户'
  @State url: string = '/resources/common/user.png'
  @State record: Array<RecordModel> = recordData
  @State orderData: Array<OrderModel> = orderData
  @State discountData: Array<DiscountModel> = discountData
  @State serviceData: Array<ServiceModel> = serviceData
  @Prop num: number
  @Prop ratio: number

  build() {
    Scroll() {
      Column() {
        Row() {
          Image(this.url)
            .width(90 * this.ratio)
            .aspectRatio(1)
            .objectFit(ImageFit.Contain)
            .border({ radius: 45 * this.ratio } as BorderOptions)
            .margin(10)
          Column() {
            Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
              Text(this.cardName)
                .fontSize(15 * this.ratio)
                .fontWeight(FontWeight.Bold)
                .margin({ top: 10, bottom: 5 } as Margin)
              Row() {
                Image('/resources/icon/code.png')
                  .width(30 * this.ratio)
                  .aspectRatio(1)
                  .objectFit(ImageFit.Contain)
                  .margin({ right: 10 } as Margin)
                Image('/resources/icon/setting.png')
                  .width(30 * this.ratio)
                  .aspectRatio(1)
                  .objectFit(ImageFit.Contain)
              }
            }
            .padding({ right: 20 } as Padding)

            Row() {
              Text(this.signName)
                .fontSize(10 * this.ratio)
                .fontColor(Color.Grey)
                .margin({ right: 5 } as Margin)
                .maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis } as TextOverflowOptions)
              Image('/resources/icon/setting.png')
                .width(10 * this.ratio)
                .aspectRatio(1)
                .objectFit(ImageFit.Contain)
            }
            .width('52%')
            .margin({ top: 5, bottom: 5 } as Margin)

            Row() {
              Text('关注 100')
                .fontSize(10 * this.ratio)
                .margin({ right: 10 } as Margin)
              Text(' | ')
                .fontSize(10 * this.ratio)
                .fontColor(Color.Grey)
              Text('粉丝 0')
                .fontSize(10 * this.ratio)
                .margin({ right: 10 } as Margin)
            }
            .margin({ top: 5, bottom: 5 } as Margin)
          }
          .alignItems(HorizontalAlign.Start)
          .width('70%')
        }

        Flex({ justifyContent: FlexAlign.SpaceAround, alignItems: ItemAlign.Center }) {
          ForEach(this.record, (item: RecordModel, index: number) => {
            Column() {
              Image(item.img)
                .width(30 * this.ratio)
                .aspectRatio(1)
                .objectFit(ImageFit.Contain)
                .margin({ bottom: 5 } as Margin)
              Text(item.title)
                .fontSize(10 * this.ratio)
            }
            .onClick((e?: ClickEvent) => {
              router.pushUrl({
                url: item.uri,
                params: {
                  num: this.num,
                  ratio: this.ratio
                } as SearchParam
              })
            })
          }, (item: RecordModel, index: number) => JSON.stringify(item))
        }

        Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
          Row() {
            Text('活动')
              .fontSize(12 * this.ratio)
              .fontColor(Color.White)
              .backgroundColor(Color.Orange)
              .padding({ top: 2, bottom: 2, left: 5, right: 5 } as Padding)
              .border({ radius: 4 * this.ratio } as BorderOptions)
              .margin({ right: 5 } as Margin)
            Text('限时充50元送优惠券30元')
              .fontSize(13 * this.ratio)
          }

          Row() {
            Text('去看看')
              .fontSize(14 * this.ratio)
              .fontColor(Color.Grey)
            Image('/resources/icon/arrow_right.png')
              .width(20 * this.ratio)
              .aspectRatio(1)
              .objectFit(ImageFit.Contain)
          }
        }
        .margin({ top: 10 } as Margin)
        .padding(10)
        .backgroundColor(Color.White)
        .border({ radius: 10 * this.ratio } as BorderOptions)

        Column() {
          Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text('我的订单')
              .fontSize(14 * this.ratio)
              .fontWeight(FontWeight.Bolder)
            Row() {
              Text('全部')
                .fontSize(14 * this.ratio)
                .fontColor(Color.Grey)
              Image('/resources/icon/arrow_right.png')
                .width(20 * this.ratio)
                .aspectRatio(1)
                .objectFit(ImageFit.Contain)
            }
          }

          Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            ForEach(this.orderData, (item: OrderModel, index: number) => {
              Column() {
                Image(item.img)
                  .width(30 * this.ratio)
                  .aspectRatio(1)
                  .objectFit(ImageFit.Contain)
                  .margin({ bottom: 5 } as Margin)
                Text(item.title)
                  .fontSize(12 * this.ratio)
              }
            }, (item: OrderModel, index: number) => JSON.stringify(item))
          }
          .margin({ top: 10 } as Margin)

          Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Image('/resources/homeImg/imgLoads/product006.png')
              .width(55 * this.ratio)
              .aspectRatio(1)
              .objectFit(ImageFit.Cover)
              .border({ radius: 5 * this.ratio } as BorderOptions)
            Flex({ justifyContent: FlexAlign.SpaceAround, direction: FlexDirection.Column }) {
              Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
                Text('已签收')
                  .fontSize(13 * this.ratio)
                Text('12-17')
                  .fontSize(12 * this.ratio)
                  .fontColor(Color.Grey)
              }
              .margin({ left: 8 } as Margin)

              Text('派送成功')
                .fontSize(12 * this.ratio)
                .fontColor(Color.Grey)
                .margin({ left: 8 } as Margin)
            }
            .backgroundColor('#f8f8fa')
            .width('90%')
            .padding({ right: 10 } as Padding)
            .border({ radius: 10 } as BorderOptions)
          }
          .height(55 * this.ratio)
          .margin({ top: 10 } as Margin)
        }
        .margin({ top: 10 } as Margin)
        .padding(10)
        .backgroundColor(Color.White)
        .border({ radius: 10 * this.ratio } as BorderOptions)

        Column() {
          Flex({ justifyContent: FlexAlign.SpaceAround, alignItems: ItemAlign.Center }) {
            ForEach(this.discountData, (item: DiscountModel, index: number) => {
              Column() {
                Image(item.img)
                  .width(35 * this.ratio)
                  .aspectRatio(1)
                  .objectFit(ImageFit.Contain)
                  .margin({ bottom: 5 } as Margin)
                Text(item.name)
                  .fontSize(12 * this.ratio)
                  .margin(5)
                Text(item.text)
                  .fontSize(12 * this.ratio)
                  .fontColor(Color.Grey)
              }
            }, (item: DiscountModel, index: number) => JSON.stringify(item))
          }
          .margin({ top: 10 } as Margin)
        }
        .margin({ top: 10 } as Margin)
        .padding(10)
        .backgroundColor(Color.White)
        .border({ radius: 10 * this.ratio } as BorderOptions)

        Column() {
          Flex({ justifyContent: FlexAlign.SpaceAround, alignItems: ItemAlign.Center }) {
            ForEach(this.serviceData, (item: ServiceModel, index: number) => {
              Column() {
                Image(item.img)
                  .width(30 * this.ratio)
                  .aspectRatio(1)
                  .objectFit(ImageFit.Contain)
                  .margin({ bottom: 5 } as Margin)
                Text(item.title)
                  .fontSize(12 * this.ratio)
              }
            }, (item: ServiceModel, index: number) => JSON.stringify(item))
          }
          .margin({ top: 10 } as Margin)
        }
        .margin({ top: 10 } as Margin)
        .padding(10)
        .backgroundColor(Color.White)
        .border({ radius: 10 * this.ratio } as BorderOptions)

        Column() {
          Core2({ ratio: this.ratio })
        }
        .margin({ top: 10 } as Margin)
        .padding(10)
        .backgroundColor(Color.White)
        .border({ radius: 10 * this.ratio } as BorderOptions)
      }
      .padding(10)
      .backgroundColor('#dfe5e3')
    }
    .scrollBar(BarState.Off)
  }
}
