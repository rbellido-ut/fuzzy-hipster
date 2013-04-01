#ifndef __SERVER_H
#define __SERVER_H

#include <string>
#include <vector>

DWORD WINAPI handleClientRequests(LPVOID param);
DWORD WINAPI listenThread(LPVOID args);
DWORD WINAPI multicastThread(LPVOID args);
ServerState DecodeRequest(char * request, std::string& filename, int& uploadfilesize);
void requestDispatcher(ServerState prevState, ServerState currentState, SOCKET clientsocket, std::string filename = "", int uploadfilesize = 0);
std::string getMusicDir();
int populateSongList(std::vector<std::string>& song_list);

#endif
