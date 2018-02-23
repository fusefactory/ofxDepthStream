#pragma once

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> //hostent
#include <thread>

namespace persee {
  class Transmitter {

    public:
      typedef std::function<void(Transmitter&,char)> ByteHandler;
      typedef std::function<void(Transmitter&)> TransmitterHandler;
      typedef TransmitterHandler BindFailedHandler;
      typedef TransmitterHandler BoundHandler;

    public:

      Transmitter(int port);
      ~Transmitter();

      void stop(){ bRunning = false; }

      int getPort() const { return port; }
      void setPort(int newport) { port = newport; }

      bool transmitRaw(const char* data, size_t size);
      bool transmitInt(int value);
      bool transmitFrame(const char* data, size_t size);


      bool hasClient() const {
        return bConnected;
      }

      void setFirstByteHandler(ByteHandler func) { this->firstByteHandler = func; }
      void setBindFailedHandler(BindFailedHandler func) { bindFailedHandler = func; }
      void setBoundHandler(BoundHandler func){ boundHandler = func; }
      void setUnboundHandler(TransmitterHandler func) { unboundHandler = func; }
      void setDisconnectHandler(TransmitterHandler func) { this->disconnectHandler = func; }

      void whenBound(BoundHandler func){
        if(bBound) {
          func(*this);
          return;
        }

        this->setBoundHandler(func);
      }

      void whenUnbound(TransmitterHandler func){
        if(!bBound) {
          func(*this);
          return;
        }

        this->setUnboundHandler(func);
      }

      void whenDisconnected(TransmitterHandler func) {
        if(this->bConnected) {
          this->setDisconnectHandler(func);
        } else {
          func(*this);
        }
      }

    protected:

      std::ostream& cout() { return std::cout << "[persee::Transmitter] "; }
      std::ostream& cerr() { return std::cerr << "[persee::Transmitter] "; }

      void error(const char *msg) {
        perror(msg);
      }

      bool bindServer();
      void serverThread();

    private:
      static const int MAXPACKETSIZE = 16;
      char packet[MAXPACKETSIZE];

      int port;
      std::thread *thread = NULL;

      bool bRunning=true;
      bool bConnected=false;
      bool bBound=false;
      struct hostent *he;
      int sockfd, newsockfd;

      ByteHandler firstByteHandler = nullptr;
      BindFailedHandler bindFailedHandler = nullptr;
      BoundHandler boundHandler = nullptr;
      TransmitterHandler disconnectHandler=nullptr;
      TransmitterHandler unboundHandler=nullptr;
  };
}
