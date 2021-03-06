#ifndef _MEGA_STORAGE_H
#define _MEGA_STORAGE_H

#include "bus.h"

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

/*
* 查询存储器中，可用的运动文件
* vi :visa设备句柄
* type:0表示本地存储（本地文件系统）；1表示外部存储（U盘之类）
* fileList: 点坐标文件名列表，以逗号分隔
* len : fileList的缓存长度
* 返回值：大于零表示返回实际的字节数，小于零表示出错
*/
EXPORT_API int CALL mrgStorageMotionFileQuery(ViSession vi, int type, char* fileList, int len);
/*
* 删除本地存储器中的运动文件
* vi :visa设备句柄
* location:0表示本地存储（本地文件系统）；1表示外部存储（U盘之类）
* fileList: 点坐标文件名列表，以逗号分隔
* len : fileList的缓存长度
* 返回值：大于零表示返回实际的字节数，小于零表示出错
*/
EXPORT_API int CALL mrgStorageMotionFileDelete(ViSession vi, char* fileName);
/*
* 读取运动文件内容到上位机
* vi :visa设备句柄
* filename: 文件名
* context：读取到的内容
* wantlen： context（存储区）的长度
* 返回值：返回实际的文件长度
*/
EXPORT_API int CALL  mrgStorageMotionFileContextRead(ViSession vi, char* filename, char* context);
/*
* 保存运动文件内容到本地存储器
* vi :visa设备句柄
* context: 文件内容
* len:文件内容长度
* saveFileName：目的文件名
* 返回值：  0：写入成功；1：写入失败
*/
EXPORT_API int CALL mrgStorageMotionFileSaveContext(ViSession vi, char* context, int len, char * saveFileName);
/*
* 保存文件内容
* vi :visa设备句柄
* isUdisk: 是否写在U盘上
* path: 文件路径
* saveFileName：目的文件名
* data: 文件内容
* dataLen:文件内容长度
* 返回值：  0：写入成功；1：写入失败
*/
EXPORT_API int CALL mrgStorageWriteFile(ViSession vi, int isUdisk, char *path, char *saveFileName, unsigned char *data, int dataLen);
/*
* 读取文件到上位机
* vi :visa设备句柄
* isUdisk: 是否写在U盘上
* path: 文件路径
* filename: 文件名
* context：读取到的内容
* 返回值：返回实际的文件长度
*/
EXPORT_API int CALL mrgStorageReadFile(ViSession vi, int isUdisk, char *path, char* filename, unsigned char* context);

#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif // !MR_SYSTEM_H
