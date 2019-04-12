// MegaGateway.cpp : 定义 DLL 应用程序的导出函数。
//
#include "megarobot.h"
#include "mrqdevice.h"
#include "device.h"

#define     SEND_BUF        (100)
#define     DELAYTIME       (100)  //! 查询延时时间

/*********************** 机器人操作 *******************************/
/*
* 构建一个机器人
* vi :visa设备句柄
* robotType：要构建的机器人类型（MRX-T4,MRX-H2,MRX-DELTA）
* devList：设备列表 “0@512,1@512,”
* robotname :分配的机器人名子
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgBuildRobot(ViSession vi, char * robotType, char * devList, int * robotname)
{
    char args[SEND_BUF];
    char name[8];
    int  id = 0;
    int retLen = 0;
    if (STRCASECMP("MRX-RAW", robotType) == 0 || devList == NULL)
    {
        snprintf(args, SEND_BUF, "ROBOT:ALLOC? %s\n", robotType);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:ALLOC? %s,(%s)\n", robotType, devList);
    }
    
    if ((retLen = busQuery(vi, args, strlen(args), name,8)) == 0) {
        return -1;
    }
    else {
        name[retLen-1] = '\0'; //去掉“\n”
    }
    id = atoi(name);
    if (id > 0)
    {
        *robotname = id;
        return 0;
    }
    return -1;
}
/*
* 构建一个机器人,机器人的名子由用户指定
* vi :visa设备句柄
* robotType：要构建的机器人类型（MRX-T4,MRX-H2,MRX-DELTA）
* devList：设备列表 “0@512,1@512,”
* robotid: 用户设定的机器人名子
* robotname :返回的机器人名子
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgBuildRobotNamed(ViSession vi, char * robotType, char * devList,int robotid, int * robotname)
{
    char args[SEND_BUF];
    char name[8];
    int  id = 0;
    int retLen = 0;
    if (STRCASECMP("MRX-RAW", robotType) == 0 || devList == NULL)
    {
        snprintf(args, SEND_BUF, "ROBOT:ALLOC? %s,,%d\n", robotType, robotid);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:ALLOC? %s,(%s),%d\n", robotType, devList, robotid);
    }

    if ((retLen = busQuery(vi, args, strlen(args), name, 8)) == 0) {
        return -1;
    }
    else {
        name[retLen - 1] = '\0'; //去掉“\n”
    }
    id = atoi(name);
    if (id > 0)
    {
        *robotname = id;
        return 0;
    }
    return -1;
}
/*
* 删除当前机器人
* vi :visa设备句柄
* name:机器人名称
* 返回值：0表示执行成功，－1表示执行失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgDeleteRobot(ViSession vi, int name)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:DELETE %d\n",name);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 设置当前机器人的构形的连秆长度  单位:mm
* vi :visa设备句柄
* name:机器人名称
* 返回值：小于零表示出错
* 说明：对T4来说: links[0] 基座高度;links[1] 大臂长度 ;links[2] 小臂长度 
 *  对H2来说: links[0] 宽;links[1] 高 ;links[2] 滑块宽度;links[3] 滑块高度,links[4] 模具类型;links[5] 齿数;
*/
EXPORT_API int CALL mrgSetRobotLinks(ViSession vi, int name,float * links,int link_count)
{
    char args[SEND_BUF];
    char as8Links[SEND_BUF] = { 0 }, tmp[20] = {0};
    int i;
    for (i = 0; i < link_count; i++)
    {
        if (i != 0)
        {
            strcat(as8Links, ",");
        }
        snprintf(tmp, 20, "%f", links[i]);
        strcat(as8Links, tmp);
    }
    snprintf(args, SEND_BUF, "ROBOT:LINK %d,(%s)\n", name, as8Links);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 获取当前机器人的构形的连秆长度  单位:mm
* vi :visa设备句柄
* name:机器人名称
* link_count: 获取到的连秆长度
* 返回值：0成功,否则失败
* 说明：
*/
EXPORT_API int CALL mrgGetRobotLinks(ViSession vi, int name, float * links, int *link_count)
{
    char args[SEND_BUF];
    char ret[100];
    char *pNext,*p;
    int retlen = 0;
    *link_count = 0;
    snprintf(args, SEND_BUF, "ROBOT:LINK? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 100)) == 0) {
        return -1;
    }
    else
    {
        ret[retlen - 1] = 0;
        p = STRTOK_S(ret, ",", &pNext);
        while (p)
        {
            *links++ = atof(p);
            p = STRTOK_S(NULL, ",", &pNext);
            (*link_count)++;
        }
    }
    return 0;
}
/*
* 查询当前机器人的构形
* vi :visa设备句柄
* name:机器人名称
* 返回值：小于零表示出错。 0：MRX-T4;1:MRX-AS;2:MRX-H2,3:MRX-DELTA;4:MRX-RAW
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgGetRobotType(ViSession vi, int name)
{
    char args[SEND_BUF];
    char ret[100];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:CONFIGURATION? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 100)) == 0) {
        return -1;
    }
    else
    {
        ret[retlen - 1] = 0;
        if (STRCASECMP(ret, "MRX-T4") == 0)
        {
            return MRX_TYPE_T4;
        }
        else if (STRCASECMP(ret, "MRX-AS") == 0)
        {
            return MRX_TYPE_AS;
        }
        else if (STRCASECMP(ret, "MRX-H2") == 0)
        {
            return MRX_TYPE_H2;
        }
        else if (STRCASECMP(ret, "MRX-DELTA") == 0)
        {
            return MRX_TYPE_DELTA;
        }
        else if (STRCASECMP(ret, "MRX-RAW") == 0)
        {
            return MRX_TYPE_RAW;
        }
    }
    return MRX_TYPE_UNKOWN;
}
/*
* 保存当前系统中所有机器人构形
* vi :visa设备句柄
* 返回值：0表示执行成功，－1表示失败
* 说明：
*/
EXPORT_API int CALL mrgExportRobotConfig(ViSession vi)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:CONFIGURATION:FILE:EXPort\n");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 保存当前系统中所有机器人构形为默认配置文件
* vi :visa设备句柄
* 返回值：0表示执行成功，－1表示失败
* 说明：工程命令，不对外开放
*/
EXPORT_API int CALL mrgExportRobotConfig_default(ViSession vi)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:CONFIGURATION:FILE:EXPort::DEFault\n");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 恢复上次保存的配置
* vi :visa设备句柄
* 返回值：0表示执行成功，－1表示失败
* 说明：
*/
EXPORT_API int CALL mrgRestoreRobotConfig(ViSession vi)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:CONFIGURATION:FILE:IMPort\n");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询导入或导出配置文件的状态
* vi :visa设备句柄
* 返回值：0表示执行完成；1表示正在执行；－1表示执行过程中出错
* 说明：
*/
EXPORT_API int CALL mrgGetRobotConfigState(ViSession vi)
{
    char args[SEND_BUF];
    char ret[100];
    int retlen = 0,try_times = 0;
    snprintf(args, SEND_BUF, "ROBOT:CONFIGURATION:FILE:STATE?\n");
    while (try_times < 10)
    {
        if ((retlen = busQuery(vi, args, strlen(args), ret, 100)) == 0) {
            try_times++;
            continue;
        }
        else
        {
            ret[retlen - 1] = '\0';
            if (STRCASECMP(ret, "BUSY") == 0)
            {
                return 1;
            }
            else if (STRCASECMP(ret, "IDLE") == 0)
            {
                return 0;
            }
            else if (STRCASECMP(ret, "ERROR") == 0)
            {
                return 2;
            }
        }
    }
    return -1;
}
/*
* 设置当前机器人所使用的机械结构的序列号
* vi :visa设备句柄
* name:机器人名称
* serial:机械结构序列号
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效. 
*/
EXPORT_API int CALL mrgSetRobotMachineSerialNum(ViSession vi, int name, char * sn)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:CONFIGuration:SN %d,%s\n", name, sn);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询当前机器人所使用的机械结构的序列号
* vi :visa设备句柄
* name:机器人名称
* serial:机械结构序列号
* 返回值：0表示执行正确，否则表示执行失败
* 说明：此函数目前只对H2有效. serial是不安全的，请在外部确保serial的空间足够
*/
EXPORT_API int CALL mrgGetRobotMachineSerialNum(ViSession vi, int name,char*serial)
{
    char args[SEND_BUF];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:CONFIGuration:SN? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), serial, 100)) == 0) {
        return -1;
    }
    serial[retlen - 1] = 0;
    return 0;
}
/*
* 设置当前机器人构形下的子类型
* vi :visa设备句柄
* name:机器人名称
* subtype:子类型。 对于H2来说，0表示小H2，1表示中H2（802x494）；2表示大H2（891x769）
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgSetRobotSubType(ViSession vi,int name,int subtype)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:SUBTYPE %d,%d\n", name,subtype);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询当前机器人构形下的子类型
* vi :visa设备句柄
* name:机器人名称
* 返回值：子类型。 对于H2来说，0表示小H2，1表示中H2（802x494）；2表示大H2（891x769）;3表示用户定制
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgGetRobotSubType(ViSession vi, int name)
{
    char args[SEND_BUF];
    char ret[8];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:SUBTYPE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 4)) == 0) {
        return -1;
    }
    else {
        ret[retlen] = '\0';
        return atoi(ret);
    }
}
/*
* 设置当前机器人的坐标系
* vi :visa设备句柄
* name:机器人名称
* coord:坐标系索引（详情参考对应的命令系统）
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgSetRobotCoordinateSystem(ViSession vi, int name, int coord)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:COORDinate %d,%d\n", name, coord);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询当前机器人的坐标系
* vi :visa设备句柄
* name:机器人名称
* 返回值：坐标系索引，（详情参考对应的命令系统）
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgGetRobotCoordinateSystem(ViSession vi, int name)
{
    char args[SEND_BUF];
    char ret[12];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:COORDinate? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 12)) == 0) {
        return -1;
    }
    else {
        ret[retlen] = '\0';
        return atoi(ret);
    }
}
/*
* 查询CAN网络中机器人的个数
* vi :visa设备句柄
* 返回值：返回机器人个数
*/
EXPORT_API int CALL mrgGetRobotCount(ViSession vi)
{
    char args[SEND_BUF];
    char ret[12];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:COUNT?\n");
    if ((retlen = busQuery(vi, args, strlen(args),ret,12)) == 0) {
        return -1;
    }
    else {
        ret[retlen - 1] = '\0';
        return atoi(ret);
    }
}
/*
* 查询CAN网络中所有机器人的名子
* vi :visa设备句柄
* robotnames：输出参数，机器人名称集
* 返回值：返回机器人个数
* 说明: robotnames是不安全的，请在外部确保robotnames的空间足够
*/
EXPORT_API int CALL mrgGetRobotName(ViSession vi,int *robotnames)
{
    char args[SEND_BUF];
    char names[100];
    char *p, *pNext;
    int retlen = 0,count = 0;
    snprintf(args, SEND_BUF, "ROBOT:NAME?\n");
    if ((retlen = busQuery(vi, args, strlen(args), names, 100)) > 0)
    {
        names[retlen - 1] = '\0';
        p = STRTOK_S(names, ",", &pNext);
        while (p)
        {
            *robotnames++ = atoi(p);
            p = STRTOK_S(NULL, ",", &pNext);
            count++;
        }
        return count;
    }

    if(count <= 0 || robotnames[0] == 0)
    {
        char buff[128] = "";
        int i,j;
        int deviceList[32] = {0};
        int deviceName = 0;
        int channelCount =0;
        int robotName = 0;

        count = mrgFindDevice(vi, 800);
        if(count <= 0){
            return -2;
        }

        mrgGetDeviceName(vi, deviceList);
        for(i=0,j=0; i<count && j<count; i++)
        {
            deviceName = deviceList[i];
            channelCount = mrgGetDeviceChannelCount(vi, deviceName);
            memset(buff, 0, sizeof(buff));
            if(channelCount >=2 && channelCount <= 4)
            {
                //! 构建一个H2
                sprintf(buff,"0@%d,1@%d", deviceName, deviceName);
                if(0 == mrgBuildRobot(vi, "MRX-H2", buff, &robotName ) )
                {
                    robotnames[j++] = robotName;
                }
            }
            else if(channelCount > 4)
            {
                //! 构建一个T4
                sprintf(buff,"0@%d,1@%d,2@%d,3@%d,4@%d",
                        deviceName, deviceName, deviceName, deviceName, deviceName);
                if(0 == mrgBuildRobot(vi, "MRX-T4", buff, &robotName ) )
                {
                    robotnames[j++] = robotName;
                }
            }
        }
        count = j;
    }

    return count;
}
/*
* 查询当前机器人的所使用的设备
* robotname: 机器人名称
* vi :visa设备句柄
* device：设备名称列表
* 返回值：返回所使用的设备个数
* 说明: device是不安全的，请在外部确保device的空间足够
*/
EXPORT_API int CALL mrgGetRobotDevice(ViSession vi,int robotname,int * device)
{
    char args[SEND_BUF];
    char devlist[100];
    char *p, *pNext;
    int count = 0, retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:DEVICE:NAME? %d\n", robotname);
    if ((retlen = busQuery(vi, args, strlen(args), devlist, 100)) == 0) {
        return -1;
    }
    devlist[retlen] = '\0';
    p = STRTOK_S(devlist, ",", &pNext);
    while (p)
    {
        *device++ = atoi(p);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    return count;
}

/*
* 设置当前机器人的项目零点
* vi :visa设备句柄
* name:机器人名称
* x,y，z:项目零点值（x,y,z）
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgSetRobotProjectZero(ViSession vi, int name, float x,float y,float z)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:ZERO:PROJECT %d,%f,%f,%f\n", name, x,y,z);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询当前机器人的项目零点
* vi :visa设备句柄
* name:机器人名称
* x,y,z: 输出参数，项目零点值
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgGetRobotProjectZero(ViSession vi, int name,float * x,float *y,float *z)
{
    char args[SEND_BUF];
    char ret[100];
    char *p, *pNext;
    int retlen = 0,count = 0;
    float values[10] = {0.0};
    snprintf(args, SEND_BUF, "ROBOT:ZERO:PROJECT? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 100)) == 0) {
        return -1;
    }
    ret[retlen - 1] = '\0';
    p = STRTOK_S(ret, ",", &pNext);
    while (p)
    {
        values[count++] = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
    }
    *x = values[0];
    *y = values[1];
    *z = values[2];
    return 0;
}
/*
* 设置当前机器人的校准(轴)零点
* vi :visa设备句柄
* name:机器人名称
* x,y，z:校准零点值（x,y,z）
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgSetRobotAxisZero(ViSession vi, int name, float x, float y, float z)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:ZERO:AXIS %d,%f,%f,%f\n", name, x, y, z);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询当前机器人的校准(轴)零点
* vi :visa设备句柄
* name:机器人名称
* x,y,z: 输出参数，校准零点值
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgGetRobotAxisZero(ViSession vi, int name, float * x, float *y, float *z)
{
    char args[SEND_BUF];
    char ret[100];
    char *p, *pNext;
    int retlen = 0, count = 0;
    float values[10] = { 0.0 };
    snprintf(args, SEND_BUF, "ROBOT:ZERO:AXIS? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 100)) == 0) {
        return -1;
    }
    ret[retlen - 1] = '\0';
    p = STRTOK_S(ret, ",", &pNext);
    while (p)
    {
        values[count++] = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
    }
    *x = values[0];
    *y = values[1];
    *z = values[2];
    return 0;
}
/*
* 设置当前机器人的软件限位
* vi :visa设备句柄
* name:机器人名称
* type: 正向限位还是负向限位。 0：表示正向限位；1表示负向限位
* x,y，z:限位值（x,y,z）
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgSetRobotSoftWareLimit(ViSession vi, int name,int type,float x, float y, float z)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:LIMIt:SOFT:%s %d,%f,%f,%f\n",type == 0?"POSITive":"NEGATive", name, x, y, z);
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
* 查询当前机器人的软件限位
* vi :visa设备句柄
* name:机器人名称
* type: 正向限位还是负向限位。 0：表示正向限位；1表示负向限位
* x,y,z: 输出参数，校准零点值
* 返回值：0表示执行成功，－1表示失败
* 说明：此函数目前只对H2有效
*/
EXPORT_API int CALL mrgGetRobotSoftWareLimit(ViSession vi, int name,int type, float * x, float *y, float *z)
{
    char args[SEND_BUF];
    char ret[100];
    char *p, *pNext;
    int retlen = 0, count = 0;
    float values[10] = { 0.0 };
    snprintf(args, SEND_BUF, "ROBOT:LIMIt:SOFT:%s? %d\n", type == 0 ? "POSITive" : "NEGATive", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 100)) == 0) {
        return -1;
    }
    ret[retlen - 1] = '\0';
    p = STRTOK_S(ret, ",", &pNext);
    while (p)
    {
        values[count++] = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
    }
    *x = values[0];
    *y = values[1];
    *z = values[2];
    return 0;
}
/*
* 设置指定机器人的波表
* vi :visa设备句柄
* name: 机器人名称
* wavetable:波表索引值，取值范围0~9
* 返回值：0表示设置成功，否则表示设置失败
*/
EXPORT_API int CALL mrgSetRobotWavetable(ViSession vi, int name,int wavetable)
{
    char args[SEND_BUF];
    if (wavetable < 0 || wavetable >= 10)
    {
        return -2;
    }
    snprintf(args, SEND_BUF,"ROBOT::WAVETABLE %d,%d\n", name,wavetable);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询指定机器人的波表
* vi :visa设备句柄
* name: 机器人名称
* wavetable:波表索引值，取值范围0~9
* 返回值：大于0表示波表索引，小于零表示查询失败
*/
EXPORT_API int CALL mrgRobotWavetableQuery(ViSession vi, int name)
{
    char args[SEND_BUF];
    char wavetable[4];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT::WAVETABLE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), wavetable,4)) == 0) {
        return -1;
    }
    wavetable[retlen-1] = '\0'; //去掉回车符
    if (STRCASECMP(wavetable, "MAIN") == 0 || STRCASECMP(wavetable, "0") == 0){
        return 0;
    }else if (STRCASECMP(wavetable, "SMALL") == 0){
        return 1;
    }else if (STRCASECMP(wavetable, "P1") == 0){
        return 2;
    }else if (STRCASECMP(wavetable, "P2") == 0){
        return 3;
    }else if (STRCASECMP(wavetable, "P3") == 0){
        return 4;
    }else if (STRCASECMP(wavetable, "P4") == 0){
        return 5;
    }else if (STRCASECMP(wavetable, "P5") == 0){
        return 6;
    }else if (STRCASECMP(wavetable, "P6") == 0){
        return 7;
    }else if (STRCASECMP(wavetable, "P7") == 0){
        return 8;
    }else if (STRCASECMP(wavetable, "P8") == 0){
        return 9;
    }else {
        return -2;
    }
}
/*
* 启动机器人的运行
* vi :visa设备句柄
* name: 机器人名称
* wavetable:波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* 返回值：0表示启动成功，否则表示启动失败
*/
EXPORT_API int CALL mrgRobotRun(ViSession vi,int name,int wavetable)
{
    char args[SEND_BUF];
    if (wavetable == -1)
    {
        snprintf(args, SEND_BUF, "ROBOT:RUN %d\n", name);
    }
    else if(wavetable >=0 && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:RUN %d,%d\n", name,wavetable);
    }
    else {
        return -2;
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 停止机器人的运行
* vi :visa设备句柄
* name: 机器人名称
* wavetable:波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* 返回值：0表示停止成功，否则表示停止失败
*/
EXPORT_API int CALL mrgRobotStop(ViSession vi, int name, int wavetable)
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:STOP %d,%d\n", name, wavetable);
    }
    else {
        snprintf(args, SEND_BUF, "ROBOT:STOP %d\n", name);
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}

/*
* 等待机器人的特定波表的ready状态（等待模块设备解算完成）
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* timeout_ms：等待超时时间 timeout_ms小于零表示不等待
* 返回值：0表示等待成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
*/
EXPORT_API int CALL mrgRobotWaitReady(ViSession vi, int name,int wavetable, int timeout_ms)
{
    int ret = -3, error_count = 0;
    char args[SEND_BUF];
    char state[12];
    int time = 0, retlen = 0;
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:STATe? %d,%d\n", name, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:STATe? %d\n", name);
    }
    while (1)
    {
        Sleep(DELAYTIME);
        if ((retlen = busQuery(vi, args, strlen(args), state, 12)) == 0) {
            if (++error_count > 3)
            {
                return -1;
            }
            Sleep(DELAYTIME);
            time += DELAYTIME;
            continue;
        }
        state[retlen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "READY") == 0 || STRCASECMP(state, "IDLE") == 0) //下发过程中停止会进入“IDLE”状态
        {
            ret = 0; break;
        }
        else if (STRCASECMP(state, "ERROR") == 0) {
            ret = -2; break;
        }
        Sleep(DELAYTIME);
        time += DELAYTIME;
        if (timeout_ms > 0) {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}

