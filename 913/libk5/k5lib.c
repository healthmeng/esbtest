#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "k5_include.h"

#define   PORT    3545  

void show_data( const char *title, const char *data, int size )
{
	int i = 0;
	char tmp[9] = {0};
	char output[3] = {0};
	char buf[17] = {0};

	printf("@@@@@@@@@@@@  show data of [%s] : start  @@@@@@@@@@@@\n", title);
	for ( i = 0; i < size; i++ )
	{
		if ( i % 16 == 0 )
		{
			printf("%04x  ", i);
			memset(buf, 0, sizeof(buf));
		}
		memset(tmp, 0, sizeof(tmp));
		memset(output, 0, sizeof(output));
		sprintf(tmp, "%08x", data[i]);
		strncpy(output, tmp + 6, 2);
		printf("%s ", output);
		if ( i % 8 == 7 )
			printf(" ");
		buf[i % 16] = ( data[i] >= 0x20 && data[i] <= 0x7e ) ? data[i] : '.';
		if ( i % 16 == 15 )
			printf(" %s\n", buf);
	}
	if ( size % 16 != 0 )
	{
		for ( i = 0; i < 16 - size % 16; i++ )
			printf("   ");
		if ( size % 16 < 8 )
			printf(" ");
		printf("  %s\n", buf);
	}
	printf("@@@@@@@@@@@@  show data of [%s] : end  @@@@@@@@@@@@\n\n", title);
}

tI4  k5_call (         //5个参数     
     tK5_esb   *esb,   //ESB总线数据结构（改为第1个参数）
     tU2    svc,   //服务编码
     tK5_net    *to,   //目的端口号（pid,fd,sock）及网络地址  
     tU4      len,   //该服务的第一个整型参数，4字节；
     tU1     *buf    //该服务的串型参数，参数之间用空格分隔；
)
/*tU4  k5_call( tU2 svc, tK5_net to,   tK5_esb_f0 esb, tU4 len, tU1* buf  )*/
{
	int sockfd=0;
	char buffer[8192];
	struct sockaddr_in addr;
	tI4 ret=0;
	tU1 *pbuf;
	int sent=0;
	len=4096;
    esb->service = svc;             
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;

	addr.sin_addr.s_addr=0;
/*	if(to->net_level==0) addr.sin_addr.s_addr=0;
	else 
		addr.sin_addr.s_addr=to->hn[0].dst_addr;
*/
	addr.sin_port=htons(PORT);
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
		return -0xff;
	if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)))
		return -0xff;	
/*	write(sockfd,esb,sizeof(*esb));
	write(sockfd,to,sizeof(tK5_net));
	write(sockfd,&len,sizeof(len));
	write(sockfd,buf,len);
*/
	while(*buf==' ')
		buf++;
	if(len>1)
	{
		pbuf=buf+strlen(buf)-1;
		while(*pbuf==' ' && pbuf!=buf)
		{
			*pbuf='\0';
			pbuf--;
		}
	}
	
	len=strlen(buf)+1;
	memcpy(buffer,esb,sizeof(*esb));
	sent+=sizeof(*esb);
//	memcpy(buffer+sent,to,sizeof(tK5_net));
//	sent+=sizeof(tK5_net);
	memcpy(buffer+sent,&len,sizeof(len));
	sent+=sizeof(len);
	if(len>0){
		memcpy(buffer+sent,buf,len);
		sent+=len;
	}
	write(sockfd,buffer,sent);
	read(sockfd,esb,sizeof(*esb));
	read(sockfd,&ret,sizeof(tU4));
	read(sockfd,&len,sizeof(len));
	if(len>0)
		read(sockfd,buf,len);
	close(sockfd);
	if (ret==-1 || ret==-0xff) return 0;	
	return strlen(buf)+1;
}

