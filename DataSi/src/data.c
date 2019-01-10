#include "data.h"
#include<math.h>
/*
 struct setting_t
   {
   uint8_t	range;
   uint8_t	brange;
   uint8_t	bchance;
   uint16_t  distance;	
   };
*/
struct setting_t Setting[4];
uint8_t CRchance[4][3]={
	1,5,24,
	1,6,27,
	1,6,32,
	1,7,35
};


//7 8 ..  BW125
int16_t Sensitivity[6]={
	-123,-126,-129,-132,-134,-137
};


FILE*	GW[4];


int RSSIBasedGenerate(uint16_t distance)
{
int a;
//printf("dis=%d\n",distance);
a=-55- (distance/9);
//printf("base=%d\n",a);
// return -120- (int8_t)floor(distance/10);
	return -55- (distance/10);
}

int InterferenceRange(uint8_t range,int inter)
{
	return inter% (int)range;

}
int BInterferenceRange(uint8_t range ,uint8_t chance,int ch,int inter)
{
int a;
a=(ch%10)<=chance ?  inter%range : 0;

//printf("Binter=%d\n",a);
	return  a ;	
}

int RSSIPower(uint8_t power)
{
int a;
a=14-(int)power;
//printf("power=%d\n",a);
	return a;	
}
int SNRGenerate(uint8_t range,int RSSI,int noice, uint16_t distance)
{
int a;
if(RSSI < -120)
{
a = RSSI +100 - noice%range - distance/100;
}
else
{
a = RSSI +110 - noice%range - distance/100;
}
	return a;

}



bool  Packetreceive(int16_t R,uint8_t SF,uint8_t C,int ch)
{
	if(R < (Sensitivity[SF-7]-3))
	{
		return 0;
	}
	else if(R >= (Sensitivity[SF-7]))
	{
		return 1;
	}
	else
	{
		return  (ch%100) < CRchance[C-1][Sensitivity[SF-7]-R-1] ? 0 : 1;
	}
}
extern int16_t Sensitivity[6];








