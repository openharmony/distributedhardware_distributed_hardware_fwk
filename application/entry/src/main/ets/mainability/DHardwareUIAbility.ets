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

import AbilityConstant from '@ohos.app.ability.AbilityConstant';
import hilog from '@ohos.hilog';
import UIAbility from '@ohos.app.ability.UIAbility';
import Want from '@ohos.app.ability.Want';
import window from '@ohos.window';
import hardwareManager from '@ohos.distributedHardware.hardwareManager';
import { BusinessError } from '@ohos.base';
import { Configuration } from '@ohos.app.ability.Configuration';
import screenLock from '@ohos.screenLock';

const TAG = '[testTag_DHardwareUIAbility] : ';
let systemLanguage: string | undefined;

PersistentStorage.persistProp('camera', 0);
AppStorage.setOrCreate('camera', 0);
console.log(TAG + 'OUT : AppStorage.get<number>(camera) : ' + AppStorage.get<number>('camera'));

PersistentStorage.persistProp('mic', 0);
AppStorage.setOrCreate('mic', 0);
console.log(TAG + 'OUT : AppStorage.get<number>(mic) : ' + AppStorage.get<number>('mic'));

export default class DHardwareUIAbility extends UIAbility {
  private isInit: boolean = true;
  private isStopAbility: boolean = false;

  onConfigurationUpdate(newConfig: Configuration) {
    console.info(`onConfigurationUpdated systemLanguage is ${systemLanguage}, newConfig: ${JSON.stringify(newConfig)}`);

    if (systemLanguage !== newConfig.language) {
      console.info(TAG + `systemLanguage from ${systemLanguage} changed to ${newConfig.language}`);
      systemLanguage = newConfig.language;
    }
  }

