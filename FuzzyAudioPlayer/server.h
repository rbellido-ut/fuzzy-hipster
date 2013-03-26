#include "server_net.h"

class Server {

public:
	Server(){
		listenSocket_ = 0;
	}

	~Server(){}

	void runServer(WSADATA * wsadata) {
		HANDLE acceptClientThreadHandle;
		DWORD acceptClientThreadID;
		listenSocket_ = createServer(wsadata, TCP);
		acceptClientThreadHandle = CreateThread(NULL, 0, runListenThread, this, 0, &acceptClientThreadID);
	}

private:
	SOCKET listenSocket_;
	std::vector<SOCKET> clientList;
	DWORD WINAPI listenThread(void *param);
	DWORD WINAPI handleClient(void *param);
	static DWORD WINAPI runListenThread(LPVOID args);
	static DWORD WINAPI runHandleClientThread(LPVOID args);
};