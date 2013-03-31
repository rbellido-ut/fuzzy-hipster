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

DWORD WINAPI handleClientRequests(LPVOID param);
DWORD WINAPI listenThread(LPVOID args);
DWORD WINAPI multicastThread(LPVOID args);
ServerState DecodeRequest(char * request, string& filename);
void requestDispatcher(ServerState prevState, ServerState currentState, SOCKET clientsocket, string filename = "");

int main(int argc, char* argv[])
{
	SOCKET listenSocket;
	std::vector<SOCKET> clientList;

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
thread, handleClientRequests, to listen for that client's requests.
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
			CreateThread(NULL, 0, handleClientRequests, (LPVOID) &newClientSocket, 0, 0);
        }

        ::SleepEx(100, TRUE); //make this thread alertable
    }

    return 1;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleClientRequests
--
-- DATE: March 23, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido, Jesse Braham
--
-- INTERFACE: DWORD WINAPI handleClientRequests(LPVOID param)
--				
--
-- RETURNS: DWORD
--
-- NOTES: Thread that listens for the client's requests. Calls DecodeRequest to parse the request received.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI handleClientRequests(LPVOID param)
{
	SOCKET controlSocket = *((SOCKET*) param);
	int numBytesRecvd, bytesToRead;
	char request[DATABUFSIZE];
	char * p;
	ServerState newState = UNDEFINED, prevState = UNDEFINED;
	string filename;

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
			return 0;
		}

		//updated previous state if necessary
		if (prevState != UNDEFINED)
			prevState = newState;

		//get the new current state
		newState = DecodeRequest(request, filename);

		if (newState == SERVERROR)
			break;

		requestDispatcher(prevState, newState, controlSocket, filename);
	}

	return 1;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: requestDispatcher
