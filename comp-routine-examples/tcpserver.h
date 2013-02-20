#include "advancedio_utils.h"

int StartTCPServer(HWND hDlg);

void CALLBACK CompRoutineIORequest(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags);

DWORD WINAPI CompleteIORequestThread(LPVOID lpParameter);

DWORD WINAPI AcceptConnectionThread(LPVOID lpParameter);