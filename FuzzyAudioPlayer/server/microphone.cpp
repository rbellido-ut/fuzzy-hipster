#include "microphone.h"

bool MicChat::onStart()
{
	return true;
}

bool MicChat::onProcessSamples(const sf::Int16 * Samples, std::size_t SamplesCount)
{
	//send over network
	return true;
}

void MicChat::onStop()
{
}

int MicChat::micSession()
{
	if (sf::SoundRecorder::isAvailable() == false)
	{
		std::cout << "Sorry, audio capture is not supported by your system" << std::endl;
		//perhaps send this same message to the client, or some error message
		return 0;
	}

	//initialize sample rate
}