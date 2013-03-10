/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		client.h -  This header file contains the constructor, copy constructor, destructor, getters/setters
--								function prototypes, and class members related to the client class.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		
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

#ifndef CLIENT_H
#define CLIENT_H

#include "util.h"

//The server class extends the Communication class
class Client : public Communication{
public:
	//Constructor calles parrent constructor for shared members
	explicit Client(const std::string& readBuffer = "",const std::string& writeBuffer = "",
		const int& bytesSent = 0, const int& bytesRecvd = 0, 
		const SOCKET& readSocket = 0, const SOCKET& writeSocket = 0)
		:Communication(readBuffer, writeBuffer, bytesSent, bytesRecvd, readSocket, writeSocket)
		//musicList_(musicList)
	{ count_++; }

	//Copy constructor calls parent copy constructor for shared members
	Client(const Client& src)
		:Communication(src), musicList_(src.musicList_)
	{ count_++; } 

	//Destructor
	~Client(){ count_--; }

	//Getter functions
	std::vector<int> getMusicList() { return musicList_; }
	static size_t getCount(){ return count_; }

	//Setter functions
	bool setMusicList(std::vector<int> ml);

	//Other function prototypes
	
	bool createTCPClient(WSADATA* wsaData);
	bool startTCPClient();

	bool createUDPClient(WSADATA* wsaData);
	bool startUDPClient();
	
	bool stopClient();
	bool requestConnect();
	bool requestDownload();
	bool requestUpload();
	bool requestStream();
	bool saveToFile();

	
	static LPSOCKETDATA allocData(SOCKET fd);
	static void freeData(LPSOCKETDATA data);
	static bool postSendRequest(LPSOCKETDATA data);
	static bool postRecvRequest(LPSOCKETDATA data);
	static void CALLBACK recvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
	static void CALLBACK sendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

	static DWORD WINAPI clientThread(LPVOID lpParameter);
	static void sendTCP(SOCKET& clntSock);



	//friend decleration of input and output operators
	friend std::ostream& operator<< (std::ostream& os, const Client& c);
	friend std::istream& operator>> (std::istream& is, Client& c);

private:
	//Data members
	std::vector<int> musicList_; //std::vector<music>? would have to create a music class
	static size_t count_;
	static SOCKET connectSocket_;
	struct hostent	*hp;
	char **pptr;
	SOCKADDR_IN addr;

	HANDLE threadHandle_;
	DWORD threadID_;
	static char  sbuf[255];
	
};

#endif
