#ifndef _DEVICE_H_
#define _DEVICE_H_

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

#include "bus.h"
#include "export.h"

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
EXPORT_API int CALL mrgFindGateWay(int  bus, char *output, int len, int method);
/*
* 打开网关设备。
* desc :findGateWay()返回的设备描述符
* timeout：设备通讯时的最大超时时间
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgOpenGateWay(char * desc, int timeout_ms);
/*
* 关闭网关设备。
* desc :findGateWay()返回的设备描述符
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgCloseGateWay(ViSession  vi);
/*
* 向网关直接发送设置命令。
* vi :设备句柄
* cmd：要发送的设置命令
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGateWaySendCmd(ViSession  vi, char* cmd, int len);
/*
* 向网关直接发送查询命令。
* vi :设备句柄
* cmd：要发送的设置命令
* len:命令长度
* output:查询命令的返回
* wantlen:期望查询命令返回的长度，即output的长度
* 返回值：实际返回的数据长度
*/
EXPORT_API int CALL mrgGateWayQuery(ViSession  vi, char* cmd, char * output, int wantlen);
/*
* 查询网关设备的*IDN。
* idn :返回的设备描述符
* len：idn缓存长度
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGateWayIDNQuery(ViSession  vi, char * idn);
/*
* 查找连接在网关上的设备。
* vi :visa设备句柄
* timeout为最长查找时间，单位：ms.
* 返回值：查找到的设备个数
*/
EXPORT_API int CALL mrgFindDevice(ViSession vi, int timeout_ms);
/*
* 获取所有与网关相连的设备名称
* vi :visa设备句柄
* names：返回名称的存放处
* names_len：存储区的长度
* 返回值：设备名称个数
*/
EXPORT_API int CALL mrgGetDeviceName(ViSession vi, int* names);
/*
* 获取指定设备的类型
* vi :visa设备句柄
* type：返回设备类型的存储区
* type可能的取值： “MRQM2304”，“MRQM2305”，“MRQM2310”，“MRQC23D”，“MRQC23S”，
* type_len：存储区的长度，长度最少8个字节
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetDeviceType(ViSession vi, int name, char * type);
/*
*查询指定设备的通道个数,name为仪器名
*vi :visa设备句柄
*name:机器人的名字
*返回值：大于零，表示通道个数，小于或等于零表示失败
*/
EXPORT_API int CALL mrgGetDeviceChannelCount(ViSession vi, int name);

/*
* 获取指定设备的信息，包括（序列号：硬件版本号：软件版本号：boot版本号：逻辑版本号）
* vi :visa设备句柄
* info：返回信息的存储区
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetDeviceInfo(ViSession vi, int name, char * info);
/*
* 获取指定设备的软件版本号
* vi :visa设备句柄
* version：返回设备版本号的存储区
* len：存储区的长度
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetDeviceSoftVersion(ViSession vi, int name, char * version);
/*
*查询设备硬件版本号
*vi :visa设备句柄
*name:机器人的名字
*buf:设备硬件版本号,长度最少12个字节
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetFirmWareHard(ViSession vi, int name, char *buf);
/*
*查询设备BOOT版本号
*vi :visa设备句柄
*name:机器人的名字
*buf:设备BOOT版本号
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetFirmWareBoot(ViSession vi, int name, char *buf);
/*
*查询设备逻辑版本号
*vi :visa设备句柄
*name:机器人的名字
*buf:设备逻辑版本号
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetFirmWareFpga(ViSession vi, int name, char *buf);
/*
* 写入设备的序列号
* idn :返回的设备描述符
* len：idn缓存长度
* 返回值：
* 说明：此函数为工程接口函数，不对外开放
*/
EXPORT_API int CALL mrgWriteDeviceSerial(ViSession  vi, int name, char * serial);
/*
* 获取指定设备的序列号
* vi :visa设备句柄
* serial：返回设备序列号的存储区
* len：存储区的长度，长度最少18个字节
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetDeviceSerialNumber(ViSession vi, int name, char * serial);


/*
* 识别网关设备（ON时，LED1常亮；OFF时LED闪烁）
* vi :visa设备句柄
* name：机器人
* state：识别状态
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgIdentify(ViSession vi, int state);

/*
*切换 MRH - T 的模式
*vi :visa设备句柄
*mode : MRH - T 的模式，取值范围： [0,1]
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgModeSwitch(ViSession vi, int mode);
/*
*读取MRHT的IO状态
*vi :visa设备句柄
*state : xin的状态,低四位
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetXinState(ViSession vi, int* state);
/*
*设置MRHT的IO输出状态
*vi :visa设备句柄
*yout : 0表示Y1,1表示Y2
*state : YOUT的状态,0表示低电平,1表示高电平
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSetYoutState(ViSession vi, int yout, int state);


#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif // !_DEVICE_H_

