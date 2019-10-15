
//========================================================================
// K5 微内核操作系统基本测试程序  k5_test.c
//
// 命令使用方法，当仅2个参数时表示本地访问，当有3个参数时表示网络访问。
//
//         k5_test  serive_code  file_name   [ addr_ipv4 ] 
//          命令     服务编码     文件名称    网络地址（可选）
//
// 该程序进入循环后，须从键盘获取下一步的单字符命令：
//
//      当键盘输入字符为：
//                         'r'  : 表示读一次文件
//                         'R'  : 表示持续读到文件末尾
//                         'w'  : 表示写一次文件
//                         'W'  : 表示写多次文件，暂不用
//                         'c'  : 表示关闭文件并退出程序
//                         'C'  : 表示关闭文件并退出程序
//
//  创建者：辛耀中   2018-09-06
//  修改者：韦书智   2018-09-07
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

int main ( int argc, char **argv )	//主程序
{
	tU2 svc         = 0;	//服务编码
	tU4 ret         = 0;	//返回码
	tU4 r_len       = 0;	//接收缓冲区长度
	tU1 r_buf[4096] = {0};	//接收缓冲区

	tK5_esb esb;			//ESB帧结构描述
	tK5_net to;				//网络地址描述

	//----------------------------------------------------
	// 初始化参数和数据
	//----------------------------------------------------
	memset ( &esb,   0, sizeof(esb)   ); //清零ESB帧结构描述
	memset ( &to,    0, sizeof(to)    ); //清零网络地址描述
	memset ( &r_buf, 0, sizeof(r_buf) ); //清零接收缓冲区

	if ( argc < 2 || argc > 3 )
	{//提示命令行用法
		printf("Usage: k5_test serive_code [addr_ipv4] \n" );
		return -1;
	}
	printf("\n Welcome to k5_test ! let's go !  ...... \n\n" );

	//处理命令行参数
	svc = atoi( argv[1] );        //获得服务编码
	if ( argc == 2 )
	{
		to.net_level = 0;              //设置网络层级标识0，仅本处理器服务
	} else {
		to.net_level = 1;              //设置网络层级标识1，局域网服务
		to.hn[0].dst_addr = inet_addr(argv[2]); //获得网络IP地址
	}

	r_len = 128;
	//    原语      帧头  服务码  目的  长度    缓冲区
	ret = k5_call ( &esb, svc,    &to,  r_len,  r_buf );
	if ( ret <= 0 )  {
		printf("k5 [%d] unimplemented : ret = [%d]\n", svc, ret);
	} else {
		printf("k5 [%d] success : ret = [%d]\n", svc, ret);
		for ( i = 0; i < ret; i++ )
			printf("%x ", r_buf[i]); 
		printf("\n\n");
	}

	printf("\n Thanks to stay with k5_test !  Bye Bye !  \n\n" );
	return 0;
}	/* end of main */

//=========================================================
