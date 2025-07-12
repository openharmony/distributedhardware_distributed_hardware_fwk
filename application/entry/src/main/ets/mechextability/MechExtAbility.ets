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

const TAG: string = '[MechExtAbility]';

export default class MechExtAbility extends UIExtensionAbility {
  onCreate() {
    logger.info(`${TAG} UIExtAbility onCreate`);
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