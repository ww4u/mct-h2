#include "lanfinddevice.h"

//#include "stdafx.h"
#include <WINSOCK2.H>
#include <stdio.h>
#include <Ws2tcpip.h> //�鲥Ҫ�õ���ͷ�ļ�
#include <string.h>
#define ADD_GROUP//���Բ�������Ҳ���������ڷ���Ϣ
#define MCASTADDR "224.0.0.251"
#define UDP_PORT  5353//5566
#pragma comment(lib, "ws2_32")

#include <iostream>
#include <IPHlpApi.h>
#pragma comment(lib,"IPHlpApi.lib") //��Ҫ���Iphlpapi.lib��

//#define DEBUG_INFO printf("%s,%s,%d\n",__FILE__,__func__,__LINE__)

using namespace std;

int getHostIpAddr(string* strHostIp, unsigned int len);

#if 1
/*
    timeout һ�β�ѯ�豸���ʱ��
    desc :��ѯ�����豸������
    �����ҵ��������ĸ���
*/
int findResources(char* ip,int timeout)
{
    WSADATA ws;
    int ret,count = 0, hostIpCount = 0;
    SOCKET sock[1024];
    SOCKADDR_IN localAddr, remoteAddr, servaddr;
    int len = sizeof(SOCKADDR);
    char recvBuf[50];
    char s;
    int timeout_s = timeout*1000; //��תΪ����
    ip[0] = 0;

	string strHostIpAddr[1024];

	hostIpCount = getHostIpAddr(strHostIpAddr, 1024);
	if (hostIpCount <= 0)
	{
		return -1;
	}


    ret = WSAStartup(MAKEWORD(2, 2), &ws);
    if (0 != ret)
    {
        return -1;
    }

	for (int i = 0; i < hostIpCount; i++)
	{
		sock[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (INVALID_SOCKET == sock[i])
		{
			WSACleanup();
			return -1;
		}
		localAddr.sin_family = AF_INET;
		localAddr.sin_port = htons(0);
		localAddr.sin_addr.S_un.S_addr = inet_addr(strHostIpAddr[i].c_str());//htonl(INADDR_ANY);
		ret = bind(sock[i], (SOCKADDR*)&localAddr, sizeof(SOCKADDR));
		if (SOCKET_ERROR == ret)
		{
			return -1;
		}

		ret = setsockopt(sock[i], SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_s, sizeof(int));
		if (ret != 0)
		{
			printf("SO_RCVTIMEO ERROR!CODE IS:%d\n", WSAGetLastError());
			return -1;
		}

		bool bOpt = true;
		//���ø��׽���Ϊ�㲥����
		setsockopt(sock[i], SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));
	}


	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(6000);
	remoteAddr.sin_addr.S_un.S_addr = inet_addr("255.255.255.255");//htonl(INADDR_ANY);

	for (int i = 0; i < hostIpCount; i++)
	{
		sendto(sock[i], "*?", 2, 0, (SOCKADDR*)&remoteAddr, sizeof(SOCKADDR));

		while (1)
		{
			memset(recvBuf, 0, 50);
			try
			{
				ret = recvfrom(sock[i], recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&servaddr, &len);
			}
			catch (exception& e)
			{
				break;
			}

			if (ret > 0)
			{
				count++;
				strcat_s(ip, 1024, inet_ntoa(servaddr.sin_addr));
				strcat_s(ip, 1024, ";");
				//printf("%s response to us : %s \n", inet_ntoa(servaddr.sin_addr), recvBuf);
			}
			else
			{
				break;
			}
			
		}
	}
    
   
	for (int i = 0; i < hostIpCount; i++)
	{
		shutdown(sock[i], 2);//�����"2"��win32�п���дΪSD_BOTH
		closesocket(sock[i]);
	}

    WSACleanup();

    return count;
}
#endif


int getHostIpAddr(string* strHostIp, unsigned int len)
{
	//PIP_ADAPTER_INFO�ṹ��ָ��洢����������Ϣ
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//�õ��ṹ���С,����GetAdaptersInfo����
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	//��¼��������
	int netCardNum = 0;

	int ipcount = 0;

	//��¼ÿ�������ϵ�IP��ַ����
	int IPnumPerNetCard = 0;
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//����������ص���ERROR_BUFFER_OVERFLOW
		//��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
		//��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������
		//�ͷ�ԭ�����ڴ�ռ�
		delete pIpAdapterInfo;
		//���������ڴ�ռ������洢����������Ϣ
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}
	if (ERROR_SUCCESS == nRel)
	{
		//���������Ϣ
		//�����ж�����,���ͨ��ѭ��ȥ�ж�
		while (pIpAdapterInfo)
		{			
			IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);

			do
			{
				//cout << "�������ϵ�IP������" << ++IPnumPerNetCard << endl;
				//cout << "IP ��ַ��" << pIpAddrString->IpAddress.String << endl;
				//cout << "������ַ��" << pIpAddrString->IpMask.String << endl;
				//cout << "���ص�ַ��" << pIpAdapterInfo->GatewayList.IpAddress.String << endl;

				if (strcmp("0.0.0.0", pIpAddrString->IpAddress.String) == 0)
				{

				}
				else
				{
					strHostIp[ipcount] = string(pIpAddrString->IpAddress.String);
					ipcount++;
				}



				pIpAddrString = pIpAddrString->Next;
			} while (pIpAddrString);
			pIpAdapterInfo = pIpAdapterInfo->Next;
			//cout << "--------------------------------------------------------------------" << endl;
		}

	}
	//�ͷ��ڴ�ռ�
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}

	return ipcount;
}
