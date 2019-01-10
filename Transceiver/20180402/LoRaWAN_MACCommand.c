#include<string.h>
#include "LoRaWAN_MACCommand.h"

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
void  LinkADRReq(bool state,uint8_t datarate, uint8_t txpower,uint8_t chmask, uint8_t nbtrans, uint8_t chmaskcntl)
{
	if(state==1)
	{
uint8_t i,temp1,temp;			
//MHDR 101 confirmed  DL	 
MACHeader[0]|=  (1<<7);  
MACHeader[0]&= ~(1<<6);
MACHeader[0]|=  (1<<5);
//FOptsLen[3..0]			
MACHeader[5]=4;
//MACHeader[8]:  [7:4]datarate  [3:0]txpower
temp = datarate<<3;
MACHeader[8]|=(temp>>7);  
temp = datarate<<2;
MACHeader[8]|=(temp>>7);
temp = datarate<<1;
MACHeader[8]|=(temp>>7);
temp = datarate;
MACHeader[8]|=(temp>>7);	

temp = txpower<<3;
MACHeader[8]|=(temp>>7);  
temp = txpower<<2;
MACHeader[8]|=(temp>>7);
temp = txpower<<1;
MACHeader[8]|=(temp>>7);
temp = txpower;
MACHeader[8]|=(temp>>7);

UartPrint("MACHeader[8]: ");
for(i=0;i<=7;i++)
{
	temp=MACHeader[0]<<i;
	temp1= temp>>7;
	UartPrint("%d",temp1);
 } 
UartPrint("\n");
		
		
	}
	else{
	// datarate and  txpower and nbtrans
    Radio.SetTxConfig( MODEM_LORA, txpower, 0, LORA_BANDWIDTH,
                                   datarate, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH,datarate,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
	NbTrans=nbtrans;
	}
	
}


/*!
*  0x03  LinkADRAns      1        0        Acknowledges the LinkADRReq.
*/
//100 confirmed  UL
void LinkADRAns(void)
{

}


