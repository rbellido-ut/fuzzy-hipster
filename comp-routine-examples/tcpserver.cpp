/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: tcpserver.cpp - contains function calls that are used to create a tcp server and process data it receives
--
-- PROGRAM: advanced-io
--
-- FUNCTIONS:
		int StartTCPServer(HWND hDlg)
		void InitializeDialogBox(int whatisthis, HWND hDlg)
		OPENFILENAME GetFileToSendFromDialog(HWND hDlg)
--
-- DATE: February 12, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- NOTES:
Source file contains function calls that are related to a TCP server.
----------------------------------------------------------------------------------------------------------------------*/

#include "tcpserver.h"

using namespace std;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: StartTCPServer
--
-- DATE: January 12, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: int StartTCPServer(HWND hDlg)
--
-- RETURNS: 1 - successful preparing connection sockets
--			0 - otherwise
-- NOTES:
Grabs data from GUI menu and uses the data to establish a TCP server. 
After creating the socket and binding it, it creates two threads to handle an accept function call and
for handling the completion routine.
----------------------------------------------------------------------------------------------------------------------*/
int StartTCPServer(HWND hDlg)
{
	DWORD res;
	SOCKET Listen;
	SOCKADDR_IN InternetAddr;
	WSADATA wsaData;
	HANDLE CompleteIOThreadHandle, AcceptConnectionThreadHandle;
	DWORD CompleteIOThreadID, AcceptConnectionThreadID;
	LPEXTRA_DATA extradata= (LPEXTRA_DATA) malloc(sizeof(EXTRA_DATA));
	extradata->hDlg = hDlg;

	//Grab data from text boxes
	char *addr_text, *port_text, *savefile;
	int input_length;

	input_length = GetWindowTextLength(GetDlgItem(hDlg, IDC_IPHOSTNAME)) + 1;
	addr_text = (char *) malloc(sizeof(char) * input_length);
	GetDlgItemText(hDlg, IDC_IPHOSTNAME, addr_text, input_length);

	input_length = GetWindowTextLength(GetDlgItem(hDlg, IDC_PORTNO)) + 1;
	port_text = (char *) malloc(sizeof(char) * input_length);
	GetDlgItemText(hDlg, IDC_PORTNO, port_text, input_length);

	input_length = GetWindowTextLength(GetDlgItem(hDlg, IDC_SERVERFILESTORE)) + 1;
	savefile = (char *) malloc(sizeof(char) * input_length);
	GetDlgItemText(hDlg, IDC_SERVERFILESTORE, savefile, input_length);
	savefile[strlen(savefile) - 1] = '\0';
	extradata->filename = savefile;
	//finished grabbing data from text boxes

	if ((res = WSAStartup(0x0202, &wsaData)) != 0)
	{
		ErrorMsg("WSAStartup failed!", hDlg);
		return 0;
	}

	if ((Listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		ErrorMsg("WSASocket() failed!", hDlg);
		return 0;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY); //TODO: might not be a good idea to use any type of address. Better way is to get the actual ip address from dialog box
	InternetAddr.sin_port = htons(atoi(port_text));

	if (bind(Listen, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		ErrorMsg("bind() failed!", hDlg);
		return 0;
	}

	if (listen(Listen, 5))
	{
		int error =  WSAGetLastError();
		ErrorMsg("listen() failed!", hDlg);
		return 0;
	}

	if ((extradata->wsaEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		ErrorMsg("WSACreateEvent() failed!", hDlg);
		return 0;
	}

	extradata->listensocket = Listen;

	if ((AcceptConnectionThreadHandle = CreateThread(NULL, 0, AcceptConnectionThread, (LPVOID) extradata, 0, &AcceptConnectionThreadID)) == NULL)
	{
		ErrorMsg("CreateThread() for accept connection failed!", hDlg);
		return 0;
	}

	if ((CompleteIOThreadHandle = CreateThread(NULL, 0, CompleteIORequestThread, (LPVOID) extradata, 0, &CompleteIOThreadID)) == NULL)
	{
		ErrorMsg("CreateThread() failed!", hDlg);
		return 0;
	}

	/*free(extradata);
	free(addr_text);
	free(port_text);*/
	return 1;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: AcceptConnectionThread
--
-- DATE: January 14, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: DWORD WINAPI AcceptConnectionThread(LPVOID lpParameter)
--
-- RETURNS: TRUE - when there's a new socket created for a new client
--			FALSE - if any error occurs while creating the new socket
-- NOTES:

----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI AcceptConnectionThread(LPVOID lpParameter)
{
	LPEXTRA_DATA extradata = (LPEXTRA_DATA) lpParameter;

	SetDlgItemText(extradata->hDlg, IDC_STATSSCREEN, "Listening for clients...\r\n");
	while(TRUE)
	{
		extradata->acceptsocket = accept(extradata->listensocket, NULL, NULL);

		if (WSASetEvent(extradata->wsaEvent) == FALSE)
		{
			ErrorMsg("WSASetEvent() failed!", extradata->hDlg);
			return FALSE;
		}
	}

	SetDlgItemText(extradata->hDlg, IDC_STATSSCREEN, "Client request received...\r\n");

	return TRUE;
}

DWORD WINAPI CompleteIORequestThread(LPVOID lpParameter)
{
	DWORD flags;
	LPSOCKET_INFORMATION SocketInfo;
	WSAEVENT EventArray[1];
	DWORD index;
	DWORD recvbytes;

	LPWSAOVERLAPPED overlapped;
	overlapped = (LPWSAOVERLAPPED) malloc(sizeof(WSAOVERLAPPED));

	LPEXTRA_DATA extradata = (LPEXTRA_DATA) lpParameter;

	EventArray[0] = extradata->wsaEvent;

	while(TRUE)
	{
		// Wait for accept() to signal an event and also process WorkerRoutine() returns.
		while(TRUE)
		{
			index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

			if (index == WSA_WAIT_FAILED)
			{
				ErrorMsg("WSAWaitForMultipleEvents failed!", extradata->hDlg); //TODO: Don't know what will happen if HWND here is NULL
				return FALSE;
			}

			if (index != WAIT_IO_COMPLETION)
			{
				// An accept() call event is ready - break the wait loop
				break;
			}
		}

		WSAResetEvent(EventArray[index - WSA_WAIT_EVENT_0]);
		SocketInfo = (LPSOCKET_INFORMATION) malloc(sizeof(SOCKET_INFORMATION));
		SocketInfo->Socket = extradata->acceptsocket;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->BytesRecvd = 0;
		SocketInfo->BytesSent = 0;
		SocketInfo->DataBuf.len = DATA_BUFSIZE;
		SocketInfo->DataBuf.buf = SocketInfo->Buffer;
		SocketInfo->filename = extradata->filename;
		SocketInfo->hDlg = extradata->hDlg;

		flags = 0;

		overlapped->hEvent = SocketInfo;

		if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &recvbytes, &flags, 
			&(SocketInfo->Overlapped), CompRoutineIORequest) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				ErrorMsg("WSARecv() failed!", extradata->hDlg);
				return FALSE;
			}
		}
	}

	return TRUE;
}

void CALLBACK CompRoutineIORequest(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD sendbytes, recvbytes;
	DWORD flags;

	static DWORD dwTotalTransferred;
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;
	ostringstream buffer, stats;

   if (Error != 0)
   {
		printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0)
   {
      printf("Closing socket %d\n", SI->Socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      closesocket(SI->Socket);
      free(SI);
      return;
   }


   SetDlgItemText(SI->hDlg, IDC_STATSSCREEN, "Client connected\r\n");

   // Check to see if the BytesRECV field equals zero. If this is so, then
   // this means a WSARecv call just completed so update the BytesRECV field
   // with the BytesTransferred value from the completed WSARecv() call.

   if (SI->BytesRecvd == 0)
   {
      SI->BytesRecvd = BytesTransferred;
	  SI->BytesSent = 0;

	  
   }
   else
   {
	   SI->BytesSent += BytesTransferred;
   }

   //write to the output file
   ofstream savefile(SI->filename, ofstream::binary);
   buffer << SI->Buffer;
   savefile.write(buffer.str().c_str(), strlen(buffer.str().c_str()));
   savefile.close();

   
   
   if (SI->BytesRecvd > SI->BytesSent)
   {
	   stats << "Bytes received: " << SI->BytesRecvd << "\r\n";
	   SetDlgItemText(SI->hDlg, IDC_STATSSCREEN, stats.str().c_str());

      // Post another WSASend() request.
      // Since WSASend() is not gauranteed to send all of the bytes requested,
      // continue posting WSASend() calls until all received bytes are sent.

      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

	  SI->DataBuf.buf = SI->Buffer + SI->BytesSent;
	  SI->DataBuf.len = SI->BytesRecvd - SI->BytesSent;

      if (WSASend(SI->Socket, &(SI->DataBuf), 1, &sendbytes, 0,
		  &(SI->Overlapped), CompRoutineIORequest) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
   else
   {
	   stats << "Total bytes received: " << SI->BytesRecvd << "\r\n";
      SetDlgItemText(SI->hDlg, IDC_STATSSCREEN, stats.str().c_str());

      SI->BytesRecvd = 0;

      // Now that there are no more bytes to send post another WSARecv() request.

      flags = 0;
      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.len = DATA_BUFSIZE;
      SI->DataBuf.buf = SI->Buffer;

	  
      if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &recvbytes, &flags,
         &(SI->Overlapped), CompRoutineIORequest) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING )
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
}