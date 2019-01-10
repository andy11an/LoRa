/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"


#include "commandset.h"
#include "loraheader.h"
#include "LoRaWAN_MACCommand.h"



typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;
States_t State = LOWPOWER;

typedef enum
{
	Jr,  //000
	Ja,  //001
	uUL, //010
	uDL,//011
	cUL,//100
	cDL,//101
	Rr,//110
	Pr,	//111
}MType;
MType mtype = Pr;

int16_t RssiValue = 0;
int8_t SnrValue = 0;
int8_t CID;
int8_t temp8bit,temp8bit2;



/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );
/*!
 * \brief Check the  ACK bit
 */
void ACKbitcheck(void);
/*!
 * \brief handle the receive packet
 */
void receive_data_handle(void); 
 /*!
 * \brief Check the  mtype
 */
void mtype_check(void);
/**
 * Main application entry point.
 */
int main( void )
{
    uint32_t uart_read = 0;
    uint8_t i;
	  

    // Target board initialisation
    BoardInitMcu( );
    BoardInitPeriph( );

    UartPrint("\nS7678S SDK V1.43\n");

#if POWER_SAVING_DEMO
    //Enter power-saving stop mode and then back to normal mode.
    UartPrint("Enter Power Saving mode for %ds ...\n", POWER_SAVING_INTERVAL);
    Demo_Enter_Stop_Mode(POWER_SAVING_INTERVAL, GPIOA);
    UartPrint("Leave Power Saving mode ...\n", POWER_SAVING_INTERVAL);
#endif

    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );
		UartPrint("Radio initialization again\n");

#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

#elif defined( USE_MODEM_FSK )

    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );
    
    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif
    
    Radio.Rx( RX_TIMEOUT_VALUE );

#if ENABLE_UART2
		Uart2Init();
#endif

		JoinRequestgenerate();
    MACPayloadgenerate();	
		Joinacceptgenerate();	
									
