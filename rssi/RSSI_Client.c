#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"
#include "signal.h"

#define MAX_USER_NUM 200
#define ReconnectionDelayTime 2
#define WLAN "wlan0"

char* server_ip;
int server_port;

void *sensing_func(void *parm);
void *report_func(void *parm);
char* get_interface_MAC_string();
char* search_pattern_following(char* str1, char* str2);
static pthread_t sensing_thread;
static pthread_t report_thread;
static pthread_mutex_t sensing_report_mutex;

static int sensing_socket;
static int construct_link(char ip[], int port);
void disconnection(int sig);

struct sensing_result
{
	char src_mac[100];
	float rssi_mean;
	float rssi_variance;
	int sample_num;
};
static struct sensing_result sensing_result[100];
static int sensing_result_num=0;

int main(int argc, char *argv[])
{
	if(argc<3)
	{
		printf("argument: [Server IP] [Server Port]\n");
		return;
	}
	else
	{
		server_ip = argv[1];
		server_port = atoi(argv[2]);
	}
	(void)signal(SIGINT, disconnection);

	pthread_mutex_init(&sensing_report_mutex, NULL);
	pthread_mutex_lock(&sensing_report_mutex);

	pthread_create(&sensing_thread, NULL, sensing_func, NULL);
	pthread_create(&report_thread, NULL, report_func, NULL);

	pthread_mutex_unlock(&sensing_report_mutex);

	while(1);

	return 0;
}

#define TCPDUMP_CMD "tcpdump -ne -y ieee802_11_radio -s 65535 -i wlan0-1 |grep \"signal\" | grep \"SA\\|TA\" |grep \"dBm\""

void *sensing_func(void *parm)
{
	FILE *fp;
	char sensing_dump[1000];
	char src_mac[100];
	float rssi;
	float old_rssi_mean;
	float new_rssi_mean;
	float old_rssi_variance;
	float new_rssi_variance;
	int i;

	if( (fp = popen(TCPDUMP_CMD, "r"))== NULL)
	{
		printf("popen() error!\n");
		exit(1);
	}

	while(fgets(sensing_dump, sizeof(sensing_dump), fp))
	{
		sscanf(strstr(sensing_dump, "dBm ")-4, "%fdB", &rssi);

		memset(src_mac, 0, sizeof(src_mac));
		if(strstr(sensing_dump, "SA:")!=NULL)
		{
			sscanf(strstr(sensing_dump, "SA:")+3, "%s", src_mac);
		}
		else if(strstr(sensing_dump, "TA:")!=NULL)
		{
			sscanf(strstr(sensing_dump, "TA:")+3, "%s", src_mac);
		}
		else
		{
			printf("ERROR: SA and TA are not found\n");
		}

		for(i=0;i<strlen(src_mac);i++) if(islower(src_mac[i]))
		{
			src_mac[i]=toupper(src_mac[i]);
		}

		pthread_mutex_lock(&sensing_report_mutex);

		//search old item
		for(i=0;i<sensing_result_num;i++)
		{
			if(strcmp(sensing_result[i].src_mac, src_mac)==0)
			{
				//policy 1: keep the latest RSSI sample for all devices
				new_rssi_mean = rssi;
				new_rssi_variance = 0;
				sensing_result[i].rssi_mean=new_rssi_mean;
				sensing_result[i].rssi_variance=new_rssi_variance;
				sensing_result[i].sample_num=1;
				
				break;
			}
		}

		//create new item if necessary
		if(i==sensing_result_num)
		{
			memcpy(sensing_result[i].src_mac, src_mac, strlen(src_mac));
			sensing_result[i].rssi_mean=rssi;
			sensing_result[i].rssi_variance=0.0;
			sensing_result[i].sample_num=1;
			sensing_result_num++;
		}

		pthread_mutex_unlock(&sensing_report_mutex);

	}

	pclose(fp);

	return;
}

