//Application logic
#include "server.h"
#include "net_stream.h"

using namespace std;

void Server::runServer(WSADATA * wsadata)
{
	HANDLE acceptClientThreadHandle;
	DWORD acceptClientThreadID;

	listenSocket_ = createServer(wsadata, TCP);
	acceptClientThreadHandle = CreateThread(NULL, 0, runListenThread, this, 0, &acceptClientThreadID);
}


DWORD WINAPI Server::listenThread(LPVOID args)
{
    SOCKET* pListenSock = (SOCKET*) args;

    cout << "Server started, listening on socket " << *pListenSock << endl;

    while(TRUE)
    {
        SOCKADDR_IN addr = {};
        int addrLen = sizeof(addr);

        SOCKET newClientSocket = WSAAccept(*pListenSock, (sockaddr*)&addr, &addrLen, NULL, NULL);

        if(newClientSocket == INVALID_SOCKET)
        {
            if(WSAGetLastError() != WSAEWOULDBLOCK)
            {
                cerr << "accept() failed with error " << WSAGetLastError() << endl;
                break;
            }
        }
        else
        {
            cout << "Socket " << newClientSocket << " accepted." << endl;
			//create handleClientThread,  passing &newClientSocket
			//push back to clientlist
			//CreateThread(NULL, 0, handleClient, (LPVOID) newClientSocket, 0, 0);
        }

        ::SleepEx(100, TRUE); //make this thread alertable
    }

    return 1;
}


DWORD WINAPI Server::handleClient(LPVOID param)
{
	SOCKET* controlSocket = (SOCKET*) param;
	int numBytesRecvd, bytesToRead;
	char request[DATABUFSIZE];
	char * p;

	while (TRUE)
	{
		//listen for requests
		//when you receive, parse the request

		//recv(...)
		//when you receive stuff, call parse request
		
		p = request;
		bytesToRead = DATABUFSIZE;
		while ((numBytesRecvd = recv(*controlSocket, p, bytesToRead, 0)) > 0)
		{
			p += numBytesRecvd;
			bytesToRead -= numBytesRecvd;
		}

		ParseRequest(request);
	}
}


DWORD WINAPI Server::runListenThread(LPVOID args)
{
	Server *s = (Server*) args;
	return s->listenThread( &s->listenSocket_);
}


/*DWORD WINAPI Server::runHandleClientThread(LPVOID args)
{
	Server* s = (Server*) args;
	return s->handleClient();
}*/


void Server::ParseRequest(char * request)
{
	string req = request;
	stringstream ss(req);
	string filename;
	string requesttype;

	ss >> requesttype;
	cout << requesttype << " ";

	if (requesttype == "ST")
	{
		ss >> filename;
		cout << filename << endl;
	}
}
