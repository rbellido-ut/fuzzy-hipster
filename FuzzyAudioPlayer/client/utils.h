#ifndef UTIL_H
#define UTIL_H

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libzplay.lib")

#include <WinSock2.h>
#include <Windows.h>

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
#define TCPPORT 5555
#define UDPPORT 4444

class Server ;
class Client ;

enum {
    TCP,
    UDP
};

//client states
enum {
	NOTCONNECTED,
    WFUCOMMAND, //Waiting for User's command
    SENTDLREQUEST, //have sent the DL request
    WAITFORDOWNLOAD,
	DOWNLOADING, //waiting for download data (after a download request was sent successfuly)
    SENTULREQUEST, //send an upload request
    WAITFORUPLOAD, //waiting for Server's approval (after a upload request was sent successfuly)
    UPLOADING,
    STREAMING, //streaming data from server
    L2MULTICAST, //listening to multicast channel
    MICROPHONE, //in mic mode
	SENTSTREQUEST,
	WAITFORSTREAM,
	SENTLISTREQUEST, 
	WAITFORLIST // get song list from server
};

typedef struct socket_data{
    SOCKET sock;
    WSABUF	wsabuf;
    char databuf[DATABUFSIZE];
    WSAOVERLAPPED overlap;
}SOCKETDATA, *LPSOCKETDATA;

typedef struct request_context{
    LPSOCKETDATA data;
    Client* clnt;
} REQUESTCONTEXT;

typedef struct list_context {
	Client* clnt;
	HWND* hwnd;
} LISTCONTEXT;

typedef struct upload_context {
	Client* clnt;
	LPSTR filename;
} UPLOADCONTEXT;



bool populateSongList(HWND*, std::string);

#endif // UTIL_H
