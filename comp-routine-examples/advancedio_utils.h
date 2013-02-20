#ifndef ADVANCEDIO_UTILS_H
#define ADVANCEDIO_UTILS_H

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <Shlwapi.h>
#include <sstream>
#include <fstream>
#include "resource.h"

#define IS_UDPSERVER 1337
#define IS_UDPCLIENT 1338
#define IS_TCPSERVER 1339
#define IS_TCPCLIENT 1340

#define WM_HANDLESERVER 1341
#define WM_HANDLECLIENT 1342

#define DATA_BUFSIZE 2048

typedef struct _SOCKET_INFORMATION
{
	OVERLAPPED Overlapped;
	BOOL RecvPosted;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSent;
	DWORD BytesRecvd;
	_SOCKET_INFORMATION *Next;
	HWND hDlg;
	char *filename;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

typedef struct EXTRA_DATA
{
	SOCKET connectsocket;
	SOCKET acceptsocket;
	SOCKET listensocket;
	SOCKADDR_IN addrin;
	WSAEVENT wsaEvent;
	HWND hDlg;
	char *filename;
} EXTRA_DATA, *LPEXTRA_DATA;

void ErrorMsg(const char* err_msg, HWND hwnd);

void InitializeDialogBox(int whatisthis, HWND hDlg);

OPENFILENAME GetFileToSendFromDialog(HWND);

void SetFileToSend(LPOPENFILENAME lpofn, LPSOCKET_INFORMATION lpSI);

#endif