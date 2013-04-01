#include "utils.h"
#include <SFML/Audio.hpp>
#include "NetStream.h"

class Client {

public:
	Client()
	{ 
		currentState = NOTCONNECTED;
		connectSocket_ = 0; 
	}
	~Client(){ }

	bool runClient(WSADATA *wsadata, const char*, const int);
	

	int currentState;
	DWORD dlThreadID;
	HANDLE dlThreadHandle, stThreadHandle, ulThreadHandle;

	DWORD stThreadID, ulThreadID;
	DWORD stThread(LPVOID);
	static DWORD WINAPI runSTThread(LPVOID);

	DWORD dlThread(LPVOID);
	static DWORD WINAPI runDLThread(LPVOID);

	DWORD ulThread(LPVOID param);
	static DWORD WINAPI runULThread(LPVOID param);

	void dispatchOneSend(std::string dlReq);
	void dispatchOneRecv();

	std::ofstream downloadFileStream;
	std::ifstream uploadFileStream;
	int sizeOfDownloadFile;
	int sizeOfUploadFile;

	int dlFileSize, ulFileSize;
	int downloadedAmount, uploadedAmount;

private:
	SOCKET connectSocket_;
	SOCKADDR_IN addr_;
	hostent *hp_;
	NetStream stream_;

	SOCKET createTCPClient(WSADATA*, const char*, const int);

	static DWORD WINAPI runRecvThread(LPVOID param);
	DWORD WINAPI Client::recvThread(/*LPVOID param*/);

	LPSOCKETDATA allocData(SOCKET fd);
    void freeData(LPSOCKETDATA data);

	bool dispatchWSASendRequest(LPSOCKETDATA data);
    bool dispatchWSARecvRequest(LPSOCKETDATA data);
	void recvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    void sendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runRecvComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
    static void CALLBACK runSendComplete (DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

};

/*class AudioStream : public sf::SoundStream
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
};*/