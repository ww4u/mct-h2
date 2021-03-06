#include "bus.h"

#if defined(_WIN32) && (!defined(__MINGW64__)) && (!defined(__MINGW32__))
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "visa32.lib")
#pragma comment (lib,"IPHlpApi.lib")
#pragma comment (lib, "pthreadVC2.lib")
#endif

#define ADD_GROUP
#define MCASTADDR "224.0.0.251"
#define UDP_PORT  5353
#define MEGA_TCP_SOCKET_PORT (5555)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define LOCK()     pthread_mutex_lock(&mutex)
#define UNLOCK()   pthread_mutex_unlock(&mutex)


int socketFindResources(char ip[][100], int timeout);

#ifndef _WIN32
int busFindDevice(int bus, char *output, int len,int method)
{
    int r = 0;
    if (bus == 0)
    {
        if (method == 0)
        {
            strcpy(output, "");
            goto END;
        }
        else if (method == 1)
        {
            char ip_list[256][100];
            int status = socketFindResources(ip_list, 500);
            int i;
            for (i = 0; i < status; i++)
            {
                snprintf(&output[r], len - r, "TCPIP0::%s::inst0::INSTR,", ip_list[i]);
                r = strlen(output);
            }
        }
    }
    else if (bus == 1) //USBTMC
    {
        strcpy(output, "");
    }

END:
    return 0;
}

char _g_device_IP[512] = ""; //设备IP，用于关闭
int _g_timeout = 2000; //收发超时时间
VXI11_CLINK *_g_clink = NULL;    //vxi11句柄

int busOpenDevice(char *ip, int timeout_ms)
{
    memset(_g_device_IP,'\0',sizeof(_g_device_IP));
    _g_clink = NULL;
    _g_timeout = 2000;

    // TCPIP0::192.168.1.2::inst0::INSTR,
    char buff[20][64] = {""};
    char *p, *pNext;
    int index = 0;
    p = STRTOK_S(ip, "::", &pNext);
    while ( p && (index<20) )
    {
        strcpy(buff[index++], p);
        p = STRTOK_S(NULL, "::", &pNext);
    }
    char *strIP = buff[1];

    VXI11_CLINK *clink; //sizeof(VXI11_CLINK)
    if(vxi11_open_device(&clink, strIP, NULL)){
        //        printf("vxi11_open_device error: %s\n", strIP);
        free(clink);
        return -1;
    }

    strcpy(_g_device_IP, strIP);
    _g_clink = clink;
    _g_timeout = timeout_ms;

    srand((int)time(0));
    int value = (unsigned int)rand()/10000+1;
    return value;//随便返回一个非零数字
}

static int SyncSend(int vi, char *buf, int dataLen, int isBlock)
{
    if(_g_clink == NULL)
        return -1;

    int ret = vxi11_send(_g_clink, buf, dataLen);
    if(ret > 0)
        return ret;
    if(ret < 0){
        perror("SyncSend error!");
    }
    return -1;
}

static int SyncRead(int vi, char *data, int dataLen, int isBlock)
{
    if(_g_clink == NULL)
        return -1;

    int ret = -1;
    if(isBlock)
    {
        ret = vxi11_receive(_g_clink, data, dataLen);
    }
    else
    {
        // -VXI11_NULL_READ_RESP 超时未判断
        ret = vxi11_receive_timeout(_g_clink, data, dataLen, _g_timeout);
    }
    if(ret > 0){
        return ret;
    }
    if(ret < 0){
        perror("SyncRead error!");
    }
    return -1;
}

int busCloseDevice(ViSession vi)
{
    if( (_g_clink != NULL) && (0!=strcmp(_g_device_IP,"")) )
    {
        vxi11_close_device(_g_clink, _g_device_IP);
    }

    memset(_g_device_IP,'\0',sizeof(_g_device_IP));
    _g_clink = NULL;
    return 0;
}

unsigned int busWrite(ViSession vi, char *data, unsigned int len)
{
    LOCK();
    int retCount = SyncSend(vi, data, len, 1);
    if(retCount < 0){
        UNLOCK();
        return 0;
    }
    //    printf("\nTO:\n\t%s", data);
    UNLOCK();
    return (unsigned int)retCount;
}

unsigned int busRead(ViSession vi, char *buf, unsigned int len)
{
    int errCount = 3;
    int retCount;
    LOCK();
    while(errCount--)
    {
        retCount = SyncRead(vi, buf, len, 1);
        if(retCount > 0)
        {
            break;
        }
        retCount=0;
        msSleep(1);
    }

    if( (retCount==0) || STRCASECMP(buf, "Command error") == 0 )
    {
        UNLOCK();
        return 0;
    }

    UNLOCK();
    return (unsigned int)retCount;
}

