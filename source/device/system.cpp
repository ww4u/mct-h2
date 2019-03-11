#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"

#define SEND_BUF  (100)
/*
*启动MRH的CAN口
*vi :visa设备句柄
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysStartCan(ViSession vi)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "CAN:STARt\n");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*复位MRH的CAN口
*vi :visa设备句柄
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysResetCan(ViSession vi)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "CAN:RESET\n");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*配置MRH的CAN口波特率
*vi :visa设备句柄
*baud:波特率 1000000,800000,500000,250000,125000,100000
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysConfCan(ViSession vi,int baud)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "CAN:CONFig %d\n", baud);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*切换 MRH-T 的模式
*vi :visa设备句柄
*mode:MRH-T 的模式，取值范围： [0, 1]. 0表示直通模式,1表示MRH模式
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysModeSwitch(ViSession vi, int mode)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:MODe:SWITch %d\n", mode);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*查询 MRH-T 的模式
*vi :visa设备句柄
*返回值：MRH-T 的模式. 0表示直通模式,1表示MRH模式
*/
EXPORT_API int CALL mrgGetSysMode(ViSession vi)
{
    char args[SEND_BUF];
    char ret[8] = {0};
    int retlen = 0;
    snprintf(args, SEND_BUF, "SYSTEM:MODe:SWITch?\n");
    if ((retlen = busQuery(vi, args, strlen(args),ret,8)) <= 0)
    {
        return -1;
    }
    ret[retlen - 1] = 0;
    return atoi(ret);
}

/*
*扫描可用的WIFI
*vi :visa设备句柄
*result：可用的WIFI命令，以逗号分隔
*返回值：返回实际接收到的数据长度
*/
EXPORT_API int CALL mrgSysWifiScan(ViSession vi, char* result,int len)
{
    char args[SEND_BUF];
    char as8Ret[1024];
    int retLen = 0;
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:SEARCH?\n");
    if ((retLen = busQuery(vi, args, strlen(args),as8Ret,1024)) <= 0)
    {
        return 0;
    }
    as8Ret[retLen - 1] = 0;
    retLen = (len > retLen) ? retLen : len;
    memcpy(result, as8Ret,retLen);
    return retLen;
}
/*
*配置WIFI
*vi :visa设备句柄
*name：WIFi名称
* passwd：密码
*返回值：返回实际接收到的数据长度
*/
EXPORT_API int CALL mrgSysWifiConfig(ViSession vi, char* name, char* passwd)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:CONFig %s,%s\n", name, passwd);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*停止或启动无线网卡
*vi :visa设备句柄
*state：0表示停止， 1表示启用
*返回值：0:执行成功； 否则执行失败
*/
EXPORT_API int CALL mrgSysWifiUpDown(ViSession vi, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:%s\n", state?"UP":"DOWN");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*检查WIFI的连接状态
*vi :visa设备句柄
*timeout_ms：等待超时时间，单位：ms
*返回值：0:连接超时或未连接； 1：连接成功
*/
EXPORT_API int CALL mrgSysWifiCheckState(ViSession vi, int timeout_ms)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0,time = 0;
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:STATe?\n");
    while (time < timeout_ms)
    {
        Sleep(200);
        if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 1024)) <= 0)
        {
            continue;
        }
        as8Ret[retLen - 1] = 0;
        if (STRCASECMP(as8Ret, "ON") == 0)
        {
            return 1;
        }
        time += 200;
    }
    return 0;
}
/*
*查询当前已连接上的WIFI名称
*vi :visa设备句柄
*name：WIFI名
*返回值：0:执行成功； 否则执行失败
*/
EXPORT_API int CALL mrgSysGetWifiConnected(ViSession vi, char* name)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0, time = 0;
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:NAMe?\n");
   
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 1024)) <= 0)
    {
        return -1;
    }
    as8Ret[retLen - 1] = 0;
    strcpy(name, as8Ret);
    return 0;
}
/*
*设置系统的心跳时间,单位:秒
*vi :visa设备句柄
*value: 心跳时间,单位:秒
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysSetHeartPeriod(ViSession vi, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:HEARTBEAT:VALUE %d\n", value);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询系统的心跳时间,单位:秒
* vi :visa设备句柄
* value: 心跳时间,单位:秒
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysGetHeartPeriod(ViSession vi, int * value)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0, time = 0;
    snprintf(args, SEND_BUF, "SYSTEM:HEARTBEAT:VALUE?\n");

    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) <= 0)
    {
        return -1;
    }
    as8Ret[retLen - 1] = 0;
    *value = atoi(as8Ret);
    return 0;
}
/*
*设置系统上电值
*vi :visa设备句柄
*value: 上电值 DEFAULT|LAST
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysSetPowerOn(ViSession vi, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:POWERON %s\n", value == 0? "DEFAULT":"LAST");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*查询系统上电值
*vi :visa设备句柄
* value: 上电值 0:DEFAULT; 1:LAST
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysGetPowerOn(ViSession vi, int * value)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0, time = 0;
    snprintf(args, SEND_BUF, "SYSTEM:POWERON?\n");

    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) <= 0)
    {
        return -1;
    }
    as8Ret[retLen - 1] = 0;
    if (STRCASECMP(as8Ret, "DEFAULT") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *value = 0;
    }
    else if (STRCASECMP(as8Ret, "LAST") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *value = 1;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置系统控制接口
*vi :visa设备句柄
*interface: 上电值 LAN|USB|IO
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysSetInterface(ViSession vi, int interface)
{
    char args[SEND_BUF];
    char *ps8Interface[3] = { "LAN", "USB", "IO" };
    snprintf(args, SEND_BUF, "SYSTEM:INTERFACE %s\n", ps8Interface[interface]);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询系统控制接口
* vi :visa设备句柄
* interface: 控制接口 LAN|USB|IO
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysGetInterface(ViSession vi, int * interface)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "SYSTEM:INTERFACE?\n");

    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) <= 0)
    {
        return -1;
    }
    as8Ret[retLen - 1] = 0;
    if (STRCASECMP(as8Ret, "LAN") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *interface = 0;
    }
    else if (STRCASECMP(as8Ret, "USB") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *interface = 1;
    }
    else if (STRCASECMP(as8Ret, "IO") == 0 || STRCASECMP(as8Ret, "2") == 0)
    {
        *interface = 2;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置紧急停止
*vi :visa设备句柄
*state: 0->OFF| 1->ON
*返回值：0表示执行成功；－1表示执行失败
*紧急停止时,会停止当前系统CAN网络中的所有设备,并设置紧急停止标志位.在紧急标志位被清除前,不接受任何CAN帧.
* 当state为OFF时,只是清除标志位
*/
EXPORT_API int CALL mrgSysSetEmergencyStop(ViSession vi, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SCRAM:STATE %s\n", state?"ON":"OFF");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询紧急停止状态
* vi :visa设备句柄
* state:  0->OFF| 1->ON
* 返回值：0表示执行成功；－1表示执行失败
* 紧急停止时,会自动设置此标志位
*/
EXPORT_API int CALL mrgSysGetEmergencyStopState(ViSession vi,int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "SCRAM:STATE?\n");

    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) <= 0)
    {
        return -1;
    }
    as8Ret[retLen - 1] = 0;
    if (STRCASECMP(as8Ret, "OFF") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *state = 0;
    }
    else if (STRCASECMP(as8Ret, "ON") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = 1;
    }
    else
    {
        return -1;
    }
    return 0;
}

