#include  "ADR.h"
#include <stdlib.h>
#include <math.h>

int p;
int sf;
int cr;

double Tau;

int snr[4];
int rssi[4];
double prr[4];
double weight[4];


//handle one round
int datar[4][30];
int datas[4][30];
//int prr[4];

int pnumber[4];
int midrssi[4];
int minrssi[4];
int midsnr[4];

//average  
int avr[4];
int avs[4]; 

int Sigmar[4];
int Sigmas[4];


int r[4];
int s[4];
int m[4];

int Asnr;
int Arssi;
int Amrssi;

float DemodulatorSNR[6]=
{
	-7.5,-10,-12.5,-15,-17.5,-20
};

int PowerI[8]=
{
	24,24,25,25,30,34,44,85
};

int np;
int nsf;
int ncr;

int mp;
int msf;
int mcr;

bool StopFlag1=1;
bool StopFlag2=0;

 int pp;
 int psf;
int pcr;


void InputData(int n)
{
	int i;
	int j;
	int m;	
	char a[8];
	char dbuffer[50];	
	for(i=0;i<n;i++)
	{
		int totalr=0;
		int totals=0;
		int lost=0;
		int k=0;
		fseek(GW[i],6,SEEK_CUR);
		for(j=1;j<=30;j++)
		{

fgets(dbuffer,50,GW[i]);
			//fix RSSI -200 SNR -100 =lost
			sscanf(dbuffer,"%d %s %d %s %d %s %d %s %d",&m,a,&m,a,&m,a,&datar[i][k],a,&datas[i][k]);
if(i==0 &&j==1)
printf("fir:%s",dbuffer);
if(i==0 &&j==2)
printf("sec:%s",dbuffer);

			printf(" %d ",datar[i][k]);		
			if(datar[i][k]!= -200)
			{
				totalr +=datar[i][k];
				totals +=datas[i][k];
				k++;

			}
			else
			{
				lost++;
			}

		}	
		pnumber[i]=30-lost;
		if(pnumber[i]!=0)
		{
			avr[i]=(int) totalr/pnumber[i];
			avs[i]= (int)totals/pnumber[i];
		}
		else
		{
			avr[i]=-200;
			avs[i]=-99;
			pnumber[i]=1;
		}		
		printf("\npnumber[%d]=%d     avr=%d      avs=%d     ",i, pnumber[i],avr[i],avs[i]);
		prr[i]=(double)pnumber[i]/(double)30;	
		printf(" prr= %lf\n",prr[i]);
	}
}

int compare (const void *a,const void *b)
{
	int c= *(int*)a;
	int d= *(int*)b;
	if(c<d)
	{
		return -1;
	}
	else
	{
		return  1;
	}
}

void Sort(int n)
{
	int i;
	int j;
	printf("sorting start\n");
	for(j=0;j< n;j++){
		qsort((void*)datar[j],pnumber[j],sizeof(datar[j][0]),compare);
		qsort((void*)datas[j],pnumber[j],sizeof(datas[j][0]),compare);	
	}
}
void Weight(double t,int n)
{
	double total=0;
	double total2=0;
	int i;
	int ignore=0;
	if(n==1)
	{
		weight[0]=1;
	}
	else
	{
		//prr after normalize
		for(i=0;i<n;i++)
		{
			if(prr[i]>=t)
			{
				total+= (int)pow((prr[i]-t),2);
			}
			else
			{
				ignore++;
				printf("ignore = %d\n",ignore);
			}
		}
		if((n-1-ignore)>0)
		{
			total = sqrt(total/(n-1-ignore));
		}
		else
		{
			total = sqrt(total/(n-1));
		}
		for(i=0;i<4;i++)
		{
			if(prr[i]>t)
			{
				total2+=  (prr[i]-t)/total ;
			}
			else
			{
				prr[i]=t;
			}
		}
		for(i=0;i<n;i++)
		{
			weight[i]=   ((prr[i]-t)/total)/total2;
			if(weight[i]==0)
			{
				weight[i]=0.000001;
			}
			printf("weight[%d]= %lf  ",i,weight[i]);		
		}

		printf("\n");	
	}
}

