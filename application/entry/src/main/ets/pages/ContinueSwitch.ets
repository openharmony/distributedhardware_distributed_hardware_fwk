/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
import common from '@ohos.app.ability.common';
import display from '@ohos.display';
import settings from '@ohos.settings';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import deviceInfo from '@ohos.deviceInfo';
import { BusinessError } from '@ohos.base';
import mediaQuery from '@ohos.mediaquery';
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import CommonConstants, { FontSizeScale, LanguagesUtil } from '../common/constants/CommonConstants';
import { logger } from '../utils/Logger';
import { TipsJumpUtils } from '../utils/TipsJumpUtils';
import systemParameterEnhance from '@ohos.systemParameterEnhance';
import { LengthMetrics } from '@ohos.arkui.node';
import { DeviceUtil } from '../utils/DeviceUtil';

const TAG = '[ContinueSwitch_Page] : ';
let context = getContext(this) as common.UIAbilityContext;
let localStorage = LocalStorage.getShared();

interface switchStatus {
  open: string;
  close: string;
}

let switchState: switchStatus = {
  open: CommonConstants.SWITCH_STATUS_OPEN,
  close: CommonConstants.SWITCH_STATUS_CLOSE
}

@Entry(localStorage)
@Component
struct ContinueSwitch {
  @StorageLink('isSwitchOn') isSwitchOn: boolean | undefined = true;
  @StorageLink('continueSession') continueSession: UIExtensionContentSession | undefined = undefined;
  @State title: string = '';
  @State screenHeight: number = 0;
  @State screenWidth: number = 0;
  @State shortSideSize: number = 0;
  @State imageAnimatorHeight: number = this.getImageAnimatorHeight();
  @State imageAnimatorWidth: number = this.getImageAnimatorWidth();
  @State textWidth: number = 0;
  @State gapLength: number = 0;
  @State isShow: boolean = false;
  @State isPC: boolean = DeviceUtil.isPC();
  @State portraitFunc: mediaQuery.MediaQueryResult | void | null = null;
  @State isVideoVisible: Visibility = Visibility.Hidden;
  @State contentHeight: number = 0;
  @State imageArray: Array<ImageFrameInfo> = [];
  @State animationState: AnimationStatus = AnimationStatus.Running;
  @State reverse: boolean = false;
  @State iterations: number = -1;
  @State isEnabled: boolean = true;
  @State isShowBack: boolean = true;
  @StorageProp('currentFontSizeScale') @Watch('onFontSizeScaleChange') fontSizeScale: number = 1;
  @State phoneSwitchTextTopMargin: string = '17vp';
  @State phoneSwitchTextBottomMargin: string = '18vp';
  @State titleBarMargin: LocalizedMargin = {
    start: LengthMetrics.resource($r('sys.float.margin_left')),
  }
  @State titleBarMarginPc: LocalizedMargin = {
    start: LengthMetrics.vp(16),
  }
  @State paddingStartNormal: LengthMetrics = LengthMetrics.resource($r('sys.float.margin_left'));
  @State paddingStartPc: LengthMetrics = LengthMetrics.vp(16);
  @State isBacked: boolean = false;
  @State isAnimatorDone: boolean = false;
  private listener: mediaQuery.MediaQueryListener = mediaQuery.matchMediaSync('(dark-mode:true)');
  private extContext?: common.UIExtensionContext;
  private scroller: Scroller = new Scroller();
  private learnMore: ResourceStr = $r('app.string.learn_more');
  private continueDesc: ResourceStr = $r('app.string.continue_desc_text', '');
  private startReason?: string = '';
  private isSmallFoldProduct: boolean = DeviceUtil.isSmallFoldProduct();
  private pageInfos: NavPathStack = new NavPathStack();
  private transitionType: NavigationSystemTransitionType = NavigationSystemTransitionType.TITLE;