/**
 * @brief mrgSysUpdateFileSearch
 * @param vi
 * @param 查询可用的升级文件名字
 * @return 成功返回0,失败返回-1,返回1表示没有U盘
 */
EXPORT_API int CALL mrgSysUpdateFileSearch(ViSession vi, char *fileList)
{
    char args[SEND_BUF];
    char ret[1024] = {0};
    int retlen = 0;
    snprintf(args, SEND_BUF, "SYST:UPGR:SEAR?\n");
    if ((retlen = busQuery(vi, args, strlen(args),ret,sizeof(ret))) <= 0)
    {
        return -1;
    }
    ret[retlen - 1] = 0;
    strcpy(fileList,ret);

    if( STRCASECMP(ret, "NO_UDISK") == 0 )
        return 1;

    return 0;
}

/**
 * @brief mrgSysUpdateFileStart 更新固件
 * @param vi 句柄
 * @param filename 升级的文件名
 * @return 失败返回负数,成功返回0,超时返回1
 */
EXPORT_API int CALL mrgSysUpdateFileStart(ViSession vi, char *filename)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYST:UPGR:STAR %s\n", filename);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }

    int time = 0, intervalTime = 20;
    while (1)
    {
        if (time > (20 * 1000) )
        {
            return 1;
        }
        int ret = mrgSysUpdateFirmwareStatusQuery(vi);
        if(ret <= 0)
        {
            return ret;
        }
        if(ret == 1)
        {
            Sleep(intervalTime);
            time += intervalTime;
            continue;
        }
    }
}
/**
 * @brief mrgSysUpdateFirmwareStatusQuery 查询当前升级状态
 * @param vi
 * @return 成功返回0，失败返回负数，正在升级返回1
 */
EXPORT_API int CALL mrgSysUpdateFirmwareStatusQuery(ViSession vi)
{
    char args[SEND_BUF];
    char state[1024] = {0};
    int retlen = 0;
    snprintf(args, SEND_BUF, "SYST:UPGR:STAT?\n");
    if ((retlen = busQuery(vi, args, strlen(args),state,sizeof(state))) <= 0)
    {
        return -1;
    }
    state[retlen - 1] = 0;

    if( STRCASECMP(state, "COMPLETED") == 0 )
        return 0;
    else if( STRCASECMP(state, "UPDATING") == 0 )
        return 1;
    else if( STRCASECMP(state, "FAILD") == 0 )
        return -2;
    else
        return -1;
}

