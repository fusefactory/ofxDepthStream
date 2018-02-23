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
  this->thread = new std::thread(std::bind(&Transmitter::serverThread, this));
}

Transmitter::~Transmitter() {
  if(this->thread) {
    this->cout() << "stopping server thread" << this->port << std::endl;
    bRunning = false;
    thread->join();
    this->cout() << "server thread for port " << this->port << " stopped" << std::endl;
    delete thread;
  }
}

bool Transmitter::transmitRaw(const char* data, size_t size) {
  if (!bConnected) {
    // this->cout() << "no client, didn't sent " << size << " bytes." << std::endl;
    return false;
  }

  // content
  // this->cout() << "Sending raw data: " << *data << std::endl;
  auto n = send(newsockfd, data, size, 0);

  if (n < 0) {
    error("ERROR writing data to socket");
    return false;
  }

  // this->cout() << "sent " << size << " bytes." << std::endl;
  return true;
}

bool Transmitter::transmitInt(int value){
  // transmittion-header; 4-byte package length
  char buffer[4];
  buffer[0] = (char)((value >> 24) & 0x0ff);
  buffer[1] = (char)((value >> 16) & 0x0ff);
  buffer[2] = (char)((value >> 8) & 0x0ff);
  buffer[3] = (char)(value & 0x0ff);
  return this->transmitRaw(buffer, 4);
}

bool Transmitter::transmitFrame(const char* data, size_t size) {
  return transmitInt(size) && transmitRaw(data, size);
}


bool Transmitter::bindServer() {
  struct sockaddr_in serv_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  int option=1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if (sockfd < 0) {
     error("ERROR opening socket");
     return false;
   }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(this->port);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     error("ERROR on binding");
     return false;
   }

  listen(sockfd, 5);
  this->bBound = true;
  return true;
}

void Transmitter::serverThread() {
  int n;
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  while(bRunning) {

    // bind
    if(!bBound) {
      if(!this->bindServer()) {
        if(this->bindFailedHandler) this->bindFailedHandler(*this);
        continue;
      }

      if(this->boundHandler) this->boundHandler(*this);
    }

    // receive client connection
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
      error("ERROR on accept");
      continue;
    }

    bConnected = true;
    this->cout() << "client connected to port " << this->port << std::endl;

    while(bRunning){
      // this->cout() << "waiting for client message" << std::endl;

      n=recv(newsockfd,packet,1,0);
      if(n==0){
        this->cout() << "client disconnected message" << std::endl;
        break;
      }

      if(n==1 && this->firstByteHandler) {
        this->firstByteHandler(*this, packet[0]);
      }

      Sleep(100);
      // msg[n]=0;
      //send(newsockfd,msg,n,0);
      // Message = string(msg);
    }

    this->cout() << "client disconnected from port " << this->port << std::endl;
    bConnected = false;
    if(this->disconnectHandler) this->disconnectHandler(*this);

    close(newsockfd);
  } // while running

  // close(sockfd);
  // this->bBound = false;
  // if(this->unboundHandler) this->unboundHandler(*this);
  // Sleep(1000);
}
