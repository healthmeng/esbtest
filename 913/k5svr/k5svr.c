#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "k5_include.h"


#define   PORT    3545 

tU4 do_serve(tK5_esb* head, tK5_net *net, tU4* len, tU1* buf){
	tI4 ret= -1;
	switch (head->service){
	case file_open:
		buf[*len]='\0';
		ret=open(buf,O_RDWR);
		break;
	case file_read:
		ret=read(net->dst_port,buf,*len);
		break;
	case file_write:
		ret=write(net->dst_port,buf,*len);
		break;
	case file_close:
		ret=close(net->dst_port);
		break;
	default:
		ret=-1;
	}	
	return ret;
}

ssize_t do_read(int fd, void* buf, ssize_t len){
	int rd=0;
	while(rd<len){
		ssize_t ret;
		ret=read(fd,buf,len-rd);
		if(ret<0)
			break;
		else
			rd+=ret;
	}
	return rd;
}

void* proc_req(void* param){
	int sockfd=param-NULL;
	tU4 len;
	tK5_net net;
	tU1* buf=NULL;
	tK5_esb esb;
	tU4 ret=-1;
	if(do_read(sockfd,&esb,sizeof(esb))==sizeof(esb) &&
		do_read(sockfd,&net,sizeof(net))==sizeof(net) &&
		do_read(sockfd,&len,sizeof(len))==sizeof(len)){
		buf=(tU1*)malloc(len);
		if(len>0)
			do_read(sockfd,buf,len);
		ret=do_serve(&esb,&net,&len,buf);
	}
	write(sockfd,&esb,sizeof(esb));
	write(sockfd,&ret,sizeof(ret));
	write(sockfd,&len,sizeof(len));
	if(len>0)
		write(sockfd,buf,len);
	close(sockfd);
	if(buf)
		free(buf);
}

int main(){
	int sockfd;  
	int reuse=1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  
    if(sockfd < 0)  
    {  
        perror("socket");  
        exit(-1);  
    }  
      

    struct sockaddr_in addr;  
    memset(&addr,0, sizeof(addr));   
    addr.sin_family = AF_INET;       
    addr.sin_port   = htons(PORT);  
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int)); 

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) || listen(sockfd,10)){
		perror("bind/listen error:");
		return 1;
	}
	while(1){
		int conn=accept(sockfd,NULL,NULL);
		pthread_t pt;
		pthread_create(&pt,NULL,proc_req,conn+NULL);
	//	proc_req(conn+NULL);
	}
	return 0;
}
