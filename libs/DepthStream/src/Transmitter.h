#pragma once

#include <stdlib.h>
#include <vector>
#include <sys/types.h>
#ifdef _WIN32
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <netdb.h> //hostent
#endif

#include <thread>

namespace depth {
  class Transmitter;
  typedef std::shared_ptr<Transmitter> TransmitterRef;

  /**
   * \brief Network stream transmitter class
   *
   * The Transmitter class runs a network server on separate thread, which receives
   * incoming connection requests over TCP and starts sending a (compressed) stream
   * of Frames when a connection is established.
   */
  class Transmitter {

    public:

      /// The constructor immediately starts the network server thread
      Transmitter(int port);

      /// The destructor stops the network-server if it's still running
      ~Transmitter();

      /// Transmits a the given frame-data if the network-server has a connected client
      bool transmit(const void* data, size_t size);

      /// Stops the network server
      void stop(bool wait=true);

    protected:

      void error(const char *msg) {
          perror(msg);
      }

      void unbind();
      bool bind();
      void serverThread();

      bool transmitRaw(const void* data, size_t size);

    private:
      static const int MAXPACKETSIZE = 16;
      char packet[MAXPACKETSIZE];

      int port;
      std::thread *thread = NULL;

      bool bRunning=true;
      bool bBound=false;
      bool bConnected=false;

      int sockfd, clientsocket, portno;
      int cycleSleep=200;
  };
}
