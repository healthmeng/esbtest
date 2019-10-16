//========================================================================
//  K5 微内核操作系统基本测试程序  k5_test_loop.c
//
//  k5_test_loop
//  命令
//
//  该程序循环调用服务编码，测试服务实现的覆盖性。
//
//  创建者：辛耀中   2018-09-06
//  修改者：韦书智   2018-09-07
//  修改者：韦书智   2019-10-12
//===================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "k5_include.h"
#define RED          "\033[0;32;31m" 
#define GREEN        "\033[0;32;32m" 
#define YELLOW       "\033[1;33m" 
#define REST		 "\033[0m"

void show_data(const char* start, char* buf, tU4 len, tI4 ret)
{
	int i;
	if(buf[len-1]!='\0')
		buf[len-1]='\0';
	printf("%s: %d\tbuf:%s\n",start,ret,buf);
/*	for(i=0;i<len;i++)
		printf("%c",buf[i]);

	printf("\n");	
*/
}

int main() //主程序
{
	tU2 svc         =  0 ;	//服务编码
	tI4 ret         =  0 ;	//返回码
	tU4 r_len       =  128 ;	//接收缓冲区长度
	tU1 r_buf[128]  = {0};	//接收缓冲区

	tK5_esb           esb;	//ESB帧结构描述
	tK5_net            to;	//网络地址描述

	int success=0, error=0, unknown=0;

	//----------------------------------------------------
	// 初始化参数和数据
	//----------------------------------------------------
	memset ( &esb,   0, sizeof(esb)   ); //清零ESB帧结构描述
	memset ( &to,    0, sizeof(to)    ); //清零网络地址描述
	memset ( &r_buf, 0, sizeof(r_buf) ); //清零接收缓冲区

	//处理命令行参数
	printf("\n Welcome to k5_test_loop ! let's go !  ...... \n\n" );

	//----------------------------------------------------------
	// 进入主循环： 从键盘获取单字符命令，读：r/R； 写：w/W；关：c/C；
	//----------------------------------------------------------
	for ( svc = sys_reboot; svc < K5_MAX_SVC; svc++ )
	{
		memset(r_buf, 0, sizeof(r_buf));
		switch ( svc )
		{
			case sys_start_vmm:
			case sys_stop_vmm:
				strcpy(r_buf, "test_vm"); // 虚拟机名
				break;
			case sys_mount_fs:
				strcpy(r_buf, "/dev/sdb"); // 文件系统
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , " /media/test"); // 挂载点
				break;
			case sys_unmount_fs:
				strcpy(r_buf, "/media/test"); // 挂载点
				break;
			case sys_login:
				strcpy(r_buf, "test"); // 账号
				strcpy(r_buf + 4, " test1234"); // 密码
				break;
			case sys_logout:
				strcpy(r_buf, "test"); // 账号
				break;
			case sys_svc_grp_reg:
			case sys_svc_grp_del:
			case sys_svc_reg:
			case sys_svc_del:
				strcpy(r_buf, "test_svc"); // 服务（组）名
				break;
			case sys_svc_list:
				break;

			case prc_start:
			case prc_stop:
			case prc_fork:
			case prc_exit:
			case prc_pause:
			case prc_resume:
			case prc_sync:
			case prc_wait:
				strcpy(r_buf, "test_proc"); // 进程名
				break;

			case thr_start:
			case thr_stop:
			case thr_exit:
			case thr_pause:
			case thr_resume:
			case thr_sync:
			case thr_wait:
				strcpy(r_buf, "test_proc"); // 进程名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "2"); // 线程号
				break;

			case file_create:
				strcpy(r_buf, "/tmp/test_f_cre"); // 文件名
				break;
			case file_delete:
				strcpy(r_buf, "/tmp/test_f_del"); // 文件名
				break;
			case file_open:
			case file_close:
			case file_read:
			case file_rewind:
			case file_state:
			case file_get_mode:
				strcpy(r_buf, "/tmp/test_f_file"); // 文件名
				break;
			case file_write:
				strcpy(r_buf, "/tmp/test_f_file"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "input content"); // 写入内容
				break;
			case file_seek:
				strcpy(r_buf, "/tmp/test_f_file"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "5"); // 定位位置
				break;
			case file_rename:
				strcpy(r_buf, "/tmp/test_f_rename_src"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "/tmp/test_f_rename_dst"); // 改名名称
				break;
			case file_link:
				strcpy(r_buf, "/tmp/test_f_link"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "/tmp/test_f_ln"); // 连接
				break;
			case file_unlink:
				strcpy(r_buf, "/tmp/test_f_ln"); // 连接
				break;
			case file_set_mode:
				strcpy(r_buf, "/tmp/test_f_setmode"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "777"); // 访问模式
				break;
			case file_copy:
				strcpy(r_buf, "/tmp/test_f_copy"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "/tmp/test_f_copyed"); // 目标文件名
				break;

			case dir_remove:
				strcpy(r_buf,"/tmp/test_del_dir");
				break;
			case dir_make:
			case dir_list:
			case dir_change:
			case dir_get_owner:
			case dir_get_mode:
				strcpy(r_buf, "/tmp/test_d_dir"); // 目录名
				break;
			case dir_rename:
				strcpy(r_buf, "/tmp/test_d_rename_src"); // 目录名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "/tmp/test_d_rename_dst"); // 改名名称
				break;
			case dir_link:
				strcpy(r_buf, "/tmp/test_d_link"); // 目录名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "/tmp/test_d_ln"); // 连接
				break;
			case dir_unlink:
				strcpy(r_buf, "/tmp/test_d_unln"); // 连接
				break;
			case dir_set_owner:
				strcpy(r_buf, "/tmp/test_d_setowner"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "test_dir"); // 拥有者
				break;
			case dir_set_mode:
				strcpy(r_buf, "/tmp/test_d_setmode"); // 文件名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "777"); // 访问模式
				break;

			case usr_create:
			case usr_delete:
			case usr_get:
			case usr_get_uid:
			case usr_check:
				strcpy(r_buf, "test_u"); // 用户名
				break;
			case usr_set_uid:
				strcpy(r_buf, "test_u"); // 用户名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "1102"); // 用户ID
				break;

			case grp_create:
			case grp_delete:
			case grp_get:
			case grp_get_gid:
			case grp_check:
				strcpy(r_buf, "test_g"); // 用户组名
				break;
			case grp_set_gid:
				strcpy(r_buf, "test_g"); // 用户组名
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "32000"); // 用户组ID
				break;

			case mem_map:
			case mem_unmap:
			case mem_alloc:
			case mem_free:
			case mem_share:
			case mem_unshare:
			case mem_lock:
			case mem_unlock:
			case mem_sem_get:
			case mem_sem_set:
			case mem_sem_on:
			case mem_sem_off:
			case mem_control:
				continue; // 如何测？

			case time_get:
			case time_alarm:
			case time_clear:
			case time_sync:
				break;
			case time_set:
				continue;// to be annotated
				strcpy(r_buf, "20191018090000"); // 时间
				break;
			case time_sleep:
			case time_wait:
				strcpy(r_buf, "test_g"); // 间隔
				break;

			case net_socket:
			case net_close:
			case net_bind:
			case net_control:
			case net_connect:
			case net_accept:
			case net_send:
			case net_recv:
			case net_send_to:
			case net_recv_from:
			case net_select:
				continue; // 如何测？

			case dev_start:
			case dev_stop:
			case dev_open:
			case dev_close:
			case dev_read:
			case dev_state:
			case dev_control:
			case dev_reset:
			case dev_sleep:
			case dev_get:
				strcpy(r_buf, "/dev/sdb1"); // 设备
				break;
			case dev_write:
				strcpy(r_buf, "/dev/sdb2"); // 设备
				strcpy(r_buf + strlen(r_buf), " "); // 空格间隔
				strcpy(r_buf + strlen(r_buf) , "test_v"); // 测试数据
				break;
			case dev_set:
				continue; // 如何测？

			case sys_reboot:
			case sys_halt:
			case usr_set:
			case grp_set:
			default:
				continue; // 暂不执行
		}

		//r_len = strlen(r_buf);
		//    原语      帧头  服务码  目的  长度    缓冲区
		ret = k5_call ( &esb, svc,    &to,  r_len,  r_buf );
		if ( ret < 0 )  {
			if (ret==-0xff){
				unknown++;
				printf(RED);
				printf("k5 [0x%04x] unimplemented : ret = [%d]\n", svc, ret);
				printf(REST);
			}
			else{
				error++;
				printf(YELLOW);
				printf("k5 [0x%04x] call failed: ret = [%d], please check your parameters.\n",svc,ret);
				printf(REST);
			}
		} else {
			success++;
			printf(GREEN);
			printf("k5 [0x%04x] success : ret = [%d]\n", svc, ret);
			show_data( "return", r_buf, r_len, ret );
			printf(REST);
		}
	} //end of for 
	printf("\n Results:\n %d successful calls.\n %d calls implemented but failed on incorrect parameters.\n %d unsupported calls.\n",success,error,unknown);
	printf("\n Thanks to stay with k5_test !  Bye Bye !  \n\n" );
	return 0;
}	/* end of main */

//=========================================================
