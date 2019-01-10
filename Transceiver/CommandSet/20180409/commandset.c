#include<string.h>
#include "commandset.h"

#ifdef GateWayOnly
bool flag_showtable=0;
#endif
bool GW=0;

bool flag_autosend=0;
uint8_t  autosendCount=0;
bool flag_RSSISNR=0;

//state 
void info(){		
	//UartPrint("DevEUI: ");
	//p64(DevEUI);
	UartPrint("DevAddr: %u\n",DevAddr);
		UartPrint("Datarate: %d\n",datarate);
	UartPrint("TxPower: %d\n",txpower);
	//Uplink “cconfirmed and unconfirmed  frames are transmitted “NNbTrans”ttimes
		UartPrint("NbTrans: %d\n",NbTrans);
}

//change DevEUI
void setDevEUI(uint64_t devEUI){
DevEUI=devEUI;
JoinRequestgenerate();	
}


//change DevAddr
void setDevAddr(uint32_t devAddr){
DevAddr=devAddr;
 MACPayloadgenerate();		
}




void commandcheck( uint32_t uart_read){
	if( strncmp( ( const char*) uart_cmd,( const char*) "gw", 2 ) == 0 )
	{
	GW=1;	
flag_joinprocedure=0;		
	}
if( strncmp( ( const char*) uart_cmd,( const char*) "info", 4 ) == 0 )
	{
	info();				
	}
else if( strncmp( ( const char*) uart_cmd,( const char*) "send", 4) == 0 )
{
	UartPrint("command: send\n");
	// send 0 2
 send(uart_read,uart_cmd[5]-'0',uart_cmd[7]-'0');
}
else if( strncmp( ( const char*) uart_cmd,( const char*) "Jasend", 6) == 0 )
{
	
	uint32_t devaddr;
	uint8_t a[10];
	if(GW==0)
	UartPrint("command:  Jasend\n");
	sscanf(uart_cmd,"%s %d",a,&devaddr);
  Jasend(devaddr);
}
else if( strncmp( ( const char*) uart_cmd,( const char*) "Jrsend", 6) == 0 )
{
Jrsend();
}
else if(strncmp( ( const char*) uart_cmd,( const char*) "autosend", 8) == 0)
{
flag_autosend=1;
}
else if(strncmp( ( const char*) uart_cmd,( const char*) "unautosend", 10) == 0)
{
flag_autosend=0;
}
else if(strncmp( ( const char*) uart_cmd,( const char*) "setDevEUI", 9) == 0)
{
uint64_t devEUI;
	uint8_t a[10];
	UartPrint("command:setDevEUI\n");
	sscanf(uart_cmd,"%s %d",a,&devEUI);	
	UartPrint("setDevEUI: %l\n",devEUI);
	
setDevEUI(devEUI);
}
else if(strncmp( ( const char*) uart_cmd,( const char*) "setDevAddr", 10) == 0)
{
uint32_t dDvAddr;
	uint8_t a[10];
	UartPrint("command:setDevAddr\n");
	sscanf(uart_cmd,"%s %d",a,&dDvAddr);
	UartPrint("setDevAddr: %ll\n",dDvAddr);	
setDevAddr(dDvAddr);
}


#ifdef GateWayOnly
else if(strncmp( ( const char*) uart_cmd,( const char*) "showtable", 9) == 0)
{
flag_showtable=1;
}
#endif
}





