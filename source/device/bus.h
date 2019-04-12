#ifndef _BUS_H_
#define _BUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

#define  BUS_LAN   0
#define  BUS_USB   1
#define  METHOD_VISA  0
#define  METHOD_UDP  1

/*
 * 对于网线连接的网关，查找方式有两种： 一种是用VISA方式查找，另一种是用UDP广播方式查找。
 * method: 0:表示使用VISA方式查找；1表示使用UDP方式查找
 * 当method=0时，按照VISA的规范，需要指定总线类型，所以使用bus来传入总线类型。
 * output为输出参数，输出找到的设备信息
 */
int busFindDevice(int bus, char *output, int len, int method);
int busOpenDevice(char * ip, int timeout_ms);
ViSession busOpenSocket(const char *pName, const char *addr, unsigned int port);
int busCloseDevice(ViSession vi);
unsigned int busWrite(ViSession vi, char * buf, unsigned int len);
unsigned int busRead(ViSession vi, char * buf, unsigned int len);
unsigned int busQuery(ViSession vi, char * input, unsigned int inputlen, char* output, unsigned int wantlen);

#ifdef __cplusplus
}
#endif

#endif // !_BUS_H_

