#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"

#define SEND_BUF  (100)
/*
* 查询存储器中，可用的运动文件
* vi :visa设备句柄
* type:0表示本地存储（本地文件系统）；1表示外部存储（U盘之类）
* fileList: 点坐标文件名列表，以逗号分隔
* len : fileList的缓存长度
* 返回值：大于零表示返回实际的字节数，小于零表示出错
*/
EXPORT_API int CALL mrgStorageMotionFileQuery(ViSession vi, int type, char* fileList, int len)
{
    int retlen = 0;
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "STOR:FIL:MOT:%s?\n", (type == 0) ? "LOC" : "EXTER");
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
* 读取运动文件内容到上位机
* vi :visa设备句柄
* filename: 文件名
* context：读取到的内容
* wantlen： context（存储区）的长度
* 返回值：返回实际的文件长度
*/
EXPORT_API int CALL mrgStorageMotionFileContextRead(ViSession vi, char* filename, char* context, int wantlen)
{
    int retlen = 0, count = 0, lenOfLen = 0, readLen = 0, left = 0;
    char args[SEND_BUF];
    char as8Ret[1024],as8StrLen[20];
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
* 保存运动文件内容到本地存储器,
* vi :visa设备句柄
* srcFileName: 源文件名
* saveFileName：目的文件名
* 返回值：  0：写入成功；1：写入失败
*/
EXPORT_API int CALL mrgStorageMotionFileSave(ViSession vi, char* srcFileName, char * saveFileName)
{
    int retlen = 0, count = 0, filesize = 0, writeLen = 0, cmdLen = 0;
    char args[SEND_BUF];
    char as8Ret[1024], as8StrLen[20];
    FILE * pFile = NULL;
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:NAMe %s\n", saveFileName);
    if ((pFile = fopen(srcFileName, "r")) == NULL)
    {
        return -1;
    }
    fseek(pFile, 0L, SEEK_END);
    filesize = ftell(pFile);
    fseek(pFile, 0L, SEEK_SET);
    if (busWrite(vi, args, strlen(args)) == 0)//写入文件名
    {
        fclose(pFile);
        return -2;
    }
    //写入文件内容
    while (filesize > 0)
    {
        writeLen = filesize > 512 ? 512 : filesize;
        snprintf(as8Ret, 1024, "STORage:FILe:MOTion:CONText:WRITe:DATa #9%09d", writeLen);
        cmdLen = strlen(as8Ret);
        fread(&as8Ret[cmdLen], 1, writeLen, pFile);
        if (busWrite(vi, as8Ret, writeLen+cmdLen) == 0)
        {
            fclose(pFile);
            return -3;
        }
        if (waitMotionFileWirteEnd(vi) != 0)
        {
            fclose(pFile);
            return -5;
        }
        filesize -= writeLen;
    }
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:END\n");
    if (busWrite(vi, args, strlen(args)) == 0)//写入文件结束
    {
        fclose(pFile);
        return -4;
    }
    fclose(pFile);
    return 0;
}
/*
* 保存运动文件内容到本地存储器
* vi :visa设备句柄
* context: 文件内容
* len:文件内容长度
* saveFileName：目的文件名
* 返回值：  0：写入成功；1：写入失败
*/
EXPORT_API int CALL mrgStorageMotionFileSaveContext(ViSession vi, char* context,int len, char *saveFileName)
{
    int retlen = len, count = 0;
    char args[SEND_BUF];
    char as8Ret[1024];
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:NAMe %s\n", saveFileName);
    
    if (busWrite(vi, args, strlen(args)) == 0)//写入文件名
    {
        return -1;
    }

    //写入文件内容
    while (retlen > 0)
    {
        int writeLen = retlen > 512 ? 512 : retlen;
        memset(as8Ret, 0, sizeof(as8Ret));
        snprintf(as8Ret, 1024, "STORage:FILe:MOTion:CONText:WRITe:DATa #9%09d", writeLen);
        int cmdLen = strlen(as8Ret);
        memcpy(&as8Ret[cmdLen],&context[count], writeLen);
        as8Ret[cmdLen+writeLen] = '\n';
        if (busWrite(vi, as8Ret, writeLen + cmdLen + 1) == 0)
        {
            return -1;
        }        
        if (waitMotionFileWirteEnd(vi) != 0)
        {
            return -1;
        }
        retlen -= writeLen;
        count += writeLen;
    }

    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:END\n");
    if (busWrite(vi, args, strlen(args)) == 0)//写入文件结束
    {
        return -1;
    }
    return 0;
}

/*
* 等待文件写入完成
* vi :visa设备句柄
* 返回值：0表示等待成功，－1：表示等待过程中出错
*/
EXPORT_API int CALL waitMotionFileWirteEnd(int vi)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0,time = 0, intervalTime = 50;
    snprintf(args, SEND_BUF, "STORage:FILe:MOTion:CONTEXT:WRITe:DATA:STATE?\n");
    while (1)
    {
        if (time > 300)
        {
            break;
        }
        if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0)
        {
            Sleep(intervalTime);
            time += intervalTime;
            continue;
        }
        as8Ret[retLen - 1] = '\0';//去掉回车符
        if (STRCASECMP(as8Ret, "IDLE") == 0)
        {
            return 0;
        }
    }
    return -1;
}

