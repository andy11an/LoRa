#include <string.h>
#include <stdbool.h>
#include "board.h"
#include "radio.h"

/*!
 * test mode
 */
#define TestMode 1


/*!
 * no network server, handle join procedure un gateway 
 */
//#define GateWayOnly 1

/*!
 * mode select 
 */
#define EndDevice 1
#define GateWay 1




//#define RF_FREQUENCY                                1000000000 //政翰 Hz (SX1276: 862-1020MHz; SX1278: 137-525MHz).
//#define RF_FREQUENCY                                888000000 //張逸Hz (SX1276: 862-1020MHz; SX1278: 137-525MHz).
//#define RF_FREQUENCY                                897000000 //收資料用
#define RF_FREQUENCY                                917000000 //改code時用
//#define RF_FREQUENCY                                850000000 //家祥 Hz (SX1276: 862-1020MHz; SX1278: 137-525MHz).
//#define RF_FREQUENCY                                1010000000 //展胤Hz (SX1276: 862-1020MHz; SX1278: 137-525MHz).
#define TX_OUTPUT_POWER                             14        // dBm
#define BUFFER_SIZE                                 59 // Define the payload size here
#define RX_TIMEOUT_VALUE                            20000

//in this version , using USE_MODEM_LORA
#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12      // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif





/*!
 * join procedure enable 
 */
extern bool flag_joinprocedure;


/*!
 * end device mode 
 */
#ifdef EndDevice
extern uint8_t state_connect;
#endif

extern uint8_t uart_cmd[UART_LENGTH];
//uint16_t BufferSize = BUFFER_SIZE; // 原本
extern uint16_t BufferSize;
extern uint8_t Buffer[BUFFER_SIZE];
extern uint8_t SBuffer[BUFFER_SIZE];


/*!
 * lora message format elements definition
 */

extern uint32_t MIC;
extern uint8_t FOptsLen ;
extern uint32_t DevNonce;
extern uint16_t CFList;
extern uint16_t state_Class;   //classA: 1   classB: 2   class C: 3  
extern uint32_t AppNonce;
extern uint32_t NetID;   //size: 3bytes
extern uint8_t DLSettings;
extern uint8_t  RXDelay;

extern uint16_t  FCtrl;
extern uint32_t DevAddr;
extern uint64_t DevEUI;

extern uint64_t AppEUI;
extern uint16_t FCnt;

/*!
* confirmed UL/DL handle
* size_resend: the size of the confirmed package
*/
extern bool ACK;
extern uint8_t NbTrans;
extern uint8_t flag_NbTrans;
extern uint8_t size_resend;


/*!
* MAC command's setting
*/
extern uint8_t datarate;
extern uint8_t txpower;
//extern uint8_t LoRabandwith;

/*!
* Class B
*/
extern uint16_t TCnt;
extern bool ClassB;
/*!
* Beacon   00  0000 6bytes
*/
extern  uint8_t  Beacon[6];

/*!
* RX widows controll
*/
extern bool WaitRX ;
extern bool WaitRX2 ;



/*
*=====================*
*       Big endian    *
*=====================*
*/
/*!
* PHYPayload      MHDR 1bytes
                 +MACPayload
								 /JoinRequest: 8bytes APPEUI + 8bytes DevEUI + 2bytes DevNonce
 								 /JoinResponse 
								 + MIC
 */

/*
MACPayload       MHDR 1bytes
                 +MACPayload  1..M bytes
								 DR0/DR1/DR2:59    DR3:123 DR4: 230  DR5/DR6/DR7:230  DR8:15 RFU
								 +MIC 4bytes


*/
extern  uint8_t  MACHeader[20];
/*
JoinRequest       MHDR 1bytes
                 +AppEUI 8bytes
								 +DevEUI 8bytes
								 +DevNonce 2bytes
                 +MIC 4bytes
*/
 #ifdef EndDevice
extern  uint8_t  JoinRequest[19];

/*
JoinAccept       MHDR 1bytes
                 +AppNonce 3bytes
								 +NetID 3bytes
								 +DevAddr 4bytes
								 +DLSeyyings 1bytes
								 +RxDelay 1bytes
								 +CFlist (16)Optional
                 +MIC 4bytes
*/
#endif
//#elif defined (NetworkServer)

 #ifdef  GateWay
extern  uint8_t  Joinaccept[33];
#endif



/*!
* lora header construct funtion: end device
 */
 
 #ifdef EndDevice
 void  JoinRequestgenerate();
 #endif
 
 /*!
* lora header construct funtion: GateWay
 */
 
 #ifdef GateWay
 void  Joinacceptgenerate();
 #endif

/*!
* lora header construct funtion: all device
 */
void  MACPayloadgenerate(void);
void  FCntAdd(void);
 

/*!
* Class B
*/
void Timer(uint32_t timeout);
void EnableClassB();
void UnableClassB();
void RXforTimer(void);



















