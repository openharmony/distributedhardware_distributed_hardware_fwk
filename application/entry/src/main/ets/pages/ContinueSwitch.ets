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
import { EditableLeftIconType, EditableTitleBar } from '@ohos.arkui.advanced.EditableTitleBar';
import mediaQuery from '@ohos.mediaquery';
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import CommonConstants, { FontSizeScale } from '../common/constants/CommonConstants';
import { logger } from '../utils/Logger';
import { TipsJumpUtils } from '../utils/TipsJumpUtils';
import osAccount from '@ohos.account.osAccount';
import systemParameterEnhance from '@ohos.systemParameterEnhance';
import i18n from '@ohos.i18n';
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
  @State is2in1: boolean = false;
  @State portraitFunc: mediaQuery.MediaQueryResult | void | null = null;
  @State isVideoVisible: Visibility = Visibility.Hidden;
  @State contentHeight: number = 0;
  @State imageArray: Array<ImageFrameInfo> = [];
  @State animationState: AnimationStatus = AnimationStatus.Running;
  @State reverse: boolean = false;
  @State iterations: number = -1;
  @State isEnabled: boolean = true;
  @State isShowBack: boolean = true;
  @State isAnimatorDone: boolean = false;
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
  private listener: mediaQuery.MediaQueryListener = mediaQuery.matchMediaSync('(dark-mode:true)');
  private extContext?: common.UIExtensionContext;
  private scroller: Scroller = new Scroller();
  private learnMore: ResourceStr = $r('app.string.learn_more');
  private continueDesc: ResourceStr = $r('app.string.continue_desc_text', '');
  private accountManager: osAccount.AccountManager = osAccount.getAccountManager();
  private startReason?: string = '';
  private isSmallFoldProduct: boolean = DeviceUtil.isSmallFoldProduct();

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
    let fontSizeScale = parseFloat(systemParameterEnhance.getSync(CommonConstants.FONT_SIZE_SCALE_PARAM, '1'));
    this.phoneSwitchTextTopMargin = this.UpdateMarginBasedOnFontSize(17, fontSizeScale);
    this.phoneSwitchTextBottomMargin = this.UpdateMarginBasedOnFontSize(18, fontSizeScale);
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
    if (deviceInfo.deviceType == 'phone') {
      this.gapLength = CommonConstants.GENERAL_PHONE_GAP_LENGTH;
    } else if (deviceInfo.deviceType == '2in1' || deviceInfo.deviceType == 'tablet') {
      this.gapLength = CommonConstants.PC_PAD_GAP_LENGTH;
    }
    logger.info(`${TAG} this.gapLength : ${this.gapLength}`);
  }

  getForegroundOsAccountLocalId(): void {
    logger.info(`${TAG} getForegroundOsAccountLocalId in`);
    try {
      this.accountManager.getForegroundOsAccountLocalId().then((localId: number) => {
        logger.info(`${TAG} getForegroundOsAccountLocalId, localId: ${localId}`);
        this.getAccountInfo(localId);
      }).catch((err: BusinessError) => {
        logger.error(`${TAG} getForegroundOsAccountLocalId errCode: ${err?.code}`);
      });
    } catch (err) {
      logger.error(`${TAG} getForegroundOsAccountLocalId exception: ${err?.message}`);
    }
  }

  getAccountInfo(localId: number): void {
    logger.info(`${TAG} getAccountInfo in`);
    try {
      this.accountManager.queryOsAccountById(localId).then((accountInfo: osAccount.OsAccountInfo) => {
        logger.info(`${TAG} queryOsAccountById, accountInfo.type: ${accountInfo.type}`);
        if (accountInfo.type === osAccount.OsAccountType.PRIVATE) {
          let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY, switchState.close,
            settings.domainName.USER_SECURITY);
          this.isSwitchOn = false;
          this.isEnabled = false;
          logger.info(`${TAG} set value isSuccess : status = ${status}; set:Continue_Switch_Status is 0`);
        }
      }).catch((err: BusinessError) => {
        logger.error(`${TAG} queryOsAccountById errCode: ${err?.code}`);
      });
    } catch (err) {
      logger.error(`${TAG} queryOsAccountById exception: ${err?.message}`);
    }
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
    this.is2in1 = deviceInfo.deviceType === '2in1' ? true : false;
  }

  aboutToAppear() {
    logger.info(`${TAG} aboutToAppear in`);
    this.initSwitchStatus();
    this.getStringSync();
    this.getImageArray();
    this.getForegroundOsAccountLocalId();
    this.listener.on('change', (mediaQueryResult: mediaQuery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    })
    this.registerFoldChangeListener();
    this.extContext = localStorage.get<common.UIExtensionContext>('context');
    this.startReason = AppStorage.get<string>('startReason');
    this.isShowBack = AppStorage.get<boolean>('isShowBack') ?? true;
    logger.info(`${TAG} aboutToAppear: startReason is ${this.startReason}, isShowBack: ${this.isShowBack}`);
    this.phoneSwitchTextMarginInit();
    setTimeout(() => {
      this.isAnimatorDone = true;
    }, 20)
  }

  aboutToDisappear() {
    logger.info(`${TAG} aboutToDisappear in`);
    this.unRegisterFoldChangeListener();
  }

  onBackPress() {
    logger.info(`${TAG} onBackPress in`);
  }

  @Builder
  NormalRootContent() {
    this.titleBar();
    this.ContentBuilder();
  }

  @Builder
  SearchRootContent() {
    NavDestination() {
      this.ContentBuilder();
    }
    .hideTitleBar(false)
    .title(this.title, {
      paddingStart: this.is2in1 ? this.paddingStartPc : this.paddingStartNormal,
    })
    .backgroundColor($r('sys.color.ohos_id_color_titlebar_sub_bg'))
  }

  @Builder
  titleBar() {
    Column() {
      EditableTitleBar({
        leftIconStyle: EditableLeftIconType.Back,
        title: $r('app.string.continue_title'),
        isSaveIconRequired: false,
        contentMargin: this.is2in1 ? this.titleBarMarginPc : this.titleBarMargin,
        onCancel: () => {
          logger.info(`${TAG} onCancel in.`);
          if (this.isBacked) {
            logger.info(`${TAG} onCancel: The back button has been clicked.`);
            return;
          }
          try {
            if (this.continueSession) {
              this.continueSession.sendData({ 'action': 'pop' });
              this.isBacked = true;
            } else {
              logger.error(`${TAG} continueSession is undefined`);
            }
          } catch (error) {
            logger.error(`${TAG} continueSession sendData failed. error.message: ${error.message}`);
          }
        }
      })
    }
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
              TipsJumpUtils.jumpTips(getContext(this) as common.UIAbilityContext, CommonConstants.FUN_NUM,
                CommonConstants.TIPS_TYPE);
            })
        }
        .margin({
          bottom: CommonConstants.CONTINUE_DESC_TEXT_MARGIN_BOTTOM,
          top: CommonConstants.CONTINUE_DESC_TEXT_MARGIN_TOP
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
                top: this.is2in1 ? CommonConstants.ITEM_LIST_PADDING_TOP_PC : this.phoneSwitchTextTopMargin,
                bottom: this.is2in1 ? CommonConstants.ITEM_LIST_PADDING_BOTTOM_PC : this.phoneSwitchTextBottomMargin
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
          .borderRadius(this.is2in1 ? CommonConstants.PC_BORDER_RADIUS : CommonConstants.NON_PC_BORDER_RADIUS)
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
              .margin(this.isMirrorLanguages() ? { left: CommonConstants.SYMBOL_MARGIN_RIGHT } :
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
              .lineHeight(this.isTibetanLanguages() ? 19 : 0)
          }
          .padding({
            left: CommonConstants.TEXT_LIST_ALIGN_DISTANCE,
            right: CommonConstants.TEXT_LIST_ALIGN_DISTANCE
          })
        }.width('100%')

      }
      .width('100%')
      .padding({ left: this.gapLength, right: this.gapLength })
      .margin({ bottom: this.contentHeight * 0.09 })
      .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
      .visibility(this.getContentVisibility())
    }
    .width('100%')
    .height(this.screenHeight)
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
    .onAreaChange((oldArea: Area, newArea: Area) => {
      logger.info(`${TAG} onAreaChange: newArea.width = ${newArea.width}, newArea.height = ${newArea.height}`);
      if (typeof newArea.width === 'number' && typeof newArea.height === 'number') {
        let width = newArea.width as number;
        let halfHeight = (newArea.height as number) / 2;
        try {
          let displayInfo: display.Display | undefined = display.getDefaultDisplaySync();
          if (displayInfo) {
            halfHeight = px2vp(displayInfo.height / 2);
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
    })
  }

  private getContentVisibility(): Visibility {
    return (this.imageAnimatorWidth !== 0 && this.isAnimatorDone) ? Visibility.Visible : Visibility.Hidden;
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

  private isPhone(): boolean {
    logger.info(`${TAG} isPhone in`);
    return (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'default');
  }

  /**
   * 根据折叠状态更新是否显示返回按钮
   */
  private refreshFoldStatus(foldStatus: display.FoldStatus): void {
    logger.info(`${TAG} refreshFoldStatus in. foldStatus: ${foldStatus}. startReason: ${this.startReason}`);
    this.isShowBack = (this.startReason !== 'from_search' || display.FoldStatus.FOLD_STATUS_FOLDED);
    logger.info(`${TAG} refreshFoldStatus end. this.isShowBack: ${this.isShowBack}.`);
  }

  /**
   * 折叠状态发生变化
   */
  private displayCallback(data: display.FoldStatus): CallBack<display.FoldStatus> {
    this.refreshFoldStatus(data)
  }

  private registerFoldChangeListener(): void {
    logger.info(`${TAG} registerFoldChangeListener in.isSmallFoldProduct: ${this.isSmallFoldProduct}`);
    try {
      // 首次非L+V折叠刷新isShowBack
      if (!this.isSmallFoldProduct && display.isFoldable()) {
        let data: display.FoldStatus = display.getFoldStatus();
        this.refreshFoldStatus(data);
      }
      display.on('foldStatusChange', this.displayCallback);
    } catch (err) {
      logger.error(`${TAG} registerFoldChangeListener failed. err.message: ${err.message}`);
    }
  }

  private unRegisterFoldChangeListener(): void {
    logger.info(`${TAG} unRegisterFoldChangeListener in.`);
    try {
      display.off('foldStatusChange', this.displayCallback);
    } catch (err) {
      logger.error(`${TAG} unRegisterFoldChangeListener failed. err.message: ${err.message}`);
    }
  }

  private isMirrorLanguages():
    boolean {
    logger.info(`${TAG} isMirrorLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    return CommonConstants.MIRROR_LANGUAGES.includes(locale);
  }

  private isTibetanLanguages():
    boolean {
    logger.info(`${TAG} isTibetanLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    logger.info(`${TAG} isTibetanLanguages: ${locale}`);
    return CommonConstants.TIBETAN_LANGUAGES.includes(locale);
  }
}