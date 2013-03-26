#ifndef UTIL_H
#define UTIL_H

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "ws2_32.lib")

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

class Server ;
class Client ;

enum {
    TCP,
    UDP
};

typedef struct socket_data{
    SOCKET sock;
    WSABUF	wsabuf;
    char databuf[DATABUFSIZE];
    WSAOVERLAPPED overlap;
}SOCKETDATA, *LPSOCKETDATA;

typedef struct request_contex{
    LPSOCKETDATA data;
    Client* clnt;
}REQUESTCONTEX;


#endif // UTIL_H
