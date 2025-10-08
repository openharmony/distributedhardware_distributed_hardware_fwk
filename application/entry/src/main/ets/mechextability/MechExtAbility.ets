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
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import Want from '@ohos.app.ability.Want';
import { logger } from '../utils/Logger';
import mechanicManager from '@ohos.distributedHardware.mechanicManager';
import { BusinessError } from '@ohos.base';

const TAG: string = '[MechExtAbility]';

export default class MechExtAbility extends UIExtensionAbility {
  private callback = (result: mechanicManager.TrackingEventInfo) => {
    logger.info(`${TAG} result.event = ${result.event}`);
    if (result.event === mechanicManager.TrackingEvent.CAMERA_TRACKING_USER_ENABLED) {
      AppStorage.set('isTrackingEnabled', true);
    } else if (result.event === mechanicManager.TrackingEvent.CAMERA_TRACKING_USER_DISABLED) {
      AppStorage.set('isTrackingEnabled', false);
    }
    logger.info(`${TAG}trackingStateChange callback end, AppStorage.get(isTrackingEnabled) =
      ${AppStorage.get<boolean>('isTrackingEnabled')}`);
  };

  onTrackingStateChange() {
    logger.info(`${TAG} onTrackingStateChange in`);
    try {
      mechanicManager.on('trackingStateChange', this.callback);
    } catch (error) {
      let code: number = error.code;
      let message: string = error.message;
      logger.error(`${TAG}trackingStateChange on failed. error.code: ${code}, message: ${message}`);
    }
  }

  offTrackingStateChange() {
    logger.info(`${TAG} offTrackingStateChange in`);
    try {
      mechanicManager.off('trackingStateChange', this.callback);
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      logger.error(`${TAG}trackingStateChange off failed. error.code: ${code}, message: ${message}`);
    }
  }

  onCreate() {
    logger.info(`${TAG} UIExtAbility onCreate`);
    AppStorage.setOrCreate('isTrackingEnabled', true);
    AppStorage.setOrCreate('currentLayout', mechanicManager.CameraTrackingLayout.DEFAULT);
    this.onTrackingStateChange();
  }

  onForeground() {
    logger.info(`${TAG} UIExtAbility onForeground`);
  }

  onBackground() {
    logger.info(`${TAG} UIExtAbility onBackground`);
  }

  onDestroy() {
    logger.info(`${TAG} UIExtAbility onDestroy`);
    this.offTrackingStateChange();
  }

  onSessionCreate(want: Want, session: UIExtensionContentSession) {
    logger.info(`${TAG} UIExtAbility onSessionCreate.`);

    let param: Record<string, UIExtensionContentSession> = {
      'session': session
    };
    let storage: LocalStorage = new LocalStorage(param);
    session.loadContent('pages/MechControl', storage);
    session.setWindowBackgroundColor('#00000000');
    logger.info(`${TAG} onSessionCreate end.`);
  }

  onSessionDestroy(session: UIExtensionContentSession) {
    logger.info(`${TAG} UIExtAbility onSessionDestroy`);
  }
}