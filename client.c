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

void handler()
{
	printf("退出聊天程序\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	if (argc != 4)
		err_exit("Usage: a.out IP name file");
	printf("正在连接用户，请稍后...\n");

	// 打开一个文件流，用于保存聊天记录
	FILE *fp;
	if ((fp = fopen(argv[3], "w")) == NULL)
		err_exit("fopen");

	// 创建一个套接字
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_exit("socket");

	// 绑定服务端的IP地址与端口
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(30000);
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);

	// 向服务端发起连接
	if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		err_exit("connect");
	
	// 发送自己的昵称给服务端
	write(sockfd, argv[2], strlen(argv[2]));

	// 读服务端的昵称到peername缓冲区
	char peername[25] ={0};
	if (read(sockfd, peername, sizeof(peername)) < 0)
		err_exit("read1");

	printf("用户连接成功，请开始聊天吧！\n");

	// 创建一个子进程
	pid_t pid;
	if ((pid = fork()) < 0)
		err_exit("fork");

	if (pid == 0) { // 子进程用于向服务端发送数据
		char sendbuf[1024] = {0};
		signal(SIGUSR1, handler);
		while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
			write(sockfd, sendbuf, strlen(sendbuf));
			if (strcmp(sendbuf, "_EXIT\n") == 0)
				break;
			printf("%s:\n%s\n", argv[2], sendbuf);
			fprintf(fp, "%s:\n%s\n", argv[2], sendbuf);
			bzero(&sendbuf, sizeof(sendbuf));
		}
		exit(EXIT_SUCCESS);
	} else { // 父进程用于接收服务端的数据
		char recvbuf[1024] = {0};
		int ret = 0;
		while (1) {
			if ((ret = read(sockfd, recvbuf, sizeof(recvbuf))) < 0)
				err_exit("read2");
			if (ret == 0 || strcmp(recvbuf, "_EXIT\n") == 0) {
				printf("对方已经退出聊天程序\n");
				break;
			}
			printf("%s:\n%s\n", peername, recvbuf);
			fprintf(fp, "%s:\n%s\n", peername, recvbuf);
			bzero(&recvbuf, sizeof(recvbuf));
		}
		printf("现在退出聊天程序\n");
		kill(pid, SIGUSR1); // 发送信号给子进程
		exit(EXIT_SUCCESS);
	}

	// 关闭流和套接字
	close(sockfd);
	close(fp);

	exit(EXIT_SUCCESS);
}
