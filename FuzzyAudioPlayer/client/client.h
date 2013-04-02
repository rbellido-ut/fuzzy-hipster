#include "utils.h"
#include "libzplay.h"

class Client {

public:
	Client()
	{ 
		currentState = NOTCONNECTED;
		connectSocket_ = 0; 
		player_ = libZPlay::CreateZPlay();
	}
	~Client()
	{
		player_->Release();
	}

	bool runClient(WSADATA *wsadata, const char*, const int);
	

	int currentState;
	DWORD dlThreadID;
	HANDLE dlThreadHandle, stThreadHandle, ulThreadHandle, listThreadHandle;

	DWORD stThreadID, ulThreadID, listThreadID;
	DWORD stThread(LPVOID);
	static DWORD WINAPI runSTThread(LPVOID);

	DWORD dlThread(LPVOID);
	static DWORD WINAPI runDLThread(LPVOID);

	DWORD ulThread(LPVOID param);
	static DWORD WINAPI runULThread(LPVOID param);

	DWORD listThread(LPVOID param);
	static DWORD WINAPI runListThread(LPVOID param);

	void dispatchOneSend(std::string dlReq);
	void dispatchOneRecv();

	std::ofstream downloadFileStream;
	std::ifstream uploadFileStream;
	int sizeOfDownloadFile;
	int sizeOfUploadFile;

	long int dlFileSize, ulFileSize;
	int downloadedAmount, uploadedAmount;
	std::string cachedServerSongString;
	std::vector<std::string> localSongList;
	std::string currentSongFile;

private:
	SOCKET connectSocket_;
	SOCKADDR_IN addr_;
	hostent *hp_;
	libZPlay::ZPlay *player_;

	SOCKET createTCPClient(WSADATA*, const char*, const int);

	static DWORD WINAPI runRecvThread(LPVOID param);
	DWORD WINAPI Client::recvThread(/*LPVOID param*/);

	LPSOCKETDATA allocData(SOCKET fd);
    void freeData(LPSOCKETDATA data);

	bool dispatchWSASendRequest(LPSOCKETDATA data);
    bool dispatchWSARecvRequest(LPSOCKETDATA data);
	void recvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    void sendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runRecvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runSendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
	libZPlay::TStreamFormat parseFileFormat(std::string filename);
};

