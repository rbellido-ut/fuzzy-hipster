#include "net_stream.h"

sf::Int64 NetStream::read(void* data, sf::Int64 size)
{
	int bytesrecvd;

	while ( (position+size) >= streambuffer.size()) 
	{
		std::cout << "receiving " << size;
		if ((bytesrecvd = recv(streamsocket, (char*) data, size, 0)) == SOCKET_ERROR) 
		{
			std::cerr << "Error in recv. Got error " << WSAGetLastError() << std::endl;
			return -1;
		}

		if (bytesrecvd > 0) 
		{ //recvd data, add to buffer
			streambuffer.append((char*) data, bytesrecvd);
		} 
		else 
		{
			std::cerr << "Stream disconnected" << std::endl;
			return 0;
		}
	}

	if ( (position+size) < streambuffer.size()) 
	{
		memcpy(data, streambuffer.data() + position, size);
		position += size;
		return size;
	} 
	else if ( position < streambuffer.size() ) 
	{
		sf::Int64 bytes_read = streambuffer.size() - position;
		memcpy( data, streambuffer.data() + position, bytes_read);
		position += bytes_read;
		return bytes_read;
	}
}