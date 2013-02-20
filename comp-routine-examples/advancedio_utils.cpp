/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: advancedio_utils - contains function calls that are used all throughout the tcp and udp source files
--
-- PROGRAM: advanced-io
--
-- FUNCTIONS:
		void ErrorMsg(const char* error_msg, HWND hwnd)
		void InitializeDialogBox(int whatisthis, HWND hDlg)
		OPENFILENAME GetFileToSendFromDialog(HWND hDlg)
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
This source file contains some of common functions and most importantly, the structs and includes that are used 
by the tcp and udp source files.
----------------------------------------------------------------------------------------------------------------------*/

#include "advancedio_utils.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ErrorMsg
--
-- DATE: January 11, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: void ErrorMsg(const char* err_msg, HWND hwnd)
--
-- RETURNS: void.
--
-- NOTES:
Call this function to display an error message (err_msg) in an error message box.
----------------------------------------------------------------------------------------------------------------------*/
void ErrorMsg(const char* error_msg, HWND hwnd)
{
	MessageBox(hwnd, error_msg, "Error", MB_ICONERROR);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: InitializeDialogBox
--
-- DATE: February 12, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: void InitializeDialogBox(int whatisthis, HWND hDlg)
--
-- RETURNS: void
--
-- NOTES:
Identifies the dialog box whether it is a server or a client by naming the window and disabling sections
meant for the identified dialog box.
----------------------------------------------------------------------------------------------------------------------*/
void InitializeDialogBox(int whatisthis, HWND hDlg)
{
	SetDlgItemText(hDlg, IDC_IPHOSTNAME, "localhost");
	SetDlgItemText(hDlg, IDC_PORTNO, "9999");
	SetDlgItemInt(hDlg, IDC_TIMESTOSENDPACKET, 1, FALSE);
	SetDlgItemInt(hDlg, IDC_PACKETSIZE, 1024, FALSE);

	if (whatisthis == IS_UDPCLIENT)
	{
		SetWindowText(hDlg, "UDP Client");
		EnableWindow(GetDlgItem(hDlg, IDC_SERVERFILESTORE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STARTSERVER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_SERVERBROWSEFILESTORE), FALSE);
	}
	else if (whatisthis == IS_UDPSERVER)
	{
		SetWindowText(hDlg, "UDP Server");
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSENDTEXT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSENDFILETEXT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSENDFILEBROWSE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PACKETSIZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TIMESTOSENDPACKET), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSEND), FALSE);
	}
	else if (whatisthis == IS_TCPCLIENT)
	{
		SetWindowText(hDlg, "TCP Client");
		EnableWindow(GetDlgItem(hDlg, IDC_SERVERFILESTORE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STARTSERVER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_SERVERBROWSEFILESTORE), FALSE);
	}
	else if (whatisthis == IS_TCPSERVER)
	{
		SetWindowText(hDlg, "TCP Server");
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSENDTEXT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSENDFILETEXT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSENDFILEBROWSE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PACKETSIZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_TIMESTOSENDPACKET), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLIENTSEND), FALSE);
	}
	else
		ErrorMsg("I have no idea what this is!", hDlg);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: GetFileToSendFromDialog
--
-- DATE: January 10, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: OPENFILENAME GetFileToSendFromDialog(HWND hDlg)
--				hDlg - the handle to the dialog boox
--
-- RETURNS: The OPENFILENAME struct containing the info of the file to send
--
-- NOTES:
Obtains the filename to send by initiating a "Browse Folders" dialog box.
----------------------------------------------------------------------------------------------------------------------*/
OPENFILENAME GetFileToSendFromDialog(HWND hDlg)
{
		char fname[MAX_PATH];
		memset(fname, 0, MAX_PATH);

		OPENFILENAME ofn = {0};

		ofn.hwndOwner = hDlg;
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = fname;
		ofn.nMaxFile = sizeof(fname);
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		GetOpenFileName(&ofn);

		return ofn;
}