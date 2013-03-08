/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		client.cpp -  This file contains function implementations related to the client class.
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		bool Client::setMusicList(std::vector<int> ml)
--					bool Client::requestConnect()
--					bool Client::requestDownload()
--					bool Client::requestUpload()
--					bool Client::requestStream()
--					bool Client::saveToFile()
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

#include "util.h"
using namespace std;
size_t Client::count_ = 0;

//Setter functions
bool Client::setMusicList(std::vector<int> ml){

	return true;
}

//Other function prototypes

bool Client::initClient(){

	return true;
}

bool Client::startClient(){

	return true;
}

bool Client::stopClient(){

	return true;
}

bool Client::requestConnect(){

	return true;
}

bool Client::requestDownload(){

	return true;
}

bool Client::requestUpload(){

	return true;
}

bool Client::requestStream(){

	return true;
}

bool Client::saveToFile(){

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
-- INTERFACE:	ostream& operator<< (ostream& os, const Client& c)
--				os: An output stream
--				c: The Client object that needs to be outputed
--
-- RETURNS:		Returns an output stream containing the client's data members
--
-- NOTES:		
----------------------------------------------------------------------------------------------------------------------*/
ostream& operator<< (ostream& os, const Client& c){

	return os;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	operator>>
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	istream& operator>> (istream& is, Client& c)
--				is: An iutput stream
--				c: The Client object to be read
--
-- RETURNS:		Returns the state of the input stream after the client object was read.
--				If a client object was read successful the return value evaluates to true, and false if it fails.
--
-- NOTES:		
----------------------------------------------------------------------------------------------------------------------*/
istream& operator>> (istream& is, Client& c){

	
	return is;

}