void Median(int n)
{
	int i;
	for(i=0;i<n;i++)
	{
		if(pnumber[i]%2 ==1)
		{
			midrssi[i]=(datar[i][pnumber[i]/2]+datar[i][(pnumber[i]/2 )+1])/2;
			midsnr[i]=(datas[i][pnumber[i]/2]+datas[i][(pnumber[i]/2 )+1])/2;
		}
		else
		{
			midrssi[i]=(datar[i][pnumber[i]/2]);
			midsnr[i]=(datas[i][pnumber[i]/2]);	
		}
		minrssi[i]=datar[i][0];
		printf("minrssi[%d]=%d\n",i,minrssi[i]);
	}
}

void sigma(int n)
{
	int i ,j;
	double standardr =0;
	double sigmar=0 ;

	double standards =0;
	double sigmas=0;
	for(i=0;i<n;i++)
	{
		for(j=0;j<pnumber[i];j++)
		{
			standardr =datar[i][j] - avr[i];
			standards =datas[i][j] - avs[i];

			standardr*= standardr;
			standards*= standards;

			sigmar += standardr;
			sigmas += standards;
		}
		sigmar /= pnumber[i];
		sigmas /= pnumber[i];

		Sigmar[i] = sqrt(sigmar);
		Sigmas[i] = sqrt(sigmas);
	}
}


void rsm(int n)
{
	int i;
	for (i=0;i<n;i++)
	{
		r[i] =   	midrssi[i] - 2*Sigmar[i];
		s[i] =   	midsnr[i] - 2*Sigmas[i];
		m[i] =      	minrssi[i];
		printf("r[%d]=%d  s[%d]=%d  m[%d]=%d\n",i,r[i],i,s[i],i,m[i]);	
	}
}


void Adjustment(int n)
{
	int i;
	Arssi =0;
	Asnr=0;
	Amrssi=0;
	for(i=0;i<n;i++)
	{
		Arssi	+=	r[i] * weight[i];
		Asnr	+= 	s[i] * weight[i];
		Amrssi	+=	m[i] * weight[i];

	}
	printf("Arssi=%d Asnr=%d Amrssi=%d\n",Arssi,Asnr,Amrssi);
}

void Poweradj()
{

	int a;
	a=14-p;
	Arssi	+=	a;
	Asnr	+= 	a;
	Amrssi	+=	a;

}

void FindMinSF()
{
	int i=5;
	while(( ((float)Asnr/4 -  DemodulatorSNR[i]) >= 0) && (i>0)   )
	{
		i--;
	}
	nsf= i+7;
	printf("nsf= %d\n",nsf);
}

void ChecknSF()
{
	while(  Arssi   <= Sensitivity[nsf-7]  )
	{

		if(nsf==12)
		{
			break;
		}

		nsf++;
	}	
	printf("after check nsf= %d\nrs=%d\n",nsf,Sensitivity[nsf-7]);
}


void DecreaseP(int n)
{
	int temp=14;	
	//np=14;
	while(   ((Arssi   -  Sensitivity[nsf-7]) -(14-temp) ) >= 0 )
	{
		if(temp<=2)
		{
			break;
		}
		else
		{
			temp-=2;	
		}

	}
	//check prr and sf  p
	int i;
	double max=0.01;
	for(i=0;i<4;i++)
	{
		if(max < prr[i])
		{
			max =prr[i];
		}
	}
	printf("max: %lf temp=%d\n",max,temp);
	//max >= Tau
	//setting record  
	if(max >=Tau)
	{
printf(" setting record : min: %d %d %d curr: %d %d %d \n",msf,mp,mcr,sf,p,cr);
		if ((msf >= sf) )
		{
			//min
			double I;
			//current
			double I2;
			I= ( 12.25 + 8 + ceil( (8*32 - 4*msf + 28+16) /  (4*(msf ))  ) *(mcr+4)) * ( pow(2.0,msf) / 125) *PowerI[(mp)/2-1];
			I2= ( 12.25 + 8 + ceil( (8*32 - 4*sf + 28+16) /  (4*(sf ))  ) *(cr+4)) * ( pow(2.0,sf) / 125) *PowerI[(p)/2-1];
printf("I=%lf I2=%lf\n",I,I2);		
	if((I2 < I) || (msf >=sf)) 
			{
printf("pass\n");
				msf=sf;
				mp=p;
				mcr=cr;
printf("msf=%d mp=%d mcr=%d\n",msf,mp,mcr);		
	}
		}

	}
	if((nsf > sf) && (max >= Tau))
	{
		nsf = sf;
	}
	if((nsf >=sf) && (max >= Tau) && (temp >=p))
	{
		np = p;
	}

	// max <Tau
	if((nsf<=sf && temp <=p &&  max<Tau) )
	{
		printf("<Tau\n");

		if(nsf<sf)
		{
			nsf=sf;
		}
		if(p<14)
		{
			np=p+2;
		}
		else if(sf <12)
		{
			nsf++;
			np=p;
		}
		else
		{
			np=temp;
		}
	}
	else
	{
		np=temp;
	}
	printf("np=%d nsf=%d\n",np,nsf);
//for Probing
if(sf==7 && max <Tau && StopFlag1==0)
{
StopFlag2=1;
printf("Probing done\n");
}

}



