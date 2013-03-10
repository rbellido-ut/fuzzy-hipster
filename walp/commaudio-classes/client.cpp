/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		client.cpp -  This file contains function implementations related to the client class.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		bool Client::setMusicList(std::vector<int> ml)
--					bool Client::requestConnect()
--					bool Client::requestDownload()
--					bool Client::requestUpload()
--					bool Client::requestStream()
--					bool Client::saveToFile()
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
size_t Client::count_ = 0;
SOCKET Client::connectSocket_;
char  Client::sbuf[255];

//Setter functions
bool Client::setMusicList(vector<int> ml){

	return true;
}

//Other function prototypes

bool Client::createTCPClient(WSADATA* wsaData){
	int res;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );

	if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
	{
		cerr << "WSAStartup falied with error " << res << endl;
		WSACleanup();
		return false;
	}

	if ((connectSocket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		cerr << "Failed to get a socket with error " << WSAGetLastError() << endl;
		exit(1);
	}

	// Initialize and set up the address structure
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(TCPPORT);

	if ((hp = gethostbyname("localhost")) == NULL)
	{
		cerr << "Unknown server address" << endl;
		exit(1);
	}

	// Copy the server address
	memcpy((char *)&addr.sin_addr, hp->h_addr, hp->h_length);

	return true;
}

bool Client::startTCPClient(){

	// Connecting to the server
	if (WSAConnect (connectSocket_, (struct sockaddr *)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == INVALID_SOCKET)
	{
		cerr << "Can't connect to server" << endl;
		cerr << "connect()" << endl;
		return false;
	}
	cout << "Connected:    Server Name: " << hp->h_name << endl;
	pptr = hp->h_addr_list;
	cout << "\t\tIP Address: " <<  inet_ntoa(addr.sin_addr) << endl;

	cout << "Server started, connected to socket " << connectSocket_ << endl;
	
	//postRecvRequest();

	while(TRUE)
	{
		
		SOCKETDATA* data = allocData(connectSocket_);
		if(data)
		{
			//thread??
			postSendRequest(data);
		}
		
		::SleepEx(100, TRUE); //make this thread alertable
	}

	

	//threadHandle_ = CreateThread(NULL, 0, clientThread, NULL, 0, &threadID_);
	
	return true;
}


bool Client::postRecvRequest(LPSOCKETDATA data){
	DWORD flag = 0;
	DWORD bytesRecvd = 0;
			
	//setBytesRecvd(0);
	int error;
	
	if(data)
	{
		//thread??
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

}

bool Client::postSendRequest(LPSOCKETDATA data)
{
	DWORD flag = 0;
	DWORD bytesSent = 0;
	DWORD bytesRecvd = 0;
	int error;
	
	gets(data->databuf);

	error = WSASend(data->sock, &data->wsabuf, 1, &bytesSent, flag, &data->overlap, sendComplete);
	if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
	{
		return true;
	}
	else
	{
		cerr << "WSASend() failed on " << data->sock << endl;
		freeData(data);
		return false;
	}

}

void CALLBACK Client::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	LPSOCKETDATA data = (LPSOCKETDATA) overlapped->hEvent;

	if(error || bytesTransferred == 0)
	{
		freeData(data);
		return;
	}

	cout << data->databuf << endl;

}

void CALLBACK Client::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	LPSOCKETDATA data = (LPSOCKETDATA)overlapped->hEvent;
	if(error || bytesTransferred == 0)
	{
		freeData(data);
		return;
	}
	
	postRecvRequest(data);
}


LPSOCKETDATA Client::allocData(SOCKET socketFD)
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


	return data;
}

void Client::freeData(LPSOCKETDATA data){
	if(data)
	{
		cout << "Socket " << data->sock <<" Closed." << endl;
		closesocket(data->sock);
		delete data;
	}
}

void Client::sendTCP(SOCKET& clntSock){

	Sleep(10000);
}


DWORD WINAPI Client::clientThread(LPVOID lpParameter){

	while(TRUE)
	{
		SOCKETDATA* data = allocData(connectSocket_);
		if(data)
		{
			//thread??
			postRecvRequest(data);
		}
		
		::SleepEx(100, TRUE); //make this thread alertable
	}
}

bool Client::createUDPClient(WSADATA* wsaData){

	return true;
}

bool Client::startUDPClient(){

	return true;
}


bool Client::stopClient(){

	return true;
}

bool Client::requestConnect(){

	return true;
}

bool Client::requestDownload(){

	return true;
}

bool Client::requestUpload(){

	return true;
}

bool Client::requestStream(){

	return true;
}

bool Client::saveToFile(){

	return true;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	operator<<
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	ostream& operator<< (ostream& os, const Client& c)
--				os: An output stream
--				c: The Client object that needs to be outputed
--
-- RETURNS:		Returns an output stream containing the client's data members
--
-- NOTES:		
----------------------------------------------------------------------------------------------------------------------*/
ostream& operator<< (ostream& os, const Client& c){

	return os;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	operator>>
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	istream& operator>> (istream& is, Client& c)
--				is: An iutput stream
--				c: The Client object to be read
--
-- RETURNS:		Returns the state of the input stream after the client object was read.
--				If a client object was read successful the return value evaluates to true, and false if it fails.
--
-- NOTES:		
----------------------------------------------------------------------------------------------------------------------*/
istream& operator>> (istream& is, Client& c){

	
	return is;

}
