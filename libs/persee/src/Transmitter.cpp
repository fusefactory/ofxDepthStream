#include <stdio.h>
#include <time.h>
#include <iostream>
#include <sstream> // std::stringstream
#include <chrono>

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr

#include <netdb.h> //hostent
#include <functional>
#include <math.h>

#include "OniSampleUtilities.h"
#include "Transmitter.h"

using namespace persee;

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

  auto n = write(clientsocket, data, size);
  if(n < 0) {
    bConnected=false;
    close(clientsocket);
    return false;
  }

  return true;
}

bool Transmitter::bind() {
  struct sockaddr_in serv_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  int option=1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if (sockfd < 0) {
     error("ERROR opening socket");
     return false;
   }

  bzero((char *) &serv_addr, sizeof(serv_addr));
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
  close(sockfd);
  close(clientsocket);
}

void Transmitter::serverThread() {
  struct sockaddr_in cli_addr;
  socklen_t clilen;
  int n;

  while(bRunning) {
    if(!bBound) {
      this->bind();
    }

    if(bBound) {
      clilen = sizeof(cli_addr);

      if(!bConnected) {
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

        if(n < 1){
          close(clientsocket);
          bConnected=false;
        } else {
          std::cerr << "TODO: handle incoming data in persee::Transmitter, disconnecting for now" << std::endl;
          close(clientsocket);
          bConnected=false;
        }
      }
    }

    Sleep(this->cycleSleep);
  }
}