void FindCR()
{


	//x~1
	if(  (Amrssi - 1) >=  Sensitivity[nsf-7]  )
	{
		ncr = 1;
	}
	//1~2
	else if(  (Amrssi - 2) >=  Sensitivity[nsf-7]  )
	{
		ncr = 2;
	} 
	//2~3
	else if(  (Amrssi - 3) >=  Sensitivity[nsf-7]  )
	{
		ncr = 3;
	} 
	else
	{
		ncr =4;
	}	
	printf("ncr=%d\n",ncr);
	// CR and power adjustment
	double I;
	double I2;
	double I3;
	if(ncr ==4)
	{
		if(np<= 10)
		{
//cr 4
                        I= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4)) * ( pow(2.0,nsf) / 125) *PowerI[(np)/2-1];
                        //cr 1 p+4
                        I2= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4 -3)) * ( pow(2.0,nsf) / 125) *PowerI[(np+4)/2-1];
                        //cr 2 p+2
                        I3= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4 -2)) * ( pow(2.0,nsf) / 125) *PowerI[(np+2)/2-1];

                        if((I2 <I) && (I2 < I3))
                        {
                                ncr=1;
                                np+=4;
                        }

                        if((I3 <I2) && (I3 < I))
                        {
                                ncr=2;
                                np+=2;
                        }

		}
		else if(np <=12)
		{
			//cr 4
			I= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4)) * ( pow(2.0,nsf) / 125) *PowerI[(np)/2-1];
			//cr 2 p+2
			I2= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4 -2)) * ( pow(2.0,nsf) / 125) *PowerI[(np+2)/2-1];
			if(I >I2)
			{
				ncr=2;
				np+=2;
			}
		}
	}
	else if(ncr ==3)
	{
		if(np<= 10)
		{
			//cr 3
			I= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4)) * ( pow(2.0,nsf) / 125) *PowerI[(np)/2-1];
			//cr 1 p+4
			I2= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4 -2)) * ( pow(2.0,nsf) / 125) *PowerI[(np+4)/2-1];
			//cr 2 p+2
			I3= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4 -1)) * ( pow(2.0,nsf) / 125) *PowerI[(np+2)/2-1];

			if((I2 <I) && (I2 < I3))
			{
				ncr=1;
				np+=4;
			}

			if((I3 <I2) && (I3 < I))
			{
				ncr=2;
				np+=2;
			}           
		}
		else if(np <=12)
		{
			//cr 3
			I= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4)) * ( pow(2.0,nsf) / 125) *PowerI[(np)/2-1];
			//cr 2 p+2
			I2= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4 -2)) * ( pow(2.0,nsf) / 125) *PowerI[(np+2)/2-1];
			if(I >I2)
			{
				ncr=2;
				np+=2;
			}
		}

	}
	else if (ncr ==2)
	{
		if(np <=12)
		{
			//cr 2
			I= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4)) * ( pow(2.0,nsf) / 125) *PowerI[(np)/2-1];
			//cr 2 p+2
			I2= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4 -1)) * ( pow(2.0,nsf) / 125) *PowerI[(np+2)/2-1];
			if(I >I2)
			{
				ncr=1;
				np+=2;
			}
		}



	}
	else 
	{

	}

}





