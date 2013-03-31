/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		client.cpp -  This file contains the implementations of the client member functions
--
-- PROGRAM:			Fuzzy-Hipster
--
-- FUNCTIONS:		bool Client::runClient(WSADATA* wsadata, const char* hostname, const int port) 
--					SOCKET Client::createTCPClient(WSADATA* wsaData, const char* hostname, const int port) 
--					bool Client::dispatchWSARecvRequest(LPSOCKETDATA data)
--					void CALLBACK Client::runRecvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--					void Client::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--					bool Client::dispatchWSASendRequest(LPSOCKETDATA data)
--					void CALLBACK Client::runSendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--					void Client::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--					void Client::dispatchOneSend(string usrData)
--					void Client::dispatchOneRecv()
--					DWORD WINAPI Client::runDLThread(LPVOID param)
--					DWORD Client::dlThread(LPVOID param)
--					DWORD WINAPI Client::runULThread(LPVOID param)
--					DWORD Client::ulThread(LPVOID param)
--					LPSOCKETDATA Client::allocData(SOCKET socketFD)
--					void Client::freeData(LPSOCKETDATA data)
-- 
-- DATE:			March 15, 2013
--
-- REVISIONS: 
--
-- DESIGNER:		Behnam Bastami
--
-- PROGRAMMER:		Behnam Bastami
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

#include "client.h"

