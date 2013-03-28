#ifndef __SERVER_H
#define __SERVER_H

#include "server_net.h"

class Server {

public:
	// CONSTRUCTOR
	Server()
	{
		listenSocket_ = 0;
	}

	// DESTRUCTOR
	~Server()
	{
	}

	void runServer(WSADATA * wsadata)
	{
		HANDLE acceptClientThreadHandle;
		DWORD acceptClientThreadID;

		listenSocket_ = createServer(wsadata, TCP);
		acceptClientThreadHandle = CreateThread(NULL, 0, runListenThread, this, 0, &acceptClientThreadID);
	}

private:
	// Private data members
	SOCKET listenSocket_;
	std::vector<SOCKET> clientList;

	// Function prototypes
	DWORD WINAPI listenThread(void *param);
	DWORD WINAPI handleClient(void *param);
	static DWORD WINAPI runListenThread(LPVOID args);
	static DWORD WINAPI runHandleClientThread(LPVOID args);
};

#endif