void Next(int n)
{
	int i;
	char buf[60];
	//next round setting
        double max=0.01;
        for(i=0;i<4;i++)
        {
                if(max < prr[i])
                {
                        max =prr[i];
                }
        }
printf("compare with m\n");
	if ( msf <= sf && max >=Tau)
	{
		double I;
		double I2;
		//next
		I= ( 12.25 + 8 + ceil( (8*32 - 4*nsf + 28+16) /  (4*(nsf ))  ) *(ncr+4)) * ( pow(2.0,nsf) / 125) *PowerI[(np)/2-1];
		//min      
		I2= ( 12.25 + 8 + ceil( (8*32 - 4*msf + 28+16) /  (4*(msf ))  ) *(mcr+4)) * ( pow(2.0,msf) / 125) *PowerI[(mp)/2-1];
printf("I : %d  %d  %d  \n",nsf,np,ncr);
printf("I2 : %d  %d  %d  \n",msf,mp,mcr);		
printf("I=%lf\nI2=%lf\n",I,I2);

if((I2 < I) && (msf <=sf))
		{
			sf=msf;
			p=mp;
			cr=mcr;
printf("min setting: sf=%d p=%d cr=%d\n",sf,p,cr);
		}
		else
		{
			p=np;
			sf=nsf;
			cr=ncr;
		}
	}
	else
	{
		p=np;
		sf=nsf;
		cr=ncr;

	}


//Start Probing
if(msf==sf && mcr==cr && mp==p && StopFlag1==1)
{
printf("ADR done. Start testing\n");
StopFlag1= 0;
psf=msf;
pp=mp;
pcr=mcr;
}

 if(StopFlag1!=1)
                {
                        Probing();
                }



for(i=0;i<n;i++)
        {

//p2->16
//if((((12-sf)*7*4 + 4*(16-p)/2 + (cr-1))*(35*30+8)) ==0)


//p2->14
if((((12-sf)*7*4 + 4*(14-p)/2 + (cr-1))*(35*30+8)) ==0)
{
printf("start\n");
fseek(GW[i],          2 ,SEEK_SET);
}
else
{


//p2->16
               // fseek(GW[i],   ((12-sf)*8*4 + 4*(16-p)/2 + (cr-1))*(35*30+8) +2         ,SEEK_SET);

//p2->14
               fseek(GW[i],   ((12-sf)*7*4 + 4*(14-p)/2 + (cr-1))*(35*30+8) +2         ,SEEK_SET);

} 
 }
if(StopFlag1!=0)
{
	printf("=============new round sf: %d p: %d  cr: %d===============\n",sf,p,cr);
}
else
{
printf("=====Probing:  new round sf: %d p: %d  cr: %d===============\n",sf,p,cr);

}
}



void PC()
{
	double I;
	double TOA;
double dr;
	TOA = ( 12.25+8  + ceil( (8*32 - 4*sf + 28+16) /  (4*(sf ))  ) *(double)(cr+4)) * ( pow(2.0,sf) / 125) ;
	printf("TOA=%lf\n",TOA);

	I= ( 12.25 +8 + ceil( (8*32 - 4*sf + 28+16) /  (4*(sf - 2))  ) *(cr+4)) * ( pow(2.0,sf) / 125) *PowerI[p/2-1];
	printf("I=%lf\n",I);
dr= 1000* sf * 125  /(pow(2.0,sf)) * ((double)4 / (cr+4));
printf("dara rate = %lf \n",dr);
}




void Probing()
{
p=pp;
sf=psf;
cr=pcr;

printf("Probing sf=%d p=%d cr=%d\n",sf,p,cr);
if(cr!=1)
{
cr--;
}
else if (p!=2)
{
p -=2;
cr=4;
}
else if(sf!=7)
{
sf--;
cr=4;
p=14;

}

printf("After Probing sf=%d p=%d cr=%d\n",sf,p,cr);
pp=p;
psf=sf;
pcr=cr;

}













