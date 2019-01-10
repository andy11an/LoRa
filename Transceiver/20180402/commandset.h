#include "loraheader.h"

#ifdef GateWayOnly
extern bool flag_showtable;
#endif
extern bool GW;

/*!
* autosend 1~99
*/
extern bool flag_autosend;
extern uint8_t  autosendCount;
/*!
* show RSSI,SNR after package
*/
extern bool flag_RSSISNR;











/*!
* show state of DevEUI, DevAddr, Datarate, TxPower, NbTrans
*/
void info(void);
/*!
*  setDevEUI
*/
void setDevEUI(uint64_t devEUI);
void setDevAddr(uint32_t devAddr);
//void send(uint32_t uart_read);
void send(uint32_t uart_read, bool FlagACK ,uint8_t Flagmtype);


void commandcheck( uint32_t uart_read);

/*!
*  send join-accept, 
*  devaddr: set the devAddr od the end-device that sent the join-request
*/
void Jasend(uint32_t devaddr);
/*
*  send the join-request
*/
void Jrsend(void);
/*
*  enable showing RSSI,SNR
*/
void enRS(void);
/*
*  unable showing RSSI,SNR
*/
void unRS(void);
/*!
* send 1~99 for test
*/
void autosend(void);

void p64(uint64_t var );

 