/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: server.cpp
--
-- PROGRAM: FuzzyAudioPlayerServer
--
-- FUNCTIONS:
--
--
-- DATE: March 20, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido, Jesse Braham
--
-- PROGRAMMER: Ronald Bellido, Jesse Braham
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

#include "utils.h"
#include "server_net.h"

using namespace std;

DWORD WINAPI handleClient(LPVOID param);
DWORD WINAPI listenThread(LPVOID args);
void DecodeRequest(char * request, SOCKET clientsocket);

int main(int argc, char* argv[])
{
	SOCKET listenSocket;
	std::vector<SOCKET> clientList;

	/*Server srvr;
	WSADATA wsaData;
	srvr.runServer(&wsaData);

	getchar();*/

	WSADATA wsadata;
	listenSocket = createServer(&wsadata, TCP);

	CreateThread(NULL, 0, listenThread, (LPVOID) &listenSocket, 0, 0);

	getchar();
	return EXIT_SUCCESS;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: listenThread
--
-- DATE: March 23, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: DWORD WINAPI listenThread(LPVOID args)
--
-- RETURNS: DWORD
--
-- NOTES: Thread that listens for new client connections. When a new client connects (succesfully), it spawns a new
thread, handleClient, to listen for that client's requests.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI listenThread(LPVOID args)
{
    SOCKET* pListenSock = (SOCKET*) args;

    cout << "Server started, listening on socket " << *pListenSock << endl;

    while(TRUE)
    {
        SOCKADDR_IN addr = {};
        int addrLen = sizeof(addr);

        SOCKET newClientSocket = WSAAccept(*pListenSock, (sockaddr*)&addr, &addrLen, NULL, NULL);

        if(newClientSocket == INVALID_SOCKET)
        {
            if(WSAGetLastError() != WSAEWOULDBLOCK)
            {
                cerr << "accept() failed with error " << WSAGetLastError() << endl;
                break;
            }
        }
        else
        {
            cout << "Socket " << newClientSocket << " accepted." << endl;
			//create handleClientThread,  passing &newClientSocket
			//push back to clientlist
			CreateThread(NULL, 0, handleClient, (LPVOID) &newClientSocket, 0, 0);
        }

        ::SleepEx(100, TRUE); //make this thread alertable
    }

    return 1;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleClient
--
-- DATE: March 23, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido, Jesse Braham
--
-- INTERFACE: DWORD WINAPI handleClient(LPVOID param)
--				
--
-- RETURNS: DWORD
--
-- NOTES: Thread that listens for the client's requests. Calls DecodeRequest to parse the request received.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI handleClient(LPVOID param)
{
	SOCKET controlSocket = *((SOCKET*) param);
	int numBytesRecvd, bytesToRead;
	char request[DATABUFSIZE];
	char * p;

	while (TRUE)
	{
		memset(request, 0, DATABUFSIZE);
		//listen for requests
		//when you receive, parse the request
		p = request;
		bytesToRead = DATABUFSIZE;

		while ((numBytesRecvd = recv(controlSocket, p, bytesToRead, 0)) > 0)
		{
			p += numBytesRecvd;
			bytesToRead -= numBytesRecvd;

			if (numBytesRecvd <= DATABUFSIZE)
				break;
		}

		//numBytesRecvd = recv(controlSocket, p, bytesToRead, 0);

		if ((numBytesRecvd < 0))
		{
			if (GetLastError() == WSAECONNRESET)
			{
				cout << "client disconnected" << endl;
				return 0;
			}

			cout << "Error: " << WSAGetLastError() << endl;
			continue;
		}

		DecodeRequest(request, controlSocket);
	}
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DecodeRequest
--
-- DATE: March 25, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido, Jesse Braham
--
-- INTERFACE: void DecodeRequest(char * request, SOCKET clientsocket)
--					request - the request packet to parse
--					clientsocket - the socket of the client that made the request
--				
--
-- RETURNS: void
--
-- NOTES: This function parses a request packet with respect to the specification document. After parsing,
it performs the appropriate steps to handle the request.
----------------------------------------------------------------------------------------------------------------------*/
void DecodeRequest(char * request, SOCKET clientsocket)
{
	string req = request;
	SOCKET sck = clientsocket;
	stringstream ss(req);
	string requesttype, filename;
	string tmp;

	ss >> requesttype;
	cout << requesttype << " ";

	if (requesttype == "ST")
	{
		while (ss >> tmp)
		{
			filename += tmp;
			filename += " ";
		}
		cout << filename << endl;
	}
	else if (requesttype == "DL")
	{
		int bytessent		= 0;
		int totalbytessent	= 0;

		while (ss >> tmp)
		{
			filename += tmp;
			filename += " ";
		}

		cout << filename << endl;

		string line;
		/*ifstream fileToSend;
		fileToSend.open(filename.c_str());

		if (!fileToSend.is_open())
			return;*/

		send(sck, request, DATABUFSIZE, 0); //sends a reply to client

		/*while (getline(fileToSend, line))
		{
			if ((bytessent = send(*clientsocket, line.c_str(), line.size(), 0)) == 0)
			{
				cerr << "Failed to send! Exited with error " << GetLastError() << endl;
				fileToSend.close();
				closesocket(*clientsocket);
				break;
			}

			totalbytessent += bytessent;
			cout << "Bytes sent: " << bytessent << endl;
			cout << "Total bytes sent: " << totalbytessent << endl;
		}*/

		req = "DL END\n";
		send(sck, req.c_str(), DATABUFSIZE, 0);
	}
	else if (requesttype == "UL")
	{
		while (ss >> tmp)
		{
			filename += tmp;
			filename += " ";
		}
		cout << filename << endl;
	}
	else if (requesttype == "MC")
	{
	}
	else if (requesttype == "MIC")
	{
	}
}
