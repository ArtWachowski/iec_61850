#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<signal.h>
#include<pcap.h>
#include<stdio.h>	//For standard things
#include<stdlib.h>	//malloc
#include<string.h>	//strlen
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header
#include<netinet/if_ether.h>	//For ETH_P_ALL
#include<net/ethernet.h>	//For ether_header
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>

//Python Binding
#include "Python.h"
#include "ml_interface.h"
struct API api;

char * get_prediction(unsigned char*, int);
void ProcessPacket(unsigned char* , int, unsigned char*);
void process_tcp(unsigned char * , int, unsigned char*);
char * raw_payload(unsigned char* , int);
_Bool starts_with(unsigned char*, unsigned char* , int);

FILE *logfile;
struct sockaddr_in source,dest;
int sock_raw,tcp=0,dst_p=102,src_p=102,others=0,id=1,total=0,mms=0,und=0,i,j,src,dst,c=0;
char ml_string[] ="";

void sigint_handler(int signum) { //Handler for SIGINT
	signal(SIGINT, sigint_handler);
	printf("Stopped using Ctrl+C \n");
	close(sock_raw);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sigint_handler);

	char *dev = argv[1];
	if (dev == NULL) {
		printf("Please provide int name ./cc_snifer eth0 \n");
		return(0);
	}

	Py_Initialize();
	wchar_t *name = Py_DecodeLocale(argv[0], NULL);
	Py_SetProgramName(name); 

	int saddr_size , data_size;
	struct sockaddr saddr;

	unsigned char *buffer = (unsigned char *) malloc(65536);
	
	logfile=fopen("log.txt","w");
	if(logfile==NULL) 
	{
		printf("Unable to create log.txt file.");
	}
	printf("Starting... device: %s\n", dev);
	
	sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
	setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , dev , strlen(dev)+ 1 );
	
	if(sock_raw < 0)
	{
		//Print the error with proper message
		perror("Socket Error");
		return 1;
	}

	while(1)
	{
		//unsigned char *buffer = (unsigned char *) malloc(65536);
		saddr_size = sizeof saddr;
		//Receive a packet
		data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &saddr , (socklen_t*)&saddr_size);
		if(data_size < 0 )
		{
			printf("Recvfrom error , failed to get packets\n");
			return 1;
		}
		//Now process the packet
		ProcessPacket(buffer,data_size,dev);
		//free(buffer);
	}
	Py_Finalize();
	printf("Finished");	
	return 0;
}

void ProcessPacket(unsigned char* buffer, int size, unsigned char* dev)
{
	unsigned short iphdrlen;

	unsigned short ethlen;

	ethlen = sizeof(struct ethhdr);

	if(strcmp( dev, "ppp0") == 0){

		ethlen = 16;
	}
	
	//Get the IP Header part of this packet , excluding the ethernet header
	struct iphdr *iph = (struct iphdr*)(buffer + ethlen);

	iphdrlen = iph->ihl*4;
	
	struct tcphdr *tcph=(struct tcphdr*)(buffer + iphdrlen + ethlen);
	dst = ntohs(tcph->dest);
	src = ntohs(tcph->source);
	
	//printf("Source port: %d  Dest port : %d \r\n", src, dst);

	//TODO Introduce Flow inspections instead of individual packet ins ...

        if (dst == dst_p ) //if 102 .  src == src_p || ..(src == src_p) //
	{   
		++total;
		switch (iph->protocol) //Check the Protocol and do accordingly...
		{
			case 6:  //TCP Protocol
				++tcp;
				process_tcp(buffer , size, dev);
				break;
		
			default: //Some Other Protocol like ARP etc.
				++others;
				break;
		}
		++c;
		if(c>=10){printf("Traffic update:TCP port 102: %d - MMS Payload: %d - Undefined %d - Others: %d - Total: %d\n",tcp,mms,und,others,total);c=0;}
	}
}

