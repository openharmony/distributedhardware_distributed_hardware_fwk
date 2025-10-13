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
import { BusinessError } from '@ohos.base';
import router from '@ohos.router';
import curves from '@ohos.curves';
import accessibility from '@ohos.accessibility';
import common from '@ohos.app.ability.common';
import CommonConstants,{ FontSizeScale, LanguagesUtil } from '../common/constants/CommonConstants';

const TAG = '[MechControl_Page] : ';
let localStorage = LocalStorage.getShared();

@Entry(localStorage)
@Component
struct MechControl {
  @StorageLink('isTrackingEnabled') @Watch('onSwitchChange') isTrackingEnabled: boolean =
    this.getDefaultValue('isTrackingEnabled', true);
  @StorageLink('currentLayout') @Watch('onLayoutChange') currentLayout: mechanicManager.CameraTrackingLayout =
    this.getDefaultValue('currentLayout', mechanicManager.CameraTrackingLayout.DEFAULT);
  @StorageLink('currentTrackingLayoutText') currentTrackingLayoutText: ResourceStr = $r('app.string.mech_center');
  @StorageLink('currentTrackingLayoutImage') currentTrackingLayoutImage: ResourceStr =
    $r('app.media.track_layout_center');
  @State fontSizeScale: number = AppStorage.get('currentFontSizeScale') as number;
  @State pageOpacity: number = 1;
  @State visibilityState: Visibility = Visibility.Hidden;
  private context = getContext(this) as common.UIAbilityContext;

  private getDefaultValue<T>(key: string, defaultValue: T): T {
    const value = AppStorage.get<T>(key);
    return value !== undefined ? value : defaultValue;
  }

  pageTransition() {
    PageTransitionEnter({ type: RouteType.None, duration: 0 })
    PageTransitionExit({ type: RouteType.None, duration: 0 })
  }

  onSwitchChange() {
    logger.info(`${TAG} onSwitchChange in`);
  }

  onLayoutChange() {
    logger.info(`${TAG} onLayoutChange in`);
    switch (this.currentLayout) {
      case mechanicManager.CameraTrackingLayout.DEFAULT:
      case mechanicManager.CameraTrackingLayout.MIDDLE: {
        this.currentTrackingLayoutImage = $r('app.media.track_layout_center');
        this.currentTrackingLayoutText = $r('app.string.mech_center');
        break;
      }
      case mechanicManager.CameraTrackingLayout.LEFT: {
        this.currentTrackingLayoutImage = $r('app.media.track_layout_left');
        this.currentTrackingLayoutText = $r('app.string.mech_left');
        break;
      }
      case mechanicManager.CameraTrackingLayout.RIGHT: {
        this.currentTrackingLayoutImage = $r('app.media.track_layout_right');
        this.currentTrackingLayoutText = $r('app.string.mech_right');
        break;
      }
    }
  }

  toggleEffectChange(){
    logger.info(`${TAG} toggleEffectChange in`);
    const originalState = this.isTrackingEnabled;
    try {
      if (!this.isTrackingEnabled) {
        logger.info(`${TAG} enable camera tracking`);
        mechanicManager.setCameraTrackingEnabled(true);
        this.isTrackingEnabled = true;
      } else {
        logger.info(`${TAG} disable camera tracking`);
        mechanicManager.setCameraTrackingEnabled(false);
        this.isTrackingEnabled = false;
      }
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(`${TAG} setUserOperation switch failed. error.code: ${code}, message: ${message}`);
      this.isTrackingEnabled = originalState;
    }
  }

  getSwitchStateInfo(isTrackingEnabled: boolean): string {
    if (isTrackingEnabled) {
      logger.info(`SwitchState is on. isTrackingEnabled: ${isTrackingEnabled}`);
      return this.context.resourceManager.getStringSync($r('app.string.audio_tracking_switch_on').id);
    } else {
      logger.info(`SwitchState is off. isTrackingEnabled: ${isTrackingEnabled}`);
      return this.context.resourceManager.getStringSync($r('app.string.audio_tracking_switch_off').id);
    }
  }

