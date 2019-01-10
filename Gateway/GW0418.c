#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>   /* File control definitions */ 
#include <errno.h>   /* Error number definitions */ 
#include <termios.h> /* POSIX terminal control definitions */
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/time.h>

//for select
#include <sys/stat.h>
#include <sys/select.h>

#define SERV_PORT 5134
#define MAXDATA   1024

#define MAXNAME 1024
int fd_socket;     /* fd into transport provider */
int i;     /* loops through user name */
int length;    /* length of message */
int size;    /* the length of servaddr */
int fdesc;    /* file description */
int ndata;    /* the number of file data */
//char data[MAXDATA]; /* read data form file */
char buffer_udp_receive[MAXDATA];  /*server DL */
char buf[BUFSIZ];     /* holds message from server */
struct hostent *hp;   /* holds IP address of server */
struct sockaddr_in myaddr;   /* address that client uses */
struct sockaddr_in servaddr; /* the server's full addr */

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


ssize_t size_buffer_lora;
uint8_t packet[255];
//
int fd_lora;
uint8_t buffer_lora[256];
//DevEUI DevNonce DevAddr framecounter
int table[10][50];
int dev=0;
// e.g. 蕞篙emp[3], size=4  offset_store=4
int offset_store=0;
int offset_finish=0;
uint8_t temp[1000];
uint8_t temp2[1000];

//for read_from_port
uint16_t packetsize=0;
bool packetsizeget =0;
bool mtypeget =0;
int size_forwarding,size_header,end_header,end_forwarding,a=0;
bool wait_packet=0;
fd_set set;
struct timeval timeout;
int rv_select;

//for read_from_server();
fd_set set_udp;
struct timeval timeout_udp;
int rv_select_udp;


int   error;
struct termios new_port_settings,
	       old_port_settings[38];



char comports[38][16]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4","/dev/ttyS5",
	"/dev/ttyS6","/dev/ttyS7","/dev/ttyS8","/dev/ttyS9","/dev/ttyS10","/dev/ttyS11",
	"/dev/ttyS12","/dev/ttyS13","/dev/ttyS14","/dev/ttyS15","/dev/ttyUSB0",
	"/dev/ttyUSB1","/dev/ttyUSB2","/dev/ttyUSB3","/dev/ttyUSB4","/dev/ttyUSB5",
	"/dev/ttyAMA0","/dev/ttyAMA1","/dev/ttyACM0","/dev/ttyACM1",
	"/dev/rfcomm0","/dev/rfcomm1","/dev/ircomm0","/dev/ircomm1",
	"/dev/cuau0","/dev/cuau1","/dev/cuau2","/dev/cuau3",
	"/dev/cuaU0","/dev/cuaU1","/dev/cuaU2","/dev/cuaU3"};



void ShowTable(void);
int write_to_port(const char* buff);
int read_from_port(void);
int open_udp_client(int argc, char* argv[]);
void udp_send(uint16_t packetsize);
int RS232_OpenComport(int comport_number, int baudrate, const char *mode);
void mtype_check();
/*
 *  LoRa network server DL
 */
void read_from_server();


int main(int argc, char* argv[])
{
    char initial_command[20];
	printf("trying connect to %s\n",comports[16]);
	RS232_OpenComport(16,115200,"8N1");
	open_udp_client(argc,argv);
	if (fd_lora > 0) {
		printf("connect to ttyUSBX\n"); 
	} 
	//initial_command
	strncpy(initial_command,"gw",2);
	write_to_port(initial_command);
	while(1)
	{
		read_from_server();
		read_from_port(); 	
		


	}
	return 0;
}




