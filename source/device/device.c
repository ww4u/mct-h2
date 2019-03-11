
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device.h"

#define  SEND_BUF   (100)

/*
* 查找网关。
* bus :总线类型. ＝0，网络接口；＝1，USBTMC接口
* output：查找到的设备描述符，多个设备之间以逗号分隔
* len : output的最大容量（字节个数）
* method:当网关设备使用网络连接时，在查找网关设备时有两种查找方式：
    method = 0,使用VISA查找方式；
    method = 1,使用UDP方式查找
* 返回值：查找到的设备个数
*/
EXPORT_API int CALL mrgFindGateWay(int  bus, char *output, int len,int method)
{
    return busFindDevice(bus, output, len,method);
}
/*
* 打开网关设备。
* desc :findGateWay()返回的设备描述符
* timeout：设备通讯时的最大超时时间
* 返回值：查找到的设备个数
*/
EXPORT_API int CALL mrgOpenGateWay(char * desc, int timeout_ms)
{
    return busOpenDevice(desc, timeout_ms);
}
/*
* 关闭网关设备。
* desc :findGateWay()返回的设备描述符
* timeout：设备通讯时的最大超时时间
* 返回值：查找到的设备个数
*/
EXPORT_API int CALL mrgCloseGateWay(ViSession  vi)
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
EXPORT_API int CALL mrgGateWaySendCmd(ViSession  vi, char* cmd, int len)
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
* 返回值：实际返回的数据长度
*/
EXPORT_API int CALL mrgGateWayQuery(ViSession  vi, char* cmd, char * output, int wantlen)
{
    int retlen = 0;
    if (wantlen > 1024)
    {
        return -1;
    }
    retlen = busQuery(vi, cmd, strlen(cmd), output, wantlen);
    return retlen;
}

/*
* 查询网关设备的*IDN。
* idn :返回的设备描述符
* len：idn缓存长度
* 返回值：查找到的设备个数
*/
EXPORT_API int CALL mrgGateWayIDNQuery(ViSession  vi, char * idn)
{
    char args[SEND_BUF];
    int retlen = 0;
    snprintf(args, SEND_BUF, "*IDN?\n");
    if ((retlen = busQuery(vi, args, strlen(args), idn, 100)) <= 0)
    {
        return -1;
    }
    idn[retlen-1] = 0;
    return 0;
}

