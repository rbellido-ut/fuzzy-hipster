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
int __stdcall micCallback(void * instasnce, void * user_data, TCallbackMessage message, unsigned int param1, unsigned int param2);

#endif