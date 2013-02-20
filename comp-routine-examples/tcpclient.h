#include "advancedio_utils.h"

int StartTCPClient(HWND hDlg);

void CALLBACK SendComplete(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags);

DWORD WINAPI ConnectThread(LPVOID lpParameter);

DWORD WINAPI HandleConnectionsThread(LPVOID lpParameter);