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

import deviceInfo from '@ohos.deviceInfo';
import { logger } from '../utils/Logger';
import systemParameterEnhance from '@ohos.systemParameterEnhance';

const TAG = '[DeviceUtil] : ';

export enum DeviceType {
  TYPE_PC = 0x0C,
  TYPE_PHONE = 0x0E,
  TYPE_TABLET = 0x11,
  TYPE_2IN1 = 0xA2F,
  TYPE_UNKNOWN = 0x0,
}

enum FoldPhoneTypeValue {
  INVALID_VALUE = -1, //无效值
  STRAIGHT = 0, //直板机
  LARGE_FOLD = 1, //大折（内折）
  SMALL_FOLD = 2, //小折叠（只有内折）(L)
  EXTERNAL_FOLD = 3, //外折
  EXPANDING_NEW_FORMS = 4, //扩展新形态(V)
}

const TYPE_DEFAULT: string = 'default';
const TYPE_PHONE: string = 'phone';
const TYPE_TABLET: string = 'tablet';
const TYPE_PC: string = '2in1';

export class DeviceUtil {
  public static readonly IS_PC: boolean = deviceInfo.deviceType === TYPE_PC;
  public static foldProductTypeValue: FoldPhoneTypeValue = FoldPhoneTypeValue.INVALID_VALUE;

  public static isPhone(): boolean {
    logger.info(`${TAG} isPhone in`);
    return (deviceInfo.deviceType === TYPE_PHONE || deviceInfo.deviceType === TYPE_DEFAULT);
  }

  public static isPC(): boolean {
    logger.info(`${TAG} isPC in`);
    return deviceInfo.deviceType === TYPE_PC;
  }

  public static isDeviceTablet(deviceType: number): boolean {
    logger.info(`${TAG} isDeviceTablet in. deviceType: ${deviceType}`);
    return deviceType === DeviceType.TYPE_TABLET;
  }

  public static isExpandingNewForms(deviceType: number): boolean {
    logger.info(`${TAG} isExpandingNewForms in. deviceType: ${deviceType}`);
    return FoldPhoneTypeValue.EXPANDING_NEW_FORMS === DeviceUtil.getFoldProductType();
  }

  /**
   * 获取代表产品形态的配置项值
   *
   * @returns FoldPhoneTypeValue
   */
  private static getFoldProductType(): number {
    logger.info(`${TAG} getFoldProductType in.`);
    if (DeviceUtil.foldProductTypeValue !== FoldPhoneTypeValue.INVALID_VALUE) {
      return DeviceUtil.foldProductTypeValue;
    }
    try {
      let productValue: string = systemParameterEnhance.getSync('const.window.foldscreen.type', '0.0.0.0');
      logger.info(`${TAG} productValue: ${productValue}`);
      let result: string[] = productValue?.split(',');
      if (result.length > 0) {
        let res = Number.parseInt(result[0]);
        logger.info(`${TAG} productValue res(num): ${res}`);
        return res;
      }
    } catch (err) {
      logger.error(`${TAG} getFoldProductType failed. error.message: ${err.message}`);
    }
    return FoldPhoneTypeValue.INVALID_VALUE;
  }

  /**
   * 是否是小内折产品(L+V)
   *
   * @returns boolean
   */
  public static isSmallFoldProduct(): boolean {
    logger.info(`${TAG} isSmallFoldProduct in.`);
    return [FoldPhoneTypeValue.SMALL_FOLD,
      FoldPhoneTypeValue.EXPANDING_NEW_FORMS].includes(DeviceUtil.getFoldProductType());
  }

  private static deviceTypeToNumber(deviceType: DeviceType): number {
    switch (deviceType) {
      case DeviceType.TYPE_TABLET:
        return 1;
      case DeviceType.TYPE_PHONE:
        return 2;
      case DeviceType.TYPE_2IN1:
        return 3;
      case DeviceType.TYPE_PC:
        return 4;
      default:
        return 5;
    }
  }

  private static deviceTypeToString(deviceType: number): string {
    switch (deviceType) {
      case DeviceType.TYPE_2IN1:
        return TYPE_PC;
      case DeviceType.TYPE_PHONE:
        return TYPE_PHONE;
      case DeviceType.TYPE_TABLET:
        return TYPE_TABLET;
      default:
        return JSON.stringify(deviceType);
    }
  }
}