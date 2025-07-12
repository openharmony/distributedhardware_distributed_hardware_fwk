/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import { logger } from '../utils/Logger';
import mechanicManager from '@ohos.distributedHardware.mechanicManager';
import { BusinessError } from '@kit.BasicServicesKit';

const TAG = '[MechControl_Page] : ';

@Entry
@Component
struct MechControl {
  @State isTrackingEnabled: boolean = false;
  @State currentTrackingLayoutText: ResourceStr = $r('app.string.mech_center');
  @State currentTrackingLayoutImage: ResourceStr = $r('app.media.track_layout_center');


  aboutToAppear() {
    logger.info(`${TAG} aboutToAppear in`);
  }

  @Builder
  IntelligentTracking(){
    Column(){
      Button() {
        Image(this.isTrackingEnabled ? $r('app.media.intelligent_tracking') :$r('app.media.intelligent_tracking_dark'))
          .width(20)
          .height(20)
          .margin({left: 3, top: 3})
      }
      .width('40vp')
      .height('40vp')
      .backgroundColor(this.isTrackingEnabled ? $r('app.color.color_0A59F7_blue') : $r('sys.color.ohos_id_color_component_normal'))
      .onClick(() => {
        this.isTrackingEnabled = !this.isTrackingEnabled;
        try {
          if (this.isTrackingEnabled) {
            logger.info(`${TAG} enable camera tracking`);
            mechanicManager.setCameraTrackingEnabled(true);
          } else {
            logger.info(`${TAG} disable camera tracking`);
            mechanicManager.setCameraTrackingEnabled(false);
          }
        } catch (error) {
          let code: number = (error as BusinessError).code;
          let message: string = (error as BusinessError).message;
          logger.error(`${TAG} setCameraTrackingEnabled failed. error.code: ${code}, message: ${message}`);
        }
        logger.info(`${TAG} onClick end, isTrackingEnabled = ${this.isTrackingEnabled}`);
      })

      Text($r('app.string.mech_intelligent_tracking'))
        .fontSize(12)
        .lineHeight(16)
        .fontWeight(FontWeight.Medium)
        .fontColor($r('app.color.color_black'))
        .margin({ left: 0, top: 4, right: 0 })
        .textAlign(TextAlign.Center)

      Text(this.isTrackingEnabled ? $r('app.string.mech_enable'): $r('app.string.mech_close'))
        .fontSize(10)
        .lineHeight(13)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('app.color.color_black'))
        .margin({ top: 2 })
        .textAlign(TextAlign.Center)
    }
  }

  @Builder
  TrackingLayout(){
    Column(){
      Button() {
        Image(this.currentTrackingLayoutImage)
          .width(20)
          .height(20)
      }
      .width('40vp')
      .height('40vp')
      .backgroundColor($r('app.color.color_0A59F7_blue'))
      .onClick(() => {
        this.isTrackingEnabled = !this.isTrackingEnabled;
        try {
          if (this.isTrackingEnabled) {
            logger.info(`${TAG} enable camera tracking`);
            mechanicManager.setCameraTrackingEnabled(true);

            this.currentTrackingLayoutImage = $r('app.media.track_layout_center');
          } else {
            logger.info(`${TAG} disable camera tracking`);
            mechanicManager.setCameraTrackingEnabled(false);

            this.currentTrackingLayoutImage = $r('app.media.track_layout_center_dark');
          }
        } catch (error) {
          let code: number = (error as BusinessError).code;
          let message: string = (error as BusinessError).message;
          logger.error(`${TAG} setCameraTrackingEnabled failed. error.code: ${code}, message: ${message}`);
        }
        logger.info(`${TAG} onClick end, isTrackingEnabled = ${this.isTrackingEnabled}`);
      })

      Text($r('app.string.mech_tracking_layout'))
        .fontSize(12)
        .lineHeight(16)
        .fontWeight(FontWeight.Medium)
        .fontColor($r('app.color.color_black'))
        .margin({ left: 0, top: 4, right: 0 })
        .textAlign(TextAlign.Center)

      Text(this.currentTrackingLayoutText)
        .fontSize(10)
        .lineHeight(13)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('app.color.color_black'))
        .margin({ top: 2 })
        .textAlign(TextAlign.Center)
    }
  }

  build() {
    Column() {

      Flex({
        direction: FlexDirection.Row,
        justifyContent: FlexAlign.SpaceEvenly,
        alignItems: ItemAlign.Center
      }) {
        this.IntelligentTracking();

        if (this.isTrackingEnabled) {
          this.TrackingLayout();
        }
      }
      .width('100%')
      .height('100%')

    }
    .width('100%')
    .height('100%')
    .justifyContent(FlexAlign.Center)
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM])
  }
}