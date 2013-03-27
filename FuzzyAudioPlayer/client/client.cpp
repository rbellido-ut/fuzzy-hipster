#include "client.h"

using namespace std;

SOCKET Client::createTCPClient(WSADATA* wsaData) 
{

	int res;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );
	SOCKET connectSocket;

	if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
	{
		//cerr << "WSAStartup failed with error " << res << endl;
		WSACleanup();
		return NULL;
	}

	if ((connectSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		//cerr << "Failed to get a socket with error " << WSAGetLastError() << endl;
		return NULL;
	}

	// Initialize and set up the address structure
	memset((char *)&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(TCPPORT); //TODO: this is hard coded

	if ((hp_ = gethostbyname("localhost")) == NULL) //TODO: this is hard-coded
	{
		MessageBox(NULL, "Unknown server address", "Connection Error", MB_ICONERROR);
		return NULL;
	}

	// Copy the server address
	memcpy((char *)&addr_.sin_addr, hp_->h_addr, hp_->h_length);

	return connectSocket;

}

bool Client::runClient(WSADATA *wsadata) 
{

	char **pptr;

	connectSocket_ = createTCPClient(wsadata);

	if (WSAConnect (connectSocket_, (struct sockaddr *)&addr_, sizeof(addr_), NULL, NULL, NULL, NULL) == INVALID_SOCKET)
	{
		MessageBox(NULL, "Can't connect to server", "Connection Error", MB_ICONERROR);
		return false;
	}

	MessageBox(NULL, "IM CONNECTED! YEAAAAAA !", "Connected to server", MB_ICONINFORMATION);
	currentState = WFUCOMMAND;

	return true;

	//now that we are connected , we are going to wait for user command



	//cout << "Connected:    Server Name: " << hp_->h_name << std::endl;
	//pptr = hp_->h_addr_list;
	//cout << "\t\tIP Address: " <<  inet_ntoa(addr_.sin_addr) << std::endl;
	//cout << "Server started, connected to socket " << connectSocket_ << std::endl;
	
	//DWORD recvThreadID;
	//HANDLE recvThreadHandle;
	//recvThreadHandle = CreateThread(NULL, 0, runRecvThread, this, 0, &recvThreadID);



	/*
	//while(TRUE)
	//{
		//string command, userRequest, songName;
		//SOCKETDATA* data = allocData(connectSocket_);

		//cout << "Enter your command: ";
		//getline(cin, command);

		if(command == "download")
		{	//send download request
			userRequest = "DL ";
			songName = "Behnam's Party Mix\n";
			userRequest += songName;
		}
		if(command == "upload")
		{
			//send upload request
			userRequest = "UL ";
			songName = "Behnam's Party Mix\n";
			userRequest += songName;
		}
		if(command == "stream")
		{
			//send stream request
		}
		if(command == "multicast")
		{
			//send multicast request
		}
		if(command == "mic")
		{
			//send mic request
		}

		//cout << "Sent " << userRequest << endl;

		//strcpy(data->databuf,command.c_str());

		//if(data)
		//{
		//postSendRequest(data);
		//}

		//send(connectSocket_, userRequest.c_str(), sizeof(userRequest)+1, NULL);

		::SleepEx(100, TRUE); //make this thread alertable
	}
	*/


}

bool Client::postRecvRequest(LPSOCKETDATA data){
    DWORD flag = 0;
    DWORD bytesRecvd = 0;
    int error;

    if(data)
    {
        REQUESTCONTEXT* rc = (REQUESTCONTEXT*)malloc(sizeof(REQUESTCONTEXT));
        rc->clnt = this;
        rc->data = data;
        data->overlap.hEvent = rc;

        error = WSARecv(data->sock, &data->wsabuf, 1, &bytesRecvd, &flag, &data->overlap, runRecvComplete);
        if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
        {
            return true;
        }
        else
		{
            freeData(data);
			free(rc);
			MessageBox(NULL, "WSARecv() failed", "Critical Error", MB_ICONERROR);
            return false;
        }

    }

}



void CALLBACK Client::runRecvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
    Client* c = (Client*) rc->clnt;

    c->recvComplete(error, bytesTransferred, overlapped, flags);

}

