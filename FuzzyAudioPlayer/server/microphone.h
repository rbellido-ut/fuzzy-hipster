#ifndef __MICROPHONE_H
#define __MICROPHONE_H

#include "utils.h"
#include "server_net.h"


class MicChat : public sf::SoundRecorder
{
	virtual bool onStart(); //optional to implement
	//virtual bool onProcessSamples(const sf::Int16 * Samples, std::size_t SamplesCount);
	virtual void onStop(); //optional to implement

	int micSession();
};

#endif