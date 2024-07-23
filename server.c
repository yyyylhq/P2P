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
	printf("关闭信息发送进程\n");
	printf("等待其他用户的连接\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		err_exit("Usage: name");

	printf("聊天服务器程序正在启动，请稍等...\n");

	// 创建套接字
	int listenfd;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_exit("socket");
	
	// 设置套接字选项
	int on = 1;
	int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	 
	// 绑定本地地址和端口
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(30000);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 设置套接字为被动套接字
	if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		err_exit("bind");
	
	// 监听客户端的接入，一次只能处理一个客户端的接入
	if((listen(listenfd, 1)) < 0)
		err_exit("listen");
    printf("服务器启动成功，正在等待连接\n"); 

	struct sockaddr_in peeraddr;
	socklen_t socklen = sizeof(peeraddr);
	int conn;
	char peername[25];
	pid_t pid;
	while (1) { // 使用一个while循环一直监听
		// 与请求接入的客户端连接
		if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &socklen)) < 0)
			err_exit("accept");

		bzero(&peername, sizeof(peername));
		if ((ret = read(conn, peername, sizeof(peername))) < 0) // 读取客户端的昵称
			err_exit("read1");
		if (strcmp(peername, "_EXIT") == 0)
			exit(EXIT_SUCCESS);

		write(conn, argv[1], strlen(argv[1])); // 发送自己的昵称给服务端

		printf("ip: %s	name: %s\n", inet_ntoa(peeraddr.sin_addr), peername);

		// 创建一个子进程
		if ((pid = fork()) < 0)
			err_exit("fork");

		if (pid == 0) { // 子进程用于发送数据给服务端
			char sendbuf[1024] = {0};
			signal(SIGUSR1, handler);
			while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
				write(conn, sendbuf, strlen(sendbuf));
				printf("%s:\n%s\n", argv[1], sendbuf);
				bzero(&sendbuf, sizeof(sendbuf));
			}
		} else { // 父进程用于接受客户端发来的数据
			char recvbuf[1024];
			int ret;
			while (1) {
				bzero(&recvbuf, sizeof(recvbuf));
				if ((ret = read(conn, recvbuf, sizeof(recvbuf))) < 0)
					err_exit("read2");
				if (ret == 0 || strcmp(recvbuf, "_EXIT\n") == 0) {
					printf("对方已经退出了聊天程序\n");
					write(conn, recvbuf, strlen(recvbuf));
					break;
				}
				printf("%s:\n%s\n", peername, recvbuf);
			}
			kill(pid, SIGUSR1); // 发送信号给子进程
		}
	}
}
