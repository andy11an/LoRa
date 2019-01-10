#include<stdio.h>
#include<stdint.h>
#include<stdarg.h>

#include "data.h"



extern int p;
extern int sf;
extern int cr;

//prr threshold
extern  double Tau;

extern int snr[4];
extern int rssi[4];
extern double prr[4];
extern double weight[4];


extern int datar[4][30];
extern int datas[4][30];
//extern int prr[4];

//number of receive packet og each gateway
extern int pnumber[4];
//mideian of rssi
extern int midrssi[4];
//minof rssi
extern int minrssi[4];
//mideian of snr
extern int midsnr[4];

//average  
extern int avr[4];
extern int avs[4]; 
 
extern int Sigmar[4];
extern int Sigmas[4];
//r
extern int r[4];
//s
extern int s[4];
//m
extern int m[4];
//feature do each round

extern int Asnr;
extern int Arssi;
extern int Amrssi;

extern float DemodulatorSNR[6];

extern int np;
extern int nsf;
extern int ncr;

//stop and  testing the optimal setting
extern int mp;
extern int msf;
extern int mcr;

// flag that control the ADR testing
extern bool StopFlag1;
extern bool StopFlag2;


// fortesting the optimal setting
extern int pp;
extern int psf;
extern int pcr;

/*
* \brief read the file and get the data
* \param [IN] n  input file number
* \
*/
void InputData(int n);


/*
* \brief sort the arry
* \param [IN] n 
*/
void Sort(int n);


/*
* \brief find the weight
* \param [IN] n number of GW
* \param [IN] t prr rate thteshold
*/
void Weight(double t,int n);

/*
* \brief find the median and min
* \param [IN] n number of GW
*/
void Median(int n);

/*
* \brief find the sigma
* \param [IN] n number of GW
*/
void sigma(int n);

/*
* \brief find thersm of each gw
* \param [IN] n number of GW
*/
void rsm(int n);




/*
* \brief  get the s r and m of round
* \param [IN]  n number of GW
*/
void Adjustment(int n);

/*
* \brief  increase delta p
*/
void Poweradj();


/*
* \brief  increase delta p
*/
void FindMinSF();



/*
* \brief  check rs < r (nSF generate)
*/
void ChecknSF();

/*
* \brief  decrease the power (np generate)
*/
void DecreaseP();


/*
* \brief   (ncr generate)
*/
void FindCR();


/*
* \brief jump to the next setting
*/
void Next(int n);



/*
* \brief power consumption
*/
void PC();



/*
* \brief  Probing
*/
void Probing();











