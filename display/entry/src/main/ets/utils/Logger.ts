import hilog from '@ohos.hilog';

export class Logger{
  static PREFIX: string = '[DHardware_UI]'
  static DOMAIN: number = 0xFF00
  static FORMAT: string = '%{public}s'

  static debug(...args: any[]) {
    hilog.debug(Logger.DOMAIN, Logger.PREFIX, Logger.FORMAT, args)
  }

  static info(...args: any[]) {
    hilog.info(Logger.DOMAIN, Logger.PREFIX, Logger.FORMAT, args)
  }

  static warn(...args: any[]) {
    hilog.warn(Logger.DOMAIN, Logger.PREFIX, Logger.FORMAT, args)
  }

  static error(...args: any[]) {
    hilog.error(Logger.DOMAIN, Logger.PREFIX, Logger.FORMAT, args)
  }
}
