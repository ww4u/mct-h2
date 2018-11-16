
//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "device.h"

#define  SEND_BUF   (100)

/*
* 查找网关。
* bus :总线类型
* output：查找到的设备描述符，多个设备之间以逗号分隔
* len : output的最大容量（字节个数）
* method:当网关设备使用网络连接时，在查找网关设备时有两种查找方式：
    method = 0,使用VISA查找方式；
    method = 1,使用UDP方式查找
* 返回值：查找到的设备个数
*/
MEGAGATEWAY_API int CALL mrgFindGateWay(char * bus, char *output, int len,int method)
{
    return busFindDevice(bus, output, len,method);
}
/*
* 打开网关设备。
* desc :findGateWay()返回的设备描述符
* timeout：设备通讯时的最大超时时间
* 返回值：查找到的设备个数
*/
MEGAGATEWAY_API int CALL mrgOpenGateWay(char * desc, int timeout_ms)
{
    return busOpenDevice(desc, timeout_ms);
}
/*
* 关闭网关设备。
* desc :findGateWay()返回的设备描述符
* timeout：设备通讯时的最大超时时间
* 返回值：查找到的设备个数
*/
MEGAGATEWAY_API int CALL mrgCloseGateWay(ViSession  vi)
{
    return busCloseDevice(vi);
}
/*
* 向网关直接发送设置命令。
* vi :设备句柄
* cmd：要发送的设置命令
* len:命令长度
* 返回值：0表示执行成功，－1表示失败
*/
MEGAGATEWAY_API int CALL mrgGateWaySendCmd(ViSession  vi, char* cmd, int len)
{
    return busWrite(vi, cmd, strlen(cmd));
}
/*
* 向网关直接发送查询命令。
* vi :设备句柄
* cmd：要发送的设置命令
* len:命令长度
* output:查询命令的返回
* wantlen:期望查询命令返回的长度，即output的长度
* 返回值：查找到的设备个数
*/
MEGAGATEWAY_API int CALL mrgGateWayQuery(ViSession  vi, char* cmd, char * output, int wantlen)
{
    char ret[1024];
    int retlen = 0;
    if (wantlen > 1024)
    {
        return -1;
    }
    retlen = busQuery(vi, cmd, strlen(cmd), ret, 1024);
    memcpy(output, ret, retlen);
    output[retlen] = 0;
    return 0;
}

/*
* 查询网关设备的*IDN。
* idn :返回的设备描述符
* len：idn缓存长度
* 返回值：查找到的设备个数
*/
MEGAGATEWAY_API int CALL mrgGateWayIDNQuery(ViSession  vi, char * idn)
{
    char args[SEND_BUF];
    int retlen = 0;
    snprintf(args, SEND_BUF, "*IDN?\n");
    if ((retlen = busQuery(vi, args, strlen(args), idn, 100)) <= 0)
    {
        return -1;
    }
    idn[retlen] = 0;
    return 0;
}

