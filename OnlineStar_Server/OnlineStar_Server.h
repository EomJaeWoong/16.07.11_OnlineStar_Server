#ifndef __ONLINESTAR_SERVER__H__
#define __ONLINESTAR_SERVER__H__

#define USER_MAX 64

typedef struct st_Client
{
	SOCKET socket;
	int ID;
	int x;
	int y;
} stClient;

typedef struct st_Packet
{
	int type;
	int ID;
	int x;
	int y;
} stPacket;
static DWORD IDkey;

BOOL InitServer();
void OnServer();
void Network();
void Draw();

void SendUnicast();
void SendBroadcast();
#endif