void *report_func(void *parm)
{
	int i;
	char sensing_report[100];
	char rcv_buf[100];
	char waiting[] = {"Waiting for polling..."};	
	char exit[] = {"exit"};

start:
	sensing_socket = construct_link(server_ip, server_port);

	while(1)
	{
		//format: [round] [observer MAC] [target MAC] [RSSI]

		memset(rcv_buf, 0, sizeof(rcv_buf));
		printf("%s\n", waiting);
		while(read(sensing_socket, rcv_buf, sizeof(rcv_buf))==0); //wait for polling

		printf("%s is received\n", rcv_buf);
		pthread_mutex_lock(&sensing_report_mutex);

		for(i=0;i<sensing_result_num;i++)
		{
			printf("sensing_result[%d].src_mac=%s\n", i, sensing_result[i].src_mac);
			if(1) 
			{
				memset(sensing_report, 0, sizeof(sensing_report));
				sprintf(sensing_report, "%s %s %s %f\n",
					rcv_buf,
					get_interface_MAC_string(),
					sensing_result[i].src_mac,
					sensing_result[i].rssi_mean);
					
				printf("%s is sent\n", sensing_report);

				if(write(sensing_socket, sensing_report, strlen(sensing_report))<=0)
				{
					printf("%s:socket write error\n", __FUNCTION__);
					goto start;
				}
			}
		}

		memset(sensing_result, 0, sizeof(sensing_result));
		sensing_result_num=0;

		pthread_mutex_unlock(&sensing_report_mutex);

		printf("\n");
	}
}

void disconnection(int sig)
{
	close(sensing_socket);
	(void)signal(SIGINT, SIG_DFL);
}

static int construct_link(char ip[], int port)
{
	int client_fd = -1;
	struct hostent *host;
	struct sockaddr_in client_sin;
	char buf[100];

	while(client_fd <= 0)
	{
		if((host = gethostbyname(ip)) == NULL)
		{
			fprintf(stderr, "Can't get ip !!!\n");
			return -1;
		}
		client_fd = socket(AF_INET, SOCK_STREAM, 0);
		bzero(&client_sin, sizeof(client_sin));
		client_sin.sin_family = AF_INET;
		client_sin.sin_addr = *((struct in_addr *)host->h_addr);
		client_sin.sin_port = htons(port);
		if(connect(client_fd, (struct sockaddr *)&client_sin, sizeof(client_sin)) == -1)
		{
			fprintf(stderr, "Can't connect to server \n");
			fprintf(stdout, "Reconstruct link after 2 seconds \n");
			client_fd = -1;
			sleep(ReconnectionDelayTime);	
		}
	}
	return client_fd;
}

char* get_interface_MAC_string()
{
	FILE *fp;
	char cmd[100];
	static char ret[50];

	char line_buf[100];
	char *cursor;
	int mac[6];

	memset(ret, 0, sizeof(ret));
	sprintf(ret, "-1");

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ifconfig %s", WLAN);

	if((fp=popen(cmd, "r"))!=NULL)
	{
		while(!feof(fp))
		{
			memset(line_buf, 0, sizeof(line_buf));
			fgets(line_buf, sizeof(line_buf), fp);

			if( (cursor=search_pattern_following(line_buf, "HWaddr ")) != NULL )
			{
				sscanf(cursor, "%02X:%02X:%02X:%02X:%02X:%02X",
					&mac[0], &mac[1], &mac[2],
					&mac[3], &mac[4], &mac[5] );

				memset(ret, 0, sizeof(ret));
				sprintf(ret, "%02X:%02X:%02X:%02X:%02X:%02X",
					mac[0], mac[1], mac[2],
					mac[3], mac[4], mac[5] );
					
				break;
			}

		}
		pclose(fp);
	}
	else
	{
		printf("%s:Fail to open the file\n", __FUNCTION__);
	}

	return ret;
}

char* search_pattern_following(char* str1, char* str2)
{
	char* ret;
	if( (ret=strstr(str1, str2)) != NULL ) ret+=strlen(str2);

	return ret;
}
