/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		util.h -  This header file contains all of the utilities needed for the application.
--					Including constants, header includes, and defenition of a message struct.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:	
--
-- DATE:			March 4th, 2013
--
-- REVISIONS: 
--
-- DESIGNER:		Behnam Bastami
--
-- PROGRAMMER:		Behnam Bastami
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

#ifndef UTIL_H
#define UTIL_H
#include<WinSock2.h>
#include<Windows.h>

#include <limits.h>	
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <errno.h>
#include <sys/types.h>
#include <fstream>
#include <map>

#define DATABUFSIZE 102400

class Server;

typedef struct _sock_info {
	OVERLAPPED ov; //the overlapped structured used for async i/o
	SOCKET sock; //the socket used for the entire connection
	WSABUF DataBuf; 
	CHAR Buffer[DATABUFSIZE]; 
	DWORD BytesSEND;
	DWORD BytesRECV;
}SOCK_INFO, * LPSOCK_INFO;

typedef struct socket_data{
	SOCKET sock;
	WSABUF	wsabuf;
	char databuf[DATABUFSIZE];
	WSAOVERLAPPED overlap;
}SOCKETDATA, *LPSOCKETDATA;

typedef struct xx{
	WSAEVENT acceptEvent;
	Server* s;
};



#include "communication.h"
#include "client.h"
#include "server.h"

#endif
