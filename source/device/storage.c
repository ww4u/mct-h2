#include "storage.h"

#define SEND_BUF  (100)
/*
* 查询存储器中，可用的运动文件
* vi :visa设备句柄
* location:0表示本地存储（本地文件系统）；1表示外部存储（U盘之类）
* fileList: 点坐标文件名列表，以逗号分隔
* len : fileList的缓存长度
* 返回值：大于零表示返回实际的字节数，小于零表示出错
*/
EXPORT_API int CALL mrgStorageMotionFileQuery(ViSession vi, int location, char* fileList, int len)
{
    int retlen = 0;
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "STORage:FIL:MOT:%s?\n", (location == 0) ? "LOC" : "EXTER");
    if ((retlen = busQuery(vi, args, strlen(args), fileList, len)) == 0) {
        return -1;
    }
    if (retlen < len)
    {
        fileList[retlen - 1] = 0;
        return retlen - 1;
    }
    else
    {
        return len;
    }
    
}
/*
* 删除本地存储器中的运动文件
* vi :visa设备句柄
* location:0表示本地存储（本地文件系统）；1表示外部存储（U盘之类）
* fileList: 点坐标文件名列表，以逗号分隔
* len : fileList的缓存长度
* 返回值：大于零表示返回实际的字节数，小于零表示出错
*/
EXPORT_API int CALL mrgStorageMotionFileDelete(ViSession vi, char* fileName)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "STORage:FIL:MOT:DEL %s\n", fileName);
    if ((busWrite(vi, args, strlen(args))) == 0) {
        return -1;
    }
    return 0;
}
/*
* 读取运动文件内容到上位机
* vi :visa设备句柄
* filename: 文件名
* context：读取到的内容
* wantlen： context（存储区）的长度
* 返回值：返回实际的文件长度
*/
EXPORT_API int CALL mrgStorageMotionFileContextRead(ViSession vi, char* filename, char* context)
{
    int retlen = 0;
    int count = 0;
    int lenOfLen = 0;
    int readLen = 0;
    int left = 0;
    char args[SEND_BUF];
    char as8Ret[1024];
    char as8StrLen[20];
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:READ? %s\n", filename);
    if (busWrite(vi, args, strlen(args)) == 0)
    {
        return 0;
    }
    // 1. 先读回一个#9的头。
    retlen = busRead(vi, as8Ret, 12);
    if (retlen <= 0)
    {
        return 0;
    }
    if (as8Ret[0] != '#')//格式错误
    {
        return count;
    }
    lenOfLen = as8Ret[1] - 0x30;
    memcpy(as8StrLen, &as8Ret[2], lenOfLen);//取出长度字符串
    left = strtoul(as8StrLen, NULL, 10);
    if (left == 0)
    {
        return 0;
    }
    context[0] = as8Ret[11];
    count = 1;
    while (left >0)
    {
        readLen = (left > 512) ? 512 : left;
        //返回的#9数据最后，会有一个分号，所以这里多读一个字节。
        if ((retlen = busRead(vi, as8Ret, readLen)) == 0)
        {
            break;
        }
        memcpy(&context[count], as8Ret, retlen);
        count += retlen;
        left -= retlen;
    }
    return count;
}
/*
* 等待文件写入完成
* vi :visa设备句柄
* 返回值：0表示等待成功，－1：表示等待过程中出错
*/
int waitMotionFileWriteEnd(int vi)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    int time = 0;
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:DATA:STATE?\n");
    while (1)
    {
        if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0)
        {
            msSleep(10);
            time += 10;
            continue;
        }
        as8Ret[retLen - 1] = '\0';//去掉回车符
        if (STRCASECMP(as8Ret, "IDLE") == 0)
        {
            return 0;
        }
        if (time > 100)
        {
            break;
        }
    }
    return -1;
}
/*
* 保存运动文件内容到本地存储器
* vi :visa设备句柄
* context: 文件内容
* len:文件内容长度
* saveFileName：目的文件名
* 返回值：  0：写入成功；1：写入失败
*/
EXPORT_API int CALL mrgStorageMotionFileSaveContext(ViSession vi, char* context,int len, char * saveFileName)
{
#if 1
    int count = 0;
    int writeLen = 0;
    int cmdLen = 0;
    char args[SEND_BUF];
    char as8Ret[1024];
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:NAMe %s\n", saveFileName);
    
    if (busWrite(vi, args, strlen(args)) == 0)//写入文件名
    {
        return -1;
    }
    //写入文件内容
    while (len > 0)
    {
        writeLen = len > 512 ? 512 : len;
        snprintf(as8Ret, 1024, "STORage:FILe:MOTion:CONText:WRITe:DATa #9%09d", writeLen);
        cmdLen = strlen(as8Ret);
        memcpy(&as8Ret[cmdLen],&context[count], writeLen);
        if (busWrite(vi, as8Ret, writeLen + cmdLen) == 0)
        {
            return -1;
        }
        len -= writeLen;
        count += writeLen;
    }
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:END\n");
    if (busWrite(vi, args, strlen(args)) == 0)//写入文件结束
    {
        return -1;
    }
    return 0;
#endif

    // MRH-T 大于1.20版本才可用下面这种方式
    return mrgStorageWriteFile(vi, 0, "/home/megarobo/MRH-T/motionfile/", saveFileName, context, len);
}
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
EXPORT_API int CALL mrgStorageWriteFile(ViSession vi, int isUdisk, char *path, char *saveFileName,unsigned char *data, int dataLen)
{
    int retLen = -1;
    int count = 0;
    int len = dataLen;
    int writeLen = 0;
    int cmdLen = 0;
    char args[SEND_BUF];
    char as8Ret[1024];
    char as8State[64];

    snprintf(args, SEND_BUF, "STORage:FILe:WRITe:START %s,%s,%s\n",isUdisk?"UDISK":"LOCAL", path, saveFileName);
    if (busWrite(vi, args, strlen(args)) == 0)//写入文件名
    {
        return -1;
    }
    //写入文件内容
    while (len > 0)
    {
        writeLen = len > 512 ? 512 : len;
        snprintf(as8Ret, 1024, "STORage:FILe:WRITe:DATa? #9%09d", writeLen);
        cmdLen = strlen(as8Ret);
        memcpy(&as8Ret[cmdLen],&data[count], writeLen);
        memset(as8State,0,sizeof(as8State));
        if (busWrite(vi, as8Ret, writeLen + cmdLen) == 0)
        {
            msSleep(1);
            continue;
        }
        retLen = busRead(vi, as8State, sizeof(as8State));
        if(retLen == 0)
        {
            return -2;
        }
        as8State[retLen - 1] = '\0';//去掉回车符
        if(STRCASECMP(as8State, "ERROR") == 0)
        {
            msSleep(1);
            continue;
        }
        len -= writeLen;
        count += writeLen;
    }
    snprintf(args, SEND_BUF, "STORage:FILe:WRITe:END? %d\n", dataLen);
    memset(as8State,0,sizeof(as8State));
    retLen = busQuery(vi, args, strlen(args), as8State, sizeof(as8State));
    if (retLen == 0)//写入文件结束
    {
        return -3;
    }
    as8State[retLen - 1] = '\0';//去掉回车符
    if(STRCASECMP(as8State, "ERROR") == 0)
        return -4;

    return 0;
}
/*
* 读取文件到上位机
* vi :visa设备句柄
* isUdisk: 是否写在U盘上
* path: 文件路径
* filename: 文件名
* context：读取到的内容
* 返回值：返回实际的文件长度
*/
EXPORT_API int CALL mrgStorageReadFile(ViSession vi, int isUdisk, char *path, char* filename, unsigned char* context)
{
    int retlen = 0;
    int count = 0;
    int lenOfLen = 0;
    int readLen = 0;
    int left = 0;
    char args[SEND_BUF];
    char as8Ret[1024];
    char as8StrLen[20];

    snprintf(args, SEND_BUF, "STORage:FILe:READ? %s,%s,%s\n",
             isUdisk?"UDISK":"LOCAL", path, filename);

    if (busWrite(vi, args, strlen(args)) == 0)
    {
        return 0;
    }
    // 1. 先读回一个#9的头。
    retlen = busRead(vi, as8Ret, 12);
    if (retlen <= 0)
    {
        return 0;
    }
    if (as8Ret[0] != '#')//格式错误
    {
        return count;
    }
    lenOfLen = as8Ret[1] - 0x30;
    memcpy(as8StrLen, &as8Ret[2], lenOfLen);//取出长度字符串
    left = strtoul(as8StrLen, NULL, 10);
    if (left == 0)
    {
        return 0;
    }
    context[0] = as8Ret[11];
    count = 1;
    while (left >0)
    {
        readLen = (left > 512) ? 512 : left;
        //返回的#9数据最后，会有一个分号，所以这里多读一个字节。
        if ((retlen = busRead(vi, as8Ret, readLen)) == 0)
        {
            break;
        }
        memcpy(&context[count], as8Ret, retlen);
        count += retlen;
        left -= retlen;
    }
    return count;
}
