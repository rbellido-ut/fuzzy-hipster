#include "client_net.h"

using namespace std;

SOCKET createTCPClient(WSADATA* wsaData) {
	int res;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );
	SOCKET connectSocket;
	SOCKADDR_IN addr;
	hostent *hp;

	if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
	{
		cerr << "WSAStartup failed with error " << res << endl;
		WSACleanup();
		return NULL;
	}

	if ((connectSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		cerr << "Failed to get a socket with error " << WSAGetLastError() << endl;
		return NULL;
	}

	// Initialize and set up the address structure
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(TCPPORT);

	if ((hp = gethostbyname("localhost")) == NULL)
	{
		cerr << "Unknown server address" << endl;
		return NULL;
	}

	// Copy the server address
	memcpy((char *)&addr.sin_addr, hp->h_addr, hp->h_length);

	return connectSocket;
}