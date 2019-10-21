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
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <crypt.h>

#include "k5_include.h"


#define   PORT    3545 


tI4 do_mount(const char* mnt){
//	char dev[1024],dir[1024];
//	sscanf(mnt,"%s%s",mnt);
	char cmd[1024];
	sprintf(cmd,"mount %s",mnt);	
	//sprintf(cmd,"mount %s 1>/dev/null 2>/dev/null",mnt);	
	return -WEXITSTATUS(system(cmd));
}

tI4 do_umount(const char* mnt){
	return umount(mnt);
}

tI4 do_login(const char* user_psd){
	char usr[1024],pwd[1024];
    struct spwd *sp;
	sscanf(user_psd,"%s%s",usr,pwd);
	sp=getspnam(usr);
    if(sp )
	{
		char* res=crypt(pwd,sp->sp_pwdp);
		if (res && strcmp(sp->sp_pwdp, res)==0)
		return 0;
	}
	return -1;
}

tI4 do_logout(const char* user){
	struct spwd *sp=getspnam(user);
	if(sp==NULL)
		return -1;
	return 0;
}

tI4 do_proc_start(const char* proc){
/*	struct stat st;
	if(stat(proc,&st)<0) return -1;
	if(S_ISDIR(st.st_mode) || st.st_mode & 1==0)
		return -1;*/
	if(fork()==0)
		execlp(proc,proc,NULL);
	return 0;
}

tI4 do_proc_stop(const char* proc){
	char cmd[1024];
	sprintf(cmd,"killall -9 %s",proc);
	//sprintf(cmd,"killall -9 %s 1>/dev/null 2>/dev/null",proc);
	system(cmd);
	return 0;
}

tI4 do_threads(const char* proc){
	// if proc exists
	return 0;
}

tI4 do_rename(const char* buf){
	char src[1024],dst[1024];
	char cmd[1024];
	sscanf(buf,"%s%s",src,dst);
	sprintf(cmd,"mv %s %s",src,dst);
	//sprintf(cmd,"mv %s %s 1>/dev/null 2>/dev/null",src,dst);
	return -WEXITSTATUS(system(cmd));
}

tI4 do_link(const char * buf){
	char src[1024],dst[1024];
	sscanf(buf,"%s%s",src,dst);
	return link(src,dst);
}

tI4 do_set_mode(const char* buf){
	char file[1024];
	int mode;
	sscanf(buf,"%s%o",file,&mode);
	return chmod(file,mode);
}

tI4 do_get_mode(char *buf){
	struct stat st;
	if(stat(buf,&st)<0) return -1;
	sprintf(buf,"%o",st.st_mode);
	return 0;
}

tI4 do_copy(const char* buf){
	char src[1024],dst[1024];
	char cmd[1024];
	sscanf(buf,"%s%s",src,dst);
	sprintf(cmd,"/bin/cp %s %s",src,dst);
	return -WEXITSTATUS(system(cmd));
}

tI4 do_set_dir_owner(const char* buf){
	char dir[1024],usr[1024];
	sscanf(buf,"%s%s",dir,usr);
	struct passwd* pwd=getpwnam(usr);
	if(pwd){
		return chown(dir,pwd->pw_uid,-1);
	}
	return -1;
}

tI4 do_get_dir_owner(char* dir){
	uid_t uid;
	struct stat st;
	if(stat(dir,&st)==0){
		struct passwd* pwd=getpwuid(st.st_uid);
		if(pwd){
			strcpy(dir,pwd->pw_name);
			return 0;
		}
	}
	return -1;
}

tI4 do_get_uid(char* buf){
	struct passwd* pwd=getpwnam(buf);
	if(pwd){
		sprintf(buf,"%d",pwd->pw_uid);
		return 0;
	}else return -1;
}


tI4 do_set_uid(const char* buf)
{
	char user[1024];
	unsigned int uid;
	char cmd[1024];
	sscanf(buf,"%s%d",user,&uid);	
	sprintf(cmd,"usermod -u %d %s",uid,user);
	//sprintf(cmd,"usermod -u %d %s 1>/dev/null 2>/dev/null",uid,user);
	return -WEXITSTATUS(system(cmd));
}

tI4 do_get_gid(char* buf)
{
    struct group* grp=getgrnam(buf);
	if(grp){
    	sprintf(buf,"%d",grp->gr_gid);
		return 0;
	}else return -1;
}

tI4 do_set_gid(const char* buf)
{
	char grp[1024];
	unsigned int gid;
	char cmd[1024];
	sscanf(buf,"%s%d",grp,&gid);	
	sprintf(cmd,"groupmod -g %d %s",gid,grp);
//	sprintf(cmd,"groupmod -g %d %s 1>/dev/null 2>/dev/null",gid,grp);
	return -WEXITSTATUS(system(cmd));
}

tI4 do_time_get(char* buf)
{	
	time_t tm;
	time(&tm);
	strcpy(buf,ctime(&tm));
	return 0;
}

