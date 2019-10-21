
//========================================================================
//
// K5 微内核操作系统程序访问原语  k5_primitive.c
//
//几点说明：
//
//注1：原来的四个原语（同步发送SS、同步接收RS、异步发送SA、异步接收RA），
//     逻辑上是完备的，但名称太抽象，参数也需要优化，建议名称改为：
//     同步发送SS == call同步调用,    同步接收RS == wait同步等待,  
//     异步发送SA == send异步发送,    异步接收RA == reply异步应答==ack+nak.
//
//注2：将原来的两个异步原语换为意义更为明确的异步应答和异步发送，主要是为了
//     配合零拷贝技术，需要尽快确认以便释放送端缓冲区，也使接收端继续等待。
//
//注3：该服务原语有两种用法，一是供新开发的应用程序直接调用，二是由新CLIB
//     中POSIX系统服务函数间接调用，原应用程序不需修改，但不支持K5新功能。
//
//注4  为四原语设置两个通用参数（一个整型、一串型），便于原语组装ESB帧，
//     若还需传递其他参数，可直接通过ESB数据结构传递。 2018-09-05增加的
//     各个系统服务的特定参数结构不再需要了，使用更为方便。2018-09-06.
//
//注5：在ESB帧结构统一改为8字节数组后（2018-09-08），所有原语可以使用
//     单一esb帧体结构，当需要网络地址、序列号、错误号的时候，可直接用
//     tK5_eh1和tK5_ehn映射（cast）到body[]的相应位置，并通过head头部
//     扩展长度指明。（2018-09-08修改）
//
//注6：在服务编码中增加该服务的参数个数，占3比特，需要由服务提供者在服务注
//     册时提供，在服务向量表中保存。（2018-09-08）
//
//注7：服务原语为各类服务预留两个通用参数位置：1个整型参数和1个字符串型
//     参数；当参数为线程标识pid、文件描述符fd、网络套接字sock时，可直接
//     用ESB目的端口dst_port传递，不占用整型参数位置（8字节，好对齐）。
//    （2018-09-08）
//
//注8：当两个通用参数位置不够时，可以通过字符串型通用参数进行扩展，将多个
//     字符串合并为一字符串，之间用空格分隔；当用字符串表示十进制或十六
//     进制数时，其首为必须为数字；在一个字符串中描述的参数个数，须与服务
//     向量表中的参数个数相匹配。（2018-09-08）
//
//注9：服务结果及返回数据的结构格式，由服务提供方确定，可采用静态和动态
//     方式。静态方式需要预先定义数据结构，效率高，但不灵活；动态方式
//     采用tK5_gdd动态描述复杂数据结构，很灵活，效率低。（2018-09-08）
//
//注10：将所有原语都一致的参数esb,固定为第1个参数，最后两个参数固定为
//      整型长度、缓冲区地址指针，便于记忆 。  （2018-09-11）
//
//
//
//  创建：辛耀中   2018-09-05
//  修改：韦书智   2018-09-07
//  修改：辛耀中   2018-09-09
//===================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "k5_primitive.h"

void debug( char *str )
{
#if DEBUG
	printf("\n#-#-#  %s  #-#-#\n", str);
#endif
	return;
}

void show_data( const char *title, const char *data, int size )
{
	int i = 0;
	char tmp[9] = {0};
	char output[3] = {0};
	char buf[17] = {0};

	printf("@@@@@@@@@@@@  show data of [%s] : start  @@@@@@@@@@@@\n", title);
	for ( i = 0; i < size; i++ )
	{
		if ( i % 16 == 0 )
		{
			printf("%04x  ", i);
			memset(buf, 0, sizeof(buf));
		}
		memset(tmp, 0, sizeof(tmp));
		memset(output, 0, sizeof(output));
		sprintf(tmp, "%08x", data[i]);
		strncpy(output, tmp + 6, 2);
		printf("%s ", output);
		if ( i % 8 == 7 )
			printf(" ");
		buf[i % 16] = ( data[i] >= 0x20 && data[i] <= 0x7e ) ? data[i] : '.';
		if ( i % 16 == 15 )
			printf(" %s\n", buf);
	}
	if ( size % 16 != 0 )
	{
		for ( i = 0; i < 16 - size % 16; i++ )
			printf("   ");
		if ( size % 16 < 8 )
			printf(" ");
		printf("  %s\n", buf);
	}
	printf("@@@@@@@@@@@@  show data of [%s] : end  @@@@@@@@@@@@\n\n", title);
}

