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
	static int  cxClient,cyClient;
	static OPENFILENAME ofn = {0};
	static HANDLE hFile;
	HMENU hMenu;
	static SYSTEMTIME stStart, stStop;
	DWORD dwNumberOfBytesPerWrite = 1024;

	
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
				if(!haveClient)
				{
					haveClient = true;
					WSADATA wsaData;
					if(clnt.runClient(&wsaData)){
						SendMessage(hStatus, SB_SETTEXT, STATUSBAR_STATUS, (LPARAM)"Connected");
						EnableWindow(hButtonOk, TRUE); 
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
			if (IsDlgButtonChecked(hWnd, IDC_RADIO_DOWNLOAD) == BST_CHECKED )
            {
                if(clnt.currentState != WFUCOMMAND)
				{
					MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
				}
				else{
					userRequest += "DL ";
					userRequest += "Behnam's party mix.wav\n";
					
					clnt.sendDLRequest(userRequest);
					
					clnt.currentState = SENTDLREQUEST;
					clnt.dlThreadHandle = CreateThread(NULL, 0, clnt.runDLThread, &clnt, 0, &clnt.dlThreadID);
					
					//postSendRequest
				}
				break;
            }
			/*
			switch(currentUserChoice){
			case IDC_RADIO_DOWNLOAD:
				if(clnt.currentState != WFUCOMMAND)
					MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
				else
					userRequest += "DL ";
					userRequest += "Behnam's party mix.wav\n";
					MessageBox(hWnd, "I am sending your download request" , "YAY" , MB_ICONWARNING);
					
					
					clnt.currentState = SENTDLREQUEST;
					clnt.dlThreadHandle = CreateThread(NULL, 0, clnt.runDLThread, &clnt, 0, &clnt.dlThreadID);
					//postSendRequest
				break;

			case IDC_RADIO_UPLOAD:
				if(clnt.currentState != WFUCOMMAND)
					MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
				else
					//process users download request
					MessageBox(hWnd, "I am sending your upload request" , "YAY" , MB_ICONWARNING);
					//fill data with "DL Song Name.wav\n"
					//postSendRequest
					clnt.currentState = SENTULREQUEST;
				break;

			case IDC_RADIO_STREAM:
				if(clnt.currentState != WFUCOMMAND)
					MessageBox(hWnd, "CAN ONLY DO ONE THING AT A TIME" , "Warning" , MB_ICONWARNING);
				else
					//process users download request
					MessageBox(hWnd, "I am sending your stream request" , "YAY" , MB_ICONWARNING);
					//postSendRequest
					clnt.currentState = STREAMING;
				break;
			
			}
			*/
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
			hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
			SendMessage(hStatus, WM_SIZE, 0, 0);
		}
		break;
	}

	return DefWindowProc(hWnd,msg,wParam,lParam);
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
		0, 30, 200, 25, hWnd, 0, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);
	
	SendMessage(
	CreateWindow(TEXT("STATIC"), TEXT("Port"), WS_CHILD | WS_VISIBLE | SS_LEFT,
		300, 30, 100, 25, hWnd, 0, GetModuleHandle(NULL), NULL)
		,WM_SETFONT, (WPARAM)hFont, TRUE);

			ZeroMemory(szHistory,sizeof(szHistory));

			// Create stat message box
			hEditStat=CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_VSCROLL|ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_READONLY,
						0, 80, 400, 280, hWnd, (HMENU)IDC_EDIT_OUT, GetModuleHandle(NULL), NULL);
			if (!hEditStat)
			{
				//MessageBox(hWnd, "Could not create incoming edit box.", "Error", MB_OK|MB_ICONERROR);
				return false;
			}
			HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEditStat, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
			strcat(szHistory,"this program doesn't use the above hostname/port inputs yet...");
			SendMessage(hEditStat, WM_SETTEXT, sizeof(szHistory), reinterpret_cast<LPARAM>(&szHistory));

			// Create hostname box
			hEditHostname = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL,
				0, 50, 300, 25, hWnd, (HMENU)IDC_EDIT_HOSTNAME, GetModuleHandle(NULL), NULL);
			if (!hEditHostname)
			{
				//MessageBox(hWnd, "Could not create outgoing edit box.", "Error", MB_OK|MB_ICONERROR);
				return false;
			}

			SendMessage(hEditHostname, WM_SETFONT,(WPARAM)hfDefault, MAKELPARAM(FALSE,0));
			SendMessage(hEditHostname, WM_SETTEXT, NULL, (LPARAM)szServer);

			// radio buttons
			hRadioUpload = CreateWindowEx(0, "BUTTON", "Upload", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
				20, 400, 120, 25, hWnd, (HMENU)IDC_RADIO_UPLOAD, GetModuleHandle(NULL), NULL);

			hRadioDownload= CreateWindowEx(0, "BUTTON", "Download", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
				20+150*1, 400, 120, 25, hWnd, (HMENU)IDC_RADIO_DOWNLOAD, GetModuleHandle(NULL), NULL);
			
			hRadioStream= CreateWindowEx(0, "BUTTON", "Streaming", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
				20+150*2, 400, 120, 25, hWnd, (HMENU)IDC_RADIO_STREAM, GetModuleHandle(NULL), NULL);
			
			hRadioMulticast= CreateWindowEx(0, "BUTTON", "Multicast", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON, 
				20+150*3, 400, 100, 25, hWnd, (HMENU)IDC_RADIO_MULTICAST, GetModuleHandle(NULL), NULL);

			hRadioMic= CreateWindowEx(0, "BUTTON", "Microphone Chat", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
				20+140*4, 400, 170, 25, hWnd, (HMENU)IDC_RADIO_MIC, GetModuleHandle(NULL), NULL);

			// Port box
			hEditPort = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_NUMBER,
				300, 50, 100, 25, hWnd, (HMENU)IDC_EDIT_PORT, GetModuleHandle(NULL), NULL);
			if (!hEditPort)
			{
				//MessageBox(hWnd, "Could not create outgoing edit box.", "Error", MB_OK|MB_ICONERROR);
				return false;
			}
			SendMessage(hEditPort, WM_SETFONT,(WPARAM)hfDefault, MAKELPARAM(FALSE,0));
			SendMessage(hEditPort, WM_SETTEXT, NULL, (LPARAM)szPort);

			// create music control buttons
			hButtonRewind=CreateWindow("BUTTON", "&Rewind", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
				10,	450, 150, 46, hWnd, (HMENU)IDC_BUTTON_REWIND, GetModuleHandle(NULL), NULL);
			SendMessage(hButtonRewind,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			
			hButtonPlay = CreateWindow("BUTTON", "&Play", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
				10+150*1+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_PLAY, GetModuleHandle(NULL), NULL);
			SendMessage(hButtonPlay,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			
			hButtonPause = CreateWindow("BUTTON", "P&ause", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
				10+150*2+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_PAUSE, GetModuleHandle(NULL), NULL);
			SendMessage(hButtonPause,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			
			hButtonForward = CreateWindow("BUTTON", "&Forward", WS_TABSTOP|WS_VISIBLE| WS_CHILD|BS_DEFPUSHBUTTON,
				10+150*3+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_FORWARD, GetModuleHandle(NULL), NULL);
			SendMessage(hButtonForward,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			
			hButtonStop = CreateWindow("BUTTON", "&Stop", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
									10+150*4+10, 450, 150, 46, hWnd, (HMENU)IDC_BUTTON_STOP, GetModuleHandle(NULL), NULL);
			SendMessage(hButtonStop,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			EnableWindow(hButtonStop, FALSE);


			hButtonOk = CreateWindow("BUTTON", "&OK", WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
									150*4+50, 360, 100, 23, hWnd, (HMENU)IDC_BUTTON_OK, GetModuleHandle(NULL), NULL);
			SendMessage(hButtonOk,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));

			// xfer Status bar
			int statwidths[] = {80, 200, 360, 490,590, -1};
			hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);

			SendMessage(hStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_MODE, (LPARAM)"Client");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_TIME, (LPARAM)"0 ms");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_XFRD, (LPARAM)"Sent: 0 b");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_STATUS, (LPARAM)"Not Connected");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_PROTOCOL, (LPARAM)"TCP");

			EnableWindow(hButtonOk, FALSE); 
			SendMessage(hRadioDownload, BM_SETCHECK, 1, 0);
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
