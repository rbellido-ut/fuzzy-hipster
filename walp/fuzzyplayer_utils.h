#ifndef FUZZYPLAYERUTILS_H
#define FUZZYPLAYERUTILS_H

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <fstream>
#include <sstream>

#define SERVER_TCP_PORT 7000

typedef struct _SOCKET_INFORMATION 
{
	OVERLAPPED Overlapped;
	SOCKET acceptsocket;
	std::string buffer;
	WSABUF DataBuf;
	DWORD BytesSend;
	DWORD BytesRecv;
	std::string filename;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

#endif