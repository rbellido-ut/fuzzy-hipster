#ifndef SERVER_H
#define SERVER_H

#include "util.h"

class Client;
class Communication;

//The server class extends the Communication class
class Server:public Communication {
public:

	//Constructor calls parrent's constructor
	explicit Server(const std::string& readBuffer = "",const std::string& writeBuffer = "",
		const int& bytesSent = 0, const int& bytesRecvd = 0, 
		const SOCKET& readSocket = 0, const SOCKET& writeSocket = 0, const SOCKET& listenSocket = 0)
		//const std::vector<int>& musicList = 0, const std::vector<Client>& clientList)
		:Communication(readBuffer, writeBuffer, bytesSent, bytesRecvd, readSocket, writeSocket)
		//musicList_(musicList), clientList_(clientList)
	{
		
		count_++; 
	}

	//Copy constructor, calls parrent's copy constructor
	Server(const Server& src)
		: Communication(src)//, musicList_(src.musicList_), clientList_(src.clientList_)
	{ count_++; } 

	//Destructor
	~Server(){ count_--; }

	//Getter functions
	SOCKET getacceptSocket(){ return acceptSocket; }
	//std::vector<int> getMusicList() { return musicList_; }
	//std::vector<Client> getClientList() { return clientList_; }
	static size_t getCount(){ return count_; }

	//Setter functions
	//bool setMusicList(std::vector<int> ml);
	//bool setClientList(std::vector<Client> cl);

	//Other function prototypes
	bool createTCPServer(WSADATA* wsaData);
	bool startTCPServer();

	bool stopServer();
	bool acceptConnect();
	bool acceptDownload();
	bool acceptUpload();
	bool acceptStream();
	bool saveToFile();
	//bool addToMusicList();
	//bool addToClientList(Client c);

	//SOCKET createListenTCPSocket(const std::string& strHost, const int& TCPPort);
	static LPSOCKETDATA allocData(SOCKET fd);
	static void freeData(LPSOCKETDATA data);
	static bool postRecvRequest(LPSOCKETDATA data);
	static bool postSendRequest(LPSOCKETDATA data);
	static void CALLBACK recvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
	static void CALLBACK sendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
	
	void DecodeRequest(int requesttype);
	void startStream(int songindex);
	void endStream();

private:
	//std::vector<int> musicList_;
	//std::vector<Client> clientList_;
	
	int Ret;

	WSADATA wsaData;
	SOCKET listenSocket;
	SOCKET acceptSocket;
	static size_t count_;
	static std::map<int, LPSOCKETDATA> mSocketList_;
	
	bool isStreaming_;
};

#endif
