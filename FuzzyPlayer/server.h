#ifndef SERVER_H
#define SERVER_H

#include "communication.h"
#include "util.h"

class Client;
class Communication;

//The server class extends the Communication class
class Server:public Communication
{
public:

	//Constructor calls parrent's constructor
    explicit Server(const std::string& readBuffer = "", const std::string& writeBuffer = "",
		const int& bytesSent = 0, const int& bytesRecvd = 0, 
		const SOCKET& readSocket = 0, const SOCKET& writeSocket = 0, const SOCKET& listenSocket = 0)
        :Communication(readBuffer, writeBuffer, bytesSent, bytesRecvd, readSocket, writeSocket)
	{
	}

	//Copy constructor, calls parrent's copy constructor
	Server(const Server& src)
        : Communication(src)
    {
    }

	//Destructor
    ~Server()
    {
    }

	//Getter functions

	//Setter functions

	//Other function prototypes
    bool createServer(WSADATA* wsaData, int protocol);
    bool startServer();
	//SOCKET createListenTCPSocket(const std::string& strHost, const int& TCPPort);

    static LPSOCKETDATA allocData(SOCKET fd);
	static void freeData(LPSOCKETDATA data);

    static bool postRecvRequest(LPSOCKETDATA data);
	static bool postSendRequest(LPSOCKETDATA data);
    static void CALLBACK recvComplete(DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK sendComplete(DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
	
	void DecodeRequest(int requesttype);
	void startStream(int songindex);
	void endStream();

private:
	int Ret;
	WSADATA wsaData;
	SOCKET listenSocket;
	SOCKET acceptSocket;
	static std::map<int, LPSOCKETDATA> mSocketList_;

    int protocolType_;
	bool isStreaming_;
    static unsigned long WINAPI serverWorkThread(void *);
};

#endif
