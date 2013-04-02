#ifndef __MIC_H
#define __MIC_H

#include "server\utils.h"
#include "server\server_net.h"
#include "libzplay\libzplay.h"

typedef struct 
{
	SOCKET micsocket;
	SOCKADDR_IN micaddr;

} MICVARS, *LPMICVARS;

int startMicSession();
int __stdcall micCallback(void * instance, void * user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);

#endif