void ShowTable(void)
{
	int a,b;
	printf("DevEUI DevNonce DevAddr framecounter\n");
	for(a=0;a<dev;a++)
		for(b=0;b<=3;b++)
		{
			printf("%d",table[a][b]);	 	
		}
	printf("\n");			
}
int write_to_port(const char* buff) { 
	int n;	
	n = write(fd_lora,buff,strlen(buff));
	if (n < 0) 
		fputs("write() of 6 bytes failed!\n", stderr); 
	return 0;
} 
int RS232_OpenComport(int comport_number, int baudrate, const char *mode) 
{ 
	printf("inRS232\n");
	int baudr,
	    status;
	printf("mode= %c %c %c\n",mode[0],mode[1],mode[2]);
	if((comport_number>37)||(comport_number<0))
	{
		printf("illegal comport number\n");
		return(1);
	}

	switch(baudrate)
	{
		case      50 : baudr = B50;
			       break;
		case      75 : baudr = B75;
			       break;
		case     110 : baudr = B110;
			       break;
		case     134 : baudr = B134;
			       break;
		case     150 : baudr = B150;
			       break;
		case     200 : baudr = B200;
			       break;
		case     300 : baudr = B300;
			       break;
		case     600 : baudr = B600;
			       break;
		case    1200 : baudr = B1200;
			       break;
		case    1800 : baudr = B1800;
			       break;
		case    2400 : baudr = B2400;
			       break;
		case    4800 : baudr = B4800;
			       break;
		case    9600 : baudr = B9600;
			       break;
		case   19200 : baudr = B19200;
			       break;
		case   38400 : baudr = B38400;
			       break;
		case   57600 : baudr = B57600;
			       break;
		case  115200 : baudr = B115200;
			       break;
		case  230400 : baudr = B230400;
			       break;
		case  460800 : baudr = B460800;
			       break;
		case  500000 : baudr = B500000;
			       break;
		case  576000 : baudr = B576000;
			       break;
		case  921600 : baudr = B921600;
			       break;
		case 1000000 : baudr = B1000000;
			       break;
		case 1152000 : baudr = B1152000;
			       break;
		case 1500000 : baudr = B1500000;
			       break;
		case 2000000 : baudr = B2000000;
			       break;
		case 2500000 : baudr = B2500000;
			       break;
		case 3000000 : baudr = B3000000;
			       break;
		case 3500000 : baudr = B3500000;
			       break;
		case 4000000 : baudr = B4000000;
			       break;
		default      : printf("invalid baudrate\n");
			       return(1);
			       break;
	}

	int cbits=CS8,
	    cpar=0,
	    ipar=IGNPAR,
	    bstop=0;

	if(strlen(mode) != 3)
	{
		printf("invalid mode \"%s\"\n", mode);
		return(1);
	}

	switch(mode[0])
	{
		case '8': cbits = CS8;
			  break;
		case '7': cbits = CS7;
			  break;
		case '6': cbits = CS6;
			  break;
		case '5': cbits = CS5;
			  break;
		default : printf("invalid number of data-bits '%c'\n", mode[0]);
			  return(1);
			  break;
	}

	switch(mode[1])
	{
		case 'N':
		case 'n': cpar = 0;
			  ipar = IGNPAR;
			  break;
		case 'E':
		case 'e': cpar = PARENB;
			  ipar = INPCK;
			  break;
		case 'O':
		case 'o': cpar = (PARENB | PARODD);
			  ipar = INPCK;
			  break;
		default : printf("invalid parity '%c'\n", mode[1]);
			  return(1);
			  break;
	}

	switch(mode[2])
	{
		case '1': bstop = 0;
			  break;
		case '2': bstop = CSTOPB;
			  break;
		default : printf("invalid number of stop bits '%c'\n", mode[2]);
			  return(1);
			  break;
	}

	/*
http://pubs.opengroup.org/onlinepubs/7908799/xsh/termios.h.html
http://man7.org/linux/man-pages/man3/termios.3.html
	 */

	fd_lora = open(comports[comport_number], O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd_lora==-1)
	{
		perror("unable to open comport /dev/ttyUSB0 ");
		//return(1);


		comport_number++;

		fd_lora = open(comports[comport_number], O_RDWR | O_NOCTTY | O_NDELAY);
		if(fd_lora==-1)
		{

			perror("unable to open comport /dev/ttyUSB1 ");	

			comport_number++;
			fd_lora = open(comports[comport_number], O_RDWR | O_NOCTTY | O_NDELAY);
			if(fd_lora==-1)
			{
				perror("unable to open comport /dev/ttyUSB2 ");		
				if(fd_lora == -1)
				{				
					comport_number++;
					fd_lora = open(comports[comport_number], O_RDWR | O_NOCTTY | O_NDELAY);

					return(1);
				}	  	
			}  	
		}    
	}

	error = tcgetattr(fd_lora, old_port_settings + comport_number);
	if(error==-1)
	{
		close(fd_lora);
		perror("unable to read portsettings ");
		return(1);
	}
	memset(&new_port_settings, 0, sizeof(new_port_settings));  /* clear the new struct */

	new_port_settings.c_cflag = cbits | cpar | bstop | CLOCAL | CREAD;
	new_port_settings.c_iflag = ipar;
	new_port_settings.c_oflag = 0;
	new_port_settings.c_lflag = 0;
	new_port_settings.c_cc[VMIN] = 0;      /* block untill n bytes are received */
	new_port_settings.c_cc[VTIME] = 0;     /* block untill a timer expires (n * 100 mSec.) */

	cfsetispeed(&new_port_settings, baudr);
	cfsetospeed(&new_port_settings, baudr);

	error = tcsetattr(fd_lora, TCSANOW, &new_port_settings);
	if(error==-1)
	{
		close(fd_lora);
		perror("unable to adjust portsettings ");
		return(1);
	}

	if(ioctl(fd_lora, TIOCMGET, &status) == -1)
	{
		perror("unable to get portstatus");
		return(1);
	}

	status |= TIOCM_DTR;    /* turn on DTR */
	status |= TIOCM_RTS;    /* turn on RTS */

	if(ioctl(fd_lora, TIOCMSET, &status) == -1)
	{
		perror("unable to set portstatus");
		return(1);
	}

	return(0); 
} 
/*
 *1.check '!'
 *2.if  uint16_t packetsize 讀完 then check packet 
 *3.read 
 *
 */

