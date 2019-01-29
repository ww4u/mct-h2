#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"

#define SEND_BUF  (100)
/*
*切换 MRH-T 的模式
*vi :visa设备句柄
*mode:MRH-T 的模式，取值范围： [0, 1]
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
*mode:MRH-T 的模式，取值范围： [0, 1]
*返回值：0表示执行成功，－1表示失败
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


EXPORT_API int CALL mrgSysWifiSearch(ViSession vi, char *wifiList)
{
    char args[SEND_BUF];
    char ret[1024] = {0};
    int retlen = 0;
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:SEARCH?\n");
    if ((retlen = busQuery(vi, args, strlen(args),ret,sizeof(ret))) <= 0)
    {
        return -1;
    }
    ret[retlen - 1] = 0;
    strcpy(wifiList,ret);
    return 0;
}

EXPORT_API int CALL mrgSysWifiConnect(ViSession vi, char *wifi, char *password)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:CONFig %s,%s\n", wifi, password);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }

    int time = 0, intervalTime = 20;
    while (1)
    {
        if (time > 200)
        {
            return 0;
        }
        if(mrgSysWifiStatusQuery(vi) != 1)
        {
            Sleep(intervalTime);
            time += intervalTime;
            continue;
        }
        return 1;
    }
}

EXPORT_API int CALL mrgSysWifiStatusQuery(ViSession vi)
{
    char args[SEND_BUF];
    char state[1024] = {0};
    int retlen = 0;
    snprintf(args, SEND_BUF, "SYSTEM:WIFI:STATe?\n");
    if ((retlen = busQuery(vi, args, strlen(args),state,sizeof(state))) <= 0)
    {
        return -1;
    }
    state[retlen - 1] = 0;

    if( STRCASECMP(state, "ON") == 0 )
        return 1;
    else if( STRCASECMP(state, "OFF") == 0 )
        return 0;
    else
        return -1;
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

