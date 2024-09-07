/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the 'License');
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an 'AS IS' BASIS,
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
import CommonConstants from '../common/constants/CommonConstants';
import { logger } from '../utils/Logger';
import { TipsJumpUtils } from '../utils/TipsJumpUtils';
import osAccount from '@ohos.account.osAccount';

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

@Entry
@Component
struct ContinueSwitch {
  @StorageLink('isSwitchOn') isSwitchOn: boolean | undefined = true;
  @StorageLink('continueSession') continueSession: UIExtensionContentSession | undefined = undefined;
  @State title: string = '';
  @State screenHeight: number = 0;
  @State screenWidth: number = 0;
  @State shortSideSize: number = 0;
  @State imageAnimatorHeight: number = 0;
  @State imageAnimatorWidth: number = 0;
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
  private videoController: VideoController | undefined = undefined;
  private listener: mediaQuery.MediaQueryListener = mediaQuery.matchMediaSync('(dark-mode:true)');
  private extContext?: common.UIExtensionContext;
  private scroller: Scroller = new Scroller();
  private learnMore: ResourceStr = $r('app.string.learn_more');
  private continueDesc: ResourceStr = $r('app.string.continue_desc_text', '');
  private accountManager: osAccount.AccountManager = osAccount.getAccountManager();