unsigned int busQuery(ViSession vi, char * input, unsigned int inputlen, char* output, unsigned int wantlen)
{
    int retCount;
    int errCount = 3;
    LOCK();
    retCount = SyncSend(vi, input, inputlen, 1);
    if(retCount < 0)
    {
        UNLOCK();
        return 0;
    }

    while(errCount--)
    {
        retCount = SyncRead(vi, output, wantlen, 1);
        if(retCount > 0)
        {
            break;
        }
        retCount=0;
        msSleep(1);
    }
    if( (retCount==0) || STRCASECMP(output, "Command error") == 0 )
    {
        UNLOCK();
        return 0;
    }

    UNLOCK();
    return (unsigned int)retCount;
}

ViSession busOpenSocket(const char *pName, const char *addr, unsigned int port)
{
    //! FIXME
}


/* strHostIp:返回的IP地址
 *  len: IP地址数组长度
 */
int getHostIpAddr(char strHostIp[][100])
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[2048];

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        return -2;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len/sizeof(struct ifreq));

    int count = 0;
    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0){
            return -3;
        }else{
            if (!(ifr.ifr_flags & IFF_LOOPBACK))
            { // don't count loopback
                if (ioctl(sock, SIOCGIFADDR, &ifr) == 0){
                    //printf("IP address is: %s\n", inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr));
                }else
                    continue;
                sprintf(strHostIp[count],"%s", inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr));
                count++;
            }
        }
    }

    return count;
}


#else //_WIN32

/*
 * 对于网线连接的网关，查找方式有两种： 一种是用VISA方式查找，另一种是用UDP广播方式查找。
 * method: 0:表示使用VISA方式查找；1表示使用UDP方式查找
 * 当method=0时，按照VISA的规范，需要指定总线类型，所以使用bus来传入总线类型。
 * output为输出参数，输出找到的设备信息
 */
ViSession defaultRM;
int busFindDevice(int bus, char *output, int len,int method)
{
    ViStatus status;
    int r = 0;
    static ViUInt32 numInstrs;
    static ViFindList findList;
    char instrDescriptor[VI_FIND_BUFLEN];
    /* First we will need to open the default resource manager. */
    status = viOpenDefaultRM(&defaultRM);
    if (status < VI_SUCCESS)
    {
        printf("Could not open a session to the VISA Resource Manager!\n");
        return 0;
    }
    /*
     * Find all the VISA resources in our system and store the number of resources
     * in the system in numInstrs.  Notice the different query descriptions a
     * that are available.
     Interface         Expression
     --------------------------------------
     GPIB              "GPIB[0-9]*::?*INSTR"
     VXI               "VXI?*INSTR"
     GPIB-VXI          "GPIB-VXI?*INSTR"
     Any VXI           "?*VXI[0-9]*::?*INSTR"
     Serial            "ASRL[0-9]*::?*INSTR"
     PXI               "PXI?*INSTR"
     All instruments   "?*INSTR"
     All resources     "?*"
    */
    if (bus == BUS_LAN)
    {
        if (method == METHOD_VISA)
        {
            status = viFindRsrc(defaultRM, "TCPIP[0-9]*::?*INSTR", &findList, &numInstrs, instrDescriptor);
            if (status < VI_SUCCESS)
            {
                viClose(defaultRM);
                return 0;
            }
            strcpy(&output[r], instrDescriptor);
            while (--numInstrs)
            {
                /* stay in this loop until we find all instruments */
                status = viFindNext(findList, instrDescriptor);  /* find next desriptor */
                if (status < VI_SUCCESS)
                {
                    goto END;
                }
                strcat(&output[r], ",");
                strcat(output, instrDescriptor);
            }    /* end while */
        }
        else if (method == METHOD_UDP)
        {
            char ip_list[256][100];
            status = socketFindResources(ip_list, 500);
            for (int i = 0; i < status; i++)
            {
                snprintf(&output[r], len - r, "TCPIP0::%s::inst0::INSTR,", ip_list[i]);
                r = strlen(output);
            }
        }
    }
    else if (bus == BUS_USB) //USBTMC
    {
        status = viFindRsrc(defaultRM, "USB0::?*::INSTR", &findList, &numInstrs, instrDescriptor);
        if (status < VI_SUCCESS)
        {
            viClose(defaultRM);
            return 0;
        }
        strcpy(&output[r], instrDescriptor);
        while (--numInstrs)
        {
            /* stay in this loop until we find all instruments */
            status = viFindNext(findList, instrDescriptor);  /* find next desriptor */
            if (status < VI_SUCCESS)
            {
                goto END;
            }
            strcat(&output[r], ",");
            strcat(output, instrDescriptor);
        }    /* end while */
    }
    
END:
    return 0;
}

