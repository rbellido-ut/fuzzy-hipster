#include "client_net.h"

class Client {
public:
	Client() {connectSocket_ = 0;}
	~Client(){}

	void runClient(WSADATA *wsadata) {
		char **pptr;
		
		connectSocket_ = createTCPClient(wsadata);
		if (WSAConnect (connectSocket_, (struct sockaddr *)&addr_, sizeof(addr_), NULL, NULL, NULL, NULL) == INVALID_SOCKET)
		{
			std::cerr << "Can't connect to server" << std::endl;
			std::cerr << "connect()" << std::endl;
			exit(0);
		}
		std::cout << "Connected:    Server Name: " << hp_->h_name << std::endl;
		pptr = hp_->h_addr_list;
		std::cout << "\t\tIP Address: " <<  inet_ntoa(addr_.sin_addr) << std::endl;

		std::cout << "Server started, connected to socket " << connectSocket_ << std::endl;

	}

private:
	SOCKET connectSocket_;
	SOCKADDR_IN addr_;
	hostent *hp_;
	SOCKET createTCPClient(WSADATA* wsaData);
};