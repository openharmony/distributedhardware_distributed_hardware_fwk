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

const TAG = '[ContinueSwitch_Page] : ';
let context = getContext(this) as common.UIAbilityContext;
let localStorage = LocalStorage.getShared();

interface switchStatus {
  open: string;
  close: string;
}

let switchState: switchStatus = {
  open: '1',
  close: '0'
}

@Entry
@Component
struct ContinueSwitch {
  @StorageLink('isSwitchOn') isSwitchOn: boolean | undefined = true;
  @StorageLink('continueSession') continueSession: UIExtensionContentSession | undefined = undefined;
  @State title: string = '';
  @State screenHeight: number = 0;
  @State screenWidth: number = 0;
  @State imageHeight: number = 0;
  @State textWidth: number = 0;
  @State gapLength: number = 0;
  @State videoSrc: Resource = $rawfile('continue.mp4');
  @State isShow: boolean = false;
  @State portraitFunc: mediaQuery.MediaQueryResult | void | null = null;
  private videoController: VideoController | undefined = undefined;
  private listener: mediaQuery.MediaQueryListener = mediaQuery.matchMediaSync('(dark-mode:true)');
  private extContext?: common.UIExtensionContext;

  onPortrait(mediaQueryResult: mediaQuery.MediaQueryResult) {
    console.log(TAG + 'onPortrait in');
    if (mediaQueryResult.matches as boolean) {
      this.videoSrc = $rawfile('continue_dark.mp4');
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_DARK);
    } else {
      this.videoSrc = $rawfile('continue.mp4');
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT);
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
    console.log(TAG + 'onPageShow in');
    this.getGapLength();
    display.getAllDisplays((err, data) => {
      this.screenWidth = px2vp(data[0].width);
      this.screenHeight = px2vp(data[0].height);
      this.getImageHeight();
      console.log(TAG + 'screenWidth = ' + this.screenWidth + '; screenHeight = ' + this.screenHeight);
    })
    if (this.videoController) {
      this.videoController.start();
    }
  }

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear in');
    let value = settings.getValueSync(context, 'Continue_Switch_Status', switchState.open);
    this.isSwitchOn = value != switchState.close ? true : false;
    console.log(TAG + '<aboutToAppear> this.isSwitchOn : ' + this.isSwitchOn + '; value: ' + value);

    AppStorage.setOrCreate('isSwitchOn', this.isSwitchOn);
    console.log(TAG + 'AppStorage.get<boolean>(isSwitchOn) : ' + AppStorage.get<boolean>('isSwitchOn'));

    if (this.isSwitchOn) {
      let status: boolean = settings.setValueSync(context, 'Continue_Switch_Status', switchState.open);
      console.log(TAG + 'set value success :' + status + '; set:Continue_Switch_Status is 1');
    }

    try {
      context.resourceManager.getStringValue($r('app.string.continue_title')
        .id, (error: BusinessError, value: string) => {
        if (error != null) {
          console.error(TAG + 'error is ' + error);
        } else {
          this.title = value;
          console.info(TAG + '<aboutToAppear> this.title : ' + this.title);
        }
      })
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      console.error(TAG + `callback getStringValue failed,error code: ${code},message: ${message}.`);
    }

    this.listener.on('change', (mediaQueryResult: mediaQuery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    })
    this.extContext = localStorage.get<common.UIExtensionContext>('context');
  }

  getImageHeight() {
    console.log(TAG + 'getImageHeight in, deviceInfo.deviceType : ' + deviceInfo.deviceType);
    if (deviceInfo.deviceType === 'phone') {
      this.imageHeight = 220;
    } else if (deviceInfo.deviceType === 'table') {
      this.imageHeight = 212;
    } else if (deviceInfo.deviceType === '2in1') {
      this.imageHeight = 261;
    }
    console.log(TAG + 'this.imageHeight' + this.imageHeight);
  }

  getGapLength() {
    console.log(TAG + 'getGapLength in, deviceInfo.deviceType : ' + deviceInfo.deviceType);
    if (deviceInfo.deviceType == 'phone') {
      this.gapLength = 16;
    } else if (deviceInfo.deviceType == '2in1' || deviceInfo.deviceType == 'tablet') {
      this.gapLength = 24;
    }
    console.log(TAG + 'this.gapLength : ' + this.gapLength);
  }

  onBackPress() {
    console.log(TAG + 'onBackPress in');
  }

  async awaitTime(time: number) {
    setTimeout(() => {
      console.log(TAG, 'awaitTime is' + time);
    }, time)
  }

  build() {
    Navigation() {
      Column() {
        EditableTitleBar({
          leftIconStyle: EditableLeftIconType.Back,
          title: $r('app.string.continue_title'),
          isSaveIconRequired: false,
          onCancel: () => {
            if (this.continueSession) {
              this.continueSession.sendData({ 'action': 'pop' })
            } else {
              console.error(TAG + 'continueSession is undefined');
            }
          }
        })
      }

      Row() {
        Column() {
          Video({ src: this.videoSrc, controller: this.videoController })
            .autoPlay(true)
            .loop(true)
            .objectFit(ImageFit.Contain)
            .muted(true)
            .controls(false)
            .height(this.imageHeight)
            .margin({ left: 35, right: 35, bottom: 24 })
            .backgroundColor(Color.Transparent)
            .visibility(this.isShow ? Visibility.Visible : Visibility.Hidden)
            .onPrepared(async (event) => {
              console.info('onPrepared is ' + event.duration);
              if (this.videoController) {
                this.videoController.start();
              }
              await this.awaitTime(100);
              this.isShow = true;
            })

          Text($r('app.string.continue_desc'))
            .fontSize($r('sys.float.ohos_id_text_size_body2'))
            .fontWeight(FontWeight.Regular)
            .margin({ bottom: 32 })
            .fontColor($r('sys.color.ohos_id_color_text_secondary'))
            .textAlign(TextAlign.Center)
            .width('100%')

          Column() {
            Row() {
              Text($r('app.string.continue_title'))
                .fontSize($r('sys.float.ohos_id_text_size_sub_title2'))
                .fontWeight(FontWeight.Medium)
                .fontColor($r('sys.color.ohos_id_color_text_primary'))

              Toggle({ type: ToggleType.Switch, isOn: this.isSwitchOn })
                .hoverEffect(HoverEffect.None)
                .onChange((isOn: boolean) => {
                  console.log(TAG + 'isOn:' + isOn);
                  this.isSwitchOn = isOn;
                  AppStorage.setAndLink('isSwitchOn', isOn);
                  if (isOn) { // open
                    let status: boolean = settings.setValueSync(context, 'Continue_Switch_Status', switchState.open);
                    console.log(TAG + 'is set success :' + status + '; set:Continue_Switch_Status is on');
                  } else { // close
                    let status: boolean = settings.setValueSync(context, 'Continue_Switch_Status', switchState.close);
                    console.log(TAG + 'is set success :' + status + '; set:Continue_Switch_Status is close');
                  }
                })
            }
            .justifyContent(FlexAlign.SpaceBetween)
            .width('100%')
            .height(56)
            .padding({ left: 12, right: 8 })
            .backgroundColor($r('sys.color.ohos_id_color_list_card_bg'))
            .borderRadius(20)
          }.width('100%')

          Column() {
            Row() {
              Text($r('app.string.continue_privacy_text'))
                .fontSize($r('sys.float.ohos_id_text_size_body2'))
                .fontWeight(FontWeight.Regular)
                .margin({ top: 8 })
                .fontColor($r('sys.color.ohos_id_color_text_secondary'))
                .textAlign(TextAlign.Start)
                .width('100%')
            }
            .padding({ left: 8, right: 8 })
          }.width('100%')

        }.padding({ left: this.gapLength, right: this.gapLength }).width('100%')
      }
      .width('100%')
      .height('100%')
      .alignItems(VerticalAlign.Top)
      .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
    }
    .hideTitleBar(true)
    .title(this.NavigationTitle)
    .titleMode(NavigationTitleMode.Mini)
    .mode(NavigationMode.Stack)
    .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
    .onAppear(() => {
      console.log(TAG + 'Navigation onAppear')
    })
  }
}