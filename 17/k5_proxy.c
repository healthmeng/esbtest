#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "k5_primitive.h"

int main()
{
	tU4 ret = 0;
	int sockfd = 0;
	int connfd = 0;
	char buf[K5_MAX_BUF + 1] = {0};
	ssize_t size = 0;
    int reuse=1;

	k5_call_send *p = NULL;
	k5_call_recv rcv;
	struct sockaddr_in scktaddr;

	memset(&rcv, 0, sizeof(k5_call_recv));
	// 设置IP和端口
	memset(&scktaddr, 0, sizeof(scktaddr));
	scktaddr.sin_family = AF_INET;
	scktaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	scktaddr.sin_port = htons(K5_SERV_PORT);

	// 创建TCP连接，接收数据
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		printf("socket() error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}
  	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	if ( bind(sockfd, (struct sockaddr*)&scktaddr, sizeof(scktaddr)) == -1 )
	{
		printf("bind() error: %s(errno: %d)\n", strerror(errno), errno);
		close(sockfd);
		return -1;
	}

	if ( listen(sockfd, 10) == -1 )
	{
		printf("listen() error: %s(errno: %d)\n", strerror(errno), errno);
		close(sockfd);
		return -1;
	}

	while ( 1 )
	{
		if ( (connfd = accept(sockfd, (struct sockaddr*)NULL, NULL)) == -1 )
		{
			printf("accept() error: %s(errno: %d)\n", strerror(errno), errno);
			close(sockfd);
			return -1;
		}

		//接收网络信息
		memset(buf, 0, sizeof(buf));
		if ( (size = recv(connfd, buf, K5_MAX_BUF, 0)) == 0 )
		{
			close(connfd);
			continue;
		}

		show_data( "recv", buf, size );
		p = (k5_call_send *)buf;

		//远程转本地
		p->net.net_level = 1;
		p->svc=p->esb.service;
		ret = k5_call( &p->esb, p->esb.service, &p->net, p->num, p->buf );
		if ( ret == 0 )
		{
			send(connfd, buf, 2, 0);//发送 失败 信息
			close(connfd);
			continue;
		}

		//成功，组织报文
		rcv.ack_err = 1;
		rcv.len = ret;
		memcpy( rcv.buf, p->buf, ret);

		//返回网络信息
		send(connfd, &rcv, sizeof(k5_call_recv), 0);
		close(connfd);
	}
	close(sockfd);

	return 0;
}
