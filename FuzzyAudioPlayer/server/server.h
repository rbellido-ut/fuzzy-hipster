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

	void runServer(WSADATA * wsadata);

private:
	// Private data members
	SOCKET listenSocket_;
	std::vector<SOCKET> clientList_;
	int currentClient;

	// Function prototypes
	DWORD WINAPI listenThread(void *param);
	DWORD WINAPI handleClient(void *param);
	static DWORD WINAPI runListenThread(LPVOID args);
	static DWORD WINAPI runHandleClientThread(LPVOID args);
	//DWORD WINAPI StreamThread(LPVOID lpParameter);
	void ParseRequest(char * request);
};

#endif
