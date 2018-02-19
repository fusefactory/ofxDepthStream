#pragma once

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <thread>

namespace persee {
  class Transmitter {

    public:

      Transmitter(int port);
      ~Transmitter();

      bool transmit(char* data, size_t size);

      bool hasClient() const {
        return bConnected;
      }

    protected:

      void error(const char *msg) {
          perror(msg);
      }

      bool start();
      void serverThread();

    private:
      static const int MAXPACKETSIZE = 16;
      char packet[MAXPACKETSIZE];

      int port;
      std::thread *thread = NULL;

      bool bRunning=true;
      bool bConnected=false;
      struct hostent *he;
      int sockfd, newsockfd, portno;
      socklen_t clilen;
      char buffer[256];
      struct sockaddr_in serv_addr, cli_addr;
  };
}
