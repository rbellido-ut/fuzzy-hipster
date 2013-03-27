#include "utils.h"

class Client {

public:
	Client()
	{ 
		currentState = NOTCONNECTED;
		connectSocket_ = 0; 
	}
	~Client(){ }

	bool runClient(WSADATA *wsadata);
	

	int currentState;
	DWORD dlThreadID;
	HANDLE dlThreadHandle;

	DWORD Client::dlThread(/*LPVOID param*/);
	static DWORD WINAPI Client::runDLThread(LPVOID param);
	void sendDLRequest(std::string dlReq);

private:
	SOCKET connectSocket_;
	SOCKADDR_IN addr_;
	hostent *hp_;

	SOCKET createTCPClient(WSADATA* wsaData);

	static DWORD WINAPI runRecvThread(LPVOID param);
	DWORD WINAPI Client::recvThread(/*LPVOID param*/);

	LPSOCKETDATA allocData(SOCKET fd);
    void freeData(LPSOCKETDATA data);

	bool postSendRequest(LPSOCKETDATA data);
    bool postRecvRequest(LPSOCKETDATA data);
	void recvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    void sendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runRecvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runSendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

};