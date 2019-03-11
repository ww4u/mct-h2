#ifndef MR_SYSTEM_H
#define MR_SYSTEM_H

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif


#include "bus.h"
#include "export.h"
/*
*切换 MRH-T 的模式
*vi :visa设备句柄
*mode:MRH-T 的模式，取值范围： [0, 1]
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysModeSwitch(ViSession vi, int mode);

/**
 * @brief 搜索Wifi
 * @param vi: visa设备句柄
 * @param wifiList: 输出wifi列表
 * @return 0表示成功, -1表示失败
 */
EXPORT_API int CALL mrgSysWifiSearch(ViSession vi, char *wifiList);

/**
 * @brief 连接或断开wifi
 * @param vi: visa设备句柄
 * @param wifi: wifi名称
 * @param password: wifi密码
 * @return 1表示连接成功,0表示连接失败, -1表示出错
 */
EXPORT_API int CALL mrgSysWifiConnect(ViSession vi, char *wifi, char *password);

/**
 * @brief 查询wifi连接状态
 * @param vi: visa设备句柄
 * @return 1表示已连接，0表示未连接，-1表示失败
 */
EXPORT_API int CALL mrgSysWifiStatusQuery(ViSession vi);

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