void Client::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
    LPSOCKETDATA data = (LPSOCKETDATA) rc->data;
    Client* clnt = rc->clnt;

    if(error || bytesTransferred == 0)
    {
        freeData(data);
        return;
    }

    //check to see what mode we are in and handle data accordingly
    //will only receive when in DL, Waiting for UL approval, Streaming, Multicasting, or microphone states
    string tmp(rc->data->databuf);
	tmp;

    switch(clnt->currentState)
	{
	case DOWNLOADING:
		MessageBox(NULL, tmp.c_str(), NULL, NULL);
		break;

	case UPLOADING:
		break;

	case STREAMING:
		break;

	case L2MULTICAST:
		break;

	case MICROPHONE:
		break;
	}


}

bool Client::postSendRequest(LPSOCKETDATA data)
{
    DWORD flag = 0;
    DWORD bytesSent = 0;
    int error;

    REQUESTCONTEXT* rc = (REQUESTCONTEXT*)malloc(sizeof(REQUESTCONTEXT));
    rc->clnt = this;
    rc->data = data;
    data->overlap.hEvent = rc;

    error = WSASend(data->sock, &data->wsabuf, 1, &bytesSent, flag, &data->overlap, runSendComplete);
    if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
    {
        return true;
    }
    else
    {
        freeData(data);
        free(rc);
        MessageBox(NULL, "WSASend() failed", "Critical Error", MB_ICONERROR);
		return false;
    }

}


void CALLBACK Client::runSendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
    Client* c = (Client*) rc->clnt;

    c->sendComplete(error, bytesTransferred, overlapped, flags);

}

void Client::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{

    REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
    LPSOCKETDATA data = (LPSOCKETDATA) rc->data;
    Client* clnt = rc->clnt;

    if(error || bytesTransferred == 0)
    {
        freeData(data);
        return;
    }
	
	//if we r here we have successfully sent
	//check current state to determine next step
	switch(clnt->currentState)
	{
	case SENTDLREQUEST:
		rc->clnt->currentState = DOWNLOADING;
		break;

	case UPLOADING:
		break;

	//...

	}

}


void Client::sendDLRequest(string dlReq){
	
					
	SOCKETDATA* data = allocData(connectSocket_);
	strncpy(data->databuf, dlReq.c_str(), dlReq.size());

	if(data)
	{
		postSendRequest(data);
	}

	::SleepEx(100, TRUE); //make this thread alertable

	MessageBox(NULL, "sent DL Req" , "YAY" , MB_ICONWARNING);
}

DWORD WINAPI Client::runDLThread(LPVOID param)
{
	Client* c = (Client*) param;
	return c->dlThread();
}
DWORD Client::dlThread(/*LPVOID param*/)
{
	
	currentState = DOWNLOADING;

	while(currentState == DOWNLOADING)
	{
		SOCKETDATA* data = allocData(connectSocket_);
		if(data)
		{
			postRecvRequest(data);
		}

		::SleepEx(100, TRUE); //make this thread alertable
	}

	currentState = WFUCOMMAND;

	return 1;
}



LPSOCKETDATA Client::allocData(SOCKET socketFD)
{
    LPSOCKETDATA data = NULL;

    try{
        data = new SOCKETDATA();

    }catch(std::bad_alloc&){
		MessageBox(NULL, "Allocate socket data failed", "Error!", MB_ICONERROR);
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
		MessageBox(NULL, "Socket Closed", "", MB_ICONWARNING);
        closesocket(data->sock);
        delete data;
    }
}




DWORD WINAPI Client::runRecvThread(LPVOID param)
{
	Client* c = (Client*) param;
	return c->recvThread();
}

DWORD WINAPI Client::recvThread(/*LPVOID param*/)
{

	while(TRUE)
	{
		//SOCKETDATA* data = allocData(connectSocket_);
		//if(data)
		//{
		//postRecvRequest(data);
		//}

		cout << "In Recv Thread" << endl;

		::SleepEx(100, TRUE); //make this thread alertable
	}

}
