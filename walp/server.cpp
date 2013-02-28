/*
*	This is a win32 console program that wil start a server on port 7000 and stream a single song specified by the user.
*	This is written in C++, using blocking socket functions to simply transfer a file (the song) to a client over a TCP network.
*
*	Usage: FuzzyPlayer song
*
*/
#include "fuzzyplayer_utils.h"
#include "NetStream.h"

using namespace std;

DWORD WINAPI StreamThread(LPVOID lpParameter);

bool quit = false;

int server(int argc, char **argv)
{
	int	client_len, port, err;
	SOCKET listensocket;
	struct	sockaddr_in server, client;
	string bp, buffer;
	WSADATA WSAData;
	WORD wVersionRequested;

	LPSOCKET_INFORMATION socketinfo;
	socketinfo = (LPSOCKET_INFORMATION) malloc(sizeof(LPSOCKET_INFORMATION));
	HANDLE StreamThreadHandle;
	DWORD StreamThreadID;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s song\n", argv[0]);
		return 0;
	}

	port = SERVER_TCP_PORT;
	socketinfo->filename = argv[1];
	
	wVersionRequested = MAKEWORD( 2, 2 ); 
	err = WSAStartup( wVersionRequested, &WSAData );
	if ( err != 0 )  //No useable DLL
	{
		printf ("DLL not found!\n");
		return 0;
	}

	// Create a stream socket
	if ((listensocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 
		0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 0;
	}

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in)); 
	server.sin_family = AF_INET;
	server.sin_port = htons(port); 
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client
	
	// Bind an address to the socket
	if (bind(listensocket, (PSOCKADDR)&server, sizeof(server)) == SOCKET_ERROR)
	{
		perror("Can't bind name to socket");
		return 0;
	}

	// Listen for connections
	// queue up to 5 connect requests
	listen(listensocket, 5);

	while (!quit)
	{
		client_len= sizeof(client); 

		//block here until a new client connects....
		if ((socketinfo->acceptsocket = accept (listensocket, (PSOCKADDR)&client, &client_len)) == -1)
		{
			fprintf(stderr, "Can't accept client\n"); 
			return 0;
		}

		cout << "client with IP address " << inet_ntoa(client.sin_addr) << " has connected" << endl;

		//...then we create a separate thread for it to stream music.
		if ((StreamThreadHandle = CreateThread(NULL, 0, StreamThread, (LPVOID) socketinfo, 0, &StreamThreadID)) == NULL)
		{
			printf("CreateThread failed with error %d\n", GetLastError());
			return 0;
		}
	}

	free(socketinfo);
	closesocket(listensocket);
	WSACleanup();
	return 1;
}

DWORD WINAPI StreamThread(LPVOID lpParameter)
{
	int bytessent = 0, totalbytessent = 0;
	//int filesize;
	LPSOCKET_INFORMATION socketinfo = (LPSOCKET_INFORMATION) lpParameter;
	string line;

	//Open file for sending
	ifstream musicfile;
	musicfile.open(socketinfo->filename);
	
	//figure out the filesize and store it in socketinfo
	/*musicfile.seekg(0, ios::end);
	filesize = musicfile.tellg();
	musicfile.seekg(0, ios::beg);
	socketinfo->filesize = (sf::Int64) filesize;*/

	if (musicfile.is_open())
	{
		//TODO: I have no idea if this will work.
		while (getline(musicfile, line))
		{
			//Send through the network. Note that this will block until specified amount of bytes are sent
			if ((bytessent = send(socketinfo->acceptsocket, line.c_str(), line.size(), 0)) == 0)
			{
				cerr << "Failed to send! Exited with error " << GetLastError() << endl;
				musicfile.close();
				closesocket(socketinfo->acceptsocket);
				return FALSE;
			}

			totalbytessent += bytessent;
			cout << "Bytes sent: " << bytessent << endl;
			cout << "Total bytes sent: " << totalbytessent << endl;
		}
	}
	else
	{
		cerr << "music file did not open for some reason!" << endl;
		closesocket(socketinfo->acceptsocket);
		return FALSE;
	}

	closesocket(socketinfo->acceptsocket);
	musicfile.close();
	quit = true;
	return TRUE;
}