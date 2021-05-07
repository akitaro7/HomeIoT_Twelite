/*
 * mySensorIF.h
 *
 * Definitions for transmit data inter face
 */
#include <stdint.h>

#define NAME_TX_BYTE_LEN    7
#define NAME_DATA_BYTE_LEN  4
#define DATA_TX_MAX         9

typedef struct 
{
    /* Header data */
    char                nameTx[NAME_TX_BYTE_LEN];       //送信名前              7byte
    char                numData;                        //センサーデータ        1byte
    uint16_t            onBoardTmp;                     //温度                 2byte
    uint16_t            batteryMV;                      //バッテリー電圧[mV]    2byte
    uint32_t            timesTx;                        //送信回数              4byte
} mySensorHeader_t_typ;

typedef struct 
{
    /* data 8byte */
    char                nameData[NAME_DATA_BYTE_LEN];   //データ名前            4byte
    int32_t             val;                            //データ                4byte
} mySensorData_t_typ;

// MAX 92 byte
typedef struct 
{
    /* sensorf I/F data */
    mySensorHeader_t_typ    head;                       //共通情報              16byte
    mySensorData_t_typ      data[DATA_TX_MAX];          //センサデータ          8*9=72byte
} mySensorIF_t_typ;