/*
* 查找连接在网关上的设备。
* vi :visa设备句柄
* timeout为最长查找时间，单位：ms.
* 返回值：查找到的设备个数
*/
MEGAGATEWAY_API int CALL mrgFindDevice(ViSession vi, int timeout_ms)
{
    char ret[40];
    char args[SEND_BUF];
    int retlen = 0;
    snprintf(args, SEND_BUF, "DEVICE:SEARCH\n");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return 0;
    }
    Sleep(timeout_ms);
    snprintf(args, SEND_BUF, "DEVICE:COUNT?\n");
    if ((retlen = busQuery(vi, args, strlen(args), ret, 40)) <= 0)
    {
        return 0;
    }
    ret[retlen] = 0;
    return atoi(ret);
}
/*
* 获取所有与网关相连的设备名称（sendid的字符形式）
* vi :visa设备句柄
* names：返回名称的存放处
* 返回值：设备名称个数
*/
MEGAGATEWAY_API int CALL mrgGetDeviceName(ViSession vi, int * name)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char names[1024];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "DEVICE:NAME?\n");
    if ((retlen = busQuery(vi, args, strlen(args), names, 1024)) == 0)
    {
        return -1;
    }
    name[retlen] = 0;
    p = strtok_r(names, ",", &pNext);
    while (p)
    {
        *name++ = atoi(p);
        p = strtok_r(NULL, ",", &pNext);
        count++;
    }
    return count;
}
/*
* 获取指定设备的类型
* vi :visa设备句柄
* type：返回设备类型的存储区
* type可能的取值： “MRQM2304”，“MRQM2305”，“MRQM2310”，“MRQC23D”，“MRQC23S”，
* type_len：存储区的长度，长度最少8个字节
* 返回值：0表示执行成功，－1表示失败
*/
MEGAGATEWAY_API int CALL mrgGetDeviceType(ViSession vi, int name, char * type)
{
    char args[SEND_BUF];
    int len = 0;
    snprintf(args, SEND_BUF, "DEVICE:TYPe? %d\n", name);
    if ((len = busQuery(vi, args, strlen(args), type, 12)) == 0)
    {
        return -1;
    }
    type[len] = '\0';
    return 0;
}
/*
* 获取指定设备的信息，包括（序列号：硬件版本号：软件版本号：boot版本号：逻辑版本号）
* vi :visa设备句柄
* info：返回信息的存储区
* 返回值：0表示执行成功，－1表示失败
*/
MEGAGATEWAY_API int CALL mrgGetDeviceInfo(ViSession vi, int name, char * info)
{
    char args[SEND_BUF];
    int retlen = 0;
    snprintf(args, SEND_BUF, "DEVICE:FIRMWARE:ALL? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), info, 100)) == 0) {
        return -1;
    }
    else {
        info[retlen - 1] = '\0';
        return 0;
    }
}
/*
* 获取指定设备的软件版本号
* vi :visa设备句柄
* version：返回设备版本号的存储区
* len：存储区的长度
* 返回值：0表示执行成功，－1表示失败
*/
MEGAGATEWAY_API int CALL mrgGetDeviceSoftVersion(ViSession vi, int name, char * version)
{
    char args[SEND_BUF];
    int len = 0;
    snprintf(args, SEND_BUF, "DEVICE:FIRMWARE:SOFT? %d\n", name);
    if ((len = busQuery(vi, args, strlen(args), version, 20)) == 0) {
        return -1;
    }
    else {
        version[len] = '\0';
        return 0;
    }
}
/*
* 查询网关设备的*IDN。
* idn :返回的设备描述符
* len：idn缓存长度
* 返回值：查找到的设备个数
*/
MEGAGATEWAY_API int CALL mrgWriteDeviceSerial(ViSession  vi, int name, char * serial)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "PROJECT:DEVICE:SN %d,%s", name, serial);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 获取指定设备的序列号
* vi :visa设备句柄
* serial：返回设备序列号的存储区
* len：存储区的长度，长度最少18个字节
* 返回值：0表示执行成功，－1表示失败
*/
MEGAGATEWAY_API int CALL mrgGetDeviceSerialNumber(ViSession vi, int name, char * serial)
{
    char args[SEND_BUF];
    int len = 0;
    snprintf(args, SEND_BUF, "DEVICE:FIRMWARE:SN? %d\n", name);
    if ((len = busQuery(vi, args, strlen(args), serial, 20)) == 0) {
        return -1;
    }
    else {
        serial[len] = '\0';
        return 0;
    }
}
/*
* 识别网关设备（ON时，LED1常亮；OFF时LED闪烁）
* vi :visa设备句柄
* name：机器人
* state：识别状态
* 返回值：0表示执行成功，－1表示失败
*/
MEGAGATEWAY_API int CALL  mrgIdentify(ViSession vi, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:IDENTIFY %s", state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}