/*
* 等待机器人的特定波表的运行完成状态
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* timeout_ms：等待超时时间 timeout_ms小于零表示不等待
* 返回值：0表示等待成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时;-4:参数出错
*/
EXPORT_API int CALL mrgRobotWaitEnd(ViSession vi, int name, int wavetable, int timeout_ms)
{
    int ret = -3,error_count = 0;
    char args[SEND_BUF];
    char state[100];
    int time = 0, retlen = 0;

    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:STATe? %d,%d\n", name, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:STATe? %d\n", name);
    }
    while (1)
    {
        if ((retlen = busQuery(vi, args, strlen(args), state, 100)) == 0)
        {
            if (++error_count > 30)
            {
                return -1;
            }
            Sleep(DELAYTIME);
            time += DELAYTIME;
            continue;
        }
        state[retlen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "STOP") == 0 || STRCASECMP(state, "IDLE") == 0) {
            ret = 0; break;
        }
        else if (STRCASECMP(state, "ERROR") == 0) {
            ret = -2; break;
        }
        Sleep(DELAYTIME);
        time += DELAYTIME;
        if (timeout_ms > 0) {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}
/*
* 机器人从当前位置移动到指定位置（随机移动）
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* x,y,z: 位置坐标
* time : 移动到目标位置期望使用的时间
* timeout_ms:表示等待超时时间,0表示无限等待，小于零表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
* 说明：绝对位置移动,末端保持不动
*/
EXPORT_API int CALL mrgRobotMove(ViSession vi, int name,int wavetable, float x, float y, float z, float time,int timeout_ms)
{
    char args[SEND_BUF];

    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE %d,%f,%f,%f,%f,%d\n", name, x, y, z, time,wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE %d,%f,%f,%f,%f\n", name, x, y, z, time);
    }
    
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms < 0)
    {
        return 0;
    }
    return mrgRobotWaitEnd(vi, name, wavetable, timeout_ms);
}
/*
* 机器人末端沿指定的坐标轴持续运动
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* ax: 坐标轴，0表示X轴;1表示Y轴 ; 2表示Z轴
* speed : 移动的速度。单位： 度/秒。speed的符号决定方向，speed大于零 ，表示正方向。
* 返回值：0表示执行成功，否则表示过程中出错
* 说明：非阻塞函数
*/
EXPORT_API int CALL mrgRobotMoveOn(ViSession vi, int name, int wavetable, int ax, float speed)
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:HOLD %d,%d,%f,%d\n", name, ax, speed,wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:HOLD %d,%d,%f\n", name, ax, speed);
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 机器人末端沿指定的坐标轴持续运动(阶跃运行）
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* ax: 坐标轴，0表示X轴;1表示Y轴 ; 2表示Z轴
* cr_time：爬升时间
* cr_speed：爬升的速度
* speed : 移动的保持速度。单位： 度/秒。speed的符号决定方向，speed大于零 ，表示正方向。
* 返回值：0表示执行成功，否则表示过程中出错
* 说明：非阻塞函数
*/
EXPORT_API int CALL mrgRobotMoveJog(ViSession vi, int name, int wavetable, int ax,float cr_time,float cr_speed, float speed)
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:HOLD %d,%d,%f,%f,%f,%d\n", name, ax, cr_time, cr_speed, speed, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:JOG %d,%d,%f,%f,%f\n", name, ax, cr_time, cr_speed, speed);
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 机器人从当前位置移动给定的距离（随机移动）
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* x,y,z: 位置坐标
* time : 移动到目标位置期望使用的时间
* timeout_ms:表示等待超时时间,0表示无限等待，－1表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
* 说明：相对位置移动,末端保持不动
*/
EXPORT_API int CALL mrgRobotRelMove(ViSession vi, int name, int wavetable, float x, float y, float z, float time,int timeout_ms)
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:RELATive %d,%f,%f,%f,%f,%d\n", name, x, y, z, time, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:RELATive %d,%f,%f,%f,%f\n", name, x, y, z, time);
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    return mrgRobotWaitEnd(vi, name,wavetable, timeout_ms);
}
/*
* 机器人从当前位置移动到指定位置（直线移动）
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* x,y,z: 位置坐标
* time : 移动到目标位置期望使用的时间
* timeout_ms:表示等待超时时间,0表示无限等待，－1表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
* 说明：绝对位置移动,末端保持不动
*/
EXPORT_API int CALL mrgRobotMoveL(ViSession vi, int name, int wavetable, float x, float y, float z, float time,int timeout_ms)
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:LINear %d,%f,%f,%f,%f,%d\n", name, x, y, z, time, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:LINear %d,%f,%f,%f,%f\n", name, x, y, z, time);
    }

    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    return mrgRobotWaitEnd(vi, name,wavetable, timeout_ms);
}
/*
* 机器人从当前位置移动给定的距离（直线移动）
* vi :visa设备句柄
* name: 机器人名称
* wavetable ：波表索引，－1表示使用默认索引（调用mrgSetRobotWavetable设置的波表索引）
* x,y,z: 位置坐标
* time : 移动到目标位置期望使用的时间
* timeout_ms:表示等待超时时间,0表示无限等待，小于零表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
* 说明：相对位置移动,末端保持不动
*/
EXPORT_API int CALL mrgRobotRelMoveL(ViSession vi, int name, int wavetable, float x, float y, float z, float time, int timeout_ms)
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:LINear:RELATive %d,%f,%f,%f,%f,%d\n", name, x, y, z, time, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:MOVE:LINear:RELATive %d,%f,%f,%f,%f\n", name, x, y, z, time);
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if(timeout_ms < 0)
    {
        return 0;
    }
    return mrgRobotWaitEnd(vi, name, wavetable,timeout_ms);
}
/*
* 设置机器人当前插值模式
* vi :visa设备句柄
* name: 机器人名称
* mode: 插值模式
* 返回值：0表示执行成功，－1：表示出错，
*/
EXPORT_API int CALL mrgSetRobotInterPolateMode(ViSession vi, int name,int mode)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:INTERPOLATE:MODE %d,%d\n", name, mode);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询机器人当前插值模式
* vi :visa设备句柄
* name: 机器人名称
* mode: 插值模式
* 返回值：0表示执行成功，－1：表示出错，
*/
EXPORT_API int CALL mrgGetRobotInterPolateMode(ViSession vi, int name, int* mode)
{
    char args[SEND_BUF];
    char ret[8];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:INTERPOLATE:MODE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args),ret,8)) == 0) {
        return -1;
    }
    ret[retlen] = '\0';
    *mode = atoi(ret);
    return 0;
}
/*
* 设置机器人当前插值步长
* vi :visa设备句柄
* name: 机器人名称
* step: 插值步长
* 返回值：0表示执行成功，－1：表示出错，
*/
EXPORT_API int CALL mrgSetRobotInterPolateStep(ViSession vi, int name, float step)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:INTERPOLATE:STEP %d,%f\n", name, step);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询机器人当前插值步长
* vi :visa设备句柄
* name: 机器人名称
* step: 插值步长
* 返回值：0表示执行成功，－1：表示出错，
*/
EXPORT_API int CALL mrgGetRobotInterPolateStep(ViSession vi, int name, float* step)
{
    char args[SEND_BUF];
    char ret[8];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:INTERPOLATE:STEP? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 8)) == 0) {
        return -1;
    }
    ret[retlen] = '\0';
    *step = strtod(ret,NULL);
    return 0;
}
/*
* 设置机器人回零位时使用的波表
* vi :visa设备句柄
* name: 机器人名称
* wavetable:波表索引
* 返回值：0表示执行成功，－1：表示出错
*/
EXPORT_API int CALL mrgSetRobotHomeWavetable(ViSession vi, int name, int wavetable)
{
    char args[SEND_BUF];
    if (wavetable < 0 || wavetable > 9)
    {
        return -2;
    }
    snprintf(args, SEND_BUF, "ROBOT:HOME:WAVETABLE %d,%d\n", name, wavetable);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询机器人回零位时使用的波表
* vi :visa设备句柄
* name: 机器人名称
* wavetable:返回的波表索引
* 返回值：大于等于0表示查询到的波表索引，小于零：表示出错
*/
EXPORT_API int CALL mrgGetRobotHomeWavetable(ViSession vi, int name)
{
    char args[SEND_BUF];
    char wavetable[8];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:HOME:WAVETABLE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), wavetable,8)) == 0) {
        return -1;
    }
    wavetable[retlen - 1] = 0; //去掉回车符
    if (STRCASECMP(wavetable, "MAIN") == 0) {
        return 0;
    }
    else if (STRCASECMP(wavetable, "SMALL") == 0) {
        return 1;
    }
    else if (STRCASECMP(wavetable, "P1") == 0) {
        return 2;
    }
    else if (STRCASECMP(wavetable, "P2") == 0) {
        return 3;
    }
    else if (STRCASECMP(wavetable, "P3") == 0) {
        return 4;
    }
    else if (STRCASECMP(wavetable, "P4") == 0) {
        return 5;
    }
    else if (STRCASECMP(wavetable, "P5") == 0) {
        return 6;
    }
    else if (STRCASECMP(wavetable, "P6") == 0) {
        return 7;
    }
    else if (STRCASECMP(wavetable, "P7") == 0) {
        return 8;
    }
    else if (STRCASECMP(wavetable, "P8") == 0) {
        return 9;
    }
    else {
        return -2;
    }
}
/*
* 机器人回零位操作
* vi :visa设备句柄
* name: 机器人名称
* timeout_ms:表示等待超时时间,0表示无限等待，小于零表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示执行超时
* 说明：末端保持不动
*/
EXPORT_API int CALL mrgRobotGoHome(ViSession vi, int name,int timeout_ms)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:HOME:RUN %d\n", name);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms < 0)
    {
        return 0;
    }
    return mrgRobotWaitHomeEnd(vi, name, timeout_ms); //等待波表 SMALL
}
/*
* 机器人回零位操作
* vi :visa设备句柄
* name: 机器人名称
* param: 参数，对于T4来说，指的是时间，即在多秒时间内回到零位。对于H2来说，指的是回零位的速度，度/秒
* timeout_ms:表示等待超时时间,0表示无限等待，小于零表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示执行超时
* 说明：末端保持不动
*/
EXPORT_API int CALL mrgRobotGoHomeWithParam(ViSession vi, int name,float param, int timeout_ms)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:HOME:RUN %d,%f\n", name, param);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms < 0)
    {
        return 0;
    }
    return mrgRobotWaitHomeEnd(vi, name, timeout_ms); //等待波表 SMALL
}
/*
* 停止机器人回零位操作
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功，-1:表示失败
*/
EXPORT_API int CALL mrgRobotGoHomeStop(ViSession vi, int name)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:HOME:STOP %d\n", name);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 等待机器人回零位结束状态（等待运行完成）
* vi :visa设备句柄
* name: 机器人名称
* timeout_ms：等待超时时间，零表示无限等待, 小于零表示不等待
* 返回值：0表示等待成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时;-4: 超时时间非法
*/
EXPORT_API int CALL mrgRobotWaitHomeEnd(ViSession vi, int name, int timeout_ms)
{
    int ret = 0,error_count = 0;
    char args[SEND_BUF];
    char state[12];
    int time = 0,retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:HOME:STATe? %d\n", name);
    if (timeout_ms < 0)
    {
        return -4;
    }
    while (1)
    {
        if ((retlen = busQuery(vi, args, strlen(args), state, 12)) == 0) {
            if (++error_count > 3)
            {
                return -1;
            }
            continue;
        }
        state[retlen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "STOP") == 0 || STRCASECMP(state, "IDLE") == 0) {
            ret = 0; break;
        }
        else if (STRCASECMP(state, "ERROR") == 0) {
            ret = -2; break;
        }
        Sleep(DELAYTIME);
        time += DELAYTIME;
        if (timeout_ms != 0) {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}
/*
* 获取机器人在原点时的各关节的角度
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功，否则表示失败
* 说明：angles是不安全的，请在外部确保angles的空间足够
*/
EXPORT_API int CALL mrgGetRobotHomeAngle(ViSession vi, int name,float * angles)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "ROBOT:HOME:ANGLE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen] = 0;
    p = STRTOK_S(tmp, ",", &pNext);
    while (p)
    {
        *angles++ = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    return count;
}
/*
* 获取机器人在零位时，末端的坐标点值
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgGetRobotHomePosition(ViSession vi, int name, float * x, float *y, float* z)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    float position[3];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "ROBOT:HOME:POSITION? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen] = 0;
    p = STRTOK_S(tmp, ",", &pNext);
    while (p)
    {
        position[count] = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    *x = position[0];
    *y = position[1];
    *z = position[2];
    return 0;
}
/*
* 设置机器人的回零方式
* vi :visa设备句柄
* name: 机器人名称
* mode: 回零方式。 0： 先x后y ； 1：先y后x
* 返回值：0表示执行成功，－1：表示出错，
* 说明：此命令目前只对H2有效
*/
EXPORT_API int CALL mrgSetRobotHomeMode(ViSession vi, int name, int mode)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:HOME:MODE %d,%d\n", name, mode);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询机器人的回零方式
* vi :visa设备句柄
* name: 机器人名称
* 返回值：大于等于0表示回零方式，否则表示出错，
* 说明：此命令目前只对H2有效
*/
EXPORT_API int CALL mrgGetRobotHomeMode(ViSession vi, int name)
{
    char args[SEND_BUF];
    char ret[8];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:HOME:MODE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 8)) == 0) {
        return -1;
    }
    ret[retlen] = '\0';
    return atoi(ret);
}
/*
* 查询机器人的是否需要回零
* vi :visa设备句柄
* name: 机器人名称
* 返回值：1表示需要回零 ，0表示不需要回零，小于零表示出错，
* 说明：
*/
EXPORT_API int CALL mrgGetRobotHomeRequire(ViSession vi, int name)
{
    char args[SEND_BUF];
    char ret[8];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:HOME:REQUIRE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), ret, 8)) == 0) {
        return -1;
    }
    ret[retlen] = '\0';
    return atoi(ret);
}

