#ifndef NETSTREAM_H
#define NETSTREAM_H

#include "utils.h"
#include <SFML/Audio.hpp>

class NetStream : public sf::InputStream
{

public:
	
	NetStream();

	NetStream(std::string host, int port);

	NetStream::NetStream(SOCKET);

	NetStream(const NetStream& ns) : filesize(ns.filesize), streamsocket(ns.streamsocket), streambuffer(ns.streambuffer) {};

	~NetStream();

	void close();
	virtual sf::Int64 read(void* data, sf::Int64 size);
	virtual sf::Int64 seek(sf::Int64 new_position);
	virtual sf::Int64 tell();
	virtual sf::Int64 getSize();
	std::string streambuffer;


private:
	
	sf::Int64 filesize;
    sf::Int64 position;
	int bytestoread;
    SOCKET streamsocket;
};
#endif
