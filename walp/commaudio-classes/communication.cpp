/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		communication.cpp -  This file contains function implementations related to the communication class.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		
--
-- DATE:			
--
-- REVISIONS: 
--
-- DESIGNER:		Behnam Bastami
--
-- PROGRAMMER:		Behnam Bastami
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

#include "util.h"
using namespace std;
size_t Communication::count_ = 0;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Constructor
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	
--
-- RETURNS:		
--
-- NOTES:		
----------------------------------------------------------------------------------------------------------------------*/
/*Communication::Communication(const std::string& readBuffer = "",const std::string& writeBuffer = "",
const int& bytesSent = 0, const int& bytesRecvd = 0, 
const SOCKET& readSocket = 0, const SOCKET& writeSocket = 0)
:readBuffer_(readBuffer), writeBuffer_(writeBuffer), bytesSent_(bytesSent),
bytesRecvd_(bytesRecvd), readSocket_(readSocket), writeSocket_(writeSocket)
{ count_++; }*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Copy Constructor
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	Communication::Communication(const Communication& src)
--				src: The source communication object that needs to be copied
--
-- RETURNS:		
--
-- NOTES:		This is the copy constructor of the communications class. It takes a source communication object and 
--				copies its data members to a new communication object.
----------------------------------------------------------------------------------------------------------------------*/
Communication::Communication(const Communication& src)
	:readBuffer_(src.readBuffer_), writeBuffer_(src.writeBuffer_), bytesSent_(src.bytesSent_),
	bytesRecvd_(src.bytesRecvd_), readSocket_(src.readSocket_), writeSocket_(src.writeSocket_)
{ count_++; }


bool Communication::sendPacket()
{

	return true;

}

bool Communication::recvPacket()
{

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	operator<<
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	ostream& operator<< (ostream& os, const Communication& cs)
--				os: An output stream
--				cs: The Communication object that needs to be outputed
--
-- RETURNS:		Returns an output stream containing the communication object's data members
--
-- NOTES:		
----------------------------------------------------------------------------------------------------------------------*/
ostream& operator<< (ostream& os, const Communication& cs)
{
	return os;
}

