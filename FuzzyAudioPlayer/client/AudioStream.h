class AudioStream : public sf::SoundStream
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