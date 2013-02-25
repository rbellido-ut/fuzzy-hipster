/* I was playing around with SFML a bit today.  I got an audio file playing
 * from my hard drive fairly quickly.  I can say first hand this library is
 * brilliantly easy to use.  In order to just open a nice, boring window
 * that will play you a song, the following code will get you there. */

// Note that you need to:
//  - Have SFML installed
//  - Set up your VS2012 project for SFML use
//      (I'll help out with this later, there's a few
//       tricks to get it working properly)
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance,
        HINSTANCE hprevInstance,
        LPSTR lspszCmdParam,
        int nCmdShow)
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "Play Music");
    sf::Music music;

    // This is just opening a *.flac file from my mounted network
    // drive.  *.ogg, *.flac and *.wav are all supported, as well
    // as numerous other formats as seen here:
    //  http://www.sfml-dev.org/features.php
    if (!music.openFromFile("Z:\\The Acacia Strain\\2010 - Wormwood\\08 - Jonestown.flac"))
        return -1;

    music.play();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Close)
                window.close();
        }

        window.clear();
        window.display();
    }

    return 0;
}

/* Now obviously we need to read from a stream.  Fortunately, SFML provides
 * a method of reading from custom streams.  This is a simple example, which
 * doesn't use networking, but is the general approach to custom streams. */
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <vector>

class MyStream : public sf::SoundStream
{
    public:
        void load(const sf::SoundBuffer& buffer)
        {
            m_samples.assign(buffer.getSamples(), buffer.getSamples() + buffer.getSampleCount());
            m_currentSample = 0;
            initialize(buffer.getChannelCount(), buffer.getSampleRate());
        }

    private:
        virtual bool onGetData(Chunk& data)
        {
            const int samplesToStream = 50000;
            data.samples = &m_samples[m_currentSample];

            if (m_currentSample + samplesToStream <= m_samples.size())
            {
                data.sampleCount = samplesToStream;
                m_currentSample += samplesToStream;
                return true;
            }
            else
            {
                data.sampleCount = m_samples.size() - m_currentSample;
                m_currentSample = m_samples.size();
                return false;
            }
        }

        virtual void onSeek(sf::Time timeOffset)
        {
            m_currentSample = static_cast<std::size_t>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
        }

        std::vector<sf::Int16> m_samples;
        std::size_t m_currentSample;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");

    sf::SoundBuffer buffer;
    buffer.loadFromFile("Z:\\The Acacia Strain\\2010 - Wormwood\\08 - Jonestown.flac");

    MyStream stream;
    stream.load(buffer);
    stream.play();

    while(stream.getStatus() == MyStream::Playing)
        sf::sleep(sf::seconds(0.1f));

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.display();
    }

    return 0;
}

