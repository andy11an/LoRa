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



typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

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

#ifdef GateWayOnly
struct LoRa_Table
{ 
	uint64_t DevEUI;
uint64_t AppNonce;
	uint32_t DevAddr;
	uint16_t FCnt;
	uint16_t JCnt;
	bool connect;
};
struct LoRa_Table  table[100];
uint8_t offset_table=0;
void joinrequest();
void showtable(void);
#endif

States_t State = LOWPOWER;

int16_t RssiValue = 0;
int8_t SnrValue = 0;

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
 * \brief check LoRa header MHDR  [7..5]mtype
 */
void mtype_check(void);

void receive_data_handle(void);
void autosend(void);
void ACKbitcheck(void);

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


/*!
*  using USE_MODEM_LORA
*/
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
	
																												
    while( 1 )
    {
			//command			 
        uart_read = UartScan(uart_cmd);
        if ( uart_read != 0 ) 
				{
            UartPrint("\n>> %s\n", uart_cmd);	
				   	commandcheck(uart_read);
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
        flag_NbTrans++;
				//send 	NbTrans times		 
        if( flag_NbTrans == NbTrans)
				{
				ACK=0;
				flag_NbTrans=0;	
				}
					
				}

				//ED: join procedure
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
								 
								Radio.Rx( 1000 );
								 Radio.Rx( 2000 );
							//	 DelayMs( 2000 );								
            // Radio.Rx( RX_TIMEOUT_VALUE );
				}
			}
				
			
			#ifdef GateWayOnly
				if(flag_showtable==1)
				{
				showtable();
					flag_showtable=0;
				}
				#endif
        switch( State )
        {
        case RX:  
					receive_data_handle();	
				// ED autosend for test?

								 if(flag_autosend==1){
								autosend();		
								 }		
								 
							 

                Radio.Rx( RX_TIMEOUT_VALUE );
							//	 DelayMs( 2000 );								
             Radio.Rx( RX_TIMEOUT_VALUE );
             State =RX;
								 	if(GW==1)
					{
            State = RX;
					} 
					else
					{
				State =RX_ERROR;
					}
						  //State =RX_ERROR;
            break;
        case TX:
            // Indicates on a LED that we have sent a PING [Master]
            // Indicates on a LED that we have sent a PONG [Slave]
#if LED_ENABLE
            GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
#endif
            Radio.Rx( RX_TIMEOUT_VALUE );
            //State = LOWPOWER;
				
			    	if(GW==1)
					{
            State = RX;
					} 
					else
					{
					State = LOWPOWER;
					}
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
					if(GW==1)
					{
            State = RX;
					}
					else
					{
					State = LOWPOWER;
					}
            break;
        case TX_TIMEOUT:
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
					   if(GW==1)
					{
            State = RX;
					}
					else
					{
					State = LOWPOWER;
					}
        default:
            // Set low power
            break;
        }
    
        TimerLowPowerHandler( );
#if ENABLE_I2C
				I2C_CpltPolling(0x80);
#endif
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
					}
					else//100
						{
					mtype=cUL;
						// UartPrint("mtype=cUL\n");
						uint32_t a[2];
							send(11,1,3);
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
}
								 //to cut payload' '!XX!
								  UartPrint(" ");				 
                 if(flag_RSSISNR)
								 {
								 UartPrint("RSSI:%4d SNR:%4d ",RssiValue,SnrValue);	
								 }									 
								memset(Buffer,'0x00',sizeof(Buffer));
                Buffer[0] = '\0';								
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
								 
								 
               if(flag_RSSISNR==1)
							 {
								 UartPrint("RSSI:%4d SNR:%4d ",RssiValue,SnrValue);
							 }								 
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

void ACKbitcheck(void)
{
uint8_t t,t1;
	t=Buffer[5]<< 2;
	t1=t>>7;
	if(t1==1)
	{
	ACK = 0;
	}	
}

#ifdef GateWayOnly
void joinrequest()
{
	uint8_t i;
	uint16_t temp;
	uint64_t DevEUI;
	uint8_t t1,t2,t3,t4,t5,t6,t7,t8;
	uint8_t j,handle_offset_table,a=0;
	handle_offset_table=0;
	uint16_t c,temp2=0;


	t1=Buffer[9];
	t2=Buffer[10];
	t3=Buffer[11];
	t4=Buffer[12];
	t5=Buffer[13];
	t6=Buffer[14];
	t7=Buffer[15];
	t8=Buffer[16];
	DevEUI= (uint64_t)t1 << 56 | (uint64_t)t2 << 48 |(uint64_t)t3 << 40 |(uint64_t)t4 << 32 |(uint64_t)t5 << 24 |(uint64_t)t6 << 16 |(uint64_t)t7 << 8 |(uint64_t)t8;
	UartPrint("DevEUI= %d \n",DevEUI);
	UartPrint("offset_table=%d\n",offset_table);
	
	for(i=1;i<=offset_table;i++)
	{
		if(DevEUI == table[i].DevEUI)
		{
		handle_offset_table=i;	
		i=offset_table+1;
		}
			
	}
	//no find in table
	if(handle_offset_table==0)
	{
		offset_table++;
		table[offset_table].DevEUI=DevEUI;
		table[offset_table].DevAddr=offset_table;
		table[offset_table].FCnt=0;
		table[offset_table].JCnt=0;
		table[offset_table].connect=0;	
		Jasend(table[offset_table].DevAddr);	
	}	
	else
	{
	Jasend(table[handle_offset_table].DevAddr);	
	}
}

void showtable(void)
{
uint8_t i;
for(i=1;i<=offset_table;i++)
{
	UartPrint("DevAddr:%4d    DevEUI:%10d FCnt:%5d\n");	
}
}
#endif 


		