  stop() {
    console.info(TAG + 'stop');
    try {
      hardwareManager.stopDistributedHardware(AppStorage.get('description') as hardwareManager.HardwareDescriptor)
        .then(() => {
          console.log('stop distributed hardware successfully');
        })
        .catch((error: BusinessError) => {
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
      hardwareManager.pauseDistributedHardware(AppStorage.get('description') as hardwareManager.HardwareDescriptor)
        .then(() => {
          console.log('pause distributed hardware successfully');
        })
        .catch((error: BusinessError) => {
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
      hardwareManager.resumeDistributedHardware(AppStorage.get('description') as hardwareManager.HardwareDescriptor)
        .then(() => {
          console.log('resume distributed hardware successfully');
        })
        .catch((error: BusinessError) => {
          console.error('resume distributed hardware failed, cause:' + error);
        })
      console.log('resume distributed hardware successfully');
    } catch (error) {
      console.error('resume distributed hardware failed:' + error);
    }
  }


  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam) {
    console.info(TAG + 'onCreate : ' + 'Ability onCreate');

    systemLanguage = this.context.config.language;
    console.info(TAG + `systemLanguage is ${systemLanguage} `);

    this.isStopAbility = false;
    console.info(TAG + 'onCreate : ' + 'this.isStopAbility : ' + this.isStopAbility);

    if (want.parameters != undefined) {
      console.info(TAG + 'onCreate : want.parameters.type : ' + JSON.stringify(want.parameters['type']));
      console.info(TAG + 'onCreate : want.parameters.srcNetWorkId : ' + JSON.stringify(want.parameters['srcNetWorkId']));
      console.info(TAG + 'onCreate : want.parameters.deviceName : ' + JSON.stringify(want.parameters['deviceName']));
      console.info(TAG + 'onCreate : want.parameters.deviceType : ' + JSON.stringify(want.parameters['deviceType']));
      console.info(TAG + 'onCreate : want.parameters.returnCode : ' + JSON.stringify(want.parameters['returnCode']));

      AppStorage.setOrCreate('type', want.parameters['type']);
      AppStorage.setOrCreate('srcNetworkId', want.parameters['srcNetworkId']);
      AppStorage.setOrCreate('deviceName', want.parameters['deviceName']);
      AppStorage.setOrCreate('deviceType', want.parameters['deviceType']);

      console.log(TAG + 'oncreate : after set want -- type --');

      // startAbility:type+1
      if (want.parameters['type'] == 1) {
        let camera: number = AppStorage.get<number>('camera') as number + 1;
        AppStorage.set('camera', camera);
        console.log(TAG + 'onCreate : ' + 'AppStorage.get<number>(camera) : ' + AppStorage.get<number>('camera'));
      } else if (want.parameters['type'] == 1024) {
        let mic: number = AppStorage.get<number>('mic') as number + 1;
        AppStorage.set('mic', mic);
        console.log(TAG + 'onCreate : ' + 'AppStorage.get<number>(mic) : ' + AppStorage.get<number>('mic'));
      }

      if (want.parameters['returnCode'] == 24200102) {
        try {
          this.context.terminateSelf((err: BusinessError) => {
            if (err.code) {
              console.error(`terminateSelf failed,code is ${err.code},message is ${err.message}`);
              return;
            }
            console.log(TAG + 'terminateSelf succeed');
          });
        } catch (err) {
          let code = (err as BusinessError).code;
          let message = (err as BusinessError).message;
          console.error(`terminateSelf failed,code is ${code},message is ${message}`);
        }
      }
    }
  }

  onNewWant(want: Want) {
    console.info(TAG + 'onNewWant : ' + 'Ability onNewWant');

    if (want.parameters != undefined) {
      console.info(TAG + 'onNewWant : want.parameters : ' + JSON.stringify(want.parameters));
      console.info(TAG + "onNewWant : want.parameters['returnCode'] : " + JSON.stringify(want.parameters['returnCode']));
      console.info(TAG + "onNewWant : want.parameters['type'] : " + JSON.stringify(want.parameters['type']));

      AppStorage.setOrCreate('returnCode', want.parameters['returnCode']);
      AppStorage.setOrCreate('type', want.parameters['type']);

      let returnCode: number = want.parameters['returnCode'] as number;
      console.info(TAG + 'onNewWant : returnCode : ' + returnCode);
      AppStorage.setOrCreate('returnCode', returnCode);

      // returnCode==undefined:type+1
      if (returnCode == undefined) {
        // 切换前后置后刷新按钮文本为‘暂停’
        AppStorage.setOrCreate('isPause', true);
        console.log(TAG + 'onNewWant : refresh Btn Text : ' + 'AppStorage.get<boolean>(isPause) : ' + AppStorage.get<boolean>('isPause'));

        if (want.parameters['type'] == 1) {
          let camera: number = AppStorage.get<number>('camera') as number + 1;
          AppStorage.set('camera', camera);
          AppStorage.setOrCreate('switchedCamera', camera);
          console.log(TAG + 'onNewWant : ' + '[returnCode==undefined]--[stop camera]--AppStorage.get<number>(camera) : ' + AppStorage.get<number>('camera'));
        } else if (want.parameters['type'] == 1024) {
          let mic: number = AppStorage.get<number>('mic') as number + 1;
          AppStorage.set('mic', mic);
          console.log(TAG + 'onNewWant : ' + '[returnCode==undefined]--[stop mic]--AppStorage.get<number>(mic) : ' + AppStorage.get<number>('mic'));
        }
      }

      // returnCode == 24200102: stopAbility called
      if (returnCode == 24200102) {
        this.isStopAbility = true;
        console.info(TAG + 'onNewWant :[returnCode == 24200102] this.isStopAbility : ' + this.isStopAbility);

        // stopAbility:type-1
        if (want.parameters['type'] == 1) {
          let camera: number = AppStorage.get<number>('camera') as number - 1;
          AppStorage.set('camera', camera);
          console.log(TAG + 'onNewWant : ' + '[returnCode==24200102]--[stop camera]--AppStorage.get<number>(camera) : ' + AppStorage.get<number>('camera'));
        } else if (want.parameters['type'] == 1024) {
          let mic: number = AppStorage.get<number>('mic') as number - 1;
          AppStorage.set('mic', mic);
          console.log(TAG + 'onNewWant : ' + '[returnCode==24200102]--[stop mic]--AppStorage.get<number>(mic) : ' + AppStorage.get<number>('mic'));
        }

        if (AppStorage.get<number>('camera') == 0 && AppStorage.get<number>('mic') == 0) {
          try {
            this.context.terminateSelf((err: BusinessError) => {
              if (err.code) {
                console.error(`terminateSelf failed, code is ${err.code}, message is ${err.message}`);
                return;
              }
              console.info(TAG + 'terminateSelf succeed');
            });
          } catch (err) {
            let code = (err as BusinessError).code;
            let message = (err as BusinessError).message;
            console.error(`terminateSelf failed, code is ${code}, message is ${message}`);
          }
        }
      }
    }
  }

  onDestroy() {
    console.info(TAG + 'onDestroy : ' + 'Ability onDestroy');

    let sign: string = AppStorage.get<string>('sign') as string;
    if (sign == 'stop') {
      return;
    }
    let stopFlag = AppStorage.get<number>('camera') == 0 && AppStorage.get<number>('mic') == 0;

    // stopAbility:app destroys itself: the stop function is no longer executed
    if (!stopFlag && AppStorage.get<string>('sign') != 'onBackPress') {
      console.info(TAG + 'onDestroy : ' + 'this.isStopAbility ： ' + this.isStopAbility);
      console.info(TAG + 'onDestroy : ' + "AppStorage.get<string>('sign') ： " + AppStorage.get<string>('sign'));
      this.stop();
      console.info(TAG + 'onDestroy : ' + 'Distributed Hardware stopped');
    }

  }

  onWindowStageCreate(windowStage: window.WindowStage) {
    // Main window is created, set main page for this ability
    console.info(TAG + 'onWindowStageCreate : ' + 'Ability onWindowStageCreate');

    windowStage.loadContent('pages/DHardwareUI', (err, data) => {
      if (err.code) {
        hilog.error(0x0000, 'testTag', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err) ?? '');
        return;
      }
      hilog.info(0x0000, 'testTag', 'Succeeded in loading the content. Data: %{public}s', JSON.stringify(data) ?? '');
    });
  }

  onWindowStageDestroy() {
    // Main window is destroyed, release UI related resources
    console.info(TAG + 'onWindowStageDestroy : ' + 'Ability onWindowStageDestroy');

  }

  eventFunc() {
    console.info(TAG + 'call anonymous eventFunc : pause');
  }

  onForeground() {
    // Ability has brought to foreground
    console.info(TAG + 'onForeground : ' + 'Ability onForeground');

    // isInit:Whether the app is initialized for the first time into the background
    AppStorage.setOrCreate('isInit', this.isInit);
    console.info(TAG + 'onForeground : ' + " AppStorage.get<boolean>('isInit') : " + AppStorage.get<boolean>('isInit'));
    console.info(TAG + 'onForeground : ' + " AppStorage.get<boolean>('isPauseTag') : " + AppStorage.get<boolean>('isPauseTag'));

    // Back to foreground again: Neither initializing the page for the first time nor pausing state triggers the resume
    if (!this.isInit && !AppStorage.get<boolean>('isPauseTag')) {
      console.info(TAG + 'onForeground : ' + 'this.isInit : ' + this.isInit);

      if (AppStorage.get<number>('camera') != 0 || AppStorage.get<number>('mic') != 0) {
        console.log(TAG + 'onForeground : ' + 'AppStorage.get<number>(camera) : ' + AppStorage.get<number>('camera'));
        console.log(TAG + 'onForeground : ' + 'AppStorage.get<number>(mic) : ' + AppStorage.get<number>('mic'));

        this.resume();
      }
    }
    this.isInit = false;
    AppStorage.setOrCreate('isInit', false);
    // Restore unClicked pause state: Click Pause state initialization
    AppStorage.setAndLink('isPause', true);
    console.info(TAG + 'onForeground : ' + "AppStorage.get<boolean>('isPause') : " + AppStorage.get<boolean>('isPause'));
  }

  onBackground() {
    // Ability has back to background
    console.info(TAG + 'onBackground : ' + 'Ability onBackground');

    let sign: string = AppStorage.get<string>('sign') as string;
    if (sign == 'stop') {
      return;
    }
    // stopAbility:app destroys itself: the pause function is no longer executed
    let stopFlag = AppStorage.get<number>('camera') == 0 && AppStorage.get<number>('mic') == 0;
    console.log(TAG + 'onBackground : stopFlag : ' + stopFlag)
    let isScreenLock = screenLock.isLocked();
    console.log(TAG + 'onBackground : isScreenLock : ' + isScreenLock)
    if (!stopFlag && !isScreenLock) {
      console.info(TAG + 'onBackground : ' + 'this.isStopAbility : ' + this.isStopAbility);
      this.pause();
      console.info(TAG + 'onBackground : ' + 'Distributed Hardware paused');
    }
  }

  onBackPressed() {
    return true;
  }
}
