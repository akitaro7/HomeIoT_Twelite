#include "basicio.h"

#ifndef SHT31_H
#define SHT31_H

#define SHT31_DEFAULT_ADDR 0x45
#define SHT31_MEAS_HIGHREP_STRETCH 0x2C06
#define SHT31_READSTATUS 0xF32D
#define SHT31_CLEARSTATUS 0x3041
#define SHT31_SOFTRESET 0x30A2
#define SHT31_HEATER_DIS 0x3066

  typedef enum 
  {
    E_SHT31_NOSTA=0,
    E_SHT31_INIT_RESET,
    E_SHT31_INIT_CLEAR,
    E_SHT31_INIT_HEATEROFF,
    E_SHT31_INIT_WAIT,
    E_SHT31_INIT_END,
    E_SHT31_READ_WAIT,
    E_SHT31_READ_END,
    E_SHT31_SEND_WAIT,
    E_SHT31_SEND_END,
    E_SHT31_MAX
  } eSHT31_STA;

  eSHT31_STA sht31_sta;

  uint16_t sht31_readStatus(void);
  void sht31_reset(void);
  float sht31_getTemperature(void);
  float sht31_getHumidity(void);
  bool_t sht31_readTempHum(void);

// read Data
  float humidity;
  float temp;


#endif
