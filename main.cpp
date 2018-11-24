#include <winsock2.h>  
#include <WS2tcpip.h>
#include<stdio.h>
#include<iostream>
#include<cstring>
 
using namespace std;
#pragma comment(lib, "ws2_32.lib")

bool send_data(int fd, const char *data, int len)
{
	int sl=0;
	while (sl<len)
	{
		int s = send(fd,data+sl,len-sl,0);
		if(s<=0)
		{
			printf("send error:%d\n",s);
			return false;
		}
		sl+=s;
	}
	return true;
}
void output_binary(const char*tag,char* data,int len)
{
	printf("%s { ",tag);
	for(int i=0;i<len;i++)
	{
		printf("%hhx ",data[i]);
	}
	printf(" }\n");
}
char BUF[1024];
int main()
{
	printf("I'm socks5 client demo on windows\n");
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(3702);
	inet_pton(AF_INET, "127.0.0.1", &serAddr.sin_addr);
 
	while (true) 
	{
		SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sclient == INVALID_SOCKET)
		{
			printf("invalid socket!\n");
			break;
		}
		if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{  
			printf("connect error !\n");
			closesocket(sclient);
			break;
		}
		printf("connect success\n");
		{
			std::string sd;
			char version = 0x05;
			char nmethods = 1;
			char methods = 0x00;
			sd.append(&version, 1);
			sd.append(&nmethods, 1);
			sd.append(&methods, 1);
			if (!send_data(sclient, sd.c_str(), sd.size()))
			{
				printf("send data error\n");
				break;
			}
			int ret = recv(sclient, BUF, sizeof(BUF), 0);
			if (ret == 0)
			{
				printf("connect closed by peer\n");
				break;
			}
			else if (ret < 0)
			{
				printf("something error");
				break;
			}
			output_binary("recv_data", BUF, ret);
			if (BUF[0] != 0x05 || BUF[1] != 0x00 || ret != 2)
			{
				printf("recv socks5 not satisfy respect\n");
				break;
			}
		}
		{
			printf("send command ...\n");
			std::string sd;
			sd.push_back(0x05);
			sd.push_back(0x01);
			sd.push_back(0x00);
			sd.push_back(0x01);//try ipv4
			unsigned ip=0;
			inet_pton(AF_INET,"216.58.203.14",&ip);
			sd.append((char*)&ip,sizeof(ip));
			unsigned short port=htons(80);
			sd.append((char*)&port,sizeof(port));
			
			if (!send_data(sclient, sd.c_str(), sd.size()))
			{
				printf("send data error\n");
				break;
			}
			int ret = recv(sclient, BUF, sizeof(BUF), 0);
			if (ret == 0)
			{
				printf("connect closed by peer\n");
				break;
			}
			else if (ret < 0)
			{
				printf("something error");
				break;
			}
			output_binary("recv_data2", BUF, ret);
			if(BUF[1]!=0)
			{
				printf("connect error\n");
				break;
			}
			printf("socks5 handshake successful\n");
		}
		{
			std::string sd;
			sd.append("GET / HTTP/1.1\r\n");
			sd.append("Host: www.google.com.hk\r\n");
			sd.append("Connection: close\r\n\n");//close connection after get result
			
			if (!send_data(sclient, sd.c_str(), sd.size()))
			{
				printf("send data error\n");
				break;
			}

			while(true)
			{
				int ret = recv(sclient, BUF, sizeof(BUF), 0);
				if (ret == 0)
				{
					printf("finish receiving connect closed by peer\n");
					break;
				}
				else if (ret < 0)
				{
					printf("something error");
					break;
				}
				else
				{
					for(int i=0;i<ret;i++)
					{
						printf("%c",BUF[i]);
					}
				}
			}
		}
		closesocket(sclient);
		Sleep(1000);
		break;
	}
	WSACleanup();
	system("pause");
	return 0;
}
