#include "loraheader.h"
/*!
*  for all command bool state is the bit that indicate the command need to receive or to send
*  1: send
*  0: receive
*/


/*!
* LoRaWAN MAC Commands
*  CID   Command         Transmitteed by    hort  Description
*                        ED       GW
*                        (1:send )
*/


/*!
*  0x02  LinkADRReq      0        1        Requests the end-device to change data rate, transmit power, repetition rate or channel.
*  value of datarate equal to LORA_SPREADING_FACTOR
*/
//Haven't handle chmask
//Haven't handlechmaskcntl
void  LinkADRReq(bool state,uint8_t datarate, uint8_t txpower,uint8_t chmask, uint8_t nbtrans, uint8_t chmaskcntl);



/*!
*  0x03  LinkADRAns      1        0        Acknowledges the LinkADRReq.
*/
void LinkADRAns(void);



//uint8_t datarate, uint8_t txpower,uint8_t chmask, uint8_t nbtrans, uint8_t chmaskcntl