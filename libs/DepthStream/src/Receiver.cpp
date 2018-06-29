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

#include <stdio.h>
#include <time.h>
#include <iostream>
#include <sstream> // std::stringstream
#include <chrono>

#include <string.h>
#ifdef _WIN32
	#pragma comment(lib, "ws2_32.lib")
	#include "Windowsstuff.h"
#else
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h> //inet_addr
	#include<netdb.h> //hostent
#endif

#include <functional>
#include <math.h>
#include "zlib.h"

#include "OniSampleUtilities.h"
#include "Receiver.h"

using namespace depth;

Receiver::~Receiver() {
  this->stop(false);
}

void Receiver::start(const std::string& host, int port) {
  this->stop(true); // calling start while running will cause restart
  this->host = host;
  this->port = port;
  bRunning = true;
  // start thread
  this->thread = new std::thread(std::bind(&Receiver::threadFunc, this));
}

void Receiver::stop(bool wait){
  // this->cout() << "stopping client thread...";
  bRunning = false;
  this->disconnect();

  if(wait) {
    if(this->thread) {
      std::cout << "[depth::Receiver] waiting for thread to end" << std::endl;
      thread->join();
      // this->cout() << " done" << std::endl;
      delete thread;
      thread=NULL;
    }
  }
}

void Receiver::error(const char *msg) {
    perror(msg);
    // exit(1);
}

void Receiver::threadFunc() {
  auto nextConnectAttemptTime = std::chrono::steady_clock::now();

  // connect/reconnect loop
  while (bRunning) {
    // not connected?
    if(!bConnected) {
      // time for a new connect attempt?
      auto now = std::chrono::steady_clock::now();
      if(now >= nextConnectAttemptTime) {
        if(bVerbose) this->cout() << "Attempting connection to: " << this->host << ":" << this->port << std::endl;
        // attempt fails
        if(!this->connectToServer(this->host, this->port)){
          if(bVerbose) this->cout() << "connection failure..." << std::endl;
          // schedule retry
          nextConnectAttemptTime = now + std::chrono::milliseconds(connectAttemptInterval);
        } else {
          this->cout() << "connected..." << std::endl;
        }
      }
    }

    int packageSize=0;

    // read header
    if(bConnected) {
      // if(bVerbose) this->cout() << "receive header..." << std::endl;
      // get 4-byte header

      if(this->receiveInt(packageSize)) {
        if(bSuperVerbose) this->cout() << "got header for " << packageSize << " bytes" << std::endl;
      } else {
        this->disconnect();
      }
    }

    // read body
    if(bConnected && packageSize > 0) {
      // get X-byte package
      if(!this->receive(this->buffer, packageSize)) {
        this->disconnect();
      } else {
        if(bSuperVerbose) this->cout() << "received: " << packageSize << " byte-package" << std::endl;
        this->bHasNew = true;
        this->lastPackageSize = packageSize;

        // our depth::Buffer interface
        Buffer::write(this->buffer, packageSize);

        if(frameCallback)
          frameCallback((const void*)(this->buffer), packageSize);
      }
    }
  }

  if(bVerbose) this->cout() << "Thread end... " << std::endl;
}

bool Receiver::connectToServer(const std::string& address, int port) {
#ifdef _WIN32
  makeSureWindowSocketsAreInitialized();
#endif

  //create socket if it is not already created
  // if(sock == -1)
  // {
      //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1)
  {
      error("Could not create socket");
  }

  // std::cout<<"Socket created\n";
  // }
  // else    {   /* OK , nothing */  }

  // memset(&server, '\0', sizeof(server));
  //setup address structure
  if(inet_addr(address.c_str()) == INADDR_NONE)
  {
      struct hostent *he;
      struct in_addr **addr_list;

      //resolve the hostname, its not an ip address
      if ( (he = gethostbyname( address.c_str() ) ) == NULL)
      {
          //gethostbyname failed
          error("gethostbyname");
          std::cout<<"Failed to resolve hostname\n";

          return false;
      }

      //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
      addr_list = (struct in_addr **) he->h_addr_list;

      for(int i = 0; addr_list[i] != NULL; i++)
      {
          //strcpy(ip , inet_ntoa(*addr_list[i]) );
          server.sin_addr = *addr_list[i];

          std::cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i]) << std::endl;

          break;
      }
  }

  //plain ip address
  else
  {
      server.sin_addr.s_addr = inet_addr( address.c_str() );
  }

  server.sin_family = AF_INET;
  server.sin_port = htons( port );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
      // perror("connect failed. Error");
      this->cout() << "Failed to connect to " << address << ":" << port << std::endl;
      return false;
  }

  this->cout() << "connected to " << address << ":" << port << std::endl;
  bConnected=true;
  return true;
}

void Receiver::disconnect(){
#ifdef _WIN32
  closesocket(sock);
#else
  close(sock);
#endif

  if(bConnected) {
    bConnected=false;
    this->cout() << "disconnected from " << this->host << ":" << this->port << std::endl;
  }
}

bool Receiver::receive(size_t size){
  if(size > BUF_SIZE) {
    std::cerr << "Requested recveive-size larger than buffer" << std::endl;
    return false;
  }

  return this->receive(this->buffer, size);
}

bool Receiver::receive(char* buffer, size_t size) {
  // if(size > bufferSize){
  //   if(buffer){
  //     delete buffer;
  //     buffer = new char[size];
  //     bufferSize = size;
  //   }
  // }

  size_t amount=0;

  while(bConnected && amount < size) {
    int packageSize = recv(sock, buffer+amount, size-amount, 0);

    if(packageSize <= 0) {
      // this->disconnect();
      return false;
    }

    // if(packageSize <= 0) {
    //
    //   // perror("recv failed");
    //   return false;
    // }

    amount += packageSize;
  }

  this->recvSize = amount;
  return true;
}

bool Receiver::receiveInt(int& target){
  char buf[4];
  if(!this->receive(buf, 4))
    return false;

  int b0 = (int)(0x0ff & buf[0]);
  int b1 = (int)(0x0ff & buf[1]);
  int b2 = (int)(0x0ff & buf[2]);
  int b3 = (int)(0x0ff & buf[3]);
  target = ((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
  return true;
}

bool Receiver::send_data(std::string data) {
    this->cout() << "Sending data...";
    //Send some data
    if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    std::cout<<"Sent\n";

    return true;
}
