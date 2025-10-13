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
import router from '@ohos.router';
import mechanicManager from '@ohos.distributedHardware.mechanicManager';
import { BusinessError } from '@ohos.base';
import CommonConstants, { FontSizeScale, LanguagesUtil } from '../common/constants/CommonConstants';

const TAG = '[TrackingLayout_Page] : ';
let localStorage = LocalStorage.getShared();

@Entry(localStorage)
@Component
struct TrackingLayout {
  @State trackingLayoutLeftImage: ResourceStr = $r('app.media.track_layout_left');
  @State trackingLayoutRightImage: ResourceStr = $r('app.media.track_layout_right');
  @State trackingLayoutCenterImage: ResourceStr = $r('app.media.track_layout_center');
  @StorageLink('currentTrackingLayoutText') currentTrackingLayoutText: ResourceStr = $r('app.string.mech_center');
  @StorageLink('currentTrackingLayoutImage') currentTrackingLayoutImage: ResourceStr =
    $r('app.media.track_layout_center');
  @StorageLink('currentLayout') @Watch('onLayoutChange') currentLayout: mechanicManager.CameraTrackingLayout =
    this.getDefaultValue('currentLayout', mechanicManager.CameraTrackingLayout.DEFAULT);
  @State isIconHover:boolean = false;
  @State longPressCanceled: boolean = false;
  @State visibilityState: Visibility = Visibility.Hidden;

  private getDefaultValue<T>(key: string, defaultValue: T): T {
    const value = AppStorage.get<T>(key);
    return value !== undefined ? value : defaultValue;
  }

  pageTransition() {
    PageTransitionEnter({ type: RouteType.None, duration: 0 })
    PageTransitionExit({ type: RouteType.None, duration: 0 })
  }

  aboutToAppear() {
    logger.info(`${TAG} aboutToAppear in. currentLayout: ${this.currentLayout}`);
  }

