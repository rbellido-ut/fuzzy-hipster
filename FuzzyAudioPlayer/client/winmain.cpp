/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		winmain.cpp -  Entry point for the client GUI
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
-- DATE:			April 3, 2013
--
-- REVISIONS: 
--
-- DESIGNER:		Aaron Lee
--
-- PROGRAMMER:		Aaron Lee
--
-- NOTES: 			requires libzplay library
----------------------------------------------------------------------------------------------------------------------*/

#include "winmain.h"

using namespace std;
using namespace libZPlay;

bool createMicSocket();
bool startMicSession();
SOCKET createMulticastSocket();
DWORD WINAPI multicastThread(LPVOID args);
DWORD WINAPI listThreadProc(LPVOID args);
DWORD WINAPI micSessionThread(LPVOID param);
int __stdcall micCallBack (void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);

SOCKET micSocket;
SOCKADDR_IN micServer, micClient;	
ZPlay * player;
ZPlay * netplay;


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: December 7, 2012
--
-- REVISIONS: December 7, 2012 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
--				HINSTANCE hInstance: handle to our application instance
--				HINSTANCE hPrevInstance: handle to previous application instance
--				PSTR szCmdLine: arguments passed in before execution
--				int iCmdShow: window state
--
-- RETURNS: int -- error code; 0 indicates no error
--
-- NOTES:
-- Main entry point. Message loop is here as well.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
{
	WNDCLASSEX wClass;
	ZeroMemory(&wClass,sizeof(WNDCLASSEX));
	wClass.cbClsExtra=NULL;
	wClass.cbSize=sizeof(WNDCLASSEX);
	wClass.cbWndExtra=NULL;
	wClass.hbrBackground=(HBRUSH)COLOR_WINDOW;
	wClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wClass.hIcon=NULL;
	wClass.hIconSm=NULL;
	wClass.hInstance=hInst;
	wClass.lpfnWndProc=(WNDPROC)WinProc;
	wClass.lpszClassName="Window Class";
	wClass.lpszMenuName=MAKEINTRESOURCE(IDR_MENU1) ;
	wClass.style=CS_HREDRAW|CS_VREDRAW;

	if (!RegisterClassEx(&wClass))
	{
		int nResult = GetLastError();
		MessageBox(NULL,
			"Window class creation failed\r\nError code:",
			"Window Class Failed",
			MB_ICONERROR);
	}

	HWND hWnd=CreateWindowEx(NULL, "Window Class", "FuzzyPlayer Client", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInst, NULL);

	if (!hWnd)
	{
		int nResult = GetLastError();

		MessageBox(NULL,
			"Window creation failed\r\nError code:",
			"Window Creation Failed",
			MB_ICONERROR);
	}

	ShowWindow(hWnd,nShowCmd);

	MSG msg;
	ZeroMemory(&msg,sizeof(MSG));

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinProc
--
-- DATE: February 15, 2013
--
-- REVISIONS: February 15, 2013 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: LRESULT CALLBACK WinProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
--							HWND hwnd: The handle to the window.
--							UINT Message: The message received.
--							WPARAM wParam: Additional information.
--							LPARAM lParam: Additional information.
--
-- RETURNS: LRESULT
--
-- NOTES:
-- Program message handler. Interprets messages from WSAasyncSelect and handles them.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WinProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static OPENFILENAME ofn = {0};
	static HANDLE hFile;
	HMENU hMenu;

	static bool haveClient;
	static Client clnt;
	static int currentUserChoice;
	static string userRequest;

	switch(msg)
	{

	case WM_CREATE:
		{
			// draw gui
			if (!createGUI(hWnd))
				PostQuitMessage(1);

			haveClient = false;
		}
		break;

	case WM_COMMAND:
		hMenu = GetMenu (hWnd) ;

		switch(LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			{
				// graceful quit
				PostQuitMessage(0);
			}
			break;

		case ID_FILE_CONNECT:
			{
				if (!haveClient)
				{
					// get user input
					SendMessage(GetDlgItem(hWnd,IDC_EDIT_PORT), WM_GETTEXT,sizeof(szPort)/sizeof(szPort[0]),(LPARAM)szPort);
					SendMessage(GetDlgItem(hWnd,IDC_EDIT_HOSTNAME), WM_GETTEXT,sizeof(szServer)/sizeof(szServer[0]),(LPARAM)szServer);

					WSADATA wsaData;
					if (clnt.runClient(&wsaData, szServer, atoi(szPort)))
					{
						SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_STATUS, (LPARAM)"Connected");
						EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_OK), TRUE); 
						EnableWindow(GetDlgItem(hWnd, IDC_EDIT_PORT), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_EDIT_HOSTNAME), FALSE);
						haveClient = true;

						listRequest(clnt,&hWnd);

					}
					else
						MessageBox(hWnd, "Try Again!" , "Sorry" , MB_ICONWARNING);
				}
				else
				{
					MessageBox(hWnd, "Already connected as a client!" , "Warning" , MB_ICONWARNING);
				}

			}
			break;

		case ID_FILE_DISCONNECT:
			{
				//delete clnt;
			}
			break;
		case ID_HELP_ABOUT:
			{
				MessageBox(hWnd,"You","About",MB_ICONINFORMATION|MB_OK);
			}
			break;

		case IDC_BUTTON_OK:
			{
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_DOWNLOAD) == BST_CHECKED )
				{
					if (clnt.currentState != WFUCOMMAND)
					{
						MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
					}
					else {
						clnt.currentSongFile = getSelectedListBoxItem(&hWnd,IDC_SRVSONGLIST);

						if (clnt.currentSongFile != "ERROR")
							downloadRequest(clnt);
					}
					break;
				}
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_UPLOAD) == BST_CHECKED)
				{
					if (clnt.currentState != WFUCOMMAND)
					{
						MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
					}
					else {
						uploadRequest(clnt, hWnd, ofn);
						
						DWORD result = WaitForSingleObject(clnt.ulThreadHandle, INFINITE);
						if (result == WAIT_OBJECT_0)
							listRequest(clnt,&hWnd);
					}
					break;
				}
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_STREAM) == BST_CHECKED)
				{
					if (clnt.currentState != WFUCOMMAND)
					{
						MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
					}
					else {
						clnt.currentSongFile = getSelectedListBoxItem(&hWnd,IDC_SRVSONGLIST);
						streamRequest(clnt);
					}
					break;
				}
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_MIC) == BST_CHECKED)
				{
					if (clnt.currentState != WFUCOMMAND)
					{
						MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
					}
					else {
						micRequest(clnt);
					}
					break;
				}
				else if (IsDlgButtonChecked(hWnd, IDC_RADIO_MULTICAST) == BST_CHECKED)
				{
					if (clnt.currentState != WFUCOMMAND)
					{
						MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
					}
					else {
						castRequest(clnt);
					}
					break;
				}
			}
			break;

		case IDC_BUTTON_CANCEL:
			{
				//closesocket(micSocket);
				if ((netplay != NULL) || (player != NULL))
				{
					if (clnt.currentState == MICROPHONE) {
						sendto(micSocket, 0, 0, 0, (const sockaddr* )&micServer, sizeof(sockaddr_in));
						closesocket(micSocket);

					}

					netplay->Stop();
					//player->Stop();
				}

				closesocket(multicastsocket);
				closesocket(clnt.connectSocket_);
				// return to idle state at earliest convenience
				clnt.currentState = NOTCONNECTED;
				clnt.player_->Stop();

				// get user input
				SendMessage(GetDlgItem(hWnd,IDC_EDIT_PORT), WM_GETTEXT,sizeof(szPort)/sizeof(szPort[0]),(LPARAM)szPort);
				SendMessage(GetDlgItem(hWnd,IDC_EDIT_HOSTNAME), WM_GETTEXT,sizeof(szServer)/sizeof(szServer[0]),(LPARAM)szServer);

				WSADATA wsaData;
				if (clnt.runClient(&wsaData, szServer, atoi(szPort)))
				{
					SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_STATUS, (LPARAM)"Connected");
					EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_OK), TRUE); 
					EnableWindow(GetDlgItem(hWnd, IDC_EDIT_PORT), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_EDIT_HOSTNAME), FALSE);
					haveClient = true;

					listRequest(clnt,&hWnd);

				}
				else
					MessageBox(hWnd, "Try Again!" , "Sorry" , MB_ICONWARNING);
			}
			break;
		}
		break;

	case WM_DESTROY:
		{
			PostQuitMessage(0);
			//shutdown(Socket,SD_BOTH);
			//closesocket(Socket);
			WSACleanup();
			return 0;
		}
		break;

	case WM_SIZE:
		{
			// dynamically resize status bar
			SendMessage(GetDlgItem(hWnd, IDC_MAIN_STATUS), WM_SIZE, 0, 0);
		}
		break;
	}

	return DefWindowProc(hWnd,msg,wParam,lParam);
}