//-------------------------------------------------------------------------
// 同步调用请求原语（原SS），一直等待对方应答后，才释放发送缓冲区
//-------------------------------------------------------------------------

tU4 k5_call (
	tK5_esb     *esb,  //ESB总线数据结构
	tU2      service,  //服务编码
	tK5_net      *to,  //目的端口号（pid,fd,sock）及网络地址  
	tU4        p_int,  //该服务第一个整型参数
	tU1       *p_str   //该服务其他参数的组合字符串
)
{
	tU4                      ret = 0;
	tI4                       fd = 0;
	tI1          buf[K5_MAX_BUF] = { 0 };
	tI1       strerr[K5_MAX_BUF] = { 0 };
	tI1                       *p = NULL;
	tU2                      ack = 0;
	k5_call_send        send_msg;
	k5_call_recv        recv_msg;
	struct sockaddr_in  scktaddr;

	if ( service <= 0 || service >= K5_MAX_SVC ) return 0;//return ( ERR_SERVICE );
	if ( esb == NULL  || to == NULL            ) return 0;//return ( ERR_POINTER );

	memset ( esb,       0, sizeof(tK5_esb)      );  //清零ESB帧结构
	memset ( buf,       0, sizeof(buf)          );  //清零缓冲区
	memset ( strerr,    0, sizeof(strerr)       );  //清零错误信息
	memset ( &send_msg, 0, sizeof(k5_call_send) );  //清零发送信息
	memset ( &recv_msg, 0, sizeof(k5_call_recv) );  //清零接受信息

	esb->head      = to->net_level;       //设置帧扩展长度
	esb->service   = service;             //设置服务编码
	esb->primitive = K5_CALL;             //设置服务原语
	esb->dst_port  = to->dst_port;        //设置目的端口
	esb->src_port  = to->src_port;        //设置源端口
	esb->size      = esb->head + 1 + (tU1)(p_int/8) + 1;  //帧总长 = 头长+体长

	memcpy( &send_msg.esb, (char *)esb, sizeof(tK5_esb) );
	memcpy( &send_msg.net, (char *)to,  sizeof(tK5_net) );
	if ( p_int )
	{
		send_msg.num = p_int;
		memcpy( &send_msg.buf, p_str, p_int );
	}
	show_data( "send_msg", (const char *)&send_msg, sizeof(send_msg) );

/*	if ( to->net_level == 1 ) //本地通信
	{
		if ( ( fd = open( K5_FIFO, O_RDWR ) ) < 0 )
		{
			sprintf(strerr, "open fifo error: %s(errno: %d)", strerror(errno), errno);
			debug(strerr);
			return 0;
		}

		if ( write( fd, buf, sizeof(buf) ) < 0 )
		{
			sprintf(strerr, "write fifo error: %s(errno: %d)", strerror(errno), errno);
			debug(strerr);
			return 0;
		}

		memset(buf, 0, sizeof(buf));
		while ( 1 )
		{
			usleep(1000);
			if ( read( fd, buf, sizeof(buf) ) > 0 )
				break;
			debug("k5_call read ok");
		}
		close( fd );//关闭
	} else */{ //网络通信
		if( (fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			sprintf(strerr, "create socket error: %s(errno: %d)\n", strerror(errno), errno);
			debug(strerr);
			return 0;
		}

		memset(&scktaddr, 0, sizeof(scktaddr));
		scktaddr.sin_family      = AF_INET;
		scktaddr.sin_addr.s_addr = to->hn[0].dst_addr;
		scktaddr.sin_port        = htons(K5_SERV_PORT);

		if( connect( fd, (struct sockaddr*)&scktaddr, sizeof(scktaddr) ) == -1 )
		{
			sprintf(strerr, "connect() error: %s(errno: %d)\n", strerror(errno), errno);
			debug(strerr);
			close(fd);
			return 0;
		}

		send( fd, &send_msg, sizeof(send_msg), 0 );
		debug( "send ok" );
		ret = recv( fd, p_str, sizeof(k5_call_recv), 0 );
		close( fd );//关闭socket
	//	memcpy( p_int, buf, )
	}

	// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了！
/*
  	_asm {   //此处与计算机架构密切相关，由各家自行实现！！！
       	mov r0, esb;    //将ESB帧首地址存入寄存器r0;
        int 21;         //软中断指令陷入内核；
  	}
*/
/*
	memcpy( (char *)&recv_msg, buf, sizeof(k5_call_recv) );
	memcpy( (char *)&ret, buf + 2, 4);
	memset( p_str, 0, p_int );
	memcpy( p_str, buf + 2 + 4, ret);
*/
//	debug( "k5_call over!" );
	return ret;
//	return recv_msg.len;
}   //end of k5_call

//-------------------------------------------------------------------------
//同步等待接收原语（原RS)，等待接收服务请求或服务确认;
//-------------------------------------------------------------------------
tU4 k5_wait ( tK5_esb *esb, tK5_net *from, tU4 *r_len, tU1 *r_buf )
{
	debug( "enter k5_wait()" );
	tI1          buf[K5_MAX_BUF] = { 0 };
	tI1       strerr[K5_MAX_BUF] = { 0 };
	tI4                       fd = 0;
	tI1                       *p = NULL;

	if ( mkfifo( K5_FIFO, 0644 ) < 0 && errno != EEXIST )
	{
		sprintf( strerr, "mkfifo() error: %s(errno: %d)\n", strerror(errno), errno );
		debug( strerr );
		return 0;
	}

	if ( ( fd = open( K5_FIFO, O_RDONLY ) ) == -1 )
	{
		sprintf( strerr, "open() error: %s(errno: %d)\n", strerror(errno), errno );
		debug( strerr );
		return 0;
	}

	memset( buf, 0, sizeof(buf) );
	while ( 1 )
	{
		if ( read( fd, buf, sizeof(buf) ) > 0 )
			break;
	}
	close( fd );

	p = buf;
	memcpy( esb, p, sizeof(tK5_esb) );
	p += sizeof(tK5_esb);
	memcpy( from, p, sizeof(tK5_net) );
	p += sizeof(tK5_net);
	memcpy((char *)r_len, p, 4);
	p += 4;
	memcpy( r_buf, p, *r_len );

	return 1;
}

//-------------------------------------------------------------------------
//异步应答原语（原SA,ack+nak)，确认服务请求，并返回服务结果，然后继续等待;
//-------------------------------------------------------------------------

tU4  k5_reply  (     //reply原语不将目的端口地址与源端口地址对调！！！
     tK5_esb  *esb,  //ESB总线帧数据结构
     tI2   ack_err,  //为正数表示确认序列号，为负数表示错误编号；
     tU4     s_len,  //发送缓冲区长度,按8字节计，含帧头；
     tU1    *s_buf   //发送缓冲区地址,带服务结果数据；
)
{
	debug( "enter k5_reply()" );
	tI1  buf[K5_MAX_BUF]    = {0};
	tI1  strerr[K5_MAX_BUF] = {0};
	tI4  fd                 =  0 ;

	memset( buf, 0, sizeof(buf) );
	memcpy( buf, (char *)&ack_err, 2);
	if ( s_len > 0 )
	{
		memcpy( buf + 2, (char *)&s_len, 4);
	}
	if ( s_buf != NULL )
	{
		memcpy( buf + 6, s_buf, s_len);
	}

	if ( ( fd = open( K5_FIFO, O_WRONLY ) ) < 0 )
	{
		sprintf(strerr, "open fifo error: %s(errno: %d)", strerror(errno), errno);
		debug(strerr);
		return 0;
	}

	if ( write( fd, buf, 6 + s_len ) < 0 )
	{
		sprintf(strerr, "write fifo error: %s(errno: %d)", strerror(errno), errno);
		debug(strerr);
		return 0;
	}
	close(fd);

	usleep(1500);
	return 1;
}

//-------------------------------------------------------------------------
// 同步调用请求服务原语（原SS），一直等待对方应答后，才释放发送缓冲区
//-------------------------------------------------------------------------
#if 0
tU4  k5_call (   
     tK5_esb     *esb,  //ESB总线数据结构            
     tU2      service,  //服务编码
     tK5_net      *to,  //目的端口号（pid,fd,sock）及网络地址  
     tU4        p_int,  //该服务第一个整型参数
     tU1       *p_str   //该服务其他参数的组合字符串 
     )
{
	tK5_svc  svc;    //服务编码的展开结构，14比特展开4字节
	tK5_ehn  ehn;    //ESB扩展网络地址，8字节
	tK5_pint pint;   //整型参数描述结构，8字节

	if ( service <= 0 || service > MAX_SVC  )
		return ( -1 );
	if ( esb == NULL  || to == NULL )
		return ( -2 );

	memset ( esb,  0,  sizeof(tK5_esb) ）;     //清零ESB帧结构

	esb->primitive = K5_CALL;              //设置服务原语
	esb->service   = service;              //设置服务编码
	esb->head      = to->net_level;        //设置帧扩展长度
	esb->dst_port  = to->dst_port;         //设置目的端口
	esb->dst_port  = to->dst_port;         //设置源端口

	if （to->net_level >= 1 )
	{   }      //设置扩展头body[0],已清零

	if （to->net_level >= 2 )
	{ 
		ehn.dst_addr = to->fn[0].dst_addr; //第一级网络目的地址
		ehn.src_addr = to->fn[0].src_addr; //第一级网络源地址
		esb->body[1] = (tU8)ehn;           //cast映射到body[1]
	}

	if （to->net_level >= 3 )
	{ 
		ehn.dst_addr = to->fn[1].dst_addr; //第二级网络目的地址
		ehn.src_addr = to->fn[1].src_addr; //第二级网络源地址
		esb->body[2] = (tU8)ehn;           //cast映射到body[2]   
	}

	svc = (tK5_svc)service;                //展开14比特的服务编码

	if ( svc.pnum >=1 )
	{
	   pint.p_num = svc.pnum;              //设置该服务的参数个数
	   pint.pi4u = p_int;                  //保存整型参数

		if ( ps_buf ！= NULL )
		{            //设置字符串型参数（若非空）
			pint.p_len = strlen ( p_str );   //获取串长度，然后定长串拷贝
			strncpy ( &（tU1）esb->body[to->level+1],pint.p_len, p_str ); 
		} else {
			pint.p_len = 0;
			esb->body[to->level] = (tU8)pint; //设置整型参数和串长度
		}// end of if ( ps_buf ！= NULL)   
	}// end of if ( svc.pnum >=1 )

	esb->size = esb->head+1 + (tU1)(len/8)+1;  //帧总长 = 头长+体长

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了！
//以下与处理器架构密切相关，由各家自行实现！此处仅示意！

  _asm {   
        mov r0, esb;    //将ESB帧首地址存入寄存器r0;
        int 21;         //软中断指令陷入内核；
  }

	return (1);
}//end of k5_call
 
