/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: winmain.cpp - Entry point for an application is designed to demonstrate
-- UDP, TCP file transfers with Windows Sockets. Packet sizes, repetition attributes 
-- can be changed.
--
-- PROGRAM: Win Sockets
--
-- FUNCTIONS:
-- int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
-- LRESULT CALLBACK WinProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
-- BOOL initTP()
-- int Connect(HWND hWnd) 
-- int Listen(HWND hWnd)
-- int initOpenFileStruct(HWND hWnd, OPENFILENAME &ofn) 
-- long delay (SYSTEMTIME t1, SYSTEMTIME t2)

-- DATE: February 18, 2013
--
-- REVISIONS: February 18, 2013 - Initial version
--
-- DESIGNER: Aaron Lee
--
-- PROGRAMMER: Aaron Lee
--
-- NOTES:
-- Based on TCP, UDP sender, receiver code from Aman Abdulla.
-- Uses WSAasyncSelect for async socket ops.
-- May be buggy with sequential file transfers. Restart program to fix.
----------------------------------------------------------------------------------------------------------------------*/

#include "winmain.h"

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

	HWND hWnd=CreateWindowEx(NULL,
			"Window Class",
			"FuzzyPlayer Client",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			800,
			600,
			NULL,
			NULL,
			hInst,
			NULL);

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
	static int  cxClient,cyClient;
	static OPENFILENAME ofn = {0};
	static HANDLE hFile;
	HMENU hMenu;
	static SYSTEMTIME stStart, stStop;
	DWORD dwNumberOfBytesPerWrite = 1024;

	switch(msg)
	{
		case WM_CREATE:
		{
			ZeroMemory(szHistory,sizeof(szHistory));

			// create button toolbar
			hTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
			hWnd, (HMENU)IDC_MAIN_TOOL, GetModuleHandle(NULL), NULL);
			// Send the TB_BUTTONSTRUCTSIZE message, which is required for
			// backward compatibility.
			SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			TBBUTTON tbb[2];
			TBADDBITMAP tbab;
			tbab.hInst = HINST_COMMCTRL;
			tbab.nID = IDB_STD_SMALL_COLOR;
			SendMessage(hTool, TB_ADDBITMAP, 0, (LPARAM)&tbab);
			ZeroMemory(tbb, sizeof(tbb));
			tbb[0].iBitmap = STD_DELETE;
			tbb[0].fsState = TBSTATE_ENABLED;
			tbb[0].fsStyle = TBSTYLE_BUTTON;
			tbb[0].idCommand = ID_TOOL_ABORT;
			tbb[1].iBitmap = STD_HELP;
			tbb[1].fsState = TBSTATE_ENABLED;
			tbb[1].fsStyle = TBSTYLE_BUTTON;
			tbb[1].idCommand = ID_TOOL_HELP;
			SendMessage(hTool, TB_ADDBUTTONS, sizeof(tbb)/sizeof(TBBUTTON), (LPARAM)&tbb);

			// Create stat message box
			hEditStat=CreateWindowEx(WS_EX_CLIENTEDGE,
				"EDIT",
				"",
				WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_VSCROLL|
				ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_READONLY,
				0,
				80,
				400,
				280,
				hWnd,
				(HMENU)IDC_EDIT_OUT,
				GetModuleHandle(NULL),
				NULL);
			if (!hEditStat)
			{
				MessageBox(hWnd,
					"Could not create incoming edit box.",
					"Error",
					MB_OK|MB_ICONERROR);
			}
			HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEditStat,
				WM_SETFONT,
				(WPARAM)hfDefault,
				MAKELPARAM(FALSE,0));
			strcat(szHistory,"this program doesn't use the above hostname/port inputs yet...");
			SendMessage(hEditStat,
				WM_SETTEXT,
				sizeof(szHistory),
				reinterpret_cast<LPARAM>(&szHistory));
			
			// static controls
			CreateWindowEx(NULL,
						"Button",
						"Operations",
						WS_CHILD|WS_VISIBLE|BS_GROUPBOX,
						10,
						360,
						760,
						80,
						hWnd,
						0,
						GetModuleHandle(NULL),
						NULL);
			CreateWindow(
						TEXT("STATIC"),
						TEXT("Hostname"),
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						0,
						30,
						200,
						25,
						hWnd, 
						0,
						GetModuleHandle(NULL),
						NULL);  
			CreateWindow(
						TEXT("STATIC"),
						TEXT("Port"),
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						300,
						30,
						100,
						25,
						hWnd, 
						0,
						GetModuleHandle(NULL),
						NULL);  

			// Create hostname box
			hEditHostname = CreateWindowEx(WS_EX_CLIENTEDGE,
						"EDIT",
						"",
						WS_CHILD|WS_VISIBLE|ES_MULTILINE|
						ES_AUTOVSCROLL|ES_AUTOHSCROLL,
						0,
						50,
						300,
						25,
						hWnd,
						(HMENU)IDC_EDIT_HOSTNAME,
						GetModuleHandle(NULL),
						NULL);
			if (!hEditHostname)
			{
				MessageBox(hWnd,
					"Could not create outgoing edit box.",
					"Error",
					MB_OK|MB_ICONERROR);
			}

			SendMessage(hEditHostname,
				WM_SETFONT,(WPARAM)hfDefault,
				MAKELPARAM(FALSE,0));
			SendMessage(hEditHostname,
				WM_SETTEXT,
				NULL,
				(LPARAM)szServer);

			// radio buttons
			hRadioUpload = CreateWindowEx(0,
						"BUTTON",
						"Upload",
						WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
						20,
						400,
						120,
						25,
						hWnd,
						(HMENU)IDC_RADIO_UPLOAD,
						GetModuleHandle(NULL),
						NULL);
			hRadioDownload= CreateWindowEx(0,
						"BUTTON",
						"Download",
						WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
						20+150*1,
						400,
						120,
						25,
						hWnd,
						(HMENU)IDC_RADIO_DOWNLOAD,
						GetModuleHandle(NULL),
						NULL);
			hRadioStream= CreateWindowEx(0,
						"BUTTON",
						"Streaming",
						WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
						20+150*2,
						400,
						120,
						25,
						hWnd,
						(HMENU)IDC_RADIO_STREAM,
						GetModuleHandle(NULL),
						NULL);
			hRadioMulticast= CreateWindowEx(0,
						"BUTTON",
						"Multicast",
						WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
						20+150*3,
						400,
						100,
						25,
						hWnd,
						(HMENU)IDC_RADIO_MULTICAST,
						GetModuleHandle(NULL),
						NULL);
			hRadioMic= CreateWindowEx(0,
						"BUTTON",
						"Microphone Chat",
						WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
						20+140*4,
						400,
						170,
						25,
						hWnd,
						(HMENU)IDC_RADIO_MIC,
						GetModuleHandle(NULL),
						NULL);

			// Port box
			hEditPort = CreateWindowEx(WS_EX_CLIENTEDGE,
						"EDIT",
						"",
						WS_CHILD|WS_VISIBLE|ES_MULTILINE|
						ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_NUMBER,
						300,
						50,
						100,
						25,
						hWnd,
						(HMENU)IDC_EDIT_PORT,
						GetModuleHandle(NULL),
						NULL);
			if (!hEditPort)
			{
				MessageBox(hWnd,
					"Could not create outgoing edit box.",
					"Error",
					MB_OK|MB_ICONERROR);
			}
			SendMessage(hEditPort,
				WM_SETFONT,(WPARAM)hfDefault,
				MAKELPARAM(FALSE,0));
			SendMessage(hEditPort,
				WM_SETTEXT,
				NULL,
				(LPARAM)szPort);
			
			// create music control buttons
			HWND hButtonRewind=CreateWindow( 
					    "BUTTON",
						"&Rewind",
						WS_TABSTOP|WS_VISIBLE|
						WS_CHILD|BS_DEFPUSHBUTTON,
						10,	
						450,
						150,
						46,
						hWnd,
						(HMENU)IDC_BUTTON_REWIND,
						GetModuleHandle(NULL),
						NULL);
			SendMessage(hButtonRewind,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			HWND hButtonPlay = CreateWindow( 
					    "BUTTON",
						"&Play",
						WS_TABSTOP|WS_VISIBLE|
						WS_CHILD|BS_DEFPUSHBUTTON,
						10+150*1+10,	
						450,
						150,
						46,
						hWnd,
						(HMENU)IDC_BUTTON_PLAY,
						GetModuleHandle(NULL),
						NULL);
			SendMessage(hButtonPlay,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			HWND hButtonPause = CreateWindow( 
					    "BUTTON",
						"P&ause",
						WS_TABSTOP|WS_VISIBLE|
						WS_CHILD|BS_DEFPUSHBUTTON,
						10+150*2+10,	
						450,
						150,
						46,
						hWnd,
						(HMENU)IDC_BUTTON_PAUSE,
						GetModuleHandle(NULL),
						NULL);
			SendMessage(hButtonPause,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			HWND hButtonForward = CreateWindow( 
					    "BUTTON",
						"&Forward",
						WS_TABSTOP|WS_VISIBLE|
						WS_CHILD|BS_DEFPUSHBUTTON,
						10+150*3+10,	
						450,
						150,
						46,
						hWnd,
						(HMENU)IDC_BUTTON_FORWARD,
						GetModuleHandle(NULL),
						NULL);
			SendMessage(hButtonForward,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			HWND hButtonStop = CreateWindow( 
					    "BUTTON",
						"&Stop",
						WS_TABSTOP|WS_VISIBLE|
						WS_CHILD|BS_DEFPUSHBUTTON,
						10+150*4+10,	
						450,
						150,
						46,
						hWnd,
						(HMENU)IDC_BUTTON_STOP,
						GetModuleHandle(NULL),
						NULL);
			SendMessage(hButtonStop,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
			EnableWindow(hButtonStop, FALSE);

			// xfer Status bar
			int statwidths[] = {80, 200, 360, 490,590, -1};
			hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);

			SendMessage(hStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_MODE, (LPARAM)"Client");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_TIME, (LPARAM)"0 ms");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_XFRD, (LPARAM)"Sent: 0 b");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_LOST, (LPARAM)"Lost: 0 P");
			SendMessage(hStatus, SB_SETTEXT, STATUSBAR_PROTOCOL, (LPARAM)"TCP");
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
					Client clnt;
					WSADATA wsaData;
					clnt.runClient(&wsaData);
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
			hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
			SendMessage(hStatus, WM_SIZE, 0, 0);
		}
		break;
	}

	return DefWindowProc(hWnd,msg,wParam,lParam);
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
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Delay
--
-- DATE: January 6, 2008
--
-- REVISIONS: January 6, 2008 -- Initial version.
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: long delay (SYSTEMTIME t1, SYSTEMTIME t2)
--
-- RETURNS: long -- delay between tl and t2 in milliseconds.
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
long delay (SYSTEMTIME t1, SYSTEMTIME t2)
{
	long d;

	d = (t2.wSecond - t1.wSecond) / 1000;
	d += (t2.wMilliseconds - t1.wMilliseconds);
	return(d);
}