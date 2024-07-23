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

int main(int argc, char *argv[])
{
	if (argc != 2)
		err_exit("Usage: cx name");

	// 设置输出为无缓冲
	setvbuf(stdout, NULL, _IONBF, 0);
	fflush(stdout);

	// 创建套接字
	int sockfd = 0;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		err_exit("socket");

	// 设置该套接字为广播类型
	const int opt = 1;
	int nb = 0;
	if ((nb = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt))) < 0)
		err_exit("setsockopt");

	// 绑定本机地址，发送广播
	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addrto.sin_port = htons(20000);
	int len = sizeof(addrto);
	int ret = sendto(sockfd, argv[1], strlen(argv[1]), 0, (struct sockaddr *)&addrto, len);
	if (ret <= 0)
		err_exit("sendto");
	else
		printf("广播发送成功\n");
}
