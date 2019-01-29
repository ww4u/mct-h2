#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errorcode.h"

#define  SEND_BUF   (100)

/*
上传错误代码配置
vi :visa设备句柄
code:错误代码值
error:返回错误代码配置值
len: 长度
*/
EXPORT_API int CALL mrgErrorCodeConfigUpload(ViSession vi, int code, int *type, int *diagnose, int *response, int *enable)
{
    char error[256] = "";
    int len = 256;
	char args[SEND_BUF];
    int retlen = 0, count = 0;;
    snprintf(args, SEND_BUF, ":ERRCode:UPLoad? %d\n", code);
	if ((retlen = busQuery(vi, args, strlen(args), error, len)) == 0) {
		return -1;
	}
    error[retlen-1] = '\0';

    char *p, *pNext;
    char values[32][64] = {""};
    p = STRTOK_S(error, ",", &pNext);
    while (p)
    {
        strcpy(values[count++], p);
        p = STRTOK_S(NULL, ",", &pNext);
    }

    //! type
    if(STRCASECMP(values[0], "F") == 0){
        *type = 1;
    }
    else if(STRCASECMP(values[0], "W") == 0){
        *type = 2;
    }
    else if(STRCASECMP(values[0], "I") == 0){
        *type = 3;
    }
    else{
        return -2;
    }

    //! diagnose
    if(STRCASECMP(values[1], "ON") == 0){
        *diagnose = 1;
    }
    else if(STRCASECMP(values[1], "OFF") == 0){
        *diagnose = 0;
    }
    else{
        return -3;
    }

    //! response
    if(STRCASECMP(values[2], "A") == 0){
        *response = 1;
    }
    else if(STRCASECMP(values[2], "B") == 0){
        *response = 2;
    }
    else if(STRCASECMP(values[2], "C") == 0){
        *response = 3;
    }
    else if(STRCASECMP(values[2], "D") == 0){
        *response = 4;
    }
    else if(STRCASECMP(values[2], "E") == 0){
        *response = 5;
    }
    else if(STRCASECMP(values[2], "F") == 0){
        *response = 6;
    }
    else if(STRCASECMP(values[2], "G") == 0){
        *response = 7;
    }
    else{
        return -4;
    }

    //! enable
    if(STRCASECMP(values[3], "Y") == 0){
        *enable = 1;
    }
    else if(STRCASECMP(values[3], "N") == 0){
        *enable = 0;
    }
    else{
        return -5;
    }

	return 0;
}

/*
下载错误代码配置
vi :visa设备句柄
code:错误代码值
type:类型，F/W/I ->错误/警告/信息 -> 1,2,3
diagnose: 诊断器是否配置 必配/选配 -> 0,1
response: 错误响应，A/B/C/D/E/F/G -> 1,2,3,4,5,6,7
enable: 是否使能 Y/N ->0,1
*/
EXPORT_API int CALL mrgErrorCodeConfigDownload(ViSession vi, int code, int type, int diagnose, int response, int enable)
{
	char args[SEND_BUF];
	int retlen = 0;

	char* ps8Type = NULL;
	char* ps8Diagnose = NULL;
	char* ps8Response = NULL;
	char* ps8Enable = NULL;

	if (type == 1)
	{
		ps8Type = "F";
	}
	else if (type == 2)
	{
		ps8Type = "W";
	}
	else if (type == 3)
	{
		ps8Type = "I";
	}
	else
	{
		return -1;
	}


    if (diagnose == 1)
	{
		ps8Diagnose = "ON";
	}
    else if (diagnose == 0)
	{
		ps8Diagnose = "OFF";
	}
	else
	{
		return -2;
	}

	if (response == 1)
	{
		ps8Response = "A";
	}
	else if (response == 2)
	{
		ps8Response = "B";
	}
	else if (response == 3)
	{
		ps8Response = "C";
	}
	else if (response == 4)
	{
		ps8Response = "D";
	}
	else if (response == 5)
	{
		ps8Response = "E";
	}
	else if (response == 6)
	{
		ps8Response = "F";
	}
	else if (response == 7)
	{
		ps8Response = "G";
	}
	else
	{
		return -3;
	}

	if (enable == 0)
	{
		ps8Enable == "N";
	}
	else if (enable == 1)
	{
		ps8Enable = "Y";
	}
	else
	{
		return -4;
	}

    snprintf(args, SEND_BUF, ":ERRCode:DOWNLoad %d,%s,%s,%s,%s\n", code, ps8Type, ps8Diagnose, ps8Response, ps8Enable);
	if ((retlen = busWrite(vi, args, strlen(args))) <= 0) {
		return -5;
	}
	return 0;
}

/*
* 读取错误文件内容到上位机
* vi :visa设备句柄
* format: NORMAL|ZIP|TARGZ|TAR -> 0,1,2,3
* errorLog:返回错误日志
* wantlen： errorLog（存储区）的长度
* 返回值：返回实际的文件长度
*/
EXPORT_API int CALL mrgErrorLogUpload(ViSession vi, int format, char* errorLog, int wantlen)
{
    int retlen = 0, count = 0, lenOfLen = 0, readLen = 0, left = 0;
    char args[SEND_BUF];
    char as8Ret[1024], as8StrLen[20];
    char *as8Format[] = { "NORMAL","ZIP","TARGZ","TAR" };

	if (format < 0 || format > 3)
	{
		return -2;
	}
    snprintf(args, SEND_BUF, ":ERRLog:UPLoad? %s\n", as8Format[format]);
    if (busWrite(vi, args, strlen(args)) == 0)
    {
        return 0;
    }
    // 1. 先读回一个#9的头。
    retlen = busRead(vi, as8Ret, 12);
    if (retlen <= 0)
    {
        return retlen;
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
    errorLog[0] = as8Ret[11];
    count = 1;
    while (left >0)
    {
        readLen = (left > 512) ? 512 : left;
        //返回的#9数据最后，会有一个分号，所以这里多读一个字节。
        if ((retlen = busRead(vi, as8Ret, readLen)) == 0)
        {
            break;
        }
        memcpy(&errorLog[count], as8Ret, retlen);
        count += retlen;
        left -= retlen;
    }
    return count;
}

/*
清除错误日志
vi :visa设备句柄
*/
EXPORT_API int CALL mrgErrorLogClear(ViSession vi)
{
	char args[SEND_BUF];
	int retlen = 0;

    snprintf(args, SEND_BUF, ":ERRL:CLEar\n");
	if ((retlen = busWrite(vi, args, strlen(args))) == 0) {
		return -1;
	}
	return 0;
}