/*
* 给指定的机器人加载坐标点
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功，否则表示失败
* 说明：在调 用mrgRobotPointLoad（）函数前，请使用此函数清空上次的坐标点
*/
EXPORT_API int CALL mrgRobotPointClear(ViSession vi, int name)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF,"ROBOT:POINT:CLEAR %d\n", name);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 给指定的机器人加载坐标点
* vi :visa设备句柄
* name: 机器人名称
* x,y,z: 坐标点信息
* end: 末端执行器电机转动的角度值
* time: 时间，表示当前点在主时间轴上的位置
* 返回值：0表示执行成功，否则表示失败
* 说明：此函数只是将上位机的坐标点信息下载到MRG中，MRG并未开始解算.
*  另，  在调用此函数开始下发坐标点前，务必使用mrgRobotPointClear()函数，通知机器人清空其缓存中的坐标点。
*/
EXPORT_API int CALL mrgRobotPointLoad(ViSession vi, int name, float x, float y, float z, float end, float time,int mod,float step)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:POINT:LOAD %d,%f,%f,%f,%f,%f,%d,%f\n", name,x,y,z,end,time,mod,step);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 通知机器人开始解算其缓存中的坐标点，并下发给模块设备，直到模块设备解算完成
* vi :visa设备句柄
* name: 机器人名称
* wavetable: 解算到指定的波表。 如果为－1，表示解算到当前机器人默认的波表中
* timeout_ms: 等待解算的超时时间,0表示无限等待，－1表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
*/
EXPORT_API int CALL mrgRobotPointResolve(ViSession vi, int name, int wavetable,int timeout_ms )
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:POINT:RESOLVe %d,%d\n", name, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:POINT:RESOLVe %d\n", name);
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    return mrgRobotWaitReady(vi, name,wavetable,0);
}
/*
* 通知机器人清空PVT缓存
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功，否则表示失败
* 说明：在调 用mrgRobotPvtLoad（）函数前，请使用此函数清空上次的PVT
*/
EXPORT_API int CALL mrgRobotPvtClear(ViSession vi, int name)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:PVT:CLEAR %d\n", name);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 给指定的机器人加载坐标点
* vi :visa设备句柄
* name: 机器人名称
* p,v,t: pvt信息
* axle: 轴索引
* 返回值：0表示执行成功，否则表示失败
* 说明：此函数只是将上位机的坐标点信息下载到MRG中，MRG并未开始解算.
*  另，  在调用此函数开始下发坐标点前，务必使用mrgRobotPvtClear()函数，通知机器人清空其缓存中的坐标点。
*/
EXPORT_API int CALL mrgRobotPvtLoad(ViSession vi, int name, float p, float v, float t, int axle)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:PVT:LOAD %d,%f,%f,%f,%d\n", name, p, v, t,axle);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 通知机器人开始下发其缓存中的PVT到模块设备，直到模块设备解算完成
* vi :visa设备句柄
* name: 机器人名称
* wavetable: 解算到指定的波表。 如果为－1，表示解算到当前机器人默认的波表中
* timeout_ms: 等待解算的超时时间,0表示无限等待，－1表示不等待，立即返回
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
*/
EXPORT_API int CALL mrgRobotPvtResolve(ViSession vi, int name, int wavetable,int timeout_ms)
{
    char args[SEND_BUF];
    if (wavetable >= WAVETABLE_MIN && wavetable <= WAVETABLE_MAX)
    {
        snprintf(args, SEND_BUF, "ROBOT:PVT:RESOLVe %d,%d\n", name, wavetable);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:PVT:RESOLVe %d\n", name);
    }

    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    return mrgRobotWaitReady(vi, name, wavetable, timeout_ms);
}

