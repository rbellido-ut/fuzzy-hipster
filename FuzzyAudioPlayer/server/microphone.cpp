#include "microphone.h"

bool MicChat::onStart()
{
	return true;
}

bool MicChat::OnProcessSamples(const sf::Int16 * Samples, std::size_t SamplesCount)
{
	return true;
}

void MicChat::onStop()
{
}