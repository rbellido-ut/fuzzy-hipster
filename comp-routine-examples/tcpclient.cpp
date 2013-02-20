#include "tcpclient.h"

using namespace std;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: StartTCPClient
--
-- DATE: February 12, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: int StartTCPClient(HWND hDlg)
--
-- RETURNS: 1 - successfully started TCP client
--			0 - unsuccesful
-- NOTES:
This function starts a TCP client. First, it grabs the data set in the menu, then it initializes the connection 
sockets and creates threads that will establish a connection.
----------------------------------------------------------------------------------------------------------------------*/
int StartTCPClient(HWND hDlg)
{
	DWORD res;
	SOCKET Connect;
	SOCKADDR_IN InternetAddr;
	PHOSTENT pHostent;
	WSADATA wsaData;
	HANDLE ConnectThreadHandle, ConnectionsHandle;
	DWORD ConnectThreadID, HandleConnectsThreadID;
	LPEXTRA_DATA extradata = (LPEXTRA_DATA) malloc(sizeof(EXTRA_DATA));

	//Grab data from text boxes
	char *addr_text, *port_text, *savefile;
	int input_length;

	input_length = GetWindowTextLength(GetDlgItem(hDlg, IDC_IPHOSTNAME)) + 1;
	addr_text = (char *) malloc(sizeof(char) * input_length);
	GetDlgItemText(hDlg, IDC_IPHOSTNAME, addr_text, input_length);

	input_length = GetWindowTextLength(GetDlgItem(hDlg, IDC_PORTNO)) + 1;
	port_text = (char *) malloc(sizeof(char) * input_length);
	GetDlgItemText(hDlg, IDC_PORTNO, port_text, input_length);

	input_length = GetWindowTextLength(GetDlgItem(hDlg, IDC_CLIENTSENDFILETEXT)) + 1;
	savefile = (char *) malloc(sizeof(char) * input_length);
	GetDlgItemText(hDlg, IDC_CLIENTSENDFILETEXT, savefile, input_length);
	extradata->filename = savefile;
	//finished grabbing data from text boxes

	extradata->hDlg = hDlg;

	if ((res = WSAStartup(0x0202, &wsaData)) != 0)
	{
		ErrorMsg("WSAStartup failed!", hDlg);
		return 0;
	}

	if ((Connect = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		ErrorMsg("WSASocket() failed!", hDlg);
		return 0;
	}


	memset((char*) &InternetAddr, 0, sizeof(SOCKADDR_IN));
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_port = htons(atoi(port_text));
	if ((pHostent = gethostbyname(addr_text)) == NULL)
	{
		ErrorMsg("Unknown Server Address!", hDlg);
		return 0;
	}
	InternetAddr.sin_addr.s_addr = *(unsigned long*) pHostent->h_addr;

	if ((extradata->wsaEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		ErrorMsg("WSACreateEvent() failed!", hDlg);
		return 0;
	}

	extradata->connectsocket = Connect;
	extradata->addrin = InternetAddr;

	if ((ConnectThreadHandle = CreateThread(NULL, 0, ConnectThread, (LPVOID) extradata, 0, &ConnectThreadID)) == NULL)
	{
		ErrorMsg("CreateThread() failed!", hDlg);
		return 0;
	}

	if ((ConnectionsHandle = CreateThread(NULL, 0, HandleConnectionsThread, (LPVOID) extradata, 0, &HandleConnectsThreadID)) == NULL)
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
-- FUNCTION: ConnectThread
--
-- DATE: February 13, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: DWORD WINAPI ConnectThread(LPVOID lpParameter)
--
-- RETURNS: TRUE - if a connection was established succesfully,
--			FALSE - if unsuccesful
--
-- NOTES:
Thread that establishes a connection to a server
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ConnectThread(LPVOID lpParameter)
{
	LPEXTRA_DATA extradata = (LPEXTRA_DATA) lpParameter;

	while(TRUE)
	{
		WSAConnect(extradata->connectsocket, (SOCKADDR*) &extradata->addrin, sizeof(SOCKADDR), NULL, NULL, NULL, NULL);

		if (WSASetEvent(extradata->wsaEvent) == FALSE)
		{
			ErrorMsg("WSASetEvent failed!", extradata->hDlg);
			return FALSE;
		}
		break;
	}

	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: HandleConnectionsThread
--
-- DATE: February 13, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: DWORD WINAPI HandleConnectionsThread(LPVOID lpParameter)
--
-- RETURNS: TRUE - if 
--			FALSE
-- NOTES:
Thread that processes packets to send from a file. It waits for a succesful connection to happen and
then it grabs the send parameters from the GUI. Finally, it packetizes data grabbed from the file and 
sends it to the server.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI HandleConnectionsThread(LPVOID lpParameter)
{
	DWORD flags = 0;
	LPSOCKET_INFORMATION SocketInfo;
	WSAEVENT EventArray[1];
	DWORD index;
	DWORD sendbytes;

	int filesize;
	char* sendbuffer;
	ostringstream stats;

	LPEXTRA_DATA extradata = (LPEXTRA_DATA) lpParameter;

	//Get send parameters from dialog boxes
	BOOL ret;
	UINT timestosendpacket = GetDlgItemInt(extradata->hDlg, IDC_TIMESTOSENDPACKET, &ret, FALSE);
	if (!ret) 
		timestosendpacket = 1; //set to default value of 1 in case something weird happens in 
	UINT packetsize = GetDlgItemInt(extradata->hDlg, IDC_PACKETSIZE, &ret, FALSE);
	if (!ret)
		packetsize = 1024; //set to default values in case something weird happens in GetDlgItemInt

	EventArray[0] = extradata->wsaEvent;

	SocketInfo = (LPSOCKET_INFORMATION) malloc(sizeof(SOCKET_INFORMATION));
	SocketInfo->BytesSent = 0;

	while(TRUE)
	{
		// Wait for connect() to signal an event and also process SendComplete() returns
		while(TRUE)
		{
			index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

			if (index == WSA_WAIT_FAILED)
			{
				ErrorMsg("WSAWaitForMultipleEvents failed!", extradata->hDlg);
				return FALSE;
			}

			if (index != WAIT_IO_COMPLETION)
			{
				// An call event is ready - break the wait loop
				stats << "Connected!\r\n";
				SetDlgItemText(extradata->hDlg, IDC_STATSSCREEN, stats.str().c_str());
				break;
			}
		}
		WSAResetEvent(EventArray[index - WSA_WAIT_EVENT_0]);

		//Prepare for send!
		ifstream file;
		extradata->filename[strlen(extradata->filename) - 1] = '\0';
		file.open(extradata->filename);
		file.seekg(0, ios::end);
		filesize = file.tellg();
		file.seekg(0, ios::beg);
		sendbuffer = new char[filesize];

		stats << "File Size: " << filesize << " (in bytes)\r\n";
		SetDlgItemText(extradata->hDlg, IDC_STATSSCREEN, stats.str().c_str());

		//Set all the SocketInfo parameters
		SocketInfo->Socket = extradata->connectsocket;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->BytesRecvd = 0;

		for (size_t i = 0; i < timestosendpacket; i++)
		{
			file.read(sendbuffer, packetsize);
			strncpy(SocketInfo->Buffer, sendbuffer, packetsize);
			SocketInfo->DataBuf.len = ((filesize < packetsize) ? filesize : packetsize);
			SocketInfo->DataBuf.buf = SocketInfo->Buffer;
		
			while (1)
			{
				if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &sendbytes, flags, 
					&(SocketInfo->Overlapped), SendComplete) == SOCKET_ERROR)
				{
					if ((WSAGetLastError()) != WSA_IO_PENDING)
					{
						ErrorMsg("WSASend() failed!", extradata->hDlg);
						return FALSE;
					}
				}
				SocketInfo->BytesSent += sendbytes;
				stats << "Bytes sent so far: " << SocketInfo->BytesSent << "\r\n";
				SetDlgItemText(extradata->hDlg, IDC_STATSSCREEN, stats.str().c_str());

				if (SocketInfo->BytesSent >= filesize)
					break;

				if (ios::cur >= ios::end)
					break;
				
				file.seekg(SocketInfo->DataBuf.len);
				file.readsome(sendbuffer, SocketInfo->DataBuf.len);
				strncpy(SocketInfo->Buffer, sendbuffer, SocketInfo->DataBuf.len);
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
			}
		}

		stats << "Total bytes sent: " << SocketInfo->BytesSent << "\r\n";
		SetDlgItemText(extradata->hDlg, IDC_STATSSCREEN, stats.str().c_str());
		
		file.close();
	}

	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: SendComplete
--
-- DATE: February 13, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: void CALLBACK SendComplete(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
-- RETURNS: void
--
-- NOTES:
The completion routine that gets called after a connection has been made. All it does is 
shutdown the connection since there's really not much to do after sending data.
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK SendComplete(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

	if (Error == 0)	
		return;

	shutdown(SI->Socket, SD_SEND);
}