int busOpenDevice(char * ip, int timeout)
{
    ViStatus status;
    ViSession vi;
    status = viOpenDefaultRM(&defaultRM);
    if (status < VI_SUCCESS)
    {
        printf("Could not open a session to the VISA Resource Manager!\n");
        return 0;
    }
    status = viOpen(defaultRM, ip, VI_NO_LOCK, VI_TMO_IMMEDIATE, &vi);
    if (status < VI_SUCCESS)
    {
        vi = -1;
        return status;
    }
    if (_strnicmp(ip, "USB",3)== 0)
    {
        viSetAttribute(vi, VI_ATTR_TERMCHAR, 0x0A);
        viSetAttribute(vi, VI_ATTR_TERMCHAR_EN, VI_FALSE);
        viSetAttribute(vi, VI_ATTR_SEND_END_EN, VI_TRUE);
        viSetAttribute(vi, VI_ATTR_IO_PROT, 1);
    }
    else if (_strnicmp(ip, "TCPIP", 5) == 0)
    {
        viSetAttribute(vi, VI_ATTR_TCPIP_NODELAY, VI_TRUE);
        viSetAttribute(vi, VI_ATTR_TCPIP_KEEPALIVE, VI_TRUE);
    }
    viSetAttribute(vi, VI_ATTR_TMO_VALUE, timeout);
    return vi;
}

ViSession busOpenSocket(const char *pName, const char *addr, unsigned int port)
{
    ViStatus viSta;
    ViSession viDef;
    ViSession viDev;
    viSta = viOpenDefaultRM(&viDef);
    if (viSta != VI_SUCCESS)
    {
        return 0;
    }

    //! cat name
    char rsrc[64];
    snprintf(rsrc, 64, "TCPIP::%s::%d::SOCKET", addr, port);

    viSta = viOpen(viDef, rsrc, 0, 2000, &viDev);
    if (viSta != VI_SUCCESS)
    {
        return 0;
    }

    //set attribute
    viSetAttribute(viDev, VI_ATTR_TERMCHAR, 0X0A);
    viSetAttribute(viDev, VI_ATTR_TERMCHAR_EN, VI_TRUE);

    //set the name
    viPrintf(viDev, "%s\n", pName);
    viFlush(viDev, VI_WRITE_BUF);
    return viDev;
}

int busCloseDevice(ViSession vi)
{
    return viClose(vi);
}

unsigned int busWrite(ViSession vi, char * buf, unsigned int len)
{
    //返回VI_ERROR_CONN_LOST 表示断开连接
    ViUInt32 retCount;
    LOCK();
    if(viWrite(vi, (ViBuf)buf, len, &retCount) != VI_SUCCESS)
    {
        UNLOCK();
        return 0;
    }
    UNLOCK();
    return retCount;
}

unsigned int busRead(ViSession vi, char * buf, unsigned int len)
{
    //返回VI_ERROR_CONN_LOST 表示断开连接
    ViUInt32 retCount = 0;
    int errCount = 3;

    LOCK();
    while(errCount--)
    {
        viRead(vi, (ViBuf)buf, len, &retCount);
        if(retCount > 0)
        {
            break;
        }
        retCount = 0;
        msSleep(1);
    }
    if( (retCount == 0) ||  STRCASECMP(buf, "Command error") == 0 )
    {
        UNLOCK();
        return 0;
    }

    UNLOCK();
    return retCount;
}

unsigned int busQuery(ViSession vi, char * input, unsigned int inputlen,char* output, unsigned int wantlen)
{
    ViUInt32 retCount;
    int errCount = 3;

    LOCK();
    if (viWrite(vi, (ViBuf)input, inputlen, &retCount) != VI_SUCCESS)
    {
        UNLOCK();
        return 0;
    }
    while(errCount--)
    {
        viRead(vi, (ViBuf)output, wantlen, &retCount);
        if(retCount > 0)
        {
            break;
        }
        retCount = 0;
        msSleep(1);
    }
    if( (retCount == 0) ||  STRCASECMP(output, "Command error") == 0 )
    {
        UNLOCK();
        return 0;
    }

    UNLOCK();
    return retCount;
}
/* strHostIp:返回的IP地址
 *  len: IP地址数组长度
 */
