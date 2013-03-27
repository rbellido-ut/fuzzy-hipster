#ifndef _WINMAIN_
#define _WINMAIN_

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
#define IDC_EDIT_OUT			6902
#define IDC_BUTTON_REWIND		6903
#define IDC_EDIT_PORT			6905
#define IDC_BUTTON_PLAY			6906
#define IDC_RADIO_UPLOAD		6907
#define IDC_RADIO_DOWNLOAD		6908
#define IDC_RADIO_STREAM		6909
#define IDC_RADIO_MULTICAST		6910
#define IDC_RADIO_MIC			6911
#define IDC_BUTTON_PAUSE		6912
#define IDC_BUTTON_PLAY			6913
#define IDC_BUTTON_FORWARD		6914
#define IDC_BUTTON_STOP			6915
#define ID_TOOL_HELP			6916
#define ID_TOOL_ABORT			6917
#define IDC_MAIN_STATUS			6918
#define IDC_MAIN_TOOL			6919
#define WM_SOCKET				6920
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
char szServer[255] = "localhost"; // edit box
char szPort[255] = "5555"; // edit box
int nPort = 5555; // internal
int nRepeat = 1;

// controls
HWND hEditStat = NULL;
HWND hEditHostname = NULL;
HWND hEditPort = NULL;
HWND hStatus = NULL;
HWND hTool = NULL;
HWND hRadioUpload = NULL;
HWND hRadioDownload = NULL;
HWND hRadioStream = NULL;
HWND hRadioMulticast = NULL;
HWND hRadioMic = NULL;
HWND hButtonOk = NULL;
HWND hButtonStop = NULL;
HWND hButtonForward = NULL;
HWND hButtonPause = NULL;
HWND hButtonPlay = NULL;
HWND hButtonRewind = NULL;

SOCKET Socket = NULL;
char szHistory[10000];
SOCKADDR_IN SockAddr;
DWORD BytesRECV=0;


/***********************************
*			PROTOTYPES
***********************************/
LRESULT CALLBACK WinProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
bool createGUI(HWND hWnd);
int initOpenFileStruct(HWND, OPENFILENAME &);
long delay (SYSTEMTIME, SYSTEMTIME);

#endif