using namespace std;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	runClient
--
-- DATE:		March 15, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	bool Client::runClient(WSADATA* wsadata, const char* hostname, const int port) 
--				wsaData: pointer to WSADATA struct
--				hostname: the host to connect to
--				port: the port number on the host being connected to
--
-- RETURNS:		true on success and false on failure
--				
--
-- NOTES:		This is the main entry point to the client side of the application, an async TCP socket is
--				created by a call to createTCPClient() and if that call was successful, the socket is connected
--				to the server which the user specified
--
----------------------------------------------------------------------------------------------------------------------*/
bool Client::runClient(WSADATA* wsadata, const char* hostname, const int port) 
{

	char **pptr;

	connectSocket_ = createTCPClient(wsadata, hostname, port);

	if(connectSocket_ != NULL){
		if (WSAConnect (connectSocket_, (struct sockaddr *)&addr_, sizeof(addr_), NULL, NULL, NULL, NULL) == INVALID_SOCKET)
		{
			MessageBox(NULL, "Can't connect to server", "Connection Error", MB_ICONERROR);
			return false;
		}

		currentState = WFUCOMMAND;

		return true;
	}

	return false;
	
	//now that we are connected , we are going to wait for user command

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	createTCPClient
--
-- DATE:		March 15, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	SOCKET Client::createTCPClient(WSADATA* wsaData, const char* hostname, const int port) 
--				wsaData: pointer to WSADATA struct
--				hostname: the host to connect to
--				port: the port number on the host being connected to
--
-- RETURNS:		A async socket if the socket was created successfully and NULL on failure
--				
--
-- NOTES:		This function is called by runClient, to create a TCP async socket
--
----------------------------------------------------------------------------------------------------------------------*/
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
		MessageBox(NULL, "Failed to get create a socket!", "WSASocket Error", MB_ICONERROR);
		return NULL;
	}

	// Initialize and set up the address structure
	memset((char *)&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port); 

	if ((hp_ = gethostbyname(hostname)) == NULL) 
	{
		MessageBox(NULL, "Unknown server address", "Connection Error", MB_ICONERROR);
		return NULL;
	}

	// Copy the server address
	memcpy((char *)&addr_.sin_addr, hp_->h_addr, hp_->h_length);

	return connectSocket;

}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	dispatchWSARecvRequest
--
-- DATE:		March 10, 2013
--
-- REVISIONS:	March 14, 2013, Made the function work with the client object
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	bool Client::dispatchWSARecvRequest(LPSOCKETDATA data)
--				data: pointer to a LPSOCKETDATA struct which contains the information needed for a WSARecv call
--					  including the socket, buffers, etc.
--
-- RETURNS:		true on success and false on failure
--				
--
-- NOTES:		This function posts a WSARecv (Async Recv call) request specifying a call back function to be 
--				executed upon the completion of receive call
--
----------------------------------------------------------------------------------------------------------------------*/
bool Client::dispatchWSARecvRequest(LPSOCKETDATA data)
{
	DWORD flag = 0;
	DWORD bytesRecvd = 0;
	int error;

	if(data)
	{
		//create a client request context which includes a client and a data structure
		REQUESTCONTEXT* rc = (REQUESTCONTEXT*)malloc(sizeof(REQUESTCONTEXT));
		rc->clnt = this;
		rc->data = data;
		data->overlap.hEvent = rc;

		//perform the async recv and return right away
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	runRecvComplete
--
-- DATE:		March 12, 2013
--
-- REVISIONS:	March 27, 2013, made it work with this application
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void CALLBACK Client::runRecvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--				error: indicates if there were any errors in the async receive call
--				bytesTransferred: number of bytes received
--				overlapped: the overlapped structure used to make the asynf recv call
--				flags: other flags
--
-- RETURNS:		void
--				
--
-- NOTES:		This function is used to run a class member function as a call back function for the completion routine,
--				whenever an async recv request was completed, without having to make everything else static
--				
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK Client::runRecvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
	Client* c = (Client*) rc->clnt;

	c->recvComplete(error, bytesTransferred, overlapped, flags);

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	recvComplete
--
-- DATE:		March 9, 2013
--
-- REVISIONS:	March 14, 2013, Made the function work for this application
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void Client::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--				error: indicates if there were any errors in the async receive call
--				bytesTransferred: number of bytes received
--				overlapped: the overlapped structure used to make the asynf recv call
--				flags: other flags
--
-- RETURNS:		void
--				
--
-- NOTES:		This is the function thet gets executed after each async recv call is completed
--				It extracts the received data from the data buffers and dictates how the client 
--				should behave (do next) by changing the client state
--
----------------------------------------------------------------------------------------------------------------------*/
void Client::recvComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
	LPSOCKETDATA data = (LPSOCKETDATA) rc->data;
	Client* clnt = rc->clnt;
	bool endOfTransmit = false;



	if(error || bytesTransferred == 0)
	{
		freeData(data);
		return;
	}
	//check to see what mode we are in and handle data accordingly
	//will only receive when in DL, Waiting for UL approval, Streaming, Multicasting, or microphone states
	string tmp;
	tmp = "";
	tmp.append(data->databuf, bytesTransferred);

	//if last character is EOT, End the transmit
	if(tmp == "DLEND\n")
		endOfTransmit = true;

	istringstream iss(tmp);
	string reqType, extra;


	switch(clnt->currentState)
	{
	case WAITFORDOWNLOAD:

		if(iss >> reqType && getline(iss, extra)){
			clnt->sizeOfDownloadFile = 0;
			clnt->currentState = DOWNLOADING; 
			extra.erase(0, extra.find_first_not_of(' ')); // get file size

			//DL Approved
			clnt->downloadFileStream.open("result.mp3", ios::binary);
			clnt->dlThreadHandle = CreateThread(NULL, 0, clnt->runDLThread, clnt, 0, &clnt->dlThreadID);
		}
		else
		{
			MessageBox(NULL, "DL Denied", "NOT APPROVED", NULL);
		}

		break;

	case WAITFORUPLOAD:
		MessageBox(NULL, "UL'ing", "", NULL);

		if(iss >> reqType && getline(iss, extra)){
			clnt->currentState = UPLOADING; 
			extra.erase(0, extra.find_first_not_of(' ')); // get file name
			//UL Approved
			clnt->uploadFileStream.open("result.mp3", ios::binary);
			clnt->ulThreadHandle = CreateThread(NULL, 0, clnt->runULThread, clnt, 0, &clnt->ulThreadID);
		}
		else
		{
			MessageBox(NULL, "UL Denied", "NOT APPROVED", NULL);
		}

		break;

	case DOWNLOADING:

		if(endOfTransmit){
			clnt->currentState = WFUCOMMAND;
			clnt->downloadFileStream.close();
			break;
		}else{
			clnt->downloadFileStream.write(tmp.c_str(), tmp.size());

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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	dispatchWSASendRequest
--
-- DATE:		March 9, 2013
--
-- REVISIONS:	March 14, 2013, Made the function work with the client object
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	bool Client::dispatchWSASendRequest(LPSOCKETDATA data)
--				data: pointer to a LPSOCKETDATA struct which contains the information needed for a WSARecv call
--					  including the socket, buffers, etc.
--
-- RETURNS:		true on success and false on failure
--				
--
-- NOTES:		This function posts a WSASend (Async Send call) request specifying a call back function to be 
--				executed upon the completion of send call
--
----------------------------------------------------------------------------------------------------------------------*/
bool Client::dispatchWSASendRequest(LPSOCKETDATA data)
{
	DWORD flag = 0;
	DWORD bytesSent = 0;
	int error;

	//create a client request context which includes a client and a data structure
	REQUESTCONTEXT* rc = (REQUESTCONTEXT*)malloc(sizeof(REQUESTCONTEXT));
	rc->clnt = this;
	rc->data = data;
	data->overlap.hEvent = rc;

	//perform the async send and return right away
	error = WSASend(data->sock, &data->wsabuf, 1, &bytesSent, flag, &data->overlap, runSendComplete);
	if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
	{
		return true;
	}
	else	//errors
	{
		freeData(data);
		free(rc);
		MessageBox(NULL, "WSASend() failed", "Critical Error", MB_ICONERROR);
		return false;
	}

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	runSendComplete
--
-- DATE:		March 11, 2013
--
-- REVISIONS:	March 27, 2013, made it work with this application
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void CALLBACK Client::runSendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--				error: indicates if there were any errors in the async receive call
--				bytesTransferred: number of bytes received
--				overlapped: the overlapped structure used to make the asynf recv call
--				flags: other flags
--
-- RETURNS:		void
--				
--
-- NOTES:		This function is used to run a class member function as a call back function for the completion routine,
--				whenever an async send request was completed, without having to make everything else static
--				
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK Client::runSendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
	Client* c = (Client*) rc->clnt;

	c->sendComplete(error, bytesTransferred, overlapped, flags);

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	sendComplete
--
-- DATE:		March 10, 2013
--
-- REVISIONS:	March 14, 2013, Made the function work for this application
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void Client::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--				error: indicates if there were any errors in the async receive call
--				bytesTransferred: number of bytes received
--				overlapped: the overlapped structure used to make the asynf recv call
--				flags: other flags
--
-- RETURNS:		void
--				
--
-- NOTES:		This is the function thet gets executed after each async send call is completed
--				It looks at the data that was sent and dictates how the client should behave (do next)
--				by changing the client state
--
----------------------------------------------------------------------------------------------------------------------*/

void Client::sendComplete (DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{

	REQUESTCONTEXT* rc = (REQUESTCONTEXT*) overlapped->hEvent;
	LPSOCKETDATA data = (LPSOCKETDATA) rc->data;
	Client* clnt = rc->clnt;
	bool endOfTransmit = false;

	if(error || bytesTransferred == 0)
	{
		freeData(data);
		return;
	}

	if(data->databuf[bytesTransferred-1] == '\n')
		endOfTransmit = true;

	//if we r here we have successfully sent
	//check current state to determine next step
	switch(clnt->currentState)
	{
	case SENTDLREQUEST:
		clnt->currentState = WAITFORDOWNLOAD;
		dispatchOneRecv();
		break;

	case SENTULREQUEST:
		clnt->currentState = WAITFORUPLOAD;
		dispatchOneRecv();
		break;

	case UPLOADING:
		if(endOfTransmit)
		{
			clnt->currentState = WFUCOMMAND;
		}
		break;

		//...

	}


}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	dispatchOneSend
--
-- DATE:		March 12, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void Client::dispatchOneSend(string usrData)
--				data: the user data to send, in the form of a c++ string
--
-- RETURNS:		void
--				
--
-- NOTES:		This function makes an async Send request and then puts the thread in an alertable
--				state, meaning that the same thread will serve the completed operation when it is completed
--
----------------------------------------------------------------------------------------------------------------------*/
void Client::dispatchOneSend(string usrData)
{


	SOCKETDATA* data = allocData(connectSocket_);
	strncpy(data->databuf, usrData.c_str(), usrData.size());

	if(data)
	{
		dispatchWSASendRequest(data);
	}

	::SleepEx(INFINITE, TRUE); //make this thread alertable

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	dispatchOneRecv
--
-- DATE:		March 12, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void Client::dispatchOneRecv()
--
-- RETURNS:		void
--				
--
-- NOTES:		This function makes an async Recv request and then puts the thread in an alertable
--				state, meaning that the same thread will serve the completed operation when it is completed
--
----------------------------------------------------------------------------------------------------------------------*/
void Client::dispatchOneRecv()
{

	SOCKETDATA* data = allocData(connectSocket_);
	if(data)
	{
		dispatchWSARecvRequest(data);
	}

	::SleepEx(INFINITE, TRUE); //make this thread alertable

}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	runDLThread
--
-- DATE:		March 20, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	DWORD WINAPI Client::runDLThread(LPVOID param)
--				param: the parameter we would like to pass to the runned thread (a client object)
--
-- RETURNS:		The result of the thread proc operation
--				
--
-- NOTES:		This function is used to run a class member function as the download thread proc,
--				without having to make everything static
--				
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI Client::runDLThread(LPVOID param)
{
	Client* c = (Client*) param;
	return c->dlThread(c);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	dlThread
--
-- DATE:		March 21, 2013
--
-- REVISIONS:	March 26, 2013, started using dispatchOneRecv()
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	DWORD Client::dlThread(LPVOID param)
--				param: the parameter we would like to pass to the runned thread (a client object)
--
-- RETURNS:		0 after download was completed
--				
--
-- NOTES:		This is the Download thread proc function.
--				It posts requests to receive data off the socket, while the client is in download mode.
--				
----------------------------------------------------------------------------------------------------------------------*/
DWORD Client::dlThread(LPVOID param)
{
	Client* c = (Client*) param;

	while(c->currentState == DOWNLOADING)
	{
		dispatchOneRecv();
	}


	MessageBox(NULL, "DL Done", "Download Successful", NULL);
	return 0;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	runULThread
--
-- DATE:		March 20, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	DWORD WINAPI Client::runULThread(LPVOID param)
--				param: the parameter we would like to pass to the runned thread (a client object)
--
-- RETURNS:		void
--				
--
-- NOTES:		This function is used to run a class member function as the upload thread proc,
--				without having to make everything static
--				
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI Client::runULThread(LPVOID param)
{
	Client* c = (Client*) param;
	return c->ulThread(c);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	ulThread
--
-- DATE:		March 28, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	DWORD Client::ulThread(LPVOID param)
--				param: the parameter we would like to pass to the runned thread (a client object)
--
-- RETURNS:		0 after upload was completed or 1 on failure to open a file
--				
--
-- NOTES:		This is the Upload thread proc function.
--				While the client is in upload state, it reads data from the file and sends it to the server
--				by posting async send calls
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD Client::ulThread(LPVOID param)
{
	Client* c = (Client*) param;



	while(c->currentState == UPLOADING)
	{
		if (!uploadFileStream.is_open()) 
			return 1;

		char* tmp;
		string data;

		while (true)
		{
			tmp = new char [DATABUFSIZE];
			int n = 0;
			data.clear();

			c->uploadFileStream.read(tmp, DATABUFSIZE);
			if((n=c->uploadFileStream.gcount()) > 0)
			{
				data.append(tmp, n);
				dispatchOneSend(data.c_str());
				data.clear();
			}
			else
			{
				delete[] tmp;
				break;
			}

			delete[] tmp;
		}

		data = "UL END\n";
		dispatchOneSend(data);
		
	}

	MessageBox(NULL, "UL Done", "Upload Successful", NULL);
	return 0;
}



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	allocData
--
-- DATE:		March 11, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	LPSOCKETDATA Client::allocData(SOCKET socketFD)
--				
--
-- RETURNS:		returns a pointer to the memory block that was allocated for the LPSOCKETDATA struct
--				
--
-- NOTES:		This function is used to safely allocate memory for a LPSOCKETDATA type variable
--				It also sets up other variables that are needed for the operation
--
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	freeData
--
-- DATE:		March 10, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	void Client::freeData(LPSOCKETDATA data)
--				data: pointer to a LPSOCKETDATA struct that contain the information needed to 
--					  perform an async call
--
-- RETURNS:		void
--				
--
-- NOTES:		This function is used to safely free the memory block that was allocated for the 
--				LPSOCKETDATA struct
----------------------------------------------------------------------------------------------------------------------*/
void Client::freeData(LPSOCKETDATA data)
{
	if(data)
	{
		closesocket(data->sock);
		delete data;
	}
}