  onPortrait(mediaQueryResult: mediaQuery.MediaQueryResult): void {
    logger.info(TAG + 'onPortrait in');
    if (mediaQueryResult.matches as boolean) {
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_DARK);
    } else {
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT);
    }
  }

  /**
   * @description Calculate the ImageAnimator size
   *   Calculation rule:
   *    1.general phone & pad : screen height/2 compare with width,shorter is ImageAnimator width
   *    2.pc: screen height/2 compare with width,shorter is ImageAnimator height
   *    3.ratio: 3:2
   * @param height
   * @param width
   */
  getImageAnimatorSize(height: Length, width: Length): void {
    logger.info(TAG + 'getImageAnimatorSize in, deviceInfo.deviceType : ' + deviceInfo.deviceType);
    let totalHeight = height as number;
    this.shortSideSize = width < totalHeight / 2 ? width as number : totalHeight / 2 as number;
    if (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'tablet') {
      this.imageAnimatorWidth = this.shortSideSize * 0.8;
      this.imageAnimatorHeight = this.shortSideSize * 0.8 * 2 / 3;
    } else if (deviceInfo.deviceType === '2in1') {
      this.imageAnimatorWidth = this.shortSideSize * 0.8 * 3 / 2;
      this.imageAnimatorHeight = this.shortSideSize * 0.8;
    }
    logger.info(TAG + 'this.shortSideSize = ' + this.shortSideSize + ', this.imageAnimatorWidth = ' +
    this.imageAnimatorWidth + ', this.imageAnimatorHeight = ' + this.imageAnimatorHeight);
  }

  getStringSync(): void {
    logger.info(TAG + 'getStringSync in');
    try {
      context.resourceManager.getStringValue($r('app.string.continue_title')
        .id, (error: BusinessError, value: string) => {
        if (error != null) {
          logger.error(TAG + 'error is ' + error);
        } else {
          this.title = value;
          logger.info(TAG + '<aboutToAppear> this.title : ' + this.title);
        }
      })
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(TAG + `callback getStringValue failed,error code: ${code},message: ${message}.`);
    }
  }

  getImageArray(): void {
    logger.info(TAG + 'getImageArray in');
    for (let i = 0; i <= CommonConstants.IMAGE_COUNT; ++i) {
      this.imageArray.push({
        src: $r(`app.media.continue_${i}`),
        duration: (i == CommonConstants.IMAGE_COUNT) ? CommonConstants.IMG_ANIMATOR_OVER_DURATION
          : CommonConstants.IMG_ANIMATOR_NORMAL_DURATION
      })
    }
  }

  getGapLength(): void {
    logger.info(TAG + 'getGapLength in, deviceInfo.deviceType : ' + deviceInfo.deviceType);
    if (deviceInfo.deviceType == 'phone') {
      this.gapLength = CommonConstants.GENERAL_PHONE_GAP_LENGTH;
    } else if (deviceInfo.deviceType == '2in1' || deviceInfo.deviceType == 'tablet') {
      this.gapLength = CommonConstants.PC_PAD_GAP_LENGTH;
    }
    logger.info(TAG + 'this.gapLength : ' + this.gapLength);
  }

  getForegroundOsAccountLocalId(): void {
    logger.info(TAG + 'getForegroundOsAccountLocalId in');
    try {
      this.accountManager.getForegroundOsAccountLocalId().then((localId: number) => {
        logger.info(TAG + 'getForegroundOsAccountLocalId, localId: ' + localId);
        this.getAccountInfo(localId);
      }).catch((err: BusinessError) => {
        logger.error(TAG + 'getForegroundOsAccountLocalId err: ' + JSON.stringify(err));
      });
    } catch (e) {
      logger.error(TAG + 'getForegroundOsAccountLocalId exception: ' + JSON.stringify(e));
    }
  }

  getAccountInfo(localId: number): void {
    logger.info(TAG + 'getAccountInfo in');
    try {
      this.accountManager.queryOsAccountById(localId).then((accountInfo: osAccount.OsAccountInfo) => {
        logger.info(TAG + 'queryOsAccountById, accountInfo.type: ' + JSON.stringify(accountInfo.type));
        if (accountInfo.type === osAccount.OsAccountType.PRIVATE) {
          let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY, switchState.close,
            settings.domainName.USER_SECURITY);
          this.isSwitchOn = false;
          this.isEnabled = false;
          logger.info(TAG + 'set value isSuccess : status =' + status + '; set:Continue_Switch_Status is 0');
        }
      }).catch((err: BusinessError) => {
        logger.error(TAG + 'queryOsAccountById err: ' + JSON.stringify(err));
      });
    } catch (e) {
      logger.error(TAG + 'queryOsAccountById exception: ' + JSON.stringify(e));
    }
  }

  @Builder
  NavigationTitle() {
    Column() {
      Text($r('app.string.continue_title'))
        .fontColor($r('sys.color.ohos_id_color_text_primary'))
        .fontSize($r('sys.float.ohos_id_text_size_headline8'))
        .lineHeight(45)
        .fontWeight(FontWeight.Bold)
    }
  }

  onPageShow() {
    logger.info(TAG + 'onPageShow in');
    this.getGapLength();
    display.getAllDisplays((err, data) => {
      this.screenWidth = px2vp(data[0].width);
      this.screenHeight = px2vp(data[0].height);
      this.contentHeight = this.screenHeight;
      logger.info(TAG + 'screenWidth = ' + this.screenWidth + '; screenHeight = ' + this.screenHeight);
    })
    this.is2in1 = deviceInfo.deviceType === '2in1' ? true : false;
  }

  aboutToAppear() {
    logger.info(TAG + 'aboutToAppear in');
    let value = settings.getValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY, switchState.open,
      settings.domainName.USER_SECURITY);
    this.isSwitchOn = value != switchState.close ? true : false;
    logger.info(TAG + '<aboutToAppear> this.isSwitchOn : ' + this.isSwitchOn + '; value: ' + value);

    AppStorage.setOrCreate('isSwitchOn', this.isSwitchOn);
    logger.info(TAG + 'AppStorage.get<boolean>(isSwitchOn) : ' + AppStorage.get<boolean>('isSwitchOn'));

    if (this.isSwitchOn) {
      let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY, switchState.open,
        settings.domainName.USER_SECURITY);
      logger.info(TAG + 'set value success :' + status + '; set:Continue_Switch_Status is 1');
    }

    this.getStringSync();
    this.getImageArray();
    this.getForegroundOsAccountLocalId();

    this.listener.on('change', (mediaQueryResult: mediaQuery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    })
    this.extContext = localStorage.get<common.UIExtensionContext>('context');
  }

  onBackPress() {
    logger.info(TAG + 'onBackPress in');
  }

  async awaitTime(time: number) {
    setTimeout(() => {
      logger.info(TAG + 'awaitTime is' + time);
    }, time)
  }

  build() {
    Column() {
      Column() {
        EditableTitleBar({
          leftIconStyle: EditableLeftIconType.Back,
          title: $r('app.string.continue_title'),
          isSaveIconRequired: false,
          onCancel: () => {
            if (this.continueSession) {
              this.continueSession.sendData({ 'action': 'pop' })
            } else {
              logger.error(TAG + 'continueSession is undefined');
            }
          }
        })
      }

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
              logger.info(TAG + 'ImageAnimator Start');
            })
            .onFinish(() => {
              logger.info(TAG + 'ImageAnimator Finish');
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
                  top: this.is2in1 ? CommonConstants.ITEM_LIST_PADDING_TOP_PC : CommonConstants.ITEM_LIST_PADDING_TOP,
                  bottom: this.is2in1 ? CommonConstants.ITEM_LIST_PADDING_BOTTOM_PC :
                  CommonConstants.ITEM_LIST_PADDING_BOTTOM
                })

              Toggle({ type: ToggleType.Switch, isOn: this.isSwitchOn })
                .width(CommonConstants.CONTINUE_SWITCH_WIDTH)
                .height(CommonConstants.CONTINUE_SWITCH_HEIGHT)
                .hoverEffect(HoverEffect.None)
                .enabled(this.isEnabled)
                .onChange((isOn: boolean) => {
                  logger.info(TAG + 'isOn:' + isOn);
                  this.isSwitchOn = isOn;
                  AppStorage.setAndLink('isSwitchOn', isOn);
                  if (isOn) {
                    let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY,
                      switchState.open, settings.domainName.USER_SECURITY);
                    logger.info(TAG + 'is set success :' + status + '; set:Continue_Switch_Status is open');
                  } else {
                    let status: boolean = settings.setValueSync(context, CommonConstants.CONTINUE_SWITCH_KEY,
                      switchState.close, settings.domainName.USER_SECURITY);
                    logger.info(TAG + 'is set success :' + status + '; set:Continue_Switch_Status is close');
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
                .fontSize(this.getSymbolSize())
                .fontColor([$r('sys.color.ohos_id_color_activated')])
                .margin({ right: CommonConstants.SYMBOL_MARGIN_RIGHT })

              Text($r('app.string.update_version_prompt'))
                .fontSize($r('sys.float.ohos_id_text_size_body3'))
                .fontWeight(FontWeight.Medium)
                .fontColor($r('sys.color.ohos_id_color_text_primary'))
                .textAlign(TextAlign.Start)
                .lineHeight(CommonConstants.UPDATE_PROMPT_LINE_HEIGHT)
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
      }
      .width('100%')
      .height(this.screenHeight)
      .scrollable(ScrollDirection.Vertical)
      .scrollBar(BarState.Off)
      .align(Alignment.TopStart)
      .friction(0.6)
      .edgeEffect(EdgeEffect.Spring)
      .onWillScroll((xOffset: number, yOffset: number) => {
        logger.debug(TAG + 'onScroll : xOffset:' + xOffset + ' ; yOffset' + yOffset);
      })
      .onScrollEdge(() => {
        logger.info('To the edge');
      })
      .onScrollStop(() => {
        logger.info('Scroll Stop');
      })
      .onAreaChange((oldArea: Area, newArea: Area) => {
        logger.info(TAG + 'Scroll, oldArea.height = ' + oldArea.height + ', newArea.height = ' + newArea.height);
      })
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
    .onAreaChange((oldArea: Area, newArea: Area) => {
      logger.info(TAG + 'build column , oldArea.height = ' + oldArea.height + ', newArea.height = ' + newArea.height);
      this.getImageAnimatorSize(newArea.height, newArea.width);
      logger.info(TAG + 'this.shortSideSize = ' + this.shortSideSize + ', this.imageAnimatorWidth = ' +
      this.imageAnimatorWidth + ', this.imageAnimatorHeight = ' + this.imageAnimatorHeight);
    })
  }

  private getSymbolSize(): Length {
    logger.info(TAG + 'getSymbolSize in');
    let fontSizeScale: number = AppStorage.get<number>('currentFontSizeScale') as number;
    logger.info(TAG + 'getSymbolSize : fontSizeScale =' + fontSizeScale);
    if (fontSizeScale >= 2) {
      return `${CommonConstants.SYMBOL_INFO_CIRCLE * 2}vp`
    }
    return CommonConstants.SYMBOL_INFO_CIRCLE;
  }
}