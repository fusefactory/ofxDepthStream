//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#pragma once

#include <string>
#include <thread>
#include <iostream>
#include <sys/types.h>
#ifdef _WIN32
	#include "winsock.h"
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h> //inet_addr
#endif

// local
#include "Buffer.h"

namespace depth {

  class Receiver;
  typedef std::shared_ptr<Receiver> ReceiverRef;

  /**
   * \brief Network stream receiver class
   *
   * The receiver class runs a network client on separate thread, which receives
   * a (compressed) stream of Frames (probably sent by a Transmitter instance).
   * The Receiver class inherits from the Buffer class and thus lets you
   * automatically redirect newly received data using the setOutputTo method.
   */
  class Receiver : public Buffer {

    public: // types, consts and static factory methods

      const static int DEFAULT_PORT = 4445;

      typedef std::function<void(const void* data, size_t size)> FrameCallback;

      /// Starts a Receiver instance which tries to connect to a server at the specified address, using the DEFAULT_PORT
      static ReceiverRef createAndStart(const std::string& host) {
        return createAndStart(host, DEFAULT_PORT);
      }

      /// Starts a Receiver instance which tries to connect to a server at the specified address and port
      static ReceiverRef createAndStart(const std::string& host, int port) {
        return std::make_shared<Receiver>(host, port);
      }

    public:
      /// Default constructor; does not start a network client
      Receiver() {}

      /// This constructor immediately starts the network client in a separate thread
      Receiver(const std::string& host, int port) : thread(NULL) {
        start(host, port);
      }

      ~Receiver();

      /// Starts network client, with host and port values provides in the constructor
      void start(){
        this->start(host, port);
      }

      /// Starts network client with the specified host and port values
      void start(const std::string& host, int port);

      /**
       * Stops network client
       * @param wait When true blocks until the network-client thread has finished. False by default.
       */
      void stop(bool wait=false);

      /// Provides a pointer to the incoming data memory block (never returns NULL)
      char* getData() { return (char*)(buffer + 4); }
      /// The size (in number of bytes) of the last package that was received (excluding the 4-byte package header)
      int getSize() const { return lastPackageSize; }
      /// Whether a new package was received since the last call to reset()
      bool hasNew() const { return bHasNew; }
      /// Resets the hasNew flag
      void reset(){ bHasNew = false; }
      /// Configures the network client to attempt reconnect at the specified interval
      /// @param interval interval in milliseconds, default is 5000
      void setConnectAttemptInterval(unsigned int interval) { connectAttemptInterval = interval; }
      /// Enables/disables verbose logging output
      /// @param v enable or disables
      void setVerbose(bool v){ bVerbose=v; }
      /// Registers a custom callback for incoming frame-data DEPRECATED: use the inherited Buffer::setOutputTo method
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