--
-- DATE: March 29, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Ronald Bellido
--
-- PROGRAMMER: Ronald Bellido
--
-- INTERFACE: void requestDispatcher(ServerState prevState, ServerState currentState, SOCKET clientsocket, string filename)
--					prevState - the previous state the server was in
--					currentState - the new state the server is 
--				
--
-- RETURNS: void
--
-- NOTES: This function keeps track of the current and previous server state, then executes the steps to
the handle the current request. Note that any server errors is checked outside of this function, and thus 
assumes that everything was pretty ok before executing this function.
----------------------------------------------------------------------------------------------------------------------*/
void requestDispatcher(ServerState prevState, ServerState currentState, SOCKET clientsocket, string filename)
{
	int bytessent		= 0;
	int totalbytessent	= 0;
	string line;
	ifstream fileToSend;
	char* tmp;
	int n;

	//computing filesizes
	std::streampos filesize, begin, end;

	cout << "Previous State: " << prevState << endl;

	switch (currentState)
	{
		case STREAMING:

		break;

		case DOWNLOADING:

			//fileToSend.open(filename.c_str());
			fileToSend.open("test.mp3", ios::binary);
			if (!fileToSend.is_open()) //server can't open the file, file probably doesn't exist
				break;

			//compute size of the file
			begin = fileToSend.tellg();
			fileToSend.seekg(0, ios::end);
			filesize = fileToSend.tellg() - begin;
			fileToSend.seekg(ios::beg);
			cout << "File size of " << filename << ": " << filesize << " bytes" << endl;

			//echo the file size to the client to signal server's intent to establish a download line
			line = "DL " + filename + '\n';
			send(clientsocket, line.c_str(), line.size(), 0);
			line = ""; //just clear the line buffer	

			while (true)
			{
				tmp = new char [DATABUFSIZE];

				fileToSend.read(tmp, DATABUFSIZE);
				
				if((n=fileToSend.gcount()) > 0)
				{
					line.append(tmp, n);
					if ((bytessent = send(clientsocket, line.c_str(), line.size(), 0)) == 0)
					{
						cerr << "Failed to send! Exited with error " << GetLastError() << endl;
						fileToSend.close();
						return;
					}

					totalbytessent += bytessent;
					cout << "Bytes sent: " << bytessent << endl;
					cout << "Total bytes sent: " << totalbytessent << endl;
					line.clear();
				}
				else
				{
					delete[] tmp;
					break;
				}

				 delete[] tmp;
			}

			//EOT in hex
			line = "DLEND\n";
			send(clientsocket, line.c_str(), line.size(), 0);
		break;

		case UPLOADING:
		break;

		case MICCHATTING:
		break;

		case MULTICASTING:
			// Spawn a new thread:
			//		Set up Multicast server
			//		Stream audio to the multicast address
		break;
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
-- INTERFACE: void DecodeRequest(char * request, string& filename)
--					request - the request packet to parse
--					filename - the name of the file
--				
--
-- RETURNS: returns an int which will indicate one of the specified ServerStates in util.h (STREAMING, DOWNLOADING, etc.)
--			On error, the function will return SERVERROR, which typcially means that there was an error parsing the request, or 
--			an invalid request was received.
--
-- NOTES: This function parses a request packet with respect to the specification document. After parsing,
it will return the current state of the server.
----------------------------------------------------------------------------------------------------------------------*/
ServerState DecodeRequest(char * request, string& filename)
{
	string req = request;
	stringstream ss(req);
	string requesttype;

	ss >> requesttype;
	cout << "received " << requesttype << " ";

	if (requesttype == "ST")
	{
		getline(ss, filename); //read ss up to newline
		cout << filename << endl;
		return STREAMING;
	}
	else if (requesttype == "DL")
	{
		getline(ss, filename);
		cout << filename << endl;
		return DOWNLOADING;
	}
	else if (requesttype == "UL")
	{
		getline(ss, filename);
		cout << filename << endl;
		return UPLOADING;
	}
	else if (requesttype == "MC")
	{
		//'hook' the client to the multicast broadcast channel
		cout << "hooking client to the multicast channel" << endl;
		return MULTICASTING;
	}
	else if (requesttype == "MIC")
	{
		cout << "2 way chat requested" << endl;
		return MICCHATTING;
	}

	return SERVERROR;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: multicastThread
--
-- DATE: March 31, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Braham
--
-- PROGRAMMER: Jesse Braham
--
-- INTERFACE: DWORD WINAPI multicastThread(LPVOID args)
--
-- RETURNS: DWORD
--
-- NOTES: This thread starts up a multicast server.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI multicastThread(LPVOID args)
{
	char achMCAddr[MAXADDRSTR] = TIMECAST_ADDR;
	u_short nPort              = TIMECAST_PORT;
	u_long  lTTL               = TIMECAST_TTL;
	
	int				nRet;
	BOOL			fFlag;
	SOCKADDR_IN		server, destination;
	struct ip_mreq	stMreq;
	SOCKET			hSocket;
	WSADATA			stWSAData;

	nRet = WSAStartup(0x0202, &stWSAData);
	if (nRet)
	{
		cerr << "WSAStartup failed: " << nRet << endl;	
		return 0;
	}

	hSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		cerr << "socket() failed, Err: " << WSAGetLastError() << endl;	
		return 0;
	}

	server.sin_family      = AF_INET; 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port        = 0;

	nRet = bind(hSocket, (struct sockaddr*)&server, sizeof(server));
	if (nRet == SOCKET_ERROR) 
	{
		cerr << "bind() port: " << nPort << " failed, Err: " << WSAGetLastError() << endl;
		return 0;
	}

	stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
	stMreq.imr_interface.s_addr = INADDR_ANY;

	nRet = setsockopt(hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
	if (nRet == SOCKET_ERROR)
	{
		cerr << "setsockopt() IP_ADD_MEMBERSHIP address " << achMCAddr << " failed, Err: " << WSAGetLastError() << endl;
		return 0;
	}

	nRet = setsockopt(hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&lTTL, sizeof(lTTL));
	if (nRet == SOCKET_ERROR)
	{
		cerr << "setsockopt() IP_MULTICAST_TTL failed, Err: " << WSAGetLastError() << endl;
		return 0;
	}

	fFlag = FALSE;
	nRet = setsockopt(hSocket, IPPROTO_IP,  IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));
	if (nRet == SOCKET_ERROR)
	{
		cerr << "setsockopt() IP_MULTICAST_LOOP failed, Err: " << WSAGetLastError() << endl;
		return 0;
	}

	destination.sin_family =      AF_INET;
	destination.sin_addr.s_addr = inet_addr(achMCAddr);
	destination.sin_port =        htons(nPort);

	// SEND DATA HERE

	return 1;
}