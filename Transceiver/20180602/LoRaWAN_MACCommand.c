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
*  
*  value of datarate equal to LORA_SPREADING_FACTOR
*/
//Haven't handle chmask
//Haven't handlechmaskcntl
//state 1: send  0:setting
void  LinkADRReq(bool _state,uint8_t _datarate, uint8_t _txpower,uint8_t chmask, uint8_t _nbtrans, uint8_t chmaskcntl)
{
	if(_state==1)
	{		
uint8_t i,temp1,temp;			
//MHDR 101 confirmed  DL	 
//MACHeader[0]|=  (1<<7);  
//MACHeader[0]&= ~(1<<6);
//MACHeader[0]|=  (1<<5);
		
//Unconfirmed Data Up 
MACHeader[0]&= ~(1<<7);  
MACHeader[0]|=  (1<<6);
MACHeader[0]&= ~(1<<5);			
//FOptsLen[3..0]			
MACHeader[5]=4;		
////MACHeader[8]: : 	CID  2
MACHeader[8]=2;
//MACHeader[9]:  [7:4]datarate  [3:0]txpower
MACHeader[9]=0;		
UartPrint("_datarate= %d   _txpower= %d ",_datarate,_txpower);		
MACHeader[9]|=(_datarate<<4);
MACHeader[9]|=(_txpower);

UartPrint("MACHeader[9]: ");
for(i=0;i<=7;i++)
{
	temp=MACHeader[9]<<i;
	temp1= temp>>7;
	UartPrint("%d",temp1);
 } 
UartPrint("\n");
 //MACHeader[10]:  [7:4]datarate  [3:0]txpower
		
	Radio.Send( MACHeader,10);	
DelayMs( 1000 );
 	Radio.Send( MACHeader,10);	
 DelayMs( 1000 );
 	Radio.Send( MACHeader,10);	
 FCntAdd();
	}
	else{
		txpower=_txpower;
		datarate= _datarate;
		//LoRabandwith = _LORA_BANDWIDTH;
	// datarate and  txpower and nbtrans
		/*
    Radio.SetTxConfig( MODEM_LORA, _txpower, 0, LoRabandwith,
                                   datarate, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
		*/
		Radio.SetTxConfig( MODEM_LORA, _txpower, 0, LORA_BANDWIDTH,
                                   datarate, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
		
		
    
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH,_datarate,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
	NbTrans=_nbtrans;
		// Radio initialization
    Radio.SetChannel( RF_FREQUENCY );
		UartPrint("Radio initialization again\n");
Radio.Rx( RX_TIMEOUT_VALUE );


	}
	
}


/*!
*  0x03  LinkADRAns      1        0        Acknowledges the LinkADRReq.
*/
//100 confirmed  UL
void LinkADRAns(void)
{

}


