Fuzzy Audio Player
===============
Fuzzy Audio Player is a network streaming audio player, designed and implemented for our BCIT Data Communications course (COMP 4985) final project.  
It is a Win32 application written in C++, making use of the [libzplay library](http://libzplay.sourceforge.net/WELCOME.html) for it's audio component, and the Windows Socket API for the network component.
The application suite comes with a server and a client application.

Features
========
* Streams .wav files
* Multicasting
* Upload songs to the server
* Download songs from the server

Getting Started
===============
Before starting, make sure that you have the libzplay.dll in the same folder as all the .exe files. In addition,
make sure to have a 'Stream' folder which will contain all the .wav files you want to listen to through the player.

Server
-------
Launching the `FuzzyAudioPlayerServer.exe` will start the server. It automatically starts a multicast channel, streaming
all the .wav songs contained in the 'Stream' folder; make sure you have at least one .wav file in this folder to
start streaming.


Client
-------
* Launch the `FuzzyAudioPlayerClient.exe` to launch the client.
* Type the correct IP address and port of the server.
* Once finished, you will see a list of songs available for downloading or streaming.
* To select an operation, click the radio button and press the 'OK' button.

For more information on how the application works and the design, see the [documentation](https://github.com/rbellido/fuzzy-hipster/blob/master/design/Design_v2.0.docx).

fuzzy-hipster
=============
Fuzzy Hipsters is the team and this is the Fuzzy Audio Player.
