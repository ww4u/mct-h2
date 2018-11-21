#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"

#define SEND_BUF  (100)
/*
*�л� MRH-T ��ģʽ
*vi :visa�豸���
*mode:MRH-T ��ģʽ��ȡֵ��Χ�� [0, 1]
*����ֵ��0��ʾִ�гɹ�����1��ʾʧ��
*/
EXPORT_API int CALL mrgSysModeSwitch(ViSession vi, int mode)
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
*��ѯ MRH-T ��ģʽ
*vi :visa�豸���
*mode:MRH-T ��ģʽ��ȡֵ��Χ�� [0, 1]
*����ֵ��0��ʾִ�гɹ�����1��ʾʧ��
*/
EXPORT_API int CALL mrgGetSysMode(ViSession vi)
{
    char args[SEND_BUF];
    char ret[8] = {0};
    int retlen = 0;
    snprintf(args, SEND_BUF, "SYSTEM:MODe:SWITch?\n");
    if ((retlen = busQuery(vi, args, strlen(args),ret,8)) <= 0)
    {
        return -1;
    }
    ret[retlen - 1] = 0;
    return atoi(ret);
}



