interface DemoItem {
  title: string;
  description: string;
}

@Entry
@Component
struct Index {
  @State currentDemo: number = -1;

  private demos: DemoItem[] = [
    { title: 'Basic Image', description: 'Display images from resource media' },
    { title: 'Object Fit', description: 'All six objectFit scaling modes' },
    { title: 'Image Repeat', description: 'Tile images along X, Y and XY axes' },
    { title: 'Image Size', description: 'Various width and height settings' },
    { title: 'Image Border', description: 'Border width, color and style' },
    { title: 'Border Radius', description: 'Rounded corners and circle shape' },
    { title: 'Opacity', description: 'Control image transparency' },
    { title: 'Blur', description: 'Apply gaussian blur' },
    { title: 'Grayscale', description: 'Desaturate the image' },
    { title: 'Brightness', description: 'Adjust image brightness' },
    { title: 'Contrast', description: 'Adjust image contrast' },
    { title: 'Saturate', description: 'Adjust color saturation' },
    { title: 'Invert', description: 'Invert image colors' },
    { title: 'Sepia', description: 'Apply sepia tone' },
    { title: 'Hue Rotate', description: 'Rotate the hue wheel' },
    { title: 'Shadow', description: 'Drop shadow with offset and radius' },
    { title: 'Scale', description: 'Scale image along X and Y' },
    { title: 'Rotate', description: 'Rotate around X, Y, Z axes' },
    { title: 'Translate', description: 'Move image along X and Y' },
    { title: 'Color Blend', description: 'Blend a solid color over the image' },
    { title: 'Gradient Bg', description: 'Image on a gradient background' },
    { title: 'Image Clip', description: 'Clip with rounded and circle shapes' },
    { title: 'Grid Gallery', description: 'A grid of images' },
    { title: 'Swiper', description: 'Image carousel with indicator' },
    { title: 'Image List', description: 'List of images with titles' },
    { title: 'Loading Events', description: 'onComplete and onError callbacks' },
    { title: 'Interpolation', description: 'Image interpolation quality modes' },
    { title: 'Draggable', description: 'Toggle draggable behavior' },
    { title: 'Copy & Alt', description: 'Copy option and alt placeholder' },
    { title: 'Combined FX', description: 'Stack multiple effects together' }
  ];

  @Builder
  HomeView() {
    Column() {
      Text('Image Example')
        .fontSize(26)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 18, bottom: 4 })
      Text('Tap an entry to explore the Image component')
        .fontSize(13)
        .fontColor('#888888')
        .margin({ bottom: 8 })
      Scroll() {
        Column() {
          ForEach(this.demos, (item: DemoItem, index: number) => {
            Row() {
              Column() {
                Text(item.title)
                  .fontSize(16)
                  .fontWeight(FontWeight.Medium)
                Text(item.description)
                  .fontSize(12)
                  .fontColor('#999999')
                  .margin({ top: 2 })
              }
              .alignItems(HorizontalAlign.Start)
              .layoutWeight(1)

              Text(String(index + 1))
                .fontSize(13)
                .fontColor('#FFFFFF')
                .backgroundColor('#3F7AF0')
                .borderRadius(12)
                .width(28)
                .height(28)
                .textAlign(TextAlign.Center)
                .textAlign(TextAlign.Center)

              Text('›')
                .fontSize(20)
                .fontColor('#BBBBBB')
                .margin({ left: 8 })
            }
            .width('100%')
            .padding(14)
            .onClick(() => {
              this.currentDemo = index;
            })
            .border({ width: { bottom: 1 }, color: '#EEEEEE' })
          }, (item: DemoItem, index: number) => index.toString())
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }

  build() {
    Column() {
      if (this.currentDemo === -1) {
        this.HomeView()
      } else if (this.currentDemo === 0) {
        BasicImageDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 1) {
        ObjectFitDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 2) {
        ImageRepeatDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 3) {
        ImageSizeDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 4) {
        ImageBorderDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 5) {
        BorderRadiusDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 6) {
        ImageOpacityDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 7) {
        ImageBlurDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 8) {
        ImageGrayscaleDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 9) {
        ImageBrightnessDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 10) {
        ImageContrastDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 11) {
        ImageSaturateDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 12) {
        ImageInvertDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 13) {
        ImageSepiaDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 14) {
        ImageHueRotateDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 15) {
        ImageShadowDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 16) {
        ImageScaleDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 17) {
        ImageRotateDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 18) {
        ImageTranslateDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 19) {
        ImageColorBlendDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 20) {
        ImageGradientDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 21) {
        ImageClipDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 22) {
        ImageGridDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 23) {
        ImageSwiperDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 24) {
        ImageListDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 25) {
        ImageEventDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 26) {
        ImageInterpolationDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 27) {
        ImageDraggableDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 28) {
        ImageCopyAltDemo({ onBack: () => { this.currentDemo = -1; } })
      } else if (this.currentDemo === 29) {
        ImageCombinedDemo({ onBack: () => { this.currentDemo = -1; } })
      } else {
        Text('Unknown demo')
      }
    }
    .width('100%')
    .height('100%')
  }
}

@Component
struct DemoHeader {
  title: string = '';
  onBack: () => void = () => {};

  build() {
    Row() {
      Button({ type: ButtonType.Circle }) {
        Text('‹')
          .fontSize(22)
          .fontColor('#FFFFFF')
      }
      .width(36)
      .height(36)
      .backgroundColor('#3F7AF0')
      .onClick(() => {
        this.onBack();
      })

      Text(this.title)
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ left: 12 })
    }
    .width('100%')
    .height(56)
    .padding({ left: 12, right: 12 })
    .alignItems(VerticalAlign.Center)
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct DescriptionText {
  text: string = '';

  build() {
    Text(this.text)
      .fontSize(13)
      .fontColor('#666666')
      .margin({ left: 16, right: 16, top: 8, bottom: 8 })
      .width('100%')
  }
}

@Component
struct BasicImageDemo {
  onBack: () => void = () => {};

