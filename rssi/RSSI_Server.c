#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ServerPort 55688

void GetRecord(int connfd)
{
	char buf[256];
	while(1)
	{
		read(connfd, buf, sizeof(buf));

		if(strcmp(buf, "end") == 0)
		{
			break;
		}

		printf("Get: %s\n", buf);

	}
}

int main(int argc, char *argv[])
{
	//create socket
	int sockfd, clientSockfd;
	sockfd = clientSockfd = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd == -1)
	{
		printf("Fail to create a socket.\n");
	}

	//socket connection
	struct sockaddr_in serverInfo, clientInfo;
	int addrlen = sizeof(clientInfo);
	bzero(&serverInfo, sizeof(serverInfo));

	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = INADDR_ANY;
	serverInfo.sin_port = htons(ServerPort);
	bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
	listen(sockfd, 5);

	char message[] = {"Can Send!\n"};
	char buf[256] = {};

	while(1)
	{
		clientSockfd = accept(sockfd, (struct sockaddr*) &clientInfo, &addrlen);
		printf("Accept!\n");		

		write(clientSockfd, message, sizeof(message));
		while(1){
		
		printf("RECV!!!!\n");
		recv(clientSockfd, buf, sizeof(buf), 0);		
		if(strcmp(buf, "exit") == 0){
			memset(buf, 0, sizeof(buf));
			printf("BREAK!!!!!\n");
			break;
		}

		printf("Get: %s From Client...\n", buf);
		memset(buf, 0, sizeof(buf));
		
		}
	}

	puts("End!!!");
	return 0;
}

