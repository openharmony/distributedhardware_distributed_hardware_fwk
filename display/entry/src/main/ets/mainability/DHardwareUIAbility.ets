/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import power from '@ohos.power';
import screenLock from '@ohos.screenLock';
import {BusinessError} from '@ohos.base';
import router from '@ohos.router';
import * as shareData from '../pages/DHardwareUI'

const TAG = 'testTag_DHardwareUIAbility : '

export default class DHardwareUIAbility extends UIAbility {

  isInit:boolean = true; //是否是第一次初始化
  isStopAbility:boolean = false;

  stop(){
    console.info(TAG + 'stop')
    try {
      hardwareManager.stopDistributedHardware(AppStorage.get('description') as hardwareManager.HardwareDescriptor, () => {
        console.log(TAG + "stopDistributedHardware in ")
      })
    }catch (err){
      console.info(TAG + 'stopDistributedHardware err' + err)
    }
  }

  pause(){
    console.log(TAG + 'pause')
    try {
      hardwareManager.pauseDistributedHardware(AppStorage.get('description') as hardwareManager.HardwareDescriptor, () => {
        console.log(TAG + "pauseDistributedHardware in ")
      })
    }catch (err){
      console.log(TAG + 'pauseDistributedHardware err' + err)
    }
  }

  resume(){
    console.log(TAG + 'resume');
    try {
      hardwareManager.resumeDistributedHardware(AppStorage.get('description') as hardwareManager.HardwareDescriptor, () => {
        console.log(TAG + "resumeDistributedHardware in ")
      })
    }catch (err){
      console.log(TAG + 'resumeDistributedHardware err' + err)
    }
  }

  screenToUnlock(){
    // 检测当前设备是否处于活动状态(有屏的设备为亮屏状态)
    try {
      let isActive = power.isActive();
      console.info('power is active: ' + isActive);
      // 灭屏状态：
      if(!isActive){
        // 唤醒设备
        try {
          power.wakeup('wakeup_test');
        } catch(err) {
          console.error('wakeup failed, err: ' + err);
        }

        // 解锁屏幕
        screenLock.unlock((err:BusinessError, data:Boolean) => {
          if (err) {
            console.error(`Failed to unlock the screen, Code: ${err.code}, message: ${err.message}`);
            return;
          }
          console.info(`Succeeded in unlocking the screen. result: ${data}`);
        });

      }
    } catch(err) {
      console.error('check active status failed, err: ' + err);
    }
  }

  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam) {
    console.info(TAG+ 'onCreate : '+'Ability onCreate');

    // 每次被拉起时，默认设置没有执行StopAbility
    this.isStopAbility = false;
    console.info(TAG+'onCreate : ' +'this.isStopAbility : '+ this.isStopAbility);

    if (want.parameters != undefined) {
      console.info(TAG+'onCreate : want.parameters : '+JSON.stringify(want.parameters));

      AppStorage.setOrCreate('type',want.parameters['type']);
      AppStorage.setOrCreate('srcNetworkId',want.parameters['srcNetworkId']);
      AppStorage.setOrCreate('deviceName',want.parameters['deviceName']);
      AppStorage.setOrCreate('deviceType',want.parameters['deviceType']);

    }

    this.screenToUnlock();

  }

  onNewWant(want:Want){
    console.info(TAG+ 'onNewWant : '+'Ability onNewWant');

    if(want.parameters!=undefined){
      console.info(TAG+'onNewWant : want.parameters : '+JSON.stringify(want.parameters));
      console.info(TAG+"onNewWant : want.parameters['returnCode'] : "+JSON.stringify(want.parameters['returnCode']));

      AppStorage.setOrCreate('returnCode',want.parameters['returnCode']);

      let returnCode:number =want.parameters['returnCode'] as number;
      console.info(TAG+'onNewWant : returnCode : '+returnCode);

      // 接收到的returnCode如果满足条件，则表面已调用stopAbility
      if (returnCode == 24200102) {
        this.isStopAbility = true;
        console.info(TAG+'onNewWant : this.isStopAbility : '+this.isStopAbility);
        try {
          this.context.terminateSelf((err: BusinessError)=>{
            if (err.code) {
              console.error( `terminateSelf failed, code is ${err.code}, message is ${err.message}`);
              return;
            }
            console.info(TAG+'terminateSelf succeed');
            router.back();
          });
        }catch (err){
          let code = (err as BusinessError).code;
          let message = (err as BusinessError).message;
          console.error( `terminateSelf failed, code is ${code}, message is ${message}`);
        }
      }
    }
    this.screenToUnlock();
  }

  onDestroy() {
    console.info(TAG+ 'onDestroy : '+'Ability onDestroy');

    let sign:string = AppStorage.get<string>('sign') as string;
    if (sign == 'stop') {
      return;
    }

    // stopAbility后应用销毁自身：不再执行停止函数
    if (!this.isStopAbility && !(AppStorage.get<string>('sign')=='onBackPress')) {
      console.info(TAG+ 'onDestroy : '+'this.isStopAbility ： '+this.isStopAbility);
      console.info(TAG+ 'onDestroy : '+"AppStorage.get<string>('sign') ： "+AppStorage.get<string>('sign'));

      this.stop();
      console.info(TAG+ 'onDestroy : '+'Distributed Hardware stopped');
    }

  }

  onWindowStageCreate(windowStage: window.WindowStage) {
    // Main window is created, set main page for this ability
    console.info(TAG+ 'onWindowStageCreate : '+'Ability onWindowStageCreate');

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
    console.info(TAG+ 'onWindowStageDestroy : '+'Ability onWindowStageDestroy');

  }

  eventFunc(){
    console.info(TAG + 'call anonymous eventFunc : pause');
  }

  onForeground() {
    // Ability has brought to foreground
    console.info(TAG+ 'onForeground : '+'Ability onForeground');

    this.context.eventHub.on('pause',this.pause);
    this.context.eventHub.on('resume',this.resume);

    if (!this.isInit && !shareData.isPause) {
      this.context.eventHub.emit('resume');
      console.info(TAG+'onForeground : ' + 'this.isInit : ' + this.isInit);
      console.info(TAG+'onForeground : ' + 'shareData.isPause : ' + shareData.isPause);
    }

    this.isInit = false;

    AppStorage.setAndLink('isPause',false);
    console.info(TAG+'onForeground : ' + "AppStorage.get<boolean>('isPause') : "+ AppStorage.get<boolean>('isPause'));
  }

  onBackground() {
    // Ability has back to background
    console.info(TAG+ 'onBackground : '+'Ability onBackground');

    let sign:string = AppStorage.get<string>('sign') as string;
    if (sign == 'stop') {
      return;
    }
    // stopAbility后应用销毁自身：不再执行暂停函数
    if (!this.isStopAbility){
      console.info(TAG+ 'onBackground : '+'this.isStopAbility' + this.isStopAbility);
      this.pause();
      console.info(TAG+ 'onBackground : '+'Distributed Hardware paused');
    }
  }
}
