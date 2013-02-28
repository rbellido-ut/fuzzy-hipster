#include "fuzzyplayer_utils.h"
#include "NetStream.h"

int main(int argc, char **argv)
{
	int port;
	char  *host;
	WSADATA wsaData;
	WORD wVersionRequested;

	switch(argc)
	{
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		default:
			fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
			exit(1);
	}

	wVersionRequested = MAKEWORD(2, 2);
	if ((WSAStartup( wVersionRequested, &wsaData)) != 0) {
		std::cerr << "WSAStartup failed with error " << WSAGetLastError() << std::endl;
		exit(0);
	}

	NetStream netstream(host, port);
	sf::Music song;
	if (!song.openFromStream(netstream)) {
		std::cerr << "Failed opening the stream!" << std::endl;
		return 0;
	}

	song.play();

	SleepEx(INFINITE, TRUE);

	
	WSACleanup();

	return 1;
}