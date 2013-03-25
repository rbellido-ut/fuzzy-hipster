//#include "util.h"
#include "client.h"

using namespace std;

size_t Client::count_ = 0;
SOCKET Client::connectSocket_;
char  Client::sbuf[255];

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
    addr.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL)
    {
        cerr << "Unknown server address" << endl;
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
        string command;
        SOCKETDATA* data = allocData(connectSocket_);

        cout << "Enter your command: ";
        getline(cin, command);

        if(command == "download")
        {
            filetrans_req_t reqDL = {0};
            reqDL.head.type = REQDL;
            reqDL.songname = "xxx"; //note: hard coded!
            reqDL.head.size = sizeof(reqDL.songname);

            data->typeOfRequest = REQDL; // used to track the request locally

            memcpy(data->databuf,&reqDL, sizeof(filetrans_req_t));
            //send download request
        }
        if(command == "upload")
        {
            filetrans_req_t reqUL = {0};
            reqUL.head.type = REQUL;
            reqUL.songname = "xxx"; //note: hard coded!
            reqUL.head.size = sizeof(reqUL.songname);

            data->typeOfRequest = REQUL; // used to track the request locally

            memcpy(data->databuf,&reqUL, sizeof(filetrans_req_t));
            //send upload request
        }
        if(command == "stream")
        {
            stream_req_t reqStream = {0};
            reqStream.head.type = REQST;
            reqStream.songIndex = 1; //note: hard coded!
            reqStream.head.size = sizeof(int);

            data->typeOfRequest = REQST; // used to track the request locally

            memcpy(data->databuf,&reqStream, sizeof(stream_req_t));
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

        if(data)
        {
            postSendRequest(data);
        }


        ::SleepEx(100, TRUE); //make this thread alertable
    }
}

bool Client::startTCPClient()
{

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

    cout << "Client started, connected to socket " << connectSocket_ << endl;

    // will eventually port all COUT calls to QT calls
    emit statusChanged(QString("Connected to %1 (%2)").arg(hp->h_name).arg(inet_ntoa(addr.sin_addr)));

    threadHandle_ = CreateThread(NULL, 0, clientThread, NULL, 0, &threadID_);

    // start work thread (prevent the GUI from being blocked)
    HANDLE hThread;
    DWORD threadID;

    hThread = CreateThread(NULL, 0, clientWorkThread, this, 0, &threadID);

    return true;
}

void Client::encodeRequest() {

    /*
//Request packet structs
typedef struct {
    int type;
    int size;
} header_t;

typedef struct {
    header_t head;
    int songIndex;
} stream_req_t;

typedef struct {
    header_t head;
    string songname;
} filetrans_req_t; //Struct can be used for both download and upload requests
*/
    return;
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
    case REQDL:
        cout << "Client handling DL" << endl;
        //do whatever you want to do after a download request was sent successfully (save to file)
        break;
    case REQUL:
        cout << "Client handling UL" << endl;
        //do whatever you want to do after a upload request was sent successfully (wait for server to send u a confirmation)
        break;
    case REQST:
        cout << "Client handling Stream" << endl;
        //do whatever you want to do after a stream request was sent successfully (wait for approval)
        break;
    case REQMC:
        cout << "Client handling Multicast" << endl;
        //do whatever you want to do after a multicast request was sent successfully ( ... )
        break;
    case REQMIC:
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
DWORD WINAPI Client::clientThread(LPVOID lpParameter)
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
