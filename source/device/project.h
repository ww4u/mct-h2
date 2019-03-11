#ifndef MR_PROJECT_H
#define MR_PROJECT_H
#include "bus.h"
#include "export.h"
/*
* 设置系统为工程模式
* vi :visa设备句柄
* state: 0->OFF| 1->ON
* 返回值：0表示执行成功；－1表示执行失败
* 说明: 只有在工程模式下,才允许读取IO状态
*/
EXPORT_API int CALL mrgSetProjectMode(ViSession vi, int state);
/*
* 查询外部IO的状态
* vi :visa设备句柄
* state: 每一位表示一个IO的状态
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgProjectGetXinState(ViSession vi, unsigned int *state);
/*
* 设置系统的外部输出IO的状态
* vi :visa设备句柄
* index: 0->YOUT1; 1->YOUT2
* state: 0->low| 1->high
* 返回值：0表示执行成功；－1表示执行失败
* 说明: 只有在工程模式下,才允许读取IO状态
*/
EXPORT_API int CALL mrgProjectSetYout(ViSession vi, int index, int state);
/*
* 设置系统的序列号
* vi :visa设备句柄
* serial: 序列号
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgProjectSetSerialNum(ViSession vi, char * serial);
/*
* 查询系统的序列号
* vi :visa设备句柄
* serial: 序列号
* 返回值：返回序列号的长度
*/
EXPORT_API int CALL mrgProjectGetSerialNum(ViSession vi, char * serial);
#endif