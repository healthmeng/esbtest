
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

#include "k5_include.h"

void print_para(tK5_esb *esb, tK5_net *net)
{
	printf("Parameter in esb head:\n");
	printf("  head:		[%u]\n", esb->head);
	printf("  page:		[%u]\n", esb->page);
	printf("  size:		[%u]\n", esb->size);
	printf("  service:		[%u]\n", esb->service);
	printf("  primitive:	[%u]\n", esb->primitive);
	printf("  dst_port:		[%u]\n", esb->dst_port);
	printf("  drc_port:		[%u]\n", esb->src_port);
	printf("Parameter in net:\n");
	printf("  net_level:	[%u]\n", net->net_level);
	printf("  name_len:		[%u]\n", net->name_len);
	printf("  dst_port:		[%u]\n", net->dst_port);
	printf("  src_port:		[%u]\n", net->src_port);
	printf("  dst_addr:		[%u]\n", net->hn[0].dst_addr);//...
	printf("  src_addr:		[%u]\n", net->hn[0].src_addr);
	printf("  net_name:		[%s]\n", net->net_name);

};    // end of print_para

char* get_local_ip()
{
	char hname[128];
	struct hostent *hent;

	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);

	return inet_ntoa(*(struct in_addr*)(hent->h_addr_list[0]));

};     //end of get_local _ip

