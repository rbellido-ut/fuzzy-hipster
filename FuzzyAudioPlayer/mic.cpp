#include "mic.h"

using namespace std;
using namespace libZPlay;

int startMicSession()
{
	return 0;
}

int __stdcall micCallback(void * instasnce, void * user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
{
	LPMICVARS micvar = (LPMICVARS) user_data;
	switch (message)
	{
		case MsgWaveBuffer:
			if (sendto(micvar->micsocket, (const char *) param1, param2, 0, (const SOCKADDR *)& micvar->micaddr, sizeof(micvar->micaddr)) < 0)
			{
				cerr << "Error streaming UDP with microphone: " << GetLastError() << endl;
				return 0;
			}
		break;

		case MsgStop:
			return closesocket(micvar->micsocket);
	}
}
