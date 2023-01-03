#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define TCPDUMP_CMD "tcpdump -c 10 -ne -y ieee802_11_radio -s 65535 -i wlan0-1 |grep \"signal\" | grep \"SA\\|TA\" |grep \"dBm\""

int main(void)
{
	FILE *fp; 
	char buffer[1000];
	char src_mac[50];
	float rssi;

	fp = popen(TCPDUMP_CMD, "r"); 
	while(fgets(buffer, sizeof(buffer), fp))
	{
		sscanf(strstr(buffer, "dBm ")-4, "%fdB", &rssi);
		printf("rssi = %.2f", rssi);

		memset(src_mac, 0, sizeof(src_mac));
		if(strstr(buffer, "SA:")!=NULL)
		{
			sscanf(strstr(buffer, "SA:")+3, "%s", src_mac);
			printf(", SA: %s", src_mac);
		}
		else if(strstr(buffer, "TA:")!=NULL)
		{
			sscanf(strstr(buffer, "TA:")+3, "%s", src_mac);
			printf(", TA: %s", src_mac);
		}
		else
		{
			printf(", ERROR: SA and TA are not found\n");
		}
		puts("");
	}


	pclose(fp);
	return 0;
}
