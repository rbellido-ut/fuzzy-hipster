//Application logic
#include "server.h"

using namespace std;

DWORD WINAPI Server::runListenThread(LPVOID args) {
	Server *s = (Server*) args;
	return s->listenThread( &s->listenSocket_);
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
        }

        ::SleepEx(100, TRUE); //make this thread alertable
    }

    return 1;
}