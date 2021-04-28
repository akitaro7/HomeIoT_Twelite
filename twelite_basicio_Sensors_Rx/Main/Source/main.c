/*
 * basicio.h でセンサから受けたデータをシリアル通信する
 */
#include "basicio.h"
#include "mySensorIF.h"
#include "myRadioKey.h"

//無線受信コールバック
void radioRxFunc(uint32_t u32SrcAddr, bool_t bBroadcast, uint8_t u8CbId,
    uint8_t u8DataType, uint8_t *pu8Data, uint8_t u8Length, uint8_t u8Lqi) {

    //受信情報を表示
    serial_printf("from=%X, ID=%d, LQI=%d, DataType=%d, DataLen=%d, ",
        u32SrcAddr, u8CbId, u8Lqi, u8DataType, u8Length);

    //受信データのアンパック
    mySensorIF_t_typ rxData;
    memcpy(&rxData, pu8Data, u8Length);

    //データを表示
    char tmpNameTx[NAME_TX_BYTE_LEN+1] = {};
    memcpy(tmpNameTx, rxData.head.nameTx, NAME_TX_BYTE_LEN);
    serial_printf("txName=%s, tmp=%d, bat=%d, times=%d, numData=%d, ",
        tmpNameTx, rxData.head.onBoardTmp, rxData.head.batteryMV, rxData.head.timesTx, rxData.head.numData);

    int i;
    char tmpDataName[NAME_DATA_BYTE_LEN+1] = {};
    for (i = 0; i < rxData.head.numData; i++)
    {
        memcpy(tmpDataName, rxData.data[i].nameData, NAME_DATA_BYTE_LEN);
        serial_printf("d%dName=%s, d%dVal=%d, ",
            i+1, tmpDataName, i+1, rxData.data[i].val);
    }
    serial_puts("\r\n");
}

// 起動時や起床時に呼ばれる関数
void setup(bool_t warmWake, uint32_t bitmapWakeStatus) {
    //シリアル0の初期化
    serial_init(SERIAL_BAUD_115200);
    //無線通信を初期化する
    radio_setupInit(
        RADIO_MODE_TXRX,    //送受信可能
        APP_ID,             //アプリケーションID (受信側も同じ値)
        CHANNEL,            //チャンネル（受信側も同じ値）
        3);                 //無線出力(最大値)
    //無線受信コールバックを登録
    radio_attachCallback(NULL, radioRxFunc);
}

// setup()後、３種類のイベントで呼ばれるループ関数
void loop(EVENTS event) {
    if (event == EVENT_START_UP) {
        // 最初に呼ばれる
        //自身のモジュールアドレスを表示
        serial_printf("My module address = %X\r\n", getModuleAddress());
    } else if (event == EVENT_TICK_SECOND) {
        // 1秒毎に呼ばれる
    } else if (event == EVENT_TICK_TIMER) {
        // 4ミリ秒毎(デフォルト)に呼ばれる
    }
}