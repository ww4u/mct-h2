#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h> 

#define strcpy_s(x,y,z)     	strncpy(x,z,y)

#ifdef _WIN32
//////////////////////////////////
//Windows头文件

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WINXPSP3
#endif

#include <windows.h>
#include <winsock2.h>
#include <IPHlpApi.h>

#include "visatype.h"
#include "visa.h"

#define STRCASECMP(x,y)		_stricmp(x,y)
#define STRTOK_S(x,y,z)		strtok_s(x,y,z)

#else
//////////////////////////////////
//Linux 头文件
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define INVALID_SOCKET 	(-1)
#define SOCKET 		int
#define SOCKET_ERROR 	(-1)
#define SOCKADDR 	struct sockaddr
#define SOCKADDR_IN 	struct sockaddr_in

#define IFRSIZE 	((int)(size*sizeof(struct ifreq)))

typedef unsigned long ViSession;
#include "vxi11/vxi11_user.h"

#define STRCASECMP(x,y)		strcasecmp(x,y)
#define STRTOK_S(x,y,z)     	strtok_r(x,y,z)
//#define _strnicmp           strncasecmp
#define Sleep(x)            usleep( ((x) * 1000) )


#endif //_WIN32




#endif //__PLATFORM_H__
