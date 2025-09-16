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
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import Want from '@ohos.app.ability.Want';
import { Configuration } from '@ohos.app.ability.Configuration';
import { logger } from '../utils/Logger';

const TAG: string = '[ContinueSwitchAbility]';

export default class ContinueSwitchAbility extends UIExtensionAbility {
  onCreate() {
    logger.info(`${TAG} UIExtAbility onCreate`);
    AppStorage.setOrCreate('currentColorMode', this.context.config.colorMode);
    AppStorage.setOrCreate('currentFontSizeScale', this.context.config.fontSizeScale);
  }

  onConfigurationUpdate(newConfig: Configuration) {
    AppStorage.setOrCreate('currentColorMode', this.context.config.colorMode);
    AppStorage.setOrCreate('currentFontSizeScale', this.context.config.fontSizeScale);
  }

  onForeground() {
    logger.info(`${TAG} UIExtAbility onForeground`);
  }

  onBackground() {
    logger.info(`${TAG} UIExtAbility onBackground`);
  }

  onDestroy() {
    logger.info(`${TAG} UIExtAbility onDestroy`);
  }

  onSessionCreate(want: Want, session: UIExtensionContentSession) {
    logger.info(`${TAG} UIExtAbility onSessionCreate.`);
    let parameters = want.parameters;
    let pushParams = want.parameters?.pushParams as string | undefined;
    let startReason = '';
    let isShowBack =
      pushParams?.includes('isShowBack') ? (pushParams.includes('isShowBack:false') ? false : true) : true;
    let navigationMode = NavigationMode.Auto;

    if (parameters) {
      startReason = parameters.startReason as string;
      navigationMode = parameters.navigationMode as NavigationMode;
      logger.info(`${TAG} settings navigationMode: ${navigationMode}`);
    }

    if (startReason === 'from_search' && navigationMode === NavigationMode.Split) {
      logger.info(`${TAG} current display is search & split mode`);
      isShowBack = false;
    } else {
      logger.info(`${TAG} current display is not search & split mode`);
      isShowBack = true;
    }

    AppStorage.setOrCreate('continueSession', session);
    AppStorage.setOrCreate('startReason', startReason);
    AppStorage.setOrCreate('isShowBack', isShowBack);
    let param: Record<string, UIExtensionContentSession> = {
      'session': session
    };
    let storage: LocalStorage = new LocalStorage(param);
    session.loadContent('pages/ContinueSwitch', storage);
    logger.info(`${TAG} onSessionCreate end. startReason: ${startReason}, isShowBack: ${isShowBack}`);
  }

  onSessionDestroy(session: UIExtensionContentSession) {
    logger.info(`${TAG} UIExtAbility onSessionDestroy`);
  }
}