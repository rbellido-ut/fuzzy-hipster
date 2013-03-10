/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		communication.h -  This header file contains the constructor, copy constructor, destructor, 
--					getters/setters, function prototypes, and class members related to the server class.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		explicit Communication(const pid_t& pid, const int& ipcID);
--					Communication (const Communication& src);
--					~Communication();
--					
--
-- DATE:			March 4th, 2013
--
-- REVISIONS: 
--
-- DESIGNER:		Behnam Bastami
--
-- PROGRAMMER:		Behnam Bastami
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

#ifndef Communication_H
#define Communication_H

#include "util.h"

class Communication{
private:

	std::string readBuffer_, writeBuffer_;
	int bytesSent_, bytesRecvd_;
	SOCKET readSocket_, writeSocket_;
	static size_t count_;

public:

	int TCPPORT, UDPPORT;



	//Constructor
	explicit Communication(const std::string& readBuffer = "",const std::string& writeBuffer = "",
		const int& bytesSent = 0, const int& bytesRecvd = 0, 
		const SOCKET& readSocket = 0, const SOCKET& writeSocket = 0)
		:readBuffer_(readBuffer), writeBuffer_(writeBuffer), bytesSent_(bytesSent),
		bytesRecvd_(bytesRecvd), readSocket_(readSocket), writeSocket_(writeSocket)
	{ 
		TCPPORT= 5150;
		UDPPORT = 6000;
		count_++; 
	}

	//Copy constructor
	Communication (const Communication& src);	//copy ctor

	//Destructor
	~Communication(){ count_--; }

	//Getter functions
	std::string getReadBuffer ()const { return readBuffer_; }
	std::string getwriteBuffer() const{ return writeBuffer_; }
	int getBytesSent () const{ return bytesSent_; }
	int getBytesRecvd ()const { return bytesRecvd_; }
	SOCKET getReadSocket ()const { return readSocket_; }
	SOCKET getWriteSocket ()const { return writeSocket_; }
	static size_t getCount() { return count_; }

	//Setter functions
	std::string setReadBuffer ();
	std::string setwriteBuffer ();
	int setBytesSent (const int sb);
	int setBytesRecvd (const int rb);
	SOCKET setReadSocket (const SOCKET rs);
	SOCKET setWriteSocket (const SOCKET ws);

	//Other function prototypes
	bool sendPacket();
	bool recvPacket();

	

	//Friend decleration of input/output operators
	friend std::ostream& operator<< (std::ostream& os, const Communication& cs);
	friend std::istream& operator>> (std::istream& is, Communication& cs);



};

#endif