int getHostIpAddr(char strHostIp[][100])
{
    //得到结构体大小,用于GetAdaptersInfo参数
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //PIP_ADAPTER_INFO结构体指针存储本机网卡信息
    PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(stSize);
    //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    //记录网卡数量
    int ipcount = 0;

    //记录每张网卡上的IP地址数量
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        //如果函数返回的是ERROR_BUFFER_OVERFLOW
        //则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
        //这也是说明为什么stSize既是一个输入量也是一个输出量
        //释放原来的内存空间
        free(pIpAdapterInfo);
        //重新申请内存空间用来存储所有网卡信息
        pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(stSize);
        //再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    }
    if (ERROR_SUCCESS == nRel)
    {
        //输出网卡信息
        //可能有多网卡,因此通过循环去判断
        while (pIpAdapterInfo)
        {
            //可能网卡有多IP,因此通过循环去判断
            IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            do
            {
                if (strcmp("0.0.0.0", pIpAddrString->IpAddress.String) != 0)
                {
                    strcpy_s(strHostIp[ipcount],100, pIpAddrString->IpAddress.String);
                    ipcount++;
                }
                pIpAddrString = pIpAddrString->Next;
            } while (pIpAddrString);
            pIpAdapterInfo = pIpAdapterInfo->Next;
            //cout << "--------------------------------------------------------------------" << endl;
        }
    }
    //释放内存空间
    if (pIpAdapterInfo)
    {
        free(pIpAdapterInfo);
    }
    return ipcount;
}
#endif //_WIN32

/*
  ip: 输出参数，返回找到的IP地址
  timeout 一次查询设备的最长时间
  len :ip长度
  返回找到的仪器的个数
*/
int socketFindResources(char ip[][100],int timeout_ms)
{
#if _WIN32
    WSADATA ws;
#endif
    int ret;
    int count = 0;
    int hostIpCount = 0;
    int i;
    SOCKET sock[256];
    SOCKADDR_IN localAddr;
    SOCKADDR_IN remoteAddr;
    SOCKADDR_IN servaddr;

#ifdef _WIN32
    int len = sizeof(SOCKADDR);
#else
    socklen_t len = sizeof(SOCKADDR);
#endif
    char recvBuf[50];
    
    char strHostIpAddr[256][100];
    hostIpCount = getHostIpAddr(strHostIpAddr);
    if (hostIpCount <= 0)
    {
        return -1;
    }

#if _WIN32
    ret = WSAStartup(MAKEWORD(2, 2), &ws);
    if (0 != ret)
    {
        return -1;
    }
#endif	
    for (i = 0; i < hostIpCount; i++)
    {
        sock[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (INVALID_SOCKET == sock[i])
        {
#ifdef _WIN32		
            WSACleanup();
#endif
            return -1;
        }
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(0);
#ifdef _WIN32
        localAddr.sin_addr.S_un.S_addr = inet_addr(strHostIpAddr[i]);//htonl(INADDR_ANY);
#else
        localAddr.sin_addr.s_addr  = inet_addr(strHostIpAddr[i]);//htonl(INADDR_ANY);
#endif
        ret = bind(sock[i], (SOCKADDR*)&localAddr, sizeof(SOCKADDR));
        if (SOCKET_ERROR == ret)
        {
            return -1;
        }

#ifdef _WIN32
        ret = setsockopt(sock[i], SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_ms, sizeof(int));
#else
        struct timeval timeout;
        timeout.tv_sec = timeout_ms/1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        ret = setsockopt(sock[i], SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
#endif
        if (ret != 0)
        {
            //            printf("SO_RCVTIMEO ERROR!CODE IS:%d\n", WSAGetLastError());
            return -1;
        }
#ifdef _WIN32		
        char bOpt = 1;
#else
        int bOpt = 1;
#endif
        //设置该套接字为广播类型
#ifdef _WIN32
        ret = setsockopt(sock[i], SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));
#else
        ret = setsockopt(sock[i], SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, (char*)&bOpt, sizeof(bOpt));
#endif
        if(ret != 0)
        {
            return -1;
        }
    }
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(6000);
#ifdef _WIN32
    remoteAddr.sin_addr.S_un.S_addr = inet_addr("255.255.255.255");//htonl(INADDR_ANY);
#else
    remoteAddr.sin_addr.s_addr = inet_addr("255.255.255.255");//htonl(INADDR_ANY);
#endif

    for (i = 0; i < hostIpCount; i++)
    {
        sendto(sock[i], "*?", 2, 0, (SOCKADDR*)&remoteAddr, sizeof(SOCKADDR));
        while (1)
        {
            memset(recvBuf, 0, 50);
            ret = recvfrom(sock[i], recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&servaddr, &len);
            if (ret > 0)
            {
                strcpy_s(ip[count], 100, inet_ntoa(servaddr.sin_addr));
                count++;
                //printf("%s response to us : %s \n", inet_ntoa(servaddr.sin_addr), recvBuf);
            }
            else
            {
                break;
            }
        }
    }
    for (i = 0; i < hostIpCount; i++)
    {
#ifdef _WIN32
        shutdown(sock[i], 2);//这里的"2"在win32中可以写为SD_BOTH
        closesocket(sock[i]);
#else
        close(sock[i]);
#endif		
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return count;
}





