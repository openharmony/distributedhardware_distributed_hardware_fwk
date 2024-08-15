/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import common from '@ohos.app.ability.common';
import bundleManager from '@ohos.bundle.bundleManager';
import hilog from '@ohos.hilog';
import Want from '@ohos.app.ability.Want';
import { BusinessError } from '@ohos.base';
import util from '@ohos.util';

const DOMAIN_ID = 0x3F3F;
const TAG = 'TipsJump';
const APP_BUNDLE_NAME = 'com.huawei.hmos.tips';
const TIPS_APP_ID =
  'com.huawei.hmos.tips_BB4oLXt8JLOw5djd42S0oLOGzO6kOnT8hZfFRAAel2gbcQBG5jIsO4genni5cn2SQpKpKvkwOA7Ajsc7qf+MZgM=';
const MODULE_NAME_PHONE = 'phone_widget';
const MODULE_NAME_PC = 'pc_widget';
const URI_FORMATTER = 'hwtips://com.huawei.hmos.tips.app?funNum=%s&type=%s';
const MODULE_NAME_APPGALLERY = 'com.huawei.hmsapp.appgallery';
const URI_APPGALLERY = 'store://appgallery.huawei.com/app/detail?id=com.huawei.hmos.tips';
const CODE_BUNDLE_NAME_NOT_FOUND = 17700001;
const CODE_SUCCESS_ERROR = 0;
const FUN_NUM_HOME = 'SF-20005815_f101';
type startAbleContext = common.UIAbilityContext | common.ServiceExtensionContext;

export class TipsJumpUtils {
  /**
   * Jump to Tips Home
   *
   * @param startAbleContext:common.UIAbilityContext | common.ServiceExtensionContext
   * @param type: entry type ID, used for dotting
   * @param moduleName: phone_widget: jump phone, pc_widget: jump pc
   */
  public static jumpHome(context: startAbleContext, type: string, moduleName: string = MODULE_NAME_PHONE) {
    TipsJumpUtils.jumpTips(context, FUN_NUM_HOME, type, moduleName ?? MODULE_NAME_PHONE);
  }

  /**
   * Jump to the specified page of Tips by FUN_NUM
   *
   * @param startAbleContext:common.UIAbilityContext | common.ServiceExtensionContext
   * @param funNum: page FUN_NUM
   * @param type: entry type ID, used for dotting
   * @param moduleName: phone_widget: jump phone, pc_widget: jump pc
   */
  public static jumpTips(context: startAbleContext, funNum: string, type: string,
                         moduleName: string = MODULE_NAME_PHONE) {
    let uri = util.format(URI_FORMATTER, funNum, type);
    TipsJumpUtils.jumpTipsByUri(context, uri, moduleName ?? MODULE_NAME_PHONE);
  }

  /**
   * Jump to the specified page of Tips by uri
   *
   * @param startAbleContext:common.UIAbilityContext | common.ServiceExtensionContext
   * @param uri: uri format:hwtips://com.huawei.hmos.tips.app?funNum=xxx&type=xxx
   * @param moduleName: phone_widget: jump phone, pc_widget: jump pc
   */
  public static jumpTipsByUri(context: startAbleContext, uri: string, moduleName: string = MODULE_NAME_PHONE) {
    if (moduleName !== MODULE_NAME_PHONE && moduleName !== MODULE_NAME_PC) {
      hilog.error(DOMAIN_ID, TAG, `unknown moduleName, supported value:"${MODULE_NAME_PHONE}" or "${MODULE_NAME_PC}"`);
      return;
    }
    TipsJumpUtils.isAppInstalled().then((isAppInstalled: boolean) => {
      if (isAppInstalled) {
        TipsJumpUtils.jumpAppByUri(context, uri);
      } else {
        TipsJumpUtils.jumpAppGallery(context);
      }
    })
  }

  /**
   * Jump to Tips APP by uri
   *
   * @param startAbleContext:common.UIAbilityContext | common.ServiceExtensionContext
   * @param uri: uri format:hwtips://com.huawei.hmos.tips.app?funNum=xxx&type=xxx
   */
  private static jumpAppByUri(context: startAbleContext, uri: string) {
    hilog.info(DOMAIN_ID, TAG, 'try jump to tips app');
    let want: Want = {
      bundleName: APP_BUNDLE_NAME,
      action: 'ohos.want.action.viewData',
      entities: ['entity.system.browsable'],
      uri
    };
    context.startAbility(want, (err: BusinessError) => {
      if (err.code !== CODE_SUCCESS_ERROR) {
        hilog.warn(DOMAIN_ID, TAG, 'jump to [tips app] failed, error: %{private}s', JSON.stringify(err));
      } else {
        hilog.info(DOMAIN_ID, TAG, 'jump to [tips app] success');
      }
    });
  }

  /**
   * Jump to AppGallery by uri
   *
   * @param startAbleContext:common.UIAbilityContext | common.ServiceExtensionContext
   */
  private static jumpAppGallery(context: startAbleContext) {
    hilog.info(DOMAIN_ID, TAG, 'try to jump to AppGallery');
    const want: Want = { bundleName: MODULE_NAME_APPGALLERY, uri: URI_APPGALLERY };
    context.startAbility(want).then(() => {
      hilog.info(DOMAIN_ID, TAG, 'jump to AppGallery success');
    }).catch(() => {
      hilog.warn(DOMAIN_ID, TAG, 'jump to AppGallery error');
    });
  }

  private static isAppInstalled(): Promise<boolean> {
    return bundleManager.getBundleInfo(APP_BUNDLE_NAME, bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_SIGNATURE_INFO)
      .then((info) => {
        if (TIPS_APP_ID === info.signatureInfo.appId) {
          hilog.info(DOMAIN_ID, TAG, 'tips app is installed');
          return true;
        } else {
          hilog.warn(DOMAIN_ID, TAG, 'tips app is forged');
          return false;
        }
      })
      .catch((err: BusinessError) => {
        if (err.code === CODE_BUNDLE_NAME_NOT_FOUND) {
          hilog.info(DOMAIN_ID, TAG, 'tips app is not installed')
        } else {
          hilog.info(DOMAIN_ID, TAG, 'failed to check if tips app is installed, err: %{private}s', JSON.stringify(err));
        }
        return false;
      })
  }
}