//-------------------------------------------------------------------------
//同步等待接收原语（原RS)，等待接收服务请求或服务确认;
//-------------------------------------------------------------------------

tU4  k5_wait  ( 
     tK5_esb        esb,  //ESB总线数据结构
     tK5_net      *from,  //指定期望的端口号及网络地址 
     tU4          r_len,  //接收缓冲区长度，使用零拷贝时不用
     tU1         *r_buf   //接收缓冲区地址，使用零拷贝时不用
     ) 
{

if ( esb == NULL  )  return ( -1 );

memset ( esb,  0,  sizeof(esb) ）;        //清零ESB帧结构

esb->primitive = K5_WAIT;                 //设置服务原语

if （r_len > 0 ) esb->size = r_len;       //若非0则设置缓冲区长
else             esb->size = MAX_ESB_LEN; //若为0则设置最大长度

if （r_buf != NULL )                      //设置自己的缓冲区
        esb->body[esb->head+1] = r_buf;   //用零拷贝时不需要

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了！
//以下与处理器架构密切相关，由各家自行实现！此处仅示意！

  _asm {   
        mov r0, esb;    //将ESB帧首地址存入寄存器r0;
        int 21;         //软中断指令陷入内核；
  }

  return (1);

}   //end of k5_wait

//-------------------------------------------------------------------------
//异步应答原语（原SA,ack+nak)，确认服务请求，并返回服务结果，然后继续等待;
//-------------------------------------------------------------------------

