/*
 * basicio.h で紫外線計測する
 */
#include "basicio.h"
#include "mySensorIF.h"
#include "myRadioKey.h"
#include "sht31.h"

#define LED 16

//アナログデジタル変換
#define ADC_CNT 2
#define ADC_TGT (ADC_SOURCE_BITMAP_TEMP | ADC_SOURCE_BITMAP_VOLT)

//送信名称
#define TX_NAME ("BR_TH")  //MAX7文字

uint16_t u16AdcData[ADC_CNT] = {};
uint32_t cntTxTimes = 0;
mySensorIF_t_typ txData;

//送信の共通情報を設定
void setTxHeadData()
{
    //送信名称を設定
    strncpy(txData.head.nameTx, TX_NAME, NAME_TX_BYTE_LEN);
    //送信回数を設定
    cntTxTimes++;
    txData.head.timesTx = cntTxTimes;
    //機器情報を設定
    txData.head.onBoardTmp = u16AdcData[ADC_CNT-2];     //ボード温度
    txData.head.batteryMV = u16AdcData[ADC_CNT-1];      //電圧
}

//センサデータを設定
bool_t pushSensorData(char* name, int32_t val)
{
    if (txData.head.numData >= DATA_TX_MAX)
    {
        //最大送信データ数を超過
        return FALSE;
    }
    
    //データを送信バッファへ設定
    int idx = txData.head.numData;
    strncpy(txData.data[idx].nameData, name, NAME_DATA_BYTE_LEN);
    txData.data[idx].val = val;

    //送信データ数をインクリメント
    txData.head.numData++;

    return TRUE;
}

//無線データ送信バッファをすべて送信する
void sendDataBuff()
{
    radio_write(
        HOST_ADD,
        0,                      //データタイプ 0～7。自由に設定可
        (uint8_t*)&txData,
        sizeof(txData)-sizeof(mySensorData_t_typ)*(DATA_TX_MAX-txData.head.numData)
        );
}

//ADC変換が完了した
void adcDone() {
    //送信データクリア
    memset(&txData, 0, sizeof(txData));

    //************************************************************
    //送信データの設定は【ここから】行う
    //************************************************************
    //送信の共通情報を設定
    setTxHeadData();

    // センサデータを読み出して送信する
    int32_t temp, hum;
    temp = (int32_t)(sht31_getTemperature() * 1000.0) ;
    hum = (int32_t)(sht31_getHumidity() * 1000.0) ;

    //センサデータを無線送信データへ設定
    pushSensorData("TEMP", temp);
    pushSensorData("HUM", hum);

    //************************************************************
    //送信データの設定は【ここまで】で行う
    //************************************************************

    //無線データ送信バッファをすべて送信する
    sendDataBuff();

    sht31_sta = E_SHT31_SEND_END;
}

// 起動時や起床時に呼ばれる関数
void setup(bool_t warmWake, uint32_t bitmapWakeStatus) {

    // 動作確認用のLED
    dio_pinMode(LED, OUTPUT);
    dio_write(LED, HIGH);

    //無線通信を初期化する
    radio_setupInit(
        RADIO_MODE_TXONLY,  //送信のみ
        APP_ID,             //アプリケーションID (受信側も同じ値)
        CHANNEL,            //チャンネル（受信側も同じ値）
        3);                 //無線出力(最大値)

    //ADCを設定
    adc_enable(ADC_SAMPLE_6,  //サンプル数 6
        ADC_CLOCK_500KHZ,     //ADCクロック 500KHZ
        FALSE);               //内部の基準電圧を使用

    //センサステータスを初期化
    sht31_sta = E_SHT31_NOSTA;

    // I2C通信を有効にします。
    i2c_enable(I2C_CLOCK_100KHZ,     // 通信速度
                FALSE);              // 使用ピンセット切り替え

    //シリアル0の初期化
    serial_init(
        SERIAL_BAUD_115200 //ボーレート
        );
}

// setup()後、３種類のイベントで呼ばれるループ関数
void loop(EVENTS event) {
    if (event == EVENT_START_UP) {
        // 最初に呼ばれる
        //自身のモジュールアドレスを表示
        // radio_printf(
        //     RADIO_ADDR_BROADCAST,   //不特定多数に送信
        //     0,                      //データタイプ 0～7。自由に設定可
        //     "Send module address = %X\r\n", getModuleAddress());         //送信内容
    } else if (event == EVENT_TICK_SECOND) {
        // 1秒毎に呼ばれる
    } else if (event == EVENT_TICK_TIMER) {
        // 4ミリ秒毎(デフォルト)に呼ばれる

        switch (sht31_sta) {
            case E_SHT31_NOSTA:
            {
                sht31_reset();
                break;
            }
            case E_SHT31_INIT_END:
            {
                sht31_readTempHum();
            }
            default:
                break;
        }

        if (sht31_sta == E_SHT31_READ_END) {
            //ADC開始
            adc_attachCallbackWithTimer(
                0,                  //タイマー番号。0~４
                0,                  //プリスケーラー。 0~16
                0,                  //呼び出し周期を決定する16ビットカウンタの目標値。
                FALSE,              //0V～基準電圧×1　(内部基準電圧使用時、0V～1.235V)
                ADC_TGT,            //測定対象を指定します。
                u16AdcData,         //結果を格納するバッファ。uint_16t型配列。
                ADC_CNT,            //バッファの要素数。2～2047
                FALSE,              //FALSE バッファがいっぱいになったら終了
                ADC_INT_FULL ,      //バッファいっぱいまで変換結果が書き込まれたとき
                adcDone);           //コールバック関数
                
            sht31_sta = E_SHT31_SEND_WAIT;
        }
        else if (sht31_sta == E_SHT31_SEND_END && radio_txCount() == 0) {
            // 動作確認用のLED消灯
            dio_write(LED, LOW);

            //AD変換が完了し、無線送信中のデータもなくなった
            //10秒後起床でスリープする
            sleepTimer(10000, FALSE);
        }
        else {
            //nop
        }
    }
}
