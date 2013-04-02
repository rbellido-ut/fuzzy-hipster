#include "server_net.h"

using namespace std;

/*
* Creates a server. Returns the listening socket.
*/
SOCKET createServer(WSADATA *wsaData, int protocol, SOCKADDR_IN * udpaddr)
{
	int res;
	SOCKADDR_IN addr;
	SOCKET listenSocket;

	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );

	if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
	{
		cerr << "WSAStartup failed with error " << res << endl;
		WSACleanup();
		return NULL;
	}

    if ((listenSocket = WSASocket(AF_INET, (protocol == TCP) ? SOCK_STREAM : SOCK_DGRAM , 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		cerr << "Failed to get a socket with error " << WSAGetLastError() << endl;
		return NULL;
	}

    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    addr.sin_port           = htons((protocol == TCP) ? TCPPORT : UDPPORT);

	if (bind(listenSocket, (PSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		cerr << "bind() failed with error " << WSAGetLastError() << endl;
		return NULL;
	}

	cout << "Server bound to port " << ((protocol == TCP) ? TCPPORT : UDPPORT) << endl;

    if (protocol != UDP)
    {
        if (listen(listenSocket, 5))
        {
            cerr << "listen() failed with error " << WSAGetLastError() << endl;
            closesocket(listenSocket);
            return NULL;
        }
    }
	else
	{
		*udpaddr = addr;
	}

	return listenSocket;
}