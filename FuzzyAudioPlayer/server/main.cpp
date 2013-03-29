#include "../utils.h"
#include "server.h"

using namespace std;

int main(int argc, char* argv[])
{
	Server srvr;
	WSADATA wsaData;
	srvr.runServer(&wsaData);

	getchar();

	return EXIT_SUCCESS;
}
