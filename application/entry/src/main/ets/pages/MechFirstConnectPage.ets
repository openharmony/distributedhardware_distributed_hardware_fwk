import { logger } from '../utils/Logger';
import { common } from '@kit.AbilityKit';
import { mechanicManager } from '@kit.MechanicKit';
import { LengthMetrics } from '@kit.ArkUI';


const TAG = 'MechFirstConnectPage'

const PAD_BOTTOM_MARGIN: number = 0;
const PHONE_BOTTOM_MARGIN: number = 28;


@Entry
@Preview
@Component
struct MechFirstConnectPage {
    private context: common.UIAbilityContext = getContext(this) as common.UIAbilityContext;
    @State isShow: boolean = false;
    @State bottomPadding: number = PHONE_BOTTOM_MARGIN;


    aboutToAppear(): void {
        logger.info(`${TAG} aboutToAppear`);
    }

    onPageShow(): void {
        logger.info(`${TAG} onPageShow`);

        setTimeout(() => {
            this.isShow = true;
        }, 100)
    }

    onPageHide(): void {
        logger.info(`${TAG} onPageHide`);
    }

    aboutToDisappear(): void {
        logger.info(`${TAG} aboutToDisappear`);
    }

    terminateWindow() {
        try {
            logger.info(`${TAG} terminateWindow`);
            this.context.terminateSelf();
        } catch (e) {
            logger.error(`${TAG} destroyWindow exception, err = ${JSON.stringify(e)}`);
        }
    }

    setCameraTrackingEnabled(isTrackingEnabled: boolean) {
        try {
            logger.info(`${TAG} setCameraTrackingEnabled, isTrackingEnabled == ${isTrackingEnabled}`);
            mechanicManager.setCameraTrackingEnabled(isTrackingEnabled);
        } catch (error) {
            let code: number = (error as BusinessError).code;
            let message: string = (error as BusinessError).message;
            logger.error(`${TAG} setUserOperation switch failed. error.code: ${code}, message: ${message}`);
        }
    }


    @Builder
    contentLayout() {
        Column() {
            SymbolGlyph($r('sys.symbol.dot_viewfinder'))
                .opacity(0.4)
                .fontColor(['#A1A4B3'])
                .fontSize(121)
                .draggable(false)
                .margin({ top: $r('sys.float.padding_level8'), bottom: $r('sys.float.padding_level8') })
                .flexShrink(0)

            Text($r('app.string.harmony_mech_intelligent_tracking'))
                .fontFamily('HarmonyHeiTi')
                .opacity(0.9)
                .fontWeight(FontWeight.Bold)
                .fontSize(20)
                .lineHeight(24)
                .textAlign(TextAlign.Center)
                .margin({ top: $r('sys.float.padding_level6') })

            Text($r('app.string.mech_first_connect_text1'))
                .fontFamily('HarmonyHeiTi')
                .opacity(0.6)
                .fontWeight(FontWeight.Medium)
                .fontSize(14)
                .lineHeight(17)
                .textAlign(TextAlign.Center)
                .margin({ top: $r('sys.float.padding_level6'), bottom: $r('sys.float.padding_level6') })

            Text($r('app.string.mech_first_connect_text2'))
                .fontFamily('HarmonyHeiTi')
                .opacity(0.4)
                .fontWeight(FontWeight.Regular)
                .fontSize(10)
                .lineHeight(12)
                .margin({ top: $r('sys.float.padding_level8') })

            Flex({ direction: FlexDirection.Row }) {
                Button($r('app.string.open_soon'))
                    .buttonStyle(ButtonStyleMode.NORMAL)
                    .height(40)
                    .width('50%')
                    .margin({ end: LengthMetrics.vp(6) })
                    .onClick(() => {
                        logger.info(`${TAG} open soon onclick`);
                        this.isShow = false;
                    })

                Button($r('app.string.open_now'))
                    .buttonStyle(ButtonStyleMode.EMPHASIZED)
                    .height(40)
                    .width('50%')
                    .margin({ start: LengthMetrics.vp(6) })
                    .onClick(() => {
                        logger.info(`${TAG} open now onclick`);
                        this.setCameraTrackingEnabled(true);
                        this.isShow = false;
                    })
            }
            .width('100%')
            .padding({ top: $r('sys.float.padding_level8'), bottom: $r('sys.float.padding_level8') })
        }
        .alignItems(HorizontalAlign.Center)
        .padding({
            left: $r('sys.float.padding_level8'),
            right: $r('sys.float.padding_level8'),
            bottom: this.bottomPadding,
            top: 64
        })
        .visibility(this.isShow ? Visibility.Visible : Visibility.Hidden)
        .onTouch((event: TouchEvent) => {
            event.stopPropagation();
        })
    }


    build() {
        Flex() {
        }
        .bindSheet($$this.isShow, this.contentLayout(), {
            height: SheetSize.FIT_CONTENT,
            blurStyle: BlurStyle.NONE,
            enableOutsideInteractive: false,
            dragBar: false,
            showClose: true,
            backgroundColor: $r('sys.color.ohos_id_color_panel_bg'),
            onTypeDidChange: (sheetType) => {
                if (sheetType === SheetType.BOTTOM) {
                    this.bottomPadding = PHONE_BOTTOM_MARGIN;
                } else {
                    this.bottomPadding = PAD_BOTTOM_MARGIN;
                }
            },
            onAppear: () => {
                logger.info(`${TAG} bindSheet onAppear`);
            },
            onDisappear: () => {
                logger.info(`${TAG} onDisappear enter`);
                this.terminateWindow();
            }
        })
        .width('100%')
        .height('100%')
        .backgroundColor(this.isShow ? $r('sys.color.mask_fourth') : '#00000000')
    }
}