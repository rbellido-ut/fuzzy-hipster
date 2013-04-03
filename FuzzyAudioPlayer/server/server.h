#ifndef __SERVER_H
#define __SERVER_H

#include <string>
#include <vector>
#include "../mic.h"
#include "server_net.h"
#include "utils.h"

typedef struct {
	std::ifstream * file;
	SOCKET socket;
	SOCKADDR_IN multaddr;
} MULTICASTVARS, *LPMULTICASTVARS;

DWORD WINAPI handleClientRequests(LPVOID param);
DWORD WINAPI listenThread(LPVOID args);
DWORD WINAPI multicastThread(LPVOID args);
ServerState DecodeRequest(char * request, std::string& filename, int& uploadfilesize);
void requestDispatcher(ServerState prevState, ServerState currentState, SOCKET clientsocket, std::string filename = "", int uploadfilesize = 0);
std::string getMusicDir();
int populateSongList(std::vector<std::string>& song_list);
int  __stdcall  multicastCallback(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);

#endif