/*
* ACK: ACK bit
* mtype:  2 Unconfirmed Data Up
*         3 Unconfirmed Data Down
*         4 Confirmed Data Up
*         5 Confirmed Data Down
*/
void send(uint32_t uart_read, bool FlagACK,uint8_t Flagmtype)
{
	uint8_t i;
/* FCtrl 1byte
*  DL: ADR RFU ACK FPending FOptsLen[3..0]
*  UL: ADR ADRACKReq ACK ClassB FOptsLen[3..0]
*/	
if(FlagACK==1)
{
MACHeader[5]|=  (1<<5);
}
else
{
MACHeader[5]&= ~(1<<5);
}
switch( Flagmtype)
{
	case(2)://MHDR 0100 0000	  Unconfirmed Data Up 
MACHeader[0]&= ~(1<<7);  
MACHeader[0]|=  (1<<6);
MACHeader[0]&= ~(1<<5);	
	ACK=0;
break;	
	case(3)://MHDR 0110 0000	  Unconfirmed Data Down
MACHeader[0]&= ~(1<<7);  
MACHeader[0]|=  (1<<6);
MACHeader[0]|=  (1<<5);
	ACK=0;
break;	
	case(4)://MHDR 1000 0000	 Confirmed Data Up 
MACHeader[0]|=  (1<<7);  
MACHeader[0]&= ~(1<<6);
MACHeader[0]&= ~(1<<5);	
	ACK=1;
	break;
	case(5)://MHDR 1010 0000	  Confirmed Data Down 
MACHeader[0]|=  (1<<7);  
MACHeader[0]&= ~(1<<6);
MACHeader[0]|=  (1<<5);	
	ACK=1;
	default: break;		
}


/*
MACHeader[8]= uart_read-5;	
for(i=0;i<=7;i++)
SBuffer[i]=MACHeader[i];	
strncpy(SBuffer+9,uart_cmd+5,uart_read-5)	;	
	*/
MACHeader[5]=0;	 	
MACHeader[8]= uart_read-9;	
for(i=0;i<=8;i++)
SBuffer[i]=MACHeader[i];	
//UartPrint("uart_read=%d \n",uart_read);
strncpy(SBuffer+9,uart_cmd+9,uart_read-9)	;	
//SBuffer[uart_read+5]='\0';	
SBuffer[uart_read]='\0';	
//header 9 - send' ' => 5	 =4
//Radio.Send( SBuffer,uart_read+5);
Radio.Send( SBuffer,uart_read);
if(FlagACK == 0)	
{
FCntAdd();
}
if( Flagmtype== 4)
{
size_resend= uart_read+4;
}
DelayMs( 200 );


	            for(i=0;i<=8;i++)
                 {
               UartPrint("%d ",SBuffer[i]);
								 }								 
								//UartPrint("send \n");
								for(i=9;i<uart_read;i++) UartPrint("%c",SBuffer[i]);
								  UartPrint("\n");
}



/*!
*  send join-accept, 
*  devaddr: set the devAddr od the end-device that sent the join-request
*/
void Jasend(uint32_t devaddr)
{
DevAddr= devaddr;
Joinacceptgenerate();
Radio.Send( Joinaccept,13);
}
/*!
*  send the join-request
*/
void Jrsend(void)
{
	uint8_t i;
	for(i=0;i<=18;i++)
                 {
               UartPrint("%d ", JoinRequest[i]);
								 }
								 UartPrint("\n");
Radio.Send( JoinRequest, 19 );
}

/*!
*  enable showing RSSI,SNR
*/
void enRS(void)
{
flag_RSSISNR=1;
}
/*!
*  unable showing RSSI,SNR
*/
void unRS(void)
{
flag_RSSISNR=0;
}

void autosend(void)
{
	uint8_t i;
uint8_t temp[2];
					sprintf(temp,"%d",	autosendCount);
					strncpy ( &MACHeader[8],temp,2);		
				autosendCount++;	
					for(i=0;i<8;i++)
UartPrint("%d",MACHeader[i]);
				for(i=8;i<10;i++)
UartPrint("%c",MACHeader[i]);	
					UartPrint("\n");	
Radio.Send( MACHeader, 10 );
	DelayMs( 1000 );
	FCntAdd();
	if(autosendCount==99)
	{
	autosendCount=0;
		flag_autosend=0;
	}
}


void p64(uint64_t var )
{

//var=11268435456;	
uint64_t temp;
uint32_t vr,vl;
vr=0;
vl=0;	
temp= var<<32	;
vr=temp>>32;
	
temp= var>>32	;
vl=temp;	

	UartPrint("%u ",vl);
	UartPrint("%u\n",vr);
	
	//UartPrint("var:%u  p64: vl:%u vr:%u\n",var,vl,vr);
	//UartPrint("var:%u  p64: vr:%u vl:%u\n",var,vr,vl);
	

	
	
}




















				