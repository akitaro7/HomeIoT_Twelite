#include "sht31.h"

uint16_t sht31_readStatus(void) {

  uint8_t data[3];
  i2c_read2(SHT31_DEFAULT_ADDR,      // アドレス
            SHT31_READSTATUS,        // コマンド送信
            data,                    // 読み込むデータを格納するバイト配列。NULLも指定可能。
            3);                      // 読み込むバイト配列の大きさ。

  uint16_t stat = data[0];
  stat <<= 8;
  stat |= data[1];
  return stat;
}

void sht31_reset(void) {

  bool_t ret;
  
  humidity = 0.0f;
  temp = 0.0f;

  i2c_write2(SHT31_DEFAULT_ADDR,    // アドレス
          SHT31_SOFTRESET,          // コマンド送信
          NULL, 0);                 // データなし

  i2c_write2(SHT31_DEFAULT_ADDR,    // アドレス
        SHT31_CLEARSTATUS,          // コマンド送信
        NULL, 0);                   // データなし
  
  // 内蔵ヒーターOFF
  i2c_write2(SHT31_DEFAULT_ADDR,    // アドレス
        SHT31_HEATER_DIS,           // コマンド送信
        NULL, 0);                   // データなし

  sht31_sta = E_SHT31_INIT_END;
}

float sht31_getTemperature(void) {
  return temp;
}

float sht31_getHumidity(void) {
  return humidity;
}

// チェックサム計算
static uint8_t crc8(const uint8_t *data, int len) {

  const uint8_t POLYNOMIAL = 0x31;
  uint8_t crc = 0xFF;

  int i,j;

  for (j = len; j; --j) {
    crc ^= *data++;

    for (i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}

bool_t sht31_readTempHum(void) {
  uint8_t buff[6];
  bool_t ret;

  ret = i2c_read2(SHT31_DEFAULT_ADDR,   // アドレス
            SHT31_MEAS_HIGHREP_STRETCH,   // コマンド送信
            buff, 
            6);             // データなし

  if (ret)
  {
    if (buff[2] != crc8(buff, 2) ||
        buff[5] != crc8(buff + 3, 2))
      return FALSE;

    int32_t stemp = (int32_t)(((uint32_t)buff[0] << 8) | buff[1]);
    stemp = ((4375 * stemp) >> 14) - 4500;
    temp = (float)stemp / 100.0f;

    uint32_t shum = ((uint32_t)buff[3] << 8) | buff[4];
    shum = (625 * shum) >> 12;
    humidity = (float)shum / 100.0f;

    // データ読み出し終了
    sht31_sta = E_SHT31_READ_END;
  }

  return ret;
}
