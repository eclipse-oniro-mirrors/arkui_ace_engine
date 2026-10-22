import {
  memo,
  __memo_context_type,
  __memo_id_type,
  State,
  StateDecoratedVariable,
  MutableState,
  stateOf,
  observableProxy
} from '@ohos.arkui.stateManagement' // should be insert by ui-plugins

import {
  Text,
  TextAttribute,
  Column,
  Row,
  Scroll,
  Component,
  Button,
  ButtonAttribute,
  ClickEvent,
  UserView,
  ButtonType,
  ButtonStyleMode,
  ButtonOptions,
  ButtonRole,
  Color,
  FontWeight,
  FontStyle,
  BorderStyle,
  Margin,
  Padding,
  Visibility,
  Alignment,
  Position,
  SizeOptions,
  BorderOptions,
  BorderRadiuses,
  LengthMetrics,
  TouchEvent,
  KeyEvent,
  Builder,
  CommonMethod,
  StateStyles,
  CustomStyles,
  ClickEffect,
  ClickEffectLevel,
  ItemAlign,
  Area,
  FlexAlign,
  FlexDirection,
  FlexWrap,
  NavDestination, 
  NavPathStack, 
  NavDestinationContext, 
  Callback
} from '@ohos.arkui.component' // TextAttribute should be insert by ui-plugins

import hilog from '@ohos.hilog'

@Component
export struct ButtonTest {
  @State stateVar: string = 'state var';
  message: string = 'var';
  @State buttonType: ButtonType = ButtonType.Capsule;
  @State buttonRole: ButtonRole = ButtonRole.NORMAL;
  @State buttonStyle: ButtonStyleMode = ButtonStyleMode.EMPHASIZED;
  @State buttonColor: Color = Color.Blue;
  @State buttonFontSize: number = 16;
  @State buttonFontWeight: FontWeight = FontWeight.Normal;
  @State buttonFontStyle: FontStyle = FontStyle.Normal;
  @State buttonFontFamily: string = 'sans-serif';
  @State buttonStateEffect: boolean = true;
  @State buttonVisibility: Visibility = Visibility.Visible;
  @State buttonOpacity: number = 1;
  @State buttonWidth: string = '80%';
  @State buttonHeight: number = 40;
  @State buttonBorderRadius: number = 8;
  @State buttonBorderWidth: number = 1;
  @State buttonBorderColor: Color = Color.Black;
  @State buttonBorderStyle: BorderStyle = BorderStyle.Solid;
  @State buttonMarginValue: number = 5;
  @State buttonPaddingValue: number = 8;
  @State buttonText: string = 'Test Button';
  @State clickCount: number = 0;
  @State isPressed: boolean = false;
  @State isFocused: boolean = false;
  @State isHovered: boolean = false;
  @State isAttached: boolean = false;

  changeValue() {
    this.stateVar += '~'
  }

  toggleButtonType() {
    if (this.buttonType === ButtonType.Capsule) {
      this.buttonType = ButtonType.Normal
    } else if (this.buttonType === ButtonType.Normal) {
      this.buttonType = ButtonType.Circle
    } else {
      this.buttonType = ButtonType.Capsule
    }
    hilog.info(0x0000, 'testTag', 'toggleButtonType: ' + this.buttonType);
  }

  toggleButtonRole() {
    if (this.buttonRole === ButtonRole.NORMAL) {
      this.buttonRole = ButtonRole.ERROR
    } else {
      this.buttonRole = ButtonRole.NORMAL
    }
    hilog.info(0x0000, 'testTag', 'toggleButtonRole: ' + this.buttonRole);
  }

  toggleButtonStyle() {
    if (this.buttonStyle === ButtonStyleMode.EMPHASIZED) {
      this.buttonStyle = ButtonStyleMode.NORMAL
    } else {
      this.buttonStyle = ButtonStyleMode.EMPHASIZED
    }
    hilog.info(0x0000, 'testTag', 'toggleButtonStyle: ' + this.buttonStyle);
  }

  cycleButtonColor() {
    if (this.buttonColor === Color.Blue) {
      this.buttonColor = Color.Red
    } else if (this.buttonColor === Color.Red) {
      this.buttonColor = Color.Green
    } else if (this.buttonColor === Color.Green) {
      this.buttonColor = Color.Orange
    } else if (this.buttonColor === Color.Orange) {
      this.buttonColor = Color.Pink
    } else {
      this.buttonColor = Color.Blue
    }
    hilog.info(0x0000, 'testTag', 'cycleButtonColor: ' + this.buttonColor);
  }

  cycleFontSize() {
    if (this.buttonFontSize === 16) {
      this.buttonFontSize = 20
    } else if (this.buttonFontSize === 20) {
      this.buttonFontSize = 24
    } else if (this.buttonFontSize === 24) {
      this.buttonFontSize = 12
    } else {
      this.buttonFontSize = 16
    }
    hilog.info(0x0000, 'testTag', 'cycleFontSize: ' + this.buttonFontSize);
  }

  cycleFontWeight() {
    if (this.buttonFontWeight === FontWeight.Normal) {
      this.buttonFontWeight = FontWeight.Bold
    } else if (this.buttonFontWeight === FontWeight.Bold) {
      this.buttonFontWeight = FontWeight.Lighter
    } else if (this.buttonFontWeight === FontWeight.Lighter) {
      this.buttonFontWeight = FontWeight.Bolder
    } else {
      this.buttonFontWeight = FontWeight.Normal
    }
    hilog.info(0x0000, 'testTag', 'cycleFontWeight: ' + this.buttonFontWeight);
  }

  toggleFontStyle() {
    if (this.buttonFontStyle === FontStyle.Normal) {
      this.buttonFontStyle = FontStyle.Italic
    } else {
      this.buttonFontStyle = FontStyle.Normal
    }
    hilog.info(0x0000, 'testTag', 'toggleFontStyle: ' + this.buttonFontStyle);
  }

  toggleStateEffect() {
    this.buttonStateEffect = !this.buttonStateEffect
    hilog.info(0x0000, 'testTag', 'toggleStateEffect: ' + this.buttonStateEffect);
  }

  toggleVisibility() {
    if (this.buttonVisibility === Visibility.Visible) {
      this.buttonVisibility = Visibility.Hidden
    } else if (this.buttonVisibility === Visibility.Hidden) {
      this.buttonVisibility = Visibility.None
    } else {
      this.buttonVisibility = Visibility.Visible
    }
    hilog.info(0x0000, 'testTag', 'toggleVisibility: ' + this.buttonVisibility);
  }

  cycleOpacity() {
    if (this.buttonOpacity === 1) {
      this.buttonOpacity = 0.5
    } else if (this.buttonOpacity === 0.5) {
      this.buttonOpacity = 0.3
    } else if (this.buttonOpacity === 0.3) {
      this.buttonOpacity = 0
    } else {
      this.buttonOpacity = 1
    }
    hilog.info(0x0000, 'testTag', 'cycleOpacity: ' + this.buttonOpacity);
  }

  incrementClickCount() {
    this.clickCount++
    hilog.info(0x0000, 'testTag', 'incrementClickCount: ' + this.clickCount);
  }

  normalButtonStyles: CustomStyles = (instance: CommonMethod): void => {
    instance.backgroundColor('#FFCCCCCC')
  }

  pressedButtonStyles: CustomStyles = (instance: CommonMethod): void => {
    instance.backgroundColor('#FF666666')
  }

  focusedButtonStyles: CustomStyles = (instance: CommonMethod): void => {
    instance.backgroundColor('#FF3333AA')
  }

  @Builder
  BuildButtonTestCases() {
    Column() {
      // === Section 1: ButtonType Tests ===
      Text('=== Section 1: ButtonType Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Normal Type Button', {
        type: ButtonType.Normal,
        stateEffect: true,
      } as ButtonOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Normal Type Button clicked');
        })

      Button('Circle Type Button', {
        type: ButtonType.Circle,
        stateEffect: true,
      } as ButtonOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Circle Type Button clicked');
        })

