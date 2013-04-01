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
	
	//create a socket
	connectSocket_ = createTCPClient(wsadata, hostname, port);

	if(connectSocket_ != NULL){
		//connect the socket
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

		//perform the async recv and return right away, runRecvComplete will be called upon completion
		error = WSARecv(data->sock, &data->wsabuf, 1, &bytesRecvd, &flag, &data->overlap, runRecvComplete);
		if(error == 0 || (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
		{
			return true;
		}
		else	//errors
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
	Client* clnt = (Client*) rc->clnt;

	clnt->recvComplete(error, bytesTransferred, overlapped, flags); //call the member function

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
	
	//append the binary data received to a c++ string
	string tmp;
	tmp = "";
	tmp.append(data->databuf, bytesTransferred);

	// if STREAMING, append to a custom SF stream
	//sf::Music streamplayer;
	clnt->inputstream_.streambuffer.append(data->databuf, bytesTransferred);

	//if last character is EOT, End the transmit
	if(clnt->downloadedAmount == clnt->dlFileSize)
		endOfTransmit = true;

	//open a input string stream from the binary string
	istringstream iss(tmp);
	string reqType, extra;
	int fileSize;

	switch(clnt->currentState)
	{

	case WAITFORSTREAM:
		if(iss >> reqType && iss >> fileSize)
		{
			clnt->dlFileSize = fileSize;
			clnt->downloadedAmount = 0;

			//ST Approved
			clnt->currentState = STREAMING; 

			//clnt->downloadFileStream.open("result.mp3", ios::binary);

		}
		else
		{
			clnt->currentState = WFUCOMMAND;
			MessageBox(NULL, "ST Denied", "NOT APPROVED", NULL);
		}

		break;

	case WAITFORDOWNLOAD:	//after DL request was sent in dlThread


		if(iss >> reqType && iss >> fileSize)
		{
			clnt->dlFileSize = fileSize;
			clnt->downloadedAmount = 0;


			//DL Approved
			clnt->currentState = DOWNLOADING; 
			clnt->downloadFileStream.open("result.mp3", ios::binary); //TODO: hardcoded

		}
		else
		{
			clnt->currentState = WFUCOMMAND;
			MessageBox(NULL, "DL Denied", "NOT APPROVED", NULL);
		}

		break;

	case WAITFORUPLOAD:	//after UL request was sent in ulThread

		if(iss >> reqType && getline(iss, extra)){	//get request type and file name

			extra.erase(0, extra.find_first_not_of(' ')); // trim leading white space in file name
			if(extra.empty()) //if only received "DL"
			{
				clnt->currentState = WFUCOMMAND;
				MessageBox(NULL, "UL Denied", "NOT APPROVED", NULL);
				//close file
				break;
			}

			//UL Approved
			clnt->currentState = UPLOADING; 
			clnt->uploadedAmount = 0;
		}

		break;

	case DOWNLOADING:	//while downloading

		if(endOfTransmit)	//if we have downloaded the entire file bytes
		{
			clnt->currentState = WFUCOMMAND;	//set our state to waiting for user command
			clnt->downloadFileStream.close();	//close the file stream
			clnt->dlFileSize = 0;			//reset class member values
			clnt->downloadedAmount = 0;
			break;
		}
		else	
		{
			downloadedAmount += bytesTransferred; //update the number of bytes downloaded sofar
			clnt->downloadFileStream.write(tmp.c_str(), tmp.size()); //write to the file
		}
		break;

	case STREAMING:
		if(endOfTransmit)
		{
			audiobuffer_.loadFromStream(inputstream_);
			stream_.load(audiobuffer_);
			stream_.play();

			// let it play until it is finished
			while (stream_.getStatus() == AudioStream::Playing)
				sf::sleep(sf::seconds(0.1f));
			clnt->currentState = WFUCOMMAND;
			clnt->downloadFileStream.close();
			clnt->dlFileSize = 0;
			clnt->downloadedAmount = 0;
			break;
		}
		else
		{
			downloadedAmount += bytesTransferred;

			//streamplayer_.stop();
			//streamplayer_.openFromStream(inputstream_);
			//streamplayer_.play();

			/*audiobuffer_.loadFromStream(inputstream_);
			stream_.load(audiobuffer_);
			stream_.play();

			// let it play until it is finished
			while (stream_.getStatus() == AudioStream::Playing)
				sf::sleep(sf::seconds(0.1f));*/

			//clnt->downloadFileStream.write(tmp.c_str(), tmp.size());
		}
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
	Client* clnt = (Client*) rc->clnt;

	clnt->sendComplete(error, bytesTransferred, overlapped, flags);	//call the member function

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

	//if we r here we have successfully sent
	//check current state to determine next step
	switch(clnt->currentState)
	{
	case SENTSTREQUEST:
		clnt->currentState = WAITFORSTREAM;
		dispatchOneRecv();
		break;
	case SENTDLREQUEST:	//a download request was sent successfully
		clnt->currentState = WAITFORDOWNLOAD;
		dispatchOneRecv();
		break;

	case SENTULREQUEST:	//an upload request was sent successfully
		clnt->currentState = WAITFORUPLOAD;
		dispatchOneRecv();
		break;

	case UPLOADING:		//while we are uploading
		if(clnt->ulFileSize == clnt->uploadedAmount)
		{
			clnt->currentState = WFUCOMMAND;
			clnt->uploadFileStream.close();
			clnt->ulFileSize = 0;
			clnt->uploadedAmount = 0;
		}

		clnt->uploadedAmount += bytesTransferred;
		break;

		//...other cases as need be...

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
	
	//fillup the data buffers
	memcpy(data->databuf, usrData.c_str(), usrData.size());
	data->wsabuf.len = usrData.size();

	if(data)
	{
		dispatchWSASendRequest(data);	//call WSASend
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
	Client* clnt = (Client*) param;
	return clnt->dlThread(clnt);
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
	Client* clnt = (Client*) param;
	string userRequest;

	userRequest += "DL ";
	userRequest += "Behnam's party mix.wav\n";

	clnt->currentState = SENTDLREQUEST;
	clnt->dispatchOneSend(userRequest);

	while(1)
	{
		if(clnt->currentState != DOWNLOADING)
		{
			if(clnt->currentState == WFUCOMMAND)
				break;

			continue;
		}
		dispatchOneRecv();
	}


	MessageBox(NULL, "DL Done", "Download Successful", NULL);
	return 0;
}

DWORD WINAPI Client::runSTThread(LPVOID param)
{
	Client* c = (Client*) param;
	return c->stThread(c);
}

DWORD Client::stThread(LPVOID param)
{
	Client* c = (Client*) param;
	string userRequest;

	userRequest += "ST ";
	userRequest += "Behnam's party mix.wav\n";

	c->currentState = SENTSTREQUEST;
	c->dispatchOneSend(userRequest);

	while(1)
	{
		if(c->currentState != STREAMING)
		{
			if(c->currentState == WFUCOMMAND)
				break;

			continue;
		}
		dispatchOneRecv();
	}

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
	Client* clnt = (Client*) param;
	return clnt->ulThread(clnt);
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
	Client* clnt = (Client*) param;

	streamsize numberOfBytesRead;
	string userRequest;
	ostringstream oss;

	clnt->uploadFileStream.open("result.mp3", ios::binary);
	streampos begin, end;
	begin = clnt->uploadFileStream.tellg();
	clnt->uploadFileStream.seekg(0, ios::end);
	clnt->ulFileSize = clnt->uploadFileStream.tellg()-begin;
	clnt->uploadFileStream.seekg(begin);

	oss << "UL " << clnt->ulFileSize << " Behnam's party mix.wav\n";
	userRequest = oss.str();

	clnt->currentState = SENTULREQUEST;
	clnt->dispatchOneSend(userRequest);

	while(1)
	{

		if (!uploadFileStream.is_open())
			return 1;

		char* tmp;
		string data;

		tmp = new char [DATABUFSIZE];
		memset(tmp, 0, DATABUFSIZE);
		numberOfBytesRead = 0;
		data.clear();

		clnt->uploadFileStream.read(tmp, DATABUFSIZE);
		if((numberOfBytesRead = clnt->uploadFileStream.gcount()) > 0)
		{
			data.append(tmp, numberOfBytesRead);
			dispatchOneSend(data);
			data.clear();
		}

		delete[] tmp;
		
		if(clnt->uploadedAmount == clnt->ulFileSize)
		{
			clnt->currentState = WFUCOMMAND;
			break;
		
		}
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
