#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
// For Mosquitto include
#include <mosquitto.h>
#include <unistd.h>
#include <pthread.h>

// MQTT server connection information.
#define HOST "192.168.1.217"
#define PORT  1883
#define KEEP_ALIVE 60
bool session = true;
struct mosquitto *mosq = NULL;
pthread_t pmosid = 0;

//UDP Socket server
#define SERV_PORT 5134
#define MAXNAME 1024

extern int errno;
typedef struct Table
{
	uint64_t DevEUI;
	uint64_t AppNonce;
	uint32_t DevAddr;
	uint16_t FCnt;
	uint16_t JCnt;
	bool connect;
	uint16_t GW;
} Table;
typedef enum
{
	Jr,  //000
	Ja,  //001
	uUL, //010
	uDL, //011
	cUL, //100
	cDL, //101
	Rr,  //110
	Pr,  //111
} MType;
Table table[10];
char buf[BUFSIZ];
MType mtype = Pr;
int fd_store;
int offset_table;

void mtype_check();
uint16_t FCnt_check();
void store();
void publish(int i);
int DevAddr_get();
void joinrequest();
//void send_udp(const char* buffer_udp);
void send_udp();
bool ACKbitcheck(int FCTRL);

int fd_socket; /* file description into transport */
int recfd;	 /* file descriptor to accept        */
int length;	/* length of address structure      */
int nbytes;	/* the number of read **/
//uint8_t buf[BUFSIZ];
struct sockaddr_in myaddr;		/* address of this service */
struct sockaddr_in client_addr; /* address of client    */

char buffer_udp[50];

//**********************************************MQTT**********************************************************

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if(message->payloadlen){
		//Todo 判斷DevAddr
		//1=>"1"
		// sprintf(buffer_udp, % d, table[i].GW);
		printf("%s %s\n", message->opic, message->payload);
		strncpy(buffer_udp, "setDevAddr 3", 13);
		//	sprintf(buffer_udp+7,"%d",table[i].DevAddr);
		buffer_udp[strlen(buffer_udp)] = '\0';
		send_udp();
		sleep(3);
		//strlen
		strncpy(buffer_udp, message->payload,16);
		buffer_udp[strlen(buffer_udp)] = '\0';
		send_udp();
    }else{
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    int i;
    if(!result){
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, "EndDeviceControl", 2);
    }else{
        fprintf(stderr, "Connect failed\n");
    }
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Pring all log messages regardless of level. */
    printf("%s\n", str);
}

void mos_init()
{
	
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, session, NULL);
    if(!mosq){
        fprintf(stderr, "Error: Out of memory.\n");
        exit(-1);
    }
    //mosquitto_log_callback_set(mosq, my_log_callback);
    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);
    //mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
    mosquitto_will_set(mosq,"EndDeviceControl", sizeof("livewill"), "livewill", 2, false);
    mosquitto_threaded_set(mosq, 1);
}

void * pthread_mos(void *arg)
{
    int toserver = -1;
    int timeout = 0;
    
    while(toserver){
        toserver = mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE);
        if(toserver){
            timeout++;
            fprintf(stderr, "Unable to connect server [%d] times.\n", timeout);
            if(timeout > 3){
                fprintf(stderr, "Unable to connect server, exit.\n" );
                pthread_exit(NULL);
            }
            sleep(10);
        }
    }

    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    pthread_exit(NULL);
}

//***************************************************************************************************************************

