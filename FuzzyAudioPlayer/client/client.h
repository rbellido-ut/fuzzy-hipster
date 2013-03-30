#include "utils.h"

class Client {

public:
	Client()
	{ 
		currentState = NOTCONNECTED;
		connectSocket_ = 0; 
	}
	~Client(){ }

	bool runClient(WSADATA *wsadata, const char*, const int);
	

	int currentState;
	DWORD dlThreadID;
	HANDLE dlThreadHandle;

	DWORD ulThreadID;
	DWORD Client::dlThread(LPVOID param);
	static DWORD WINAPI Client::runDLThread(LPVOID param);

	HANDLE ulThreadHandle;
	DWORD Client::ulThread(LPVOID param);
	static DWORD WINAPI Client::runULThread(LPVOID param);

	void dispatchOneSend(std::string dlReq);
	void Client::dispatchOneRecv();

private:
	SOCKET connectSocket_;
	SOCKADDR_IN addr_;
	hostent *hp_;

	SOCKET createTCPClient(WSADATA*, const char*, const int);

	static DWORD WINAPI runRecvThread(LPVOID param);
	DWORD WINAPI Client::recvThread(/*LPVOID param*/);

	LPSOCKETDATA allocData(SOCKET fd);
    void freeData(LPSOCKETDATA data);

	bool dispatchWSASendRequest(LPSOCKETDATA data);
    bool dispatchWSARecvRequest(LPSOCKETDATA data);
	void recvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    void sendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runRecvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runSendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

};