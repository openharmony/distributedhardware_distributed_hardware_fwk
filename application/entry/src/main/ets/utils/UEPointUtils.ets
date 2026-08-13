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

import hiSysEvent from '@ohos.hiSysEvent';
import { BusinessError } from '@ohos.base';
import mechanicManager from '@ohos.distributedHardware.mechanicManager';
import { logger } from '../utils/Logger';


const TAG = 'UE_POINT_UTILS';

const DOMAIN = 'HIMECH_UE';
const PKG_NAME = 'com.ohos.dhardwareui';
const MECH_CENTER_LAYOUT_SWITCH_EVENT = 'MECH_CENTER_LAYOUT_SWITCH';
const MECH_CENTER_TRACKING_SWITCH_EVENT = 'MECH_CENTER_TRACKING_SWITCH';


export class UEPointUtils {
  private static instance: UEPointUtils;

  private constructor() {
  }

  public static getInstance(): UEPointUtils {
    if (!UEPointUtils.instance) {
      UEPointUtils.instance = new UEPointUtils();
    }
    return UEPointUtils.instance;
  }

  private writeEvent(info: hiSysEvent.SysEventInfo): void {
    hiSysEvent.write(info).then(() => {
        logger.info(`${TAG} hiSysEvent success to write event`);
    }).catch((err: BusinessError) => {
        logger.error(`${TAG} hiSysEvent err.code: ${JSON.stringify(err.code)}, err.message: ${JSON.stringify(err.message)}`);
    });
  }

  public reportCameraTrackingLayoutEvent(cameraTrackingLayout: mechanicManager.CameraTrackingLayout): void {
    logger.info(`${TAG} reportLayoutSwitchStateEvent enter`);
    let info: hiSysEvent.SysEventInfo = {
      domain: DOMAIN,
      name: MECH_CENTER_LAYOUT_SWITCH_EVENT,
      eventType: hiSysEvent.EventType.BEHAVIOR,
      params: {
        PNAMEID: PKG_NAME,
        CAMERA_TRACKING_LAYOUT: cameraTrackingLayout
      }
    };
    this.writeEvent(info);
  }

  public reportFocusTrackingSwitchStateEvent(focusTrackingSwitchState: boolean): void {
    logger.info(`${TAG} reportFocusTrackingSwitchStateEvent enter`);
    let info: hiSysEvent.SysEventInfo = {
      domain: DOMAIN,
      name: MECH_CENTER_TRACKING_SWITCH_EVENT,
      eventType: hiSysEvent.EventType.BEHAVIOR,
      params: {
        PNAMEID: PKG_NAME,
        FOCUSTRACKING_SWITCH: focusTrackingSwitchState
      }
    };
    this.writeEvent(info);
  }
}