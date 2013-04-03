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

#include "server.h"

using namespace std;
using namespace libZPlay;

int main(int argc, char* argv[])
{
	SOCKET listenSocket;
	std::vector<SOCKET> clientList;

	cout << "--------------------------------------------------" << endl;
	cout << "- Fuzzy Audio Player  -  Server" << endl;
	cout << "--------------------------------------------------" << endl;


	WSADATA wsadata;
	listenSocket = createServer(&wsadata, TCP);

	CreateThread(NULL, 0, listenThread, (LPVOID) &listenSocket, 0, 0);
	CreateThread(NULL, 0, multicastThread, NULL, 0, 0);

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
--			thread, handleClientRequests, to listen for that client's requests.
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
	int uploadfilesize;

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

		if ((numBytesRecvd < 0))
		{
			if (GetLastError() == WSAECONNRESET)
			{
				cerr << "client disconnected" << endl;
				return 0;
			}

			cout << "Error: " << WSAGetLastError() << endl;
			return 0;
		}

		//updated previous state if necessary
		if (prevState != UNDEFINED)
			prevState = newState;

		//get the new current state
		newState = DecodeRequest(request, filename, uploadfilesize);

		if (newState == SERVERROR)
			break;

		requestDispatcher(prevState, newState, controlSocket, filename, uploadfilesize);
	}

	return 1;
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
-- INTERFACE: ServerState DecodeRequest(char * request, string& filename)
--					request - the request packet to parse
--					filename - the name of the file
--					uploadfilesize - the size of the file to upload
--				
--
-- RETURNS: returns ServerState which will indicate one of the specified ServerStates in util.h (STREAMING, DOWNLOADING, etc.)
--			On error, the function will return SERVERROR, which typcially means that there was an error parsing the request, or 
--			an invalid request was received.
--
-- NOTES: This function parses a request packet with respect to the specification document. After parsing,
--			it will return the current state of the server.
----------------------------------------------------------------------------------------------------------------------*/
ServerState DecodeRequest(char * request, string& filename, int& uploadfilesize)
{
	string req = request;
	stringstream ss(req);
	string requesttype;

	ss >> requesttype;
	cout << "received " << requesttype << " ";

	if (requesttype == "LIST")
	{
		return LIST;
	}
	else if (requesttype == "ST") //received: ST filename\n"
	{
		getline(ss, filename); //read ss up to newline
		cout << filename << endl;
		return STREAMING;
	}
	else if (requesttype == "DL")
	{
		getline(ss, filename); //received: DL filename\n
		cout << filename << endl;
		return DOWNLOADING;
	}
	else if (requesttype == "UL")
	{
		ss >> uploadfilesize;
		getline(ss, filename); // received: UL uploadfilesize filename\n
		cout << uploadfilesize << filename << endl;
		return UPLOADING;
	}
	else if (requesttype == "MC") //received: MC\n
	{
		//'hook' the client to the multicast broadcast channel
		cout << "hooking client to the multicast channel" << endl;
		return MULTICASTING;
	}
	else if (requesttype == "MIC") //received: MIC\n
	{
		cout << "Received: 2 way chat request" << endl;
		return MICCHATTING;
	}
	else 
	{
		cout << "\n";
	}

	return SERVERROR;
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
-- PROGRAMMER: Ronald Bellido, Jesse Braham
--
-- INTERFACE: void requestDispatcher(ServerState prevState, ServerState currentState, SOCKET clientsocket, string filename)
--					prevState - the previous state the server was in
--					currentState - the new state of the server
--					clientsocket - the socket of the client to send
--					filename - the name of the file being transferred; it's an empty string by default
--					uploadfilesize - size of the file to upload in bytes with default value of 0
--				
--
-- RETURNS: void
--
-- NOTES: This function keeps track of the current and previous server state, then executes the steps to
--			the handle the current request. Note that any server errors is checked outside of this function, and thus 
--			assumes that everything was pretty ok before executing this function.
----------------------------------------------------------------------------------------------------------------------*/
void requestDispatcher(ServerState prevState, ServerState currentState, SOCKET clientsocket, string filename, int uploadfilesize)
{
	int				bytessent = 0,
					bytesrecvd = 0;
	int				totalbytessent	= 0,
					totalbytesrecvd = 0;
	string			line;
	ifstream		fileToSend;
	ofstream		fileRecvd;
	char*			tmp;
	streamsize		numberOfBytesRead;
	vector<string>	song_list;
	int				num_songs;

	//computing filesizes
	ostringstream oss;
	std::streampos begin, end;
	long int filesize = 0;

	//cout << "Previous State: " << prevState << endl;

	switch (currentState)
	{
		case LIST:
			num_songs = populateSongList((vector<string>&)song_list);

			if (num_songs > 0)
			{
				for (vector<string>::iterator it = song_list.begin(); it != song_list.end(); ++it)
				{
					line += *it;
					line += '\n';
				}

				if (((bytessent = send(clientsocket, line.c_str(), line.size(), 0))) == 0 || (bytessent == -1))
				{
					cerr << "Failed to send packet, Error: " << GetLastError() << endl;
					return;
				}
			}

			// send EOT
			line = '\x004';
			send(clientsocket, line.c_str(), line.size(), 0);
			cout << "song list sent" << endl;
		break;

		case STREAMING:
			// get absolute path, remove added *, append filename (but remove appended space)
			fileToSend.open(getMusicDir().substr(0,getMusicDir().size()-1).insert(getMusicDir().size()-1,filename.substr(1,filename.size())), ios::binary);


			if (!fileToSend.is_open()) //server can't open the file, file probably doesn't exist. Deny client to download file.
			{
				line = "ST\n";
				send(clientsocket, line.c_str(), line.size(), 0);
				line = "";
				cout << "Streaming request denied. Can't open file." << endl;
				break;
			}

			//compute size of the file
			begin = fileToSend.tellg();
			fileToSend.seekg(0, ios::end);
			filesize = static_cast<long int>(fileToSend.tellg() - begin);
			fileToSend.seekg(begin);
			cout << "File size of " << filename << ": " << filesize << " bytes" << endl;

			//echo the file size to the client to signal server's intent to establish a download line
			oss << "ST " << filesize << "\n";
			line = oss.str();
			send(clientsocket, line.c_str(), line.size(), 0);
			line = ""; //just clear the line buffer	

			cout << "Streaming..." << endl;

			while (true)
			{
				tmp = new char [1024];

				numberOfBytesRead = 0;
				fileToSend.read(tmp, 1024);
				
				if((numberOfBytesRead = fileToSend.gcount()) > 0)
				{
					line.append(tmp, static_cast<unsigned int>(numberOfBytesRead));
					if (((bytessent = send(clientsocket, line.c_str(), line.size(), 0))) == 0 || (bytessent == -1))
					{
						cerr << "Failed to send! Exited with error " << GetLastError() << endl;
						cerr << "Ending streaming session..." << endl;
						fileToSend.close();
						delete[] tmp;
						return;
					}

					totalbytessent += bytessent;
					cout << "Bytes sent: " << bytessent << endl;
					cout << "Total bytes sent: " << totalbytessent << endl;
					line.clear();
				} 
				
				if (totalbytessent == filesize)
					break;
			}

			//EOT in hex
			cout << "Done streaming" << endl;
			line = "STEND\n";
			send(clientsocket, line.c_str(), line.size(), 0);
			fileToSend.close();
			delete[] tmp;
			break;
		case DOWNLOADING:

			// get absolute path, remove added *, append filename (but remove appended space)
			fileToSend.open(getMusicDir().substr(0,getMusicDir().size()-1).insert(getMusicDir().size()-1,filename.substr(1,filename.size())), ios::binary);

			if (!fileToSend.is_open()) //server can't open the file, file probably doesn't exist. Deny client to download file.
			{
				line = "DL\n";
				send(clientsocket, line.c_str(), line.size(), 0);
				line = "";
				cout << "Download request denied. Can't open file" << endl;
				break;
			}

			//compute size of the file
			begin = fileToSend.tellg();
			fileToSend.seekg(0, ios::end);
			filesize = static_cast<long int>(fileToSend.tellg() - begin);
			fileToSend.seekg(begin);
			cout << "File size of " << filename << ": " << filesize << " bytes" << endl;

			//echo the file size to the client to signal server's intent to establish a download line
			oss << "DL " << filesize << "\n";
			line = oss.str();
			send(clientsocket, line.c_str(), line.size(), 0);
			line = ""; //just clear the line buffer	

			while (true)
			{
				tmp = new char [DATABUFSIZE];

				numberOfBytesRead = 0;
				fileToSend.read(tmp, DATABUFSIZE);
				
				if((numberOfBytesRead = fileToSend.gcount()) > 0)
				{
					line.append(tmp, static_cast<unsigned int>(numberOfBytesRead));
					if (((bytessent = send(clientsocket, line.c_str(), line.size(), 0))) == 0 || (bytessent == -1))
					{
						cerr << "Failed to send! Exited with error " << GetLastError() << endl;
						cerr << "Ending download session..." << endl;
						fileToSend.close();
						delete[] tmp;
						return;
					}

					totalbytessent += bytessent;
					cout << "Bytes sent: " << bytessent << endl;
					cout << "Total bytes sent: " << totalbytessent << endl;
					line.clear();
				} 
				
				if (totalbytessent == filesize)
					break;
			}

			//EOT in hex
			cout << "Done downloading" << endl;
			line = "DLEND\n";
			send(clientsocket, line.c_str(), line.size(), 0);
			fileToSend.close();
			delete[] tmp;
		break;

		case UPLOADING:
			cout << "Uploading..." << endl;
			cout << "filesize of the file to upload: " << filesize << endl;

			line = "UL " + filename + '\n';
			send(clientsocket, line.c_str(), line.size(), 0);
			line = ""; //just clear the line buffer	

			fileRecvd.open(filename, ios::binary); //TODO: hardcoded!
			
			if (!fileRecvd.is_open()) //server can't open the file. Deny client to download file.
			{
				line = "UL\n";
				send(clientsocket, line.c_str(), line.size(), 0);
				line = "";
				break;
			}
			
			while (true)
			{
				tmp = new char[DATABUFSIZE];
				memset(tmp, 0, DATABUFSIZE);

				if (((bytesrecvd = recv(clientsocket, tmp, DATABUFSIZE, 0)) == 0) || (bytesrecvd == -1))
				{
					cerr << "recv failed with error " << GetLastError() << endl;
					cout << "Ending upload session..." << endl;
					fileRecvd.close();
					delete[] tmp;
					return;
				}

				fileRecvd.write(tmp, bytesrecvd);
				totalbytesrecvd += bytesrecvd;
				cout << "Bytes received: " << bytesrecvd << endl;
				cout << "Total bytes received: " << totalbytesrecvd << endl;

				if (totalbytesrecvd == uploadfilesize) //Uploading is done
				{
					cout << "Done uploading" << endl;
					fileRecvd.close();
					delete[] tmp;
					break;
				}
				delete[] tmp;
			}

		break;

		case MICCHATTING:
			cout << "Mic session started..." << endl;
			startMicSession();
		break;

		case MULTICASTING:
			// Spawn a new thread:
			//		Set up Multicast server
			//		Stream audio to the multicast address
		break;
	}
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
-- NOTES: This thread starts up a multicast server.  It then iterates through the list of songs available on the server,
--			and broadcasts each song to the multicast destination address.  If a song cannot be read, it is simply
--			skipped, and the server attempts to read the next file.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI multicastThread(LPVOID args)
{
	char			achMCAddr[MAXADDRSTR] = TIMECAST_ADDR,
					*tmp;

	u_short			nPort = TIMECAST_PORT;
	u_long			lTTL = TIMECAST_TTL;

	int				nRet,
					num_songs;

	BOOL			fFlag;

	SOCKADDR_IN		server,
					destination;

	struct ip_mreq	stMreq;
	SOCKET			hSocket;
	WSADATA			stWSAData;
	vector<string>	song_list;

	string			dir,
					line;

	ifstream*		fileToSend;

	fileToSend = new ifstream;

	streamsize		numberOfBytesRead;
	
	LPMULTICASTVARS multcastvars = (LPMULTICASTVARS) malloc(sizeof(MULTICASTVARS));


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

	dir = getMusicDir();
	num_songs = populateSongList(song_list);

	if (num_songs > 0)
	{
		//Open libzplay stream and settings
		ZPlay * multicaststream = CreateZPlay();
		
		for (vector<string>::iterator it = song_list.begin(); it != song_list.end(); ++it)
		{
			multicaststream->SetSettings(sidSamplerate, 44100);
		multicaststream->SetSettings(sidChannelNumber, 2);
		multicaststream->SetSettings(sidBitPerSample, 16);
		multicaststream->SetSettings(sidBigEndian, 0);

			std::streampos begin, end;
			int bytessent = 0,
				filesize,
				totalbytessent = 0;

			string absSongPath = dir;
			string::size_type pos = absSongPath.find_last_of("*");
			absSongPath = absSongPath.substr(0, pos);
			absSongPath += *it;

			begin = fileToSend->tellg();
			fileToSend->seekg(0, ios::end);
			end = fileToSend->tellg();
			fileToSend->seekg(0, ios::beg);
			filesize = end - begin;

			fileToSend->open(absSongPath, ios::binary);

			if (!fileToSend->is_open())
				continue;

			multcastvars->file = fileToSend;
			multcastvars->multaddr = destination;
			multcastvars->socket = hSocket;
			multcastvars->filesize = filesize;
			
			//Set up the multicast callback
			multicaststream->SetCallbackFunc(multicastCallback, (TCallbackMessage) (MsgStreamNeedMoreData | MsgWaveBuffer), (void *) multcastvars);

			//Figure out the format of the file
			//TStreamFormat format = multicaststream->GetFileFormat(it->c_str());

			int n;
			if (multicaststream->OpenStream(1, 1, &n, 1, sfPCM) == 0)
			{
				cerr << "Error in opening a multicast stream: " << multicaststream->GetError() << endl;
				multicaststream->Release();
				return 1;
			}

			multicaststream->SetMasterVolume(0,0); //turn down the volume

			multicaststream->Play(); //start streaming

			while (TRUE)
			{
				TStreamStatus status;
				multicaststream->GetStatus(&status);
				if (status.fPlay == 0)
					break; //exit the loop

				//get current position
				TStreamTime pos;
				multicaststream->GetPosition(&pos);
				cout << "Pos: " << pos.hms.hour << " " << pos.hms.minute << " " << pos.hms.second << " " << pos.hms.millisecond << endl;

				Sleep(300); //TODO: need to remove this later
			}

			fileToSend->close();
			
		}
		multicaststream->Release();
		free(multcastvars);
	}

	return 1;
}

int  __stdcall  multicastCallback(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2)
{
	//watch for MsgNeedMoreData, MsgWaveBuffer
	//return 0 on msgWavebuffer

	ZPlay * multicaststream = (ZPlay*) instance;
	LPMULTICASTVARS mcv = (LPMULTICASTVARS) user_data;
	char* buffer = new char[DATABUFSIZE];

	switch (message)
	{
		case MsgStreamNeedMoreData:
			mcv->file->read(buffer, 1024); //TODO: might need to cast databufsize and change how much I'm reading
			//cout << "Read " << mcv->file->gcount() << endl;
			multicaststream->PushDataToStream(buffer, mcv->file->gcount());
		break;

		case MsgWaveBuffer:
			if (sendto(mcv->socket, (const char *) param1, param2, 0,(const SOCKADDR *)& mcv->multaddr, sizeof(mcv->multaddr)) < 0)
			{
				cerr << "Error in sendto: " << GetLastError();
				free(mcv);
				return 2;
			}
		break;
	}
	
	//delete buffer;
	//free(mcv);
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getMusicDir
--
-- DATE: April 1, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Braham
--
-- PROGRAMMER: Jesse Braham
--
-- INTERFACE: string getMusicDir()
--
-- RETURNS: string
--
-- NOTES:  This function locates the 'Music' directory and returns the absolute path to said directory.
----------------------------------------------------------------------------------------------------------------------*/
string getMusicDir()
{
	char buf[MAX_PATH];
	string dir;

	GetModuleFileName(NULL, buf, MAX_PATH);
	string::size_type pos = string(buf).find_last_of("\\/");
	dir = string(buf).substr(0, pos);
	dir += "\\Music\\*";

	return dir;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: populateSongList
--
-- DATE: April 1, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Braham
--
-- PROGRAMMER: Jesse Braham
--
-- INTERFACE: int populateSongList()
--
-- RETURNS: int
--
-- NOTES:  This function scans the 'Music' directory (located in the current execution directory) for files, appends
--			the name of each file to the song_list, and returns the total number of files scanned.
----------------------------------------------------------------------------------------------------------------------*/
int populateSongList(vector<string>& song_list)
{
	HANDLE hFind;
	WIN32_FIND_DATA data;
	int num_songs = 0;
	string dir;

	dir = getMusicDir();
	
	hFind = FindFirstFile(dir.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (data.cFileName[0] != '.')
			{
				num_songs++;
				song_list.push_back(data.cFileName);
			}
		} while (FindNextFile(hFind, &data));

		FindClose(hFind);
	}

	return num_songs;
}

