/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:		main.cpp
--
-- PROGRAM:			COMP4985 - COMM AUDIO
--
-- FUNCTIONS:		int main(int argc, char* argv[])
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	main
--
-- DATE:		March 4th, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Behnam Bastami
--
-- PROGRAMMER:	Behnam Bastami
--
-- INTERFACE:	int main(int argc, char* argv[])
argc: The number of command line arguments
argv[]: The array containing the command line arguments 
--
-- RETURNS:		Exit status 0 if program exits successfully.
--
-- NOTES:		
--
----------------------------------------------------------------------------------------------------------------------*/

Server sv;

int main(int argc, char* argv[]){

	//if(argc < 2){

		//cout << "Usage: ./run mode" <<endl;

	//}else{

		//if(strcmp(argv[1], "server") == 0){ 		//User wanted a server instance

			//initialize and start the server
			Server srvr;
			WSADATA wsadata;
			SOCKET s;

			srvr.initTCPServer(&wsadata, &s);
			srvr.startTCPServer(&s);

	//	}else if(strcmp(argv[1], "client") == 0){	//User wanted a client instance

	//		Client clnt;					//initialize and start the client

	//	}else{

	//		cout << "Incorrect Mode" << endl;

	//	}
	//}

	exit(0);
}
