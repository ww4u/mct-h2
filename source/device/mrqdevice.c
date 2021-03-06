#include "mrqdevice.h"

#define  SEND_BUF   (100)

static char *wavetableToString(int wavetable)
{
    char *ps8Wave[10] = { "MAIN","SMALL","P1","P2", "P3", "P4", "P5", "P6", "P7","P8"};
    return ps8Wave[wavetable];
}
static char *changeResponseToString(int state)
{
    char *ps8State[4] = { "NONE" ,"ALARM" ,"STOP" ,"ALARM&STOP" };
    return ps8State[state];
}
static char *changeLevelTrigTypeToString(int type)
{
    char *ps8Type[5] = { "RESERVE" ,"LOW" ,"RISE" ,"FALL" , "HIGH" };
    return ps8Type[type];
}
char *changeReportFuncToString(int fun1)
{
    char *fun[6] = { "TORQUE" ,"CYCLE" ,"SGALL" ,"SGSE","DIST","ABSEN" };
    return fun[fun1];
}
char *changeSwitchStateToString(int fun1)
{
    char *fun[6] = { "RESET" ,"STOP" ,"RUN" ,"PREPARE","EMERGSTOP"};
    return fun[fun1];
}
char *motionStateToString(int fun1)
{
    char *fun[7] = { "POWERON" ,"IDLE" ,"CALCING" ,"CALCEND","STANDBY","RUNNING","ERROR" };
    return fun[fun1];
}
/*********************** MRQ 设备操作 *******************************/
/*
*MRQ模块识别
*vi :visa设备句柄
*name：设备名称(SEND_ID)
*state：识别状态 0 ON 1 OFF
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQIdentify(ViSession vi, int name, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:IDENtify %d,%s\n", name, state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}

/*
*查询设备模块的IO 状态
*vi :visa设备句柄
*name :设备名称
* state : DIO state
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetMRQDioState(ViSession vi, int name, unsigned short * state)
{
    char args[SEND_BUF];
    char as8Ret[100] = { 0 };
    int len = 0;
    snprintf(args, SEND_BUF, "DEVice:DIOSTATe? %d", name);
    if ((len = busQuery(vi, args, strlen(args), as8Ret, 20)) == 0) {
        return -1;
    }
    else {
        as8Ret[len - 1] = '\0';
        *state = atoi(as8Ret);
        return 0;
    }
}
/*
* 将指定的设备分在一个组中
* vi :visa设备句柄
* devList :设备名称 "512,513,514"
* groupID : 组ID,由下层返回.
* grouptype: 0:GOUPID1, 1:GROUPID2
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgGetMRQGroup(ViSession vi, char * devList, unsigned int * groupID,int grouptype)
{
    char args[SEND_BUF];
    char as8Ret[100] = { 0 };
    int len = 0;
    snprintf(args, SEND_BUF, "DEVice:GROUP:ALLOC? (%s),%s", devList, grouptype?"GROUPID2": "GROUPID1");
    if ((len = busQuery(vi, args, strlen(args), as8Ret, 20)) == 0) {
        return -1;
    }
    else {
        as8Ret[len - 1] = '\0';
        *groupID = atoi(as8Ret);
        return 0;
    }
}

/*
*设置运行状态是否自动上报给微控器
*vi :visa设备句柄
*name：设备名称(SEND_ID)
*ch：通道号
*state：状态 0 ACTIVE 1 QUERY
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionStateReport(ViSession vi, int name, int ch, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:STATe:REPORt %d,%d,%s\n",
             name, ch, state == 0 ? "ACTIVE" : "QUERY");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*查询上报状态
*vi :visa设备句柄
*name设备名称(SEND_ID)
*ch：通道号
*state:  0 ACTIVE 1 QUERY；
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionStateReport_Query(ViSession vi, int name, int ch,int *state)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100] = { 0 };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:STATe:REPORt? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"QUERY") == 0 || STRCASECMP(as8Ret,"1") == 0)
    {
        *state = 1;
    }
    else if (STRCASECMP(as8Ret, "ACTIVE") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *state = 0;
    }
    else{
        return -2;
    }
    return 0;
}
/*
*运行指定的波表
*vi :visa设备句柄
* name: 设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引值，取值范围0~9
*返回值：0表示执行成功，－1表示失败
* 在使用此函数时,需要确保模块已进入解算完成状态.使用 mrgMRQMotionRunState_Query
*/
EXPORT_API int CALL mrgMRQMotionRun(ViSession vi, int name, int ch, int wavetable)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:RUN %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询运行状态 
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引值，取值范围0~9
*state:机器人的状态 0:IDLE; 1:LOADING;2:READY;3:RUNNING;4:STOP; 5:ERROR; 
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionRunState_Query(ViSession vi, int name, 
                                               int ch, int wavetable, int*state)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:RUN:STATe? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"IDLE") == 0 || STRCASECMP(as8Ret,"0") == 0)
    {
        *state = 0;
    }
    else if(STRCASECMP(as8Ret,"LOADING") == 0 || STRCASECMP(as8Ret,"1") == 0)
    {
        *state = 1;
    }
    else if(STRCASECMP(as8Ret,"READY") == 0 || STRCASECMP(as8Ret,"2") == 0)
    {
        *state = 2;
    }
    else if(STRCASECMP(as8Ret,"RUNNING") == 0 || STRCASECMP(as8Ret,"3") == 0)
    {
        *state = 3;
    }
    else if(STRCASECMP(as8Ret,"STOP") == 0 || STRCASECMP(as8Ret,"4") == 0)
    {
        *state = 4;
    }
    else{
        *state = 5;
    }
    return 0;
}
/*
* 等待当前设备指定通道的特定波表的ready状态（等待模块设备解算完成）
* vi :visa设备句柄
* name: 设备名称(SEND_ID)
* ch : 通道索引
* wavetable ：波表索引。不允许为空
* timeout_ms：等待超时时间。0表示无限等待。不允许小于零
* 返回值：0表示等待成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
*/
EXPORT_API int CALL mrgMRQMotionWaitReady(ViSession vi, int name, int ch, int wavetable, int timeout_ms)
{
    int ret = -3,error_count = 0;
    char args[SEND_BUF];
    char state[12];
    int retLen = 0,time = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:RUN:STATe? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    while (1)
    {
        msSleep(200);
        if ((retLen = busQuery(vi, args, strlen(args), state, 12)) == 0) {
            if (++error_count > 3)
            {
                return -1;
            }
            continue;
        }
        state[retLen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "READY") == 0 || STRCASECMP(state, "IDLE") == 0) //下发过程中停止会进入“IDLE”状态
        {
            ret = 0; break;
        }
        else if (STRCASECMP(state, "ERROR") == 0) {
            ret = -2; break;
        }
        time += 200;
        if (timeout_ms > 0)
        {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}
/*
* 等待当前设备指定通道的特定波表的运行结束状态
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch : 通道索引
* wavetable ：波表索引。不允许为空
* timeout_ms：等待超时时间。0表示无限等待。不允许小于零
* 返回值：0表示等待成功，－1：表示等待过程中出错，－2：表示运行状态出错；－3：表示等待超时
*/
EXPORT_API int CALL mrgMRQMotionWaitEnd(ViSession vi, int name, int ch, int wavetable, int timeout_ms)
{
    int ret = -3;
    int error_count = 0;
    char args[SEND_BUF];
    char state[12];
    int retLen = 0;
    int time = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:RUN:STATe? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    while (1)
    {
        msSleep(200);
        if ((retLen = busQuery(vi, args, strlen(args), state, 12)) == 0) {
            if (++error_count > 3)
            {
                return -1;
            }
            continue;
        }
        state[retLen - 1] = '\0';//去掉回车符
        if (STRCASECMP(state, "READY") == 0 || STRCASECMP(state, "IDLE") == 0) //下发过程中停止会进入“IDLE”状态
        {
            ret = 0; break;
        }
        else if (STRCASECMP(state, "ERROR") == 0) {
            ret = -2; break;
        }
        time += 200;
        if (timeout_ms > 0)
        {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}

/*
*停止指定的波表
*vi :visa设备句柄
* name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引值，取值范围0~9
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionStop(ViSession vi, int name, int ch, int wavetable)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:STOP %d,%d,%d\n", name, ch, wavetable);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置启动运行的触发源
*vi :visa设备句柄
* name:设备名称(SEND_ID)
*ch：通道号
*source:触发源 0:SOFTWARE 1:DIGITALIO 2:CAN 3:ALL
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionTrigSource(ViSession vi, int name, int ch, int source)
{
    char args[SEND_BUF];
    char *ps8Source[5] = { "SOFTWARE" ,"DIGITALIO" ,"CAN" ,"ALL" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:TRIGger:SOURce %d,%d,%s\n",
             name, ch, ps8Source[source]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询启动运行触发源
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*source:触发源 0:SOFTWARE 1:DIGITALIO 2:CAN 3:ALL
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionTrigSource_Query(ViSession vi, int name, int ch, int * source)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:TRIGger:SOURce? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"SOFTWARE") == 0)
    {
        *source = 0;
    }
    else if(STRCASECMP(as8Ret,"DIGITALIO") == 0)
    {
        *source = 1;
    }
    else if(STRCASECMP(as8Ret,"CAN") == 0)
    {
        *source = 2;
    }
    else{
        *source = 3;
    }
    return 0;
}
/*
*设置电机未运动时发生位移是否上报给微控器
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:状态
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionOffsetState(ViSession vi, int name, int ch, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:OFFSet:STATe %d,%d,%s\n",
             name, ch, state == 0 ? "OFF" : "ON");
    if (busWrite(vi, args, strlen(args)) <= 0)
    {
        return -1;
    }
    return 0;
}
/*
*查询电机未运动时发生位移是否上报给微控器状态
*vi :visa设备句柄
*name: 机器人名称
*ch：通道号
*source:状态  0:OFF ; 1: ON
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionOffsetState_Query(ViSession vi, int name, int ch, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:OFFSet:STATe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"ON") == 0 || STRCASECMP(as8Ret,"1") == 0 )
    {
        *state = 1;
    }
    else{
        *state = 0;
    }
    return 0;
}
/*
*查询电机未运动时发生的位移
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*displace:位移
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionOffsetValue_Query(ViSession vi, int name, int ch, float *distance)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:OFFSet:VALue? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen] = '\0';
    }
    *distance = strtod(as8Ret,NULL);
    return 0;
}
/*
*查询增量编码器的AB相的计数值
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*返回值：AB相的计数值
*/
EXPORT_API int CALL mrgMRQMotionABCount_Query(ViSession vi, int name, int ch)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF,  "DEVICE:MRQ:MOTion:ABCOUNt? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return 0;
    }
    else {
        as8Ret[retLen-1] = '\0';
        return atoi(as8Ret);
    }
}
/*
*清空增量编码器的AB相的计数值
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionABCountClear(ViSession vi, int name, int ch)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:ABCOUNt:CLEAr %d,%d\n", name, ch);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置电机是否反向
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*state：是否反向
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionReverse(ViSession vi, int name, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:REVERSe %d,%d\n", name, state);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机反向开关状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*reverse: 0表示没有反向；1表示反向；
*返回值：0表示执行成功；－1表示失败
*/
EXPORT_API int CALL mrgMRQMotionReverse_Query(ViSession vi, int name,int * reverse)
{
    char args[SEND_BUF];
    char as8Ret[8];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTion:REVERSe? %d\n", name);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 8)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "OFF") == 0)
    {
        *reverse = 0;
    }
    else if (STRCASECMP(as8Ret, "ON") == 0)
    {
        *reverse = 1;
    }
    return 0;
}


/*
*微调
*vi :visa设备句柄
*name: 设备名称(SEND_ID)
*ch：通道号
*position:位置
* time : 移动到目标位置期望使用的时间
* timeout_ms:等待超时时间。-1表示不等待运行结束；0表示无限等待
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAdjust(ViSession vi,int name,int ch,int wavetable,float position, float time,int timeout_ms)
{
    mrgMRQPVTConfig(vi, name, ch, wavetable, 1); //clear
    mrgMRQPVTValue(vi, name, ch, wavetable, 0.0, 0.0, 0.0);//第一个点
    mrgMRQPVTValue(vi, name, ch, wavetable, position, 0.0, time);//第二个点
    mrgMRQPVTConfig(vi, name, ch, wavetable, 0); //end
    if (mrgMRQPVTStateWait(vi, name, ch, wavetable, MTSTATE_CALCEND, timeout_ms) != 0)//等待计算结束
    {
        return -1;
    }
    mrgMRQPVTState(vi, name, ch, wavetable, MTSWITCH_PREPARE); //
    if (mrgMRQPVTStateWait(vi, name, ch, wavetable, MTSTATE_STANDBY, timeout_ms) != 0)//等待standy
    {
        return -1;
    }
    mrgMRQPVTState(vi, name, ch, wavetable, MTSWITCH_RUN); //
    //if (mrgMRQPVTStateWait(vi, name, ch, wavetable, MTSTATE_CALCEND, timeout_ms) != 0)//等待计算结束
    if (mrgMRQPVTStateWaitEnd(vi, name, ch, wavetable, timeout_ms) != 0)
    {
        return -1;
    }
    mrgMRQPVTState(vi, name, ch, wavetable, MTSWITCH_RESET); //
    return 0;
}
/*
*时钟同步
*vi :visa设备句柄
*name_list:设备名列表
*time:同步的时间
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQClockSync(ViSession vi, char *name_list, float time)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:CLOCk %s,%f\n", name_list, time);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置电机的步距角
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*stepangle:电机的步距角,0->1.8度，1->0.9度，2->15度 3->7.5度
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorStepAngle(ViSession vi, int name, int ch, int stepangle)
{
    char args[SEND_BUF];
    char * ps8StepAngle[4] = { "1.8","0.9","15", "7.5", };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:STEP:ANGLe %d,%d,%s\n",
             name, ch, ps8StepAngle[stepangle]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机的步距角
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*stepangle:电机的步距角,0->1.8度，1->0.9度，2->15度 3->7.5度
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorStepAngle_Query(ViSession vi, int name, int devList, int *stepangle)
{
    char args[SEND_BUF];
    char as8Ret[12];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:STEP:ANGLe? %d,%d\n", name, devList);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 12)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "1.8") == 0)
    {
        *stepangle = 0;
    }
    else if (STRCASECMP(as8Ret, "0.9") == 0)
    {
        *stepangle = 1;
    }
    else if (STRCASECMP(as8Ret, "15") == 0)
    {
        *stepangle = 2;
    }
    else if (STRCASECMP(as8Ret, "7.5") == 0)
    {
        *stepangle = 3;
    }
    return 0;
}
/*
*设置电机的运动类型
*vi :visa设备句柄
*name: 设备名称(SEND_ID)
*ch：通道号
*type:运动类型 0->旋转运动  1->直线运动
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorMotionType(ViSession vi, int name, int devList, int type)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:MOTion:TYPe %d,%d,%s\n",
             name, devList, type==0 ? "ROTARY" : "LINEAR");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机的运动类型
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*type:运动类型
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorMotionType_Query(ViSession vi, int name, int ch, int *type)
{
    char args[SEND_BUF];
    char as8Ret[12];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:MOTion:TYPe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 12)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen-1] = '\0';
        if (STRCASECMP(as8Ret, "ROTARY") == 0)
        {
            *type = 0;
        }
        else if (STRCASECMP(as8Ret, "LINEAR") == 0)
        {
            *type = 1;
        }
        else{
            return -1;
        }
    }
    return 0;
}
/*
*设置电机运动时的单位
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*unit:电机运动时的单位0:ANGLE 1:RADIAN 2:MILLIMETER
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorPositionUnit(ViSession vi, int name, int ch, int unit)
{
    char args[SEND_BUF];
    char *ps8Unit[3] = { "ANGLE" ,"RADIAN" ,"MILLIMETER" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:POSition:UNIT %d,%d,%s\n",
             name, ch, ps8Unit[unit]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机运动时的单位
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*unit:电机运动时的单位;0:ANGLE 1:RADIAN 2:MILLIMETER
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorPositionUnit_Query(ViSession vi, int name, int ch, int *unit)
{
    char args[SEND_BUF];
    char as8Ret[12];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:POSition:UNIT? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 12)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = 0;
        if (STRCASECMP(as8Ret, "ANGLE") == 0)
        {
            *unit = 0;
        }
        else if (STRCASECMP(as8Ret, "RADIAN") == 0)
        {
            *unit = 1;
        }
        else if (STRCASECMP(as8Ret, "MILLIMETER") == 0)
        {
            *unit = 2;
        }
        else{
            return -1;
        }
    }
    return 0;
}
/*
*设置电机旋转运动时的速比
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*a:分子
*b:分母
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorGearRatio(ViSession vi, int name, int ch, int a, int b)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:GEAR:RATio %d,%d,%d,%d\n", name, ch, a, b);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机旋转运动时的速比
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*a:分子
*b:分母
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorGearRatio_Query(ViSession vi, int name, int ch, int *a, int *b)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    char *p = NULL, *pNext = NULL;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:GEAR:RATio? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    *a = atoi(p);
    p = STRTOK_S(NULL, ",", &pNext);
    *b = atoi(p);
    return 0;
}
/*
*设置电机直线运动时的导程
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*millimeter:电机直线运动时的导程
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorLead(ViSession vi, int name, int ch, float millimeter)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:LEAD %d,%d,%f\n", name, ch, millimeter);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机直线运动时的导程
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*millimeter:电机直线运动时的导程
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorLead_Query(ViSession vi, int name, int ch, float *millimeter)
{
    char args[SEND_BUF];
    char as8Ret[12];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:LEAD? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 12)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    *millimeter = strtof(as8Ret,NULL);
    return 0;
}
/*
*设置电机的尺寸
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*size:电机的尺寸
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorSize(ViSession vi, int name, int ch, int size)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:SIZE %d,%d,%d\n", name, ch, size);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机的尺寸
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*size:电机的尺寸
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorSize_Query(ViSession vi, int name, int ch, int *size)
{
    char args[SEND_BUF];
    char as8Ret[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:SIZE? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 10)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen] = '\0';
    }

    *size = atoi(as8Ret);
    return 0;
}
/*
*设置电机的额定电压
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*volt:电压值
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorVoltate(ViSession vi, int name, int ch, int volt)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:VOLTage %d,%d,%d\n", name, ch, volt);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机的额定电压
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*volt:电压值
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorVoltage_Query(ViSession vi, int name, int ch, int *volt)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:VOLTage? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }

    *volt = atoi(as8Ret);
    return 0;

}
/*
*设置电机的额定电流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*current:额定电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorCurrent(ViSession vi, int name, int ch, float current)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:CURRent %d,%d,%f\n", name, ch, current);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机的额定电流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*current:额定电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorCurrent_Query(ViSession vi, int name, int ch, float *current)
{
    char args[SEND_BUF];
    char as8Ret[12];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:CURRent? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 12)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }

    *current = strtof(as8Ret,NULL);
    return 0;
}
/*
*设置电机的反向间隙
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*lash:电机的反向间隙
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorBackLash(ViSession vi, int name, int ch, float lash)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:BACKLash %d,%d,%f\n", name, ch, lash);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机的反向间隙
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*lash:电机的反向间隙
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQMotorBackLash_Query(ViSession vi, int name, int ch, float *lash)
{
    char args[SEND_BUF];
    char state[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:MOTOR:BACKLash? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), state, 100)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *lash = strtof(state,NULL);
    return 0;
}
/*
*PVT配置命令
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:PVT 的配置状态： 0:END 或 1:CLEAR
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTConfig(ViSession vi, int name, int ch, int wavetable, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:CONFig %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), state == 0 ? "END" : "CLEAR");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*下发PVT
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*p:PVT 点的位置
*v:PVT 点的速度
*t:PVT 点的时间值
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTValue(ViSession vi, int name, int devList, int wavetable, float p, float v, float t)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:VALue %d,%d,%s,%f,%f,%f\n",
             name, devList, wavetableToString(wavetable), p, v, t);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置当前PVT的状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state1:PVT的状态  0:RESET ,1:STOP ,2:RUN ,3:PREPARE,4:EMERGSTOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTState(ViSession vi, int name, int ch, int wavetable, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STATe %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), changeSwitchStateToString(state));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询当前PVT下发的状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state1:PVT下发的状态   0:POWERON; 1:IDLE;2:CALCING;3:CALCEND; 4:STANDBY,5:RUNNING,6:ERROR,7:RESERVE;
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTState_Query(ViSession vi, int name, int devList, int wavetable, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STATe? %d,%d,%s\n",
             name, devList, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 10)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "POWERON") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *state = MTSTATE_POWERON;
    }
    else if (STRCASECMP(as8Ret, "IDLE") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = MTSTATE_RESET;
    }
    else if (STRCASECMP(as8Ret, "CALCING") == 0 || STRCASECMP(as8Ret, "2") == 0)
    {
        *state = MTSTATE_CALCING;
    }
    else if (STRCASECMP(as8Ret, "CALCEND") == 0 || STRCASECMP(as8Ret, "3") == 0)
    {
        *state = MTSTATE_CALCEND;
    }
    else if (STRCASECMP(as8Ret, "STANDBY") == 0 || STRCASECMP(as8Ret, "4") == 0)
    {
        *state = MTSTATE_STANDBY;
    }
    else if (STRCASECMP(as8Ret, "RUNNING") == 0 || STRCASECMP(as8Ret, "5") == 0)
    {
        *state = MTSTATE_RUNNING;
    }
    else if (STRCASECMP(as8Ret, "ERROR") == 0 || STRCASECMP(as8Ret, "6") == 0)
    {
        *state = MTSTATE_ERROR;
    }
    else {
        *state = MTSTATE_RESERVE; //mrq 没有返回状态给MRHT
    }
    return 0;
}
/*
*等待当前PVT的状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:期望等待的状态   0:POWERON; 1:IDLE;2:CALCING;3:CALCEND; 4:STANDBY,5:RUNNING,6:ERROR;
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStateWait(ViSession vi, int name, int ch, int wavetable, int state, int timeout_ms)
{
    int ret = -3;
    int readState;
    int time = 0;
    
    while (1)
    {
        msSleep(200);
        mrgMRQPVTState_Query(vi, name, ch, wavetable, &readState);
        msSleep(200);
        if (readState == state)
        {
            return 0;
        }
        time += 200;
        if (timeout_ms > 0) {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}
/*
*PVT下载
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:  
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int  mrgMRQPVTLoad(ViSession vi, int name, int ch, int wavetable, float * p, float * v, float *t, int step, int line)
{
    mrgMRQPVTConfig(vi, name, ch, wavetable, 1);
    for (int i = 0; i < line; i++)
    {
        mrgMRQPVTValue(vi, name, ch, wavetable, p[i*step], v[i*step], t[i*step]);
    }
    mrgMRQPVTConfig(vi, name, ch, wavetable, 0);
    return 0;
}
/*
*等待当前PVT的解算结束状态或运行结束状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStateWaitEnd(ViSession vi, int name, int ch, int wavetable,int timeout_ms)
{
    int ret = -3;
    int readState;
    int time = 0;
    if (timeout_ms == -1)
    {
        return 0;
    }
    while (1)
    {
        msSleep(100);
        mrgMRQPVTState_Query(vi, name, ch, wavetable, &readState);
        if (readState == MTSTATE_CALCEND || readState == MTSTATE_RESET)
        {
            return 0;
        }
        time += 100;
        if (timeout_ms > 0) {
            if (time > timeout_ms) {
                ret = -3; break;
            }
        }
    }
    return ret;
}
/*
*运行PVT
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*timeout_ms:等待运行结束的超时时间,-1表示不等待. 0表示无限等待.
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTRun(ViSession vi, int name, int ch, int wavetable, int timeout_ms)
{
    mrgMRQPVTState(vi, name, ch, wavetable, MTSWITCH_PREPARE);
    if (mrgMRQPVTStateWait(vi, name, ch, wavetable, MTSTATE_STANDBY, timeout_ms) == 0)
    {
        return mrgMRQPVTStateWaitEnd(vi, name, ch, wavetable,timeout_ms);
    }
    return -1;
}
/*
*设置S曲线的加减速占比，两段一起，千分之
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*speedup:加速段占比
*speedcut:减速段占比
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTTimeScale(ViSession vi, int name, int ch, int wavetable, int speedup, int speedcut)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:TSCale %d,%d,%s,%d,%d\n",
             name, ch, wavetableToString(wavetable), speedup, speedcut);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询S曲线的加减速占比，两段一起，千分之
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*speedup:加速段占比
*speedcut:减速段占比
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTTimeScale_Query(ViSession vi, int name, int ch, int wavetable, int* speedup, int* speedcut)
{
    char args[SEND_BUF];
    char as8Ret[100];
    char *p;
    char *pNext;
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:TSCale? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args),as8Ret,100)) == 0) {
        return -1;
    }
    as8Ret[retLen - 1] = '\0';
    p = STRTOK_S(as8Ret, ",", &pNext);
    *speedup = atoi(p);
    p = STRTOK_S(NULL, ",", &pNext);
    *speedcut = atoi(p);
    return 0;
}
/*
*设置循环模式下，PVT的循环次数
*vi :visa设备句柄
*name:设备名称（SEND_ID）
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*cycle:循环次数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTCycle(ViSession vi, int name, int ch, int wavetable, unsigned int cycle)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:CYCLE %d,%d,%s,%u\n",
             name, ch, wavetableToString(wavetable), cycle);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询循环模式下，PVT的循环次数
*vi :visa设备句柄
*name:设备名称（SEND_ID）
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*cycle:循环次数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTCycle_Query(ViSession vi, int name, int ch, int wavetable, unsigned int *cycle)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:CYCLE? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    as8Ret[retLen - 1] = '\0';
    *cycle = strtoul(as8Ret, NULL, 10);
    return 0;
}
/*
*设置FIFO模式下，PVT的缓冲时间
*vi :visa设备句柄
*name: 设备名称（SEND_ID）
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*time:缓冲时间，单位：ms
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTFifoBufferTime(ViSession vi, int name, int ch, int wavetable, unsigned int time)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:FIFO:TIMe %d,%d,%s,%d\n",
             name, ch, wavetableToString(wavetable), time);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询FIFO模式下，PVT的缓冲时间
*vi :visa设备句柄
*name:设备名称（SEND_ID）
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*time:缓冲时间，单位：ms
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTFifoBufferTime_Query(ViSession vi, int name, int ch, int wavetable, unsigned int *time)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:FIFO:TIMe? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    as8Ret[retLen - 1] = '\0';
    *time = strtoul(as8Ret, NULL, 10);
    return 0;
}
/*
*查询模式,包括执行模式,规划模式,运动模式
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*exe:执行模式：0: CYCLE; 1: FIFO
*plan:轨迹规划方式：0: CUBICPOLY; 1:TRAPEZOID; 2:SCURVE
*motion:运动模式： 0: PVT; 1: LVT_CORRECT ; 2: LVT_NOCORRECT
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModeConfig_Query(ViSession vi, int name, int ch, int wavetable, int *exe, int *plan, int *motion)
{
    char args[SEND_BUF];
    char as8Ret[100];
    char *p;
    char *pNext;
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:CONFig? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"FIFO") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *exe = 1;
        }
        else if(STRCASECMP(p,"CYCLE") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *exe = 0;
        }
        else{
            return -1;
        }
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"CUBICPOLY") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *plan = 0;
        }
        else if(STRCASECMP(p,"TRAPEZOID") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *plan = 1;
        }
        else if(STRCASECMP(p,"SCURVE") == 0 || STRCASECMP(p,"2") == 0 )
        {
            *plan = 2;
        }
        else{
            return -1;
        }
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"PVT") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *motion = 0;
        }
        else if(STRCASECMP(p,"LVT_CORRECT") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *motion = 1;
        }
        else if(STRCASECMP(p,"LVT_NOCORRECT") == 0 || STRCASECMP(p,"2") == 0 )
        {
            *motion = 2;
        }
        else{
            return -1;
        }
    }
    return 0;
}
/*
*设置模式,包括执行模式,规划模式,运动模式
*vi :visa设备句柄
*name: 设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*exe:执行模式：0: CYCLE; 1: FIFO
*plan:轨迹规划方式：0: CUBICPOLY; 1:TRAPEZOID; 2:SCURVE
*motion:运动模式： 0: PVT; 1: LVT_CORRECT ; 2: LVT_NOCORRECT
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModeConfig(ViSession vi, int name, int ch, 
                                        int wavetable, int exe, int plan, int motion)
{
    char args[SEND_BUF];
    char *ps8Plan[3] = { "CUBICPOLY" ,"TRAPEZOID" ,"SCURVE" };
    char *ps8MotionMode[3] = { "PVT" ,"LVT_CORRECT" ,"LVT_NOCORRECT" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:CONFig %d,%d,%s,%s,%s,%s\n",
             name, ch, wavetableToString(wavetable), exe?"CYCLE" : "FIFO", ps8Plan[plan], ps8MotionMode[motion]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置执行模式,循环或者FIFO
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*mode:执行模式， 0 ：循环模式	1：FIFO模式
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModeExe(ViSession vi, int name, int ch, int wavetable, int exemode)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:EXE %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), exemode==0? "CYCLE" : "FIFO");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询执行模式,循环或者FIFO
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*mode:执行模式；0：循环模式； 1：FIFO模式
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModeExe_Query(ViSession vi, int name, 
                                           int ch, int wavetable, int *mode)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[12];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:EXE? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 12)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
        if (STRCASECMP(as8Ret, "CYCLE") == 0 || STRCASECMP(as8Ret, "0") == 0)
        {
            *mode = 0;
        }
        else if (STRCASECMP(as8Ret, "FIFO") == 0 || STRCASECMP(as8Ret, "1") == 0)
        {
            *mode = 1;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}
/*
*设置规划模式:三次插值,梯形插值,或S曲线
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*pattern:规划模式 0 - 2  CUBICPOLY|TRAPEZOID｜SCURVE
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModePlan(ViSession vi, int name, int ch, int wavetable, int planmode)
{
    char args[SEND_BUF];
    char *ps8Plan[3] = { "CUBICPOLY" ,"TRAPEZOID" ,"SCURVE" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:PLAN %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), ps8Plan[planmode]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询规划模式:三次插值,梯形插值,或S曲线
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*mode:规划模式 0 - 2  CUBICPOLY|TRAPEZOID｜SCURVE
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModePlan_Query(ViSession vi, int name, int ch, int wavetable, int *mode)
{
    char args[SEND_BUF];
    char as8Ret[24];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:PLAN? %d,%d,%s\n", name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 24)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
        if (STRCASECMP(as8Ret, "CUBICPOLY") == 0)
        {
            *mode = 0;
        }
        else if (STRCASECMP(as8Ret, "TRAPEZOID") == 0)
        {
            *mode = 1;
        }
        else if (STRCASECMP(as8Ret, "SCURVE") == 0)
        {
            *mode = 2;
        }
    }
    return 0;
}
/*
*设置运动模式:PVT或者LVT
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*pattern:模式 0 - 2 PVT|LVT_CORRECT|LVT_NOCORRECT
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModeMotion(ViSession vi, int name, int ch, int wavetable, int mode)
{
    char args[SEND_BUF];
    char *ps8MotionMode[3] = { "PVT" ,"LVT_CORRECT" ,"LVT_NOCORRECT" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:MOTion %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), ps8MotionMode[mode]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询运动模式:PVT或者LVT
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*mode:运动模式 0 - 2 PVT|LVT_CORRECT|LVT_NOCORRECT
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModeMotion_Query(ViSession vi, int name, int ch, int wavetable, int *mode)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[24];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODe:MOTion? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 24)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
        if (STRCASECMP(as8Ret, "PVT") == 0)
        {
            *mode = 0;
        }
        else if (STRCASECMP(as8Ret, "LVT_CORRECT") == 0)
        {
            *mode = 1;
        }
        else if (STRCASECMP(as8Ret, "LVT_NOCORRECT") == 0)
        {
            *mode = 2;
        }
    }
    return 0;
}
/*
*设置LVT模式下进行时间调整的占比
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*duty:占空比  0:1/4 ;1:1/8;  2: 1/16; 3: 1/32
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModifyDuty(ViSession vi, int name, int devList, int wavetable, int duty)
{
    char args[SEND_BUF];
    char *as8duty[4] = { "1/4","1/8","1/16","1/32" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODIFy:DUTY %d,%d,%s,%s\n",
             name, devList, wavetableToString(wavetable), as8duty[duty]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询LVT模式下进行时间调整的占比
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*duty:占空比
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTModifyDuty_Query(ViSession vi, int name,
                                              int ch, int wavetable, float *duty)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:MODIFy:DUTY? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "1/4") == 0)
    {
        *duty = 0;
    }
    else if(STRCASECMP(as8Ret, "1/8") == 0)
    {
        *duty = 1;
    }
    else if (STRCASECMP(as8Ret, "1/16") == 0)
    {
        *duty = 2;
    }
    else if (STRCASECMP(as8Ret, "1/32") == 0)
    {
        *duty = 3;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置是否为速度保持
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:是否末速度保持。 0不保持；1保持
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTEndState(ViSession vi, int name, int ch, int wavetable, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:END:STATe %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), state ? "STOP" : "HOLD");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询是否为速度保持
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:是否末速度保持
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTEndState_Query(ViSession vi, int name, int ch, int wavetable, int *state)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[12];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:END:STATe? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 12)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
        if (STRCASECMP(as8Ret, "STOP") == 0 || STRCASECMP(as8Ret, "0") == 0)
        {
            *state = 0;
        }
        else if (STRCASECMP(as8Ret, "HOLD") == 0 || STRCASECMP(as8Ret, "1") == 0)
        {
            *state = 1;
        }
    }
    return 0;
}
/*
*设置急停方式,立即停止或者减速停止
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*type:急停方式;  0:立即停止；1：减速停止
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStopMode(ViSession vi, int name, int ch, int wavetable, int type)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STOP:MODe %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), type == 0 ? "IMMEDIATE" : "DISTANCE");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询急停方式,立即停止或者减速停止
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*mode:急停方式,0:立即停止； 1：减速停止
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStopMode_Query(ViSession vi, int name, int ch, int wavetable, int *mode)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STOP:MODe? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "IMMEDIATE") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *mode = 0;
    }
    else if (STRCASECMP(as8Ret, "DISTANCE") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *mode = 1;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置急停时间
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*time:急停的时间
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStopTime(ViSession vi, int name, int ch, int wavetable, float time)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STOP:TIMe %d,%d,%s,%f\n",
             name, ch, wavetableToString(wavetable), time);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询急停时间
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*time:急停的时间
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStopTime_Query(ViSession vi, int name, int ch, int wavetable, float *time)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STOP:TIMe? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *time = strtof(state,NULL);
    return 0;
}
/*
*设置减速停止时的减速距离
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*distance:减速距离
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStopDistance(ViSession vi, int name, int ch, int wavetable, float distance)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STOP:DISTance %d,%d,%s,%f\n",
             name, ch, wavetableToString(wavetable), distance);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询减速停止时的减速距离
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*distance:减速距离
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTStopDistance_Query(ViSession vi, int name, 
                                                int ch, int  wavetable, float *distance)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:STOP:DISTance? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *distance = strtof(state,NULL);
    return 0;
}
/*
*设置波表的起始地址
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*address:波表起始地址
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTWavetableAddress(ViSession vi, int name, int ch, int wavetable, unsigned int address)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:WAVETABLE:ADDRess %d,%d,%s,%u\n",
             name, ch, wavetableToString(wavetable), address);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询波表的起始地址
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*address:波表起始地址
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTWavetableAddress_Query(ViSession vi, int name,
                                                    int ch, int  wavetable, unsigned int * address)
{
    char args[SEND_BUF];
    char tmp[20];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:WAVETABLE:ADDRess? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), tmp, 20)) == 0) {
        return -1;
    }
    else
    {
        tmp[retLen - 1] = '\0';
    }
    *address = strtoul(tmp,NULL,0);
    return 0;
}
/*
*设置波表的大小
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*size:波表大小
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTWavetableSize(ViSession vi, int name, int ch, int wavetable, unsigned int size)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:WAVETABLE:SIZE %d,%d,%s,%u\n",
             name, ch, wavetableToString(wavetable), size);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询波表的大小
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*size:波表大小
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQPVTWavetableSize_Query(ViSession vi, int name,
                                                 int ch, int  wavetable, unsigned int * size)
{
    char args[SEND_BUF];
    char tmp[20];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:PVT:WAVETABLE:SIZE? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), tmp, 20)) == 0) {
        return -1;
    }
    else
    {
        tmp[retLen - 1] = '\0';
    }
    *size = strtoul(tmp, NULL, 0);
    return 0;
}

/*
*查询失步的状态,阈值及失步后的反应
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state1:失步的状态 0:禁止 ；1：使能
*threshold:失步的阈值
*resp:编码器步数偏差超过阈值后的响应方式 0:NONE;1:ALARM;2:STOP;3:ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQLostStepLineConfig_Query(ViSession vi, int name, 
                                                   int ch, int wavetable, int *state,float *threshold, int *resp)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    char *p;
    char *pNext;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:CONFig? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"ON") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *state = 1;
        }
        else if(STRCASECMP(p,"OFF") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *state = 0;
        }
        else{
            return -1;
        }
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *threshold = strtof(p,NULL);
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"NONE") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *resp = 0;
        }
        else if(STRCASECMP(p,"ALARM") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *resp = 1;
        }
        else if(STRCASECMP(p,"STOP") == 0 || STRCASECMP(p,"2") == 0 )
        {
            *resp = 2;
        }
        else if(STRCASECMP(p,"ALARM&STOP") == 0 || STRCASECMP(p,"3") == 0 )
        {
            *resp = 3;
        }
        else{
            return -1;
        }
    }
    return 0;
}
/*
*设置失步的状态,阈值及失步后的反应
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:失步的告警状态 0:禁止 ；1：使能
*threshold:失步的阈值
*resp:编码器步数偏差超过阈值后的响应方式  0:NONE;1:ALARM;2:STOP;3:ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQLostStepLineConfig(ViSession vi, int name, 
                                             int ch, int wavetable, int state, float threshold, int resp)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:CONFig %d,%d,%s,%s,%f,%s\n",
             name, ch, wavetableToString(wavetable),
             state ? "ON" : "OFF",
             threshold, changeResponseToString(resp));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置线间失步告警状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:警告状态
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQLostStepState(ViSession vi, int name, int ch, int wavetable, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:STATe %d,%d,%d,%s\n",
             name, ch, wavetable, state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询线间失步告警状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:警告状态  0:禁止 ；1：使能
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQLostStepState_Query(ViSession vi, int name, int ch, int wavetable, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:STATe? %d,%d,%d\n", name, ch, wavetable);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"ON") == 0 || STRCASECMP(as8Ret,"1") == 0 )
    {
        *state = 1;
    }
    else if(STRCASECMP(as8Ret,"OFF") == 0 || STRCASECMP(as8Ret,"0") == 0 )
    {
        *state = 0;
    }
    else{
        return -1;
    }
    return 0;
}
/*
*设置线间失步阈值
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*value:线间失步阈值
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQLostStepThreshold(ViSession vi, int name, int ch, int wavetable, float value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:THREShold %d,%d,%s,%f\n",
             name, ch, wavetableToString(wavetable), value);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询线间失步阈值
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*value:线间失步阈值
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQLostStepThreshold_Query(ViSession vi, int name, 
                                                  int ch, int wavetable, float *value)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:THREShold? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }

    *value = strtof(as8Ret,NULL);
    return 0;
}
/*
*设置当步数偏差超过LOSTNUM后的响应方式
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*state:状态0 - 3 NONE|ALARM|STOP|ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQLostStepResponse(ViSession vi, int name, int ch, int wavetable, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:RESPonse %d,%d,%s,%s\n",
             name, ch, wavetableToString(wavetable), changeResponseToString(state));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询当步数偏差超过LOSTNUM后的响应方式
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*wavetable:波表索引，取值范围： 0~9 MAIN|SMALL|P1|P2|P3|P4|P5|P6|P7|P8
*resp:  0:NONE;1:ALARM;2:STOP;3:ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int mrgMRQLostStepResponse_Query(ViSession vi, int name, int ch, int wavetable, int *resp)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:LOSTstep:LINe:RESPonse? %d,%d,%s\n",
             name, ch, wavetableToString(wavetable));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"NONE") == 0 || STRCASECMP(as8Ret,"0") == 0 )
    {
        *resp = 0;
    }
    else if(STRCASECMP(as8Ret,"ALARM") == 0 || STRCASECMP(as8Ret,"1") == 0 )
    {
        *resp = 1;
    }
    else if(STRCASECMP(as8Ret,"STOP") == 0 || STRCASECMP(as8Ret,"2") == 0 )
    {
        *resp = 2;
    }
    else if(STRCASECMP(as8Ret,"ALARM&STOP") == 0 || STRCASECMP(as8Ret,"3") == 0 )
    {
        *resp = 3;
    }
    else{
        return -1;
    }
    return 0;
}
/*
*查询上报功能配置
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*funs: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*buf:返回的功能
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQReportConfig_Query(ViSession vi, int name, int ch, int funs, int *state,float *period)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    char *p;
    char *pNext;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:CONFig? %d,%d,%s\n",
             name, ch, changeReportFuncToString(funs));
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if(p)
    {
        *state = atoi(p);
        p = STRTOK_S(NULL, ",", &pNext);
        if(p)
        {
            *period = strtof(p,NULL);
        }
        else
        {
            *period = 0.0f;
        }
    }
    else{
        *state = 0;
    }
    return 0;
}
/*
*设置上报功能配置
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:状态on/off
*funs: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*period:指定类型数据的上报周期
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQReportConfig(ViSession vi, int name, int ch, int funs, int state, float period)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:CONFig %d,%d,%s,%s,%f\n",
             name, ch, changeReportFuncToString(funs), state ? "ON" : "OFF", period);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置上报状态
*vi :visa设备句柄
*name: 设备名称(SEND_ID)
*ch：通道号
*funs: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*state:状态on/off
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQReportState(ViSession vi, int name, int ch, int funs, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:STATe %d,%d,%s,%s\n",
             name, ch, changeReportFuncToString(funs), state? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询上报状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*funs: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*state1:状态on/off 0->OFF  1->ON
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQReportState_Query(ViSession vi, int name, int ch, int funs, int *state)
{
    char args[SEND_BUF];
    int retLen = 0;
    char tmp[20];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:STATe? %d,%d,%s\n",
             name, ch, changeReportFuncToString(funs));
    if ((retLen = busQuery(vi, args, strlen(args), tmp, 10)) == 0) {
        return -1;
    }
    else
    {
        tmp[retLen - 1] = '\0';
        if (STRCASECMP(tmp, "OFF") == 0 || STRCASECMP(tmp, "0") == 0)
        {
            *state = 0;
        }
        else if (STRCASECMP(tmp, "ON") == 0 || STRCASECMP(tmp, "1") == 0)
        {
            *state = 1;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}
/*
*设置上报周期
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*funs: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*period:指定类型数据的上报周期,单位：ms
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQReportPeriod(ViSession vi, int name, int ch, int funs, int period)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:PERiod %d,%d,%s,%d\n",
             name, ch, changeReportFuncToString(funs), period);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询上报周期
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*func: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*period:指定类型数据的上报周期
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQReportPeriod_Query(ViSession vi, int name, int ch, int func, int *period)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:PERiod? %d,%d,%s\n",
             name, ch, changeReportFuncToString(func));
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }
    *period = atoi(state);
    return 0;
}
/*
*查询自动上报数据
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*func: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*data : 返回数据的存储区
*返回值：实际返回的数据个数
*说明：此处的查询，每次只返回一个数据
*/
EXPORT_API int CALL mrgMRQReportData_Query(ViSession vi, int name, int ch, int func, unsigned int *data)
{
    char args[SEND_BUF];
    char buff[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:DATA:VALue? %d,%d,%s\n",
             name, ch, changeReportFuncToString(func));
    if ((retLen = busQuery(vi, args, strlen(args), buff, 100)) == 0) {
        return 0;
    }
    else
    {
        buff[retLen - 1] = '\0';
    }
    *data = strtoul(buff, NULL, 10);
    return 1;
}
/*
*查询自动上报数据队列中的数据
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*func: 0 ~ 5 TORQUE|CYCLE|SGALL|SGSE|DIST|ABSEN
*data : 返回数据的存储区
*返回值：实际返回的数据个数
*/
EXPORT_API int CALL mrgMRQReportQueue_Query(ViSession vi, int name, int ch, int func, unsigned int *data)
{
    char args[SEND_BUF];
    char buff[1024];
    char strLen[12];
    int retLen = 0;
    int lenOfLen = 0;
    int len = 0;
    int count = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:REPort:DATA:Queue? %d,%d,%s\n",
             name, ch, changeReportFuncToString(func));
    if (busWrite(vi, args, strlen(args)) == 0)
    {
        return 0;
    }
    while ((retLen = busRead(vi, buff, 12)) > 0)
    {
        if (buff[0] != '#')//格式错误
        {
            return count;
        }
        lenOfLen = buff[1] - 0x30;
        memcpy(strLen, &buff[2], lenOfLen);//取出长度字符串
        len = strtoul(strLen, NULL, 10);
        if (len != 0)
        {
            buff[0] = buff[11];
            retLen = busRead(vi, &buff[1], len);
        }
        else
        {
            return count;
        }
        memcpy((char*)&data[count], buff, retLen);
        count += retLen / 4;
    }
    return count;
}
/*
*设置触发输入的模式,码型触发或电平触发
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*mode:触发输入的模式,0:码型触发；1：电平触发
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerMode(ViSession vi, int name, int ch, int mode)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:MODe %d,%d,%s\n",
             name, ch, mode==0? "PATTERN" : "LEVEL");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}

/*
*查询触发输入的模式,码型触发或电平触发
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*mode:触发输入的模式. 0:码型触发；1：电平触发
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerMode_Query(ViSession vi, int name, int ch, int *mode)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:MODe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "PATTERN") == 0)
    {
        *mode = 0;
    }
    else if (STRCASECMP(as8Ret, "LEVEL") == 0)
    {
        *mode = 1;
    }
    return 0;
}
/*
*查询电平触发配置
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： 0:TRIGL 或 1:TRIGR
*state:开关状态
*type:触发类型，0：无类型；1：上升沿；2；低电平；3：下降沿；4：高电平
*period:采样周期，
*response:触动触发后的反应 0：NONE，1：ALARM;2:STOP ;3:ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelConfig_Query(ViSession vi, int name, 
                                                   int ch, int trig, int * state, int * type, float *period,int * response)
{
    char args[SEND_BUF];
    char as8Ret[100];
    char*p,*pNext;
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:CONFig? %d,%d,%s\n",
             name, ch, trig==0 ? "TRIGL" : "TRIGR");
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"ON") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *state = 1;
        }
        else if(STRCASECMP(p,"OFF") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *state = 0;
        }
        else{
            return -1;
        }
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        if (STRCASECMP(p, "RESERVE") == 0 || STRCASECMP(p, "0") == 0)
        {
            *type = 0;
        }
        else if (STRCASECMP(p, "LOW") == 0 || STRCASECMP(p, "1") == 0)
        {
            *type = 1;
        }
        else if (STRCASECMP(p, "RISE") == 0 || STRCASECMP(p, "2") == 0)
        {
            *type = 2;
        }
        else if (STRCASECMP(p, "FALL") == 0 || STRCASECMP(p, "3") == 0)
        {
            *type = 3;
        }
        else if (STRCASECMP(p, "HIGH") == 0 || STRCASECMP(p, "4") == 0)
        {
            *type = 4;
        }
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *period = strtof(p,NULL);
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"NONE") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *response = 0;
        }
        else if(STRCASECMP(p,"ALARM") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *response = 1;
        }
        else if(STRCASECMP(p,"STOP") == 0 || STRCASECMP(p,"2") == 0 )
        {
            *response = 2;
        }
        else if(STRCASECMP(p,"ALARM&STOP") == 0 || STRCASECMP(p,"3") == 0 )
        {
            *response = 3;
        }
        else{
            return -1;
        }
    }
    return 0;
}
/*
*设置电平触发配置
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*state:开关状态
*type:触发类型，0：无类型；1：上升沿；2；低电平；3：下降沿；4：高电平
*period:采样周期，
*response:触动触发后的反应 0：NONE，1：ALARM;2:STOP ;3:ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelConfig(ViSession vi, int name, int devList, int trig, 
                                             int state, int type, float period, int response)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:CONFig %d,%d,%s,%s,%s,%f,%s\n",
             name, devList, trig ==0 ? "TRIGL" : "TRIGR",
             state==0 ? "OFF" : "ON",
             changeLevelTrigTypeToString(type),
             period, changeResponseToString(response));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置电平触发，打开或关闭
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*state:状态
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelState(ViSession vi, int name, int ch, int trig, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:STATe %d,%d,%s,%s\n",
             name, ch, trig == 0 ? "TRIGL" : "TRIGR", state == 0 ? "OFF" : "ON");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电平触发，打开或关闭
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*state:状态. 0:关闭； 1：打开
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelState_Query(ViSession vi, int name, int ch, int trig, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:STATe? %d,%d,%s\n",
             name, ch, trig == 0 ? "TRIGL" : "TRIGR");
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP("OFF", as8Ret) == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *state = 0;
    }
    else if (STRCASECMP("ON", as8Ret) == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = 1;
    }
    else{
        return -1;
    }
    return 0;
}
/*
*设置触发电平类型
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*type:电平触发类型： 0 - 4 RESERVE|LOW|RISE|FALL|HIGH
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelType(ViSession vi, int name, int ch, int trig, int type)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:TYPe %d,%d,%s,%s\n",
             name, ch, trig == 0 ? "TRIGL" : "TRIGR", changeLevelTrigTypeToString(type));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询触发电平类型
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*type:电平触发类型： 0:RESERVE; 1:LOW; 2: RISE; 3:FALL;4:HIGH
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelType_Query(ViSession vi, int name, int ch, int trig, int *type)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:TYPe? %d,%d,%s\n",
             name, ch, trig ==0 ? "TRIGL" : "TRIGR");
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "RESERVE") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *type = 0;
    }
    else if (STRCASECMP(as8Ret, "LOW") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *type = 1;
    }
    else if (STRCASECMP(as8Ret, "RISE") == 0 || STRCASECMP(as8Ret, "2") == 0)
    {
        *type = 2;
    }
    else if (STRCASECMP(as8Ret, "FALL") == 0 || STRCASECMP(as8Ret, "3") == 0)
    {
        *type = 3;
    }
    else if (STRCASECMP(as8Ret, "HIGH") == 0 || STRCASECMP(as8Ret, "4") == 0)
    {
        *type = 4;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置触发电平响应
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*resp:电平触发的响应方式：0 - 3 NONE|ALARM|STOP|ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelResponse(ViSession vi, int name, int ch, int trig, int resp)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:RESPonse %d,%d,%s,%s\n",
             name, ch, trig ==0 ? "TRIGL" : "TRIGR", changeResponseToString(resp));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询触发电平响应
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*resp:电平触发的响应方式：0: NONE; 1:ALARM;2:STOP; 3:ALARM&STOP
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelResponse_Query(ViSession vi, int name, int ch, int trig, int *resp)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:RESPonse? %d,%d,%s\n",
             name, ch, trig == 0 ? "TRIGL" : "TRIGR");
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "NONE") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *resp = 0;
    }
    else if (STRCASECMP(as8Ret, "ALARM") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *resp = 1;
    }
    else if (STRCASECMP(as8Ret, "STOP") == 0 || STRCASECMP(as8Ret, "2") == 0)
    {
        *resp = 2;
    }
    else if (STRCASECMP(as8Ret, "ALARM&STOP") == 0 || STRCASECMP(as8Ret, "3") == 0)
    {
        *resp = 3;
    }
    else{
        return -1;
    }
    return 0;
}
/*
*设置触发电平采样周期
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*period:采样周期,单位：秒
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelPeriod(ViSession vi, int name, int ch, int trig, float period)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:PERIod %d,%d,%s,%f\n",
             name, ch, trig ==0 ? "TRIGL" : "TRIGR", period);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询触发电平采样周期
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*trig:电平触发编号： TRIGL 或 TRIGR
*period:采样周期
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQTriggerLevelPeriod_Query(ViSession vi, int name, 
                                                   int ch, int trig, float *period)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TRIGger:LEVel:PERIod? %d,%d,%s\n",
             name, ch, trig== 0 ? "TRIGL" : "TRIGR");
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }

    *period = strtof(as8Ret,NULL);
    return 0;
}
/*
*查询驱动板配置
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:开关状态。 0：禁止；1：使能
*microstep:微步
*current:驱动器的驱动电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverConfig_Query(ViSession vi, int name, int ch, 
                                             int *state,int *microstep,float*current )
{
    char args[SEND_BUF];
    char as8Ret[100];
    char *p, *pNext;
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:CONFig? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"ON") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *state = 1;
        }
        else if(STRCASECMP(p,"OFF") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *state = 0;
        }
        else{
            return -1;
        }
    }
    else{
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *microstep = atoi(p);
    }
    else
    {
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *current = strtof(p,NULL);
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置驱动板配置
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:开关状态。 0：禁止；1：使能
*microstep:微步
*current:驱动器的驱动电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverConfig(ViSession vi, int name, int ch, 
                                       int state, int microstep, float current)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:CONFig %d,%d,%s,%d,%f\n",
             name, ch, state == 0 ? "OFF" : "ON", microstep, current);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动板类型
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*type:驱动板的类型  0:D17  ; 1: D23
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverType_Query(ViSession vi, int name, int ch, int *type)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:TYPe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        type[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"D17") == 0 || STRCASECMP(as8Ret,"0") == 0)
    {
        *type = 0;
    }
    else if(STRCASECMP(as8Ret,"D23") == 0 || STRCASECMP(as8Ret,"1") == 0)
    {
        *type = 1;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置驱动板电流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*current:驱动板电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverCurrent(ViSession vi, int name, int ch, float current)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:CURRent:VALue %d,%d,%f\n", name, ch, current);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动板电流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*current:驱动板电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverCurrent_Query(ViSession vi, int name, int ch, float *current)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:CURRent:VALue? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    *current = strtof(as8Ret,NULL);
    return 0;
}
/*
*设置驱动板空闲电流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*current:驱动板空闲电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverIdleCurrent(ViSession vi, int name, int ch, float current)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:CURRent:IDLE %d,%d,%f\n", name, ch, current);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动板空闲电流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*current:驱动板空闲电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverIdleCurrent_Query(ViSession vi, int name, int ch, float *current)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:CURRent:IDLE? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }

    *current = strtof(as8Ret,NULL);
    return 0;
}
/*
*设置电机微步数
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*microstep:电机的微步数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverMicroStep(ViSession vi, int name, int devList, int microstep)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:MICROStep %d,%d,%d\n", name, devList, microstep);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机微步数
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*microstep:电机的微步数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverMicroStep_Query(ViSession vi, int name, int devList, int *microstep)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:MICROStep? %d,%d\n", name, devList);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *microstep = atoi(state);
    return 0;
}
/*
*设置驱动开关状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:状态on/off. 0:OFF; 1:ON
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverState(ViSession vi, int name, int devList, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:STATe %d,%d,%s\n", name, devList, state? "ON":"OFF");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动开关状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state1:状态on/off
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverState_Query(ViSession vi, int name, int devList, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:STATe? %d,%d\n", name, devList);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "ON") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = 1;
    }
    else
    {
        *state = 0;
    }
    return 0;
}
/*
*设置驱动的寄存器值
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*regIndex： 寄存器地址
*value:寄存器值
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverRegisterValue(ViSession vi, int name, int ch,int regIndex, unsigned int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:REGister:VALue %d,%d,%d,%d\n", name, ch, regIndex,value);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动的寄存器值
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*regIndex： 寄存器地址
*value:寄存器值
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverRegisterValue_Query(ViSession vi, int name, int ch, int regIndex, unsigned int *value)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DRIVER:REGister:VALue? %d,%d,%d\n", name, ch, regIndex);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    *value = strtoul(as8Ret, NULL, 10);
    return 0;
}
/*
*设置驱动器TUNING功能的开关状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:开关状态 0：OFF；1：ON
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningState(ViSession vi, int name, int ch, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:STATe %d,%d,%s\n", name, ch, state?"ON":"OFF");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动器TUNING功能的开关状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:开关状态 0：OFF；1：ON
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningState_Query(ViSession vi, int name, int ch, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:STATe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "OFF") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *state = 0;
    }
    else if (STRCASECMP(as8Ret, "ON") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = 1;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置驱动器TUNING功能的最小电流比 
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*ratio:最小电流比 0：1/2；1：1/4
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningMinCurrent(ViSession vi, int name, int ch, int ratio)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:MIN %d,%d,%s\n", name, ch, ratio == 0 ? "1/2" : "1/4");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动器TUNING功能的最小电流比 
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*ratio:最小电流比 0：1/2；1：1/4
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningMinCurrent_Query(ViSession vi, int name, int ch, int *ratio)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:MIN? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "1/2") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *ratio = 0;
    }
    else if (STRCASECMP(as8Ret, "1/4") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *ratio = 1;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置驱动器TUNING功能的能效上下限
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*limitUp:能效上限
*limitDown：能效下限
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningEfficLimit(ViSession vi, int name, int ch, int limitUp,int limitDown)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:EFFIC %d,%d,%d,%d\n", name, ch, limitUp , limitDown);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动器TUNING功能的能效上下限
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*limitUp:能效上限
*limitDown：能效下限
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningEfficLimit_Query(ViSession vi, int name, int ch, int *limitUp, int *limitDown)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    char *p, *pNext;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:EFFIC? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if (p)
    {
        *limitUp = atoi(p);
    }
    else
    {
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if (p)
    {
        *limitDown = atoi(p);
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
*设置驱动器TUNING功能的电流上升和下降速度
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*speedUp:电流上升速度 0：表示每整步增加一个单位的电流；
                    1：表示每整步增加两个单位的电流
                    2：表示每整步增加4个单位的电流
                    3：表示每整步增加8个单位的电流
*speedDown：电流下降速度  0：表示每32整步减小一个单位的电流；
                        1：表示每8整步减小一个单位的电流；
                        2：表示每2整步减小一个单位的电流；
                        3：表示每1整步减小一个单位的电流；
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningCurrentRegulate(ViSession vi, int name, int ch, int speedUp, int speedDown)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:REGULATE %d,%d,%d,%d\n",
             name, ch, speedUp, speedDown);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动器TUNING功能的电流上升和下降速度
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*speedUp:电流上升速度 0：表示每整步增加一个单位的电流；
                    1：表示每整步增加两个单位的电流
                    2：表示每整步增加4个单位的电流
                    3：表示每整步增加8个单位的电流
*speedDown：电流下降速度  0：表示每32整步减小一个单位的电流；
                        1：表示每8整步减小一个单位的电流；
                        2：表示每2整步减小一个单位的电流；
                        3：表示每1整步减小一个单位的电流；
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDriverTuningCurrentRegulate_Query(ViSession vi, int name, int ch, int *speedUp, int *speedDown)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    char *p, *pNext;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:TUNing:REGULATE? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if (p)
    {
        (*speedUp) = atoi(p);
    }
    else
    {
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if (p)
    {
        (*speedDown) = atoi(p);
    }
    else
    {
        return -1;
    }
    return 0;
}

/*
*查询编码器的状态,类型,线数及通道个数.状态为OFF时,后面的参数可以省略
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:状态  0: NONE; 1: OFF;  2：ON
*type:编码器的类型： 0:INCREMENTAL 或 1: ABSOLUTE
*linenum:增量型编码器光电码盘一周的线数： 500、 1000、 1024、 2000、 2048、 4000、 4096 或 5000
*channelnum:增量型编码器的通道数： 1 或 3
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderConfig_Query(ViSession vi, int name, int ch, 
                                              int *state,int *type,int * lineNum,int *chanNum)
{
    char args[SEND_BUF];
    char as8Ret[100];
    char *p,*pNext;
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:CONFig? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if(p)
    {
        if(STRCASECMP(p,"NONE") == 0 || STRCASECMP(p,"0") == 0 )
        {
            *state = 0;
        }
        if(STRCASECMP(p,"ON") == 0 || STRCASECMP(p,"2") == 0 )
        {
            *state = 2;
        }
        else if(STRCASECMP(p,"OFF") == 0 || STRCASECMP(p,"1") == 0 )
        {
            *state = 1;
        }
        else{
            return -1;
        }
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        if (STRCASECMP(p, "INCREMENTAL") == 0 || STRCASECMP(p, "0") == 0)
        {
            *type = 0;
        }
        else if (STRCASECMP(p, "ABSOLUTE") == 0 || STRCASECMP(p, "1") == 0)
        {
            *type = 1;
        }
    }
    else{
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *lineNum = atoi(p);
    }
    else{
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *chanNum = atoi(p);
    }
    else{
        return -1;
    }
    return 0;
}
/*
*设置编码器的状态,类型,线数及通道个数.状态为OFF时,后面的参数可以省略
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:状态  0: NONE; 1: OFF;  2：ON
*type:编码器的类型： 0:INCREMENTAL 或 1: ABSOLUTE
*linenum:增量型编码器光电码盘一周的线数： 500、 1000、 1024、 2000、 2048、 4000、 4096 或 5000
*channelnum:增量型编码器的通道数： 1 或 3
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderConfig(ViSession vi, int name, int ch, 
                                        int state, int type, int linenum, int channelnum)
{
    char args[SEND_BUF];
    char *ps8EncoderState[3] = {"NONE","OFF","ON"};
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:CONFig %d,%d,%s,%s,%d,%d\n",
             name, ch,
             ps8EncoderState[state],
             type? "ABSOLUTE":"INCREMENTAL",
             linenum, channelnum);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置编码器线数
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*num:编码器线数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderLineNum(ViSession vi, int name, int ch, int num)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:LINe:NUMber %d,%d,%d\n", name, ch, num);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询编码器线数
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*num:编码器线数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderLineNum_Query(ViSession vi, int name, int ch, int *num)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:LINe:NUMber? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }

    *num = atoi(as8Ret);
    return 0;
}
/*
*设置编码器通道
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*channelnum:增量型编码器的通道数： 1 或 3
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderChannelNum(ViSession vi, int name, int ch, int channelnum)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:CHANnel:NUMber %d,%d,%d\n", name, ch, channelnum);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询编码器通道
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*channelnum:增量型编码器的通道数： 1 或 3
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderChannelNum_Query(ViSession vi, int name, int ch, int *channelnum)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:CHANnel:NUMber? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *channelnum = atoi(state);
    return 0;
}
/*
*设置编码器类型
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*type:编码器的类型： INCREMENTAL 或 ABSOLUTE
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderType(ViSession vi, int name, int devList, int type)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:TYPe %d,%d,%s\n",
             name, devList, type == 0 ? "INCREMENTAL" : "ABSOLUTE");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询编码器类型
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*type:编码器的类型： 0:INCREMENTAL 或 1:ABSOLUTE
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderType_Query(ViSession vi, int name, int ch, int *type)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:TYPe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "INCREMENTAL") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *type = 0;
    }
    else if (STRCASECMP(as8Ret, "ABSOLUTE") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *type = 1;
    }
    return 0;
}
/*
*设置编码器信号的倍乘
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*multiple:倍乘 0 - 2 "SINGLE","DOUBLE","QUADRUPLE"
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderMultiple(ViSession vi, int name, int ch, int multiple)
{
    char args[SEND_BUF];
    char *ps8Multiple[3] = { "SINGLE" ,"DOUBLE" ,"QUADRUPLE" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:MULTIPLe %d,%d,%s\n",
             name, ch, ps8Multiple[multiple]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询编码器信号的倍乘
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*multiple:倍乘 0 - 2 "SINGLE","DOUBLE","QUADRUPLE"
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderMultiple_Query(ViSession vi, int name, int ch, int *multiple)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:MULTIPLe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"SINGLE") == 0 || STRCASECMP(as8Ret,"0") == 0)
    {
        *multiple = 0;
    }
    else if(STRCASECMP(as8Ret,"DOUBLE") == 0 || STRCASECMP(as8Ret,"1") == 0)
    {
        *multiple = 1;
    }
    else if(STRCASECMP(as8Ret,"QUADRUPLE") == 0 || STRCASECMP(as8Ret,"2") == 0)
    {
        *multiple = 2;
    }
    else{
        return -1;
    }
    return 0;
}
/*
*设置编码器状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:编码器的状态：0: NONE;1: OFF ;2: ON
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderState(ViSession vi, int name, int ch, int state)
{
    char args[SEND_BUF];
    char *pEncoderState[] = { "NONE","OFF","ON" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:STATe %d,%d,%s\n", name, ch, pEncoderState[state]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询编码器状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state1:编码器的状态： 0:NONE; 1:OFF ;2: ON
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderState_Query(ViSession vi, int name, int devList, int*state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:STATe? %d,%d\n", name, devList);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "NONE") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *state = 0;
    }
    else if (STRCASECMP(as8Ret, "OFF") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = 1;
    }
    else if (STRCASECMP(as8Ret, "ON") == 0 || STRCASECMP(as8Ret, "2") == 0)
    {
        *state = 2;
    }
    return 0;
}
/*
*设置LVT模式下编码器反馈比
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*feed:编码器反馈比
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderFeedback(ViSession vi, int name, int devList, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:FEEDBACK %d,%d,%d\n", name, devList, value);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询LVT模式下编码器反馈比
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*feed:编码器反馈比
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderFeedback_Query(ViSession vi, int name, int devList, int *value)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[10];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:FEEDBACK? %d,%d\n", name, devList);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 10)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    *value = atoi(as8Ret);
    return 0;
}
/*
*设置编码器方向 
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*value:编码器方向。0表示编码器逆时针转动，数值增加；1表示编码器逆时针转，数据减小
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderDirection(ViSession vi, int name, int ch, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:DIRECTION %d,%d,%s\n", name, ch, value?"NEGATIVE":"POSITIVE");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询编码器方向
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*value:编码器方向。0表示编码器逆时针转动，数值增加；1表示编码器逆时针转，数据减小
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQEncoderDirection_Query(ViSession vi, int name, int ch, int *value)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:DIRECTION? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP("POSITIVE", as8Ret) == 0 || STRCASECMP("0", as8Ret) == 0)
    {
        *value = 0;
    }
    else if (STRCASECMP("NEGATIVE", as8Ret) == 0 || STRCASECMP("1", as8Ret) == 0)
    {
        *value = 1;
    }
    else
    {
        return -1;
    }
    
    return 0;
}
/*
* 设置绝对值编码器的报警状态
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* state:编码器报警状态 0:OFF; 1:ON
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmState(ViSession vi, int name, int ch, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:STATe %d,%d,%s\n", name, ch, state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查询绝对值编码器的报警状态
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* state:编码器报警状态 0:OFF; 1:ON
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmState_Query(ViSession vi, int name, int ch, int *state)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:STATe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP("OFF", as8Ret) == 0 || STRCASECMP("0", as8Ret) == 0)
    {
        *state = 0;
    }
    else if (STRCASECMP("ON", as8Ret) == 0 || STRCASECMP("1", as8Ret) == 0)
    {
        *state = 1;
    }
    else
    {
        return -1;
    }
    return 0;
}
/*
* 设置绝对值编码器的报警上限
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:编码器报警上限值(编码器线数)
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmUpLimit(ViSession vi, int name, int ch, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:UP %d,%d,%d\n", name, ch, value);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查绝对值编码器的报警上限
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:编码器报警上限值(编码器线数)
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmUpLimit_Query(ViSession vi, int name, int ch, int *value)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:UP? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    *value = atoi(as8Ret);
    return 0;
}
/*
* 设置绝对值编码器的报警下限
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:编码器报警下限值(编码器线数)
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmDownLimit(ViSession vi, int name, int ch, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:DOWN %d,%d,%d\n", name, ch, value);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查绝对值编码器的报警下限
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:编码器报警下限值(编码器线数)
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmDownLimit_Query(ViSession vi, int name, int ch, int *value)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:DOWN? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    *value = atoi(as8Ret);
    return 0;
}
/*
* 设置绝对值编码器的报警响应类型
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:响应类型 0:NONE;1:ALARM;2:STOP;3:ALARM&STOP
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmResponse(ViSession vi, int name, int ch, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:RESPonse %d,%d,%s\n", name, ch, changeResponseToString(value));
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查绝对值编码器的报警响应类型
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:响应类型 0:NONE;1:ALARM;2:STOP;3:ALARM&STOP
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderAlarmResponse_Query(ViSession vi, int name, int ch, int *value)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ALARM:RESPonse? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "NONE") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *value = 0;
    }
    else if (STRCASECMP(as8Ret, "ALARM") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *value = 1;
    }
    else if (STRCASECMP(as8Ret, "STOP") == 0 || STRCASECMP(as8Ret, "2") == 0)
    {
        *value = 2;
    }
    else if (STRCASECMP(as8Ret, "ALARM&STOP") == 0 || STRCASECMP(as8Ret, "3") == 0)
    {
        *value = 3;
    }
    else {
        return -1;
    }
    return 0;
}
/*
* 设置绝对值编码器的零位值
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:编码器零位值(编码器线数)
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderZeroValue(ViSession vi, int name, int ch, int value)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ZERO:VALUe %d,%d,%d\n", name, ch, value);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
* 查绝对值编码器的零位值
* vi :visa设备句柄
* name:设备名称(SEND_ID)
* ch：通道号
* value:编码器零位值(编码器线数)
* 返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQAbsEncoderZeroValue_Query(ViSession vi, int name, int ch, int *value)
{
    char args[SEND_BUF];
    int retLen = 0;
    char as8Ret[100];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:ENCODer:ABS:ZERO:VALUe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    *value = atoi(as8Ret);
    return 0;
}
/*
*设置串口应用配置，配置校验位、数据位、停止位
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*num : 1:UART1; 2:UART2
* baud: 9600,115200,2000000,2500000
*parity:RS232 的校验方式： N(ONE)、 E(VEN) 或 O(DD)
*wordlen:RS232 的数据长度： 8 或 9
*stopbit:RS232 数据帧中停止位的位数： 0:1个停止位；1：2个停止位；
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartConfig(ViSession vi, int num, int name, 
                                     int baud,char parity, int wordlen, int stopbit)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:APPLy %d,%d,%c,%d,%d\n",
             num, name,baud, parity, wordlen, stopbit);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询串口应用配置，配置校验位、数据位、停止位
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*buf:信息
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartConfig_Query(ViSession vi, int num, int name, int *baud,char * parity,int * wordlen,int * stopbit)
{
    char args[SEND_BUF];
    char as8Ret[100];
    char*p,*pNext;
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:APPLy? %d\n", num, name);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if(p)
    {
        *baud = atoi(p);
    }
    else{
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *parity = *p;
    }
    else{
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *wordlen = atoi(p);
    }
    else{
        return -1;
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if(p)
    {
        *stopbit = atoi(p);
    }
    else{
        return -1;
    }
    return 0;
}
/*
*设置串口硬件控制流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*mode:RS232 的流控制方式：  0 - 3 NONE、 RTS、 CTS 或 RTS&CTS
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartFlowctrl(ViSession vi, int num, int name, int mode)
{
    char args[SEND_BUF];
    char *ps8FlowCtrl[4] = { "NONE" ,"RTS" ,"CTS" ,"RTS_CTS" };
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:FLOWctrl %d,%s\n",
             num, name, ps8FlowCtrl[mode]);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询串口硬件控制流
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*mode:RS232 的流控制方式： NONE、 RTS、 CTS 或 RTS&CTS
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartFlowctrl_Query(ViSession vi, int num, int name, char *mode)
{
    char args[SEND_BUF];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:FLOWctrl? %d\n", num, name);
    if ((retLen = busQuery(vi, args, strlen(args), mode, 10)) == 0) {
        return -1;
    }
    else
    {
        mode[retLen - 1] = '\0';
    }
    return 0;
}
/*
*设置串口传感器状态，打开或关闭
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*state:打开或关闭
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorState(ViSession vi, int num, int num1, int name, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:STATe %d,%s\n",
             num, num1, name, state == 0 ? "OFF" : "ON");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询串口传感器状态，打开或关闭
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*state:打开或关闭  0:关闭； 1：打开
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorState_Query(ViSession vi, int num, 
                                                int num1, int name, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:STATe? %d\n", num, num1, name);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if(STRCASECMP(as8Ret,"ON") == 0 || STRCASECMP(as8Ret,"1") == 0)
    {
        *state = 1;
    }
    else{
        *state = 0;
    }
    return 0;
}
/*
*设置传感器配置，数据帧头、帧长度、周期内接收的帧数、切换周期
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*num:要设置的 UART 的串口号：数字 1 或 2
*num1:传感器所在端口号： 1， 2， 3 或 4
*sof:数据帧头
*framelen:帧长度
*num2:帧数
*period:周期
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfAll(ViSession vi, int num, 
                                            int num1, int name, int sof, int framelen, int num2, int period)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:ALL %d,%d,%d,%d,%d\n",
             num, num1, name, sof, framelen, num2, period);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询传感器配置，数据帧头、帧长度、周期内接收的帧数、切换周期
**vi :visa设备句柄
*name:设备名称(SEND_ID)
*sof:数据帧头
*framelen:帧长度
*num2:帧数
*period:周期
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfAll_Query(ViSession vi, int num, 
                                                  int num1, int name, int *sof, int* framelen, int* framenum, int* period)
{
    char args[SEND_BUF];
    char as8Ret[100];
    char*p, *pNext;
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:ALL? %d\n", num, num1, name);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    p = STRTOK_S(as8Ret, ",", &pNext);
    if (p)
    {
        *sof = atoi(p);
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if (p)
    {
        *framelen = atoi(p);
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if (p)
    {
        *framenum = atoi(p);
    }
    p = STRTOK_S(NULL, ",", &pNext);
    if (p)
    {
        *period = atoi(p);
    }
    return 0;
}
/*
*设置数据帧头
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*num:要设置的 UART 的串口号：数字 1 或 2
*num1:传感器所在端口号： 1， 2， 3 或 4
*sof:数据帧头
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfSof(ViSession vi, int num, int num1, int name, int sof)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:SOF %d,%d\n", num, num1, name, sof);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询数据帧头
*vi :visa设备句柄
*name :设备名称(SEND_ID)
*num : 要设置的 UART 的串口号：数字 1 或 2
* num1 : 传感器所在端口号： 1， 2， 3 或 4
* sof : 数据帧头
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfSof_Query(ViSession vi, int num, int num1, int name, int *sof)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:SOF? %d\n", num, num1, name);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen] = '\0';
    }
    *sof = atoi(state);
    return 0;
}
/*
*设置帧长度
*vi :visa设备句柄
*name :设备名称(SEND_ID)
*num : 要设置的 UART 的串口号：数字 1 或 2
* num1 : 传感器所在端口号： 1， 2， 3 或 4
*len:帧长度
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfFrameLen(ViSession vi, int num, int num1, int name, int len)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:FRAMELen %d,%d\n",
             num, num1, name, len);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询帧长度
*vi :visa设备句柄
*name :设备名称(SEND_ID)
*num : 要设置的 UART 的串口号：数字 1 或 2
* num1 : 传感器所在端口号： 1， 2， 3 或 4
*len:帧长度
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfFrameLen_Query(ViSession vi, 
                                                       int num, int num1, int name, int *len)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:FRAMELen? %d\n",
             num, num1, name);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen] = '\0';
    }
    *len = atoi(state);
    return 0;
}
/*
*设置帧个数
*vi :visa设备句柄
*name : 设备名称(SEND_ID)
*num : 要设置的 UART 的串口号：数字 1 或 2
* num1 : 传感器所在端口号： 1， 2， 3 或 4
*num2:帧个数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfRecvNum(ViSession vi, int num, 
                                                int num1, int name, int num2)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:NUM %d,%d\n",
             num, num1, name, num2);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询帧个数
*vi :visa设备句柄
*name :设备名称(SEND_ID)
*num : 要设置的 UART 的串口号：数字 1 或 2
* num1 : 传感器所在端口号： 1， 2， 3 或 4
*num2:帧个数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfRecvNum_Query(ViSession vi, int num, 
                                                      int num1, int name, int *num2)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:NUM? %d\n", num, num1, name);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen] = '\0';
    }
    *num2 = atoi(state);
    return 0;
}
/*
*设置周期
*vi :visa设备句柄
*name :设备名称(SEND_ID)
*num : 要设置的 UART 的串口号：数字 1 或 2
* num1 : 传感器所在端口号： 1， 2， 3 或 4
*period:周期
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfPeriod(ViSession vi, int num, int num1, int name, int period)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:PERIod %d,%d\n",
             num, num1, name, period);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询周期
*vi :visa设备句柄
*name : 设备名称(SEND_ID)
*num : 要设置的 UART 的串口号：数字 1 或 2
* num1 : 传感器所在端口号： 1， 2， 3 或 4
*period:周期
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorConfPeriod_Query(ViSession vi, int num, 
                                                     int num1, int name, int *period)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:CONFig:PERIod? %d\n", num, num1, name);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen] = '\0';
    }
    *period = atoi(state);
    return 0;
}
/*
*查询传感器数据
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*buf:数据
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQUartSensorData_Query(ViSession vi, int num, int num1, int name, char *buf)
{
    char args[SEND_BUF];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:UART%d:SENSor%d:DATA? %d\n", num, num1, name);
    if ((retLen = busQuery(vi, args, strlen(args), buf, 10)) == 0) {
        return -1;
    }
    else
    {
        buf[retLen - 1] = '\0';
    }
    return 0;
}
/*
*查询测距报警的状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:状态  0:禁止； 1：使能
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDistanceAlarmState_Query(ViSession vi, int name, int ch, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DALarm:STATe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "OFF") == 0 || STRCASECMP(as8Ret, "0") == 0)
    {
        *state = 0;
    }
    else if (STRCASECMP(as8Ret, "ON") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = 1;
    }
    return 0;
}
/*
*设置测距报警的状态
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state1:状态
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDistanceAlarmState(ViSession vi, int name, int ch, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DALarm:STATe %d,%d,%s\n", name, ch, state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*设置测距报警的响应距离
*alarm:ALARm1,ALARm2,ALARm3
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*distance:测距报警的响应距离
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDistanceAlarm(ViSession vi, int num, int name, int ch, float distance)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DALarm:ALARm%d:DISTance %d,%d,%f\n", num, name, ch, distance);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询测距报警的响应距离
*vi :visa设备句柄
*alarm:ALARm1,ALARm2,ALARm3
*name:设备名称(SEND_ID)
*ch：通道号
*distance:测距报警的响应距离
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQDistanceAlarm_Query(ViSession vi, int num, int name, int ch, float *distance)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:DALarm:ALARm%d:DISTance? %d,%d\n", num, name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *distance = strtof(state,NULL);
    return 0;
}
/*
*查询驱动板类型（只支持10轴）
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*type:驱动板类型
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQNewDriverType_Query(ViSession vi, int name, int ch, char *type)
{
    char args[SEND_BUF];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:NDRiver:TYPe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), type, 10)) == 0) {
        return -1;
    }
    else
    {
        type[retLen - 1] = '\0';
    }
    return 0;
}
/*
*设置驱动板电流（只支持10轴）
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*current:驱动板电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQNewDriverCurrent(ViSession vi, int name, float current)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:NDRiver:CURRent %d,%f\n", name, current);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询驱动板电流（只支持10轴）
*vi :visa设备句柄
*name: 设备名称(SEND_ID)
*current:驱动板电流
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQNewDriverCurrent_Query(ViSession vi, int name, float *current)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:NDRiver:CURRent? %d\n", name);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *current = strtof(state,NULL);
    return 0;
}
/*
*设置电机微步数（只支持10轴）
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*microstep:电机的微步数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQNewDriverMicrosteps(ViSession vi, int name, int microstep)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:NDRiver:MICRosteps %d,%d\n", name, microstep);
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}
/*
*查询电机微步数（只支持10轴）
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*microstep:电机的微步数
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQNewDriverMicrosteps_Query(ViSession vi, int name, int *microstep)
{
    char args[SEND_BUF];
    char state[10];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:NDRiver:MICRosteps? %d\n", name);
    if ((retLen = busQuery(vi, args, strlen(args), state, 10)) == 0) {
        return -1;
    }
    else
    {
        state[retLen - 1] = '\0';
    }

    *microstep = atoi(state);
    return 0;
}
/*
*查询驱动开关状态（只支持10轴）
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state:状态  0：禁止； 1：使能
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQNewDriverState_Query(ViSession vi, int name, int ch, int *state)
{
    char args[SEND_BUF];
    char as8Ret[100];
    int retLen = 0;
    snprintf(args, SEND_BUF, "DEVICE:MRQ:NDRiver:STATe? %d,%d\n", name, ch);
    if ((retLen = busQuery(vi, args, strlen(args), as8Ret, 100)) == 0) {
        return -1;
    }
    else
    {
        as8Ret[retLen - 1] = '\0';
    }
    if (STRCASECMP(as8Ret, "ON") == 0 || STRCASECMP(as8Ret, "1") == 0)
    {
        *state = 1;
    }
    else
    {
        *state = 0;
    }
    return 0;
}
/*
*设置驱动开关状态（只支持10轴）
*vi :visa设备句柄
*name:设备名称(SEND_ID)
*ch：通道号
*state1:状态
*返回值：0表示执行成功，－1表示失败
*/
EXPORT_API int CALL mrgMRQNewDriverState(ViSession vi, int name, int devList, int state)
{
    char args[SEND_BUF];
    snprintf(args, SEND_BUF, "DEVICE:MRQ:NDRiver:STATe %d,%d,%s\n", name, devList, state ? "ON" : "OFF");
    if (busWrite(vi, args, strlen(args)) == 0) {
        return -1;
    }
    return 0;
}


