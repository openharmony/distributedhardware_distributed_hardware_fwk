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
import { BusinessError } from '@ohos.base';
import hardwareManager from '@ohos.distributedHardware.hardwareManager';
import common from '@ohos.app.ability.common';
import router from '@ohos.router';
import CommonConstants from '../constants/CommonConstants';

const TAG = '[testTag_DHardwareUI] : ';
PersistentStorage.persistProp('sign', '');

@Entry
@Component
struct DHardwareUI {
  @State deviceName: string = '';
  @State dhTypeName: string = '';
  @State bgcImg: PixelMap | null = null;
  @StorageLink('type') type: number = 0;
  @StorageLink('srcNetworkId') srcNetworkId: string = '';
  @StorageLink('deviceName') srcDeviceName: string = '';
  @StorageLink('sign') sign: string = '';
  @StorageLink('camera') camera: number = 999;
  @StorageLink('mic') mic: number = 999;
  @StorageLink('switchedCamera') switchedCamera: number = 999;
  @StorageLink('isPause') isPause: boolean = true;
  @State description: hardwareManager.HardwareDescriptor = {
    type: this.type,
    srcNetworkId: this.srcNetworkId
  }
  private context = getContext(this) as common.UIAbilityContext;

  aboutToAppear() {
    console.info(TAG + 'aboutToAppear() ');
    this.sign = '';

    if (this.camera == 0 && this.mic == 0) {
      try {
        this.context.terminateSelf((err: BusinessError) => {
          if (err.code) {
            console.error(`terminateSelf failed,code is ${err.code},message is ${err.message}`);
            return;
          }
          console.log(TAG + 'aboutToAppear() : ' + 'terminateSelf succeed');
        });
      } catch (err) {
        let code = (err as BusinessError).code;
        let message = (err as BusinessError).message;
        console.error(`terminateSelf failed,code is ${code},message is ${message}`);
      }
    }

    AppStorage.setAndLink('description', this.description);

    console.info(TAG + 'aboutToAppear : this.description : ' + this.description);
    console.info(TAG + 'aboutToAppear : this.description.type : ' + this.description.type);
    console.info(TAG + 'aboutToAppear : this.description.srcNetworkId : ' + this.description.srcNetworkId);
  }

  onPageShow() {
    console.log(TAG + 'onPageShow() ');
    if (this.isPause === false && this.camera === 2) {
      this.isPause = true;
      console.info(TAG + 'onPageShow : this.camera ' + this.camera);
      console.info(TAG + 'onPageShow : this.isPause ' + this.isPause);
    }

    if (this.type == 1024) {
      this.context.resourceManager.getStringValue($r('app.string.DHTypeName_Mic').id, (error, value) => {
        if (error != null) {
          console.log('error is ' + error);
        } else {
          this.dhTypeName = value;
          console.info(TAG + 'aboutToAppear : this.dhTypeName : ' + this.dhTypeName);
        }
      });
    } else {
      this.context.resourceManager.getStringValue($r('app.string.DHTypeName_Camera').id, (error, value) => {
        if (error != null) {
          console.log('error is ' + error);
        } else {
          this.dhTypeName = value;
          console.info(TAG + 'aboutToAppear : this.dhTypeName : ' + this.dhTypeName);
        }
      });
    }
  }

  changePauseOrResumeStatus() {
    this.isPause = !this.isPause;
    console.log(TAG + 'changePauseOrResumeStatus - this.isPause' + JSON.stringify(this.isPause))
  }

  stop() {
    console.info(TAG + 'stop');
    try {
      hardwareManager.stopDistributedHardware(this.description).then(() => {
        console.log('stop distributed hardware successfully');
      }).catch((error: BusinessError) => {
        console.error('stop distributed hardware failed, cause:' + error);
      })
      console.log('stop distributed hardware successfully');
    } catch (error) {
      console.error('stop distributed hardware failed:' + error);
    }
  }

