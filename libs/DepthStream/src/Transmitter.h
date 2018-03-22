#pragma once

#include <stdlib.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <thread>

namespace depth {
  class Transmitter;
  typedef std::shared_ptr<Transmitter> TransmitterRef;

  class Transmitter {

    public:

      Transmitter(int port);
      ~Transmitter();

      bool transmit(const void* data, size_t size);
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
