/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - Source file that sets up the GUI of the advanced-io application
--
-- PROGRAM: advanced-io
--
-- FUNCTIONS:
--		LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
--		BOOL CALLBACK ClientServerDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
--
-- DATE: February 9, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- NOTES:
This program performs simple database lookup calls of a host name, IP address, service name, and a service's port.
----------------------------------------------------------------------------------------------------------------------*/

#include "advancedio_utils.h"
#include "tcpclient.h"
#include "tcpserver.h"
#include "udpclient.h"
#include "udpserver.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: January 8, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)
--
-- RETURNS: LRESULT
--
-- NOTES:
Initates and registers the window class, and runs the message loop.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ClientServerDlgProc
--
-- DATE: February 10, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: BOOL CALLBACK ClientServerDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
--
-- RETURNS: BOOL
--
-- NOTES:
Contains the logic for the GUI components of the Client-Server dialog box
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK ClientServerDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

using namespace std;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("Advanced IO") ;
     HWND         hwnd ;
     MSG          msg ;
     WNDCLASS     wndclass ;

     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0; //This is static
     wndclass.cbWndExtra    = sizeof(TCHAR*) ; //This is an instance
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH) ;
     wndclass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1) ;
     wndclass.lpszClassName = szAppName ;

     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }

     hwnd = CreateWindow (szAppName,                  // window class name
                          TEXT ("Advanced IO"), // window caption
                          WS_OVERLAPPEDWINDOW,        // window style
                          400,              // initial x position
                          400,              // initial y position
                          800,              // initial x size
                          600,              // initial y size
                          NULL,                       // parent window handle
                          NULL,                       // window menu handle
                          hInstance,                  // program instance handle
                          NULL) ;                     // creation parameters

	 ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ;
     
     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ; //Sending it back to O/S
     }
     return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{    
	static HINSTANCE hInstance;

	switch(message)
	{
		case WM_CREATE:
			hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_CLIENT_UDP:
					DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CSDLG), hwnd, ClientServerDlgProc, (LPARAM) IS_UDPCLIENT);
					break;
				case ID_SERVER_UDP:
					DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CSDLG), hwnd, ClientServerDlgProc, (LPARAM) IS_UDPSERVER);
					break;
				case ID_CLIENT_TCP:
					DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CSDLG), hwnd, ClientServerDlgProc, (LPARAM) IS_TCPCLIENT);
					break;
				case ID_SERVER_TCP:
					DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CSDLG), hwnd, ClientServerDlgProc, (LPARAM) IS_TCPSERVER);
					break;

				case ID_FILE_EXIT:
					SendMessage(hwnd, WM_DESTROY, NULL, NULL);
					break;
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

    return DefWindowProc (hwnd, message, wParam, lParam);
}

BOOL CALLBACK ClientServerDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	LPOPENFILENAME filetosend = (LPOPENFILENAME) malloc(sizeof(OPENFILENAME));
	static int dialogtype;
	char *filenamedisplay = (char*) malloc(sizeof(char) * DATA_BUFSIZE);

	switch(message)
	{
		case WM_INITDIALOG:
			dialogtype = (int) lParam;
			InitializeDialogBox(dialogtype, hDlg);
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_CLIENTSENDFILEBROWSE:
					*filetosend = GetFileToSendFromDialog(hDlg);
					_snprintf(filenamedisplay, DATA_BUFSIZE, "%s\n", filetosend->lpstrFile);
					SetDlgItemText(hDlg, IDC_CLIENTSENDFILETEXT, filenamedisplay);
					//TODO: need to send filename to server
					break;

				case IDC_SERVERBROWSEFILESTORE:
					*filetosend = GetFileToSendFromDialog(hDlg);
					_snprintf(filenamedisplay, DATA_BUFSIZE, "%s\n", filetosend->lpstrFile);
					SetDlgItemText(hDlg, IDC_SERVERFILESTORE, filenamedisplay);
					break;

				case IDC_CLIENTSEND:
					StartTCPClient(hDlg);
					break;

				case IDC_STARTSERVER:
					if (dialogtype == IS_TCPSERVER)
					{
						if (StartTCPServer(hDlg) == 0)
						{
							ErrorMsg("Error preparing TCP connection sockets!", hDlg);
							return FALSE;
						}
					}

					break;

				case IDCANCEL:
					free(filetosend);
					free(filenamedisplay);
					WSACleanup();
					EndDialog(hDlg, FALSE);
					break;
			}
			return TRUE;
	}

	WSACleanup();
	return FALSE;
}