  pause() {
    console.log(TAG + 'pause');
    try {
      hardwareManager.pauseDistributedHardware(this.description).then(() => {
        console.log('pause distributed hardware successfully');
      }).catch((error: BusinessError) => {
        console.error('pause distributed hardware failed, cause:' + error);
      })
      console.log('pause distributed hardware successfully');
    } catch (error) {
      console.error('pause distributed hardware failed:' + error);
    }
  }

  resume() {
    console.log(TAG + 'resume');
    try {
      hardwareManager.resumeDistributedHardware(this.description).then(() => {
        console.log('resume distributed hardware successfully');
      }).catch((error: BusinessError) => {
        console.error('resume distributed hardware failed, cause:' + error);
      })
      console.log('resume distributed hardware successfully');
    } catch (error) {
      console.error('resume distributed hardware failed:' + error);
    }
  }

  onPageHide() {
    console.info(TAG + 'onPageHide');
  }

  onBackPress() {
    console.info(TAG + 'onBackPress');
    this.sign = 'onBackPress';
  }

  aboutToDisappear() {
    console.info(TAG + 'aboutToDisappear');
  }

  build() {
    Stack() {
      Stack() {
        Image($r('app.media.bgcWall'))
      }

      Row() {
        Column() {
          Row() {
            Image(this.type == 1024 ? $r('app.media.ic_public_voice') : $r('app.media.ic_public_screencap'))
              .width(28)
              .height(28)
              .fillColor(Color.White)
              .margin({ left: 13 })
          }.borderRadius(50).width(54).height(54).backgroundColor('rgba(0,0,0,0.15)')

          Text($r('app.string.connected_to', this.srcDeviceName))
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor('#FFFFFF')
            .margin({ left: 0, top: 16, right: 0 })
            .textAlign(TextAlign.Center)

          Text($r('app.string.stop_device', this.dhTypeName))
            .fontSize(16)
            .fontWeight(FontWeight.Regular)
            .fontColor('#99FFFFFF')
            .margin({ top: 2 })
            .fontStyle(FontStyle.Normal)
            .textAlign(TextAlign.Center)

          GridRow({ columns: 5, gutter: { x: 24 } }) {
            GridCol({ span: 1 })

            GridCol({ span: 3 }) {
              Column() {
                Button(this.isPause ? $r('app.string.pause') : $r('app.string.continue'), {
                  type: ButtonType.Capsule,
                  stateEffect: true
                })
                  .backgroundColor('rgba(255,255,255,0.2)')
                  .height(40)
                  .fontSize(16)
                  .margin({ top: 24 })
                  .width('100%')
                  .onClick(() => {
                    console.log(TAG + (this.isPause ? 'click pause button' : 'click continue button'));
                    if (this.isPause) {
                      this.pause();
                      AppStorage.setOrCreate('isPauseTag', true);
                    } else {
                      this.resume();
                      AppStorage.setOrCreate('isPauseTag', false);
                    }
                    this.changePauseOrResumeStatus();
                    console.info(TAG + 'onClick : AppStorage.get<boolean>(isPauseTag) ' + AppStorage.get<boolean>('isPauseTag'));
                  })

                Button($r('app.string.disconnect'), { type: ButtonType.Capsule, stateEffect: true })
                  .backgroundColor('#FFFFFF')
                  .height(40)
                  .fontSize(16)
                  .margin({ top: 16 })
                  .width('100%')
                  .fontColor('rgba(0,0,0,0.9)')
                  .onClick(() => {
                    console.log(TAG + 'click disconnect button')
                    this.sign = 'stop';
                    this.stop();

                    try {
                      this.context.terminateSelf((err: BusinessError) => {
                        if (err.code) {
                          console.error(`terminateSelf failed, code is ${err.code}, message is ${err.message}`);
                          return;
                        }
                        console.info(TAG + 'terminateSelf succeed');
                        router.back();
                      });
                    } catch (err) {
                      let code = (err as BusinessError).code;
                      let message = (err as BusinessError).message;
                      console.error(`terminateSelf failed, code is ${code}, message is ${message}`);
                    }
                  })
              }
            }

            GridCol({ span: 1 })
          }

        }.width('100%').padding({ left: 24, right: 24 })
      }.width('100%')
    }

  }
}