#include "loraheader.h"
/*
*Big ending
*/

/*!
 * end device mode
 */
#ifdef EndDevice
uint8_t state_connect = 0; //for join procedure
#endif


uint8_t uart_cmd[UART_LENGTH];
//uint16_t BufferSize = BUFFER_SIZE; // нье╗
uint16_t BufferSize;
uint8_t Buffer[BUFFER_SIZE];
uint8_t SBuffer[BUFFER_SIZE];
uint32_t MIC;
uint8_t FOptsLen = 0;
uint32_t DevNonce = 0;
uint16_t CFList;
uint16_t state_Class;   //classA: 1   classB: 2   class C: 3
uint32_t AppNonce = 1;
uint32_t NetID;   //size: 3bytes
uint8_t DLSettings;
uint8_t  RXDelay;
uint16_t  FCtrl = 0;
uint32_t DevAddr = 53;

uint64_t DevEUI = 456;
uint64_t AppEUI = 0;
uint16_t FCnt = 1;
bool ACK = 0;
uint8_t NbTrans = 10;
uint8_t flag_NbTrans = 0;
uint8_t size_resend;
uint8_t datarate = LORA_SPREADING_FACTOR ;
uint8_t txpower = TX_OUTPUT_POWER ;
//uint8_t LoRabandwith =LORA_BANDWIDTH ;
bool flag_joinprocedure = 1;
//bool flag_joinprocedure=0;

uint16_t TCnt = 0;
bool ClassB = 0;
/*!
* Beacon   00  0000 6bytes
*/
uint8_t  Beacon[6];



/*!
* RX widows controll
*/
bool WaitRX = 0;
bool WaitRX2 = 0;

/*!
 *  PingSlot timers
 */
TimerEvent_t PingSlot;
/*!
*  PingSlot timers
*/
TimerEvent_t RX;
/*!
*  PingSlot timers
*/
TimerEvent_t RX2;




uint8_t  MACHeader[20];
#ifdef EndDevice
uint8_t  JoinRequest[19];
#endif

#ifdef GateWay
uint8_t  Joinaccept[33];
#endif


/*!
* lora header construct funtion: end device
 */
#ifdef EndDevice
void  JoinRequestgenerate()
{
    /*
    DevNonce=Radio.Random();
    AppNonce=Radio.Random();
    */
    uint8_t temp = 0;
    memset(JoinRequest, 0x00, sizeof(JoinRequest));
//MHDR 0000 0000
//for buffer[0]!=0   MHDR 0000 0100
    JoinRequest[0] &= ~(1 << 7);
    JoinRequest[0] &= ~(1 << 6);
    JoinRequest[0] &= ~(1 << 5);
    JoinRequest[0] &= ~(1 << 4);

    JoinRequest[0] &= ~(1 << 3);
    JoinRequest[0] |=  (1 << 2);
    JoinRequest[0] &= ~(1 << 1);
    JoinRequest[0] &= ~(1 << 0);
// AppEUI 8byte  0
    JoinRequest[1] = (uint64_t)temp  | (AppEUI >> 56);
    JoinRequest[2] = ((uint64_t)temp) | (AppEUI << 8) >> 56;
    JoinRequest[3] = ((uint64_t)temp) | (AppEUI << 16) >> 56;
    JoinRequest[4] = ((uint64_t)temp) | (AppEUI << 24) >> 56;
    JoinRequest[5] = ((uint64_t)temp) | (AppEUI << 32) >> 56;
    JoinRequest[6] = ((uint64_t)temp) | (AppEUI << 40) >> 56;
    JoinRequest[7] = ((uint64_t)temp) | (AppEUI << 48) >> 56;
    JoinRequest[8] = ((uint64_t)temp) | (AppEUI << 56) >> 56;
// DevEUI 8byte   1
    JoinRequest[9] = (uint64_t)temp | (DevEUI >> 56);
    JoinRequest[10] = ((uint64_t)temp) | (DevEUI << 8) >> 56;
    JoinRequest[11] = ((uint64_t)temp) | (DevEUI << 16) >> 56;
    JoinRequest[12] = ((uint64_t)temp) | (DevEUI << 24) >> 56;
    JoinRequest[13] = ((uint64_t)temp) | (DevEUI << 32) >> 56;
    JoinRequest[14] = ((uint64_t)temp) | (DevEUI << 40) >> 56;
    JoinRequest[15] = ((uint64_t)temp) | (DevEUI << 48) >> 56;
    JoinRequest[16] = ((uint64_t)temp) | (DevEUI << 56) >> 56;
//DevNonce 0
    /*
    JoinRequest[17]=((uint16_t)temp) | (DevNonce>>8);
    JoinRequest[18]=((uint16_t)temp)|(DevNonce<<8)>>8;
    */
//FCnt
    JoinRequest[17] = ((uint16_t)temp) | (FCnt >> 8);
    JoinRequest[18] = ((uint16_t)temp) | (FCnt << 8) >> 8;



}

