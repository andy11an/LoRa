#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<math.h>

extern FILE* GW[4];

struct setting_t
{
	int8_t RSSIEnviroment;
	uint8_t	range;
	uint8_t	brange;
	uint8_t	bchance;
	uint16_t  distance;		
};

//extern  struct setting;
extern  struct setting_t Setting[4];
extern uint8_t CRchance[4][3];
extern int16_t Sensitivity[6];

//RSSI based
/*
 * \brief  RSSI based, 0m -55dBm: 10m =1 (dBm)
 * \param [IN] d  
 */
int RSSIBasedGenerate(uint16_t distance);



//adding layer one
/*
 * \brief add inter(RSSI) on the RSSI based
 * \remark enviroment
 * \param [IN] inter 0~20
 */


//adding layer two
/*
 * \brief range of inter(RSSI) on the RSSI based
 * \param [IN] range 0~9
 * 
 */
int InterferenceRange(uint8_t range,int inter);



//adding layer three
/*
 * \brief  add inter(RSSI)(burst interderence) on the RSSI based+inter
 *\param [IN] binter 0~9
 *\param [IN] c  0~9 chance of  burst interderence happened
 */
int BInterferenceRange(uint8_t range ,uint8_t chance,int ch,int inter);


//TxPower
/*
 * \brief  power 
 * \remark power start at 14dBm
 *\param [IN] current power
 */
int RSSIPower(uint8_t power);

//SNR
/*
 * \brief  SNR
 * \remark RSSI-n,  when snr<0 rssi+=snr*0.25
 *\param [IN] range
 */
int SNRGenerate(uint8_t range,int RSSI,int noice, uint16_t distance);





/*
 *  \brief determine if packet receive or not 
 *  \remark  receive close to threshold has chance  be error
 *  \param [IN] 
 */
bool  Packetreceive(int16_t R,uint8_t SF,uint8_t C,int ch);


#define CR1_1  1
#define CR1_2  5
#define CR1_3  24



#define CR2_1  1
#define CR2_2  6
#define CR2_3  27



#define CR3_1  1
#define CR3_2  6
#define CR3_3  32



#define CR4_1  1
#define CR4_2  7
#define CR4_3  35