int read_from_port(void) 
{ 
	int s;
	int i;
	memset(buffer_lora,0x00,sizeof(buffer_lora));
/*
	FD_ZERO(&set);// clear the set 
	FD_SET(fd_lora, &set);//add  file descriptor to the set 
	timeout.tv_sec = 1;
	timeout.tv_usec = 10000;
	rv_select = select(fd_lora + 1, &set, NULL, NULL, &timeout);
	if(rv_select == -1)
		perror("select"); // an error accured 
	else if((rv_select == 0) && (offset_store-20)>offset_finish  ) 
	{
				printf("timeout"); // a timeout occured 
	}
	else
	{	
*/
		if(1){
		//printf("read\n");
		size_buffer_lora= read(fd_lora,buffer_lora,255);/* there was data to read */
		if(size_buffer_lora!=0 && size_buffer_lora!= -1)
		{
			printf("size= %d  ",size_buffer_lora);
			printf("receive:");
			for(i=0;i<size_buffer_lora;i++)printf("%c ",buffer_lora[i]);
		}
		if(size_buffer_lora>0 || (offset_store-20)>offset_finish)
		{	
			printf("read_from_port fun: if(size_buffer_lora>0)\n");
			printf("offser_store= %d  offset_finish= %d",offset_store,offset_finish);
			strncpy(temp+offset_store,buffer_lora,size_buffer_lora);
			offset_store+=size_buffer_lora;
			printf("offset_finish= %d ",offset_finish);
			printf("offser_store= %d\n",offset_store);
			if(packetsizeget ==1) 
			{
				printf("packetsize= %d\n",packetsize);



				//0308 + mtype bool mtypeget
				if(mtypeget==0)
				{
					if(offset_store > offset_finish)
					{
						mtype_check();
						mtypeget=1;	

					}


				}
				else //mtypeget==1
				{

					switch(mtype)
					{					
						//010 unconfirmed macpayload
						case(uUL):
                        case(cUL):	
									
							if(wait_packet==1)
							{
								printf("read_from_port fun: if(wait_packet==1)\n");
								if(offset_store>end_forwarding)
								{						
									strncpy(packet,temp+offset_finish,size_header+size_forwarding+1);
									//strncpy(packet,temp+offset_finish,size_header+size_forwarding);	
									packet[strlen(packet)]=	'\0';
									udp_send(size_header+size_forwarding+1);
									printf("udp---1\n");
									mtypeget=0;
									int j=0;
									printf("size_forwarding=%d  size_heade=%d end_header=%d end_forwarding=%d \n",size_forwarding,size_header,end_header,end_forwarding);
									for(j=0;j<=(size_forwarding+size_header);j++)printf("temp[%d]=%c\n",offset_finish+j,temp[offset_finish+j]);
									offset_finish =end_forwarding+1;
									packetsizeget = 0;
									wait_packet=0;                                       
								}

							}

							else
							{
								printf("read_from_port fun: if(packetsizeget ==1): else \n");


								a=0;
								for(i=offset_finish;i<=offset_store;i++)
								{
									if(temp[i]==' ')a++;	
									if(a ==9 )	//macpayload header 9bytes -> 9 ' '		
									{

										//without RSSI SNR

										// size_forwarding= packetsize-8;
										// end_forwarding= i+ size_forwarding;

										//with RSSi SNR
										//size_forwarding= packetsize+10;
										size_forwarding= 
											(packetsize-8) + 19  ;
										end_forwarding= i+ size_forwarding;
										end_header=i;
										size_header=end_header-offset_finish;
										printf("i=%d  size_forwarding=%d \n",i,size_forwarding);				
										if(offset_store>end_forwarding)
										{						
											strncpy(packet,temp+offset_finish,size_header+size_forwarding+1);
											packet[strlen(packet)]=	'\0';		
											udp_send(size_header+size_forwarding+1);
											mtypeget=0;
											printf("udp---2\n");
											int j=0;
											printf("size_forwarding=%d  size_heade=%d end_header=%d end_forwarding=%d \n",size_forwarding,size_header,end_header,end_forwarding);
											for(j=0;j<=(size_forwarding+size_header);j++)printf("temp[%d]=%c\n",offset_finish+j,temp[offset_finish+j]);
											offset_finish =end_forwarding+1;
											packetsizeget = 0;
											wait_packet=0;
											i=offset_store+1;     
										}

										else
										{
											wait_packet=1;
											printf(" wait_packet=1\n");
											i=offset_store+1;

											//等到forwarding的packet讀好 
										}
									}
								}			
							}
							break;
						case Jr:
							a=0;
							for(i=offset_finish;i<=offset_store;i++)
							{
								if(temp[i]==' ')a++;	
								if(a ==19 )	//macpayload header 8bit -> 8 ' '		
								{

									//without RSSI SNR

									// size_forwarding= packetsize-8;
									// end_forwarding= i+ size_forwarding;

									//with RSSi SNR
									//size_forwarding= packetsize+10;
									size_forwarding=0;
									end_forwarding= i+ size_forwarding;
									end_header=i;
									size_header=end_header-offset_finish;
									printf("i=%d  size_forwarding=%d \n",i,size_forwarding);				
									if(offset_store>end_forwarding)
									{						
										strncpy(packet,temp+offset_finish,size_header+size_forwarding+1);
										packet[strlen(packet)]=	'\0';		
										udp_send(size_header+size_forwarding+1);
										mtypeget=0;
										printf("udp---2\n");
										int j=0;
										printf("size_forwarding=%d  size_heade=%d end_header=%d end_forwarding=%d \n",size_forwarding,size_header,end_header,end_forwarding);
										for(j=0;j<=(size_forwarding+size_header);j++)printf("temp[%d]=%c\n",offset_finish+j,temp[offset_finish+j]);
										offset_finish =end_forwarding+1;
										packetsizeget = 0;
										wait_packet=0;
										i=offset_store+1;     
									}
								}
							}




						case Ja:
						default :
							break;	
					}
				}
			}
			else
			{
				for(i=offset_finish;i<=offset_store-3;i++)
				{
					if((temp[i]=='!') && (temp[i+3]=='!'))
					{
						//packetsize= (uint16_t)temp[i+1]<<8 | (uint16_t)temp[i+2];	
						int a,b;
						//a=atoi(temp[i+1]);
						//b=atoi(temp[i+2]);
						//packetsize = (a*10+b)*2;
						printf("temp[i+1]= %c  temp[i+2]= %c\n",temp[i+1],temp[i+2]);
						packetsize= ((temp[i+1]-'0')*10 + (temp[i+2]-'0')) ;				
						printf("get packetsize = %d\n",packetsize);
						packetsizeget=1 ;
						offset_finish= i+4;
						i=offset_store +1; //jump out of the for loop

					}    	
				}	
			}


			// temp快滿 => 處理 

			if(offset_store>=700)
			{
				printf("read_from_port fun: if(offset_store>=250)\n");
				strncpy(temp2,temp+offset_finish,offset_store - offset_finish);
				offset_store -= offset_finish;	
				end_forwarding-=offset_finish;	
				offset_finish=0;
				memset(temp,0x00,sizeof(temp));
				strcpy(temp,temp2); 
				printf("-------------------temp buffer reset--------------------------\n");
				printf("current buffer:\n");
				printf("end_forwarding=%d offset_finish=%d  	offset_store=%d\n",end_forwarding,offset_finish,offset_store);
				int k;
				//	for(k=0;k<offset_store;k++)printf("temp[%d]=%c\n",k,temp[k]);	
			} 

		}
	}
	return 0; 
	}  
	int open_udp_client(int argc, char* argv[])
	{
		/*
		 *  Get a socket into UDP
		 */
		if ((fd_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror ("socket failed!");
			exit(1);
		}
		/*
		 * Bind to an arbitrary return address.
		 */
		bzero((char *)&myaddr, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		myaddr.sin_port = htons(0);

		if (bind(fd_socket, (struct sockaddr *)&myaddr,sizeof(myaddr)) <0) {
			perror("bind failed!");
			exit(1);
		}
		/*
		 * Fill in the server's UDP/IP address
		 */

		bzero((char *)&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERV_PORT);
		hp = gethostbyname(argv[1]);

		if (hp == 0) {
			fprintf(stderr, "could not obtain address of %s\n", argv[2]);
			return (-1);
		}
		bcopy(hp->h_addr_list[0], (caddr_t)&servaddr.sin_addr,hp->h_length);	
		return 0;

	} 
	void udp_send(uint16_t packetsize)
	{
		uint8_t	tempp[255];

		// GW number
		strcpy(tempp,"1 ");
		strncpy(tempp+2,packet,size_header+size_forwarding+1);
		//strcat(tempp,packet);
		/* 發送資料給 Server */
		size = sizeof(servaddr);
		//if (sendto(fd_socket, buffer_lora, size_buffer_lora, 0, (struct sockaddr*)&servaddr, size) == -1) {
		printf("udp send : %s\n",packet);
		if (sendto(fd_socket, tempp,packetsize+2, 0, (struct sockaddr*)&servaddr, size) == -1) {
			//	if (sendto(fd_socket, packet,packetsize, 0, (struct sockaddr*)&servaddr, size) == -1) {
			perror("write to server error !");
			//exit(1);
		}
		//read_from_server();
		}

		void mtype_check()
		{
			uint8_t b,c;	
			int i;
			int handle;
			//sscanf(buf,"%d %d %d %d %d %d %d %d %d %d",&handle[0],&handle[1],&handle[2],&handle[3],&handle[4],&handle[5],&handle[6],&handle[7],&handle[8],&handle[9]);
			sscanf(temp+offset_finish,"%d",&handle);


			printf("MHDR: ");
			printf("%d  ",handle);
			for(i=0;i<8;i++){
				b=handle<<(i);
				c=b>>(7);
				printf("%d",c);
			}
			printf("\n");
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
						 printf("mtype=Pr\n");
					}
					else//110
						{
					mtype=Rr;
						 printf("mtype=Rr\n");
					}
				}
				else//10
				{
				b=handle<<(2);
					c=b>>(7);
					if(c)//101
					{
					mtype=cDL;
						 printf("mtype=cDL\n");
					}
					else//100
						{
					mtype=cUL;
						 printf("mtype=cUL\n");
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
					}
					else//010 unconfirmed macpayload
					{
						mtype=uUL;
						printf("mtype=uUL\n");
					}
				}
				else//00 
				{
					b=handle<<(2);
					c=b>>(7);
					if(c)//001 joint accept
					{
						mtype=Ja;
						printf("mtype=Ja\n");
					}
					else//000 joint request
					{
						mtype=Jr;
						printf("mtype=Jr\n");
					}			
				}
			}	
		}



		void read_from_server()
		{
int nbytes;			
	memset(buffer_udp_receive,0x00,sizeof(buffer_lora));
	FD_ZERO(&set);/* clear the set */
	FD_SET(fd_socket, &set_udp);/* add  file descriptor to the set */
	timeout_udp.tv_sec = 1;
	timeout_udp.tv_usec = 10000;
	rv_select_udp = select(fd_socket + 1, &set_udp, NULL, NULL, &timeout);
	if(rv_select_udp == -1)
		perror("select"); /* an error accured */
	else if(rv_select_udp == 0)
	{
		//		printf("timeout"); /* a timeout occured */
	}
	else
	{	
		nbytes = recvfrom(fd_socket,buffer_udp_receive, MAXDATA, 0, (struct sockaddr*)&servaddr, &size);
			printf("receive data from server: %s\n",buffer_udp_receive);
			write_to_port(buffer_udp_receive);	
			/*
			int nbytes;
			nbytes = recvfrom(fd_socket,buffer_udp_receive, MAXDATA, 0, (struct sockaddr*)&servaddr, &size);
			printf("receive data from server: %s\n",buffer_udp_receive);
			write_to_port(buffer_udp_receive);
			*/
		}		
}