/*
* 从本地存储器中，导入运动文件到机器人缓存中
* vi :visa设备句柄
* name: 机器人名称
* filename: 点坐标文件名
* 返回值：0表示执行成功，否则表示失败
* 说明 : 此函数同mrgRobotMotionFileImportLocal  ,只是为了兼容老版本
*/
EXPORT_API int CALL mrgRobotMotionFileImport(ViSession vi,int name,char* filename)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:FILE:IMPORT %d,%s\n",name, filename);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 从本地存储器中，导入运动文件到机器人缓存中
* vi :visa设备句柄
* name: 机器人名称
* filename: 点坐标文件名
* 返回值：0表示执行成功，否则表示失败
* 说明 : 
*/
EXPORT_API int CALL mrgRobotMotionFileImportLocal(ViSession vi, int name, char* filename)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:FILE:IMPORT:LOCal %d,%s\n", name, filename);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 从外部存储器中，导入运动文件到机器人缓存中
* vi :visa设备句柄
* name: 机器人名称
* filename: 点坐标文件名
* 返回值：0表示执行成功，否则表示失败
* 说明 : 
*/
EXPORT_API int CALL mrgRobotMotionFileImportExternal(ViSession vi, int name, char* filename)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:FILE:IMPORT:EXTERnal %d,%s\n", name, filename);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 解算当前运动文件内容到模块中
* vi :visa设备句柄
* name: 机器人名称
* section:文件中的哪个段，这是个必须的参数
* line：一个段中的哪一行（只针对MFC的文件），line从1开始计数。对于非MFC的文件，不关心line值。
* wavetable : 波表索引。如果不想明确指定波表，可设置 为-1.
* timeout_ms:等等解算完成的超时时间。若timeout_ms＝－1，表示不等待解算完成。timeout_ms ＝ 0，表示无限等待。
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
*
*/
EXPORT_API int CALL mrgRobotFileResolve(ViSession vi, int name, int section, int line, int wavetable,int timeout_ms)
{
    char args[SEND_BUF];
    if (line == 0 || line < 0)
    {
        snprintf(args, SEND_BUF, "ROBOT:FILE:RESOLVE %d,%d\n", name,section);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:FILE:RESOLVE %d,%d,%d,%d\n", name, section,line,wavetable);
    }
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    return mrgRobotWaitReady(vi, name, wavetable, timeout_ms);
}
/*
* 将系统中的运动数据，导出成文件
* vi :visa设备句柄
* name: 机器人名称
* type:0表示导出到本地存储（本地文件系统）；1表示导出到外部存储（U盘之类）
* filename：表示导出的文件名
* 返回值：0表示执行正确，否则表示失败。
*/
EXPORT_API int CALL mrgRobotMotionFileExport(ViSession vi, int name,int location, char* filename)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:FILE:EXPORT:%s %d,%s\n", location ? "LOCAL" : "EXTERNAL",name,filename);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 设置末端执行器类型及相应的设备
* vi :visa设备句柄
* name: 机器人名称
* type: 末端执行器类型 0->爪子
* dev : 末端执行器对应的通道设备 (1@513), 哪个设备的哪个轴
* 返回值：0表示执行成功，－1：表示出错
*/
EXPORT_API int CALL mrgRobotToolSet(ViSession vi, int robotname, int type, char* dev)
{
    char args[SEND_BUF];
    char *effector[] = {"CLAW","MR_F3","MR_TIP","MR_FR"};
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:SET %d,%s,(%s)\n",robotname, effector[type], dev);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询末端执行器类型
* vi :visa设备句柄
* name: 机器人名称
* type: 末端执行器类型 0->爪子
* dev : 末端执行器对应的通道设备 (1@513), 哪个设备的哪个轴
* 返回值：0表示执行成功，－1：表示出错
*/
EXPORT_API int CALL mrgRobotGetToolType(ViSession vi, int robotname, int * type)
{
    int ret = 0, error_count = 0;
    char args[SEND_BUF];
    char state[12];
    int retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:TYPe? %d\n", robotname);
    while (1)
    {
        if ((retlen = busQuery(vi, args, strlen(args), state, 12)) == 0) {
            if (++error_count > 3)
            {
                return -1;
            }
            continue;
        }
        state[retlen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "MR-F2") == 0 || STRCASECMP(state, "CLAW") == 0)
        {
            *type = 0;
        }
        else if (STRCASECMP(state, "MR-F3") == 0)
        {
            *type = 1;
        }
        else if (STRCASECMP(state, "MR-TIP") == 0)
        {
            *type = 2;
        }
    }
    return ret;
}
/*
* 等待末端执行器执行完成
* vi :visa设备句柄
* name: 机器人名称
* timeout_ms : 末端执行器执行的超时时间, 0表示无限等待
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示执行超时
*/
EXPORT_API int CALL mrgRobotWaitToolExeEnd(ViSession vi, int name,int timeout_ms)
{
    int ret = 0,error_count = 0;
    char args[SEND_BUF];
    char state[12];
    int time = 0,retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:EXEC:STATe? %d\n", name);
    while (1)
    {
        if ((retlen = busQuery(vi, args, strlen(args), state, 12)) == 0) {
            if (++error_count > 3)
            {
                return -1;
            }
            continue;
        }
        state[retlen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "STOP") == 0 || STRCASECMP(state, "IDLE") == 0) {
            ret = 0; break;
        }
        else if (STRCASECMP(state, "ERROR") == 0) {
            ret = -2; break;
        }
        Sleep(DELAYTIME);
        time += DELAYTIME;
        if (timeout_ms > 0) {
            if (time > timeout_ms) {
                mrgRobotToolStop(vi, name);
                ret = -3; break;
            }
        }
    }
    return ret;
}
/*
* 执行末端执行器
* vi :visa设备句柄
* name: 机器人名称
* position: 末端执行器电机转动的角度
* time : 末端执行器电机转动的角度所用时间
* timeout_ms : 末端执行器执行的超时时间, 0表示无限等待; -1表示不等待
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示执行超时
*/
EXPORT_API int CALL mrgRobotToolExe(ViSession vi, int name, float position,float time,int timeout_ms)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:EXEC %d,%f,%f\n",name,position,time);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    return mrgRobotWaitToolExeEnd(vi,name,timeout_ms);
}
/*
* 停止末端执行器
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功，否则表示失败
*/
EXPORT_API int CALL mrgRobotToolStop(ViSession vi, int name)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:EXEC:STOP %d\n", name);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 末端执行器执行模式
* vi :visa设备句柄
* name: 机器人名称
* mode: 末端执行器执行模式. 0:正常模式; 1:先张开,再闭合到目标位置
* 返回值：0表示执行成功，－1：表示出错，
*/
EXPORT_API int CALL mrgRobotToolExeMode(ViSession vi, int name, int mode)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:EXEC:MODe %d,%s\n", name,mode == 1? "OPEN":"NORMAL");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询末端执行器执行模式
* vi :visa设备句柄
* name: 机器人名称
* mode: 末端执行器执行模式. 0:正常模式; 1:先张开,再闭合到目标位置
* 返回值：0表示执行成功，－1：表示出错，
*/
EXPORT_API int CALL mrgRobotToolExeMode_Query(ViSession vi, int name, int* mode)
{
    char args[SEND_BUF];
    char state[12];
    int  retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:EXEC:MODe? %d\n", name);
    
    if ((retlen = busQuery(vi, args, strlen(args), state, 12)) == 0)
    {
        return -1;
    }
    state[retlen - 1] = '\0';//去掉回车符
    if (STRCASECMP(state, "NORMAL") == 0 ) {
        *mode = 0;
    }
    else if (STRCASECMP(state, "open") == 0) {
        *mode = 1;
    }
    else
    {
        return -2;
    }
    return 0;
}
/*
* 中止末端执行器回初始位
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功，否则表示失败
*/
EXPORT_API int CALL mrgRobotToolStopGoHome(ViSession vi, int name)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:HOME:STOP %d\n", name);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 等待末端执行器回零位完成
* vi :visa设备句柄
* name: 机器人名称
* timeout_ms : 回零位的等待超时时间
* 返回值：0表示执行成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示执行超时
*/
EXPORT_API int CALL mrgRobotWaitToolHomeEnd(ViSession vi, int name, int timeout_ms)
{
    int ret = 0,error_count = 0;
    char args[SEND_BUF];
    char state[12];
    int time = 0, retlen = 0;
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:HOME:STATe? %d\n", name);
    while (1)
    {
        if ((retlen = busQuery(vi, args, strlen(args), state, 12)) == 0) {
            if (++error_count > 3)
            {
                return -1;
            }
            continue;
        }
        state[retlen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "STOP") == 0 || STRCASECMP(state, "IDLE") == 0) {
            ret = 0; break;
        }
        else if (STRCASECMP(state, "ERROR") == 0) {
            ret = -2; break;
        }
        Sleep(DELAYTIME);
        time += DELAYTIME;
        if (timeout_ms > 0) {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}
/*
* 末端执行器回初始位
* vi :visa设备句柄
* name: 机器人名称
* timeout_ms: 等待的超时时间, 0表示无限等待; -1表示不等待
* 返回值：0表示执行成功，否则表示失败
*/
EXPORT_API int CALL mrgRobotToolGoHome(ViSession vi, int name,int timeout_ms)
{
    char args[SEND_BUF];
    int ret = 0;
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:HOME %d\n", name);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    ret = mrgRobotWaitToolHomeEnd(vi, name, timeout_ms);
    return ret;
}
/*
* 获取机器人末端执行器的位置
* vi :visa设备句柄
* name: 机器人名称
* position:末端执行器的位置(相对于机械零点),单位:角度.
* 返回值：0表示执行正确, 否则执行失败
*/
EXPORT_API int CALL mrgGetRobotToolPosition(ViSession vi, int name, float * position)
{
    int  retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    snprintf(args, SEND_BUF, "ROBOT:EFFECTor:POSITion:CURRent? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen-1] = 0;
    *position = atof(tmp);
    return 0;
}

/*
* 获取机器人当前各关节的角度值
* vi :visa设备句柄
* name: 机器人名称
* 返回值：大于零 表示返回角度值的个数，小于等于零表示出错
* 说明: angles是不安全的，请在外部确保angles的空间足够
*/
EXPORT_API int CALL mrgGetRobotCurrentAngle(ViSession vi, int name,float * angles)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "ROBOT:CURRENT:ANGLE? %d\n",name);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen] = 0;
    p = STRTOK_S(tmp, ",", &pNext);
    while (p)
    {
        *angles++ = strtof(p,NULL);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    return count;
}
/*
* 获取机器人末端的位置坐标
* vi :visa设备句柄
* name: 机器人名称
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgGetRobotCurrentPosition(ViSession vi, int name, float * x,float *y ,float* z)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    float position[3];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "ROBOT:CURRENT:POSITION? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen] = 0;
    p = STRTOK_S(tmp, ",", &pNext);
    while (p)
    {
        position[count] = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    *x = position[0];
    *y = position[1];
    *z = position[2];
    return 0;
}
/*
* 机器人某一个轴回零
* vi :visa设备句柄
* name: 机器人名称
* axi :轴索引
* speed: 回零速度,单位:度/秒
* timeout_ms:表示等待回零结束的超时时间. 如果为-1,表示不等待. 0表示无限等待. >0 表示等待的超时时间. 单位:ms
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgRobotJointHome(ViSession vi, int name, int axi, float speed,int timeout_ms)
{
    int retlen = 0;
    char args[SEND_BUF];

    snprintf(args, SEND_BUF, "ROBOT:JOINT:HOMe %d,%d,%f\n", name, axi, speed);
    if ((retlen = busWrite(vi, args, strlen(args))) == 0)
    {
        return -1;
    }
    if (timeout_ms == -1)
    {
        return 0;
    }
    return mrgRobotWaitHomeEnd(vi,name,timeout_ms);
}
/*
* 控制机器人某一个轴运动
* vi :visa设备句柄
* name: 机器人名称
* axi :轴索引
* position: 轴运行的距离,单位: 度
* time : 轴运动时所需要的时间. 单位:秒
* timeout_ms:表示等待回零结束的超时时间. 如果为-1,表示不等待. 0表示无限等待. >0 表示等待的超时时间. 单位:ms
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgRobotJointMove(ViSession vi, int name, int axi, float position,float time, int timeout_ms)
{
    int retlen = 0;
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:JOINT:MOVE %d,%d,%f,%f\n", name, axi, position, time);
    if ((retlen = busWrite(vi, args, strlen(args))) == 0)
    {
        return -1;
    }
    if (timeout_ms < 0)
    {
        return 0;
    }
    return mrgRobotWaitEnd(vi, name, DEFAULT_WAVETABLE,timeout_ms);
}
/*
* 控制机器人某一个轴持续运动
* vi :visa设备句柄
* name: 机器人名称
* axi :轴索引
* speed: 轴运行的速度,单位: 度/秒
* wavetable : 波表
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgRobotJointMoveOn(ViSession vi, int name, int axi, float speed)
{
    int retlen = 0;
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:JOINT:MOVE:HOLD %d,%d,%f\n", name, axi, speed);
    if ((retlen = busWrite(vi, args, strlen(args))) == 0)
    {
        return -1;
    }
    
    return 0;
}
/*
* 获取机器人各关节的当前角度
* vi :visa设备句柄
* name: 机器人名称
* joint:指定机器人的关节索引. 0表示第一个关节(对T4来说,0指的是基座, 对H2来说,0指的是左边的关节). 如果为-1,则表示获取所有的关节
* 返回值：大于零,表示返回的关节角度个数, 否则,出错
* 说明: angle 是不安全的,请确保外部分配足够的空间给angle
*/
EXPORT_API int CALL mrgGetRobotJointAngle(ViSession vi, int name, int joint, float *angle)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    char * p, *pNext = NULL;
    if (joint < 0)
    {
        snprintf(args, SEND_BUF, "ROBOT:JOINT:ANGLE? %d\n", name);
    }
    else
    {
        snprintf(args, SEND_BUF, "ROBOT:JOINT:ANGLE? %d,%d\n", name, joint);
    }
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return 0;
    }
    tmp[retlen] = 0;
    p = STRTOK_S(tmp, ",", &pNext);
    while (p)
    {
        angle[count] = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    return count;
}
/*
* 机器人当前的里程数，单位 ：米
* vi :visa设备句柄
* name: 机器人名称
* x,y,z ：各坐标轴方向上的里程
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgGetRobotCurrentMileage(ViSession vi, int name, float * x, float *y, float* z)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    float position[3];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "ROBOT:CURRENT:MILEAGE? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen] = 0;
    p = STRTOK_S(tmp, ",", &pNext);
    while (p)
    {
        position[count] = strtof(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    *x = position[0];
    *y = position[1];
    *z = position[2];
    return 0;
}
/*
* 获取机器人的目标位置
* vi :visa设备句柄
* name: 机器人名称
* x,y,z ：各坐标轴方向上的点
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgGetRobotTargetPosition(ViSession vi, int name, float * x, float *y, float* z)
{
    int count = 0, retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    float position[3];
    char * p, *pNext = NULL;
    snprintf(args, SEND_BUF, "ROBOT:TARGET? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen] = 0;
    p = STRTOK_S(tmp, ",", &pNext);
    while (p)
    {
        position[count] = strtod(p, NULL);
        p = STRTOK_S(NULL, ",", &pNext);
        count++;
    }
    *x = position[0];
    *y = position[1];
    *z = position[2];
    return 0;
}
/*
* 获取机器人的当前执行的指令索引
* vi :visa设备句柄
* name: 机器人名称
* x,y,z ：各坐标轴方向上的点
* 返回值：0表示执行成功， －1：表示执行失败
*/
EXPORT_API int CALL mrgGetRobotCurrentRecord(ViSession vi, int name, int *record)
{
    int retlen = 0;
    char args[SEND_BUF];
    char as8Ret[100];
    snprintf(args, SEND_BUF, "ROBOT:CURRENT:RECORD? %d\n", name);
    if ((retlen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0)
    {
        return -1;
    }
    *record = atoi(as8Ret);
    return 0;
}
/*
* 机器人的折叠功能(包装位)
* vi :visa设备句柄
* name: 机器人名称
* axi0 axi1,axi2,axi3：各轴相对于零点的角度值. axi0:基座; axi1:大臂;axi2:小臂;axi3:腕 
* 返回值：0表示执行成功， －1：表示执行失败
* 此命令只对T4有效！！！！！
*/
EXPORT_API int CALL mrgGetRobotFold(ViSession vi, int name, int wavetable, float axi0, float axi1, float axi2, float axi3)
{
    int device = 0;
    //如果不是T4,则返回错误
    if (mrgGetRobotType(vi,name) != MRX_TYPE_T4)
    {
        return -1;
    }
    if (mrgGetRobotDevice(vi, name, &device) == 0)
    {
        return -2; //获取设备失败
    }
    //0. 机器人本体回零位
    mrgRobotGoHome(vi,name, 0);
    //1. 先动末端执行器
    mrgMRQAdjust(vi, device, 4, wavetable, 10.0f,1.0f, 2000);
    mrgRobotToolGoHome(vi, name, 10000); //末端回零

    //2.腕
    mrgMRQAdjust(vi, device, 3, wavetable, axi3, 3.0f, 10000);
    //3. 大臂向后
    mrgMRQAdjust(vi, device, 1, wavetable, axi1, 2.0f, 10000);

    //4.小臂向下
    mrgMRQAdjust(vi, device, 2, wavetable, axi2, 3.0f, 10000);

    //5. 基座
    mrgMRQAdjust(vi, device, 2, wavetable, axi0, 1.0f, 10000);
    return 0;
}

/*
* 控制机器人腕关节的转动角度(相对于90度的算法零位)
* vi :visa设备句柄
* name: 机器人名称
* angle: 腕关节角度(垂直向下时为零)
* time : 时间
* timeout_ms: 表示等待执行的超时时间. 如果为-1,表示不等待. 0表示无限等待. >0 表示等待的超时时间. 单位:ms
* 返回值：零表示执行正确,-1表示执行错误
*/
EXPORT_API int CALL mrgSetRobotWristAngleZero(ViSession vi, int name, float angle,float time,int timeout_ms)
{
    int retlen = 0;
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "ROBOT:JOINT:MOVE %d,%d,%f,%f\n", name, 3, angle - 90.0f, time);
    if ((retlen = busWrite(vi, args, strlen(args))) == 0)
    {
        return -1;
    }
    if (timeout_ms < 0)
    {
        return 0;
    }
    return mrgRobotWaitEnd(vi, name, DEFAULT_WAVETABLE, timeout_ms);
}
/*
* 获取机器人腕关节的当前角度(相对于90度的算法零位)
* vi :visa设备句柄
* name: 机器人名称
* angle: 腕关节角度(垂直向下时为零)
* 返回值：零表示执行正确,-1表示执行错误
*/
EXPORT_API int CALL mrgGetRobotWristAngleZero(ViSession vi, int name, float *angle)
{
    int retlen = 0;
    char args[SEND_BUF];
    char tmp[100];
    snprintf(args, SEND_BUF, "ROBOT:JOINT:ANGLE? %d,%d\n", name, 3);
    if ((retlen = busQuery(vi, args, strlen(args), tmp, 100)) == 0)
    {
        return -1;
    }
    tmp[retlen] = 0;
    *angle = atof(tmp) - 90.0f;
    return 0;
}
