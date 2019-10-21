#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "k5_include.h"

//主程序入口
int main ()
{
	printf("=== START: test svc_wait(). ===\n");

	int         ret = 0;
	tU2         serv = 0;
	tK5_net     from;
	tK5_esb     esb;
	tU4         len = 4096;
	tU1         buf[4096];

	//开启服务组（或特定服务）之后，该进程进入等待对外服务的无限循环；
	while (1)
	{
		memset ( &esb,  0, sizeof(esb)  ); //清零ESB帧结构描述
		memset ( &from, 0, sizeof(from) ); //清零网络地址描述
		memset ( &buf,  0, sizeof(buf)  ); //清零接收缓冲区

		ret = k5_wait( &esb, &from, &len, buf);	//同步接收原语
		if ( ret <= 0 )
		{
			printf("ERROR: k5_wait failed. ret = [%d]\n", ret);
			return -1;
		}

		switch ( esb.service )		//服务分支转移
		{
			case file_open:
			{
				printf("## service: file_open [%s] ##\n", buf);
				if ( ( ret = open(buf, O_RDWR) ) <= 0 )
				{
					printf("open error: %s(errno: %d)\n", strerror(errno), errno);
					k5_reply( &esb, -errno, 0, NULL );
				}
				k5_reply( &esb, 1, ret, NULL);
				break;
			}
			case file_close:
			{
				printf("## service: file_close [%d] ##\n", from.dst_port);
				if ( ( ret = close(from.dst_port) ) <= 0 )
				{
					printf("close error: %s(errno: %d)\n", strerror(errno), errno);
					k5_reply( &esb, -errno, 0, NULL );
				}
				k5_reply( &esb, 1, ret, NULL);
				break;
			}
			case file_read:
			{
				printf("## service: file_read [%d] ##\n", from.dst_port);
				if ( ( ret = read(from.dst_port, buf, len) ) <= 0 )
				{
					printf("read error: %s(errno: %d)\n", strerror(errno), errno);
					k5_reply( &esb, -errno, 0, NULL );
				}
				k5_reply( &esb, 1, ret, buf);
				break;
			}
			case file_write:
			{
				printf("## service: file_write ##\n");
				if ( ( ret = write(from.dst_port, buf, len) ) <= 0 )
				{
					printf("write error: %s(errno: %d)\n", strerror(errno), errno);
					k5_reply( &esb, -errno, 0, NULL );
				}
				k5_reply( &esb, 1, ret, NULL);
				break;
			}
			default:
			{
				printf("## WARNING: unknown service ##\n");
				break;		//未知服务错误
			}
		}	//分支结束
	}		//循环结束

	return 0;
}	//主程序结束 
