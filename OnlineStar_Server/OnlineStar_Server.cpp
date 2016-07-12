/*--------------------------------------------------------------------*/
// 
// OnlineStar_Server.cpp
// 별 움직이기 서버
//
/*--------------------------------------------------------------------*/
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "OnlineStar_Server.h"
#include "console.h"

#pragma comment(lib, "Ws2_32.lib")

/*--------------------------------------------------------------------*/
// 플레이어, 소켓
/*--------------------------------------------------------------------*/
stClient g_Clients[USER_MAX];
SOCKET listen_sock;

/*--------------------------------------------------------------------*/
// 함수
/*--------------------------------------------------------------------*/
void SendUnicast(SOCKET sock, stPacket buf, int size);
void SendBroadcast(stPacket buf, int size);
void ClientDisconnect(DWORD ID);

/*--------------------------------------------------------------------*/
// 에러 표시
/*--------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------*/
// 초기화
// 서버, 콘솔 화면을 초기화 함
/*--------------------------------------------------------------------*/
BOOL InitServer()
{
	int retval;

	/*--------------------------------------------------------------------*/
	// 윈속 초기화
	/*--------------------------------------------------------------------*/
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))	return FALSE;

	/*--------------------------------------------------------------------*/
	// 소켓 생성
	/*--------------------------------------------------------------------*/
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)	err_display("listen_sock create error");

	/*--------------------------------------------------------------------*/
	// SOCKADDR_IN 셋팅과 바인드
	/*--------------------------------------------------------------------*/
	SOCKADDR_IN sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(3000);
	retval = bind(listen_sock, (SOCKADDR *)&sockaddr, sizeof(sockaddr));
	if (retval == SOCKET_ERROR)		err_display("binding error");

	IDkey = 1;
	cs_Initial();

	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
		g_Clients[iCnt].socket = INVALID_SOCKET;

	return TRUE;
}

/*--------------------------------------------------------------------*/
// 서버 On
/*--------------------------------------------------------------------*/
void OnServer()
{
	int retval;

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)		err_display("OnServer Error");
}

/*--------------------------------------------------------------------*/
// 네트워크 처리
/*--------------------------------------------------------------------*/
void Network()
{
	stPacket packet;
	int retval;

	FD_SET ReadSet;
	FD_ZERO(&ReadSet);
	FD_SET(listen_sock, &ReadSet);

	///////////////////////////////////////////////////////////////////
	// 연결중인 Client 등록
	///////////////////////////////////////////////////////////////////
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
		///////////////////////////////////////////////////////////////
		// Client 연결 요청
		///////////////////////////////////////////////////////////////
		if (FD_ISSET(listen_sock, &ReadSet))
		{
			SOCKET client_sock;
			SOCKADDR_IN sockaddr;
			int addrlen = sizeof(sockaddr);

			client_sock = accept(listen_sock, (SOCKADDR *)&sockaddr, &addrlen);
				
			if (client_sock != INVALID_SOCKET)
			{
				for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
				{
					if (g_Clients[iCnt].socket == INVALID_SOCKET){
						g_Clients[iCnt].socket = client_sock;
						g_Clients[iCnt].ID = IDkey;
						g_Clients[iCnt].x = 40;
						g_Clients[iCnt].y = 18;

						////////////////////////////////////////////////
						// ID생성 처리
						////////////////////////////////////////////////
						packet.type = 0;
						packet.ID = IDkey++;
						SendUnicast(g_Clients[iCnt].socket, packet, sizeof(packet));

						////////////////////////////////////////////////
						// 신규접속 처리
						////////////////////////////////////////////////
						packet.type = 1;
						packet.x = g_Clients[iCnt].x;
						packet.y = g_Clients[iCnt].y;
						SendUnicast(g_Clients[iCnt].socket, packet, sizeof(packet));
						SendBroadcast(packet, sizeof(packet));
						break;
					}
				}
			}			
		}

		for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
		{
			if (g_Clients[iCnt].socket != INVALID_SOCKET && FD_ISSET(g_Clients[iCnt].socket, &ReadSet))
			{
				////////////////////////////////////////////////
				// 이동 처리
				////////////////////////////////////////////////
				retval = recv(g_Clients[iCnt].socket, (char *)&packet, sizeof(packet), 0);

				if (retval == 0 || retval == SOCKET_ERROR)		ClientDisconnect(g_Clients[iCnt].ID);
				else{
					switch (packet.type)
					{
					case 3:
						if (g_Clients[iCnt].ID == packet.ID)
						{
							g_Clients[iCnt].x = packet.x;
							g_Clients[iCnt].y = packet.y;
							
							if (g_Clients[iCnt].y < 1)
								g_Clients[iCnt].y = 1;
							if (g_Clients[iCnt].x < 0)
								g_Clients[iCnt].x = 0;
							if (g_Clients[iCnt].x > dfSCREEN_WIDTH - 2)
								g_Clients[iCnt].x = dfSCREEN_WIDTH - 2;
							if (g_Clients[iCnt].y > dfSCREEN_HEIGHT - 1)
								g_Clients[iCnt].y = dfSCREEN_HEIGHT - 1;

							SendBroadcast(packet, sizeof(packet));
						}
						break;

					default:
						break;
					}
					break;
				}
			}
		}
	}
}

void Draw()
{
	char cCount = '0';

	Buffer_Clear();

	Sprite_Draw(0, 0, 'C');
	Sprite_Draw(1, 0, 'l');
	Sprite_Draw(2, 0, 'i');
	Sprite_Draw(3, 0, 'e');
	Sprite_Draw(4, 0, 'n');
	Sprite_Draw(5, 0, 't');
	Sprite_Draw(6, 0, ' ');
	Sprite_Draw(7, 0, 'C');
	Sprite_Draw(8, 0, 'o');
	Sprite_Draw(9, 0, 'n');
	Sprite_Draw(10, 0, 'n');
	Sprite_Draw(11, 0, 'e');
	Sprite_Draw(12, 0, 'c');
	Sprite_Draw(13, 0, 't');
	Sprite_Draw(14, 0, ' ');
	Sprite_Draw(15, 0, ':');
	Sprite_Draw(16, 0, ' ');

	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Clients[iCnt].socket != INVALID_SOCKET)
		{
			Sprite_Draw(g_Clients[iCnt].x, g_Clients[iCnt].y, '*');
			cCount++;
		}
	}

	Sprite_Draw(17, 0, cCount);
	Buffer_Flip();
}

/*--------------------------------------------------------------------*/
// 1 : 1 Send
/*--------------------------------------------------------------------*/
void SendUnicast(SOCKET sock, stPacket buf, int size)
{
	int retval;

	retval = send(sock, (char *)&buf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		ClientDisconnect(buf.ID);
	}
}

/*--------------------------------------------------------------------*/
// 1 : n Send
/*--------------------------------------------------------------------*/
void SendBroadcast(stPacket buf, int size)
{
	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Clients[iCnt].socket != INVALID_SOCKET)
		{
			SendUnicast(g_Clients[iCnt].socket, buf, size);
		}
	}
}

/*--------------------------------------------------------------------*/
// 접속 끊기
/*--------------------------------------------------------------------*/
void ClientDisconnect(DWORD ID)
{
	stPacket packet;

	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Clients[iCnt].ID == ID)
		{
			closesocket(g_Clients[iCnt].socket);
			g_Clients[iCnt].socket = INVALID_SOCKET;

			packet.type = 2;
			packet.ID = g_Clients[iCnt].ID;
			SendBroadcast(packet, sizeof(packet));
		}
	}
	
}