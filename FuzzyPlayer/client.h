#ifndef CLIENT_H
#define CLIENT_H

#include "util.h"
#include "communication.h"

//The server class extends the Communication class
class Client : public Communication
{


public:
    //Constructor calls parrent constructor for shared members
    explicit Client(const std::string& readBuffer = "",const std::string& writeBuffer = "",
        const int& bytesSent = 0, const int& bytesRecvd = 0,
        const SOCKET& readSocket = 0, const SOCKET& writeSocket = 0)
        :Communication(readBuffer, writeBuffer, bytesSent, bytesRecvd, readSocket, writeSocket)
        //musicList_(musicList)
    { count_++; }

    //Copy constructor calls parent copy constructor for shared members
    Client(const Client& src)
        :Communication(src), musicList_(src.musicList_)
    { count_++; }

    //Destructor
    ~Client(){ count_--; }

    //Getter functions
    std::vector<int> getMusicList() { return musicList_; }
    static size_t getCount(){ return count_; }

    //Setter functions
    bool setMusicList(std::vector<int> ml);

    //Primary function prototypes
    bool createTCPClient(WSADATA*, const char*, const int);
    bool startTCPClient();
    bool createUDPClient(WSADATA*, const char*, const int);
    bool startUDPClient();

    void encodeRequest();

    bool stopClient();
    bool requestConnect();
    bool requestDownload();
    bool requestUpload();
    bool requestStream();
    bool saveToFile();

    static DWORD WINAPI runRecvThread(LPVOID args);

    LPSOCKETDATA allocData(SOCKET fd);
    void freeData(LPSOCKETDATA data);

    bool postSendRequest(LPSOCKETDATA data);
    bool postRecvRequest(LPSOCKETDATA data);

    void recvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    void sendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

    static void CALLBACK runRecvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runSendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);


    DWORD clientRecvThread();
    void sendTCP(SOCKET& clntSock);



    //friend decleration of input and output operators
    friend std::ostream& operator<< (std::ostream& os, const Client& c);
    friend std::istream& operator>> (std::istream& is, Client& c);

private:
    //Data members
    std::vector<int> musicList_; //std::vector<music>? would have to create a music class
    static size_t count_;
    SOCKET connectSocket_;
    struct hostent	*hp;
    char **pptr;
    SOCKADDR_IN addr;

    HANDLE threadHandle_;
    DWORD threadID_;
    char  sbuf[255];

};



#endif
