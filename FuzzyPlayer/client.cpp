//#include "util.h"
#include "client.h"

using namespace std;

size_t Client::count_ = 0;

//Setter functions
bool Client::setMusicList(vector<int> ml){

    return true;
}

bool Client::createTCPClient(WSADATA* wsaData, const char* host, const int port){
    int res;
    WORD wVersionRequested;
    wVersionRequested = MAKEWORD( 2, 2 );

    if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
    {
        emit statusChanged(QString("WSAStartup falied with error (%1)").arg(res));
        WSACleanup();
        return false;
    }

    if ((connectSocket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        emit statusChanged(QString("Failed to get a socket with error (%1)").arg(WSAGetLastError()));
        return false;
    }

    // Initialize and set up the address structure
    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL)
    {
        emit statusChanged("Unknown server address");
        return false;
    }

    // Copy the server address
    memcpy((char *)&addr.sin_addr, hp->h_addr, hp->h_length);

    return true;
}

unsigned long WINAPI Client::clientWorkThread(void *ptr)
{
    Client * that = (Client*) ptr;

    while(TRUE)
    {
        REQUESTCONTEX* rc = (REQUESTCONTEX*) malloc(sizeof(REQUESTCONTEX));
        rc->data = that->allocData(that->connectSocket_);

        //SOCKETDATA* data = allocData(connectSocket_);


        //have to read user request based on their choice of GUI items
        //hardcoded for now just to download
        string command("download");
        Sleep(500);

        if(command == "download")
        {
            filetrans_req_t reqDL = {0};
            reqDL.head.type = REQDL;
            reqDL.songname = "xxx"; //note: hard coded!
            reqDL.head.size = sizeof(reqDL.songname);

            rc->data->typeOfRequest = REQDL; // used to track the request locally

            memcpy(rc->data->databuf,&reqDL, sizeof(filetrans_req_t));
            //send download request
        }
        if(command == "upload")
        {
            filetrans_req_t reqUL = {0};
            reqUL.head.type = REQUL;
            reqUL.songname = "xxx"; //note: hard coded!
            reqUL.head.size = sizeof(reqUL.songname);

            rc->data->typeOfRequest = REQUL; // used to track the request locally

            memcpy(rc->data->databuf,&reqUL, sizeof(filetrans_req_t));
            //send upload request
        }
        if(command == "stream")
        {
            stream_req_t reqStream = {0};
            reqStream.head.type = REQST;
            reqStream.songIndex = 1; //note: hard coded!
            reqStream.head.size = sizeof(int);

            rc->data->typeOfRequest = REQST; // used to track the request locally

            memcpy(rc->data->databuf,&reqStream, sizeof(stream_req_t));
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


        //strcpy(data->databuf,command.c_str());

        if(rc->data)
        {
            that->postSendRequest(rc->data);
        }

        ::SleepEx(100, TRUE); //make this thread alertable

        //free (rc);
    }
}


bool Client::startTCPClient(){

    // Connecting to the server
    if (WSAConnect (connectSocket_, (struct sockaddr *)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == INVALID_SOCKET)
    {
        emit statusChanged("Can't connect to server");
        return false;
    }

    pptr = hp->h_addr_list;

    // will eventually port all COUT calls to QT calls
    emit statusChanged(QString("Connected to %1 (%2)").arg(hp->h_name).arg(inet_ntoa(addr.sin_addr)));

    threadHandle_ = CreateThread(NULL, 0, runRecvThread, this, 0, &threadID_);

    HANDLE hThread;
    DWORD sthreadID;

    // thread the server listening function to prevent it from blocking the GUI
    hThread = CreateThread(NULL, 0, clientWorkThread, this, 0, &sthreadID);

    return true;
}

void Client::encodeRequest() {

    return;
}

bool Client::postRecvRequest(LPSOCKETDATA data){
    DWORD flag = 0;
    DWORD bytesRecvd = 0;
    int error;

    if(data)
    {
        REQUESTCONTEX* rc = (REQUESTCONTEX*)malloc(sizeof(REQUESTCONTEX));
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
            emit statusChanged("WSARecv() failed");
            freeData(data);
            return false;
        }

        free(rc);

    }

}

bool Client::postSendRequest(LPSOCKETDATA data)
{
    DWORD flag = 0;
    DWORD bytesSent = 0;
    int error;

    REQUESTCONTEX* rc = (REQUESTCONTEX*)malloc(sizeof(REQUESTCONTEX));
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
        emit statusChanged("WSASend() failedd");
        freeData(data);
        return false;
    }

    free(rc);

}

void CALLBACK Client::runRecvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    REQUESTCONTEX* rc = (REQUESTCONTEX*) overlapped->hEvent;
    Client* c = (Client*) rc->clnt;

    c->recvComplete(error, bytesTransferred, overlapped, flags);

}

void Client::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    REQUESTCONTEX* rc = (REQUESTCONTEX*) overlapped->hEvent;
    LPSOCKETDATA data = (LPSOCKETDATA) rc->data;

    if(error || bytesTransferred == 0)
    {
        freeData(data);
        return;
    }

    //handle server reply in here

}

void CALLBACK Client::runSendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    REQUESTCONTEX* rc = (REQUESTCONTEX*) overlapped->hEvent;
    Client* c = (Client*) rc->clnt;

    c->sendComplete(error, bytesTransferred, overlapped, flags);

}

void Client::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{

    REQUESTCONTEX* rc = (REQUESTCONTEX*) overlapped->hEvent;
    LPSOCKETDATA data = (LPSOCKETDATA) rc->data;

    if(error || bytesTransferred == 0)
    {
        freeData(data);
        return;
    }

    switch(data->typeOfRequest)
    {
    case REQDL:
        emit statusChanged("Client handeling download");
        //do whatever you want to do after a download request was sent successfully (save to file)
        break;
    case REQUL:
        emit statusChanged("Client handeling upload");
        //do whatever you want to do after a upload request was sent successfully (wait for server to send u a confirmation)
        break;
    case REQST:
        emit statusChanged("Client handeling stream");
        //do whatever you want to do after a stream request was sent successfully (wait for approval)
        break;
    case REQMC:
        emit statusChanged("Client handeling multicast");
        //do whatever you want to do after a multicast request was sent successfully ( ... )
        break;
    case REQMIC:
        emit statusChanged("Client handeling microphone");
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
        emit statusChanged("Allocate socket data failed");
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
        emit statusChanged("Socket Closed");
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
DWORD Client::clientRecvThread()
{

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


DWORD WINAPI Client::runRecvThread(LPVOID args)
{
    Client *c = (Client*)args;
    return c->clientRecvThread();
}

bool Client::createUDPClient(WSADATA* wsaData, const char* host, const int port){

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
