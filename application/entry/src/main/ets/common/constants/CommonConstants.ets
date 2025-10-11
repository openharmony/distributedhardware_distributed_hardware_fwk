/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import { logger } from '../../utils/Logger';
import i18n from '@ohos.i18n';

export default class CommonConstants {
  //ContinueSwitch page :
  public static readonly IMAGE_COUNT: number = 84;
  public static readonly IMG_ANIMATOR_OVER_DURATION: number = 1000;
  public static readonly IMG_ANIMATOR_NORMAL_DURATION: number = 30;
  public static readonly SWITCH_STATUS_OPEN: string = '1';
  public static readonly SWITCH_STATUS_CLOSE: string = '0';
  public static readonly GENERAL_PHONE_GAP_LENGTH: number = 16;
  public static readonly PC_PAD_GAP_LENGTH: number = 24;
  public static readonly PC_LIST_HEIGHT: number = 48;
  public static readonly PHONE_LIST_HEIGHT: number = 56;
  public static readonly TEXT_LIST_ALIGN_DISTANCE: number = 12;
  public static readonly PC_BORDER_RADIUS: number = 16;
  public static readonly NON_PC_BORDER_RADIUS: number = 20;
  public static readonly CONTINUE_DESC_TEXT_MARGIN_BOTTOM: number = 32;
  public static readonly CONTINUE_DESC_TEXT_MARGIN_TOP: number = 24;
  public static readonly CONTINUE_PRIVACY_TEXT_MARGIN_TOP: number = 4;
  public static readonly CONTINUE_SWITCH_WIDTH: number = 36;
  public static readonly CONTINUE_SWITCH_HEIGHT: number = 20;
  public static readonly ITEM_LIST_PADDING_TOP_PC: number = 13;
  public static readonly ITEM_LIST_PADDING_BOTTOM_PC: number = 13;
  public static readonly ITEM_LIST_PADDING_TOP: number = 17;
  public static readonly ITEM_LIST_PADDING_BOTTOM: number = 18;
  public static readonly CONTINUE_SWITCH_KEY: string = 'Continue_Switch_Status';
  public static readonly FONT_SIZE_SCALE_PARAM: string = 'persist.sys.font_scale_for_user0';
  public static readonly ANIMATOR_HEIGHT_PHONE: number = 192;
  public static readonly ANIMATOR_HEIGHT_PC: number = 207;
  public static readonly ANIMATOR_HEIGHT_PAD: number = 212;
  public static readonly ANIMATOR_WIDTH_PHONE: number = 290;
  public static readonly ANIMATOR_WIDTH_PC: number = 312;
  public static readonly ANIMATOR_WIDTH_PAD: number = 320;
  public static readonly SETTING_PAGE_NAME: string = 'continue_setting_page';
  // Tips
  public static readonly TIPS_TYPE: string = '105';
  public static readonly FUN_NUM = 'SF-20005815_f101';
  // Symbol
  public static readonly SYMBOL_INFO_CIRCLE: string = '14vp';
  public static readonly SYMBOL_MARGIN_RIGHT: number = 4;
  // Update prompt text
  public static readonly UPDATE_PROMPT_LINE_HEIGHT: number = 16;
  public static readonly UPDATE_PROMPT_MARGIN_TOP: number = 9;

  /**
   * Button click Scale.
   */
  public static readonly BUTTON_CLICK_SCALE: number = 0.9;

  /**
   * Animation duration.
   */
  public static readonly ANIMATE_DURATION: number = 150;

  /**
   * Line height.
   */
  public static readonly LINE_HEIGHT: number = 16;

  /**
   * Text Margin top 4.
   */
  public static readonly TEXT_MARGIN_TOP: number = 4;

  /**
   * Text Margin left 20.
   */
  public static readonly TEXT_MARGIN_LEFT: number = 20;

  /**
   * Text Max font size.
   */
  public static readonly MAX_FONT_SCALE: number = 1.5;

  /**
   * Left Icon margin left.
   */
  public static readonly LEFT_ICON_MARGIN_LEFT: number = 30;