/*
* 查找连接在网关上的设备。
* vi :visa设备句柄
* timeout为最长查找时间，单位：ms.
* 返回值：查找到的设备个数
*/
EXPORT_API int CALL mrgFindDevice(ViSession vi, int timeout_ms)
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
EXPORT_API int CALL mrgGetDeviceName(ViSession vi, int * name)
{
    int count = 0, retlen = 0,tmp = 0;
    char args[SEND_BUF];
    char names[1024];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "DEVICE:NAME?\n");
    if ((retlen = busQuery(vi, args, strlen(args), names, 1024)) == 0)
    {
        return -1;
    }
    name[retlen - 1] = 0;
    p = STRTOK_S(names, ",", &pNext);
    while (p)
    {
        tmp = atoi(p);
        if (tmp != 0)
        {
            name[count++] = tmp;
        }
        p = STRTOK_S(NULL, ",", &pNext);
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
EXPORT_API int CALL mrgGetDeviceType(ViSession vi, int name, char * type)
{
    char args[SEND_BUF];
    int len = 0;
    snprintf(args, SEND_BUF, "DEVICE:TYPe? %d\n", name);
    if ((len = busQuery(vi, args, strlen(args), type, 12)) == 0)
    {
        return -1;
    }
    type[len-1] = '\0';
    return 0;
}
/*
*查询指定设备的通道个数,name为仪器名
*vi :visa设备句柄
*name:机器人的名字
*返回值：大于零，表示通道个数，小于或等于零表示失败
*/
EXPORT_API int CALL mrgGetDeviceChannelCount(ViSession vi, int name)
{
    char args[SEND_BUF];
    int retLen = 0;
    char state[10];
    snprintf(args, SEND_BUF, "DEVice:CHANnel:COUNt? %d\n", name);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }
    return atoi(state);
}
/*
* 获取指定设备的信息，包括（序列号：硬件版本号：软件版本号：boot版本号：逻辑版本号）
* vi :visa设备句柄
* info：返回信息的存储区
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetDeviceInfo(ViSession vi, int name, char * info)
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
EXPORT_API int CALL mrgGetDeviceSoftVersion(ViSession vi, int name, char * version)
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
*查询设备硬件版本号
*vi :visa设备句柄
*name:机器人的名字
*buf:设备硬件版本号,长度最少12个字节
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetFirmWareHard(ViSession vi, int name, char *buf)
{
    char args[SEND_BUF];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVice:FIRMware:HARD? %d\n", name);
    if ((retLen = busQuery(vi, args, strlen(args), buf, 12)) == 0) {
        return -1;
    }
    else
    {
        buf[retLen - 1] = '\0';
    }
    return 0;
}
/*
*查询设备BOOT版本号
*vi :visa设备句柄
*name:机器人的名字
*buf:设备BOOT版本号
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetFirmWareBoot(ViSession vi, int name, char *buf)
{
    char args[SEND_BUF];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVice:FIRMware:BOOT? %d\n", name);
    if ((retLen = busQuery(vi, args, strlen(args), buf, 12)) == 0) {
        return -1;
    }
    else
    {
        buf[retLen - 1] = '\0';
    }
    return 0;
}
/*
*查询设备逻辑版本号
*vi :visa设备句柄
*name:机器人的名字
*buf:设备逻辑版本号
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetFirmWareFpga(ViSession vi, int name, char *buf)
{
    char args[SEND_BUF];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVice:FIRMware:FPGA? %d\n", name);
    if ((retLen = busQuery(vi, args, strlen(args), buf, 40)) == 0) {
        return -1;
    }
    else
    {
        buf[retLen - 1] = '\0';
    }
    return 0;
}
/*
* 查询网关设备的*IDN。
* idn :返回的设备描述符
* len：idn缓存长度
* 返回值：查找到的设备个数
*/
EXPORT_API int CALL mrgWriteDeviceSerial(ViSession  vi, int name, char * serial)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "PROJECT:DEVICE:SN %d,%s\n", name, serial);
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
EXPORT_API int CALL mrgGetDeviceSerialNumber(ViSession vi, int name, char * serial)
{
    char args[SEND_BUF];
    int len = 0;
    snprintf(args, SEND_BUF, "DEVICE:FIRMWARE:SN? %d\n", name);
    if ((len = busQuery(vi, args, strlen(args), serial, 20)) == 0) {
        return -1;
    }
    else {
        serial[len-1] = '\0';
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
EXPORT_API int CALL  mrgIdentify(ViSession vi, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "SYSTEM:IDENTIFY %s\n", state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*切换 MRH - T 的模式
*vi :visa设备句柄
*mode : MRH - T 的模式，取值范围： [0,1]
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgModeSwitch(ViSession vi, int mode)
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
*读取MRHT的IO状态
*vi :visa设备句柄
*state : xin的状态,低四位
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetXinState(ViSession vi, int* state)
{
    char args[SEND_BUF];
    char as8Ret[100] = { 0 };
    int len = 0;
    snprintf(args, SEND_BUF, "PROJ:XREAD? 0");
    if ((len = busQuery(vi, args, strlen(args), as8Ret, 20)) == 0) {
        return -1;
    }
    else {
        as8Ret[len - 1] = '\0';
        *state = atoi(as8Ret);
        return 0;
    }
}
/*
*设置MRHT的IO输出状态
*vi :visa设备句柄
*yout : 0表示Y1,1表示Y2
*state : YOUT的状态,0表示低电平,1表示高电平
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSetYoutState(ViSession vi, int yout,int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "PROJ:YWRITE %s,%s", yout?"Y2":"Y1", state?"H":"L");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
