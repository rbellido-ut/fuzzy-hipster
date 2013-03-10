/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		server.cpp -  This file contains function implementations related to the server class.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		bool setMusicList(std::vector<int> ml)
--					bool setClientList(std::vector<Client> cl)
--					bool addToMusicList()
--					bool acceptConnect()
--					bool acceptDownload()
--					bool acceptUpload()
--					bool acceptStream()
--					bool saveToFile()
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

#include "util.h"
using namespace std;
size_t Server::count_ = 0;
std::map<int, LPSOCKETDATA> Server::mSocketList_;

extern Server sv;

//Setter functions
/*
bool setMusicList(std::vector<int> ml){

return true;
}

bool setClientList(std::vector<Client> cl){

return true;
}

bool Server::addToMusicList(){

return true;
}

bool Server::addToClientList(Client c){

return true;
}
*/


bool Server::createTCPServer(WSADATA* wsaData){
	int res;
	SOCKADDR_IN addr;

	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );

	if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
	{
		cerr << "WSAStartup falied with error " << res << endl;
		WSACleanup();
		return false;
	}

	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		cerr << "Failed to get a socket with error " << WSAGetLastError() << endl;
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(TCPPORT);
	if (bind(listenSocket, (PSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		cerr << "bind() falied with error " << WSAGetLastError() << endl;
		return false;
	}

	if (listen(listenSocket, 5))
	{
		cerr << "listen() falied with error " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		return 0;
	}

	ULONG nonblock = 1;
	if(ioctlsocket(listenSocket, FIONBIO, &nonblock) == SOCKET_ERROR)
	{
		cerr << "ioctlsocket() falied with error " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		return 0;
	}
	
	return true;
}

//Other function prototypes
bool Server::startTCPServer(){

	cout << "Server started, listening on socket " << listenSocket << endl;
	while(TRUE)
	{
		SOCKADDR_IN addr = {};
		int addrLen = sizeof(addr);

 		SOCKET newSock = WSAAccept(listenSocket, (sockaddr*)&addr, &addrLen, NULL, NULL);
		if(newSock == INVALID_SOCKET)
		{
			if(WSAGetLastError() != WSAEWOULDBLOCK)
			{
				cerr << "accept() failed with error " << WSAGetLastError() << endl;
				break;
			}
		}
		else {
			SOCKETDATA* data = allocData(newSock);
			cout << "Socket " << newSock << " accepted." << endl;
			if(data)
			{
				//thread??
				postRecvRequest(data);
			}
		}


		::SleepEx(100, TRUE); //make this thread alertable
	}

	return true;
}

bool Server::stopServer(){

	return true;
}

bool Server::acceptConnect(){

	return true;
}

bool Server::acceptDownload(){

	return true;
}

bool Server::acceptUpload(){

	return true;
}

bool Server::acceptStream(){

	return true;
}

bool Server::saveToFile(){

	return true;
}


LPSOCKETDATA Server::allocData(SOCKET socketFD)
{
	LPSOCKETDATA data = NULL;

	try{
		data = new SOCKETDATA();
	
	}catch(std::bad_alloc&){
		cerr << "Allocate socket data failed" << endl;
		return NULL;
	}

	data->overlap.hEvent = (WSAEVENT)data;
	data->sock = socketFD;
	data->wsabuf.buf = data->databuf;
	data->wsabuf.len = sizeof(data->databuf);

	mSocketList_[socketFD] = data;

	return data;
}

void Server::freeData(LPSOCKETDATA data){
	if(data)
	{
		cout << "Socket " << data->sock <<" Closed." << endl;
		closesocket(data->sock);
		mSocketList_.erase(data->sock);
		delete data;
	}
}

bool Server::postRecvRequest(LPSOCKETDATA data)
{
	DWORD flag = 0;
	DWORD bytesRecvd = 0;
	//setBytesRecvd(0);
	int error;

	error = WSARecv(data->sock, &data->wsabuf, 1, &bytesRecvd, &flag, &data->overlap, recvComplete);
	if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
	{
		return true;
	}
	else
	{
		cerr << "WSARecv() failed on " << data->sock << endl;
		freeData(data);
		return false;
	}

}
void CALLBACK Server::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	LPSOCKETDATA data = (LPSOCKETDATA) overlapped->hEvent;

	if(error || bytesTransferred == 0)
	{
		freeData(data);
		return;
	}

	//Send Data Back
	memset(&data->overlap, 0, sizeof(data->overlap));
	data->overlap.hEvent = (WSAEVENT)data;
	data->wsabuf.len = bytesTransferred;
	DWORD bytesSent = 0;
	cout << "Received: " << data->databuf << ". From: " << data->sock << endl;
	error = WSASend(data->sock, &data->wsabuf, 1, &bytesSent, flags, &data->overlap, sendComplete);
	if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
	{
		//success
		return;
	}
	else
	{
		freeData(data);
	}

}

void CALLBACK Server::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	LPSOCKETDATA data = (LPSOCKETDATA)overlapped->hEvent;
	if(error || bytesTransferred == 0)
	{
		freeData(data);
		return;
	}

	//post another WSARecv()
	data->wsabuf.len = sizeof(data->databuf);
	postRecvRequest(data);
}
