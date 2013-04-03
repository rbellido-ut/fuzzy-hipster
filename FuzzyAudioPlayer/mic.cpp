#include "mic.h"

using namespace std;
using namespace libZPlay;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	startMicSession
--
-- DATE:		April 1, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Ronald Bellido
--
-- PROGRAMMER:	Ronald Bellido
--
-- INTERFACE:	int startMicSession()
--
-- RETURNS:		0 - Something failed, 1 - All ok
--				
--
-- NOTES:
This function will start microphone session
----------------------------------------------------------------------------------------------------------------------*/
int startMicSession()
{
	LPMICVARS micvar = (LPMICVARS) malloc(sizeof(MICVARS));
	WSADATA wsadata;
	micvar->micsocket = createServer(&wsadata, UDP, &micvar->micaddr);

	ZPlay * netstream = CreateZPlay();
	
	netstream->SetSettings(sidSamplerate, 44100);
	netstream->SetSettings(sidChannelNumber, 2);
	netstream->SetSettings(sidBitPerSample, 16);
	netstream->SetSettings(sidBigEndian, 1);

	int n;
	if (netstream->OpenStream(1, 1, &n, 1, sfPCM) == 0)
	{
		cerr << "Error opening a microphone stream: " << netstream->GetError() << endl;
		netstream->Release();
		free(micvar);
		return 0;
	}

	ZPlay * player = CreateZPlay();

	if (player->OpenFile("wavein://", sfAutodetect) == 0)
	{
		cerr << "Error in OpenFile: " << player->GetError() << endl;
		netstream->Release();
		player->Release();
		free(micvar);
		return 0;
	}

	player->SetCallbackFunc(micCallback, (TCallbackMessage) (MsgWaveBuffer | MsgStop), (VOID*) micvar); //setup callback function whenever a mic picks up a sound

	player->Play(); //start listening to the mic

	while (1)
	{
		char * buffer = new char[DATABUFSIZE];
		int size = sizeof(micvar->micaddr);
		int bytesrecvd;
		if ((bytesrecvd = recvfrom(micvar->micsocket, buffer, DATABUFSIZE, 0, (SOCKADDR*)&micvar->micaddr, &size)) == -1)
		{
			cerr << "Error in recvfrom: " << WSAGetLastError() << endl;
			break;
		}

		netstream->PushDataToStream(buffer, bytesrecvd);
		delete buffer;
		netstream->Play(); //send the received data into the stream
		
		TStreamStatus status;
		player->GetStatus(&status);
		if (status.fPlay == 0)
			break; //microphone not playing anymore

		TStreamTime pos;
		player->GetPosition(&pos);
		cout << "Pos: " << pos.hms.hour << " " << pos.hms.minute << " " <<
			pos.hms.second << " " << pos.hms.millisecond << endl;
	}

	free(micvar);
	player->Release();
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	micCallbakc
--
-- DATE:		April 1, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Ronald Bellido
--
-- PROGRAMMER:	Ronald Bellido
--
-- INTERFACE:	int __stdcall micCallback(void * instance, void * user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
--
-- RETURNS:		Dependent on the type of the TCallbackMessage (message) passed. See NOTES below.
--				
--
-- NOTES:
This function will mainly listen for the MsgWaveBuffer message, which is a message for when a decoding thread is
ready to send data to the soundcard.
	Important parameters:
		param1 - pointer to memory PCM buffer
		param2 - number of bytes in PCM data buffer
	Returns:
		0 - send data to soundcard
		1 - skip sending data to soundcard
		2 - stop playing

For MsgStop, all the parameters and the return type are not used. The message is used after a song stops playing.
----------------------------------------------------------------------------------------------------------------------*/
int __stdcall micCallback(void * instance, void * user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
{
	LPMICVARS micvar = (LPMICVARS) user_data;
	switch (message)
	{
		case MsgWaveBuffer:
			if (sendto(micvar->micsocket, (const char *) param1, param2, 0, (const SOCKADDR *)& micvar->micaddr, sizeof(micvar->micaddr)) < 0)
			{
				cerr << "Error in sendto: " << GetLastError() << endl;
				return 2;
			}
		break;

		case MsgStop:
			return closesocket(micvar->micsocket);
	}

	return 1;
}