void process_tcp(unsigned char* Buffer, int Size,unsigned char* dev)
{
	unsigned short iphdrlen;

	unsigned short ethlen = 16;

	if(strcmp( dev, "ppp0") != 0){  //hot fixes :D days before submission 
		struct ethhdr *eth = (struct ethhdr *)Buffer;
		ethlen = sizeof(struct ethhdr);
	}

	struct iphdr *iph = (struct iphdr *)(Buffer  + ethlen);
	iphdrlen =iph->ihl*4;
	
	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iph->saddr;
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = iph->daddr;

	struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + ethlen);
			
	int header_size =  ethlen + iphdrlen + tcph->doff*4;
		
	char *ml_payload = raw_payload(Buffer + header_size , Size - header_size );

	char KEEPALIVE[] = "0000";
	char TPKT[] = "0300";
	char src_mac[]= "00:00:00:00:00:00"; //ppp rulez 
	char dst_mac[]= "00:00:00:00:00:00";
	char src_ip[16];
	char dst_ip[16];
	char src_port[6];
	char dst_port[6];
	char ttl[4];
	char ml_packet[Size];
	
	//TODO Strip ISO Headers: TKPT COTP ...  
	//TODO Introduce granular MMS Packet type Verification 

	//printf("Payload is  %d\n",ml_payload[0]);

	if (ml_payload[0] == '\0'){
		++others;
	} else if (starts_with(ml_payload,KEEPALIVE, sizeof KEEPALIVE))	{
		++others;

	} else if (starts_with(ml_payload,TPKT, sizeof TPKT)){

		//printf("Lenght od ml_payload is %ld\n",sizeof(ml_payload)); //API errors with handling ml_payload 
		++mms;
		//sprintf(src_mac, "%02x:%02x:%02x:%02x:%02x:%02x",eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5]);
		//sprintf(dst_mac, "%02x:%02x:%02x:%02x:%02x:%02x",eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
		sprintf(src_ip, "%s",inet_ntoa(source.sin_addr));
		sprintf(dst_ip, "%s",inet_ntoa(dest.sin_addr));
		sprintf(src_port, "%u",ntohs(tcph->source));
		sprintf(dst_port, "%u",ntohs(tcph->dest));
		sprintf(ttl,"%d",iph->ttl);
	        snprintf(ml_packet, sizeof(ml_packet), "%s,%s,%s,%s,%s,%s,%s,%d,%s",src_mac,dst_mac,src_ip,dst_ip,src_port,dst_port,ttl,id,ml_payload);
 		char *ml_result = get_prediction(ml_packet , Size); //result from ML
 		//printf("ID: %d ML_Packet: %s ML_Result: %s\n",id,ml_packet,ml_result);
		fprintf(logfile , "ID: %d ML_Packet: %s ML_Result: %s\n",id,ml_packet,ml_result); //added result of verification 
		++id;
	} else {++und;}

	//TODO Return the result and rasi alarm 

}

char *raw_payload(unsigned char* data , int Size)
{
	unsigned char *ml_payload = (unsigned char *) malloc(65535); // free(): invalid next size (normal) Aborted (core dumped)

	ml_payload[0] = '\0';	
	int i;

	for(i=0 ; i < Size ; i++)
	{ 
		size_t offset = strlen(ml_payload);
		sprintf(&(ml_payload[offset]),"%02X",(unsigned int)data[i]);
	}
	return ml_payload;
}


_Bool starts_with(unsigned char* payload_string, unsigned char* header_cmp, int Size)
{
	//char substr[Size]; strncpy(substr,payload_string, Size); // free(): invalid next size (normal) Aborted (core dumped)
	unsigned char* substr = calloc(strlen(payload_string) + 1, 1);
	strcpy(substr,payload_string);

	//printf("Prefix %s String %s\n",header_cmp, substr);
	while(*header_cmp)
    	{
		if(*header_cmp++ != *payload_string++)
			//free(substr);
			return 0;
	}

	//free(substr);
	return 1;
}

char *get_prediction(unsigned char* Buffer, int buffer)
{

	unsigned char *ml_result = (unsigned char *) malloc(20);
	ml_result[0] = '\0';
	int rc;
	PyObject *pName = NULL; 
	PyObject *pModule = NULL;
	PyObject *py_results = NULL;
	PyObject *fill_api  = NULL;
	#define PYVERIFY(exp) if ((exp) == 0) { fprintf(stderr, "%s[%d]: ", __FILE__, __LINE__); PyErr_Print(); exit(1); }
	PyRun_SimpleString(
			"import sys;"
			"sys.path.insert(0, '.')" );

	PYVERIFY( pName = PyUnicode_FromString("ml_interface") )
	PYVERIFY( pModule = PyImport_Import(pName) )
	Py_DECREF(pName);
	PYVERIFY( fill_api = PyObject_GetAttrString(pModule, "fill_api") )
	PYVERIFY( py_results = PyObject_CallFunction(fill_api, "k", &api) )
	assert(py_results == Py_None);

        ml_result = api.get_prediction(Buffer, buffer);
        assert(ml_result != 0);

        rc = api.release_object(ml_result);
        assert(rc == 0);

	return ml_result;
}
