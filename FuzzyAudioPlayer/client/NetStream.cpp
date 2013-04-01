#include "NetStream.h"

NetStream::NetStream() :  streambuffer(), filesize(0), position(0), streamsocket(0)
{}

NetStream::NetStream(SOCKET sock) :  streambuffer(), filesize(0), position(0), streamsocket(0) {
	streamsocket = sock;
}

/*	This function takes in the port of the server that a client will connect to and
	the IP address of that server as a string*/
NetStream::NetStream(std::string host, int port) :  streambuffer(), filesize(0), position(0), streamsocket(0) {

	SOCKADDR_IN server;
	PHOSTENT pHostent;

	//Open up a UDP socket
	if ((streamsocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		std::cerr << "socket() failed!" << std::endl;
		exit(0);
	}

	memset((char*) &server, 0, sizeof(SOCKADDR_IN));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((pHostent = gethostbyname(host.c_str())) == NULL) {
		std::cerr << "Unknown server address" << std::endl;
		exit(0);
	}

	memcpy((char *) &server.sin_addr, pHostent->h_addr, pHostent->h_length);

	if (connect(streamsocket, (PSOCKADDR) &server, sizeof(server)) == -1) {
		std::cerr << "Can't connect to server. Exited with error code " << WSAGetLastError() << std::endl;
		exit(0);
	}
	std::cout << "Connected to " << pHostent->h_name << std::endl;
}

NetStream::~NetStream() {
	closesocket(streamsocket);
}

sf::Int64 NetStream::read(void* data, sf::Int64 size) {
	int bytesrecvd;

	while ( (position+size) >= streambuffer.size()) {
		std::cout << "receiving " << size;
		if ((bytesrecvd = recv(streamsocket, (char*) data, size, 0)) == SOCKET_ERROR) {
			std::cerr << "Error in recv. Got error " << WSAGetLastError() << std::endl;
			return -1;
		}

		if (bytesrecvd > 0) { //recvd data, add to buffer
			streambuffer.append((char*) data, bytesrecvd);
		} else {
			std::cerr << "Stream disconnected" << std::endl;
			return 0;
		}
	}

	if ( (position+size) < streambuffer.size()) {
		memcpy(data, streambuffer.data() + position, size);
		position += size;
		return size;
	} else if ( position < streambuffer.size() ) {
		sf::Int64 bytes_read = streambuffer.size() - position;
		memcpy( data, streambuffer.data() + position, bytes_read);
		position += bytes_read;
		return bytes_read;
	}
}

sf::Int64 NetStream::seek(sf::Int64 new_position) {
	return position = new_position;
}

sf::Int64 NetStream::tell() {
	return position;
}

sf::Int64 NetStream::getSize() {
	return 6870890;
}

