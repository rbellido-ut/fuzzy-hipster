#ifndef __MICROPHONE_H
#define __MICROPHONE_H

#include "utils.h"
#include "server_net.h"
#include "SFML\Audio.hpp"

class MicChat : public sf::SoundRecorder
{
	virtual bool onStart();
	virtual bool OnProcessSamples(const sf::Int16 * Samples, std::size_t SamplesCount);
	virtual void onStop();
};

#endif