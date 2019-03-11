#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"


#define SEND_BUF  (100)
/*
* 设置系统为工程模式
* vi :visa设备句柄
* state: 0->OFF| 1->ON
* 返回值：0表示执行成功；－1表示执行失败
* 说明: 只有在工程模式下,才允许读取IO状态
*/
EXPORT_API int CALL mrgSetProjectMode(ViSession vi, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "PROJect:STATe %s\n", state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询外部IO的状态
* vi :visa设备句柄
* state: 每一位表示一个IO的状态
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgProjectGetXinState(ViSession vi, unsigned int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "PROJect:XREAD? 0\n");

    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) <= 0)
    {
        return -1;
    }
    as8Ret[retLen - 1] = 0;
    *state = strtoul(as8Ret, NULL, 0);
    return 0;
}
/*
* 设置系统的外部输出IO的状态
* vi :visa设备句柄
* index: 0->YOUT1; 1->YOUT2
* state: 0->low| 1->high
* 返回值：0表示执行成功；－1表示执行失败
* 说明: 只有在工程模式下,才允许读取IO状态
*/
EXPORT_API int CALL mrgProjectSetYout(ViSession vi, int index, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "PROJect:YWRITE %s,%s\n", index ? "Y2" : "Y1", state ? "H" : "L");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 设置系统的序列号
* vi :visa设备句柄
* serial: 序列号
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgProjectSetSerialNum(ViSession vi, char * serial)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "PROJect:SN %s\n", serial);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询系统的序列号
* vi :visa设备句柄
* serial: 序列号
* 返回值：返回序列号的长度
*/
EXPORT_API int CALL mrgProjectGetSerialNum(ViSession vi, char * serial)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "PROJect:SN?\n");

    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) <= 0)
    {
        return 0;
    }
    as8Ret[retLen - 1] = 0;
    strcpy(serial, as8Ret);
    return retLen-1;
}
