#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "OnlineStar_Server.h"
#include "console.h"

stClient g_Clients[USER_MAX];
SOCKET listen_sock;

void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("Error[%s] : %s\n", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main()
{
	if(InitServer())
		OnServer();

	while (1)
	{
		Network();
		Draw();
	}
}

BOOL InitServer()
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))	return -1;

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)	err_display("listen_sock create error");

	SOCKADDR_IN sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(3000);
	retval = bind(listen_sock, (SOCKADDR *)&sockaddr, sizeof(sockaddr));
	if (retval == SOCKET_ERROR)		err_display("binding error");

	IDkey = 1;
	cs_Initial();
}

void OnServer()
{
	int retval;

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)		err_display("OnServer Error");
}

void Network()
{
	int retval;

	FD_SET ReadSet;
	FD_ZERO(&ReadSet);
	FD_SET(listen_sock, &ReadSet);

	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Clients[iCnt].socket != INVALID_SOCKET)
			FD_SET(g_Clients[iCnt].socket, &ReadSet);
	}

	TIMEVAL Timeval;
	Timeval.tv_sec = 0;
	Timeval.tv_usec = 0;

	retval = select(0, &ReadSet, NULL, NULL, &Timeval);
	if (retval > 0)
	{
		if (FD_ISSET(listen_sock, &ReadSet))
		{
			//ID할당
		}

		for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
		{
			if (g_Clients[iCnt].socket != INVALID_SOCKET && FD_ISSET(g_Clients[iCnt].socket, &ReadSet))
			{
				//recv 처리
			}
		}
	}
}

void Draw()
{

}

void SendUnicast()
{

}

void SendBroadcast()
{

}