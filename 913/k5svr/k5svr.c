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


tI4 do_vm(const char* dev){
	return 0;
}

tI4 do_mount(const char* mnt){
	return 0;
}

tI4 do_umount(const char* mnt){
	return 0;
}

tI4 do_login(const char* user_psd){
	return -1;
}

tI4 do_logout(const char* user){
	return 0;
}

tI4 do_proc_start(const char* proc){
	return 0;
}

tI4 do_threads(const char* proc){
	// if proc exists
	return 0;
}

tI4 do_state(void *buf){
	return 0;
}

tI4 do_rename(const char* buf){
	return 0;
}

tI4 do_link(const char * buf){
	return 0;
}

tI4 do_set_mode(const char* buf){
	return 0;
}

tI4 do_copy(const char* buf){
	return 0;
}

tI4 do_set_dir_owner(const char* buf){
	return 0;
}

tI4 do_dir_set_mode(const char* buf){
	return 0;
}

tU4 do_serve(tK5_esb* head, tK5_net *net, tU4* len, tU1* buf){
	tI4 ret= 0;
	switch (head->service){
	case sys_start_vmm:
	case sys_stop_vmm:
		ret=do_vm(buf);
		break;
	case sys_mount_fs:
		ret=do_mount(buf);
		break;
	case sys_unmount_fs:
		ret=do_umount(buf);
		break;
	case  sys_login:
		ret=do_login(buf);
		break;
	case sys_logout:
		ret=do_logout(buf);
		break;
/*	case sys_svc_grp_reg:
	case sys_svc_grp_del:
	case sys_svc_reg:
	case sys_svc_del:
	case sys_svc_list:
		ret=-0xff;
		break;
*/	
	case prc_start:
		ret=do_proc_start(buf);
		break;
	case prc_stop:
		ret=-1;
		break;
	case prc_fork:
	case prc_exit:
	case prc_pause:
	case prc_resume:
	case prc_wait:
		ret=0;
		break;
/*	case prc_sync:
		ret=-0xff;
		break;
*/
	case thr_start:
	case thr_stop:
	case thr_exit:
	case thr_pause:
	case thr_resume:
	case thr_wait:
		ret=do_threads(buf);
		break;
/*	case thr_sync:
		ret=-0xff;
		break;
*/
	case file_create:
		ret=creat(buf,0664);
		break;
	case file_delete:
		ret=unlink(buf);
		break;
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
	case file_rewind:
		ret=lseek(net->dst_port,0,SEEK_SET);
		break;
	case file_state:
	case file_get_mode:
		ret=do_state(buf);
		break;
	case file_seek:
		ret=lseek(net->dst_port,atol(buf),SEEK_SET);
		return ret;
	case file_rename:
		ret=do_rename(buf);
		break;
	case file_link:
		ret=do_link(buf);
		break;
	case file_unlink:
		ret=unlink(buf);
		break;
	case file_set_mode:
		ret=do_set_mode(buf);
		break;
	case file_copy:
		ret=do_copy(buf);
		break;

	case dir_make:
		ret=mkdir(buf,0755);
		break;
	case dir_remove:
		ret=rmdir(buf);
		break;
	case dir_change:
	case dir_link:
	case dir_unlink:
		ret=0;
		break;
	case dir_set_owner:
		ret=do_set_dir_owner(buf);
		break;
	case dir_set_mode:
		ret=do_dir_set_mode(buf);
		break;
	default:
		ret=-0xff;
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
