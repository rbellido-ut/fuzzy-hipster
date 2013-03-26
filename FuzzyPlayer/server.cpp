#include "server.h"

using namespace std;

std::map<int, LPSOCKETDATA> Server::mSocketList_;

extern Server sv;

bool Server::createServer(WSADATA* wsaData, int protocol)
{
	int res;
	SOCKADDR_IN addr;
    protocolType_ = protocol;

	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );

	if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
	{
		cerr << "WSAStartup falied with error " << res << endl;
		WSACleanup();
		return false;
	}

    if ((listenSocket = WSASocket(AF_INET, (protocolType_ == TCP) ? SOCK_STREAM : SOCK_DGRAM , 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		cerr << "Failed to get a socket with error " << WSAGetLastError() << endl;
		return false;
	}

    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    addr.sin_port           = htons((protocolType_ == TCP) ? TCPPORT : UDPPORT);

	if (bind(listenSocket, (PSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		cerr << "bind() falied with error " << WSAGetLastError() << endl;
		return false;
	}

    if (protocol != UDP)
    {
        if (listen(listenSocket, 5))
        {
            cerr << "listen() falied with error " << WSAGetLastError() << endl;
            closesocket(listenSocket);
            return 0;
        }
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

unsigned long WINAPI Server::serverWorkThread(void *ptr)
{
    Server * that = (Server*) ptr;

    cout << "Server started, listening on socket " << that->listenSocket << endl;

    while(TRUE)
    {
        SOCKADDR_IN addr = {};
        int addrLen = sizeof(addr);

        SOCKET newClientSocket = WSAAccept(that->listenSocket, (sockaddr*)&addr, &addrLen, NULL, NULL);

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
            SOCKETDATA* clientRequests = allocData(newClientSocket);
            cout << "Socket " << newClientSocket << " accepted." << endl;
            if(clientRequests)
            {
                //TODO: add thread to handle clients
                postRecvRequest(clientRequests);
            }
        }

        ::SleepEx(100, TRUE); //make this thread alertable
    }

    return true;

}

//Other function prototypes
bool Server::startServer()
{
    HANDLE hSThread;
    DWORD sthreadID;

    //server_.createServer(&wsadata, protocol);

    // thread the server listening function to prevent it from blocking the GUI
    hSThread = CreateThread(NULL, 0, serverWorkThread, this, 0, &sthreadID);
    //server_.startServer();

    return true;
}

LPSOCKETDATA Server::allocData(SOCKET socketFD)
{
	LPSOCKETDATA data = NULL;

    try
    {
		data = new SOCKETDATA();
    }
    catch(std::bad_alloc&)
    {
		cerr << "Allocate socket data failed" << endl;
		return NULL;
	}

    data->overlap.hEvent    = (WSAEVENT)data;
    data->sock              = socketFD;
    data->wsabuf.buf        = data->databuf;
    data->wsabuf.len        = sizeof(data->databuf);

	mSocketList_[socketFD] = data;

	return data;
}

void Server::freeData(LPSOCKETDATA data)
{
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

bool Server::postSendRequest(LPSOCKETDATA data)
{
	DWORD flags = 0;
	DWORD bytesSent = 0;
	int error;

	error = WSASend(data->sock, &data->wsabuf, 1, &bytesSent, flags, &data->overlap, sendComplete);
	if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
	{
		//success
		return true;
	}
	else
	{
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

    //postSendRequest(data);
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	DecodeRequest
--
-- DATE:		March 23, 2013
--
-- REVISIONS:
--
-- DESIGNER:	Ronald Bellido
--
-- PROGRAMMER:	Ronald Bellido
--
-- INTERFACE:	DecodeRequest(LPSOCKETDATA packet)
--                  packet - packet received by the server for its contents to be decoded
--
-- RETURNS: void
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
void Server::DecodeRequest(LPSOCKETDATA packet)
{
}
