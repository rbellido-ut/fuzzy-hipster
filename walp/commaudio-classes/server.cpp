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

bool Server::initTCPServer(WSADATA* wsaData, SOCKET* ListenSocket){
	int res;
	SOCKADDR_IN addr;

	if ((res = WSAStartup(0x0202,wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", res);
		WSACleanup();
		return false;
	}

	if ((*ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(TCPPORT);
	if (bind(*ListenSocket, (PSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return false;
	}
	
	return true;
}

//Other function prototypes
bool Server::startTCPServer(SOCKET* listenSocket){

	//WSAEVENT AcceptEvent;
	HANDLE hServeClntThread;
	HANDLE hStupidThread;
	DWORD servThreadId;
	DWORD stupidThreadId;
	xx* yy = (xx*) malloc (sizeof(xx*));

	if (listen(*listenSocket, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 0;
	}

	if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 0;
	}

	yy->acceptEvent = AcceptEvent;
	yy->s = this;
	// Create a worker thread to service completed I/O requests. 
	if ((hServeClntThread = CreateThread(NULL, 0, serveClientThread, (LPVOID) yy/*AcceptEvent*/, 0, &servThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return 0;
	}


	while(TRUE)
	{
		AcceptSocket = accept(*listenSocket, NULL, NULL);

		if (WSASetEvent(yy->acceptEvent) == FALSE)
		{
			printf("WSASetEvent failed with error %d\n", WSAGetLastError());
			return 0;
		}
	}

	free(yy);
	return true;
}


DWORD WINAPI serveClientThread(LPVOID lpParameter)
{

	DWORD Flags;
	LPSOCK_INFO SocketInfo;
	WSAEVENT EventArray[1];
	DWORD Index;
	DWORD RecvBytes;

	// Save the accept event in the event array.
	xx* zz  = (xx*) malloc (sizeof(xx*));
		zz = (xx*)lpParameter;
	
	EventArray[0] = (WSAEVENT) &zz->acceptEvent;

	while(TRUE)
	{
		// Wait for accept() to signal an event and also process WorkerRoutine() returns.

		while(TRUE)
		{
			Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

			if (Index == WSA_WAIT_FAILED)
			{
				printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
				return FALSE;
			}

			if (Index != WAIT_IO_COMPLETION)
			{
				// An accept() call event is ready - break the wait loop
				break;
			} 
		}

		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		// Create a socket information structure to associate with the accepted socket.


		if ((SocketInfo = (LPSOCK_INFO) GlobalAlloc(GPTR,
			sizeof(SOCK_INFO))) == NULL)
		{
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return FALSE;
		} 

		// Fill in the details of our accepted socket.

		SocketInfo->sock = zz->s->AcceptSocket;//AcceptSocket;
		ZeroMemory(&(SocketInfo->ov), sizeof(WSAOVERLAPPED));  
		SocketInfo->BytesSEND = 0;
		SocketInfo->BytesRECV = 0;
		SocketInfo->DataBuf.len = DATA_BUFSIZE;
		SocketInfo->DataBuf.buf = SocketInfo->Buffer;

		Flags = 0;
		if (WSARecv(SocketInfo->sock, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
			&(SocketInfo->ov), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return FALSE;
			}
		}

		printf("Socket %d connected\n", zz->s->AcceptSocket);//AcceptSocket);
	}

	free(zz);
	return true;

}

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{

	DWORD SendBytes, RecvBytes;
	DWORD Flags;

	// Reference the WSAOVERLAPPED structure as a SOCK_INFO structure
	LPSOCK_INFO SI = (LPSOCK_INFO) Overlapped;

	if (Error != 0)
	{
		printf("I/O operation failed with error %d\n", Error);
	}

	if (BytesTransferred == 0)
	{
		printf("Closing socket %d\n", SI->sock);
	}

	if (Error != 0 || BytesTransferred == 0)
	{
		closesocket(SI->sock);
		GlobalFree(SI);
		return;
	}

	// Check to see if the BytesRECV field equals zero. If this is so, then
	// this means a WSARecv call just completed so update the BytesRECV field
	// with the BytesTransferred value from the completed WSARecv() call.

	if (SI->BytesRECV == 0)
	{
		SI->BytesRECV = BytesTransferred;
		SI->BytesSEND = 0;
	}
	else
	{
		SI->BytesSEND += BytesTransferred;
	}

	if (SI->BytesRECV > SI->BytesSEND)
	{

		// Post another WSASend() request.
		// Since WSASend() is not gauranteed to send all of the bytes requested,
		// continue posting WSASend() calls until all received bytes are sent.

		ZeroMemory(&(SI->ov), sizeof(WSAOVERLAPPED));

		SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
		SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

		if (WSASend(SI->sock, &(SI->DataBuf), 1, &SendBytes, 0,
			&(SI->ov), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSASend() failed with error %d\n", WSAGetLastError());
				return;
			}
		}
	}
	else
	{
		SI->BytesRECV = 0;

		// Now that there are no more bytes to send post another WSARecv() request.

		Flags = 0;
		ZeroMemory(&(SI->ov), sizeof(WSAOVERLAPPED));

		SI->DataBuf.len = DATA_BUFSIZE;
		SI->DataBuf.buf = SI->Buffer;

		if (WSARecv(SI->sock, &(SI->DataBuf), 1, &RecvBytes, &Flags,
			&(SI->ov), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING )
			{
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return;
			}
		}
	}

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
