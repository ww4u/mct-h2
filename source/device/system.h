#ifndef MR_SYSTEM_H
#define MR_SYSTEM_H

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif


#include "bus.h"
#include "export.h"
/*
*启动MARH的CAN口
*vi :visa设备句柄
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysStartCan(ViSession vi);
/*
*复位MRH的CAN口
*vi :visa设备句柄
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysResetCan(ViSession vi);
/*
*配置MRH的CAN口波特率
*vi :visa设备句柄
*baud:波特率 1000000,800000,500000,250000,125000,100000
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysConfCan(ViSession vi, int baud);
/*
*切换 MRH-T 的模式
*vi :visa设备句柄
*mode:MRH-T 的模式，取值范围： [0, 1]
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysModeSwitch(ViSession vi, int mode);
/*
*查询 MRH-T 的模式
*vi :visa设备句柄
*返回值：MRH-T 的模式
*/
EXPORT_API int CALL mrgGetSysMode(ViSession vi);
/*
*扫描可用的WIFI
*vi :visa设备句柄
*result：可用的WIFI命令，以逗号分隔
*返回值：返回实际接收到的数据长度
*/
EXPORT_API int CALL mrgSysWifiScan(ViSession vi, char* result, int len);
/*
*配置WIFI
*vi :visa设备句柄
*name：WIFi名称
* passwd：密码
*返回值：返回实际接收到的数据长度
*/
EXPORT_API int CALL mrgSysWifiConfig(ViSession vi, char* name, char* passwd);
/*
*停止或启动无线网卡
*vi :visa设备句柄
*state：0表示停止， 1表示启用
*返回值：0:执行成功； 否则执行失败
*/
EXPORT_API int CALL mrgSysWifiUpDown(ViSession vi, int state);
/*
*检查WIFI的连接状态
*vi :visa设备句柄
*timeout_ms：等待超时时间，单位：ms
*返回值：0:连接超时或未连接； 1：连接成功
*/
EXPORT_API int CALL mrgSysWifiCheckState(ViSession vi, int timeout_ms);
/*
*查询当前已连接上的WIFI名称
*vi :visa设备句柄
*name：WIFI名
*返回值：0:执行成功； 否则执行失败
*/
EXPORT_API int CALL mrgSysGetWifiConnected(ViSession vi, char* name);

/*
*设置系统的心跳时间,单位:秒
*vi :visa设备句柄
*value: 心跳时间,单位:秒
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysSetHeartPeriod(ViSession vi, int value);
/*
*查询系统的心跳时间,单位:秒
*vi :visa设备句柄
* value: 心跳时间,单位:秒
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysGetHeartPeriod(ViSession vi, int * value);
/*
*设置系统上电值
*vi :visa设备句柄
*value: 上电值 DEFAULT|LAST
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysSetPowerOn(ViSession vi, int value);
/*
*查询系统上电值
*vi :visa设备句柄
* value: 上电值 0:DEFAULT; 1:LAST
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysGetPowerOn(ViSession vi, int * value);
/*
*设置系统控制接口
*vi :visa设备句柄
*interface: 上电值 LAN|USB|IO
*返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysSetInterface(ViSession vi, int face);
/*
* 查询系统控制接口
* vi :visa设备句柄
* interface: 控制接口 LAN|USB|IO
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysGetInterface(ViSession vi, int *face);
/*
*设置紧急停止
*vi :visa设备句柄
*state: 0->OFF| 1->ON
*返回值：0表示执行成功；－1表示执行失败
*紧急停止时,会停止当前系统CAN网络中的所有设备,并设置紧急停止标志位.在紧急标志位被清除前,不接受任何CAN帧.
* 当state为OFF时,只是清除标志位
*/
EXPORT_API int CALL mrgSysSetEmergencyStop(ViSession vi, int state);
/*
* 查询紧急停止标志位,主要用来清除标志位
* vi :visa设备句柄
* state:  0->OFF| 1->ON
* 返回值：0表示执行成功；－1表示执行失败
* 紧急停止时,会自动设置此标志位
*/
EXPORT_API int CALL mrgSysGetEmergencyStopState(ViSession vi, int *state);


/**
 * @brief mrgSysUpdateFileSearch
 * @param vi
 * @param 查询可用的升级文件名字
 * @return 成功返回0,失败返回-1,返回1表示没有U盘
 */
EXPORT_API int CALL mrgSysUpdateFileSearch(ViSession vi, char *fileList);

/**
 * @brief mrgSysUpdateFileStart 更新固件
 * @param vi 句柄
 * @param filename 升级的文件名
 * @return 失败返回负数,成功返回0,超时返回1
 */
EXPORT_API int CALL mrgSysUpdateFileStart(ViSession vi, char *filename);

/**
 * @brief mrgSysUpdateFirmwareStatusQuery 查询当前升级状态
 * @param vi
 * @return 成功返回0，失败返回负数，正在升级返回1
 */
EXPORT_API int CALL mrgSysUpdateFirmwareStatusQuery(ViSession vi);

#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif
#endif // !MR_SYSTEM_H

