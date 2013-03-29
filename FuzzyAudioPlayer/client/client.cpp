#include "client.h"

using namespace std;

SOCKET Client::createTCPClient(WSADATA* wsaData, const char* hostname, const int port) 
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
	addr_.sin_port = htons(port); //TODO: this is hard coded

	if ((hp_ = gethostbyname(hostname)) == NULL) //TODO: this is hard-coded
	{
		MessageBox(NULL, "Unknown server address", "Connection Error", MB_ICONERROR);
		return NULL;
	}

	// Copy the server address
	memcpy((char *)&addr_.sin_addr, hp_->h_addr, hp_->h_length);

	return connectSocket;

}

bool Client::runClient(WSADATA *wsadata, const char* hostname, const int port) 
{

	char **pptr;

	connectSocket_ = createTCPClient(wsadata, hostname, port);

	if (WSAConnect (connectSocket_, (struct sockaddr *)&addr_, sizeof(addr_), NULL, NULL, NULL, NULL) == INVALID_SOCKET)
	{
		MessageBox(NULL, "Can't connect to server", "Connection Error", MB_ICONERROR);
		return false;
	}

	MessageBox(NULL, "IM CONNECTEDDD! YEAAAAAA !", "Connected to server", MB_ICONINFORMATION);
	currentState = WFUCOMMAND;

	return true;

	//now that we are connected , we are going to wait for user command

}

bool Client::dispatchWSARecvRequest(LPSOCKETDATA data){
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
    string tmp(data->databuf);
	istringstream iss(tmp);
	string reqType, extra;

    switch(clnt->currentState)
	{
	case DOWNLOADING:
		MessageBox(NULL, "DL'ing", "", NULL);
		clnt->dlThreadHandle = CreateThread(NULL, 0, clnt->runDLThread, &clnt, 0, &clnt->dlThreadID);
		MessageBox(NULL, "UL Thread created", "DL'ing", NULL);
		break;

	case WAITFORAPPROVAL:
		MessageBox(NULL, "UL'ing", "", NULL);
		//MessageBox(NULL, tmp.c_str(), NULL, NULL);
		if(iss >> reqType && iss >> extra){
			clnt->currentState = UPLOADING; 
			MessageBox(NULL, "UL Approved", "APPROVED", NULL);
			clnt->ulThreadHandle = CreateThread(NULL, 0, clnt->runULThread, &clnt, 0, &clnt->ulThreadID);
			MessageBox(NULL, "UL Thread created", "UL'ing", NULL);
		}
		else
		{
			MessageBox(NULL, "UL Denied", "NOT APPROVED", NULL);
		}

		break;

	case STREAMING:
		break;

	case L2MULTICAST:
		break;

	case MICROPHONE:
		break;
	}


}

bool Client::dispatchWSASendRequest(LPSOCKETDATA data)
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
		clnt->currentState = DOWNLOADING;
		dispatchOneRecv();
		break;

	case SENTULREQUEST:
		clnt->currentState = WAITFORAPPROVAL;
		dispatchOneRecv();
		

		break;
					   
	case UPLOADING:
		break;

	//...

	}

}


void Client::dispatchClientRequest(string usrReq){
	
					
	SOCKETDATA* data = allocData(connectSocket_);
	strncpy(data->databuf, usrReq.c_str(), usrReq.size());

	if(data)
	{
		dispatchWSASendRequest(data);
	}

	::SleepEx(100, TRUE); //make this thread alertable

	//MessageBox(NULL, "sent Req" , "" , MB_ICONWARNING);
}

void Client::dispatchOneRecv(){

	SOCKETDATA* data = allocData(connectSocket_);
	if(data)
	{
		dispatchWSARecvRequest(data);
	}

	::SleepEx(100, TRUE); //make this thread alertable
}

DWORD WINAPI Client::runDLThread(LPVOID param)
{
	Client* c = (Client*) param;
	return c->dlThread();
}

DWORD Client::dlThread(/*LPVOID param*/)
{
	
	while(1)
	{
		MessageBox(NULL, "In DL Thread", "", NULL);
		Sleep(1000);
	}
	
	return 0;
}


DWORD WINAPI Client::runULThread(LPVOID param)
{
	Client* c = (Client*) param;
	return c->ulThread();
}

DWORD Client::ulThread(/*LPVOID param*/)
{
	
	while(1)
	{
		MessageBox(NULL, "In UL Thread", "", NULL);
		Sleep(1000);
	}
	
	return 0;
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
		//MessageBox(NULL, "Socket Closed", "", MB_ICONWARNING);
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

	while(1)
	{
		MessageBox(NULL, "In DL Thread", "", NULL);
		Sleep(1000);
	}
	
	return 0;

}