  aboutToAppear() {
    logger.info(`${TAG} aboutToAppear in`);
    this.fontSizeScale = this.context.config?.fontSizeScale ?? FontSizeScale.DEFAULT;
    logger.info(`${TAG} this.fontSizeScale: ${this.fontSizeScale}`);
    try {
      this.isTrackingEnabled = mechanicManager.getCameraTrackingEnabled();
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(`${TAG} getCameraTrackingEnabled failed. error.code: ${code}, message: ${message}`);
      this.isTrackingEnabled = this.getDefaultValue('isTrackingEnabled', true);
    }

    try {
      this.currentLayout = mechanicManager.getCameraTrackingLayout();
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(`${TAG} getCameraTrackingLayout failed. error.code: ${code}, message: ${message}`);
      this.currentLayout = mechanicManager.CameraTrackingLayout.DEFAULT;
    }
    logger.info(`${TAG} aboutToAppear end. isTrackingEnabled: ${this.isTrackingEnabled}, currentLayout: ${this.currentLayout}`);
  }

  aboutToDisappear(): void {
    logger.info(`${TAG} aboutToDisappear in`);
  }

  onPageShow(): void {
    // Trigger the entry animation
    this.visibilityState = Visibility.Visible;
  }

  @Builder
  IntelligentTracking() {
    Column() {
      Button() {
        Image(this.isTrackingEnabled ? $r('app.media.intelligent_tracking') : $r('app.media.intelligent_tracking_dark'))
          .width(CommonConstants.SWITCH_ICON_SIZE)
          .height(CommonConstants.SWITCH_ICON_SIZE)
          .margin({ left: CommonConstants.SWITCH_ICON_MARGIN, top: CommonConstants.SWITCH_ICON_MARGIN })
      }
      .width(CommonConstants.BUTTON_SIZE)
      .height(CommonConstants.BUTTON_SIZE)
      .backgroundColor(this.isTrackingEnabled ? $r('sys.color.ohos_id_color_emphasize') :
      $r('sys.color.ohos_id_color_component_normal'))
      .accessibilityText($r('app.string.mech_intelligent_tracking'))
      .type(ButtonType.Circle)
      .clickEffect({ level: ClickEffectLevel.LIGHT, scale: CommonConstants.BUTTON_CLICK_SCALE })
      .onClick(() => {
        this.toggleEffectChange();
        try {
          let eventInfo: accessibility.EventInfo = ({
            type: 'announceForAccessibility',
            bundleName: CommonConstants.BUNDLE_NAME,
            triggerAction: 'click',
            textAnnouncedForAccessibility: this.getSwitchStateInfo(this.isTrackingEnabled)
          });
          accessibility.sendAccessibilityEvent(eventInfo).then(() => {
            logger.info(`Succeeded in send event, eventInfo is ${JSON.stringify(eventInfo)}`);
          }).catch((err: Error) => {
            logger.error('Failed to send accessibility event');
          });
        } catch (error) {
          let code: number = (error as BusinessError).code;
          let message: string = (error as BusinessError).message;
          logger.error(`${TAG} sendAccessibilityEvent failed,error code: ${code},message: ${message}.`);
        }
        logger.info(`${TAG} onClick end, isTrackingEnabled = ${this.isTrackingEnabled}`);
      })

      Text($r('app.string.mech_intelligent_tracking'))
        .fontSize($r('sys.float.ohos_id_text_size_body3'))
        .lineHeight(CommonConstants.LINE_HEIGHT)
        .fontWeight(FontWeight.Medium)
        .fontColor($r('sys.color.font_primary'))
        .margin({ top: CommonConstants.TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
        .width(CommonConstants.SWITCH_TEXT_WIDTH)

      Text(this.isTrackingEnabled ? $r('app.string.mech_enable') : $r('app.string.mech_close'))
        .fontSize($r('sys.float.Caption_M'))
        .lineHeight(CommonConstants.SMALL_LINE_HEIGHT)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('sys.color.font_secondary'))
        .margin({ top: CommonConstants.SMALL_TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
    }
    .translate({
      x: this.isTrackingEnabled ? 0 :
        (LanguagesUtil.isMirrorLanguages() ? -CommonConstants.TRANSLATE_X : CommonConstants.TRANSLATE_X)
    })
    .animation({
      curve: curves.springMotion(CommonConstants.SPRING_MOTION_RESPONSE,
        CommonConstants.SPRING_MOTION_DAMPING_FRACTION, CommonConstants.SPRING_MOTION_OVERLAP_DURATION)
    })
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .margin({
      left: CommonConstants.NORMAL_SWITCH_MARGIN_LEFT,
      right: LanguagesUtil.isMirrorLanguages() ? CommonConstants.NORMAL_SWITCH_MARGIN_LEFT : 0,
      top: LanguagesUtil.isUltraLongLanguages() ? CommonConstants.SWITCH_ICON_MARGIN_TOP_ULTRA :
        CommonConstants.SWITCH_ICON_MARGIN_TOP
    })
  }

  @Builder
  TrackingLayout() {
    Column() {
      Button() {
        Image(this.currentTrackingLayoutImage)
          .width(CommonConstants.SWITCH_ICON_SIZE)
          .height(CommonConstants.SWITCH_ICON_SIZE)
      }
      .width(CommonConstants.BUTTON_SIZE)
      .height(CommonConstants.BUTTON_SIZE)
      .accessibilityText($r('app.string.mech_tracking_layout'))
      .backgroundColor($r('sys.color.ohos_id_color_emphasize'))
      .type(ButtonType.Circle)
      .clickEffect({ level: ClickEffectLevel.LIGHT, scale: CommonConstants.BUTTON_CLICK_SCALE })
      .onClick(() => {
        logger.info(`${TAG} onClick TrackingLayout in`);
        // Trigger the exit animation
        this.visibilityState = Visibility.Hidden;
        setTimeout(() => {
          router.pushUrl({ url: 'pages/TrackingLayout' }, () => {
            logger.info(`${TAG} setTimeout router pushUrl pages/TrackingLayout`);
          })
        }, CommonConstants.ANIMATE_DURATION);
      })

      Text($r('app.string.mech_tracking_layout'))
        .fontSize($r('sys.float.ohos_id_text_size_body3'))
        .lineHeight(CommonConstants.LINE_HEIGHT)
        .fontWeight(FontWeight.Medium)
        .fontColor($r('sys.color.font_primary'))
        .margin({ top: CommonConstants.TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
        .width(CommonConstants.SWITCH_TEXT_WIDTH)

      Text(this.currentTrackingLayoutText)
        .fontSize($r('sys.float.Caption_M'))
        .lineHeight(CommonConstants.SMALL_LINE_HEIGHT)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('sys.color.font_secondary'))
        .margin({ top: CommonConstants.SMALL_TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
    }
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .margin({
      left: LanguagesUtil.isMirrorLanguages() ? 0 :CommonConstants.NORMAL_LAYOUT_MARGIN_LEFT,
      top: LanguagesUtil.isUltraLongLanguages() ? CommonConstants.SWITCH_ICON_MARGIN_TOP_ULTRA :
        CommonConstants.SWITCH_ICON_MARGIN_TOP
    })
  }

  build() {
    Column() {
        Row() {
          this.IntelligentTracking();

          if (this.isTrackingEnabled) {
            this.TrackingLayout();
          }

        }
        .width('100%')
        .height('100%')
        .justifyContent(FlexAlign.Start)
        .alignItems(VerticalAlign.Top)
    }
    .width('100%')
    .height('100%')
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM])
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .visibility(this.visibilityState)
  }
}