  onPageShow(): void {
    // Trigger the entry animation
    this.visibilityState = Visibility.Visible;
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

  @Builder
  BackButton() {
    Column() {
      Button() {
        SymbolGlyph($r('sys.symbol.chevron_backward'))
          .fontWeight(FontWeight.Medium)
          .fontSize(CommonConstants.BACK_SYMBOL_SIZE)
          .fontColor([$r('sys.color.ohos_id_color_primary')])
      }
      .width(CommonConstants.BUTTON_SIZE)
      .height(CommonConstants.BUTTON_SIZE)
      .accessibilityText($r('app.string.audio_back'))
      .backgroundColor($r('sys.color.ohos_id_color_component_normal'))
      .onClick(() => {
        logger.info(`${TAG} onClick BackButton`);
        // Trigger the exit animation
        this.visibilityState = Visibility.Hidden;
        setTimeout(() => {
          router.back();
        }, CommonConstants.ANIMATE_DURATION);
      })
      .type(ButtonType.Circle)
      .clickEffect({ level: ClickEffectLevel.LIGHT, scale: CommonConstants.BUTTON_CLICK_SCALE })

      Text(' ')
        .fontSize($r('sys.float.Body_S'))
        .lineHeight(CommonConstants.LINE_HEIGHT)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('sys.color.font_primary'))
        .margin({ top: CommonConstants.TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
    }
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .margin({
      left: CommonConstants.TEXT_MARGIN_LEFT,
      top: CommonConstants.LAYOUT_ICON_MARGIN_TOP,
      right: LanguagesUtil.isMirrorLanguages() ? CommonConstants.LAYOUT_MARGIN_RIGHT_MIRROR : 0,
    })
  }

  @Builder
  TrackingLayoutLeft() {
    Column() {
      Button() {
        Image(this.currentLayout == mechanicManager.CameraTrackingLayout.LEFT ? $r('app.media.track_layout_left') :
        $r('app.media.track_layout_left_dark'))
          .width(CommonConstants.SWITCH_ICON_SIZE)
          .height(CommonConstants.SWITCH_ICON_SIZE)
      }
      .width(CommonConstants.BUTTON_SIZE)
      .height(CommonConstants.BUTTON_SIZE)
      .accessibilityText($r('app.string.mech_left'))
      .backgroundColor(this.currentLayout == mechanicManager.CameraTrackingLayout.LEFT ?
      $r('sys.color.ohos_id_color_emphasize') : $r('sys.color.ohos_id_color_component_normal'))
      .onClick(() => {
        logger.info(`${TAG} onClick TrackingLayoutLeft in`);
        this.onLeftIconClick();
      })
      .type(ButtonType.Circle)
      .clickEffect({ level: ClickEffectLevel.LIGHT, scale: CommonConstants.BUTTON_CLICK_SCALE })

      Text($r('app.string.mech_left'))
        .fontSize($r('sys.float.Body_S'))
        .lineHeight(CommonConstants.LINE_HEIGHT)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('sys.color.font_primary'))
        .margin({ top: CommonConstants.TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
        .width(CommonConstants.BUTTON_SIZE)
    }
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .margin({ left: CommonConstants.LEFT_ICON_MARGIN_LEFT, top: CommonConstants.LAYOUT_ICON_MARGIN_TOP })
  }

  @Builder
  TrackingLayoutCenter() {
    Column() {
      Button() {
        Image((this.currentLayout == mechanicManager.CameraTrackingLayout.MIDDLE ||
          this.currentLayout == mechanicManager.CameraTrackingLayout.DEFAULT) ?
        $r('app.media.track_layout_center') :
        $r('app.media.track_layout_center_dark'))
          .width(CommonConstants.SWITCH_ICON_SIZE)
          .height(CommonConstants.SWITCH_ICON_SIZE)
      }
      .width(CommonConstants.BUTTON_SIZE)
      .height(CommonConstants.BUTTON_SIZE)
      .accessibilityText($r('app.string.mech_center'))
      .backgroundColor((this.currentLayout == mechanicManager.CameraTrackingLayout.MIDDLE ||
        this.currentLayout == mechanicManager.CameraTrackingLayout.DEFAULT) ? $r('sys.color.ohos_id_color_emphasize') :
      $r('sys.color.ohos_id_color_component_normal'))
      .onClick(() => {
        logger.info(`${TAG} onClick TrackingLayoutCenter in`);
        this.onCenterIconClick();
      })
      .type(ButtonType.Circle)
      .clickEffect({ level: ClickEffectLevel.LIGHT, scale: CommonConstants.BUTTON_CLICK_SCALE })

      Text($r('app.string.mech_center'))
        .fontSize($r('sys.float.Body_S'))
        .lineHeight(CommonConstants.LINE_HEIGHT)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('sys.color.font_primary'))
        .margin({ top: CommonConstants.TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
        .width(CommonConstants.BUTTON_SIZE)
    }
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .margin({ left: CommonConstants.OTHER_ICON_MARGIN_LEFT, top: CommonConstants.LAYOUT_ICON_MARGIN_TOP })
  }

  @Builder
  TrackingLayoutRight() {
    Column() {
      Button() {
        Image(this.currentLayout == mechanicManager.CameraTrackingLayout.RIGHT ?
        $r('app.media.track_layout_right') :
        $r('app.media.track_layout_right_dark'))
          .width(CommonConstants.SWITCH_ICON_SIZE)
          .height(CommonConstants.SWITCH_ICON_SIZE)
      }
      .width(CommonConstants.BUTTON_SIZE)
      .height(CommonConstants.BUTTON_SIZE)
      .accessibilityText($r('app.string.mech_right'))
      .backgroundColor(this.currentLayout == mechanicManager.CameraTrackingLayout.RIGHT ?
      $r('sys.color.ohos_id_color_emphasize') : $r('sys.color.ohos_id_color_component_normal'))
      .onClick(() => {
        logger.info(`${TAG} onClick TrackingLayoutRight in`);
        this.onRightIconClick();
      })
      .type(ButtonType.Circle)
      .clickEffect({ level: ClickEffectLevel.LIGHT, scale: CommonConstants.BUTTON_CLICK_SCALE })

      Text($r('app.string.mech_right'))
        .fontSize($r('sys.float.Body_S'))
        .lineHeight(CommonConstants.LINE_HEIGHT)
        .fontWeight(FontWeight.Regular)
        .fontColor($r('sys.color.font_primary'))
        .margin({ top: CommonConstants.TEXT_MARGIN_TOP })
        .textAlign(TextAlign.Center)
        .maxFontScale(FontSizeScale.MD)
        .width(CommonConstants.BUTTON_SIZE)
    }
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .margin({ left: CommonConstants.OTHER_ICON_MARGIN_LEFT, top: CommonConstants.LAYOUT_ICON_MARGIN_TOP })
  }

  build() {
    Column() {
      Flex({
        direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Start
      }) {
        this.BackButton();
        this.TrackingLayoutLeft();
        this.TrackingLayoutCenter();
        this.TrackingLayoutRight();
      }
      .width('100%')
      .height('100%')

    }
    .width('100%')
    .height('100%')
    .justifyContent(FlexAlign.Center)
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM])
    .transition(TransitionEffect.OPACITY.animation({ duration: CommonConstants.ANIMATE_DURATION, curve: Curve.Sharp }))
    .visibility(this.visibilityState)
  }

  setCameraTrackingLayout(layout: mechanicManager.CameraTrackingLayout) {
    logger.info(`${TAG} setCameraTrackingLayout: ${layout}`);
    let originalLayout = this.currentLayout;
    try {
      mechanicManager.setCameraTrackingLayout(layout);
      this.currentLayout = layout;
    } catch (error) {
      this.currentLayout = originalLayout;
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(`${TAG} setCameraTrackingLayout failed. Layout: ${layout}, error.code: ${code}, message: ${message}`);
    }
  }

  onLeftIconClick = () => {
    logger.info(`${TAG} onLeftIconClick in`);
    this.setCameraTrackingLayout(mechanicManager.CameraTrackingLayout.LEFT);
  }

  onCenterIconClick = () => {
    logger.info(`${TAG} onCenterIconClick in`);
    this.setCameraTrackingLayout(mechanicManager.CameraTrackingLayout.MIDDLE);
  }

  onRightIconClick = () => {
    logger.info(`${TAG} onRightIconClick in`);
    this.setCameraTrackingLayout(mechanicManager.CameraTrackingLayout.RIGHT);
  }
}