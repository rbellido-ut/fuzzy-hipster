#include "winmain.h"

using namespace std;
using namespace libZPlay;

bool createMicSocket();
bool startMicSession();
SOCKET createMulticastSocket();
DWORD WINAPI multicastThread(LPVOID args);
DWORD WINAPI micSessionThread(LPVOID param);
int __stdcall micCallBack (void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);

SOCKET micSocket;
SOCKADDR_IN micServer, micClient;	

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

		case IDC_BUTTON_OK:
			{
				if (IsDlgButtonChecked(hWnd, IDC_RADIO_DOWNLOAD) == BST_CHECKED )
				{
					if (clnt.currentState != WFUCOMMAND)
					{
						MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
					}
					else {
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
						// get user input
						SendMessage(GetDlgItem(hWnd,IDC_EDIT_PORT), WM_GETTEXT,sizeof(szPort)/sizeof(szPort[0]),(LPARAM)szPort);
						SendMessage(GetDlgItem(hWnd,IDC_EDIT_HOSTNAME), WM_GETTEXT,sizeof(szServer)/sizeof(szServer[0]),(LPARAM)szServer);
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
		}
		break;

	case WM_DESTROY:
		{
			PostQuitMessage(0);
			shutdown(Socket,SD_BOTH);
			closesocket(Socket);
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

bool uploadRequest(Client& clnt, HWND hWnd, OPENFILENAME &ofn)
{
	initOpenFileStruct(hWnd, ofn);

	if (GetOpenFileName(&ofn))
	{
		UPLOADCONTEXT *uc = (UPLOADCONTEXT*)malloc(sizeof(UPLOADCONTEXT));
		uc->clnt = &clnt;
		uc->filename =  ofn.lpstrFile;

		//clnt.ulThreadHandle = CreateThread(NULL, 0, clnt.runULThread, uc, 0, &clnt.ulThreadID);

		clnt.ulThreadHandle = CreateThread(NULL, 0, clnt.runULThread, &clnt, 0, &clnt.ulThreadID);
	}

	return true;
}

// get song list from server and populate GUI
// returns false if recv'd list is empty
bool listRequest(Client& clnt, HWND* hWnd)
{
	//clnt.listThreadHandle = CreateThread(NULL, 0, clnt.runListThread, lc, 0, &clnt.listThreadID);

	string userRequest;

	userRequest += "LIST ";
	userRequest += "Behnam's party mix.wav\n";

	clnt.currentState = SENTLISTREQUEST;
	clnt.dispatchOneSend(userRequest);

	while (1)
	{
		if (clnt.currentState != WAITFORLIST)
		{
			// completed op
			if (clnt.currentState == WFUCOMMAND)
			{

				break;
			}

			continue;
		}
		clnt.dispatchOneRecv();
	}

	// get song list
	// remove last EOT char from received song list
	clnt.localSongList = processSongList(clnt.cachedServerSongString.substr(0,clnt.cachedServerSongString.size()-1));

	if (clnt.localSongList.size() == 0) // if recv'd song list is empty..
		return false;
	else 
		populateListBox(hWnd, IDC_SRVSONGLIST, clnt.localSongList); // populate song list on gui
		//populateSongList(&clnt, clnt.cachedServerSongList.substr(0,clnt.cachedServerSongList.size()-1));

	return true;
}

bool streamRequest(Client& clnt)
{
	clnt.stThreadHandle = CreateThread(NULL, 0, clnt.runSTThread, &clnt, 0, &clnt.stThreadID);

	return true;
}

bool micRequest(Client& clnt)
{
	MessageBox(NULL, "mic req" , "Test" , MB_OK);

	std::string userRequest;

	userRequest += "MIC";

	clnt.currentState = MICROPHONE;
	clnt.dispatchOneSend(userRequest);

	HANDLE hMicSessionThread = CreateThread(NULL, 0, micSessionThread, &clnt, 0, NULL);

	return true;
}

DWORD WINAPI micSessionThread(LPVOID param)
{
	Client* clnt = (Client*) param;

	if(!createMicSocket())
	{
		clnt->currentState = WFUCOMMAND;
		return 1;
	}

	startMicSession();

	clnt->currentState = WFUCOMMAND;

	return 0;
}

bool startMicSession()
{
	ZPlay * netplay = CreateZPlay();

	netplay->SetSettings(sidSamplerate, 44100);// 44100 samples
	netplay->SetSettings(sidChannelNumber, 2);// 2 channel
	netplay->SetSettings(sidBitPerSample, 16);// 16 bit
	netplay->SetSettings(sidBigEndian, 1); // little endian
	int i;

	int result = netplay->OpenStream(1, 1, &i, 1, sfPCM); 
	if(result == 0) {
		printf("Error: %s\n", netplay->GetError());
		netplay->Release();
		return false;
	}


	ZPlay *player = CreateZPlay();

	result = player->OpenFile("wavein://", sfAutodetect);
	if(result == 0) {
		printf("Error: %s\n", player->GetError());
		player->Release();
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
				printf("Concoction recent by Pier.\n"); 
			else printf("get last error %d\n", err);
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
		exit(1);
	}


	return true;
}


bool castRequest(Client& clnt)
{
	MessageBox(NULL, "multicast req" , "Test" , MB_OK);
	CreateThread(NULL, 0, multicastThread, NULL, NULL, NULL);
	return true;
}

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

DWORD WINAPI multicastThread(LPVOID args)
{
	SOCKET			hSocket;
	int				nRet;
	struct ip_mreq	stMreq;
	SOCKADDR_IN		server;
	char			achMCAddr[MAXADDRSTR] = TIMECAST_ADDR;
	char			achInBuf[BUFSIZE];
	u_long			lMCAddr;
	u_short			nPort = TIMECAST_PORT;
	ZPlay *			netplay = CreateZPlay();

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

	netplay->Play();

	// Read continuously, play data
	while (TRUE)
	{
		char *tmp = new char[DATABUFSIZE];
		int addr_size = sizeof(struct sockaddr_in);

		nRet = recvfrom(hSocket, tmp, DATABUFSIZE, 0, (struct sockaddr*)&server, &addr_size);
		if (nRet < 0)
		{
			MessageBox(NULL, "recvfrom() failed", "Error", MB_OK);
			WSACleanup();
			return 1;
		}

		netplay->PushDataToStream(tmp, nRet);

		delete tmp;

		if (nRet == 0)
			break;
	}

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

// create all main window gui elements
// move all createwindow stuff here eventually
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
	SendMessage(
		CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD|WS_VISIBLE|LBS_STANDARD|LBS_HASSTRINGS,
		10, 100, 380, 260, hWnd, (HMENU)IDC_SRVSONGLIST, GetModuleHandle(NULL), NULL)
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
		150*4+50, 360, 100, 23, hWnd, (HMENU)IDC_BUTTON_OK, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_OK), FALSE); 

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

// args: takes a new line separated string of songs available on the server
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


bool populateListBox(HWND* hWnd, int resIdxOfListBox, vector<string> localList)
{
	// clear list box
	SendMessage (GetDlgItem(*hWnd,resIdxOfListBox),LB_RESETCONTENT,0,0);

	// populate list box
	for (vector<string>::iterator it=localList.begin(); it!=localList.end(); ++it)
	{
		string s = *it;
		SendMessage (GetDlgItem(*hWnd,resIdxOfListBox),LB_INSERTSTRING,0,(LPARAM)s.c_str());

	}

	return true;
}

int __stdcall micCallBack (void* instance, void *user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
{

	if ( message == MsgStop )
		return closesocket(micSocket);

	if (sendto(micSocket, (const char *)param1, param2, 0, (const struct sockaddr*)&micServer, sizeof(micServer)) < 0)
		return 2;

	return 1;

}