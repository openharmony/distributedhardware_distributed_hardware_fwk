import wallpaper from '@ohos.wallpaper';
import image from '@ohos.multimedia.image';
import {BusinessError} from '@ohos.base';
import hardwareManager from '@ohos.distributedHardware.hardwareManager';
import common from '@ohos.app.ability.common';
import router from '@ohos.router';
import { Logger } from '../utils/Logger';
import CommonConstants from '../constants/CommonConstants';

const TAG = 'testTag_DHardwareUI : ';
PersistentStorage.persistProp('sign','');

export let isPause:boolean = false;

@Entry
@Component
struct DHardwareUI {
  @State deviceName: string = '';
  @State deviceType: string = '';
  @State DHTypeName:string = '';
  @State isPause:boolean = true;
  @State bgcImg:PixelMap|null = null;
  @StorageLink('type') stoLink_type:number = 0;
  @StorageLink('srcNetworkId') stoLink_srcNetworkId:string = '';
  @StorageLink('deviceName') stoLink_deviceName:string = '';
  @StorageLink('deviceType') stoLink_deviceType:string = '';
  @StorageLink('sign') sign:string = '';
  @State description: hardwareManager.HardwareDescriptor = {
    type: this.stoLink_type,
    srcNetworkId: this.stoLink_srcNetworkId
  }
  private context = getContext(this) as common.UIAbilityContext;

  aboutToAppear(){
    Logger.info(TAG+ 'aboutToAppear ');
    this.sign = '';

    this.deviceType = this.convertDeviceTypeToChStr(this.stoLink_deviceType);
    AppStorage.setAndLink('description',this.description);

    Logger.info(TAG+'aboutToAppear : this.description : '+this.description);
    Logger.info(TAG+'aboutToAppear : this.description.type : '+this.description.type);
    Logger.info(TAG+'aboutToAppear : this.description.srcNetworkId : '+this.description.srcNetworkId);

    if (this.stoLink_type == 1024) {
      this.DHTypeName = '麦克风';
      Logger.info(TAG+'aboutToAppear : this.DHTypeName : '+this.DHTypeName);
    }else {
      this.DHTypeName = '相机';
      Logger.info(TAG+'aboutToAppear : this.DHTypeName : '+this.DHTypeName);

    }
  }

  //转换对端可信设备列表的设备类型为中文字符
  convertDeviceTypeToChStr(deviceType:string): string{
    console.log(TAG + 'convertDeviceTypeToChStr',deviceType);
    switch (deviceType){
      case 'camera':{
        return '相机';
      }
      case 'audio':{
        return '音频';
      }
      case 'pc':{
        return '笔记本电脑';
      }
      case 'phone':{
        return '手机';
      }
      case 'pad':{
        return '平板';
      }
      case 'watch':{
        return '手表';
      }
      case 'car':{
        return '车机';
      }
      case 'tv':{
        return '电视';
      }
      case 'display':{
        return '显示屏';
      }
      case '2in1':{
        return '2in1';
      }
      case 'default':{
        return '手机';
      }
      case 'tablet':{
        return '平板';
      }
      case 'wearable':{
        return '可穿戴电子设备';
      }
      default:{
        return '';
      }
    }
  }

  changePauseOrResumeStatus(){
    this.isPause = !this.isPause;
    console.log(TAG + "changePauseOrResumeStatus - this.isPause" + JSON.stringify(this.isPause))
  }

  stop(){
    Logger.info(TAG + 'stop')
    try {
      hardwareManager.stopDistributedHardware(this.description, () => {
        console.log(TAG + "stopDistributedHardware in ")
      })
    }catch (err){
      Logger.info(TAG + 'stopDistributedHardware err' + err)
    }
  }

  pause(){
    console.log(TAG + 'pause')
    try {
      hardwareManager.pauseDistributedHardware(this.description, () => {
        console.log(TAG + "pauseDistributedHardware in ")
      })
    }catch (err){
      console.log(TAG + 'pauseDistributedHardware err' + err)
    }
  }

  resume(){
    console.log(TAG + 'resume')
    try {
      hardwareManager.resumeDistributedHardware(this.description, () => {
        console.log(TAG + "resumeDistributedHardware in ")
      })
    }catch (err){
      console.log(TAG + 'resumeDistributedHardware err' + err)
    }
  }

  onPageHide(){
    Logger.info(TAG+ 'onPageHide');
  }

  onBackPress(){
    Logger.info(TAG+ 'onBackPress');

    this.sign = 'onBackPress';
  }

  aboutToDisappear(){
    Logger.info(TAG+ 'aboutToDisappear');
  }



  build() {
    Stack(){
      Stack(){
        Image($r('app.media.bgcWall'))
      }

      Row(){
        Column(){
          Row(){
            Image(this.stoLink_type == 1024? $r('app.media.ic_public_voice') : $r('app.media.ic_public_screencap'))
              .width(28)
              .height(28)
              .fillColor(Color.White)
              .margin({left:13})
          }
          .borderRadius(50)
          .width(54)
          .height(54)
          .backgroundColor("rgba(0,0,0,0.15)")

          Text(CommonConstants.CONNECTED_TO + this.stoLink_deviceName + CommonConstants.CONJUNCTS + this.deviceType)
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor("#FFFFFF")
            .margin({left:0,top:16,right:0})
            .textAlign(TextAlign.Center)

          Text(CommonConstants.STOP_DEVICE + this.DHTypeName)
            .fontSize(16)
            .fontWeight(FontWeight.Regular)
            .fontColor("#99FFFFFF")
            .margin({top:2})
            .fontStyle(FontStyle.Normal)
            .textAlign(TextAlign.Center)

          GridRow({columns:5,gutter:{x:24}}) {
            GridCol({ span: 1 })

            GridCol({ span: 3 }) {
              Column() {
                Button(this.isPause ? $r('app.string.pause') : $r('app.string.continue'), {
                  type: ButtonType.Capsule,
                  stateEffect: true
                })
                  .backgroundColor('rgba(255,255,255,0.2)')
                  .height(40)
                  .fontSize(16)
                  .margin({ top: 24 })
                  .width('100%')
                  .onClick(() => {
                    console.log(TAG + (this.isPause ? "click pause" : "click continue"))
                    if (this.isPause) {
                      this.pause();
                      // 点击暂停后：存储一个标志isPause并导出（记录是否点击了暂停按钮）
                      isPause = true;
                    } else {
                      this.resume();
                    }
                    this.changePauseOrResumeStatus();
                  })

                Button($r('app.string.disconnect'), { type: ButtonType.Capsule, stateEffect: true })
                  .backgroundColor('#FFFFFF')
                  .height(40)
                  .fontSize(16)
                  .margin({ top: 16 })
                  .width('100%')
                  .fontColor('rgba(0,0,0,0.9)')
                  .onClick(() => {
                    console.log(TAG + 'click disconnect')
                    this.sign = 'stop';
                    this.stop();

                    try {
                      this.context.terminateSelf((err: BusinessError) => {
                        if (err.code) {
                          console.error(`terminateSelf failed, code is ${err.code}, message is ${err.message}`);
                          return;
                        }
                        console.info(TAG + 'terminateSelf succeed');
                        router.back();
                      });
                    } catch (err) {
                      let code = (err as BusinessError).code;
                      let message = (err as BusinessError).message;
                      console.error(`terminateSelf failed, code is ${code}, message is ${message}`);
                    }
                  })
              }
            }

            GridCol({ span: 1 })
          }

        }
        .width('100%')
        .padding({left:24,right:24})
      }
      .width('100%')
    }

  }
}