//0421 ADR testing
GW=1;																	
flag_joinprocedure=0;
flag_autosend=1;		
autosendCount=0;																	
    while( 1 )
    {
        //cmd
        uart_read = UartScan(uart_cmd);
        if ( uart_read != 0 ) 
				{
            UartPrint("\n>> %s\n", uart_cmd);	
				  // 	commandcheck(uart_read);
					  
        }	
				//ED: ACK handle
				
				if((ACK==1) && (flag_NbTrans<NbTrans))
				{
					UartPrint("ACK==1 resend confirmed package:  \n");
					for(i=0;i<=8;i++)
                 {
               UartPrint("%d ",SBuffer[i]);
								 }
								 UartPrint("\n");			 
				Radio.Send(SBuffer,size_resend);
				Radio.Rx( RX_TIMEOUT_VALUE );				 
        flag_NbTrans++;
				//send 	NbTrans times		 
        if( flag_NbTrans == NbTrans)
				{
				ACK=0;
				flag_NbTrans=0;	
				}	
				
				}
						 if(flag_autosend==1 ){
								autosend();		
								 }	
				
	//ED: join procedure
	/*
			if(flag_joinprocedure)
				{
			  if(state_connect==0 && State == LOWPOWER)
				{
				Radio.Send( JoinRequest, 19 );
					FCntAdd();
					JoinRequestgenerate();
					for(i=0;i<=18;i++)
                 {
               UartPrint("%d ", JoinRequest[i]);
								 }
								 UartPrint("\n");
								 
								Radio.Rx( 5000 );
								 DelayMs( 5000 );
								 Radio.Rx( 6000 );
								 DelayMs( 6000 );
							//	 DelayMs( 2000 );								
            // Radio.Rx( RX_TIMEOUT_VALUE );
				}
			}
				*/
        switch( State )
        {
        case RX:  
					//testing
				/*
if(BufferSize>5 && Buffer[0] != '\0')
	                {	
										UartPrint("a");
										memset(Buffer,'0x00',sizeof(Buffer));
                Buffer[0] = '\0';	
								}
									*/
				
			  receive_data_handle();	
				// ED autosend for test?

				
				
				/*
					     	 if(Buffer[0] != '\0')
                {									
		
								UartPrint("!");
	              UartPrint("%d",BufferSize);
							  UartPrint("!");
	            for(i=0;i<=8;i++)
                 {
               UartPrint("%d ",Buffer[i]);
								 }								 
								//UartPrint("send \n");
								for(i=9;i<BufferSize;i++) UartPrint("%c",Buffer[i]);
								 
								 
								 //to cut payload' '!XX!
								  UartPrint(" ");						 							 
								 //®i­N
								 UartPrint("RSSI:%4d SNR:%4d ",RssiValue,SnrValue);
								 //for RSSI:123 SNR:-123  2or3 bytes
	
								memset(Buffer,'0x00',sizeof(Buffer));
                Buffer[0] = '\0';								
                }	
								*/
								
							
				/*
                Radio.Rx( RX_TIMEOUT_VALUE );
								 DelayMs( 1500 );
             Radio.Rx( RX_TIMEOUT_VALUE );
						 */
						 
             State =RX;
						  //State =RX_ERROR;
            break;
        case TX:

#if LED_ENABLE
            GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
#endif
           // Radio.Rx( RX_TIMEOUT_VALUE );
            //State = LOWPOWER;
			    	State =RX; 
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
					
					 
				      
			//	UartPrint("v1 RX_TIMEOUT/RX_ERROR send: \n");
				
            State = RX;
            break;
        case TX_TIMEOUT:
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
        default:
            // Set low power
            break;
        }
    
        TimerLowPowerHandler( );
#if ENABLE_I2C
				I2C_CpltPolling(0x80);
#endif
				
			Radio.Rx( RX_TIMEOUT_VALUE );
								 DelayMs( 1500 );
             Radio.Rx( RX_TIMEOUT_VALUE );	
				
				
    }
}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
		Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
}
void ACKbitcheck(void)
{
uint8_t t,t1;

	t=Buffer[5]<< 2;
	t1=t>>7;
	//UartPrint("ACK check:  ");
	if(t1==1)
	{
	//	UartPrint("ACK bit == 1\n");
	ACK = 0;
	}	
	else
	{
//	UartPrint("ACK bit == %d\n",t1);
	}
	
}
void mtype_check(void)
{
			uint8_t b,c;	
			uint8_t  handle;
			//sscanf(Buffer[0],"%d",&handle); 
	handle=Buffer[0];
			b=handle<<(0);
			c=b>>(7);
			if(c) //1
			{
				b=handle<<(1);
				c=b>>(7);
				if(c)//11
				{
				b=handle<<(2);
					c=b>>(7);
					if(c)//111
					{
					mtype=Pr;
						// UartPrint("mtype=Pr\n");
					}
					else//110
						{
					mtype=Rr;
						// UartPrint("mtype=Rr\n");
					}
				}
				else//10
				{
				b=handle<<(2);
					c=b>>(7);
					if(c)//101
					{
					mtype=cDL;
						// UartPrint("mtype=cDL\n");
						ACKbitcheck();
					strncpy(uart_cmd,"aaaaaaaaaACK",12);
send(12,1,2);

					}
					else//100
						{
					mtype=cUL;
						// UartPrint("mtype=cUL\n");
						strncpy(uart_cmd,"aaaaaaaaaACK",12);
send(12,1,3);
							//send(11,1,3);
					}
					
				}		
			}
			else  //0
			{
			
				b=handle<<(1);
				c=b>>(7);
				if(c)//01
				{
				
					b=handle<<(2);
					c=b>>(7);
					if(c)//011
					{
						mtype=uDL;
					
						 //UartPrint("mtype=uDL\n");
						ACKbitcheck();
					}
					else//010 unconfirmed macpayload
					{
						mtype=uUL;
				
						// UartPrint("mtype=uUL\n");
					
					}
				}
				else//00 
				{
		
					
					b=handle<<(2);
					c=b>>(7);
					if(c)//001 joint accept
					{
						mtype=Ja;
					
					// UartPrint("mtype=Ja\n");
					}
					else//000 joint request
					{
						mtype=Jr;
				
						// UartPrint("mtype=Jr\n");
					}			
				}
			}	
}
void receive_data_handle(void)
{
	uint8_t i;
	uint32_t temp=0;
uint8_t t,t1,t2,t3,t4;
if(BufferSize>5 && Buffer[0] != '\0')
	
                {		
									
 mtype_check();
switch(mtype){									
	case(uUL):
case(uDL):
case(cUL):
case(cDL):	
		
	t=Buffer[5]<< 4;
	t1=t>>7;
	t=Buffer[5]<< 5;
	t2=t>>7;
	t=Buffer[5]<< 6;
	t3=t>>7;
	t=Buffer[5]<< 7;
	t4=t>>7;
	             FOptsLen= t1 | t2 | t3 |t4;
	if(FOptsLen== 0) // MAC payload
	{
								UartPrint("!");
	              UartPrint("%d",BufferSize);
							  UartPrint("!");
	            for(i=0;i<=8;i++)
                 {
               UartPrint("%d ",Buffer[i]);
								 }								 
								//UartPrint("send \n");
								for(i=9;i<BufferSize;i++) UartPrint("%c",Buffer[i]);
								 	
								 	
							 }
	else //MAC command
{
	UartPrint("MAC command\n");
	//UartPrint("!");
	        //      UartPrint("%d",BufferSize);
					//		  UartPrint("!");
	            for(i=0;i<=8;i++)
                 {
               UartPrint("%d ",Buffer[i]);
								 }
								 CID=Buffer[8];
								 switch(CID)
								 {
									 case 2:
										 
										 //MACHeader[9]:  [7:4]datarate  [3:0]txpower
									 datarate=0;
									 txpower=0;
									 UartPrint("Buffer[9]:%d \n",Buffer[9]);
									 for(i=0;i<=7;i++)
{
	temp8bit=Buffer[9]<<i;
	temp8bit2= temp8bit>>7;
	UartPrint("%u ",temp8bit2);
 } 
									datarate=Buffer[9]>>4;
									 //datarate|=Buffer[9]>>4;
 
									 temp8bit=Buffer[9]<<4;
 
									 txpower=temp8bit>>4;
                UartPrint(" datarate=%d  txpower=%d \n",datarate,txpower);
									 LinkADRReq(0,datarate,txpower,0, NbTrans, 0);
									 
										 
									 default:
										 break;
								 }
								 
	
	
	
	
}
								 //to cut payload' '!XX!
								  UartPrint(" ");				 
                // if(flag_RSSISNR==1)
								// {
								 UartPrint("RSSI:%4d SNR:%4d ",RssiValue,SnrValue);	
							//	 }									 
UartPrint("\n");
								memset(Buffer,'0x00',sizeof(Buffer));
                Buffer[0] = '\0';			
								// UartPrint(" ");	
	//UartPrint("\n");								 
							 break;

	case(Jr):
		#ifdef GateWayOnly
	if(GW==1)
	{
		UartPrint("joinrequest\n");
	joinrequest();
		memset(Buffer,'0x00',sizeof(Buffer));
                Buffer[0] = '\0';
	}
	#else
		UartPrint("!");
	              UartPrint("%d",BufferSize);
							  UartPrint("!");
								//0118
                 for(i=0;i<=18;i++)
                 {
               UartPrint("%d ",Buffer[i]);
								 }									 
								  UartPrint(" ");
								 
								 
              // if(flag_RSSISNR==1)
						//	 {
								 UartPrint("RSSI:%4d SNR:%4d ",RssiValue,SnrValue);
							// }				
								 UartPrint("\n");
								memset(Buffer,'0x00',sizeof(Buffer));
                Buffer[0] = '\0';	
							 #endif
								 break;
	case(Ja):
		if(state_connect==0)
		{
		//	UartPrint("in  if(state_connect==0) \n");
t1=Buffer[7];
t2=Buffer[8];
t3=Buffer[9];
t4=Buffer[10];
	temp = (  (uint32_t)t1 <<24) | ((uint32_t)t2 << 16)| ((uint32_t)t3 << 8) | (uint32_t)t4 ;		
DevAddr=temp;
				UartPrint("DevAddr=%d \n",DevAddr);
			FCnt=1;
MACPayloadgenerate();		
state_connect=1;	
		memset(Buffer,'0x00',sizeof(Buffer));
                Buffer[0] = '\0';
		}
		break;
		default:		
			break;

                }	
							}
}