tU4  k5_reply  (     //reply原语不将目的端口地址与源端口地址对调！！！
     tK5_esb  *esb,  //ESB总线帧数据结构
     tU2   ack_err,  //为正数表示确认序列号，为负数表示错误编号；
     tU4     s_len,  //发送缓冲区长度,按8字节计，含帧头；
     tU1    *s_buf   //发送缓冲区地址,带服务结果数据；
     )
{

tK5_eh1   eh1;    //扩展头结构，8字节

if ( esb == NULL  )  return ( -1 );

esb->primitive = K5_REPLY;          //设置服务原语

if ( esb->head > 0 )  {             //若存在扩展头部
      eh1 = (tK5_eh1)esb->body[0];  //获取扩展头部
      eh1.ack_seq = ack_err;        //确认已收到的帧序号
}；   //end of if ( esb->head > 0 )  

if （s_len > 0 && s_buf != NULL )   //若有服务结果数据送回
      memncpy (&esb->body[esb->head+1], s_len*8, s_buf);

eh1.snd_seq  = 0;                   //设置本侧发送帧序号（不一定为0）
esb->body[0] = (tU8)eh1;            //设置扩展头部 

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了！
//以下与处理器架构密切相关，由各家自行实现！此处仅示意！

  _asm {   
        mov r0, esb;    //将ESB帧首地址存入寄存器r0;
        int 21;         //软中断指令陷入内核；
  }

  return (1);

} // end of k5_reply

