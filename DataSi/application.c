#include <stdio.h>
//for usleep
#include <unistd.h>
#include <math.h>
//#include "./src/data.h"
#include "./src/ADR.h"
#include <stdlib.h>



uint8_t CR;
int TestNode;
int GWNumber;
//initial
p=14;
sf=12;
cr=1;
np=14;
nsf=12;
ncr=1;
mp=14;
msf=12;
mcr=4;


// TestNode  GWNumber
int main(int argc, char * argv[])
{
	CR=1;
	uint8_t i;
	FILE* setting;
	uint8_t buffer[30];	
	int RE;
	srand(time(NULL));

	//get the argc argv setting 
	TestNode = atoi(argv[1]);
	GWNumber = atoi(argv[2]);
printf("%s\n",argv[3]);
	Tau = atof(argv[3]);
	printf("TestNode : %d  GWNumber : %d Tau: %lf \n",TestNode,GWNumber,Tau);
	printf("read setting...\n");
	//read setting from setting.txt
	sprintf(buffer,"%d",TestNode);
	strncpy(buffer+strlen(buffer),"setting.txt",11);
	//buffer[strlen(buffer)]='\0';
	if(TestNode >9)
	{
		buffer[13]='\0';
	}
	else
	{
		buffer[12]='\0';
	}
	printf("open %s\n",buffer);
	//	setting=fopen("setting.txt","r");
	setting=fopen(buffer,"r");

	for(i=0;i<4;i++)
	{
		fgets(buffer,20,setting);
		sscanf(buffer,"%d %u %u %u %u",&Setting[i].RSSIEnviroment,&Setting[i].range,&Setting[i].brange,&Setting[i].bchance,&Setting[i].distance);
	}
	fclose(setting);
	printf("read setting done\n");
	for(i=0;i<4;i++)
	{
		printf("%d %u %u %u %u\n",Setting[i].RSSIEnviroment,Setting[i].range,Setting[i].brange,Setting[i].bchance,Setting[i].distance);
	}
	//output data file n
	for(i=0;i<4;i++)
	{
		char name[10];
		uint8_t number[4];
		strncpy(name,"GW",2);
		sprintf(number,"%u",i+1);
		strncpy(name+2,number,1);
		strncpy(name+3,".txt",4);
		name[7]='\0';
		GW[i]=fopen(name,"w");		
		int j;
		int k;
		int l;
		int x;
		char rbuffer[30];

		rbuffer[0]='#';
		rbuffer[1]='\n';
		//data generate;
		for(j=12;j>=7;j--)
		{
//power2 => 16 for spec
     //                   for(k=16;k>0;k-=2)
//power2 => 14
			for(k=14;k>0;k-=2)
			{
			
for(x=1;x<=4;x++)
	{
	         	char rrbuffer[30];
				strncpy(rbuffer,"#",1);
				sprintf(number,"%2d",j);
				strncpy(rbuffer+1,number,2);
				sprintf(number,"%2d",k);
				strncpy(rbuffer+3,number,2);
				sprintf(number,"%2d",x);
				strncpy(rbuffer+5,number,2);
				rbuffer[7]='\n';
				rbuffer[8]='\0';
				fwrite(rbuffer,1,strlen(rbuffer),GW[i]);
						for(l=1;l<=30;l++)
				{
					int RSSI;
					int SNR;
					usleep(50);
					RSSI= RSSIBasedGenerate(Setting[i].distance) -(Setting[i].RSSIEnviroment + InterferenceRange(Setting[i].range,rand()) + BInterferenceRange(Setting[i].brange,Setting[i].bchance,rand(),rand()) + RSSIPower(k));
					sprintf(number,"%2d",l);
					strncpy(rbuffer,number,2);
					SNR= SNRGenerate(10,RSSI,rand(),Setting[i].distance);					
					strncpy(rbuffer+2," SF ",4);
					sprintf(number,"%2d",j);
					strncpy(rbuffer+6,number,2);
					strncpy(rbuffer+8," Tx ",4);
					sprintf(number,"%2d",k);
					strncpy(rbuffer+12,number,2);
					if(Packetreceive(RSSI,j,x,rand()))
					{						 
						strncpy(rbuffer+14," RSSI: ",7);
						sprintf(number,"%4d",RSSI);
						strncpy(rbuffer+21,number,4);
						strncpy(rbuffer+25," SNR: ",6);
						sprintf(number,"%3d",SNR);
						strncpy(rbuffer+31,number,3);
						rbuffer[34]='\n';
						rbuffer[35]='\0';
						fwrite(rbuffer,1,strlen(rbuffer),GW[i]);
					}
					else
					{
						strncpy(rbuffer+14," RSSI: ",7);
						RSSI=-200;
						sprintf(number,"%4d",RSSI);
						strncpy(rbuffer+21,number,4);
						strncpy(rbuffer+25," SNR: ",6);
						SNR=-99;
						sprintf(number,"%3d",SNR);
						strncpy(rbuffer+31,number,3);
						rbuffer[34]='\n';
						rbuffer[35]='\0';
						fwrite(rbuffer,1,strlen(rbuffer),GW[i]);

						/*
						   rbuffer[14]=' ';
						   rbuffer[15]='#';
						   rbuffer[16]='\n';
						   rbuffer[17]='\0';
						 */					
					}

				}
				
				
			}
			}		
		}
		fclose(GW[i]);
	}

	for(i=0;i<GWNumber;i++)
	{
		uint8_t name[10];
		uint8_t number[4];
		strncpy(name,"GW",2);
		sprintf(number,"%u",i+1);
		strncpy(name+2,number,1);
		strncpy(name+3,".txt",4);
		name[7]='\0';
		GW[i]=fopen(name,"r");
		char str[10];
		fgets(str,10,GW[i]);
	}
	//ADR
	int ti;
	for(ti=0;ti<=10 && StopFlag2!=1;ti++)
	{
		Next(GWNumber);
		PC();
		InputData(GWNumber);
		Sort(GWNumber);
		Weight(Tau, GWNumber);
		Median(GWNumber);
		sigma(GWNumber);
		rsm(GWNumber);
		Adjustment(GWNumber);
		Poweradj();
		FindMinSF();
		ChecknSF();
		DecreaseP(GWNumber);
		FindCR();
/*		if(StopFlag1!=1)
		{
			Probing();
		}	
*/
	}
}

