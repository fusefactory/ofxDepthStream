#pragma once

#include <string>
#include <thread>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr
// local
#include "Buffer.h"

namespace depth {

  class Receiver;
  typedef std::shared_ptr<Receiver> ReceiverRef;


  class Receiver : public Buffer {

    public: // types, consts and static factory methods

      const static int DEFAULT_PORT = 4445;

      typedef std::function<void(const void* data, size_t size)> FrameCallback;

      static ReceiverRef createAndStart(const std::string& host) {
        return createAndStart(host, DEFAULT_PORT);
      }

      static ReceiverRef createAndStart(const std::string& host, int port) {
        return std::make_shared<Receiver>(host, port);
      }

    public:
      Receiver() : thread(NULL) {}

      Receiver(const std::string& host, int port) : thread(NULL) {
        start(host, port);
      }

      ~Receiver();

      void start(){
        this->start(host, port);
      }

      void start(const std::string& host, int port);
      void stop(bool wait=false);
      char* getData() { return (char*)(buffer + 4); }
      int getSize() const { return lastPackageSize; }
      bool hasNew() const { return bHasNew; }
      void reset(){ bHasNew = false; }

      void setConnectAttemptInterval(unsigned int interval) { connectAttemptInterval = interval; }
      void setVerbose(bool v){ bVerbose=v; }
      void setFrameCallback(FrameCallback func) { frameCallback = func; }

    protected:

      std::ostream& cout() { return std::cout << "[depth::Receiver] "; }
      std::ostream& cerr() { return std::cerr << "[depth::Receiver] "; }
      void error(const char *msg);

      void threadFunc();

      bool connectToServer(const std::string& address, int port);
      void disconnect();
      bool receive(size_t size);
      bool receive(char* buffer, size_t size);
      bool receiveInt(int& target);
      bool send_data(std::string data);


    private:
      std::thread* thread=NULL;
      bool bRunning=true;
      bool bConnected=false;
      bool bHasNew=false;
      int cycleSleep=10;
      bool bVerbose=false, bSuperVerbose=false;

      int sock=-1;
      std::string host;
      int port;
      struct sockaddr_in server;

      static const int BUF_SIZE = (1280*720*4);
      char buffer[BUF_SIZE];
      // int bufferSize=0;
      int recvSize=0;
      int lastPackageSize=0;

      unsigned int connectAttemptInterval = 5000;
      FrameCallback frameCallback=nullptr;
  };
}