  /**
   * Other layout Icon margin left.
   */
  public static readonly OTHER_ICON_MARGIN_LEFT: number = 36;

  /**
   * Back symbol size.
   */
  public static readonly BACK_SYMBOL_SIZE: string = '20vp';

  /**
   * Switch icon size.
   */
  public static readonly SWITCH_ICON_SIZE: number = 20;

  /**
   * Switch icon margin.
   */
  public static readonly SWITCH_ICON_MARGIN: number = 3;

  /**
   * Small line height.
   */
  public static readonly SMALL_LINE_HEIGHT: number = 13;

  /**
   * Button size.
   */
  public static readonly BUTTON_SIZE: string = '40vp';

  /**
   * Small text margin top.
   */
  public static readonly SMALL_TEXT_MARGIN_TOP: number = 2;

  /**
   * Translate x distance.
   */
  public static readonly TRANSLATE_X: number = 60;

  /**
   * Animation springMotion response.
   */
  public static readonly SPRING_MOTION_RESPONSE: number = 0.55;

  /**
   * Animation springMotion dampingFraction.
   */
  public static readonly SPRING_MOTION_DAMPING_FRACTION: number = 0.825;

  /**
   * Animation springMotion overlapDuration.
   */
  public static readonly SPRING_MOTION_OVERLAP_DURATION: number = 0.15;

  /**
   * Normal switch margin left.
   */
  public static readonly NORMAL_SWITCH_MARGIN_LEFT: number = 60;

  /**
   * Max scale switch margin left.
   */
  public static readonly MAX_SWITCH_MARGIN_LEFT: number = 75;

  /**
   * Normal Layout margin left.
   */
  public static readonly NORMAL_LAYOUT_MARGIN_LEFT: number = 46;

  /**
   * Max Layout margin left.
   */
  public static readonly MAX_LAYOUT_MARGIN_LEFT: number = 65;

  /**
   * Bundle name.
   */
  public static readonly BUNDLE_NAME: string = 'com.ohos.dhardwareui';

  /**
   * Switch off offset.
   */
  public static readonly SWITCH_OFF_OFFSET: number = 4;

  /**
   * Switch text width.
   */
  public static readonly SWITCH_TEXT_WIDTH: number = 73;

  /**
   * Switch icon margin top.
   */
  public static readonly SWITCH_ICON_MARGIN_TOP: number = 23;

  /**
   * Layout icon margin top.
   */
  public static readonly LAYOUT_ICON_MARGIN_TOP: number = 31;

  /**
   * Switch icon margin top when ultra long languages.
   */
  public static readonly SWITCH_ICON_MARGIN_TOP_ULTRA: number = 10;

  /**
   * Switch icon margin top when ultra long languages.
   */
  public static readonly LAYOUT_MARGIN_RIGHT_MIRROR: number = 19;
}

export enum FontSizeScale {
  SM = 0.85,
  MD = 1.0,
  DEFAULT = MD,
  L = 1.15,
  XL = 1.3,
  XXL = 1.45,
  XXL1 = 1.75,
  XXL2 = 2,
  XXL3 = 3.2,
}

export class LanguagesUtil {
  // isMirrorLanguages
  public static readonly MIRROR_LANGUAGES: Array<string> = ['ug', 'ar'];
  // isTibetanLanguages
  public static readonly TIBETAN_LANGUAGES: Array<string> = ['bo'];
  // isUltraLongLanguages
  public static readonly ULTRA_LONG_LANGUAGES: Array<string> = ['ru'];

  public static isMirrorLanguages(): boolean {
    logger.info(`isMirrorLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    return LanguagesUtil.MIRROR_LANGUAGES.includes(locale);
  }

  public static isUltraLongLanguages(): boolean {
    logger.info(`isUltraLongLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    logger.info(`isUltraLongLanguages: ${locale}`);
    return LanguagesUtil.ULTRA_LONG_LANGUAGES.includes(locale);
  }

  public static isTibetanLanguages(): boolean {
    logger.info(`isTibetanLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    logger.info(`isTibetanLanguages: ${locale}`);
    return LanguagesUtil.TIBETAN_LANGUAGES.includes(locale);
  }
}
