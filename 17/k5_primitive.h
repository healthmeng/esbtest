#ifndef _K5_PRIMITIVE_H
#define _K5_PRIMITIVE_H

#include "k5_include.h"

#define K5_SERV_PORT 3535
#define K5_FIFO      "/tmp/FIFO_K5"
#define DEBUG 1

typedef struct call_send {
	tK5_esb             esb;  //ESB总线数据结构
	tU2                 svc;  //服务编码
	tK5_net             net;  //目的端口号（pid,fd,sock）及网络地址  
	tU4                 num;  //该服务第一个整型参数
	tU1     buf[K5_MAX_BUF];  //该服务其他参数的组合字符串
} k5_call_send;

typedef struct call_recv {
	tI2             ack_err;  //为正数表示确认序列号，为负数表示错误编号；
	tU4                 len;  //发送缓冲区长度,按8字节计，含帧头；
	tU1     buf[K5_MAX_BUF];  //发送缓冲区地址,带服务结果数据；
} k5_call_recv;

void debug( char *str );
void show_data( const char *title, const char *data, int size );

#endif