int main ( argc, argv )   //主程序
int  argc;            //命令行参数个数 
char **argv;          //命令行参数列表
{
	tU2   svc;            //服务编码
	tI2   fd;             //文件描述符 ...............
	tI4   ret;            //返回码  .................
	tU4   r_len;          //接收缓冲区长度
	tU4   s_len;          //发送缓冲区长度
	tU2   i;              //临时变量
	tU2   next;           //下一部操作

	tU1   f_name[128];    //全路径文件名
	tU1   s_buf[4096];    //发送缓冲区
	tU1   r_buf[4096];    //接收缓冲区

	tK5_esb    esb;       //ESB帧结构描述
	tK5_net     to;       //网络地址描述

	//----------------------------------------------------
	// 初始化参数和数据
	//----------------------------------------------------
	memset ( &esb,   0, sizeof(esb)   ); //清零ESB帧结构描述
	memset ( &to,    0, sizeof(to)    ); //清零网络地址描述
	memset ( &s_buf, 0, sizeof(s_buf) ); //清零发送缓冲区
	memset ( &r_buf, 0, sizeof(r_buf) ); //清零接收缓冲区
	memset ( &f_name,0, sizeof(f_name)); //清零文件名称

	if ( argc < 3 || argc > 4 )
	{                              //提示命令行用法
	  printf("Usage: k5_test serive_code  file_name [addr_ipv4] \n" );
	  exit(1);
	}
	else
	{
		printf("argc = %d\n", argc);
	}

	//处理命令行参数
	svc   =    atoi( argv[1] );        //获得服务编码
	strcpy ( f_name, argv[2] );        //获得全路径文件名
	printf( "svc = [%d],  f_name = [%s]\n", svc, f_name );
	if ( argc == 3 )
	{
		to.net_level = 0;              //设置网络层级标识0，仅本处理器服务
	} else {
		to.net_level = 1;              //设置网络层级标识1，局域网服务
		to.hn[0].dst_addr = inet_addr(argv[3]); //获得网络IP地址
		to.hn[0].src_addr = inet_addr(get_local_ip());
		printf( "local ip = [%d], remote ip = [%d]\n", to.hn[0].src_addr, to.hn[0].dst_addr );
	}

	printf("\n Welcome to k5_test ! let's go !  ...... \n\n" );

	//----------------------------------------------------
	// 测试系统服务：打开文件 file_open;  第一个服务码来自命令行
	//----------------------------------------------------
	esb.head      = K5_H0;	       //0型帧
	esb.page      = 0;             //长度按8字节计数
	esb.primitive = K5_CALL;       //请求调用
//	svc           = file_read;     //读取文件

	print_para(&esb, &to);

	//     原语    帧头   打开文件  目的    参数   文件名     
	fd = k5_call ( &esb,  svc,      &to,    strlen(f_name),     f_name ); 
 
	if ( fd < 0 )  {  // 若返回值出错，则打印错误提示，之后程序退出....
	  printf("k5 file_open ERR ? ：svc=%d file=%s\n",svc,f_name );
	  exit (-1);
	} else {
	  to.dst_port = fd ;  //将获得的文件描述符保存到目标端口，供后续使用
	  printf("k5 file_open OK !! svc=%d file=%s\n",svc, f_name );
	};

	//----------------------------------------------------------
	// 进入主循环： 从键盘获取单字符命令，读：r/R； 写：w/W；关：c/C；
	//----------------------------------------------------------
	while (1)
	{
	printf("\n What's next? r/R :read, w/W :write, c/C :close ::\n" );
	   next = 0;
	   next = getchar();     //从标准数据流（键盘）获取1个字符
	   getchar();

	   if ( next == 'r' || next == 'R' ) {    //如果为read命令
	   //----------------------------------------------------
	   //测试系统服务：读取文件 file_read;
	   //----------------------------------------------------
read_loop:
		ret   = 0;
		r_len = 128;
		svc   = file_read;	   //读取文件

		print_para(&esb, &to);

		//       原语   帧头   读文件  目的    长度    缓冲区  
		ret = k5_call ( &esb,  svc,    &to,    r_len,  r_buf  ); 
		if ( ret < 0 )  {  
		   printf("k5 file_read ERR ? : svc=%d ret=%d\n",svc,ret);
		   break;           // 中断循环，关闭文件，退出程序。
		} else if(ret>0){
		   printf("k5 file_read OK !! : svc=%d len=%d\n",svc,ret);
		   for ( i = 0; i < ret; i++ ) printf("%x ", r_buf[i]); 
		   printf("\n\n");
			if ( next == 'R' ) goto read_loop;  //大写R表示读到文件尾
		}else{
			printf("Already end of file");
		}

	   } else if ( next=='w' || next=='W' ) { //如果为write命令

	   //----------------------------------------------------
	   //测试系统服务：写入文件 file_write:
	   //----------------------------------------------------
//write_loop:
		svc = file_write;            //设置服务编码
		ret = 0;

		for (i = 0; i < 256; i++ ) { //设置发送缓冲区数据
		    s_buf[i]     =  i;       //前256个数据从0递增到255
		    s_buf[i+256] =  255-i;   //后256个数据从255递减到0
		}
		s_len = 512;                 // 发送数据长度

		print_para(&esb, &to);

		//    原语      帧头   写文件  目的    长度    缓冲区  
		ret = k5_call ( &esb,  svc,    &to,    s_len,  s_buf ); 

		if ( ret < 0 )       // 返回出错，打印错误
		{   
		    printf("k5 file_write ERR ? svc=%d ret=%d\n",svc,ret);
		    break;            // 中断循环，关闭文件，退出程序。        
		} else {
		   printf("k5 file_write OK !! svc=%d len=%d\n",svc,ret);
		   for ( i = 0; i < s_len; i++ )
			  printf("%02x ", s_buf[i] ); //打印内容数据
		   printf("\n\n");

		}//end of if (ret<=0)

	   // if ( next == 'W' ) goto write_loop;  //暂仅写1次！

	   } else if ( next == 'c' || next == 'C' )  { //如果为close命令
		break;

	   } else  {  continue; }  //end of if (next)
		
	}   //end of while 

	//----------------------------------------------------
	//测试系统服务：关闭文件 file_close:
	//----------------------------------------------------
	svc = file_close;   //设置服务编码
	ret = 0;

	print_para(&esb, &to);

	//     原语     帧头   关闭文件  目的   帧头   长度    缓冲区   
	ret = k5_call ( &esb,    svc,    &to,   0,     NULL );//............... 

	if ( ret < 0 )  {      
	  printf("k5 file_close ERR ? : svc=%d ret=%d\n", svc, ret );
	  exit(-4);            // 返回出错，打印错误，程序退出
	} else {
	  printf("k5 file_close OK !! : svc=%d ret=%d\n", svc, ret );
	}

	printf("\n Thanks to stay with k5_test !  Bye Bye !  \n\n" );
	exit(1);

}	/* end of main */

//=========================================================