  onPortrait(mediaQueryResult: mediaQuery.MediaQueryResult): void {
    logger.info(`${TAG} 'onPortrait in`);
    if (mediaQueryResult.matches as boolean) {
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_DARK);
    } else {
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT);
    }
  }

  onFontSizeScaleChange(): void {
    logger.info(`${TAG} onFontSizeScaleChange`);
    this.phoneSwitchTextTopMargin = this.UpdateMarginBasedOnFontSize(17, this.fontSizeScale);
    this.phoneSwitchTextBottomMargin = this.UpdateMarginBasedOnFontSize(18, this.fontSizeScale);
  }

  /**
   * Update the margins of the switch list according to the font size.
   */
  public UpdateMarginBasedOnFontSize(fontFp: number, fontSizeScale: number): string {
    logger.info(`${TAG} getlistSpace, fontSizeScale: ${fontSizeScale} ; fontFp: ${fontFp}`);
    switch (fontSizeScale) {
      case FontSizeScale.XXL1:
        return '16vp';
      case FontSizeScale.XXL2:
        return '20vp';
      case FontSizeScale.XXL3:
        return '24vp';
      default:
        return `${fontFp}vp`;
    }
  }

  /**
   *  Initialize the switch list spacing size
   */
  public phoneSwitchTextMarginInit(): void {
    try {
      let fontSizeScale = parseFloat(systemParameterEnhance.getSync(CommonConstants.FONT_SIZE_SCALE_PARAM, '1'));
      this.phoneSwitchTextTopMargin = this.UpdateMarginBasedOnFontSize(17, fontSizeScale);
      this.phoneSwitchTextBottomMargin = this.UpdateMarginBasedOnFontSize(18, fontSizeScale);
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(`${TAG} systemParameterEnhance getSync failed,error code: ${code},message: ${message}.`);
    }
  }

  getStringSync(): void {
    logger.info(`${TAG} getStringSync in`);
    try {
      context.resourceManager.getStringValue($r('app.string.continue_title')
        .id, (error: BusinessError, value: string) => {
        if (error != null) {
          logger.error(TAG + 'error is ' + error);
        } else {
          this.title = value;
          logger.info(`${TAG} <aboutToAppear> this.title : ${this.title}`);
        }
      })
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(`${TAG} callback getStringValue failed,error code: ${code},message: ${message}.`);
    }
  }

  getImageArray(): void {
    logger.info(`${TAG} getImageArray in`);
    if (deviceInfo.deviceType === '2in1') {
      for (let i = 0; i <= CommonConstants.IMAGE_COUNT; ++i) {
        this.imageArray.push({
          src: $r(`app.media.continuePC_${i}`),
          duration: (i == CommonConstants.IMAGE_COUNT) ? CommonConstants.IMG_ANIMATOR_OVER_DURATION
            : CommonConstants.IMG_ANIMATOR_NORMAL_DURATION
        })
      }
    } else {
      for (let i = 0; i <= CommonConstants.IMAGE_COUNT; ++i) {
        this.imageArray.push({
          src: $r(`app.media.continue_${i}`),
          duration: (i == CommonConstants.IMAGE_COUNT) ? CommonConstants.IMG_ANIMATOR_OVER_DURATION
            : CommonConstants.IMG_ANIMATOR_NORMAL_DURATION
        })
      }
    }
  }

  getGapLength(): void {
    logger.info(`${TAG} getGapLength in, deviceInfo.deviceType : ${deviceInfo.deviceType}`);
    this.gapLength =
      DeviceUtil.isPhone() ? CommonConstants.GENERAL_PHONE_GAP_LENGTH : CommonConstants.PC_PAD_GAP_LENGTH;
    logger.info(`${TAG} this.gapLength : ${this.gapLength}`);
  }

  initSwitchStatus(): void {
    logger.info(`${TAG} initSwitchStatus in`);
    try {
      let value = settings.getValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY, switchState.open,
        settings.domainName.USER_SECURITY);
      this.isSwitchOn = value != switchState.close ? true : false;
      logger.info(`${TAG} <initSwitchStatus> this.isSwitchOn : ${this.isSwitchOn}; value: ${value}`);

      AppStorage.setOrCreate('isSwitchOn', this.isSwitchOn);
      logger.info(`${TAG} AppStorage.get<boolean>(isSwitchOn) : ${AppStorage.get<boolean>('isSwitchOn')}`);

      if (this.isSwitchOn) {
        let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY, switchState.open,
          settings.domainName.USER_SECURITY);
        logger.info(`${TAG} set value success :${status}; set:Continue_Switch_Status is 1`);
      }
    } catch (error) {
      logger.error(`${TAG} settings set or get failed. error.message: ${error.message}`);
    }
  }

  onPageShow() {
    logger.info(`${TAG} onPageShow in`);
    this.getGapLength();
    display.getAllDisplays((err, data) => {
      this.screenWidth = px2vp(data[0].width);
      this.screenHeight = px2vp(data[0].height);
      this.contentHeight = this.screenHeight;
      logger.info(`${TAG} screenWidth = ${this.screenWidth}; screenHeight = ${this.screenHeight}`);
    })
  }

  aboutToAppear() {
    logger.info(`${TAG} aboutToAppear in`);
    this.initSwitchStatus();
    this.getStringSync();
    this.getImageArray();
    this.listener.on('change', (mediaQueryResult: mediaQuery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    })
    this.extContext = localStorage.get<common.UIExtensionContext>('context');
    this.startReason = AppStorage.get<string>('startReason');
    this.isShowBack = AppStorage.get<boolean>('isShowBack') ?? true;
    if (this.isPC && this.startReason === 'from_search') {
      logger.info(`${TAG} aboutToAppear: pc & from_search`);
      this.isShowBack = false;
    }
    logger.info(`${TAG} aboutToAppear: startReason is ${this.startReason}, isShowBack: ${this.isShowBack}`);
    this.phoneSwitchTextMarginInit();
    setTimeout(() => {
      this.isAnimatorDone = true;
    }, 1)
  }

  aboutToDisappear() {
    logger.info(`${TAG} aboutToDisappear in`);
  }

  onBackPress() {
    logger.info(`${TAG} onBackPress in`);
  }

  @Builder
  PageMap(name: string) {
    if (name === CommonConstants.SETTING_PAGE_NAME) {
      this.NormalDestination();
    }
  }

  @Builder
  NormalRootContent() {
    Navigation(this.pageInfos) {
    }
    .mode(NavigationMode.Stack)
    .onAppear(()=>{
      this.pageInfos.pushPathByName(CommonConstants.SETTING_PAGE_NAME, undefined);
    })
    .navDestination(this.PageMap)
  }

  @Builder
  NormalDestination() {
    NavDestination() {
      this.ContentBuilder();
    }
    .hideTitleBar(false)
    .title(this.title, {
      paddingStart: this.isPC ? this.paddingStartPc : this.paddingStartNormal,
    })
    .backgroundColor($r('sys.color.ohos_id_color_titlebar_sub_bg'))
    .onShown(()=>{
      this.transitionType = NavigationSystemTransitionType.DEFAULT;
    })
    .systemTransition(this.transitionType)
    .onBackPressed(()=>{
      logger.info(`${TAG} onBackPressed in.`);
      if (this.isBacked) {
        logger.info(`${TAG} onBackPressed: The back button has been clicked.`);
        return true;
      }
      this.transitionType = NavigationSystemTransitionType.TITLE;
      this.pageInfos.pop(true);
      try {
        if (this.continueSession) {
          this.continueSession.sendData({ 'action': 'pop' });
          this.isBacked = true;
        } else {
          logger.error(`${TAG} continueSession is undefined`);
        }
      } catch (error) {
        let code: number = (error as BusinessError).code;
        let message: string = (error as BusinessError).message;
        logger.error(`${TAG} continueSession sendData failed. error.code: ${code}, message: ${message}`);
      }
      return false;
    })
  }

  @Builder
  SearchRootContent() {
    NavDestination() {
      this.ContentBuilder();
    }
    .hideTitleBar(false)
    .title(this.title, {
      paddingStart: this.isPC ? this.paddingStartPc : this.paddingStartNormal,
    })
    .backgroundColor($r('sys.color.ohos_id_color_titlebar_sub_bg'))
  }

  @Builder
  ContentBuilder() {
    Scroll(this.scroller) {
      Column() {
        ImageAnimator()
          .images(this.imageArray)
          .state(this.animationState)
          .reverse(this.reverse)
          .fillMode(this.iterations)
          .iterations(this.iterations)
          .width(this.imageAnimatorWidth)
          .height(this.imageAnimatorHeight)
          .onStart(() => {
            logger.info(`${TAG} ImageAnimator Start`);
          })
          .onFinish(() => {
            logger.info(`${TAG} ImageAnimator Finish`);
          })

        Text() {
          Span(this.continueDesc)
            .fontFamily('HarmonyHeiTi')
            .fontSize($r('sys.float.ohos_id_text_size_body2'))
            .fontColor($r('sys.color.ohos_id_color_text_secondary'))
            .fontWeight(FontWeight.Regular)
          Span(this.learnMore)
            .fontFamily('HarmonyHeiTi')
            .fontSize($r('sys.float.ohos_id_text_size_body2'))
            .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
            .fontWeight(FontWeight.Medium)
            .onClick(() => {
              logger.info(`${TAG} onClick learnMore`);
              TipsJumpUtils.jumpTips(getContext(this) as common.UIAbilityContext, CommonConstants.FUN_NUM,
                CommonConstants.TIPS_TYPE);
            })
        }
        .margin({
          bottom: CommonConstants.CONTINUE_DESC_TEXT_MARGIN_BOTTOM,
          top: CommonConstants.CONTINUE_DESC_TEXT_MARGIN_TOP,
        })
        .padding({
          left: DeviceUtil.isPhone() ? $r('app.float.margin_8') : 0,
          right: DeviceUtil.isPhone() ? $r('app.float.margin_8') : 0
        })
        .textAlign(TextAlign.Center)
        .width('100%')

        Column() {
          Flex({
            direction: FlexDirection.Row,
            justifyContent: FlexAlign.SpaceBetween,
            alignItems: ItemAlign.Center
          }) {
            Text($r('app.string.continue_title'))
              .fontSize($r('sys.float.ohos_id_text_size_sub_title2'))
              .fontWeight(FontWeight.Medium)
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .accessibilityLevel('no')
              .padding({
                top: this.isPC ? CommonConstants.ITEM_LIST_PADDING_TOP_PC : this.phoneSwitchTextTopMargin,
                bottom: this.isPC ? CommonConstants.ITEM_LIST_PADDING_BOTTOM_PC : this.phoneSwitchTextBottomMargin
              })

            Toggle({ type: ToggleType.Switch, isOn: this.isSwitchOn })
              .width(CommonConstants.CONTINUE_SWITCH_WIDTH)
              .height(CommonConstants.CONTINUE_SWITCH_HEIGHT)
              .hoverEffect(HoverEffect.None)
              .enabled(this.isEnabled)
              .onChange((isOn: boolean) => {
                logger.info(`${TAG} isOn: ${isOn}`);
                this.isSwitchOn = isOn;
                AppStorage.setAndLink('isSwitchOn', isOn);
                try {
                  if (isOn) {
                    let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY,
                      switchState.open, settings.domainName.USER_SECURITY);
                    logger.info(`${TAG} is set success :${status}; set:Continue_Switch_Status is open`);
                  } else {
                    let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY,
                      switchState.close, settings.domainName.USER_SECURITY);
                    logger.info(`${TAG} is set success :${status}; set:Continue_Switch_Status is close`);
                  }
                } catch (error) {
                  logger.error(`${TAG} settings.setValueSync failed. error.message: ${error.message}`);
                }
              })
          }
          .width('100%')
          .padding({
            left: CommonConstants.TEXT_LIST_ALIGN_DISTANCE,
            right: CommonConstants.TEXT_LIST_ALIGN_DISTANCE
          })
          .backgroundColor($r('sys.color.ohos_id_color_list_card_bg'))
          .borderRadius(this.isPC ? CommonConstants.PC_BORDER_RADIUS : CommonConstants.NON_PC_BORDER_RADIUS)
          .accessibilityText(this.title)
        }
        .width('100%')
        .constraintSize({
          minHeight: CommonConstants.PC_LIST_HEIGHT
        })

        Column() {
          Flex({
            direction: FlexDirection.Row,
            justifyContent: FlexAlign.Start,
            alignItems: ItemAlign.Center
          }) {
            SymbolGlyph($r('sys.symbol.info_circle_fill'))
              .fontWeight(FontWeight.Medium)
              .fontSize(CommonConstants.SYMBOL_INFO_CIRCLE)
              .fontColor([$r('sys.color.ohos_id_color_activated')])
              .margin(LanguagesUtil.isMirrorLanguages() ? { left: CommonConstants.SYMBOL_MARGIN_RIGHT } :
                { right: CommonConstants.SYMBOL_MARGIN_RIGHT })
              .width(CommonConstants.SYMBOL_INFO_CIRCLE)
              .height(CommonConstants.SYMBOL_INFO_CIRCLE)
              .accessibilityLevel('no')

            Text($r('app.string.update_version_prompt'))
              .fontSize($r('sys.float.ohos_id_text_size_body3'))
              .fontWeight(FontWeight.Medium)
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .textAlign(TextAlign.Start)
              .lineHeight(-1)
          }
          .margin({ top: CommonConstants.UPDATE_PROMPT_MARGIN_TOP })
        }
        .padding({
          left: CommonConstants.TEXT_LIST_ALIGN_DISTANCE,
          right: CommonConstants.TEXT_LIST_ALIGN_DISTANCE
        })

        Column() {
          Row() {
            Text($r('app.string.continue_privacy_text'))
              .fontSize($r('sys.float.ohos_id_text_size_body3'))
              .fontWeight(FontWeight.Regular)
              .margin({ top: CommonConstants.CONTINUE_PRIVACY_TEXT_MARGIN_TOP })
              .fontColor($r('sys.color.ohos_id_color_text_secondary'))
              .textAlign(TextAlign.Start)
              .width('100%')
              .lineHeight(LanguagesUtil.isTibetanLanguages() ? 19 : 0)
          }
          .padding({
            left: CommonConstants.TEXT_LIST_ALIGN_DISTANCE,
            right: CommonConstants.TEXT_LIST_ALIGN_DISTANCE
          })
        }.width('100%')

      }
      .width('100%')
      .padding({ left: this.gapLength, right: this.gapLength })
      .margin({ bottom: '44vp' })
      .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
      .visibility(this.getContentVisibility())
    }
    .width('100%')
    .height('100%')
    .scrollable(ScrollDirection.Vertical)
    .scrollBar(BarState.Off)
    .align(Alignment.TopStart)
    .friction(0.6)
    .edgeEffect(EdgeEffect.Spring)
    .onScrollEdge(() => {
      logger.info('To the edge');
    })
    .onScrollStop(() => {
      logger.info('Scroll Stop');
    })
    .onAreaChange((oldArea: Area, newArea: Area) => {
      logger.info(`${TAG} Scroll, oldArea.height = ${oldArea.height}, newArea.height = ${newArea.height}`);
    })

    ScrollBar({
      scroller: this.scroller,
      direction: ScrollBarDirection.Vertical,
      state: BarState.Auto
    })
      .position({ end: LengthMetrics.vp(0) })
      .margin({bottom: 28})
  }

  build() {
    Column() {
      if (this.isShowBack) {
        this.NormalRootContent();
      } else {
        this.SearchRootContent();
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM])
    .onAreaChange((oldArea: Area, newArea: Area) => {
      logger.info(`${TAG} onAreaChange: newArea.width = ${newArea.width}, newArea.height = ${newArea.height}`);
      this.getImageAnimatorSize(newArea);
    })
  }

  private getContentVisibility(): Visibility {
    return (this.imageAnimatorWidth !== 0 && this.isAnimatorDone) ? Visibility.Visible : Visibility.Hidden;
  }

  private getImageAnimatorSize(compArea:Area) {
    logger.info(`${TAG} getImageAnimatorSize in.`);
    if (typeof compArea.width === 'number' && typeof compArea.height === 'number') {
      let width = compArea.width as number;
      let halfHeight = (compArea.height as number) / 2;
      try {
        let displayInfo: display.Display | undefined = display.getDefaultDisplaySync();
        if (displayInfo) {
          halfHeight = !this.isPC && displayInfo.height > 0 ? px2vp(displayInfo.height / 2) : (compArea.height / 2);
          logger.info(`${TAG} display: halfHeight = ${halfHeight}`);
        }
      } catch (error) {
        logger.error(`${TAG} getDefaultDisplaySync failed. error.message: ${error.message}`);
      }
      if (width > halfHeight) {
        this.imageAnimatorWidth = halfHeight * 0.8;
        this.imageAnimatorHeight = this.imageAnimatorWidth * 2 / 3;
      } else {
        this.imageAnimatorWidth = width * 0.8;
        this.imageAnimatorHeight = this.imageAnimatorWidth * 2 / 3;
      }
    }
    logger.info(`${TAG} this.imageAnimatorWidth = ${this.imageAnimatorWidth}, this.imageAnimatorHeight = ${this.imageAnimatorHeight}`);
  }

  private getImageAnimatorHeight(): number {
    if (deviceInfo.deviceType === 'phone') {
      return CommonConstants.ANIMATOR_HEIGHT_PHONE;
    } else if (deviceInfo.deviceType === '2in1') {
      return CommonConstants.ANIMATOR_HEIGHT_PC;
    } else if (deviceInfo.deviceType === 'tablet') {
      return CommonConstants.ANIMATOR_HEIGHT_PAD;
    }
    return CommonConstants.ANIMATOR_HEIGHT_PHONE;
  }

  private getImageAnimatorWidth(): number {
    if (deviceInfo.deviceType === 'phone') {
      return CommonConstants.ANIMATOR_WIDTH_PHONE;
    } else if (deviceInfo.deviceType === '2in1') {
      return CommonConstants.ANIMATOR_WIDTH_PC;
    } else if (deviceInfo.deviceType === 'tablet') {
      return CommonConstants.ANIMATOR_WIDTH_PAD;
    }
    return CommonConstants.ANIMATOR_WIDTH_PHONE;
  }
}