  build() {
    Column() {
      DemoHeader({ title: 'Basic Image', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Images loaded from app resource media using $r("app.media.xxx"). Each uses a different available asset.' })

          Text('startIcon')
            .fontSize(13)
            .fontColor('#333333')
            .margin({ top: 12 })
          Column() {
            Image($r('app.media.startIcon'))
              .width(160)
              .height(160)
          }
          .width(280)
          .height(200)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)

          Text('background')
            .fontSize(13)
            .fontColor('#333333')
            .margin({ top: 16 })
          Column() {
            Image($r('app.media.background'))
              .width(160)
              .height(160)
          }
          .width(280)
          .height(200)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)

          Text('foreground')
            .fontSize(13)
            .fontColor('#333333')
            .margin({ top: 16 })
          Column() {
            Image($r('app.media.foreground'))
              .width(160)
              .height(160)
          }
          .width(280)
          .height(200)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ObjectFitDemo {
  onBack: () => void = () => {};
  @State selectedIndex: number = 1;

  private names: string[] = ['Fill', 'Contain', 'Cover', 'Auto', 'ScaleDown', 'None'];
  private fits: ImageFit[] =
    [ImageFit.Fill, ImageFit.Contain, ImageFit.Cover, ImageFit.Auto, ImageFit.ScaleDown, ImageFit.None];

  @Builder
  FitButton(index: number) {
    Text(this.names[index])
      .fontSize(12)
      .fontColor(this.selectedIndex === index ? '#FFFFFF' : '#3F7AF0')
      .backgroundColor(this.selectedIndex === index ? '#3F7AF0' : '#E8F0FE')
      .borderRadius(14)
      .padding({ left: 10, right: 10, top: 6, bottom: 6 })
      .margin({ right: 6, bottom: 6 })
      .onClick(() => {
        this.selectedIndex = index;
      })
  }

  build() {
    Column() {
      DemoHeader({ title: 'Object Fit', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'objectFit controls how the image content is resized to fit its box. A 280x180 frame shows a non-square source.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(280)
              .height(180)
              .objectFit(this.fits[this.selectedIndex])
              .backgroundColor('#F2F3F5')
          }
          .width(300)
          .height(200)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Selected: ' + this.names[this.selectedIndex])
            .fontSize(14)
            .margin({ top: 16 })

          Flex({ wrap: FlexWrap.Wrap }) {
            this.FitButton(0)
            this.FitButton(1)
            this.FitButton(2)
            this.FitButton(3)
            this.FitButton(4)
            this.FitButton(5)
          }
          .width('90%')
          .margin({ top: 8 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageRepeatDemo {
  onBack: () => void = () => {};
  @State selectedIndex: number = 0;

  private names: string[] = ['NoRepeat', 'X', 'Y', 'XY'];
  private repeats: ImageRepeat[] = [ImageRepeat.NoRepeat, ImageRepeat.X, ImageRepeat.Y, ImageRepeat.XY];

  build() {
    Column() {
      DemoHeader({ title: 'Image Repeat', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'objectRepeat tiles the source image. A small source is repeated to fill the area.' })

          Column() {
            Image($r('app.media.background'))
              .width(60)
              .height(60)
              .objectRepeat(this.repeats[this.selectedIndex])
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })
          .clip(true)

          Text('Selected: ' + this.names[this.selectedIndex])
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            ForEach(this.names, (name: string, index: number) => {
              Text(name)
                .fontSize(12)
                .fontColor(this.selectedIndex === index ? '#FFFFFF' : '#3F7AF0')
                .backgroundColor(this.selectedIndex === index ? '#3F7AF0' : '#E8F0FE')
                .borderRadius(14)
                .padding({ left: 12, right: 12, top: 6, bottom: 6 })
                .margin({ right: 6 })
                .onClick(() => {
                  this.selectedIndex = index;
                })
            }, (name: string, index: number) => index.toString())
          }
          .margin({ top: 8 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageSizeDemo {
  onBack: () => void = () => {};
  @State sizeIndex: number = 0;

  private sizes: number[] = [80, 120, 160, 200, 240];

  build() {
    Column() {
      DemoHeader({ title: 'Image Size', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Set explicit width and height. The source is scaled to the requested size using objectFit Cover.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(this.sizes[this.sizeIndex])
              .height(this.sizes[this.sizeIndex])
              .objectFit(ImageFit.Cover)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Size: ' + this.sizes[this.sizeIndex] + ' x ' + this.sizes[this.sizeIndex])
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            ForEach(this.sizes, (s: number, index: number) => {
              Text(String(s))
                .fontSize(12)
                .fontColor(this.sizeIndex === index ? '#FFFFFF' : '#3F7AF0')
                .backgroundColor(this.sizeIndex === index ? '#3F7AF0' : '#E8F0FE')
                .borderRadius(14)
                .padding({ left: 12, right: 12, top: 6, bottom: 6 })
                .margin({ right: 6 })
                .onClick(() => {
                  this.sizeIndex = index;
                })
            }, (s: number, index: number) => index.toString())
          }
          .margin({ top: 8 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageBorderDemo {
  onBack: () => void = () => {};
  @State borderThickness: number = 4;
  @State colorIndex: number = 0;

  private colors: ResourceColor[] = ['#3F7AF0', '#F0533F', '#3FB366', '#F0B53F', '#8E3FF0'];

  build() {
    Column() {
      DemoHeader({ title: 'Image Border', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Apply a border with adjustable width and color. Border is drawn outside the image bounds.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(180)
              .height(180)
              .border({
                width: this.borderThickness,
                color: this.colors[this.colorIndex]
              })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Border width: ' + this.borderThickness.toFixed(0) + 'vp')
            .fontSize(14)
            .margin({ top: 16 })
          Row() {
            Text('Width')
              .fontSize(13)
              .width(56)
            Slider({ value: this.borderThickness, min: 0, max: 20, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.borderThickness = value;
              })
            Text(this.borderThickness.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Text('Color')
            .fontSize(13)
            .margin({ top: 12, bottom: 6 })
          Row() {
            ForEach(this.colors, (c: ResourceColor, index: number) => {
              Column()
                .width(36)
                .height(36)
                .backgroundColor(c)
                .borderRadius(18)
                .border({
                  width: this.colorIndex === index ? 3 : 0,
                  color: '#333333'
                })
                .margin({ right: 10 })
                .onClick(() => {
                  this.colorIndex = index;
                })
            }, (c: ResourceColor, index: number) => index.toString())
          }

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct BorderRadiusDemo {
  onBack: () => void = () => {};
  @State radius: number = 0;
  @State isCircle: boolean = false;

  build() {
    Column() {
      DemoHeader({ title: 'Border Radius', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'borderRadius rounds corners. Set it to half the width/height for a perfect circle.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .borderRadius(this.isCircle ? 100 : this.radius)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Radius: ' + (this.isCircle ? '100 (circle)' : this.radius.toFixed(0) + 'vp'))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Radius')
              .fontSize(13)
              .width(56)
            Slider({ value: this.radius, min: 0, max: 100, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.radius = value;
                this.isCircle = false;
              })
            Text(this.radius.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Circle mode')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.isCircle })
              .onChange((isOn: boolean) => {
                this.isCircle = isOn;
              })
          }
          .width('88%')
          .margin({ top: 16 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.radius = 0; this.isCircle = false; })
            Button('20').layoutWeight(1).onClick(() => { this.radius = 20; this.isCircle = false; })
            Button('50').layoutWeight(1).onClick(() => { this.radius = 50; this.isCircle = false; })
            Button('100').layoutWeight(1).onClick(() => { this.isCircle = true; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageOpacityDemo {
  onBack: () => void = () => {};
  @State opacityValue: number = 1;

  build() {
    Column() {
      DemoHeader({ title: 'Opacity', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'opacity sets the overall transparency from 0 (invisible) to 1 (fully opaque).' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .opacity(this.opacityValue)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Opacity: ' + this.opacityValue.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Opacity')
              .fontSize(13)
              .width(56)
            Slider({ value: this.opacityValue, min: 0, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.opacityValue = value;
              })
            Text(this.opacityValue.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.opacityValue = 0; })
            Button('0.5').layoutWeight(1).onClick(() => { this.opacityValue = 0.5; })
            Button('1').layoutWeight(1).onClick(() => { this.opacityValue = 1; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageBlurDemo {
  onBack: () => void = () => {};
  @State blurValue: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Blur', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'blur applies a gaussian blur. Value is the blur radius in vp.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .blur(this.blurValue)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Blur: ' + this.blurValue.toFixed(0) + 'vp')
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Blur')
              .fontSize(13)
              .width(56)
            Slider({ value: this.blurValue, min: 0, max: 50, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.blurValue = value;
              })
            Text(this.blurValue.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.blurValue = 0; })
            Button('10').layoutWeight(1).onClick(() => { this.blurValue = 10; })
            Button('25').layoutWeight(1).onClick(() => { this.blurValue = 25; })
            Button('50').layoutWeight(1).onClick(() => { this.blurValue = 50; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageGrayscaleDemo {
  onBack: () => void = () => {};
  @State amount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Grayscale', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'grayscale converts the image to gray. 0 = original, 1 = fully gray.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .grayscale(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Amount: ' + this.amount.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Amount')
              .fontSize(13)
              .width(56)
            Slider({ value: this.amount, min: 0, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.amount = 0; })
            Button('0.5').layoutWeight(1).onClick(() => { this.amount = 0.5; })
            Button('1').layoutWeight(1).onClick(() => { this.amount = 1; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageBrightnessDemo {
  onBack: () => void = () => {};
  @State amount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Brightness', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'brightness adjusts brightness. 0 = original, 1 = fully white, negative values darken.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .brightness(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Brightness: ' + this.amount.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Value')
              .fontSize(13)
              .width(56)
            Slider({ value: this.amount, min: -1, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('-0.5').layoutWeight(1).onClick(() => { this.amount = -0.5; })
            Button('0').layoutWeight(1).onClick(() => { this.amount = 0; })
            Button('0.5').layoutWeight(1).onClick(() => { this.amount = 0.5; })
            Button('1').layoutWeight(1).onClick(() => { this.amount = 1; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageContrastDemo {
  onBack: () => void = () => {};
  @State amount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Contrast', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'contrast adjusts contrast. 0 = original, 1 = max contrast, -1 = min contrast.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .contrast(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Contrast: ' + this.amount.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Value')
              .fontSize(13)
              .width(56)
            Slider({ value: this.amount, min: -1, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('-0.5').layoutWeight(1).onClick(() => { this.amount = -0.5; })
            Button('0').layoutWeight(1).onClick(() => { this.amount = 0; })
            Button('0.5').layoutWeight(1).onClick(() => { this.amount = 0.5; })
            Button('1').layoutWeight(1).onClick(() => { this.amount = 1; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageSaturateDemo {
  onBack: () => void = () => {};
  @State amount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Saturate', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'saturate adjusts color saturation. 0 = grayscale, 1 = original, higher values boost color.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .saturate(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Saturate: ' + this.amount.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Value')
              .fontSize(13)
              .width(56)
            Slider({ value: this.amount, min: 0, max: 2, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.amount = 0; })
            Button('1').layoutWeight(1).onClick(() => { this.amount = 1; })
            Button('1.5').layoutWeight(1).onClick(() => { this.amount = 1.5; })
            Button('2').layoutWeight(1).onClick(() => { this.amount = 2; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageInvertDemo {
  onBack: () => void = () => {};
  @State amount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Invert', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'invert inverts the colors. 0 = original, 1 = fully inverted.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .invert(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Invert: ' + this.amount.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Value')
              .fontSize(13)
              .width(56)
            Slider({ value: this.amount, min: 0, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.amount = 0; })
            Button('0.5').layoutWeight(1).onClick(() => { this.amount = 0.5; })
            Button('1').layoutWeight(1).onClick(() => { this.amount = 1; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageSepiaDemo {
  onBack: () => void = () => {};
  @State amount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Sepia', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'sepia applies a sepia (vintage brown) tone. 0 = original, 1 = fully sepia.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .sepia(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Sepia: ' + this.amount.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Value')
              .fontSize(13)
              .width(56)
            Slider({ value: this.amount, min: 0, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.amount = 0; })
            Button('0.5').layoutWeight(1).onClick(() => { this.amount = 0.5; })
            Button('1').layoutWeight(1).onClick(() => { this.amount = 1; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageHueRotateDemo {
  onBack: () => void = () => {};
  @State amount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Hue Rotate', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'hueRotate rotates the hue of all colors. Value is in degrees from 0 to 360.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .hueRotate(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Hue: ' + this.amount.toFixed(0) + 'deg')
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Value')
              .fontSize(13)
              .width(56)
            Slider({ value: this.amount, min: 0, max: 360, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('0').layoutWeight(1).onClick(() => { this.amount = 0; })
            Button('90').layoutWeight(1).onClick(() => { this.amount = 90; })
            Button('180').layoutWeight(1).onClick(() => { this.amount = 180; })
            Button('270').layoutWeight(1).onClick(() => { this.amount = 270; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageShadowDemo {
  onBack: () => void = () => {};
  @State radius: number = 10;
  @State offsetX: number = 0;
  @State offsetY: number = 8;
  @State colorIndex: number = 0;

  private colors: ResourceColor[] = ['#888888', '#3F7AF0', '#F0533F', '#000000'];

  build() {
    Column() {
      DemoHeader({ title: 'Shadow', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'shadow adds a drop shadow defined by radius, offset X/Y and color.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(180)
              .height(180)
              .shadow({
                radius: this.radius,
                color: this.colors[this.colorIndex],
                offsetX: this.offsetX,
                offsetY: this.offsetY
              })
          }
          .width(300)
          .height(300)
          .backgroundColor('#FFFFFF')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Row() {
            Text('Radius')
              .fontSize(13)
              .width(70)
            Slider({ value: this.radius, min: 0, max: 40, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.radius = value;
              })
            Text(this.radius.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 16 })

          Row() {
            Text('Offset X')
              .fontSize(13)
              .width(70)
            Slider({ value: this.offsetX, min: -30, max: 30, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.offsetX = value;
              })
            Text(this.offsetX.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Offset Y')
              .fontSize(13)
              .width(70)
            Slider({ value: this.offsetY, min: -30, max: 30, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.offsetY = value;
              })
            Text(this.offsetY.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Text('Color')
            .fontSize(13)
            .margin({ top: 12, bottom: 6 })
          Row() {
            ForEach(this.colors, (c: ResourceColor, index: number) => {
              Column()
                .width(36)
                .height(36)
                .backgroundColor(c)
                .borderRadius(18)
                .border({
                  width: this.colorIndex === index ? 3 : 0,
                  color: '#333333'
                })
                .margin({ right: 10 })
                .onClick(() => {
                  this.colorIndex = index;
                })
            }, (c: ResourceColor, index: number) => index.toString())
          }

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageScaleDemo {
  onBack: () => void = () => {};
  @State scaleX: number = 1;
  @State scaleY: number = 1;

  build() {
    Column() {
      DemoHeader({ title: 'Scale', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'scale transforms the image along X and Y independently.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .scale({
                x: this.scaleX,
                y: this.scaleY
              })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Scale X: ' + this.scaleX.toFixed(2) + '  Y: ' + this.scaleY.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Scale X')
              .fontSize(13)
              .width(70)
            Slider({ value: this.scaleX, min: 0.1, max: 2, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.scaleX = value;
              })
            Text(this.scaleX.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Scale Y')
              .fontSize(13)
              .width(70)
            Slider({ value: this.scaleY, min: 0.1, max: 2, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.scaleY = value;
              })
            Text(this.scaleY.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('Reset').layoutWeight(1).onClick(() => { this.scaleX = 1; this.scaleY = 1; })
            Button('Wide').layoutWeight(1).onClick(() => { this.scaleX = 1.5; this.scaleY = 1; })
            Button('Tall').layoutWeight(1).onClick(() => { this.scaleX = 1; this.scaleY = 1.5; })
            Button('Tiny').layoutWeight(1).onClick(() => { this.scaleX = 0.5; this.scaleY = 0.5; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageRotateDemo {
  onBack: () => void = () => {};
  @State angle: number = 0;
  @State axisIndex: number = 2;

  private axisNames: string[] = ['X', 'Y', 'Z'];

  build() {
    Column() {
      DemoHeader({ title: 'Rotate', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'rotate takes an angle (degrees) and an axis vector (x,y,z). Pick an axis and slide the angle.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(180)
              .height(180)
              .rotate({
                x: this.axisIndex === 0 ? 1 : 0,
                y: this.axisIndex === 1 ? 1 : 0,
                z: this.axisIndex === 2 ? 1 : 0,
                angle: this.angle
              })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Angle: ' + this.angle.toFixed(0) + 'deg  Axis: ' + this.axisNames[this.axisIndex])
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Angle')
              .fontSize(13)
              .width(60)
            Slider({ value: this.angle, min: 0, max: 360, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.angle = value;
              })
            Text(this.angle.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Text('Axis')
            .fontSize(13)
            .margin({ top: 12, bottom: 6 })
          Row() {
            ForEach(this.axisNames, (name: string, index: number) => {
              Text(name)
                .fontSize(13)
                .fontColor(this.axisIndex === index ? '#FFFFFF' : '#3F7AF0')
                .backgroundColor(this.axisIndex === index ? '#3F7AF0' : '#E8F0FE')
                .borderRadius(14)
                .padding({ left: 16, right: 16, top: 8, bottom: 8 })
                .margin({ right: 8 })
                .onClick(() => {
                  this.axisIndex = index;
                })
            }, (name: string, index: number) => index.toString())
          }
          .margin({ top: 4 })

          Row() {
            Button('Reset').layoutWeight(1).onClick(() => { this.angle = 0; })
            Button('45').layoutWeight(1).onClick(() => { this.angle = 45; })
            Button('90').layoutWeight(1).onClick(() => { this.angle = 90; })
            Button('180').layoutWeight(1).onClick(() => { this.angle = 180; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageTranslateDemo {
  onBack: () => void = () => {};
  @State x: number = 0;
  @State y: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Translate', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'translate moves the image along X and Y by the given vp values.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(160)
              .height(160)
              .translate({
                x: this.x,
                y: this.y
              })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('X: ' + this.x.toFixed(0) + '  Y: ' + this.y.toFixed(0))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Move X')
              .fontSize(13)
              .width(60)
            Slider({ value: this.x, min: -100, max: 100, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.x = value;
              })
            Text(this.x.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Move Y')
              .fontSize(13)
              .width(60)
            Slider({ value: this.y, min: -100, max: 100, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.y = value;
              })
            Text(this.y.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('Reset').layoutWeight(1).onClick(() => { this.x = 0; this.y = 0; })
            Button('Up').layoutWeight(1).onClick(() => { this.y = -60; })
            Button('Down').layoutWeight(1).onClick(() => { this.y = 60; })
            Button('Right').layoutWeight(1).onClick(() => { this.x = 60; })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageColorBlendDemo {
  onBack: () => void = () => {};
  @State selectedIndex: number = 0;
  @State amount: number = 0.5;

  private colors: ResourceColor[] = ['#FF0000', '#00FF00', '#0000FF', '#FFFF00', '#FF00FF', '#00FFFF', '#000000', '#FFFFFF'];

  build() {
    Column() {
      DemoHeader({ title: 'Color Blend', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'colorBlend blends a solid color over the image. Pick a color and strength.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .colorBlend(this.colors[this.selectedIndex])
              .opacity(this.amount)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Blend strength: ' + this.amount.toFixed(2))
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Strength')
              .fontSize(13)
              .width(70)
            Slider({ value: this.amount, min: 0, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.amount = value;
              })
            Text(this.amount.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Text('Color')
            .fontSize(13)
            .margin({ top: 12, bottom: 6 })
          Flex({ wrap: FlexWrap.Wrap }) {
            ForEach(this.colors, (c: ResourceColor, index: number) => {
              Column()
                .width(40)
                .height(40)
                .backgroundColor(c)
                .borderRadius(20)
                .border({
                  width: this.selectedIndex === index ? 3 : 1,
                  color: this.selectedIndex === index ? '#333333' : '#CCCCCC'
                })
                .margin({ right: 10, bottom: 10 })
                .onClick(() => {
                  this.selectedIndex = index;
                })
            }, (c: ResourceColor, index: number) => index.toString())
          }
          .width('88%')

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageGradientDemo {
  onBack: () => void = () => {};
  @State angle: number = 90;
  @State color1Index: number = 0;
  @State color2Index: number = 1;

  private colors: ResourceColor[] = ['#3F7AF0', '#F0533F', '#3FB366', '#F0B53F', '#8E3FF0', '#000000'];

  build() {
    Column() {
      DemoHeader({ title: 'Gradient Bg', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Place an image over a linear gradient background created with linearGradient.' })

          Column() {
            Image($r('app.media.foreground'))
              .width(140)
              .height(140)
          }
          .width(300)
          .height(260)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .borderRadius(16)
          .linearGradient({
            angle: this.angle,
            colors: [[this.colors[this.color1Index], 0.0], [this.colors[this.color2Index], 1.0]]
          })
          .margin({ top: 8 })

          Text('Angle: ' + this.angle.toFixed(0) + 'deg')
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Angle')
              .fontSize(13)
              .width(60)
            Slider({ value: this.angle, min: 0, max: 360, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.angle = value;
              })
            Text(this.angle.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Text('Start color')
            .fontSize(13)
            .margin({ top: 12, bottom: 6 })
          Row() {
            ForEach(this.colors, (c: ResourceColor, index: number) => {
              Column()
                .width(32)
                .height(32)
                .backgroundColor(c)
                .borderRadius(16)
                .border({
                  width: this.color1Index === index ? 3 : 1,
                  color: this.color1Index === index ? '#333333' : '#CCCCCC'
                })
                .margin({ right: 8 })
                .onClick(() => {
                  this.color1Index = index;
                })
            }, (c: ResourceColor, index: number) => index.toString())
          }

          Text('End color')
            .fontSize(13)
            .margin({ top: 12, bottom: 6 })
          Row() {
            ForEach(this.colors, (c: ResourceColor, index: number) => {
              Column()
                .width(32)
                .height(32)
                .backgroundColor(c)
                .borderRadius(16)
                .border({
                  width: this.color2Index === index ? 3 : 1,
                  color: this.color2Index === index ? '#333333' : '#CCCCCC'
                })
                .margin({ right: 8 })
                .onClick(() => {
                  this.color2Index = index;
                })
            }, (c: ResourceColor, index: number) => index.toString())
          }

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageClipDemo {
  onBack: () => void = () => {};
  @State clipIndex: number = 0;
  @State radius: number = 40;

  private names: string[] = ['None', 'Rounded', 'Circle', 'Top Rounded', 'Pill'];

  build() {
    Column() {
      DemoHeader({ title: 'Image Clip', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Clip the image to various shapes using borderRadius with per-corner values.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .objectFit(ImageFit.Cover)
              .borderRadius(this.clipIndex === 0 ? 0
                : this.clipIndex === 1 ? this.radius
                  : this.clipIndex === 2 ? 100
                    : this.clipIndex === 3 ? { topLeft: 60, topRight: 60, bottomLeft: 0, bottomRight: 0 }
                      : { topLeft: 100, topRight: 100, bottomLeft: 100, bottomRight: 100 })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Clip: ' + this.names[this.clipIndex])
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            ForEach(this.names, (name: string, index: number) => {
              Text(name)
                .fontSize(12)
                .fontColor(this.clipIndex === index ? '#FFFFFF' : '#3F7AF0')
                .backgroundColor(this.clipIndex === index ? '#3F7AF0' : '#E8F0FE')
                .borderRadius(14)
                .padding({ left: 10, right: 10, top: 6, bottom: 6 })
                .margin({ right: 6, bottom: 6 })
                .onClick(() => {
                  this.clipIndex = index;
                })
            }, (name: string, index: number) => index.toString())
          }
          .margin({ top: 8, left: 12, right: 12 })

          Text('Rounded radius')
            .fontSize(13)
            .margin({ top: 12 })
          Row() {
            Text('Radius')
              .fontSize(13)
              .width(60)
            Slider({ value: this.radius, min: 0, max: 100, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.radius = value;
                this.clipIndex = 1;
              })
            Text(this.radius.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageGridDemo {
  onBack: () => void = () => {};

  private sources: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground')
  ];
  @State gapIndex: number = 1;
  private gaps: number[] = [2, 8, 16, 24];
  @State fitIndex: number = 0;
  private fits: ImageFit[] = [ImageFit.Cover, ImageFit.Contain, ImageFit.Fill];

  build() {
    Column() {
      DemoHeader({ title: 'Grid Gallery', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'A Grid of images with adjustable spacing and objectFit.' })

          Row() {
            Text('Gap')
              .fontSize(13)
              .margin({ right: 8 })
            ForEach(this.gaps, (g: number, index: number) => {
              Text(String(g))
                .fontSize(12)
                .fontColor(this.gapIndex === index ? '#FFFFFF' : '#3F7AF0')
                .backgroundColor(this.gapIndex === index ? '#3F7AF0' : '#E8F0FE')
                .borderRadius(14)
                .padding({ left: 10, right: 10, top: 4, bottom: 4 })
                .margin({ right: 6 })
                .onClick(() => {
                  this.gapIndex = index;
                })
            }, (g: number, index: number) => index.toString())
          }
          .margin({ top: 8, left: 12 })

          Row() {
            Text('Fit')
              .fontSize(13)
              .margin({ right: 8 })
            Text('Cover')
              .fontSize(12)
              .fontColor(this.fitIndex === 0 ? '#FFFFFF' : '#3F7AF0')
              .backgroundColor(this.fitIndex === 0 ? '#3F7AF0' : '#E8F0FE')
              .borderRadius(14)
              .padding({ left: 10, right: 10, top: 4, bottom: 4 })
              .margin({ right: 6 })
              .onClick(() => {
                this.fitIndex = 0;
              })
            Text('Contain')
              .fontSize(12)
              .fontColor(this.fitIndex === 1 ? '#FFFFFF' : '#3F7AF0')
              .backgroundColor(this.fitIndex === 1 ? '#3F7AF0' : '#E8F0FE')
              .borderRadius(14)
              .padding({ left: 10, right: 10, top: 4, bottom: 4 })
              .margin({ right: 6 })
              .onClick(() => {
                this.fitIndex = 1;
              })
            Text('Fill')
              .fontSize(12)
              .fontColor(this.fitIndex === 2 ? '#FFFFFF' : '#3F7AF0')
              .backgroundColor(this.fitIndex === 2 ? '#3F7AF0' : '#E8F0FE')
              .borderRadius(14)
              .padding({ left: 10, right: 10, top: 4, bottom: 4 })
              .onClick(() => {
                this.fitIndex = 2;
              })
          }
          .margin({ top: 8, left: 12 })

          Grid() {
            ForEach(this.sources, (src: Resource, index: number) => {
              GridItem() {
                Image(src)
                  .width('100%')
                  .height('100%')
                  .objectFit(this.fits[this.fitIndex])
                  .borderRadius(8)
              }
            }, (src: Resource, index: number) => index.toString())
          }
          .columnsTemplate('1fr 1fr 1fr')
          .rowsGap(this.gaps[this.gapIndex])
          .columnsGap(this.gaps[this.gapIndex])
          .width('90%')
          .height(360)
          .margin({ top: 12 })
          .padding(8)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageSwiperDemo {
  onBack: () => void = () => {};
  @State index: number = 0;
  @State autoPlay: boolean = true;
  @State loop: boolean = true;

  private sources: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground')
  ];
  private labels: string[] = ['Start Icon', 'Background', 'Foreground'];

  build() {
    Column() {
      DemoHeader({ title: 'Swiper', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Swiper creates a paged carousel. Swipe or toggle auto-play and looping.' })

          Swiper() {
            ForEach(this.sources, (src: Resource, index: number) => {
              Column() {
                Image(src)
                  .width(240)
                  .height(240)
                  .objectFit(ImageFit.Contain)
                  .borderRadius(12)
                Text(this.labels[index])
                  .fontSize(14)
                  .margin({ top: 12 })
              }
              .width('100%')
              .justifyContent(FlexAlign.Center)
              .alignItems(HorizontalAlign.Center)
            }, (src: Resource, index: number) => index.toString())
          }
          .index(this.index)
          .autoPlay(this.autoPlay)
          .loop(this.loop)
          .interval(2000)
          .indicator(true)
          .width('100%')
          .height(320)
          .margin({ top: 8 })
          .onChange((index: number) => {
            this.index = index;
          })

          Text('Current page: ' + (this.index + 1) + ' / ' + this.labels.length)
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            Text('Auto play')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.autoPlay })
              .onChange((isOn: boolean) => {
                this.autoPlay = isOn;
              })
          }
          .width('88%')
          .margin({ top: 12 })

          Row() {
            Text('Loop')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.loop })
              .onChange((isOn: boolean) => {
                this.loop = isOn;
              })
          }
          .width('88%')
          .margin({ top: 12 })

          Row() {
            Button('Prev').layoutWeight(1).onClick(() => {
              this.index = (this.index - 1 + this.sources.length) % this.sources.length;
            })
            Button('Next').layoutWeight(1).onClick(() => {
              this.index = (this.index + 1) % this.sources.length;
            })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageListDemo {
  onBack: () => void = () => {};

  private titles: string[] = [
    'Start Icon',
    'Background',
    'Foreground',
    'Start Icon Again',
    'Background Again',
    'Foreground Again'
  ];
  private subtitles: string[] = [
    'App launch icon resource',
    'Layered image background',
    'Layered image foreground',
    'Reused launch icon',
    'Reused background',
    'Reused foreground'
  ];
  private sources: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground')
  ];

  build() {
    Column() {
      DemoHeader({ title: 'Image List', onBack: () => this.onBack() })
      DescriptionText({ text: 'A vertical List pairing images with title and subtitle text.' })
      List({ space: 10 }) {
        ForEach(this.titles, (title: string, index: number) => {
          ListItem() {
            Row() {
              Image(this.sources[index])
                .width(64)
                .height(64)
                .objectFit(ImageFit.Cover)
                .borderRadius(12)
              Column() {
                Text(title)
                  .fontSize(15)
                  .fontWeight(FontWeight.Medium)
                Text(this.subtitles[index])
                  .fontSize(12)
                  .fontColor('#999999')
                  .margin({ top: 4 })
              }
              .alignItems(HorizontalAlign.Start)
              .layoutWeight(1)
              .margin({ left: 12 })

              Text('›')
                .fontSize(20)
                .fontColor('#BBBBBB')
            }
            .width('100%')
            .padding(12)
            .backgroundColor('#FFFFFF')
            .borderRadius(12)
          }
        }, (title: string, index: number) => index.toString())
      }
      .width('92%')
      .layoutWeight(1)
      .margin({ top: 4 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F7F8FA')
  }
}

@Component
struct ImageEventDemo {
  onBack: () => void = () => {};
  @State status: string = 'Idle';
  @State result: string = 'No event yet';
  @State loadCount: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Loading Events', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Image fires onComplete when loaded with size info, and onError when loading fails.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .onComplete(() => {
                this.status = 'Complete';
                this.result = 'Image decoded and rendered successfully';
                this.loadCount = this.loadCount + 1;
              })
              .onError(() => {
                this.status = 'Error';
                this.result = 'Image failed to load';
              })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Column() {
            Row() {
              Text('Status:')
                .fontSize(13)
                .fontColor('#999999')
              Text(this.status)
                .fontSize(13)
                .fontWeight(FontWeight.Bold)
                .fontColor(this.status === 'Complete' ? '#3FB366' : this.status === 'Error' ? '#F0533F' : '#999999')
                .margin({ left: 8 })
            }
            .width('100%')
            .margin({ top: 6 })

            Text('Detail')
              .fontSize(12)
              .fontColor('#999999')
              .margin({ top: 10, bottom: 4 })
            Text(this.result)
              .fontSize(13)
              .width('100%')

            Row() {
              Text('Load count:')
                .fontSize(13)
                .fontColor('#999999')
              Text(this.loadCount.toString())
                .fontSize(13)
                .fontWeight(FontWeight.Bold)
                .margin({ left: 8 })
            }
            .width('100%')
            .margin({ top: 8 })
          }
          .width('88%')
          .padding(12)
          .backgroundColor('#F7F8FA')
          .borderRadius(12)
          .margin({ top: 16 })

          Text('Note: events also include onLoadStart and onLoadSuccess on some API levels.')
            .fontSize(11)
            .fontColor('#AAAAAA')
            .margin({ top: 12, left: 16, right: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageInterpolationDemo {
  onBack: () => void = () => {};
  @State selectedIndex: number = 2;

  private names: string[] = ['None', 'Low', 'Medium', 'High'];
  private modes: ImageInterpolation[] = [
    ImageInterpolation.None,
    ImageInterpolation.Low,
    ImageInterpolation.Medium,
    ImageInterpolation.High
  ];

  build() {
    Column() {
      DemoHeader({ title: 'Interpolation', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'interpolation sets the sampling quality when the image is scaled. Visible on a small source scaled up.' })

          Column() {
            Image($r('app.media.background'))
              .width(60)
              .height(60)
              .interpolation(this.modes[this.selectedIndex])
              .objectFit(ImageFit.Fill)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Text('Selected: ' + this.names[this.selectedIndex])
            .fontSize(14)
            .margin({ top: 16 })

          Row() {
            ForEach(this.names, (name: string, index: number) => {
              Text(name)
                .fontSize(12)
                .fontColor(this.selectedIndex === index ? '#FFFFFF' : '#3F7AF0')
                .backgroundColor(this.selectedIndex === index ? '#3F7AF0' : '#E8F0FE')
                .borderRadius(14)
                .padding({ left: 12, right: 12, top: 6, bottom: 6 })
                .margin({ right: 6 })
                .onClick(() => {
                  this.selectedIndex = index;
                })
            }, (name: string, index: number) => index.toString())
          }
          .margin({ top: 8 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageDraggableDemo {
  onBack: () => void = () => {};
  @State isDraggable: boolean = true;
  @State showPreview: boolean = true;
  @State count: number = 0;

  build() {
    Column() {
      DemoHeader({ title: 'Draggable', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'draggable enables the image to be dragged out. Long-press and drag to test.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(180)
              .height(180)
              .draggable(this.isDraggable)
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Row() {
            Text('Draggable')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.isDraggable })
              .onChange((isOn: boolean) => {
                this.isDraggable = isOn;
              })
          }
          .width('88%')
          .margin({ top: 16 })

          Row() {
            Text('Drag preview')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.showPreview })
              .onChange((isOn: boolean) => {
                this.showPreview = isOn;
              })
          }
          .width('88%')
          .margin({ top: 12 })

          Text('Drop count: ' + this.count)
            .fontSize(14)
            .margin({ top: 16 })

          Column() {
            Text('Drop here')
              .fontSize(14)
              .fontColor('#888888')
          }
          .width('80%')
          .height(80)
          .border({ width: 2, color: '#3F7AF0', style: BorderStyle.Dashed })
          .borderRadius(12)
          .margin({ top: 12 })
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageCopyAltDemo {
  onBack: () => void = () => {};
  @State copyEnabled: boolean = true;
  @State useAlt: boolean = true;
  @State altLoaded: boolean = false;

  build() {
    Column() {
      DemoHeader({ title: 'Copy & Alt', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'copyOption enables long-press copy of the image. alt shows a placeholder while loading.' })

          Column() {
            Image(this.useAlt ? $r('app.media.foreground') : $r('app.media.startIcon'))
              .width(200)
              .height(200)
              .copyOption(this.copyEnabled ? CopyOptions.LocalDevice : CopyOptions.None)
              .alt($r('app.media.background'))
              .onComplete(() => {
                this.altLoaded = true;
              })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Row() {
            Text('Copy enabled')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.copyEnabled })
              .onChange((isOn: boolean) => {
                this.copyEnabled = isOn;
              })
          }
          .width('88%')
          .margin({ top: 16 })

          Row() {
            Text('Use alt placeholder')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.useAlt })
              .onChange((isOn: boolean) => {
                this.useAlt = isOn;
                this.altLoaded = false;
              })
          }
          .width('88%')
          .margin({ top: 12 })

          Text(this.altLoaded ? 'Main image loaded' : 'Alt shown (or loading)')
            .fontSize(13)
            .fontColor('#888888')
            .margin({ top: 12 })

          Text('Long-press the image to copy it when enabled.')
            .fontSize(11)
            .fontColor('#AAAAAA')
            .margin({ top: 12, left: 16, right: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}

@Component
struct ImageCombinedDemo {
  onBack: () => void = () => {};
  @State blurValue: number = 5;
  @State grayValue: number = 0.3;
  @State brightValue: number = 0.1;
  @State saturateValue: number = 1.2;
  @State hue: number = 30;
  @State radius: number = 24;
  @State shadowOn: boolean = true;
  @State borderOn: boolean = true;

  build() {
    Column() {
      DemoHeader({ title: 'Combined FX', onBack: () => this.onBack() })
      Scroll() {
        Column() {
          DescriptionText({ text: 'Stack multiple effects at once: blur, grayscale, brightness, saturate, hue, radius and shadow.' })

          Column() {
            Image($r('app.media.startIcon'))
              .width(200)
              .height(200)
              .objectFit(ImageFit.Cover)
              .borderRadius(this.radius)
              .blur(this.blurValue)
              .grayscale(this.grayValue)
              .brightness(this.brightValue)
              .saturate(this.saturateValue)
              .hueRotate(this.hue)
              .border(this.borderOn ? { width: 3, color: '#3F7AF0' } : { width: 0, color: '#000000' })
              .shadow(this.shadowOn ? {
                radius: 12,
                color: '#888888',
                offsetX: 0,
                offsetY: 8
              } : { radius: 0, color: '#000000', offsetX: 0, offsetY: 0 })
          }
          .width(280)
          .height(280)
          .backgroundColor('#F2F3F5')
          .borderRadius(12)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .margin({ top: 8 })

          Row() {
            Text('Blur')
              .fontSize(13)
              .width(70)
            Slider({ value: this.blurValue, min: 0, max: 20, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.blurValue = value;
              })
            Text(this.blurValue.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 16 })

          Row() {
            Text('Grayscale')
              .fontSize(13)
              .width(70)
            Slider({ value: this.grayValue, min: 0, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.grayValue = value;
              })
            Text(this.grayValue.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Bright')
              .fontSize(13)
              .width(70)
            Slider({ value: this.brightValue, min: -1, max: 1, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.brightValue = value;
              })
            Text(this.brightValue.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Saturate')
              .fontSize(13)
              .width(70)
            Slider({ value: this.saturateValue, min: 0, max: 2, step: 0.01 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.saturateValue = value;
              })
            Text(this.saturateValue.toFixed(2))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Hue')
              .fontSize(13)
              .width(70)
            Slider({ value: this.hue, min: 0, max: 360, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.hue = value;
              })
            Text(this.hue.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Radius')
              .fontSize(13)
              .width(70)
            Slider({ value: this.radius, min: 0, max: 100, step: 1 })
              .layoutWeight(1)
              .onChange((value: number) => {
                this.radius = value;
              })
            Text(this.radius.toFixed(0))
              .fontSize(13)
              .width(40)
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Text('Shadow')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.shadowOn })
              .onChange((isOn: boolean) => {
                this.shadowOn = isOn;
              })
          }
          .width('88%')
          .margin({ top: 12 })

          Row() {
            Text('Border')
              .fontSize(13)
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.borderOn })
              .onChange((isOn: boolean) => {
                this.borderOn = isOn;
              })
          }
          .width('88%')
          .margin({ top: 8 })

          Row() {
            Button('Reset').layoutWeight(1).onClick(() => {
              this.blurValue = 5;
              this.grayValue = 0.3;
              this.brightValue = 0.1;
              this.saturateValue = 1.2;
              this.hue = 30;
              this.radius = 24;
              this.shadowOn = true;
              this.borderOn = true;
            })
            Button('Vivid').layoutWeight(1).onClick(() => {
              this.blurValue = 0;
              this.grayValue = 0;
              this.brightValue = 0.1;
              this.saturateValue = 1.8;
              this.hue = 0;
            })
            Button('Vintage').layoutWeight(1).onClick(() => {
              this.blurValue = 1;
              this.grayValue = 0.4;
              this.brightValue = 0.1;
              this.saturateValue = 0.8;
              this.hue = 20;
              this.radius = 100;
            })
          }
          .width('88%')
          .margin({ top: 16 })

          Blank().height(20)
        }
        .width('100%')
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
  }
}
