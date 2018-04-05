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
	#include <netdb.h> //hostent
#endif
#include <functional>
#include <math.h>

#include "OniSampleUtilities.h"
#include "Transmitter.h"

using namespace depth;

Transmitter::Transmitter(int port) : port(port) {
  bRunning=true;
  this->thread = new std::thread(std::bind(&Transmitter::serverThread, this));
}

Transmitter::~Transmitter() {
  stop(false);
}

void Transmitter::stop(bool wait){
  unbind();
  bRunning = false;

  if(wait){
    if(this->thread) {
      // std::cout << "stopping server thread";
      thread->join();
      // std::cout << "server thread done";
      delete thread;
      thread=NULL;
    }
  }
}

bool Transmitter::transmit(const void* data, size_t size) {
  // transmittion-header; 4-byte package length
  char header[4];
  header[0] = (char)((size >> 24) & 0xff);
  header[1] = (char)((size >> 16) & 0xff);
  header[2] = (char)((size >> 8) & 0xff);
  header[3] = (char)(size & 0xff);
  return transmitRaw(header, 4) && transmitRaw(data, size);
}

bool Transmitter::transmitRaw(const void* data, size_t size){
  if (!bConnected) {
    // std::cout << "no client, didn't sent " << size << " bytes." << std::endl;
    return false;
  }

  #ifdef _WIN32
	auto n = send(clientsocket, (char*)data, size, 0);
  #else
	auto n = write(clientsocket, data, size);
  #endif

  if(n < 0) {
    bConnected=false;
	#ifdef _WIN32
		closesocket(clientsocket);
	#else
		close(clientsocket);
	#endif
    return false;
  }

  return true;
}

bool Transmitter::bind() {
#ifdef _WIN32
  makeSureWindowSocketsAreInitialized();
#endif

  struct sockaddr_in serv_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  int option=1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

  if (sockfd < 0) {
#ifdef _WIN32
	  std::cerr << "ERROR opening socket, WSAGetLastError gives: " << WSAGetLastError() << std::endl;
#else
     error("ERROR opening socket");
#endif
     return false;
   }

  // bzero((char *) &serv_addr, sizeof(serv_addr));
  memset((char *)&serv_addr, 0, sizeof(serv_addr));

  portno = this->port;//atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     error("ERROR on binding");
     return false;
   }

  listen(sockfd, 5);
  bBound = true;
  return true;
}

void Transmitter::unbind() {
#ifdef _WIN32
  closesocket(sockfd);
  closesocket(clientsocket);
#else
  close(sockfd);
  close(clientsocket);
#endif
}

void Transmitter::serverThread() {
  struct sockaddr_in cli_addr;
  int n;

  while(bRunning) {
    if(!bBound) {
      this->bind();
    }

    if(bBound) {
      if(!bConnected) {
#ifdef _WIN32
        int clilen = sizeof(cli_addr);
#else
        socklen_t clilen = sizeof(cli_addr);
#endif
        clientsocket = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (clientsocket < 0) {
          error("ERROR on accept");
          bConnected=false;
        } else {
          std::cout << "client connected" << std::endl;
          bConnected=true;
        }
      }

      if(bConnected) {
        n=recv(clientsocket,packet,1,0);

        if(n > 0){
          std::cerr << "TODO: handle incoming data in depth::Transmitter, disconnecting for now" << std::endl;
        }

#ifdef _WIN32
		closesocket(clientsocket);
#else
		close(clientsocket);
#endif


      }
    }

    Sleep(this->cycleSleep);
  }
}
