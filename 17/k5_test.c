//============================================================
// K5 微内核操作系统基本测试程序  k5_test.c
//
// 命令使用方法：
//         k5_test  IPaddr     port  serive_code  ...
//         命令     网络地址   端口  服务编码     参数列表
//
//  创建者：辛耀中   2018-09-06
//  修改者：韦书智   2018-09-07
//  修改者：韦书智   2019-10-15
//============================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "k5_primitive.h"
#include "k5_include.h"

int main ( int argc, char **argv )	//主程序
{
	tU2     svc              =   0;	//服务编码
	tU4     ret              =   0;	//返回码
	tU4     len              =   0;	//接收缓冲区长度
	tU1     buff[K5_MAX_BUF] = {0};	//接收缓冲区
	tK5_net to;				        //网络地址描述
	tK5_esb esb;	                //ESB帧结构描述

	if ( argc < 4 )
	{//提示命令行用法
		printf("Usage: k5_test IPaddr port serive_code [parameters ... ] \n" );
		return 0;
	}

	//----------------------------------------------------
	// 初始化参数和数据
	//----------------------------------------------------
	memset ( &esb,   0, sizeof(esb)   ); //清零ESB帧结构描述
	memset ( &to,    0, sizeof(to)    ); //清零网络地址描述
	memset ( &buff,  0, sizeof(buff)  ); //清零接收缓冲区

	//处理命令行参数
	to.net_level = strcmp( argv[1], "0.0.0.0" ) ? 2 : 1;  //设置网络级数
	to.hn[0].dst_addr = inet_addr(argv[1]);               //获得网络IP地址
	to.dst_port = atoi(argv[2]);                          //目的端口
	svc = atoi( argv[3] );                                //获得服务编码

	tI1 *p = buff;
	tI4  i = 0;
	for ( i = 4; i < argc; i++ )
	{
		memcpy(p, argv[i], strlen(argv[i]));
		p += strlen(argv[i]);
		*p = ' ';
		p++;
	}
	*p = '\0';
	len=strlen(buff);
//	printf("\n Welcome to k5_test ! let's go !  ...... \n\n" );

	//    原语      帧头  服务码  目的  长度    缓冲区
	ret = k5_call ( &esb, svc,    &to,  len,    buff );
	if ( ret <= 0 )
	{
		printf("k5 [%d] unimplemented : ret = [%d]\n", svc, ret);
	} else {
		k5_call_recv *p=(k5_call_recv*)buff;
		printf("k5 [%d] success : ret = [%d]\n", svc, ret);
		show_data( "return", p->buf, p->len );
//		printf("%s\n", buff);
	}

//	printf("\n Thanks to stay with k5_test !  Bye Bye !  \n\n" );
	return 0;
}