#endif

#ifdef GateWay

void  Joinacceptgenerate()
{
    uint8_t temp = 0;
    memset(Joinaccept, 0x00, sizeof(Joinaccept));
//MHDR 0010 0000

    Joinaccept[0] &= ~(1 << 7);
    Joinaccept[0] &= ~(1 << 6);
    Joinaccept[0] |=  (1 << 5);
    Joinaccept[0] &= ~(1 << 4);

    Joinaccept[0] &= ~(1 << 3);
    Joinaccept[0] &= ~(1 << 2);
    Joinaccept[0] &= ~(1 << 1);
    Joinaccept[0] &= ~(1 << 0);
//AppNonce 3bytes
    Joinaccept[1] = ((uint32_t)temp) | (AppNonce >> 16);
    Joinaccept[2] = ((uint32_t)temp) | ((AppNonce << 8) >> 16);
    Joinaccept[3] = ((uint32_t)temp) | ((AppNonce << 16) >> 16);


// NetID 3bytes
    Joinaccept[4] = ((uint32_t)temp) | (NetID >> 16);
    Joinaccept[5] = ((uint32_t)temp) | ((NetID << 8) >> 16);
    Joinaccept[6] = ((uint32_t)temp) | ((NetID << 16) >> 16);
//DevAddr 4bytes
    Joinaccept[7] = (uint32_t)temp | (DevAddr >> 24);
    Joinaccept[8] = ((uint32_t)temp) | (DevAddr << 8) >> 24;
    Joinaccept[9] = ((uint32_t)temp) | (DevAddr << 16) >> 24;
    Joinaccept[10] = ((uint32_t)temp) | (DevAddr << 24) >> 24;
//DLSettings
    Joinaccept[11] = 0;
//RxDelay
    Joinaccept[12] = 0;

}

#endif


/*!
* lora header construct funtion: all device
 */
/*
MACPayload       MHDR 1bytes
                 +MACPayload  1..M bytes
								 DR0/DR1/DR2:59    DR3:123 DR4: 230  DR5/DR6/DR7:230  DR8:15 RFU
                 SF12 DR0   SF11 DR1  SF10 DR2 SF9 DR3
								 +MIC 4bytes


MACPayload[0]: MHDR
MACPayload[1~4]: DevAddr
MACPayload[5]: FCtrl
MACPayload[6~7]: FCnt
*/
/*
*010 unconfirmed UL  : default
*011 unconfirmed DL
*100 confirmed  UL
*101 confirmed  DL
*/


void  MACPayloadgenerate()
{
    uint8_t temp = 0;
    memset(MACHeader, 0x00, sizeof(MACHeader));
//MHDR 0100 0000
    MACHeader[0] &= ~(1 << 7);
    MACHeader[0] |=  (1 << 6);
    MACHeader[0] &= ~(1 << 5);
    MACHeader[0] &= ~(1 << 4);

    MACHeader[0] &= ~(1 << 3);
    MACHeader[0] &= ~(1 << 2);
    MACHeader[0] &= ~(1 << 1);
    MACHeader[0] &= ~(1 << 0);
//DevAddr
    MACHeader[1] = (uint32_t)temp | (DevAddr >> 24);
    MACHeader[2] = ((uint32_t)temp) | (DevAddr << 8) >> 24;
    MACHeader[3] = ((uint32_t)temp) | (DevAddr << 16) >> 24;
    MACHeader[4] = ((uint32_t)temp) | (DevAddr << 24) >> 24;
//FCTRL
//DL: ADR RFU ACK FPending FOptsLen[3..0]
//UL: ADR ADRACKReq ACK ClassB FOptsLen[3..0]
    MACHeader[5] = 0;
//FCnt
    MACHeader[6] = ((uint16_t)temp) | (FCnt >> 8);
    MACHeader[7] = ((uint16_t)temp) | (FCnt << 8) >> 8;
//Fopts    0..15
//FPort payload's length
    MACHeader[8] = 0;
}

void  FCntAdd()
{
    uint8_t temp = 0;
    FCnt++;
    MACHeader[6] = ((uint16_t)temp) | (FCnt >> 8);
    MACHeader[7] = ((uint16_t)temp) | (FCnt << 8) >> 8;
}


/*
void ClassBtimer()
{
DelayMs (TCnt);
Radio.Rx( 5000 );

}
*/

void Timer(uint32_t timeout)
{
    TimerInit( &PingSlot, RXforTimer );
    if( timeout != 0 )
    {
        TimerSetValue( &PingSlot, timeout );
        TimerStart( &PingSlot );
    }
}

void RXforTimer(void)
{
    Radio.Rx( RX_TIMEOUT_VALUE );
}

void EnableClassB()
{
    ClassB = 1;
    MACHeader[5] |=  (1 << 4);
}



void UnableClassB()
{
    ClassB = 0;
    MACHeader[5] &=  ~(1 << 4);
}
















































