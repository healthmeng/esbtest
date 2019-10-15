
//----------------------------------------------------------------
// K5 微内核操作系统基本测试程序  k5_test.c
//----------------------------------------------------------------

#include "k5_inc.h"

#include <stdio.h>	
#include <string.h>
#include <time.h>

int main ( argc, argv )   //主程序
int  argc;            //命令行参数个数
char **argv;          //命令行参数列表

{

tU2   svc;            //服务编码
tI4 	fd;             //文件描述符 .......tU4
tI4   ret;            //返回码 .......tU4
tU4   r_len=512;          //接收缓冲区长度 .......r_len;
tU4   s_len;          //发送缓冲区长度
tU2   i;		//临时变量

tU1   f_name[128];    //全路径文件名
tU1   s_buf[4096];    //发送缓冲区
tU1   r_buf[4096];    //接收缓冲区

tK5_esb_f0 esb;       //ESB帧结构描述
tK5_net     to;       //网络地址描述

//----------------------------------------------------
// 初始化参数和数据
//----------------------------------------------------

memset ( &esb,   0, sizeof(esb)   ); //清零ESB帧结构描述
memset ( &to,    0, sizeof(to)    ); //清零网络地址描述
memset ( &s_buf, 0, sizeof(s_buf) ); //清零发送缓冲区
memset ( &r_buf, 0, sizeof(r_buf) ); //清零接收缓冲区
memset ( &f_name,0, sizeof(f_name)); //清零文件名称

if(argc>1)
	svc   =    atoi( argv[1] );        //获得服务编码
if(argc>2)
  strcpy ( f_name, argv[2] );        //获得全路径文件名
if ( argc ==3 )                    //处理命令行参数
  to.net_level = 0;                  //设置网络层级标识0，仅本处理器服务
else if ( argc==4 ) {
  to.hn[0].dst_addr = inet_addr(argv[3]); //获得网络IP地址  .......to.hn[0].dst_addr = htonl(argv[3];
  to.net_level      = 1;              //设置网络层级标识1，局域网服务

} else {                              //提示命令行用法
  printf("\n Usage: k5_test [ serive_code  file_name  addr_ipv4 ] \n" );
  return 1;
}

printf("\n Welcome to k5_test ! let's go !  ...... \n\n" );


//----------------------------------------------------
// 测试系统服务：打开文件 file_open:  第一个服务码来自命令行
//----------------------------------------------------

//   服务原语  服务号  目的   帧头  标志   文件名      返回文件描述符
fd = k5_call ( svc,    to,   esb,   strlen(f_name),   f_name ); //请求打开文件
if ( fd <= 0 )  {         // 若返回值出错，则打印错误提示，之后程序退出
	printf("k5_call file_open ERR ? ：svc=%d file=%s\n",svc ,f_name );
	return -1; 
} else {
      to.dst_port = fd ;  //将获得的文件描述符保存到目标端口，供后续使用
	printf("k5_call file_open OK !! svc=%d file=%s\n",svc,f_name );
};


//----------------------------------------------------
//测试系统服务：读取文件 file_read:
//----------------------------------------------------
read_test:

svc = file_read;   //设置服务编码
ret = 0;

//    服务原语  服务号  目的   帧头  长度   缓冲区      返回读取长度
ret = k5_call ( svc,    to,   esb,  r_len, r_buf ); //请求读取文件

if ( ret < 0 )  {        // 若返回值出错，则打印错误提示，之后程序退出 .......ret<=0 
	printf("k5_call file_read ERR ? ：svc=%d ret=%d\n", svc, ret );
	goto close_and_exit; 
} else {
	printf("k5_call file_read OK !! ：svc=%d len=%d\n", svc, ret );
}


//----------------------------------------------------
//测试系统服务：写入文件 file_write:
//----------------------------------------------------
write_test:

svc = file_write;            //设置服务编码
ret = 0;

for (i=0; i<256; i++ ) {     //设置发送缓冲区数据
    s_buf[i]     =  i;       //前256个数据从0递增到255
    s_buf[i+256] =  255-i;   //后256个数据从255递减到0
};
s_len = 512; // 发送数据长度

//    服务原语  服务号  目的   帧头  长度   缓冲区      返回写入长度
ret = k5_call ( svc,    to,   esb,  s_len, s_buf ); //请求写入文件

if ( ret < 0 )  {        // 若返回值出错，则打印错误提示，之后程序退出  ---------ret<=0
	printf("k5_call file_write ERR ? ：svc=%d ret=%d\n", svc, ret );
	goto close_and_exit; 
} else {
	printf("k5_call file_write OK !! ：svc=%d len=%d\n", svc, ret );
};


//----------------------------------------------------
//测试系统服务：关闭文件 file_close:
//----------------------------------------------------

close_and_exit:

svc = file_close;   //设置服务编码
ret = 0;

//    服务原语  服务号  目的   帧头  长度   缓冲区     返回状态
ret = k5_call ( svc,    to,   esb,  0,     NULL ); //请求关闭文件

if ( ret < 0 )  {        // 若返回值出错，则打印错误提示，之后程序退出 --------ret<=0
	printf("k5_call file_close ERR ? ：svc=%d ret=%d\n", svc, ret );
	return -4;
} else {
	printf("k5_call file_close OK !! ：svc=%d ret=%d\n", svc, ret );
};


//----------------------------------------------------
//测试系统服务：关闭文件 file_close:
//----------------------------------------------------

printf("\n Thanks to stay with k5_test !  Bye Bye !  \n\n" );

return 0;
}	/* end of main */
//=========================================================