tI4 do_time_set(const char* buf)
{
 
    struct tm tm;  
    struct timeval tv;  
    time_t timep;  
    sscanf(buf, "%04d%02d%02d%02d%02d%02d", &tm.tm_year,  
        &tm.tm_mon, &tm.tm_mday,&tm.tm_hour,  
        &tm.tm_min, &tm.tm_sec);  
    tm.tm_mon = tm.tm_mon - 1;  
    tm.tm_year = tm.tm_year - 1900;  
  
    timep = mktime(&tm);  
    tv.tv_sec = timep;  
    tv.tv_usec = 0;  
    return settimeofday (&tv, (struct timezone *) 0);	
}

tI4 do_reboot(){
	system("reboot");
	//system("reboot 1>/dev/null 2>/dev/null");
	return 0;
}

tI4 do_halt(){
	system("poweroff");
	//system("poweroff 1>/dev/null 2>/dev/null");
	return 0;
}

tU4 do_serve(tK5_esb* head, /*tK5_net *net,*/ tU4* len, tU1* buf){
	tI4 ret= 0;
	switch (head->service){
/*	case sys_start_vmm:
	case sys_stop_vmm:
		ret=do_vm(buf);
		break;
*/
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
		ret=do_proc_stop(buf);
		break;
	case prc_fork:
	case prc_exit:
	case prc_pause:
	case prc_resume:
	case prc_wait:
		ret=0;
		break;
	
	case thr_start:
	case thr_stop:
	case thr_exit:
	case thr_pause:
	case thr_resume:
	case thr_wait:
		ret=do_threads(buf);
		break;


	case file_create:
		ret=creat(buf,0664);
		break;
	case file_delete:
		ret=unlink(buf);
		break;
	case file_open:
//		buf[*len]='\0';
		ret=open(buf,O_RDWR);
		break;
	case file_read:
		if(head->dst_port>2)
		ret=read(head->dst_port,buf,*len);
		break;
	case file_write:
		if(head->dst_port>2)
		ret=write(head->dst_port,buf,*len);
		break;
	case file_close:
		if(head->dst_port>2)
			ret=close(head->dst_port);
		break;
	case file_rewind:
		ret=lseek(head->dst_port,0,SEEK_SET);
		break;
	case file_state:
	case file_get_mode:
	case dir_get_mode:
		ret=do_get_mode(buf);
		break;
	case file_seek:
		ret=lseek(head->dst_port,atol(buf),SEEK_SET);
		return ret;
	case file_rename:
	case dir_rename:
		ret=do_rename(buf);
		break;
	case file_link:
		ret=do_link(buf);
		break;
	case file_unlink:
		ret=unlink(buf);
		break;
	case file_set_mode:
	case dir_set_mode:
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
	case dir_get_owner:
		ret=do_get_dir_owner(buf);
		break;
	case usr_create:
	case usr_delete:
	case usr_check:
	case usr_get:
		ret=0;
		strcpy(buf,"0");
		break;
	case usr_get_uid:
		ret=do_get_uid(buf);
		break;
	case usr_set_uid:
		ret=do_set_uid(buf);
		break;

	case grp_create:
	case grp_delete:
	case grp_get:
	case grp_check:
		ret=0;
		break;
	case grp_get_gid:
		ret=do_get_gid(buf);
		break;
	case grp_set_gid:
		ret=do_set_gid(buf);
		break;
	
	case time_get:
		ret=do_time_get(buf);
		break;
	case time_set:
		ret=do_time_set(buf);
		break;	
	case time_sleep:
	case time_wait:
		ret=0;
		break;
	
	case sys_reboot:
		ret=do_reboot();
		break;
	case sys_halt:
		ret=do_halt();
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
//	tK5_net net;
	//tU1* buf=NULL;
	tK5_esb esb;
	tU4 ret=-1;
	char buf[8192];
	if(do_read(sockfd,&esb,sizeof(esb))==sizeof(esb) &&
//		do_read(sockfd,&net,sizeof(net))==sizeof(net) &&
		do_read(sockfd,&len,sizeof(len))==sizeof(len)){
//		buf=(tU1*)calloc(1,len);
		if(len>0)
			do_read(sockfd,buf,len);
		ret=do_serve(&esb,&len,buf);
	}
	write(sockfd,&esb,sizeof(esb));
	write(sockfd,&ret,sizeof(ret));
	len=strlen(buf);
	write(sockfd,&len,sizeof(len));
	if(len>0)
		write(sockfd,buf,len);
	close(sockfd);
//	if(buf)
//		free(buf);
}

int main(){
	int sockfd;  
	int reuse=1;
//	daemon(1,0);
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
	//	pthread_t pt;
	//	pthread_create(&pt,NULL,proc_req,conn+NULL);
		proc_req(conn+NULL);
	}
	return 0;
}
