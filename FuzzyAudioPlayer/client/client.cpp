#include "client.h"

using namespace std;

SOCKET Client::createTCPClient(WSADATA* wsaData) {
	int res;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );
	SOCKET connectSocket;

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
	memset((char *)&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(TCPPORT); //TODO: this is hard coded

	if ((hp_ = gethostbyname("localhost")) == NULL) //TODO: this is hard-coded
	{
		cerr << "Unknown server address" << endl;
		return NULL;
	}

	// Copy the server address
	memcpy((char *)&addr_.sin_addr, hp_->h_addr, hp_->h_length);

	return connectSocket;
}