#include "winmain.h"

using namespace std;

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

					haveClient = true;
					WSADATA wsaData;
					if (clnt.runClient(&wsaData, szServer, atoi(szPort))) {
						SendMessage(GetDlgItem(hWnd,IDC_MAIN_STATUS), SB_SETTEXT, STATUSBAR_STATUS, (LPARAM)"Connected");
						EnableWindow(GetDlgItem(hWnd,IDC_BUTTON_OK), TRUE); 
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
						uploadRequest(clnt);
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
	std::string userRequest;

	userRequest += "DL ";
	userRequest += "Behnam's party mix.wav\n";
					
	clnt.sendDLRequest(userRequest);
					
	clnt.currentState = SENTDLREQUEST;
	clnt.dlThreadHandle = CreateThread(NULL, 0, clnt.runDLThread, &clnt, 0, &clnt.dlThreadID);
					
	//postSendRequest

	return true;
}

bool uploadRequest(Client& clnt)
{
	MessageBox(NULL, "upload req" , "Test" , MB_OK);
	clnt.currentState = SENTULREQUEST;
	return true;
}

bool streamRequest(Client& clnt)
{
	MessageBox(NULL, "stream req" , "Test" , MB_OK);

	std::string userRequest;

	userRequest += "ST";
					
	clnt.sendDLRequest(userRequest);
					
	clnt.currentState = STREAMING;

	// need to read streamed data received from server into a SFML buffer then play it with the SFML lib
	return true;
}

bool micRequest(Client& clnt)
{
	MessageBox(NULL, "mic req" , "Test" , MB_OK);

	std::string userRequest;

	userRequest += "MIC";
					
	clnt.sendDLRequest(userRequest);
					
	//clnt.currentState = STREAMING;
	return true;
}

bool castRequest(Client& clnt)
{
	MessageBox(NULL, "multicast req" , "Test" , MB_OK);
	return true;
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
	0,0, 783, 30, hWnd, NULL, NULL, NULL)
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
		390, 80, 200, 25, hWnd, 0, GetModuleHandle(NULL), NULL)
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

	SendMessage(GetDlgItem(hWnd,IDC_SRVSONGLIST),LB_INSERTSTRING,0,(LPARAM)"Test Behnam's party mix");

	// create connected clients list box
	SendMessage(
	CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD|WS_VISIBLE|LBS_STANDARD|LBS_HASSTRINGS,
				390, 100, 380, 260, hWnd, (HMENU)IDC_CLIENTLIST, GetModuleHandle(NULL), NULL)
				,WM_SETFONT, (WPARAM)hFont, TRUE);

	SendMessage(GetDlgItem(hWnd,IDC_CLIENTLIST),LB_INSERTSTRING,0,(LPARAM)"otherusertochatwith69");

	// Create hostname box
	SendMessage(
		CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL,
		10, 50, 280, 25, hWnd, (HMENU)IDC_EDIT_HOSTNAME, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	// set default value
	SendMessage(GetDlgItem(hWnd,IDC_EDIT_HOSTNAME), WM_SETTEXT, NULL, (LPARAM)szServer);

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
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";

	return 0;
}