      Button('Capsule Type Button', {
        type: ButtonType.Capsule,
        stateEffect: true,
      } as ButtonOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Capsule Type Button clicked');
        })

      Button('Normal Type via .type()', { stateEffect: true } as ButtonOptions)
        .type(ButtonType.Normal)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Normal Type via .type() clicked');
        })

      Button('Circle Type via .type()', { stateEffect: true } as ButtonOptions)
        .type(ButtonType.Circle)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Circle Type via .type() clicked');
        })

      Button('Capsule Type via .type()', { stateEffect: true } as ButtonOptions)
        .type(ButtonType.Capsule)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Capsule Type via .type() clicked');
        })

      Button('Default Type Button (no type set)')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Default Type Button clicked');
        })

      Button('Dynamic Type Button')
        .type(this.buttonType)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section1: Dynamic Type Button clicked, type: ' + this.buttonType);
          this.toggleButtonType()
        })

      Button('Switch to Normal').type(ButtonType.Normal)
        .onClick((e: ClickEvent) => {
          this.buttonType = ButtonType.Normal
          hilog.info(0x0000, 'testTag', 'Section1: Switched to Normal');
        })

      Button('Switch to Circle').type(ButtonType.Circle)
        .onClick((e: ClickEvent) => {
          this.buttonType = ButtonType.Circle
          hilog.info(0x0000, 'testTag', 'Section1: Switched to Circle');
        })

      Button('Switch to Capsule').type(ButtonType.Capsule)
        .onClick((e: ClickEvent) => {
          this.buttonType = ButtonType.Capsule
          hilog.info(0x0000, 'testTag', 'Section1: Switched to Capsule');
        })

      // === Section 2: ButtonRole Tests ===
      Text('=== Section 2: ButtonRole Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Normal Role Button')
        .role(ButtonRole.NORMAL)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section2: Normal Role Button clicked');
        })

      Button('Error Role Button')
        .role(ButtonRole.ERROR)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section2: Error Role Button clicked');
        })

      Button('Normal Role + Circle', {
        type: ButtonType.Circle,
      } as ButtonOptions)
        .role(ButtonRole.NORMAL)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section2: Normal Role + Circle clicked');
        })

      Button('Error Role + Circle', {
        type: ButtonType.Circle,
      } as ButtonOptions)
        .role(ButtonRole.ERROR)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section2: Error Role + Circle clicked');
        })

      Button('Normal Role + Normal Type', {
        type: ButtonType.Normal,
      } as ButtonOptions)
        .role(ButtonRole.NORMAL)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section2: Normal Role + Normal Type clicked');
        })

      Button('Error Role + Normal Type', {
        type: ButtonType.Normal,
      } as ButtonOptions)
        .role(ButtonRole.ERROR)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section2: Error Role + Normal Type clicked');
        })

      Button('Dynamic Role Button')
        .role(this.buttonRole)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section2: Dynamic Role clicked, role: ' + this.buttonRole);
          this.toggleButtonRole()
        })

      Button('Switch Role to Normal')
        .role(ButtonRole.NORMAL)
        .onClick((e: ClickEvent) => {
          this.buttonRole = ButtonRole.NORMAL
          hilog.info(0x0000, 'testTag', 'Section2: Switched Role to Normal');
        })

      Button('Switch Role to Error')
        .role(ButtonRole.ERROR)
        .onClick((e: ClickEvent) => {
          this.buttonRole = ButtonRole.ERROR
          hilog.info(0x0000, 'testTag', 'Section2: Switched Role to Error');
        })

      // === Section 3: ButtonStyleMode Tests ===
      Text('=== Section 3: ButtonStyleMode Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Emphasized Style Button')
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section3: Emphasized Style clicked');
        })

      Button('Normal Style Button')
        .buttonStyle(ButtonStyleMode.NORMAL)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section3: Normal Style clicked');
        })

      Button('Emphasized + Circle', {
        type: ButtonType.Circle,
      } as ButtonOptions)
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section3: Emphasized + Circle clicked');
        })

      Button('Normal + Circle', {
        type: ButtonType.Circle,
      } as ButtonOptions)
        .buttonStyle(ButtonStyleMode.NORMAL)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section3: Normal + Circle clicked');
        })

      Button('Emphasized + Error Role')
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .role(ButtonRole.ERROR)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section3: Emphasized + Error Role clicked');
        })

      Button('Normal + Normal Role')
        .buttonStyle(ButtonStyleMode.NORMAL)
        .role(ButtonRole.NORMAL)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section3: Normal + Normal Role clicked');
        })

      Button('Dynamic Style Button')
        .buttonStyle(this.buttonStyle)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section3: Dynamic Style clicked, style: ' + this.buttonStyle);
          this.toggleButtonStyle()
        })

      Button('Switch Style to Emphasized')
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .onClick((e: ClickEvent) => {
          this.buttonStyle = ButtonStyleMode.EMPHASIZED
          hilog.info(0x0000, 'testTag', 'Section3: Switched Style to Emphasized');
        })

      Button('Switch Style to Normal')
        .buttonStyle(ButtonStyleMode.NORMAL)
        .onClick((e: ClickEvent) => {
          this.buttonStyle = ButtonStyleMode.NORMAL
          hilog.info(0x0000, 'testTag', 'Section3: Switched Style to Normal');
        })

      // === Section 4: Font Color Tests ===
      Text('=== Section 4: Font Color Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('FontColor Blue').fontColor(Color.Blue)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Blue clicked');
        })

      Button('FontColor Red').fontColor(Color.Red)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Red clicked');
        })

      Button('FontColor Green').fontColor(Color.Green)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Green clicked');
        })

      Button('FontColor Orange').fontColor(Color.Orange)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Orange clicked');
        })

      Button('FontColor Pink').fontColor(Color.Pink)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Pink clicked');
        })

      Button('FontColor White').fontColor(Color.White)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor White clicked');
        })

      Button('FontColor Black').fontColor(Color.Black)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Black clicked');
        })

      Button('FontColor Hex FF0000').fontColor('#FF0000')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Hex FF0000 clicked');
        })

      Button('FontColor Hex FFFFFF00').fontColor('#FFFF00')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Hex FFFF00 clicked');
        })

      Button('FontColor Hex FFFF00FF').fontColor('#FFFF00FF')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Hex FFFF00FF clicked');
        })

      Button('FontColor Hex 00FF00').fontColor('#00FF00')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Hex 00FF00 clicked');
        })

      Button('FontColor Number 0xFF0000').fontColor(0xFF0000)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Number 0xFF0000 clicked');
        })

      Button('FontColor Number 0x00FF00').fontColor(0x00FF00)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Number 0x00FF00 clicked');
        })

      Button('FontColor Number 0x0000FF').fontColor(0x0000FF)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Number 0x0000FF clicked');
        })

      Button('FontColor Number 0xFFFFFF').fontColor(0xFFFFFF)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: FontColor Number 0xFFFFFF clicked');
        })

      Button('Dynamic FontColor').fontColor(this.buttonColor)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section4: Dynamic FontColor clicked, color: ' + this.buttonColor);
          this.cycleButtonColor()
        })

      // === Section 5: Font Size Tests ===
      Text('=== Section 5: Font Size Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('FontSize 10').fontSize(10)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 10 clicked');
        })

      Button('FontSize 12').fontSize(12)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 12 clicked');
        })

      Button('FontSize 14').fontSize(14)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 14 clicked');
        })

      Button('FontSize 16').fontSize(16)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 16 clicked');
        })

      Button('FontSize 20').fontSize(20)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 20 clicked');
        })

      Button('FontSize 24').fontSize(24)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 24 clicked');
        })

      Button('FontSize 30').fontSize(30)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 30 clicked');
        })

      Button('FontSize 40').fontSize(40)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize 40 clicked');
        })

      Button('FontSize string 16vp').fontSize('16vp')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize string 16vp clicked');
        })

      Button('FontSize string 20fp').fontSize('20fp')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize string 20fp clicked');
        })

      Button('FontSize string 18px').fontSize('18px')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize string 18px clicked');
        })

      Button('FontSize string 100%').fontSize('100%')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: FontSize string 100% clicked');
        })

      Button('Dynamic FontSize').fontSize(this.buttonFontSize)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section5: Dynamic FontSize clicked, size: ' + this.buttonFontSize);
          this.cycleFontSize()
        })

      Button('FontSize Increase').fontSize(this.buttonFontSize)
        .onClick((e: ClickEvent) => {
          this.buttonFontSize = this.buttonFontSize + 2
          hilog.info(0x0000, 'testTag', 'Section5: FontSize increased to: ' + this.buttonFontSize);
        })

      Button('FontSize Decrease').fontSize(this.buttonFontSize)
        .onClick((e: ClickEvent) => {
          this.buttonFontSize = this.buttonFontSize - 2
          hilog.info(0x0000, 'testTag', 'Section5: FontSize decreased to: ' + this.buttonFontSize);
        })

      Button('FontSize Reset to 16').fontSize(this.buttonFontSize)
        .onClick((e: ClickEvent) => {
          this.buttonFontSize = 16
          hilog.info(0x0000, 'testTag', 'Section5: FontSize reset to 16');
        })

      // === Section 6: Font Weight Tests ===
      Text('=== Section 6: Font Weight Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('FontWeight Normal').fontWeight(FontWeight.Normal)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight Normal clicked');
        })

      Button('FontWeight Bold').fontWeight(FontWeight.Bold)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight Bold clicked');
        })

      Button('FontWeight Bolder').fontWeight(FontWeight.Bolder)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight Bolder clicked');
        })

      Button('FontWeight Lighter').fontWeight(FontWeight.Lighter)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight Lighter clicked');
        })

      Button('FontWeight 100').fontWeight(100)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight 100 clicked');
        })

      Button('FontWeight 300').fontWeight(300)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight 300 clicked');
        })

      Button('FontWeight 400').fontWeight(400)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight 400 clicked');
        })

      Button('FontWeight 500').fontWeight(500)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight 500 clicked');
        })

      Button('FontWeight 600').fontWeight(600)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight 600 clicked');
        })

      Button('FontWeight 700').fontWeight(700)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight 700 clicked');
        })

      Button('FontWeight 900').fontWeight(900)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: FontWeight 900 clicked');
        })

      Button('Dynamic FontWeight').fontWeight(this.buttonFontWeight)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section6: Dynamic FontWeight clicked, weight: ' + this.buttonFontWeight);
          this.cycleFontWeight()
        })

      // === Section 7: Font Style Tests ===
      Text('=== Section 7: Font Style Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('FontStyle Normal').fontStyle(FontStyle.Normal)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section7: FontStyle Normal clicked');
        })

      Button('FontStyle Italic').fontStyle(FontStyle.Italic)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section7: FontStyle Italic clicked');
        })

      Button('FontStyle Normal + Bold').fontStyle(FontStyle.Normal).fontWeight(FontWeight.Bold)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section7: FontStyle Normal + Bold clicked');
        })

      Button('FontStyle Italic + Bold').fontStyle(FontStyle.Italic).fontWeight(FontWeight.Bold)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section7: FontStyle Italic + Bold clicked');
        })

      Button('Dynamic FontStyle').fontStyle(this.buttonFontStyle)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section7: Dynamic FontStyle clicked, style: ' + this.buttonFontStyle);
          this.toggleFontStyle()
        })

      Button('Toggle FontStyle').fontStyle(this.buttonFontStyle)
        .onClick((e: ClickEvent) => {
          this.toggleFontStyle()
          hilog.info(0x0000, 'testTag', 'Section7: FontStyle toggled to: ' + this.buttonFontStyle);
        })

      // === Section 8: Font Family Tests ===
      Text('=== Section 8: Font Family Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('FontFamily sans-serif').fontFamily('sans-serif')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: FontFamily sans-serif clicked');
        })

      Button('FontFamily serif').fontFamily('serif')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: FontFamily serif clicked');
        })

      Button('FontFamily monospace').fontFamily('monospace')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: FontFamily monospace clicked');
        })

      Button('FontFamily Arial').fontFamily('Arial')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: FontFamily Arial clicked');
        })

      Button('FontFamily Helvetica').fontFamily('Helvetica')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: FontFamily Helvetica clicked');
        })

      Button('FontFamily Courier New').fontFamily('Courier New')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: FontFamily Courier New clicked');
        })

      Button('FontFamily Times New Roman').fontFamily('Times New Roman')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: FontFamily Times New Roman clicked');
        })

      Button('Dynamic FontFamily').fontFamily(this.buttonFontFamily)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section8: Dynamic FontFamily clicked, family: ' + this.buttonFontFamily);
        })

      Button('Change FontFamily to serif').fontFamily(this.buttonFontFamily)
        .onClick((e: ClickEvent) => {
          this.buttonFontFamily = 'serif'
          hilog.info(0x0000, 'testTag', 'Section8: Changed FontFamily to serif');
        })

      Button('Change FontFamily to monospace').fontFamily(this.buttonFontFamily)
        .onClick((e: ClickEvent) => {
          this.buttonFontFamily = 'monospace'
          hilog.info(0x0000, 'testTag', 'Section8: Changed FontFamily to monospace');
        })

      // === Section 9: StateEffect Tests ===
      Text('=== Section 9: StateEffect Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('StateEffect true').stateEffect(true)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section9: StateEffect true clicked');
        })

      Button('StateEffect false').stateEffect(false)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section9: StateEffect false clicked');
        })

      Button('StateEffect true + Normal', {
        type: ButtonType.Normal,
        stateEffect: true,
      } as ButtonOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section9: StateEffect true + Normal clicked');
        })

      Button('StateEffect false + Normal', {
        type: ButtonType.Normal,
        stateEffect: false,
      } as ButtonOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section9: StateEffect false + Normal clicked');
        })

      Button('StateEffect true + Circle', {
        type: ButtonType.Circle,
        stateEffect: true,
      } as ButtonOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section9: StateEffect true + Circle clicked');
        })

      Button('StateEffect false + Circle', {
        type: ButtonType.Circle,
        stateEffect: false,
      } as ButtonOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section9: StateEffect false + Circle clicked');
        })

      Button('Dynamic StateEffect').stateEffect(this.buttonStateEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section9: Dynamic StateEffect clicked, stateEffect: ' + this.buttonStateEffect);
          this.toggleStateEffect()
        })

      Button('Toggle StateEffect').stateEffect(this.buttonStateEffect)
        .onClick((e: ClickEvent) => {
          this.toggleStateEffect()
          hilog.info(0x0000, 'testTag', 'Section9: StateEffect toggled to: ' + this.buttonStateEffect);
        })

      // === Section 10: Background Color Tests ===
      Text('=== Section 10: Background Color Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('BgColor Blue').backgroundColor(Color.Blue)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Blue clicked');
        })

      Button('BgColor Red').backgroundColor(Color.Red)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Red clicked');
        })

      Button('BgColor Green').backgroundColor(Color.Green)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Green clicked');
        })

      Button('BgColor Yellow').backgroundColor(Color.Yellow)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Yellow clicked');
        })

      Button('BgColor Orange').backgroundColor(Color.Orange)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Orange clicked');
        })

      Button('BgColor Pink').backgroundColor(Color.Pink)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Pink clicked');
        })

      Button('BgColor Hex FFFF00FF').backgroundColor('#FFFF00FF')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Hex FFFF00FF clicked');
        })

      Button('BgColor Hex FFCCCCCC').backgroundColor('#FFCCCCCC')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Hex FFCCCCCC clicked');
        })

      Button('BgColor Hex 00FF00FF').backgroundColor('#00FF00FF')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Hex 00FF00FF clicked');
        })

      Button('BgColor Number 0xFF0000').backgroundColor(0xFF0000)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Number 0xFF0000 clicked');
        })

      Button('BgColor Number 0x00FF00').backgroundColor(0x00FF00)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Number 0x00FF00 clicked');
        })

      Button('BgColor Number 0x0000FF').backgroundColor(0x0000FF)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor Number 0x0000FF clicked');
        })

      Button('BgColor + Type Circle').backgroundColor(Color.Purple).type(ButtonType.Circle)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor + Type Circle clicked');
        })

      Button('BgColor + FontColor').backgroundColor(Color.Black).fontColor(Color.White)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor + FontColor clicked');
        })

      Button('BgColor + Role Error').backgroundColor(Color.Red).role(ButtonRole.ERROR)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section10: BgColor + Role Error clicked');
        })

      // === Section 11: Border Tests ===
      Text('=== Section 11: Border Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('BorderRadius 5').borderRadius(5)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderRadius 5 clicked');
        })

      Button('BorderRadius 10').borderRadius(10)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderRadius 10 clicked');
        })

      Button('BorderRadius 20').borderRadius(20)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderRadius 20 clicked');
        })

      Button('BorderRadius 50').borderRadius(50)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderRadius 50 clicked');
        })

      Button('BorderRadius string 10vp').borderRadius('10vp')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderRadius string 10vp clicked');
        })

      Button('BorderRadius string 50%').borderRadius('50%')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderRadius string 50% clicked');
        })

      Button('Border All Solid').border({
        width: 2,
        color: Color.Red,
        style: BorderStyle.Solid,
        radius: 8,
      } as BorderOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: Border All Solid clicked');
        })

      Button('Border Dashed').border({
        width: 2,
        color: Color.Blue,
        style: BorderStyle.Dashed,
        radius: 10,
      } as BorderOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: Border Dashed clicked');
        })

      Button('Border Dotted').border({
        width: 3,
        color: Color.Green,
        style: BorderStyle.Dotted,
        radius: 12,
      } as BorderOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: Border Dotted clicked');
        })

      Button('BorderWidth 1').borderWidth(1)
        .borderColor(Color.Black)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderWidth 1 clicked');
        })

      Button('BorderWidth 3').borderWidth(3)
        .borderColor(Color.Red)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderWidth 3 clicked');
        })

      Button('BorderWidth 5').borderWidth(5)
        .borderColor(Color.Blue)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderWidth 5 clicked');
        })

      Button('BorderStyle Solid').borderStyle(BorderStyle.Solid)
        .borderWidth(2).borderColor(Color.Black)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderStyle Solid clicked');
        })

      Button('BorderStyle Dashed').borderStyle(BorderStyle.Dashed)
        .borderWidth(2).borderColor(Color.Black)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderStyle Dashed clicked');
        })

      Button('BorderStyle Dotted').borderStyle(BorderStyle.Dotted)
        .borderWidth(2).borderColor(Color.Black)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderStyle Dotted clicked');
        })

      Button('BorderColor Red').borderColor(Color.Red)
        .borderWidth(2)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderColor Red clicked');
        })

      Button('BorderColor Blue').borderColor(Color.Blue)
        .borderWidth(2)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderColor Blue clicked');
        })

      Button('BorderColor Green').borderColor(Color.Green)
        .borderWidth(2)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section11: BorderColor Green clicked');
        })

      Button('Dynamic BorderRadius').borderRadius(this.buttonBorderRadius)
        .onClick((e: ClickEvent) => {
          this.buttonBorderRadius = this.buttonBorderRadius + 5
          if (this.buttonBorderRadius > 30) {
            this.buttonBorderRadius = 0
          }
          hilog.info(0x0000, 'testTag', 'Section11: Dynamic BorderRadius clicked, radius: ' + this.buttonBorderRadius);
        })

      Button('Dynamic BorderWidth').borderWidth(this.buttonBorderWidth)
        .borderColor(Color.Red)
        .onClick((e: ClickEvent) => {
          this.buttonBorderWidth = this.buttonBorderWidth + 1
          if (this.buttonBorderWidth > 5) {
            this.buttonBorderWidth = 1
          }
          hilog.info(0x0000, 'testTag', 'Section11: Dynamic BorderWidth clicked, width: ' + this.buttonBorderWidth);
        })

      Button('Dynamic BorderColor').borderColor(this.buttonBorderColor)
        .borderWidth(2)
        .onClick((e: ClickEvent) => {
          if (this.buttonBorderColor === Color.Black) {
            this.buttonBorderColor = Color.Red
          } else if (this.buttonBorderColor === Color.Red) {
            this.buttonBorderColor = Color.Blue
          } else {
            this.buttonBorderColor = Color.Black
          }
          hilog.info(0x0000, 'testTag', 'Section11: Dynamic BorderColor clicked, color: ' + this.buttonBorderColor);
        })

      // === Section 12: Size Tests ===
      Text('=== Section 12: Size Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Width 100').width(100)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Width 100 clicked');
        })

      Button('Width 200').width(200)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Width 200 clicked');
        })

      Button('Width 50%').width('50%')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Width 50% clicked');
        })

      Button('Width 80%').width('80%')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Width 80% clicked');
        })

      Button('Height 30').height(30)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Height 30 clicked');
        })

      Button('Height 50').height(50)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Height 50 clicked');
        })

      Button('Height 80').height(80)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Height 80 clicked');
        })

      Button('Width+Height 150x60').width(150).height(60)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Width+Height 150x60 clicked');
        })

      Button('Width+Height 200x40').width(200).height(40)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Width+Height 200x40 clicked');
        })

      Button('Size 120x40').size({ width: 120, height: 40 } as SizeOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Size 120x40 clicked');
        })

      Button('Size 180x50').size({ width: 180, height: 50 } as SizeOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: Size 180x50 clicked');
        })

      Button('AspectRatio 2').aspectRatio(2)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: AspectRatio 2 clicked');
        })

      Button('AspectRatio 0.5').aspectRatio(0.5)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section12: AspectRatio 0.5 clicked');
        })

      Button('Dynamic Width').width(this.buttonWidth)
        .onClick((e: ClickEvent) => {
          if (this.buttonWidth === '80%') {
            this.buttonWidth = '50%'
          } else if (this.buttonWidth === '50%') {
            this.buttonWidth = '100%'
          } else {
            this.buttonWidth = '80%'
          }
          hilog.info(0x0000, 'testTag', 'Section12: Dynamic Width clicked, width: ' + this.buttonWidth);
        })

      Button('Dynamic Height').height(this.buttonHeight)
        .onClick((e: ClickEvent) => {
          if (this.buttonHeight === 40) {
            this.buttonHeight = 60
          } else if (this.buttonHeight === 60) {
            this.buttonHeight = 80
          } else {
            this.buttonHeight = 40
          }
          hilog.info(0x0000, 'testTag', 'Section12: Dynamic Height clicked, height: ' + this.buttonHeight);
        })

      // === Section 13: Margin and Padding Tests ===
      Text('=== Section 13: Margin and Padding Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Margin top 10').margin({ top: 10 } as Margin)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Margin top 10 clicked');
        })

      Button('Margin bottom 10').margin({ bottom: 10 } as Margin)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Margin bottom 10 clicked');
        })

      Button('Margin left 10').margin({ left: 10 } as Margin)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Margin left 10 clicked');
        })

      Button('Margin right 10').margin({ right: 10 } as Margin)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Margin right 10 clicked');
        })

      Button('Margin all 5').margin(5)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Margin all 5 clicked');
        })

      Button('Margin all 15').margin(15)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Margin all 15 clicked');
        })

      Button('Margin top5 bottom10 left5 right10').margin({
        top: 5, bottom: 10, left: 5, right: 10
      } as Margin)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Margin top5 bottom10 left5 right10 clicked');
        })

      Button('Padding all 5').padding(5)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Padding all 5 clicked');
        })

      Button('Padding all 10').padding(10)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Padding all 10 clicked');
        })

      Button('Padding top5 bottom10').padding({
        top: 5, bottom: 10
      } as Padding)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Padding top5 bottom10 clicked');
        })

      Button('Padding left10 right10').padding({
        left: 10, right: 10
      } as Padding)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section13: Padding left10 right10 clicked');
        })

      Button('Dynamic Margin').margin(this.buttonMarginValue)
        .onClick((e: ClickEvent) => {
          this.buttonMarginValue = this.buttonMarginValue + 3
          if (this.buttonMarginValue > 20) {
            this.buttonMarginValue = 5
          }
          hilog.info(0x0000, 'testTag', 'Section13: Dynamic Margin clicked, margin: ' + this.buttonMarginValue);
        })

      Button('Dynamic Padding').padding(this.buttonPaddingValue)
        .onClick((e: ClickEvent) => {
          this.buttonPaddingValue = this.buttonPaddingValue + 2
          if (this.buttonPaddingValue > 20) {
            this.buttonPaddingValue = 8
          }
          hilog.info(0x0000, 'testTag', 'Section13: Dynamic Padding clicked, padding: ' + this.buttonPaddingValue);
        })

      // === Section 14: Click Event Tests ===
      Text('=== Section 14: Click Event Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Simple Click')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section14: Simple Click clicked');
        })

      Button('Click with Event Info')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section14: Click with Event Info, x: ' + e.x + ', y: ' + e.y);
        })

      Button('Click with Event Target')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section14: Click with Event Target, eventTarget: ' + JSON.stringify(e.target));
        })

      Button('Click Count: ' + this.clickCount)
        .onClick((e: ClickEvent) => {
          this.incrementClickCount()
        })

      Button('Change Value on Click')
        .onClick((e: ClickEvent) => {
          this.changeValue()
          hilog.info(0x0000, 'testTag', 'Section14: Change Value on Click, stateVar: ' + this.stateVar);
        })

      Button('Toggle Visibility')
        .onClick((e: ClickEvent) => {
          this.toggleVisibility()
          hilog.info(0x0000, 'testTag', 'Section14: Toggle Visibility, visibility: ' + this.buttonVisibility);
        })

      Button('Toggle Opacity')
        .onClick((e: ClickEvent) => {
          this.cycleOpacity()
          hilog.info(0x0000, 'testTag', 'Section14: Toggle Opacity, opacity: ' + this.buttonOpacity);
        })

      Button('Toggle FontStyle')
        .onClick((e: ClickEvent) => {
          this.toggleFontStyle()
          hilog.info(0x0000, 'testTag', 'Section14: Toggle FontStyle, fontStyle: ' + this.buttonFontStyle);
        })

      Button('Toggle StateEffect')
        .onClick((e: ClickEvent) => {
          this.toggleStateEffect()
          hilog.info(0x0000, 'testTag', 'Section14: Toggle StateEffect, stateEffect: ' + this.buttonStateEffect);
        })

      Button('Toggle ButtonType')
        .onClick((e: ClickEvent) => {
          this.toggleButtonType()
          hilog.info(0x0000, 'testTag', 'Section14: Toggle ButtonType, type: ' + this.buttonType);
        })

      Button('Toggle ButtonRole')
        .onClick((e: ClickEvent) => {
          this.toggleButtonRole()
          hilog.info(0x0000, 'testTag', 'Section14: Toggle ButtonRole, role: ' + this.buttonRole);
        })

      Button('Toggle ButtonStyle')
        .onClick((e: ClickEvent) => {
          this.toggleButtonStyle()
          hilog.info(0x0000, 'testTag', 'Section14: Toggle ButtonStyle, style: ' + this.buttonStyle);
        })

      Button('Cycle FontColor')
        .onClick((e: ClickEvent) => {
          this.cycleButtonColor()
          hilog.info(0x0000, 'testTag', 'Section14: Cycle FontColor, color: ' + this.buttonColor);
        })

      Button('Cycle FontSize')
        .onClick((e: ClickEvent) => {
          this.cycleFontSize()
          hilog.info(0x0000, 'testTag', 'Section14: Cycle FontSize, size: ' + this.buttonFontSize);
        })

      Button('Cycle FontWeight')
        .onClick((e: ClickEvent) => {
          this.cycleFontWeight()
          hilog.info(0x0000, 'testTag', 'Section14: Cycle FontWeight, weight: ' + this.buttonFontWeight);
        })

      Button('Reset All States')
        .onClick((e: ClickEvent) => {
          this.buttonType = ButtonType.Capsule
          this.buttonRole = ButtonRole.NORMAL
          this.buttonStyle = ButtonStyleMode.EMPHASIZED
          this.buttonColor = Color.Blue
          this.buttonFontSize = 16
          this.buttonFontWeight = FontWeight.Normal
          this.buttonFontStyle = FontStyle.Normal
          this.buttonStateEffect = true
          this.buttonVisibility = Visibility.Visible
          this.buttonOpacity = 1
          this.clickCount = 0
          hilog.info(0x0000, 'testTag', 'Section14: All states reset');
        })

      // === Section 15: Dynamic State Change Tests ===
      Text('=== Section 15: Dynamic State Change Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Dynamic Type: ' + this.buttonType)
        .type(this.buttonType)
        .onClick((e: ClickEvent) => {
          this.toggleButtonType()
        })

      Button('Dynamic Role: ' + this.buttonRole)
        .role(this.buttonRole)
        .onClick((e: ClickEvent) => {
          this.toggleButtonRole()
        })

      Button('Dynamic Style: ' + this.buttonStyle)
        .buttonStyle(this.buttonStyle)
        .onClick((e: ClickEvent) => {
          this.toggleButtonStyle()
        })

      Button('Dynamic FontColor Button')
        .fontColor(this.buttonColor)
        .onClick((e: ClickEvent) => {
          this.cycleButtonColor()
        })

      Button('Dynamic FontSize Button')
        .fontSize(this.buttonFontSize)
        .onClick((e: ClickEvent) => {
          this.cycleFontSize()
        })

      Button('Dynamic FontWeight Button')
        .fontWeight(this.buttonFontWeight)
        .onClick((e: ClickEvent) => {
          this.cycleFontWeight()
        })

      Button('Dynamic FontStyle Button')
        .fontStyle(this.buttonFontStyle)
        .onClick((e: ClickEvent) => {
          this.toggleFontStyle()
        })

      Button('Dynamic StateEffect Button')
        .stateEffect(this.buttonStateEffect)
        .onClick((e: ClickEvent) => {
          this.toggleStateEffect()
        })

      Button('Dynamic Visibility Button')
        .visibility(this.buttonVisibility)
        .onClick((e: ClickEvent) => {
          this.toggleVisibility()
        })

      Button('Dynamic Opacity Button')
        .opacity(this.buttonOpacity)
        .onClick((e: ClickEvent) => {
          this.cycleOpacity()
        })

      Button('Dynamic Width Button')
        .width(this.buttonWidth)
        .onClick((e: ClickEvent) => {
          if (this.buttonWidth === '80%') {
            this.buttonWidth = '50%'
          } else {
            this.buttonWidth = '80%'
          }
        })

      Button('Dynamic Height Button')
        .height(this.buttonHeight)
        .onClick((e: ClickEvent) => {
          if (this.buttonHeight === 40) {
            this.buttonHeight = 60
          } else {
            this.buttonHeight = 40
          }
        })

      Button('Dynamic BorderRadius Button')
        .borderRadius(this.buttonBorderRadius)
        .onClick((e: ClickEvent) => {
          this.buttonBorderRadius = this.buttonBorderRadius + 5
          if (this.buttonBorderRadius > 30) {
            this.buttonBorderRadius = 0
          }
        })

      Button('Dynamic BorderWidth Button')
        .borderWidth(this.buttonBorderWidth)
        .borderColor(Color.Red)
        .onClick((e: ClickEvent) => {
          this.buttonBorderWidth = this.buttonBorderWidth + 1
          if (this.buttonBorderWidth > 5) {
            this.buttonBorderWidth = 1
          }
        })

      Button('Dynamic All Combined')
        .type(this.buttonType)
        .role(this.buttonRole)
        .buttonStyle(this.buttonStyle)
        .fontColor(this.buttonColor)
        .fontSize(this.buttonFontSize)
        .fontWeight(this.buttonFontWeight)
        .fontStyle(this.buttonFontStyle)
        .stateEffect(this.buttonStateEffect)
        .visibility(this.buttonVisibility)
        .opacity(this.buttonOpacity)
        .onClick((e: ClickEvent) => {
          this.toggleButtonType()
          this.toggleButtonRole()
          this.toggleButtonStyle()
          this.cycleButtonColor()
          this.cycleFontSize()
          this.cycleFontWeight()
          this.toggleFontStyle()
          hilog.info(0x0000, 'testTag', 'Section15: All dynamic states changed simultaneously');
        })

      // === Section 16: Edge Case Tests ===
      Text('=== Section 16: Edge Case Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Empty text Button clicked');
        })

      Button(' ')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Space text Button clicked');
        })

      Button('A very long button text that exceeds normal width to test text overflow behavior')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Long text Button clicked');
        })

      Button('Special: !@#$%^&*()')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Special chars Button clicked');
        })

      Button('Unicode: Hello')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Unicode Button clicked');
        })

      Button('Numbers: 1234567890')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Numbers Button clicked');
        })

      Button('Mixed: abc123!@#')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Mixed content Button clicked');
        })

      Button('Newline\nText')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Newline text Button clicked');
        })

      Button('Tab\tText')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Tab text Button clicked');
        })

      Button('Dynamic Text Button')
        .onClick((e: ClickEvent) => {
          if (this.buttonText === 'Test Button') {
            this.buttonText = 'Changed Text'
          } else if (this.buttonText === 'Changed Text') {
            this.buttonText = 'Another Change'
          } else {
            this.buttonText = 'Test Button'
          }
          hilog.info(0x0000, 'testTag', 'Section16: Dynamic text changed to: ' + this.buttonText);
        })

      Button(this.buttonText)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: Bound dynamic text Button clicked, text: ' + this.buttonText);
        })

      Button('FontSize 0 edge').fontSize(0)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section16: FontSize 0 edge clicked');
        })

      // === Section 17: Visibility Tests ===
      Text('=== Section 17: Visibility Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Visibility Visible')
        .visibility(Visibility.Visible)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section17: Visibility Visible clicked');
        })

      Button('Visibility Hidden')
        .visibility(Visibility.Hidden)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section17: Visibility Hidden clicked (should not be visible)');
        })

      Button('Visibility None')
        .visibility(Visibility.None)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section17: Visibility None clicked (should not be visible)');
        })

      Button('Toggle to Visible')
        .visibility(this.buttonVisibility)
        .onClick((e: ClickEvent) => {
          this.buttonVisibility = Visibility.Visible
          hilog.info(0x0000, 'testTag', 'Section17: Toggle to Visible');
        })

      Button('Toggle to Hidden')
        .visibility(this.buttonVisibility)
        .onClick((e: ClickEvent) => {
          this.buttonVisibility = Visibility.Hidden
          hilog.info(0x0000, 'testTag', 'Section17: Toggle to Hidden');
        })

      Button('Toggle to None')
        .visibility(this.buttonVisibility)
        .onClick((e: ClickEvent) => {
          this.buttonVisibility = Visibility.None
          hilog.info(0x0000, 'testTag', 'Section17: Toggle to None');
        })

      // === Section 18: Opacity Tests ===
      Text('=== Section 18: Opacity Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Opacity 1.0').opacity(1)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section18: Opacity 1.0 clicked');
        })

      Button('Opacity 0.8').opacity(0.8)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section18: Opacity 0.8 clicked');
        })

      Button('Opacity 0.5').opacity(0.5)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section18: Opacity 0.5 clicked');
        })

      Button('Opacity 0.3').opacity(0.3)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section18: Opacity 0.3 clicked');
        })

      Button('Opacity 0.1').opacity(0.1)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section18: Opacity 0.1 clicked');
        })

      Button('Opacity 0').opacity(0)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section18: Opacity 0 clicked (invisible)');
        })

      // === Section 19: Position and Alignment Tests ===
      Text('=== Section 19: Position and Alignment Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Position x:10 y:10').position({ x: 10, y: 10 } as Position)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: Position x:10 y:10 clicked');
        })

      Button('Position x:50 y:50').position({ x: 50, y: 50 } as Position)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: Position x:50 y:50 clicked');
        })

      Button('Position x:100 y:100').position({ x: 100, y: 100 } as Position)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: Position x:100 y:100 clicked');
        })

      Button('AlignSelf Start').alignSelf(ItemAlign.Start)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: AlignSelf Start clicked');
        })

      Button('AlignSelf Center').alignSelf(ItemAlign.Center)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: AlignSelf Center clicked');
        })

      Button('AlignSelf End').alignSelf(ItemAlign.End)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: AlignSelf End clicked');
        })

      Button('AlignSelf Stretch').alignSelf(ItemAlign.Stretch)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: AlignSelf Stretch clicked');
        })

      Button('AlignSelf Auto').alignSelf(ItemAlign.Auto)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: AlignSelf Auto clicked');
        })

      Button('AlignItems Center').align(Alignment.Center)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: AlignItems Center clicked');
        })

      Button('Align Top').align(Alignment.Top)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: Align Top clicked');
        })

      Button('Align Bottom').align(Alignment.Bottom)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: Align Bottom clicked');
        })

      Button('Align TopStart').align(Alignment.TopStart)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section19: Align TopStart clicked');
        })

      // === Section 20: Event Handler Tests ===
      Text('=== Section 20: Event Handler Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('onFocus Test')
        .onFocus(() => {
          this.isFocused = true
          hilog.info(0x0000, 'testTag', 'Section20: onFocus triggered');
        })
        .onBlur(() => {
          this.isFocused = false
          hilog.info(0x0000, 'testTag', 'Section20: onBlur triggered');
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Focus Test Button clicked, isFocused: ' + this.isFocused);
        })

      Button('onHover Test')
        .onHover((isHover: boolean) => {
          this.isHovered = isHover
          hilog.info(0x0000, 'testTag', 'Section20: onHover triggered, isHovered: ' + isHover);
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Hover Test Button clicked, isHovered: ' + this.isHovered);
        })

      Button('onTouch Test')
        .onTouch((event: TouchEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: onTouch triggered, type: ' + event.type);
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Touch Test Button clicked');
        })

      Button('onKeyEvent Test')
        .onKeyEvent((event: KeyEvent): boolean => {
          hilog.info(0x0000, 'testTag', 'Section20: onKeyEvent triggered, type: ' + event.type);
          return false
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: KeyEvent Test Button clicked');
        })

      Button('onAttach Test')
        .onAttach(() => {
          this.isAttached = true
          hilog.info(0x0000, 'testTag', 'Section20: onAttach triggered, isAttached: ' + this.isAttached);
        })
        .onDetach(() => {
          this.isAttached = false
          hilog.info(0x0000, 'testTag', 'Section20: onDetach triggered, isAttached: ' + this.isAttached);
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Attach Test Button clicked, isAttached: ' + this.isAttached);
        })

      Button('onAppear Test')
        .onAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section20: onAppear triggered');
        })
        .onDisAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section20: onDisAppear triggered');
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Appear Test Button clicked');
        })

      Button('Multiple Events Test')
        .onFocus(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Multiple Events onFocus');
        })
        .onBlur(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Multiple Events onBlur');
        })
        .onHover((isHover: boolean) => {
          hilog.info(0x0000, 'testTag', 'Section20: Multiple Events onHover: ' + isHover);
        })
        .onTouch((event: TouchEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Multiple Events onTouch: ' + event.type);
        })
        .onAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Multiple Events onAppear');
        })
        .onDisAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Multiple Events onDisAppear');
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Multiple Events onClick');
        })

      Button('Focus State Indicator')
        .onFocus(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Focus State Indicator gained focus');
        })
        .onBlur(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Focus State Indicator lost focus');
        })

      Button('Hover State Indicator')
        .onHover((isHover: boolean) => {
          if (isHover) {
            hilog.info(0x0000, 'testTag', 'Section20: Hover State Indicator - hovered');
          } else {
            hilog.info(0x0000, 'testTag', 'Section20: Hover State Indicator - not hovered');
          }
        })

      Button('onAreaChange Test')
        .onAreaChange((oldValue: Area, newValue: Area) => {
          hilog.info(0x0000, 'testTag', 'Section20: onAreaChange triggered');
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: AreaChange Test Button clicked');
        })

      Button('onSizeChange Test')
        .onSizeChange((oldValue: SizeOptions, newValue: SizeOptions) => {
          hilog.info(0x0000, 'testTag', 'Section20: onSizeChange triggered');
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: SizeChange Test Button clicked');
        })

      Button('onKeyEventDispatch Test')
        .onKeyEventDispatch((event: KeyEvent): boolean => {
          hilog.info(0x0000, 'testTag', 'Section20: onKeyEventDispatch triggered');
          return true
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: KeyEventDispatch Test Button clicked');
        })

      Button('onKeyPreIme Test')
        .onKeyPreIme((event: KeyEvent): boolean => {
          hilog.info(0x0000, 'testTag', 'Section20: onKeyPreIme triggered');
          return true
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: KeyPreIme Test Button clicked');
        })

      Button('Complete Event Chain')
        .onAttach(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onAttach');
        })
        .onAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onAppear');
        })
        .onFocus(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onFocus');
        })
        .onHover((isHover: boolean) => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onHover: ' + isHover);
        })
        .onTouch((event: TouchEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onTouch: ' + event.type);
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onClick');
        })
        .onBlur(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onBlur');
        })
        .onDisAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onDisAppear');
        })
        .onDetach(() => {
          hilog.info(0x0000, 'testTag', 'Section20: Complete Event Chain onDetach');
        })

      // === Section 21: StateStyles Tests ===
      Text('=== Section 21: StateStyles Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('StateStyles Normal+Pressed')
        .stateStyles({
          normal: this.normalButtonStyles,
          pressed: this.pressedButtonStyles,
        } as StateStyles)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles Normal+Pressed clicked');
        })

      Button('StateStyles Pressed Only')
        .stateStyles({
          pressed: this.pressedButtonStyles,
        } as StateStyles)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles Pressed Only clicked');
        })

      Button('StateStyles Normal Only')
        .stateStyles({
          normal: this.normalButtonStyles,
        } as StateStyles)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles Normal Only clicked');
        })

      Button('StateStyles Focused')
        .stateStyles({
          focused: this.focusedButtonStyles,
        } as StateStyles)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles Focused clicked');
        })

      Button('StateStyles Reset')
        .stateStyles(undefined)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles Reset clicked');
        })

      Button('StateStyles All States')
        .stateStyles({
          normal: this.normalButtonStyles,
          pressed: this.pressedButtonStyles,
          focused: this.focusedButtonStyles,
        } as StateStyles)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles All States clicked');
        })

      Button('StateStyles + Normal Type')
        .type(ButtonType.Normal)
        .stateStyles({
          normal: this.normalButtonStyles,
          pressed: this.pressedButtonStyles,
        } as StateStyles)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles + Normal Type clicked');
        })

      Button('StateStyles + Circle Type')
        .type(ButtonType.Circle)
        .stateStyles({
          normal: this.normalButtonStyles,
          pressed: this.pressedButtonStyles,
        } as StateStyles)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section21: StateStyles + Circle Type clicked');
        })

      // === Section 22: ClickEffect Tests ===
      Text('=== Section 22: ClickEffect Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('ClickEffect Light')
        .clickEffect({ level: ClickEffectLevel.LIGHT, scale: 1.05 } as ClickEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section22: ClickEffect Light clicked');
        })

      Button('ClickEffect Middle')
        .clickEffect({ level: ClickEffectLevel.MIDDLE, scale: 1.1 } as ClickEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section22: ClickEffect Middle clicked');
        })

      Button('ClickEffect Heavy')
        .clickEffect({ level: ClickEffectLevel.HEAVY, scale: 1.15 } as ClickEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section22: ClickEffect Heavy clicked');
        })

      Button('ClickEffect Light + Normal Type')
        .type(ButtonType.Normal)
        .clickEffect({ level: ClickEffectLevel.LIGHT, scale: 1.05 } as ClickEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section22: ClickEffect Light + Normal Type clicked');
        })

      Button('ClickEffect Middle + Circle Type')
        .type(ButtonType.Circle)
        .clickEffect({ level: ClickEffectLevel.MIDDLE, scale: 1.1 } as ClickEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section22: ClickEffect Middle + Circle Type clicked');
        })

      Button('ClickEffect Heavy + Error Role')
        .role(ButtonRole.ERROR)
        .clickEffect({ level: ClickEffectLevel.HEAVY, scale: 1.15 } as ClickEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section22: ClickEffect Heavy + Error Role clicked');
        })

      // === Section 23: Combined Attribute Tests ===
      Text('=== Section 23: Combined Attribute Tests ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 } as Margin).width('100%')

      Button('Normal+Bold+Blue+16', {
        type: ButtonType.Normal,
      } as ButtonOptions)
        .fontColor(Color.Blue)
        .fontWeight(FontWeight.Bold)
        .fontSize(16)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Normal+Bold+Blue+16 clicked');
        })

      Button('Circle+Italic+Red+20', {
        type: ButtonType.Circle,
      } as ButtonOptions)
        .fontColor(Color.Red)
        .fontWeight(FontWeight.Bold)
        .fontStyle(FontStyle.Italic)
        .fontSize(20)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Circle+Italic+Red+20 clicked');
        })

      Button('Capsule+Bolder+Green+24')
        .type(ButtonType.Capsule)
        .fontColor(Color.Green)
        .fontWeight(FontWeight.Bolder)
        .fontSize(24)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Capsule+Bolder+Green+24 clicked');
        })

      Button('Normal+Error+Emphasized')
        .type(ButtonType.Normal)
        .role(ButtonRole.ERROR)
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Normal+Error+Emphasized clicked');
        })

      Button('Circle+Normal+Normal Style')
        .type(ButtonType.Circle)
        .role(ButtonRole.NORMAL)
        .buttonStyle(ButtonStyleMode.NORMAL)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Circle+Normal+Normal Style clicked');
        })

      Button('Full Attr: Type+Role+Style+Font')
        .type(ButtonType.Normal)
        .role(ButtonRole.ERROR)
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .fontColor(Color.White)
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .fontStyle(FontStyle.Italic)
        .fontFamily('serif')
        .stateEffect(true)
        .backgroundColor(Color.Red)
        .borderRadius(10)
        .borderWidth(2)
        .borderColor(Color.Black)
        .borderStyle(BorderStyle.Solid)
        .width('70%')
        .height(50)
        .margin(10)
        .padding(5)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Full Attr Button clicked');
        })

      Button('Full Attr: Circle Type')
        .type(ButtonType.Circle)
        .role(ButtonRole.NORMAL)
        .buttonStyle(ButtonStyleMode.NORMAL)
        .fontColor(Color.Black)
        .fontSize(14)
        .fontWeight(FontWeight.Normal)
        .fontStyle(FontStyle.Normal)
        .fontFamily('monospace')
        .stateEffect(false)
        .backgroundColor(Color.Yellow)
        .borderRadius(30)
        .width(60)
        .height(60)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Full Attr Circle Type clicked');
        })

      Button('Full Attr: Capsule Type')
        .type(ButtonType.Capsule)
        .role(ButtonRole.ERROR)
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .fontColor(Color.White)
        .fontSize(16)
        .fontWeight(FontWeight.Bolder)
        .fontStyle(FontStyle.Italic)
        .fontFamily('Arial')
        .stateEffect(true)
        .backgroundColor(Color.Purple)
        .borderRadius(20)
        .borderWidth(1)
        .borderColor(Color.White)
        .borderStyle(BorderStyle.Dashed)
        .width('80%')
        .height(45)
        .margin(8)
        .padding(10)
        .opacity(0.9)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Full Attr Capsule Type clicked');
        })

      Button('Events + Styles Combined')
        .type(ButtonType.Normal)
        .fontColor(Color.White)
        .fontSize(16)
        .fontWeight(FontWeight.Bold)
        .backgroundColor(Color.Blue)
        .borderRadius(8)
        .stateStyles({
          normal: this.normalButtonStyles,
          pressed: this.pressedButtonStyles,
        } as StateStyles)
        .clickEffect({ level: ClickEffectLevel.LIGHT, scale: 1.05 } as ClickEffect)
        .onFocus(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Events+Styles onFocus');
        })
        .onBlur(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Events+Styles onBlur');
        })
        .onHover((isHover: boolean) => {
          hilog.info(0x0000, 'testTag', 'Section23: Events+Styles onHover: ' + isHover);
        })
        .onTouch((event: TouchEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Events+Styles onTouch: ' + event.type);
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Events+Styles onClick');
        })

      Button('Dynamic All + Events')
        .type(this.buttonType)
        .role(this.buttonRole)
        .buttonStyle(this.buttonStyle)
        .fontColor(this.buttonColor)
        .fontSize(this.buttonFontSize)
        .fontWeight(this.buttonFontWeight)
        .fontStyle(this.buttonFontStyle)
        .stateEffect(this.buttonStateEffect)
        .visibility(this.buttonVisibility)
        .opacity(this.buttonOpacity)
        .width(this.buttonWidth)
        .height(this.buttonHeight)
        .borderRadius(this.buttonBorderRadius)
        .borderWidth(this.buttonBorderWidth)
        .borderColor(this.buttonBorderColor)
        .onFocus(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic All + Events onFocus');
        })
        .onBlur(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic All + Events onBlur');
        })
        .onHover((isHover: boolean) => {
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic All + Events onHover: ' + isHover);
        })
        .onClick((e: ClickEvent) => {
          this.toggleButtonType()
          this.toggleButtonRole()
          this.toggleButtonStyle()
          this.cycleButtonColor()
          this.cycleFontSize()
          this.cycleFontWeight()
          this.toggleFontStyle()
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic All + Events onClick - all states cycled');
        })

      Button('Opacity + Visibility Combined')
        .opacity(this.buttonOpacity)
        .visibility(this.buttonVisibility)
        .onClick((e: ClickEvent) => {
          this.cycleOpacity()
          this.toggleVisibility()
          hilog.info(0x0000, 'testTag', 'Section23: Opacity+Visibility combined changed');
        })

      Button('Border + BorderRadius Combined')
        .border({
          width: 2,
          color: Color.Red,
          style: BorderStyle.Solid,
          radius: 15,
        } as BorderOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Border+BorderRadius combined clicked');
        })

      Button('Margin + Padding Combined')
        .margin({ top: 10, bottom: 10, left: 5, right: 5 } as Margin)
        .padding({ top: 5, bottom: 5, left: 10, right: 10 } as Padding)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Margin+Padding combined clicked');
        })

      Button('Position + Size Combined')
        .position({ x: 20, y: 20 } as Position)
        .size({ width: 150, height: 50 } as SizeOptions)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Position+Size combined clicked');
        })

      Button('Aspect + Opacity Combined')
        .aspectRatio(2)
        .opacity(0.8)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Aspect+Opacity combined clicked');
        })

      Button('StateStyles + ClickEffect Combined')
        .stateStyles({
          normal: this.normalButtonStyles,
          pressed: this.pressedButtonStyles,
          focused: this.focusedButtonStyles,
        } as StateStyles)
        .clickEffect({ level: ClickEffectLevel.MIDDLE, scale: 1.1 } as ClickEffect)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: StateStyles+ClickEffect combined clicked');
        })

      Button('Font All Combined')
        .fontColor(Color.Purple)
        .fontSize(22)
        .fontWeight(FontWeight.Bold)
        .fontStyle(FontStyle.Italic)
        .fontFamily('Courier New')
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Font all combined clicked');
        })

      Button('Type + Role + Style + Font + Bg + Border')
        .type(ButtonType.Normal)
        .role(ButtonRole.ERROR)
        .buttonStyle(ButtonStyleMode.EMPHASIZED)
        .fontColor(Color.White)
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .fontStyle(FontStyle.Normal)
        .fontFamily('sans-serif')
        .stateEffect(true)
        .backgroundColor(0xFF6600)
        .borderRadius(12)
        .borderWidth(2)
        .borderColor(Color.White)
        .borderStyle(BorderStyle.Solid)
        .width('75%')
        .height(55)
        .margin(8)
        .padding(8)
        .opacity(0.95)
        .clickEffect({ level: ClickEffectLevel.HEAVY, scale: 1.15 } as ClickEffect)
        .onFocus(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onFocus');
        })
        .onBlur(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onBlur');
        })
        .onHover((isHover: boolean) => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onHover: ' + isHover);
        })
        .onTouch((event: TouchEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onTouch: ' + event.type);
        })
        .onAttach(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onAttach');
        })
        .onDetach(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onDetach');
        })
        .onAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onAppear');
        })
        .onDisAppear(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onDisAppear');
        })
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Mega Combined onClick at x: ' + e.x + ', y: ' + e.y);
        })

      Button('Dynamic Mega Combined')
        .type(this.buttonType)
        .role(this.buttonRole)
        .buttonStyle(this.buttonStyle)
        .fontColor(this.buttonColor)
        .fontSize(this.buttonFontSize)
        .fontWeight(this.buttonFontWeight)
        .fontStyle(this.buttonFontStyle)
        .fontFamily(this.buttonFontFamily)
        .stateEffect(this.buttonStateEffect)
        .visibility(this.buttonVisibility)
        .opacity(this.buttonOpacity)
        .width(this.buttonWidth)
        .height(this.buttonHeight)
        .borderRadius(this.buttonBorderRadius)
        .borderWidth(this.buttonBorderWidth)
        .borderColor(this.buttonBorderColor)
        .borderStyle(this.buttonBorderStyle)
        .stateStyles({
          normal: this.normalButtonStyles,
          pressed: this.pressedButtonStyles,
          focused: this.focusedButtonStyles,
        } as StateStyles)
        .clickEffect({ level: ClickEffectLevel.MIDDLE, scale: 1.1 } as ClickEffect)
        .onFocus(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic Mega Combined onFocus');
        })
        .onBlur(() => {
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic Mega Combined onBlur');
        })
        .onHover((isHover: boolean) => {
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic Mega Combined onHover: ' + isHover);
        })
        .onTouch((event: TouchEvent) => {
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic Mega Combined onTouch: ' + event.type);
        })
        .onClick((e: ClickEvent) => {
          this.toggleButtonType()
          this.toggleButtonRole()
          this.toggleButtonStyle()
          this.cycleButtonColor()
          this.cycleFontSize()
          this.cycleFontWeight()
          this.toggleFontStyle()
          this.toggleStateEffect()
          this.toggleVisibility()
          this.cycleOpacity()
          hilog.info(0x0000, 'testTag', 'Section23: Dynamic Mega Combined onClick - all states cycled');
        })

      Text('=== End of Test Cases ===').fontSize(16).fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 20 } as Margin).width('100%')
    }
  }

  build() {
    NavDestination() {
      Column(undefined) {
      Text('Hello World').fontSize(20)
      Button('OK',
        {
          type: ButtonType.Normal,
          stateEffect: false,
        } as ButtonOptions).fontColor(Color.Pink).fontWeight(600).fontSize(30)
      Button(this.message)
        .backgroundColor('#FFFF00FF')
        .type(ButtonType.Circle)
        .stateEffect(false)
        .role(ButtonRole.ERROR)
        .onClick((e: ClickEvent) => {
          hilog.info(0x0000, 'testTag', 'On Click');
          this.changeValue()
        })
      Text(this.stateVar).fontSize(20)
      this.BuildButtonTestCases()
      Child({ stateVar: this.stateVar } as __Options_Child)
    }
    }
    .title('Button基础功能测试')
  }
}

@Component
struct Child {
  @State stateVar: string = 'Child';

  build() {
    Text(this.stateVar).fontSize(50)
  }
}