main()
{
	//for test
	offset_table = 2;
	table[1].DevEUI = 5;
	table[1].DevAddr = 1;
	table[2].DevEUI = 4;
	table[2].DevAddr = 2;

	/*
	 *      Get a socket into UDP/IP
	 */
	if ((fd_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	/*
	 *    Set up our address
	 */
	bzero((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERV_PORT);

	/*
	 *     Bind to the address to which the service will be offered
	 */
	if (bind(fd_socket, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind failed\n");
		exit(1);
	}

	/*
	 * Loop continuously, waiting for datagrams
	 * and response a message
	 */
	length = sizeof(client_addr);
	printf("Server is ready to receive !!\n");
	printf("Can strike Cntrl-c to stop Server >>\n");
	fd_store = open("./store.txt", O_RDWR);
	if (fd_store == -1)
	{
		perror("open file error!");
		exit(1);
	}
	strncpy(buffer_udp, "send 12345", 10);
	buffer_udp[10] = '\0';
	send_udp();

	int ret = 0;

    mos_init();

    ret = pthread_create(&pmosid, NULL, pthread_mos, NULL);
    if(ret != 0){
        printf("create pthread mos error.\n");
        exit(-1);
    }
    pthread_detach(pmosid);

	while (1)
	{
		/*
		   char buff[15];
		   strncpy(buff,"send 12345",10);
		   buff[10]='\0';
		   send_udp(buff);
		 */

		memset(buf, '\0', sizeof(buf));
		if ((nbytes = recvfrom(fd_socket, &buf, MAXNAME, 0, (struct sockaddr *)&client_addr, (socklen_t *)&length)) < 0)
		{
			perror("could not read datagram!!");
			continue;
		}
		/*
		   if (sendto(fd_socket, &buf, nbytes, 0, (struct sockaddr*)&client_addr, length) < 0) {
		   perror("Could not send datagram!!\n");
		   continue;
		   }
		 */
		if (buf[0] != '\0')
		{
			int j;

			int k, a = 0;
			for (k = 0; k <= strlen(buf); k++)
			{
				if (buf[k] == ' ')
					a++;
				if (a == 7)
					printf("buf:%s payload:%c \n", buf, buf[k + 1]);
				k = strlen(buf) + 1;
			}

			mtype_check();
		}
		// printf("Received data form %s : %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
		/* return to client */
		/*
		   if (sendto(socket_fd, &buf, nbytes, 0, (struct sockaddr*)&client_addr, length) < 0) {
		   perror("Could not send datagram!!\n");
		   continue;
		   }
		 */
		//printf("Can Strike Crtl-c to stop Server >>\n");
	}
}
/*
 * publish to MQTT broker
 * mosquitto_pub -t enddevicei -m "nessage"  
 *    i device EUI
 */
void publish(int i)
{
	// Todo 改字串組合方式，增加HOST。
	uint8_t temp[100];
	strcpy(temp, "mosquitto_pub -t -h 192.168.1.217 enddevice");
	//uint64_t DevEUI;
	//sprintf(temp+strlen(temp),"%s",HOST);
	sprintf(temp + strlen(temp), "%d", table[i].DevAddr);
	strcpy(temp + strlen(temp), " -m \"");
	strcpy(temp + strlen(temp), buf);
	strcpy(temp + strlen(temp), "\"");
	//publish to MQTT broker
	printf("%s\n", temp);
	system(temp);
}
void mtype_check()
{
	uint8_t b, c;
	int i;
	int handle[10];

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d", &handle[0], &handle[1], &handle[2], &handle[3], &handle[4], &handle[5], &handle[6], &handle[7], &handle[8], &handle[9]);

	printf("MHDR: ");
	for (i = 0; i < 8; i++)
	{
		b = handle[1] << (i);
		c = b >> (7);
		printf("%d", c);
	}
	printf("\n");

	b = handle[1] << (0);
	c = b >> (7);
	//for test 0116
	//   store();
	if (c) //1
	{
		b = handle[1] << (1);
		c = b >> (7);
		if (c) //11
		{
			b = handle[1] << (2);
			c = b >> (7);
			if (c) //111
			{
				mtype = Pr;
				printf("mtype=Pr\n");
			}
			else //110
			{
				mtype = Rr;
				printf("mtype=Rr\n");
			}
		}
		else //10
		{
			b = handle[1] << (2);
			c = b >> (7);
			if (c) //101
			{
				mtype = cDL;
				printf("mtype=cDL\n");
			}
			else //100
			{
				mtype = cUL;
				printf("mtype=cUL\n");
				store();
			}
		}
	}
	else //0
	{
		b = handle[1] << (1);
		c = b >> (7);
		if (c) //01
		{
			b = handle[1] << (2);
			c = b >> (7);
			if (c) //011
			{
			}
			else //010 unconfirmed macpayload
			{
				mtype = uUL;
				printf("mtype=uUL\n");
				store();
			}
		}
		else //00
		{
			b = handle[1] << (2);
			c = b >> (7);
			if (c) //001 joint accept
			{
				mtype = Ja;
				printf("mtype=Ja\n");
				store();
			}
			else //000 joint request
			{
				mtype = Jr;
				printf("mtype=Jr\n");
				joinrequest();
			}
		}
	}
}
uint16_t FCnt_check()
{
	uint16_t temp;
	int j, i, a = 0;
	int handle[10];

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d", &handle[0], &handle[1], &handle[2], &handle[3], &handle[4], &handle[5], &handle[6], &handle[7], &handle[8], &handle[9]);
	for (i = 0; i <= 8; i++)
	{
		printf("%d ", handle[i]);
	}
	for (i = 0; i <= strlen(buf); i++)
	{
		if (buf[i] == ' ')
			a++;
		if (a == 9)
		{
			printf("payload:");
			for (j = i + 1; j < strlen(buf); j++)
				printf("%c", buf[j]);
			printf("\n");
			i = strlen(buf) + 1;
		}
	}
	uint16_t c, temp2 = 0;
	c = (uint16_t)handle[7] << 8 | (uint16_t)handle[8];
	int addr;
	addr = DevAddr_get();
	//printf("receive_FCnt: %d    server_FCnt: %d\n",c,table[addr].FCnt);
	printf("receive_Cnt: %d    server_JCnt: %d   server_FCnt: %d\n", c, table[addr].JCnt, table[addr].FCnt);
	/*
	* handle ED join-again
	*/
	if (table[addr].FCnt == 0)
	{
		table[addr].JCnt = 0;
	}

	if (c > table[addr].FCnt)
	{
		//ACK handle
		if (mtype = cUL)
		{
			strncpy(buffer_udp, "send 1 3 ACK", 12);
			buffer_udp[12] = '\0';
			send_udp();
		}
		table[addr].FCnt = c;
		return addr;
	}
	else
	{
		printf("ignore\n");
		printf("-----------------------------------------------------------------\n");
		return 0;
	}
}

void store()
{
	uint16_t temp;
	temp = FCnt_check();
	if (temp > 0) //  accept
	{
		/*
		 *store in store.txt
		 */

		publish(temp);
		buf[strlen(buf)] = '\n';
		write(fd_store, buf, strlen(buf));
		//table[0].FCnt= temp;
		/*
		//ACK handle
		if((mtype = cUL) && (1==ACKbitcheck(buf)) )
		{
		strncpy(buffer_udp,"send 1 0 3 ACK ",14);		
	    buffer_udp[14]='\0';
		send_udp();
		}
		*/
		printf("accept\n");
		printf("-----------------------------------------------------------------\n");
	}
	else //ignore
	{
	}
}
/*
   get DevAddr and fing the i of table[i]
   for macpayload
 */
//uint32_t DevAddr
//haven't handle it  , now addr= table[addr]
int DevAddr_get()
{
	uint32_t temp = 0;
	uint8_t t1, t2, t3, t4;
	int handle[10];
	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d", &handle[0], &handle[1], &handle[2], &handle[3], &handle[4], &handle[5], &handle[6], &handle[7], &handle[8], &handle[9]);
	t1 = handle[2];
	t2 = handle[3];
	t3 = handle[4];
	t4 = handle[5];
	//temp = ( handle[2] <<24) | ( handle[3] << 16)| (handle[5] << 8) | handle[5] ;
	temp = ((uint32_t)t1 << 24) | ((uint32_t)t2 << 16) | ((uint32_t)t3 << 8) | (uint32_t)t4;
	printf("DevAddr: %d\n", temp);

	return temp;
}

void joinrequest()
{
	uint16_t temp;
	uint64_t DevEUI;
	uint8_t t1, t2, t3, t4, t5, t6, t7, t8;
	int j, i, handle_offset_table, a = 0;
	handle_offset_table = 0;
	uint16_t c, temp2 = 0;
	int handle[20];

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &handle[0], &handle[1], &handle[2], &handle[3], &handle[4], &handle[5], &handle[6], &handle[7], &handle[8], &handle[9], &handle[10], &handle[11], &handle[12], &handle[13], &handle[14], &handle[15], &handle[16], &handle[17], &handle[18], &handle[19]);
	for (i = 0; i <= 19; i++)
	{
		printf("%d ", handle[i]);
	}
	t1 = handle[10];
	t2 = handle[11];
	t3 = handle[12];
	t4 = handle[13];
	t5 = handle[14];
	t6 = handle[15];
	t7 = handle[16];
	t8 = handle[17];
	DevEUI = (uint64_t)t1 << 56 | (uint64_t)t2 << 48 | (uint64_t)t3 << 40 | (uint64_t)t4 << 32 | (uint64_t)t5 << 24 | (uint64_t)t6 << 16 | (uint64_t)t7 << 8 | (uint64_t)t8;
	printf("DevEUI= %d \n", DevEUI);
	printf("offset_table=%d\n", offset_table);
	for (i = 1; i <= offset_table; i++)
	{
		printf("%d  DevEUI= %d \n", i, table[i].DevEUI);
	}

	for (i = 1; i <= offset_table; i++)
	{
		if (DevEUI == table[i].DevEUI)
		{
			handle_offset_table = i;
			i = offset_table + 1;
		}
	}
	//no find in table
	if (handle_offset_table == 0)
	{
		offset_table++;
		table[offset_table].DevEUI = DevEUI;
		table[offset_table].DevAddr = offset_table;
		table[offset_table].FCnt = 0;
		table[offset_table].JCnt = 0;
		table[offset_table].connect = 0;
		table[offset_table].GW = handle[0];
		handle_offset_table = offset_table;
	}
	t1 = handle[18];
	t2 = handle[19];
	c = (uint16_t)t1 << 8 | (uint16_t)t2;
	printf("receive_Cnt: %d    server_JCnt: %d   server_FCnt: %d\n", c, table[handle_offset_table].JCnt, table[handle_offset_table].FCnt);
	if (c > table[handle_offset_table].JCnt)
	{
		table[handle_offset_table].JCnt = c;
		/*
		 *Ja DL
		 */
		publish(0);
		strncpy(buffer_udp, "Jasend ", 7);
		sprintf(buffer_udp + 7, "%d", table[handle_offset_table].DevAddr);
		//	sprintf(buffer_udp+7,"%d",table[i].DevAddr);
		buffer_udp[strlen(buffer_udp)] = '\0';
		send_udp();
		printf("accept\n");
		printf("-----------------------------------------------------------------\n");
	}
	else
	{
		printf("ignore\n");
		printf("-----------------------------------------------------------------\n");
		c = 0;
	}
}
//void send_udp(const char* buffer_udp)
void send_udp()
{
	int i;
	i = strlen(buffer_udp);
	if (sendto(fd_socket, &buffer_udp, i, 0, (struct sockaddr *)&client_addr, length) < 0)
	{
		perror("Could not send datagram!!\n");
	}
}
bool ACKbitcheck(int FCTRL)
{
	uint8_t t, t1;
	t = FCTRL << 2;
	t1 = t >> 7;
	if (t1 == 1)
	{
		printf("ACKbit = 1\n");
		return 1;
		//return 0;
	}
	printf("ACKbit = 0\n");
	return 0;
}