//-------------------------------------------------------------------------
//异步发送原语（原SA)，主动发送数据或通知信号，然后继续等待;
//-------------------------------------------------------------------------

tU4  k5_send  ( 
     tK5_esb     *esb,  //ESB总线数据结构   
     tU2      service,  //服务编码
     tK5_net      *to,  //目的端口号（pid,fd,sock）及网络地址  
     tU4        s_len,  //发送数据长度（按8个字节长字计数）
     tU1       *s_buf   //发送数据缓冲区地址 
     )
{

tK5_svc  svc;    //服务编码的展开结构，14比特展开4字节
tK5_ehn  ehn;    //ESB扩展网络地址，8字节


if ( service <= 0 || service > MAX_SVC  )  return ( -1 );
if ( esb == NULL  || to == NULL         )  return ( -2 );

memset ( esb,  0,  sizeof(esb) ）;     //清零ESB帧结构

esb->primitive = K5_SEND;              //设置服务原语
esb->service   = service;              //设置服务编码
esb->head      = to->net_level;        //设置帧扩展长度
esb->dst_port  = to->dst_port;         //设置目的端口
esb->dst_port  = to->dst_port;         //设置源端口

if （to->net_level >= 1 ) {   }；      //设置扩展头body[0],已清零

if （to->net_level >= 2 ) { 
    ehn.dst_addr = to->fn[0].dst_addr; //第一级网络目的地址
    ehn.src_addr = to->fn[0].src_addr; //第一级网络源地址
    esb->body[1] = (tU8)ehn;           //cast映射到body[1]
};

if （to->net_level >= 3 ) { 
    ehn.dst_addr = to->fn[1].dst_addr; //第二级网络目的地址
    ehn.src_addr = to->fn[1].src_addr; //第二级网络源地址
    esb->body[2] = (tU8)ehn;           //cast映射到body[2]   
};

if （s_len > 0 && s_buf != NULL )      //若有数据要发送
      memncpy (&esb->body[esb->head+1], s_len*8, s_buf);


esb->size = esb->head+1 + s_len;       //帧总长 = 头长+体长



//已经准备好ESB帧头和帧体，可以通过软中断陷入内核了！
//以下与处理器架构密切相关，由各家自行实现！此处仅示意！

  _asm {   
        mov r0, esb;    //将ESB帧首地址存入寄存器r0;
        int 21;         //软中断指令陷入内核；
  }

  return (1);

} // end of k5_send
#endif
//===============================================================

