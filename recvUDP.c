#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
	
#define err_exit(m) \
	do \
	{  \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while (0)

int main()
{
	// 设置输出为无缓冲
	setvbuf(stdout, NULL, _IONBF, 0);
	fflush(stdout);

	printf("广播接收程序启动完成\n");

	// 绑定本地信息
	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_ANY);
	addrto.sin_port = htons(20000);

	// 创建套接字
	int sockfd = 0;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		err_exit("socket");

	// 设置该套接字为广播类型
	const int opt = 1;
	int nb = 0;
	if ((nb = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt))) < 0)
		err_exit("setsockopt");

	if (bind(sockfd, (struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) < 0)
		err_exit("bind");

	// 用于绑定广播信息

		struct sockaddr_in addrfrom;
		bzero(&addrfrom, sizeof(struct sockaddr_in));
		int len = sizeof(addrfrom);
		char name[50] = {0};
	while (1) { // 使用一个while循环用于不断接受广播
		int ret = recvfrom(sockfd, name, 50, 0, (struct sockaddr *)&addrfrom, (socklen_t *)&len);
		if (ret <= 0)
			err_exit("recvfrom");
		if (strcmp(name, "_EXIT") == 0) {
			printf("退出广播接收程序\n");
			exit(EXIT_SUCCESS);
		}
		printf("IP: %s Name: %s\n", inet_ntoa(addrfrom.sin_addr), name);
		bzero(&name, sizeof(name));
	}
}