bool downloadRequest(Client &clnt)
{
	clnt.dlThreadHandle = CreateThread(NULL, 0, clnt.runDLThread, &clnt, 0, &clnt.dlThreadID);

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getFileNameWithoutPath
--
-- DATE: April 3, 2013
--
-- REVISIONS: April 3, 2013 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: std::string getFileNameWithoutPath(std::string f)
--							std::string f: absolute path including filename, extension
--
-- RETURNS: std::string -- filename and extension without path
--
-- NOTES:
-- Removes file path from a filepath. I.e.c:/this/is/a/test.wav => test.wav
----------------------------------------------------------------------------------------------------------------------*/
std::string getFileNameWithoutPath(std::string f)
{
	int i = f.find_last_of('\\');
	if (i != std::string::npos)
		f = f.substr(i+1); // f contains the result :)

	return f;
}

bool uploadRequest(Client& clnt, HWND hWnd, OPENFILENAME &ofn)
{
	initOpenFileStruct(hWnd, ofn);

	if (GetOpenFileName(&ofn))
	{
		
		clnt.currentSongFile = getFileNameWithoutPath(ofn.lpstrFile);

		clnt.ulThreadHandle = CreateThread(NULL, 0, clnt.runULThread, &clnt, 0, &clnt.ulThreadID);
	}

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	listThreadProc
--
-- DATE:		March 30, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bestami
--
-- PROGRAMMER:	Aaron Lee
--
-- INTERFACE:	DWORD WINAPI listThreadProc (LPVOID param)
--				LPVOID param -- points to UPLOADCONTEXT struct
--
-- RETURNS:		NULL
--
-- NOTES:		Thread to get song list from server then populate list box on GUI.
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI listThreadProc (LPVOID param)
{
	
	UPLOADCONTEXT* uc = (UPLOADCONTEXT*) param;
	Client * clnt = uc->clnt;
	string userReq = uc->userReq;
	
	clnt->currentState = SENTLISTREQUEST;
	clnt->dispatchOneSend(userReq);

	while (1)
	{
		if (clnt->currentState != WAITFORLIST)
		{
			// completed op
			if (clnt->currentState == WFUCOMMAND)
			{

				break;
			}

			continue;
		}
		clnt->dispatchOneRecv();
	}

	// get song list
	// remove last EOT char from received song list
	clnt->localSongList.erase( clnt->localSongList.begin(), clnt->localSongList.end() );
		clnt->localSongList = processSongList(clnt->cachedServerSongString.substr(0,clnt->cachedServerSongString.size()-1));
	clnt->cachedServerSongString.clear();
	if (clnt->localSongList.size() == 0) // if recv'd song list is empty..
		return false;
	else  {
		populateListBox(hSrvList, clnt->localSongList); // populate song list on gui
		//populateSongList(&clnt, clnt.cachedServerSongList.substr(0,clnt.cachedServerSongList.size()-1));
	}
	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	listRequest
--
-- DATE:		April 3, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bestami
--
-- PROGRAMMER:	Behnam Bestami
--
-- INTERFACE:	bool listRequest(Client& clnt, HWND* hWnd)
--					
--
-- RETURNS:		returns false if recv'd list is empty
--
-- NOTES:		get song list from server and populate GUI. Entry point to thread.
--
----------------------------------------------------------------------------------------------------------------------*/
bool listRequest(Client& clnt, HWND* hWnd)
{
	string userRequest("LIST");

	UPLOADCONTEXT* uc = new UPLOADCONTEXT;
	uc->clnt = &clnt;
	uc->userReq = userRequest;

	HANDLE listThreadHandle = CreateThread(NULL, 0, listThreadProc, uc, 0, NULL);

	return true;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	streamRequest
--
-- DATE:		March 30, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	bool streamRequest(Client& clnt)
--				clnt: the client object that needs to connect to microphone
--
-- RETURNS:		true on success and false on failure
--				
--
-- NOTES:		This function is called by runClient, to create a TCP async socket
--
----------------------------------------------------------------------------------------------------------------------*/
bool streamRequest(Client& clnt)
{
	clnt.stThreadHandle = CreateThread(NULL, 0, clnt.runSTThread, &clnt, 0, &clnt.stThreadID);

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	micRequest
--
-- DATE:		March 30, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	bool micRequest(Client& clnt)
--				clnt: the client object that needs to connect to microphone
--
-- RETURNS:		true on success and false on failure
--				
--
-- NOTES:		This function is called by runClient, to create a TCP async socket
--
----------------------------------------------------------------------------------------------------------------------*/
bool micRequest(Client& clnt)
{

	//std::string userRequest;

	std::string userRequest("MIC");

	UPLOADCONTEXT* uc = new UPLOADCONTEXT;
	uc->clnt = &clnt;
	uc->userReq = userRequest;

	HANDLE hMicSessionThread = CreateThread(NULL, 0, micSessionThread, uc, 0, NULL);

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	micSessionThread
--
-- DATE:		March 30, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	DWORD WINAPI micSessionThread(LPVOID param)
--				clnt: the client object passed to the thread proc
--
-- RETURNS:		0 on success
--				
--
-- NOTES:		This is the thread proc that gets called when a micSessionThread is created
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI micSessionThread(LPVOID param)
{

	UPLOADCONTEXT* uc = (UPLOADCONTEXT*) param;
	Client * clnt = uc->clnt;
	string userReq = uc->userReq;
	
	clnt->currentState = MICROPHONE;
	clnt->dispatchOneSend(userReq);

	if(!createMicSocket())
	{
		clnt->currentState = WFUCOMMAND;
		return 1;
	}

	startMicSession();

	clnt->currentState = WFUCOMMAND;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	startMicSession
--
-- DATE:		March 30, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	bool startMicSession()
--				
--
-- RETURNS:		true on success, false on failure
--				
--
-- NOTES:		This is function creates two libZPlater objects and uses them to send microphone data back and 
--			forward between the client and the server
--
----------------------------------------------------------------------------------------------------------------------*/
bool startMicSession()
{
	netplay = CreateZPlay();

	netplay->SetSettings(sidSamplerate, 44100);// 44100 samples
	netplay->SetSettings(sidChannelNumber, 2);// 2 channel
	netplay->SetSettings(sidBitPerSample, 16);// 16 bit
	netplay->SetSettings(sidBigEndian, 1); // little endian
	int i;

	int result = netplay->OpenStream(1, 1, &i, 1, sfPCM); 
	if(result == 0) {
		printf("Error: %s\n", netplay->GetError());
		netplay->Release();
		closesocket(micSocket);
		return false;
	}


	player = CreateZPlay();

	result = player->OpenFile("wavein://", sfAutodetect);
	if(result == 0) {
		printf("Error: %s\n", player->GetError());
		player->Release();
		closesocket(micSocket);
		return false;
	}

	player->SetCallbackFunc(micCallBack, (TCallbackMessage)(MsgWaveBuffer|MsgStop), NULL);

	player->Play();

	while(1) {
		char * buf = new char[65507];

		int size = sizeof(micServer);
		int r = recvfrom (micSocket, buf, 65507, 0, (sockaddr*)&micServer, &size);
		if ( r == -1 ) {
			int err = WSAGetLastError();
			if (err == 10054)
				printf("Connection reset by peer.\n"); 
			else printf("get last error %d\n", err);
			closesocket(micSocket);
			break;
		}

		netplay->PushDataToStream(buf, r);
		delete buf;
		netplay->Play();


		TStreamStatus status;
		player->GetStatus(&status);
		if(status.fPlay == 0)
			break; 

		TStreamTime pos;
		player->GetPosition(&pos);
	}

	player->Release();
	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	createMicSocket
--
-- DATE:		March 30, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	bool createMicSocket () 
--				
--
-- RETURNS:		true on success, false on failure
--				
--
-- NOTES:		This function created a UDP socket that gets used by the mictophone session
--
----------------------------------------------------------------------------------------------------------------------*/
bool createMicSocket () {

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,2);
	WSAStartup(wVersionRequested, &wsaData);

	struct hostent  *hp;
	memset((char *)&micServer, 0, sizeof(struct sockaddr_in));
	micServer.sin_family = AF_INET;
	micServer.sin_port = htons(UDPPORT);
	if ((hp = gethostbyname(szServer)) == NULL)
	{
		MessageBox(NULL, "Unknown server address", NULL, NULL);
		closesocket(micSocket);
		return false;
	}

	memcpy((char *)&micServer.sin_addr, hp->h_addr, hp->h_length);

	micSocket = socket(AF_INET, SOCK_DGRAM, 0);

	memset((char *)&micClient, 0, sizeof(micClient));
	micClient.sin_family = AF_INET;
	micClient.sin_port = htons(0);
	micClient.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(micSocket, (struct sockaddr *)&micClient, sizeof(micClient)) == -1) {
		MessageBox(NULL, "Can't bind socket", NULL, NULL);
		closesocket(micSocket);
		exit(1);
	}


	return true;
}


bool castRequest(Client& clnt)
{
	CreateThread(NULL, 0, multicastThread, NULL, NULL, NULL);
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: createMulticastSocket
--
-- DATE: April 2, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Braham
--
-- PROGRAMMER: Jesse Braham
--
-- INTERFACE: SOCKET createMulticastSocket()
--
-- RETURNS: SOCKET
--
-- NOTES:  This function creates a multicast socket and sets up the appropriate structures.  Upon completion,
--			it returns the SOCKET created.
----------------------------------------------------------------------------------------------------------------------*/
SOCKET createMulticastSocket()
{
	SOCKET			hSocket;
	WSADATA			stWSAData;
	SOCKADDR_IN		local_address;
	u_short nPort = TIMECAST_PORT;
	BOOL			fFlag;
	int				nRet, nIP_TTL = 2;	

	nRet = WSAStartup(0x0202, &stWSAData);
	if (nRet)
	{
		MessageBox(NULL, "WSAStartup failed", "Error", MB_OK);
		return NULL;
	}

	hSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		MessageBox(NULL, "socket() failed", "Error", MB_OK);
		WSACleanup();
		return NULL;
	}

	fFlag = TRUE;
	nRet = setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&fFlag, sizeof(fFlag));
	if (nRet == SOCKET_ERROR)
	{
		MessageBox(NULL, "setsockopt() SO_REUSEADDR failed", "Error", MB_OK);
	}

	local_address.sin_family      = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port        = htons(nPort);

	nRet = bind(hSocket, (struct sockaddr*)&local_address, sizeof(local_address));
	if (nRet == SOCKET_ERROR)
	{
		MessageBox(NULL, "bind() port failed", "Error", MB_OK);
	}

	return hSocket;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: multicastThread
--
-- DATE: April 2, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Braham, Ronald Bellido
--
-- PROGRAMMER: Jesse Braham, Behnam Bastami, Ronald Bellido
--
-- INTERFACE: DWORD WINAPI multicastThread(LPVOID args)
--
-- RETURNS: DWORD
--
-- NOTES:  This function creates a multicast socket, sets up the multicast group, and plays the multicasted data
--			received from the sever.  Upon completion, the multicast group is left, and the socket is closed.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI multicastThread(LPVOID args)
{
	SOCKET			hSocket;
	int				nRet;
	struct ip_mreq	stMreq;
	SOCKADDR_IN		server;
	char			achMCAddr[MAXADDRSTR] = TIMECAST_ADDR;
	u_short			nPort = TIMECAST_PORT;

	ZPlay *			netplay = CreateZPlay();
	netplay->SetSettings(sidSamplerate, 44100);
	netplay->SetSettings(sidChannelNumber, 2);
	netplay->SetSettings(sidBitPerSample, 16);
	netplay->SetSettings(sidBigEndian, 1);

	if ((hSocket = createMulticastSocket()) == NULL)
	{
		MessageBox(NULL, "Could not create UDP socket", "Error", MB_OK);
		return 1;
	}

	stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
	stMreq.imr_interface.s_addr = INADDR_ANY;

	nRet = setsockopt(hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
	if (nRet == SOCKET_ERROR)
	{
		MessageBox(NULL, "setsockopt() IP_ADD_MEMBERSHIP failed", "Error", MB_OK);
		return 1;
	}
	multicastsocket = hSocket;

	string firstframe;
	char buff[DATABUFSIZE];
	int addr_size = sizeof(struct sockaddr_in);

	if (!netplay->OpenStream(1, 1, buff, DATABUFSIZE, sfPCM))
	{
		MessageBox(NULL, "OpenStream() failed", "Error", MB_OK);
		return 1;
	}
	

	netplay->Play();

	// Read continuously, play data
	while (TRUE)
	{
		char *tmp = new char[65507];

		nRet = recvfrom(hSocket, buff, 65507, 0, (struct sockaddr*)&server, &addr_size);
		if (nRet < 0)
		{
			MessageBox(NULL, "Stopped receiving from multicast", "Error", MB_OK);
			WSACleanup();
			return 1;
		}

		if ((nRet % 2) == 0) 
			netplay->PushDataToStream(buff, nRet);

		delete tmp;

		if (nRet == 0)
			break;
	}

	netplay->Release();

	stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
	stMreq.imr_interface.s_addr = INADDR_ANY;

	nRet = setsockopt(hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
	if (nRet == SOCKET_ERROR)
	{
		MessageBox(NULL, "setsockopt() IP_DROP_MEMBERSHIP failed", "Error", MB_OK);
	}

	closesocket(hSocket);

	WSACleanup();

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: createGUI
--
-- DATE: March 27, 2013
--
-- REVISIONS: March 27, 2013 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: bool createGUI(HWND hWnd)
--
-- RETURNS: BOOL -- TRUE no errors during creation
--
-- NOTES:
-- create all main window gui elements.
----------------------------------------------------------------------------------------------------------------------*/
bool createGUI(HWND hWnd)
{
	HANDLE hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	// Setup to create a progress bar for playback, download, upload status
	INITCOMMONCONTROLSEX InitCtrlEx;
	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&InitCtrlEx);

	// progress bar
	SendMessage(
		CreateWindowEx(WS_EX_CLIENTEDGE, PROGRESS_CLASS, "Progress",
		WS_CHILD|WS_VISIBLE|PBS_SMOOTH, 
		0,0, 781, 30, hWnd, NULL, NULL, NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	// static controls
	SendMessage(
		CreateWindowEx(NULL, "Button", "Operations", WS_CHILD|WS_VISIBLE|BS_GROUPBOX,
		10, 360, 760, 80, hWnd, 0, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow(TEXT("STATIC"), TEXT("Hostname"), WS_CHILD | WS_VISIBLE | SS_LEFT, 
		10, 30, 200, 25, hWnd, 0, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow(TEXT("STATIC"), TEXT("Songs available on server"), WS_CHILD | WS_VISIBLE | SS_LEFT, 
		10, 80, 200, 25, hWnd, 0, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow(TEXT("STATIC"), TEXT("Other users on server"), WS_CHILD | WS_VISIBLE | SS_LEFT, 
		420, 80, 160, 25, hWnd, 0, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow(TEXT("STATIC"), TEXT("Port"), WS_CHILD | WS_VISIBLE | SS_LEFT,
		300, 30, 100, 25, hWnd, 0, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	// create song list box
	hSrvList = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD|WS_VISIBLE|LBS_STANDARD|LBS_HASSTRINGS,
		10, 100, 380, 260, hWnd, (HMENU)IDC_SRVSONGLIST, GetModuleHandle(NULL), NULL);
	SendMessage(hSrvList
		
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	//SendMessage(GetDlgItem(hWnd,IDC_SRVSONGLIST),LB_INSERTSTRING,0,(LPARAM)"Test Behnam's party mix");

	// create connected clients list box
	SendMessage(
		CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD|WS_VISIBLE|LBS_STANDARD|LBS_HASSTRINGS,
		400, 100, 370, 260, hWnd, (HMENU)IDC_CLIENTLIST, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(GetDlgItem(hWnd,IDC_CLIENTLIST),LB_INSERTSTRING,0,(LPARAM)"otherusertochatwith69");

	// Create hostname box
	SendMessage(
		CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL,
		10, 50, 280, 25, hWnd, (HMENU)IDC_EDIT_HOSTNAME, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	// set default value
	SendMessage(GetDlgItem(hWnd,IDC_EDIT_HOSTNAME), WM_SETTEXT, NULL, (LPARAM)szServer);
	SendMessage(GetDlgItem(hWnd,IDC_EDIT_HOSTNAME), EM_LIMITTEXT, WPARAM(128), 0);

	// radio buttons
	SendMessage(
		CreateWindowEx(0, "BUTTON", "Upload", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		20, 400, 120, 25, hWnd, (HMENU)IDC_RADIO_UPLOAD, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindowEx(0, "BUTTON", "Download", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		20+150*1, 400, 120, 25, hWnd, (HMENU)IDC_RADIO_DOWNLOAD, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(GetDlgItem(hWnd,IDC_RADIO_DOWNLOAD), BM_SETCHECK, 1, 0);

	SendMessage(
		CreateWindowEx(0, "BUTTON", "Streaming", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		20+150*2, 400, 120, 25, hWnd, (HMENU)IDC_RADIO_STREAM, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindowEx(0, "BUTTON", "Multicast", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON, 
		20+150*3, 400, 100, 25, hWnd, (HMENU)IDC_RADIO_MULTICAST, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindowEx(0, "BUTTON", "Microphone Chat", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		20+140*4, 400, 170, 25, hWnd, (HMENU)IDC_RADIO_MIC, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	// Port box
	SendMessage(
		CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_NUMBER,
		300, 50, 100, 25, hWnd, (HMENU)IDC_EDIT_PORT, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	//SendMessage(hEditPort, WM_SETFONT,(WPARAM)hfDefault, MAKELPARAM(FALSE,0));
	SendMessage(GetDlgItem(hWnd,IDC_EDIT_PORT), WM_SETTEXT, NULL, (LPARAM)szPort);
	SendMessage(GetDlgItem(hWnd,IDC_EDIT_PORT), EM_LIMITTEXT, WPARAM(5), 0);

	// create music control buttons
	SendMessage(
		CreateWindow("BUTTON", "&Rewind", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
		10,	450, 150, 46, hWnd, (HMENU)IDC_BUTTON_REWIND, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow("BUTTON", "&Play", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
		10+150*1+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_PLAY, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow("BUTTON", "P&ause", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
		10+150*2+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_PAUSE, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow("BUTTON", "&Forward", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON,
		10+150*3+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_FORWARD, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(
		CreateWindow("BUTTON", "&Stop", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
		10+150*4+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_STOP, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_STOP), FALSE);

	SendMessage(
		CreateWindow("BUTTON", "&OK", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
		650, 360, 100, 23, hWnd, (HMENU)IDC_BUTTON_OK, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_OK), FALSE); 

	SendMessage(
		CreateWindow("BUTTON", "&Cancel", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
		540, 360, 100, 23, hWnd, (HMENU)IDC_BUTTON_CANCEL, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	// xfer Status bar
	int statwidths[] = {80, 200, 360, 490,590, -1};
	SendMessage(
		CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
	SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_MODE, (LPARAM)"Client");
	SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_TIME, (LPARAM)"0 ms");
	SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_XFRD, (LPARAM)"Sent: 0 b");
	SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_STATUS, (LPARAM)"Not Connected");
	SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_PROTOCOL, (LPARAM)"TCP");

	//currentUserChoice = DOWNLOAD;

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initOpenFileStruct
--
-- DATE: February 13, 2013
--
-- REVISIONS: February 13, 2013 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: int initOpenFileStruct(HWND hWnd, OPENFILENAME &ofn)
--
-- RETURNS: 0
--
-- NOTES:
-- Initializes a OpenFileStruct Dialog for .txt files.
----------------------------------------------------------------------------------------------------------------------*/
int initOpenFileStruct(HWND hWnd, OPENFILENAME &ofn)
{
	static char szFileName[MAX_PATH] = "";

	// initalize open file struct
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Waveform Files (*.wav)\0*.wav\0MP3 Files (*.mp3)\0*.mp3\0Free Lossless Audio (*.flac)\0*.flac\0OGG Vorbis (*.ogg)\0*.ogg\0Advanced Audio (*.aac)\0*.aac\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: processSongList
--
-- DATE: April 3, 2013
--
-- REVISIONS: April 3, 2013 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: vector<string> processSongList(std::string rawstring)
--		std::string -- string of songs separated by new-line chars
--
-- RETURNS: vector<string> -- vector of songs
--
-- NOTES:
-- Converts a string of songs separated by newlines into a vector
----------------------------------------------------------------------------------------------------------------------*/
vector<string> processSongList(std::string rawstring)
{
	vector<string> result;

	std::string songname;
	std::istringstream iss(rawstring);
	while (std::getline(iss, songname))
	{
		result.push_back(songname);
	}

	return result;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: populateListBox
--
-- DATE: April 3, 2013
--
-- REVISIONS: April 3, 2013 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: bool populateListBox(HWND hList, vector<string> localList)
--		HWND hList -- handle to list box
--		vector<string> -- vector of items to load into listbox hList
--
-- RETURNS: TRUE if successful
--
-- NOTES:
-- Clears then loads a vector into a list box window.
----------------------------------------------------------------------------------------------------------------------*/
bool populateListBox(HWND hList, vector<string> localList)
{
	// clear list box
	SendMessage(hSrvList, WM_SETREDRAW, 0, 0);
	SendMessage (hSrvList,LB_RESETCONTENT,0,0);

	// populate list box
	for (vector<string>::iterator it=localList.begin(); it!=localList.end(); ++it)
	{
		string s = *it;
		SendMessage (hSrvList,LB_INSERTSTRING,0,(LPARAM)s.c_str());

	}

	SendMessage(hSrvList, WM_SETREDRAW, 1, 0);

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getSelectedListBoxItem
--
-- DATE: April 3, 2013
--
-- REVISIONS: April 3, 2013 -- Initial version.
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- INTERFACE: std::string getSelectedListBoxItem(HWND* hWnd, int resIdxOfListBox)
--		HWND* hWnd -- pointer to main window
--		int resIdxOfListBox -- resource of list box
--
-- RETURNS: string -- song name of selected item by user
--				"ERROR" if nothing selected by user
--
-- NOTES:
-- Gets the text of the item selected by the user on the list box window.
----------------------------------------------------------------------------------------------------------------------*/
std::string getSelectedListBoxItem(HWND* hWnd, int resIdxOfListBox)
{
	char* tmp = new char[DATABUFSIZE]; // edit box

	// get the number of items in the box.
	int count = SendMessage(GetDlgItem(*hWnd,resIdxOfListBox), LB_GETCOUNT, 0, 0);

	int iSelected = -1;

	// go through the items and find the first selected one
	for (int i = 0; i < count; i++)
	{
		// check if this item is selected or not..
		if (SendMessage(GetDlgItem(*hWnd,resIdxOfListBox), LB_GETSEL, i, 0) > 0)
		{
			// yes, we only want the first selected so break.
			iSelected = i;
			break;
		}
	}

	// get the text of the selected item
	if (iSelected == -1)
	{
		MessageBox(NULL, "No file selected to download", "Error", MB_OK);
		return "ERROR";
	}

	if (iSelected != -1)
		SendMessage(GetDlgItem(*hWnd,resIdxOfListBox), LB_GETTEXT, (WPARAM)iSelected , (LPARAM)tmp);
	//SendMessage(GetDlgItem(hWnd,IDC_EDIT_HOSTNAME), WM_GETTEXT,sizeof(szServer)/sizeof(szServer[0]),(LPARAM)szServer);

	// char to string
	string result(tmp);

	return result;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	micCallBack
--
-- DATE:		March 30, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	int __stdcall micCallBack (void* instance, void *user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
--
-- RETURNS:		
--				
--
-- NOTES:		This is the callback function that gets called whenever the microphone has data to process
--
----------------------------------------------------------------------------------------------------------------------*/
int __stdcall micCallBack (void* instance, void *user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
{

	if ( message == MsgStop )
	{
		netplay->Stop();
		player->Stop();
		closesocket(micSocket);
		return 2;
	}

	if (sendto(micSocket, (const char *)param1, param2, 0, (const struct sockaddr*)&micServer, sizeof(micServer)) < 0)
		return 2;

	return 1;

}
