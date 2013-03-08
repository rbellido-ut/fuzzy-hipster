/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		server.h -  This header file contains the constructor, copy constructor, destructor, getters/setters
--								function prototypes, and class members related to the server class.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		explicit Server(const pid_t& pid = 0, const int& ipcID = 0);
--					Server(const Server& src);
--					~Server();
--					static size_t getCount();
--					void initServer();
--					void listen();
--					bool sendFile(std::string file, pid_t clientPID, int clientPriority);
--
-- DATE:			March 4th, 2013
--
-- REVISIONS: 
--
-- DESIGNER:		Behnam Bastami
--
-- PROGRAMMER:		Behnam Bastami
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

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
		const SOCKET& readSocket = 0, const SOCKET& writeSocket = 0)
		//const std::vector<int>& musicList = 0, const std::vector<Client>& clientList)
		:Communication(readBuffer, writeBuffer, bytesSent, bytesRecvd, readSocket, writeSocket)
		//musicList_(musicList), clientList_(clientList)
	{
		TCPPORT= 5150;
		UDPPORT = 6000;
		count_++; 
	}

	//Copy constructor, calls parrent's copy constructor
	Server(const Server& src)
		: Communication(src)//, musicList_(src.musicList_), clientList_(src.clientList_)
	{ count_++; } 

	//Destructor
	~Server(){ count_--; }

	//Getter functions
	SOCKET getAcceptSocket(){ return AcceptSocket; }
	//std::vector<int> getMusicList() { return musicList_; }
	//std::vector<Client> getClientList() { return clientList_; }
	static size_t getCount(){ return count_; }

	//Setter functions
	//bool setMusicList(std::vector<int> ml);
	//bool setClientList(std::vector<Client> cl);


	//Other function prototypes
	bool initTCPServer(WSADATA* wsaData, SOCKET* ListenSocket);
	bool startTCPServer(SOCKET* listenSocket);
	bool stopServer();
	bool acceptConnect();
	bool acceptDownload();
	bool acceptUpload();
	bool acceptStream();
	bool saveToFile();
	//bool addToMusicList();
	//bool addToClientList(Client c);
	friend DWORD WINAPI serveClientThread(LPVOID lpParameter);
	friend void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

private:
	//std::vector<int> musicList_;
	//std::vector<Client> clientList_;
	//SOCKET ListenSocket, AcceptSocket;
	WSADATA wsaData;
	SOCKET AcceptSocket;
	SOCKADDR_IN server;
	INT Ret;
	HANDLE ThreadHandle;
	DWORD ThreadId;
	WSAEVENT AcceptEvent;

	int TCPPORT, UDPPORT;

	static size_t count_;

};

#endif
