#ifndef _WINMAIN_
#define _WINMAIN_

/* Easy styles enabler for msvc */
#ifdef _MSC_VER
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' \
version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

/***********************************
*			IMPORTS
***********************************/
#include "client.h"
#include "utils.h"
#include "resource.h"
#include <Commctrl.h>
#pragma comment(lib,"comctl32.lib")

/***********************************
*			DEFINITIONS
***********************************/
#define IDC_EDIT_HOSTNAME		6901
#define IDC_SRVSONGLIST			6902
#define IDC_BUTTON_REWIND		6903
#define IDC_EDIT_PORT			6905
#define IDC_BUTTON_PLAY			6906
#define IDC_RADIO_UPLOAD		6907
#define IDC_RADIO_DOWNLOAD		6908
#define IDC_RADIO_STREAM		6909
#define IDC_RADIO_MULTICAST		6910
#define IDC_RADIO_MIC			6911
#define IDC_BUTTON_PAUSE		6912
#define IDC_CLIENTLIST			6913
#define IDC_BUTTON_FORWARD		6914
#define IDC_BUTTON_STOP			6915
#define ID_TOOL_HELP			6916
#define ID_TOOL_ABORT			6917
#define IDC_MAIN_STATUS			6918
#define IDC_MAIN_TOOL			6919
#define IDC_BUTTON_CANCEL		6920
#define IDC_BUTTON_OK			6921

#define STATUSBAR_MODE			0
#define STATUSBAR_TIME			1
#define STATUSBAR_XFRD			2
#define STATUSBAR_STATUS		3
#define STATUSBAR_PROTOCOL		4

#define DATA_BUFSIZE			8192
#define MAX_CLIENTS				5

/***********************************
*			GLOBALS
***********************************/
// defaults
char szServer[128] = "localhost"; // edit box
char szPort[5] = "5555"; // edit box
int nRepeat = 1;
HWND hSrvList;

// not needed
SOCKADDR_IN SockAddr;
DWORD BytesRECV=0;
SOCKET multicastsocket = NULL;

/***********************************
*			PROTOTYPES
***********************************/
LRESULT CALLBACK WinProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
bool createGUI(HWND hWnd);
int initOpenFileStruct(HWND, OPENFILENAME &);
long delay (SYSTEMTIME, SYSTEMTIME);
bool downloadRequest(Client&);
bool uploadRequest(Client& clnt, HWND hWnd, OPENFILENAME &ofn);
bool streamRequest(Client&);
bool micRequest(Client&);
bool castRequest(Client&);
bool listRequest(Client&,  HWND*) ;
bool populateListBox(HWND, std::vector<std::string> );
std::vector<std::string> processSongList(std::string );
std::string getSelectedListBoxItem(HWND* , int );
std::string getFileNameWithoutPath(std::string );

#endif