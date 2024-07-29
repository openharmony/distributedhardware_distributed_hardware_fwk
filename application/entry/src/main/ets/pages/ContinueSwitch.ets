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
import { logger } from '../utils/logger';

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
  @State videoSrc: Resource = $rawfile('continue_center.mp4');
  @State isShow: boolean = false;
  @State is2in1: boolean = false;
  @State portraitFunc: mediaQuery.MediaQueryResult | void | null = null;
  @State isVideoVisible: Visibility = Visibility.Hidden;
  @State contentHeight: number = 0;
  @State imageArray: Array<ImageFrameInfo> = [];
  @State animationState: AnimationStatus = AnimationStatus.Running;
  @State reverse: boolean = false;
  @State iterations: number = -1;
  private videoController: VideoController | undefined = undefined;
  private listener: mediaQuery.MediaQueryListener = mediaQuery.matchMediaSync('(dark-mode:true)');
  private extContext?: common.UIExtensionContext;
  private scroller: Scroller = new Scroller();

  onPortrait(mediaQueryResult: mediaQuery.MediaQueryResult) {
    logger.info(TAG + 'onPortrait in');
    if (mediaQueryResult.matches as boolean) {
      this.videoSrc = $rawfile('continue_dark_center.mp4');
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_DARK);
    } else {
      this.videoSrc = $rawfile('continue_center.mp4');
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
  getImageAnimatorSize(height: Length, width: Length) {
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

  getStringSync() {
    logger.info(TAG + 'getStringSync in');
    try {
      context.resourceManager.getStringValue($r('app.string.continue_title')
        .id, (error: BusinessError, value: string) => {
        if (error != null) {
          logger.error(TAG + 'error is ' + error);
        } else {
          this.title = value;
          console.info(TAG + '<aboutToAppear> this.title : ' + this.title);
        }
      })
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(TAG + `callback getStringValue failed,error code: ${code},message: ${message}.`);
    }
  }

  getImageArray() {
    logger.info(TAG + 'getStringSync in');
    for (let i = 0; i <= CommonConstants.IMAGE_COUNT; ++i) {
      this.imageArray.push({
        src: $r(`app.media.continue_${i}`),
        duration: (i == CommonConstants.IMAGE_COUNT) ? CommonConstants.IMG_ANIMATOR_OVER_DURATION
          : CommonConstants.IMG_ANIMATOR_NORMAL_DURATION
      })
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
    if (this.videoController) {
      this.videoController.start();
    }
    this.is2in1 = deviceInfo.deviceType === '2in1' ? true : false;
  }

  aboutToAppear() {
    logger.info(TAG + 'aboutToAppear in');
    let value = settings.getValueSync(context, 'Continue_Switch_Status', switchState.open);
    this.isSwitchOn = value != switchState.close ? true : false;
    logger.info(TAG + '<aboutToAppear> this.isSwitchOn : ' + this.isSwitchOn + '; value: ' + value);

    AppStorage.setOrCreate('isSwitchOn', this.isSwitchOn);
    logger.info(TAG + 'AppStorage.get<boolean>(isSwitchOn) : ' + AppStorage.get<boolean>('isSwitchOn'));

    if (this.isSwitchOn) {
      let status: boolean = settings.setValueSync(context, 'Continue_Switch_Status', switchState.open);
      logger.info(TAG + 'set value success :' + status + '; set:Continue_Switch_Status is 1');
    }

    this.getStringSync();
    this.getImageArray();

    this.listener.on('change', (mediaQueryResult: mediaQuery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    })
    this.extContext = localStorage.get<common.UIExtensionContext>('context');
  }

  getGapLength() {
    logger.info(TAG + 'getGapLength in, deviceInfo.deviceType : ' + deviceInfo.deviceType);
    if (deviceInfo.deviceType == 'phone') {
      this.gapLength = CommonConstants.GENERAL_PHONE_GAP_LENGTH;
    } else if (deviceInfo.deviceType == '2in1' || deviceInfo.deviceType == 'tablet') {
      this.gapLength = CommonConstants.PC_PAD_GAP_LENGTH;
    }
    logger.info(TAG + 'this.gapLength : ' + this.gapLength);
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
              console.info(TAG + 'ImageAnimator Start');
            })
            .onFinish(() => {
              console.info(TAG + 'ImageAnimator Finish');
            })

          Text($r('app.string.continue_desc'))
            .fontSize($r('sys.float.ohos_id_text_size_body2'))
            .fontWeight(FontWeight.Regular)
            .margin({ bottom: CommonConstants.CONTINUE_DESC_TEXT_MARGIN_BOTTOM })
            .fontColor($r('sys.color.ohos_id_color_text_secondary'))
            .textAlign(TextAlign.Center)
            .width('100%')
            .maxLines(10)
            .onAreaChange((oldArea: Area, newArea: Area) => {
              logger.info(TAG + 'Text:continue desc, oldArea.height = ' + oldArea.height +
                ', newArea.height = ' + newArea.height);
            })

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

              Toggle({ type: ToggleType.Switch, isOn: this.isSwitchOn })
                .width(CommonConstants.CONTINUE_SWITCH_WIDTH)
                .height(CommonConstants.CONTINUE_SWITCH_HEIGHT)
                .hoverEffect(HoverEffect.None)
                .onChange((isOn: boolean) => {
                  logger.info(TAG + 'isOn:' + isOn);
                  this.isSwitchOn = isOn;
                  AppStorage.setAndLink('isSwitchOn', isOn);
                  if (isOn) {
                    let status: boolean = settings.setValueSync(context, 'Continue_Switch_Status', switchState.open);
                    logger.info(TAG + 'is set success :' + status + '; set:Continue_Switch_Status is on');
                  } else {
                    let status: boolean = settings.setValueSync(context, 'Continue_Switch_Status', switchState.close);
                    logger.info(TAG + 'is set success :' + status + '; set:Continue_Switch_Status is close');
                  }
                })
            }
            .height(this.is2in1 ? CommonConstants.PC_LIST_HEIGHT : CommonConstants.PHONE_LIST_HEIGHT)
            .width('100%')
            .padding({
              left: CommonConstants.TEXT_LIST_ALIGN_DISTANCE,
              right: CommonConstants.TEXT_LIST_ALIGN_DISTANCE
            })
            .backgroundColor($r('sys.color.ohos_id_color_list_card_bg'))
            .borderRadius(this.is2in1 ? CommonConstants.PC_BORDER_RADIUS : CommonConstants.NON_PC_BORDER_RADIUS)
            .accessibilityText(this.title)
          }.width('100%')

          Column() {
            Row() {
              Text($r('app.string.continue_privacy_text'))
                .fontSize($r('sys.float.ohos_id_text_size_body3'))
                .fontWeight(FontWeight.Regular)
                .margin({ top: CommonConstants.CONTINUE_PRIVACY_TEXT_MARGIN_TOP})
                .fontColor($r('sys.color.ohos_id_color_text_secondary'))
                .textAlign(TextAlign.Start)
                .width('100%')
                .onAreaChange((oldArea: Area, newArea: Area) => {
                  logger.info(TAG + 'Text:continue_privacy_text, continue desc, oldArea.height = ' + oldArea.height +
                    ', newArea.height = ' + newArea.height);
                })
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
      .onScroll((xOffset: number, yOffset: number) => {
        console.info(TAG + 'onScroll : xOffset:' + xOffset + ' ; yOffset' + yOffset);
      })
      .onScrollEdge(() => {
        console.info('To the edge');
      })
      .onScrollStop(() => {
        console.info('Scroll Stop');
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
}