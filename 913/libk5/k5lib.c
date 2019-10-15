#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "k5_include.h"

#define   PORT    3545  

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
	char buffer[4096];
	struct sockaddr_in addr;
	tI4 ret=0;
	int sent=0;
    esb->service = svc;             
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	if(to->net_level==0) addr.sin_addr.s_addr=0;
	else 
		addr.sin_addr.s_addr=to->hn[0].dst_addr;
	addr.sin_port=htons(PORT);
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
		return -1;
	if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)))
		return -1;	
/*	write(sockfd,esb,sizeof(*esb));
	write(sockfd,to,sizeof(tK5_net));
	write(sockfd,&len,sizeof(len));
	write(sockfd,buf,len);
*/
	
	memcpy(buffer,esb,sizeof(*esb));
	sent+=sizeof(*esb);
	memcpy(buffer+sent,to,sizeof(tK5_net));
	sent+=sizeof(tK5_net);
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
	return ret;
}

