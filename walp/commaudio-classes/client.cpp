#include "util.h"

using namespace std;

size_t Client::count_ = 0;
SOCKET Client::connectSocket_;
char  Client::sbuf[255];

//Setter functions
bool Client::setMusicList(vector<int> ml){

	return true;
}

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
		return false;
	}

	// Initialize and set up the address structure
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(TCPPORT);

	if ((hp = gethostbyname("localhost")) == NULL)
	{
		cerr << "Unknown server address" << endl;
		return false;
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



	threadHandle_ = CreateThread(NULL, 0, clientThread, NULL, 0, &threadID_);

	while(TRUE)
	{
		string command;
		SOCKETDATA* data = allocData(connectSocket_);
	
		cout << "Enter your command: ";
		getline(cin, command);
		
		if(command == "download")
		{
			data->typeOfRequest = 1;
			//send download request
		}
		if(command == "upload")
		{
			data->typeOfRequest = 2;
			//send upload request
		}
		if(command == "stream")
		{
			data->typeOfRequest = 3;
			//send stream request
		}
		if(command == "multicast")
		{
			data->typeOfRequest = 4;
			//send multicast request
		}
		if(command == "mic")
		{
			cout << "Mic" << endl;
			data->typeOfRequest = 5;
			//send mic request
		}


		strcpy(data->databuf,command.c_str());
		
		if(data)
		{
			postSendRequest(data);
		}


		::SleepEx(100, TRUE); //make this thread alertable
	}
	
	return true;
}

bool Client::postRecvRequest(LPSOCKETDATA data){
	DWORD flag = 0;
	DWORD bytesRecvd = 0;
	int error;
	
	if(data)
	{
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
	int error;
	
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

	cout << "recvComplete(): " << data->databuf << endl;

}

void CALLBACK Client::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	LPSOCKETDATA data = (LPSOCKETDATA)overlapped->hEvent;
	if(error || bytesTransferred == 0)
	{
		freeData(data);
		return;
	}

	switch(data->typeOfRequest)
	{
	case 1:
		cout << "Client handling DL" << endl;
		//do whatever you want to do after a download request was sent successfully (save to file)
		break;
	case 2:
		cout << "Client handling UL" << endl;
		//do whatever you want to do after a upload request was sent successfully (wait for server to send u a confirmation)
		break;
	case 3:
		cout << "Client handling Stream" << endl;
		//do whatever you want to do after a stream request was sent successfully (wait for approval)
		break;
	case 4:
		cout << "Client handling Multicast" << endl;
		//do whatever you want to do after a multicast request was sent successfully ( ... )
		break;
	case 5:
		cout << "Client handling Mic" << endl;
		//do whatever you want to do after a mic request was sent successfully ( ... )
		break;
	}
	
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Client::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	LPSOCKETDATA Client::allocData(SOCKET socketFD)
--				socketFD: handle to the socket to be copied into the data structure
--
-- RETURNS:		Pointer to the allocated memory location for the data structure
--
-- NOTES:		This function allocates memory for the data structure used to hold the buffers 
--				and the communication socket
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Client::freeData(LPSOCKETDATA data)
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void Client::freeData(LPSOCKETDATA data)
--
-- RETURNS:		
--
-- NOTES:		This function frees the allocated memory for the data structure
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Client::clientThread(LPVOID lpParameter)
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	DWORD WINAPI Client::clientThread(LPVOID lpParameter)
--				lpParameters: A pointer to the parameters passed to the thread proc
-- RETURNS:		
--
-- NOTES:		This is the thread proc used to post overlapped WSARecv calls
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI Client::clientThread(LPVOID lpParameter){

	while(TRUE)
	{
		SOCKETDATA* data = allocData(connectSocket_);
		if(data)
		{
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

ostream& operator<< (ostream& os, const Client& c){

	return os;
}

istream& operator>> (istream& is, Client& c){

	
	return is;

}
