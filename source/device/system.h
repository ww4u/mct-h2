#ifndef MR_SYSTEM_H
#define MR_SYSTEM_H
#include "bus.h"

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif
	
/*
 * 识别网关设备（ON时，LED1常亮；OFF时LED闪烁）
 * vi :visa设备句柄
 * name：机器人
 * state：识别状态
 * 返回值：0表示执行成功，－1表示失败
 */
EXPORT_API int CALL mrgIdentify(ViSession vi, int state);
/*
 * 切换 MRH - T 的模式
 * vi :visa设备句柄
 * mode : MRH - T 的模式，取值范围： [0,1]
 * 返回值：0表示执行成功，－1表示失败
 */
EXPORT_API int CALL mrgModeSwitch(ViSession vi, int mode);
/*
 * 查询 MRH-T 软件版本号
*vi :visa设备句柄
*返回值：0回读正确;否则回读失败
*/
EXPORT_API int CALL mrgSysGetSoftVersion(ViSession vi, char * version);
/*
*查询 MRH-T 背板的软件版本号
*vi :visa设备句柄
*返回值：0回读正确;否则回读失败
*/
EXPORT_API int CALL mrgSysGetBackBoardSoftVersion(ViSession vi, char * version);
/*
*查询 MRH-T 背板的硬件版本号
*vi :visa设备句柄
*返回值：0回读正确;否则回读失败
*/
EXPORT_API int CALL mrgSysGetBackBoardHardVersion(ViSession vi, char * version);
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
*获取MRH的CAN口波特率
*vi :visa设备句柄
*baud:波特率 1000000,800000,500000,250000,125000,100000
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysConfCan_Query(ViSession vi, int* baud);
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
*设置指令表的模式 按行执行或者按段执行
*vi :visa设备句柄
*mode:指令表执行模式，取值范围： [0, 1]. 0表示按行执行,1表示按段执行
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgSysInstructionMode(ViSession vi, int mode);
/*
*查询指令表的模式
*vi :visa设备句柄
*mode:指令表执行模式，取值范围： [0, 1]. 0表示按行执行,1表示按段执行
*返回值：0表示执行成功,否则表示执行失败
*/
EXPORT_API int CALL mrgSysInstructionMode_Query(ViSession vi, int* mode);
/*
* 设置网络的IP模式
* vi :visa设备句柄
* mode : IP 模式 0->静态IP; 1->DHCP
* 返回值：0设置成功,-1设置失败
*/
EXPORT_API int CALL mrgSysSetNetworkIPMode(ViSession vi, int mode);
/*
* 获取网络的IP模式
* vi :visa设备句柄
* mode : IP 模式 0->静态IP; 1->DHCP
* 返回值：0执行成功,否则执行失败
*/
EXPORT_API int CALL mrgSysGetNetworkIPMode(ViSession vi, int* mode);
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
EXPORT_API int CALL mrgSysSetInterface(ViSession vi, int ifce);
/*
* 查询系统控制接口
* vi :visa设备句柄
* interface: 控制接口 LAN|USB|IO
* 返回值：0表示执行成功；－1表示执行失败
*/
EXPORT_API int CALL mrgSysGetInterface(ViSession vi, int * ifce);
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
/*
 * vi: visa句柄
 * fileList: 输出查询到的可用升级文件名字
 * 返回值: 成功返回0,失败返回-1,返回1表示没有U盘
 */
EXPORT_API int CALL mrgSysUpdateFileSearch(ViSession vi, char *fileList);
/*
 * 更新固件
 * vi: visa句柄
 * filename: 输入升级的文件名
 * 返回值：失败返负数,成功返0,超时返1
 */
EXPORT_API int CALL mrgSysUpdateFileStart(ViSession vi, char *filename);
/*
 * 查询当前升级状态
 * vi: visa句柄
 * 返回值: 成功返回0，失败返回负数，正在升级返回1
 */
EXPORT_API int CALL mrgSysUpdateFirmwareStatusQuery(ViSession vi);
/*
 * 获取MRHT上的py脚本列表
 * vi: visa句柄
 * isUdisk: 1为U盘,0为MRH-T内部
 * fileList 输出查询到文件列表
 * 失败返回-1,成功返回0，返回1表示没有文件，返回2表示没有U盘
 */
EXPORT_API int CALL mrgScriptSearch(ViSession vi, int isUdisk, char *fileList);
/*
 * 配置脚本及是否开机自启动
 * vi: visa句柄
 * filename: 要配置的文件名
 * isBoot: 设置为开机启动,0为不启动,1为启动
 * 成功返回0, 失败返回-1
 * 如果配置的文件名为U盘中的文件名，则会将U盘中的文件导入到MRHT内部并配置为当前脚本
 */
EXPORT_API int CALL mrgScriptConfig(ViSession vi, char *filename, int isBoot);
/*
 * 查询当前配置的脚本名字
 * vi: visa句柄
 * filename: 输出查询到的配置脚本名字
 * 成功返回0, 失败返回-1
 */
EXPORT_API int CALL mrgScriptConfigQuery(ViSession vi, char *filename);
/*
 * 运行当前配置的脚本
 * vi: visa句柄
 * 成功返回0, 失败返回-1
 */
EXPORT_API int CALL  mrgScriptRun(ViSession vi);
/*
 * 停止运行当前配置的脚本
 * vi: visa句柄
 * 成功返回0, 失败返回-1
 */
EXPORT_API int CALL mrgScriptStop(ViSession vi);
/*
 * 停止运行当前配置的脚本
 * vi: visa句柄
 * 失败返回-1，运行返回1，停止返回0
 */
EXPORT_API int CALL mrgScriptGetCurrentStates(ViSession vi);

#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif // !MR_SYSTEM_H

